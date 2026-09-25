#include "database.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QSqlError>
#include <QRegularExpression>
#include <QUuid>
#include <stdexcept>
#include <cmath>

static void fail(const QString &s) { throw std::runtime_error(s.toStdString()); }
static void check(bool ok, const QSqlQuery &q) { if (!ok) fail(q.lastError().text()); }
QString money(qint64 cents) { return QString::number(cents/100.0, 'f', 2); }
QString quantity(double n) { return QString::number(n, 'f', 3).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\.$")); }

Database::Database() {
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(folder)) fail("Could not create application data directory");
    filePath_ = folder + "/japan365pos.sqlite";
    // One-time migration of records from the earlier RetailDesk-branded build, if present.
    const QString oldFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                            + "/RetailDesk/RetailDesk POS/retaildesk.sqlite";
    if (!QFile::exists(filePath_) && QFile::exists(oldFile) && !QFile::copy(oldFile, filePath_))
        fail("Could not migrate old SQLite database; copy it manually before starting");
    db_ = QSqlDatabase::addDatabase("QSQLITE", "japan365pos");
    db_.setDatabaseName(filePath_);
    if (!db_.open()) fail("SQLite: " + db_.lastError().text());
    exec("PRAGMA foreign_keys=ON");
    exec("PRAGMA busy_timeout=5000");
}
Database::~Database() { db_.close(); db_ = QSqlDatabase(); QSqlDatabase::removeDatabase("japan365pos"); }
QString Database::path() const { return filePath_; }
QString Database::now() const { return QDateTime::currentDateTimeUtc().toString(Qt::ISODate); }
QSqlQuery Database::query(const QString &sql, const QVariantList &args) {
    QSqlQuery q(db_); if (!q.prepare(sql)) fail(q.lastError().text());
    for (const QVariant &v : args) q.addBindValue(v);
    check(q.exec(), q); return q;
}
void Database::exec(const QString &s, const QVariantList &a) { query(s,a); }
QVariant Database::scalar(const QString &s, const QVariantList &a) { auto q=query(s,a); return q.next()?q.value(0):QVariant(); }
int Database::insert(const QString &s,const QVariantList &a) { auto q=query(s,a); return q.lastInsertId().toInt(); }
void Database::begin(){ if(!db_.transaction()) fail(db_.lastError().text()); }
void Database::commit(){ if(!db_.commit()) fail(db_.lastError().text()); }
void Database::rollback(){ db_.rollback(); }
void Database::initialize() {
    QFile f(":/schema.sql"); if (!f.open(QIODevice::ReadOnly)) fail("Embedded schema missing");
    QString sql=QString::fromUtf8(f.readAll());
    // The shipped schema contains only simple SQL statements, not trigger bodies.
    for (const QString &statement : sql.split(';')) {
        const QString s=statement.trimmed(); if (!s.isEmpty()) exec(s);
    }
    if (scalar("SELECT COUNT(*) FROM warehouses").toInt()==0) {
        exec("INSERT INTO warehouses(name,code) VALUES('Main Store','MAIN'),('Back Store','BACK')");
    }
    if (scalar("SELECT COUNT(*) FROM expense_categories").toInt()==0) {
        exec("INSERT INTO expense_categories(name) VALUES('Rent'),('Utilities'),('Transport'),('Salary'),('Supplies'),('Other')");
    }
}
void Database::requireActive(const QString &table,int id) {
    static const QStringList valid={"products","warehouses","payment_accounts","expense_categories"};
    if(!valid.contains(table) || !scalar("SELECT 1 FROM "+table+" WHERE id=? AND active=1",{id}).isValid()) fail("Selected record is unavailable or archived");
}
qint64 Database::unitCost(int id) { return scalar("SELECT cost_cents FROM products WHERE id=?",{id}).toLongLong(); }
int Database::createProduct(const QString &sku,const QString &name,const QString &cat,const QString &unit,qint64 cost,qint64 price,double min) {
    if(sku.trimmed().isEmpty()||name.trimmed().isEmpty()||cost<0||price<0||min<0) fail("Enter valid product details");
    return insert("INSERT INTO products(sku,name,category,unit,cost_cents,price_cents,min_stock) VALUES(?,?,?,?,?,?,?)",{sku.trimmed(),name.trimmed(),cat.trimmed(),unit.trimmed(),cost,price,min});
}
void Database::editProduct(int id,const QString &sku,const QString &name,const QString &cat,const QString &unit,qint64 cost,qint64 price,double min){
    requireActive("products",id);
    if(sku.trimmed().isEmpty()||name.trimmed().isEmpty()||cost<0||price<0||min<0) fail("Invalid product details");
    exec("UPDATE products SET sku=?,name=?,category=?,unit=?,cost_cents=?,price_cents=?,min_stock=? WHERE id=?",{sku.trimmed(),name.trimmed(),cat.trimmed(),unit.trimmed(),cost,price,min,id});
}
void Database::archiveProduct(int id){requireActive("products",id);exec("UPDATE products SET active=0 WHERE id=?",{id});}
int Database::createWarehouse(const QString &name,const QString &code) {if(name.trimmed().isEmpty()||code.trimmed().isEmpty())fail("Warehouse name and code required");return insert("INSERT INTO warehouses(name,code) VALUES(?,?)",{name.trimmed(),code.trimmed()});}
void Database::editWarehouse(int id,const QString &name,const QString &code){requireActive("warehouses",id);if(name.trimmed().isEmpty()||code.trimmed().isEmpty())fail("Warehouse name and code required");exec("UPDATE warehouses SET name=?,code=? WHERE id=?",{name.trimmed(),code.trimmed(),id});}
void Database::archiveWarehouse(int id){requireActive("warehouses",id);if(scalar("SELECT COALESCE(SUM(qty),0) FROM stock WHERE warehouse_id=?",{id}).toDouble()>0.000001)fail("Transfer out all remaining stock before archiving");exec("UPDATE warehouses SET active=0 WHERE id=?",{id});}
double Database::stockAt(int w,int p){return scalar("SELECT COALESCE((SELECT qty FROM stock WHERE warehouse_id=? AND product_id=?),0)",{w,p}).toDouble();}
void Database::changeStock(int w,int p,double delta){
    if(stockAt(w,p)+delta < -0.000001)fail("Insufficient stock for product #"+QString::number(p));
    exec("INSERT OR IGNORE INTO stock(warehouse_id,product_id,qty) VALUES(?,?,0)",{w,p});
    exec("UPDATE stock SET qty=ROUND(qty+?,6) WHERE warehouse_id=? AND product_id=?",{delta,w,p});
}
void Database::stockEvent(const QString &kind,int doc,int p,int w,double d,qint64 cost,const QString &note){exec("INSERT INTO stock_events(occurred_at,kind,document_id,product_id,warehouse_id,delta,cost_cents,note) VALUES(?,?,?,?,?,?,?,?)",{now(),kind,doc,p,w,d,cost,note});}
QMap<int,double> Database::normalize(const QVector<StockLine>&lines,bool negative){
    QMap<int,double> result;for(const auto &l:lines){requireActive("products",l.productId);if(!std::isfinite(l.quantity)||std::abs(l.quantity)<0.000001||(!negative&&l.quantity<0))fail("Invalid quantity");result[l.productId]+=l.quantity;}
    if(result.isEmpty())fail("Add at least one product");for(double v:result)if(std::abs(v)<0.000001||(!negative&&v<0))fail("Invalid combined quantities");return result;
}
int Database::postTransfer(int from,int to,const QVector<StockLine>&lines,const QString &note){
    if(from==to)fail("Source and destination must differ");requireActive("warehouses",from);requireActive("warehouses",to);auto items=normalize(lines,false);
    begin();try{
        for(auto i=items.cbegin();i!=items.cend();++i)if(stockAt(from,i.key())+0.000001<i.value())fail("Not enough stock: product #"+QString::number(i.key()));
        int id=insert("INSERT INTO stock_transfers(reference,occurred_at,from_warehouse,to_warehouse,note) VALUES(?,?,?,?,?)",{"TR-"+QUuid::createUuid().toString(QUuid::WithoutBraces).left(12),now(),from,to,note});
        for(auto i=items.cbegin();i!=items.cend();++i){int p=i.key();double n=i.value();auto cost=unitCost(p);exec("INSERT INTO stock_transfer_lines(transfer_id,product_id,qty,cost_cents) VALUES(?,?,?,?)",{id,p,n,cost});changeStock(from,p,-n);changeStock(to,p,n);stockEvent("TRANSFER_OUT",id,p,from,-n,cost,note);stockEvent("TRANSFER_IN",id,p,to,n,cost,note);}
        commit();return id;
    }catch(...){rollback();throw;}
}
void Database::reverseTransfer(int id){begin();try{
    auto h=query("SELECT from_warehouse,to_warehouse,status FROM stock_transfers WHERE id=?",{id});if(!h.next()||h.value(2).toString()!="POSTED")fail("Transfer cannot be reversed");int from=h.value(0).toInt(),to=h.value(1).toInt();
    auto l=query("SELECT product_id,qty,cost_cents FROM stock_transfer_lines WHERE transfer_id=?",{id});struct Item{int p;double q;qint64 c;};QVector<Item> items;
    while(l.next()){Item x{l.value(0).toInt(),l.value(1).toDouble(),l.value(2).toLongLong()};if(stockAt(to,x.p)+0.000001<x.q)fail("Cannot reverse: destination stock has been used");items.push_back(x);}
    for(const auto &x:items){changeStock(to,x.p,-x.q);changeStock(from,x.p,x.q);stockEvent("TRANSFER_REVERSAL",id,x.p,to,-x.q,x.c,"");stockEvent("TRANSFER_REVERSAL",id,x.p,from,x.q,x.c,"");}
    exec("UPDATE stock_transfers SET status='REVERSED' WHERE id=?",{id});commit();
}catch(...){rollback();throw;}}
int Database::postAdjustment(int w,const QString &reason,const QVector<StockLine>&lines,const QString &note){requireActive("warehouses",w);if(reason.trimmed().isEmpty())fail("Specify adjustment reason");auto items=normalize(lines,true);begin();try{
    int id=insert("INSERT INTO stock_adjustments(reference,occurred_at,warehouse_id,reason,note) VALUES(?,?,?,?,?)",{"AD-"+QUuid::createUuid().toString(QUuid::WithoutBraces).left(12),now(),w,reason,note});
    for(auto i=items.cbegin();i!=items.cend();++i){int p=i.key();double d=i.value();auto c=unitCost(p);changeStock(w,p,d);exec("INSERT INTO stock_adjustment_lines(adjustment_id,product_id,delta,cost_cents) VALUES(?,?,?,?)",{id,p,d,c});stockEvent("ADJUSTMENT",id,p,w,d,c,reason+" "+note);}
    commit();return id;
}catch(...){rollback();throw;}}
void Database::reverseAdjustment(int id){begin();try{auto h=query("SELECT warehouse_id,status FROM stock_adjustments WHERE id=?",{id});if(!h.next()||h.value(1).toString()!="POSTED")fail("Adjustment cannot be reversed");int w=h.value(0).toInt();auto l=query("SELECT product_id,delta,cost_cents FROM stock_adjustment_lines WHERE adjustment_id=?",{id});struct Item{int p;double q;qint64 c;};QVector<Item> items;while(l.next())items.push_back({l.value(0).toInt(),l.value(1).toDouble(),l.value(2).toLongLong()});for(const auto &x:items){changeStock(w,x.p,-x.q);stockEvent("ADJUSTMENT_REVERSAL",id,x.p,w,-x.q,x.c,"");}exec("UPDATE stock_adjustments SET status='REVERSED' WHERE id=?",{id});commit();}catch(...){rollback();throw;}}
int Database::createCategory(const QString &name){if(name.trimmed().isEmpty())fail("Category name required");return insert("INSERT INTO expense_categories(name) VALUES(?)",{name.trimmed()});}
int Database::createAccount(const QString &name,const QString &kind,qint64 opening){if(name.trimmed().isEmpty()||opening<0)fail("Invalid account");begin();try{int id=insert("INSERT INTO payment_accounts(name,kind) VALUES(?,?)",{name.trimmed(),kind});if(opening)movement(id,opening,"OPENING",id,"Opening balance");commit();return id;}catch(...){rollback();throw;}}
void Database::editAccount(int id,const QString &name,const QString &kind){requireActive("payment_accounts",id);if(name.trimmed().isEmpty())fail("Name required");exec("UPDATE payment_accounts SET name=?,kind=? WHERE id=?",{name.trimmed(),kind,id});}
void Database::archiveAccount(int id){requireActive("payment_accounts",id);if(balance(id)!=0)fail("Account must have zero balance before archiving");exec("UPDATE payment_accounts SET active=0 WHERE id=?",{id});}
qint64 Database::balance(int id){return scalar("SELECT COALESCE(SUM(amount_cents),0) FROM account_movements WHERE account_id=?",{id}).toLongLong();}
void Database::movement(int id,qint64 amount,const QString &kind,int doc,const QString &description){exec("INSERT INTO account_movements(occurred_at,account_id,amount_cents,kind,document_id,description) VALUES(?,?,?,?,?,?)",{now(),id,amount,kind,doc,description});}
void Database::depositOrWithdraw(int id,qint64 amount,const QString &description){requireActive("payment_accounts",id);if(!amount||description.trimmed().isEmpty())fail("Valid amount and description required");begin();try{if(balance(id)+amount<0)fail("Insufficient account funds");movement(id,amount,amount>0?"DEPOSIT":"WITHDRAWAL",0,description);commit();}catch(...){rollback();throw;}}
int Database::transferFunds(int from,int to,qint64 amount,const QString &note){requireActive("payment_accounts",from);requireActive("payment_accounts",to);if(from==to||amount<=0)fail("Select different accounts and positive amount");begin();try{if(balance(from)<amount)fail("Insufficient account funds");int id=insert("INSERT INTO account_transfers(occurred_at,from_account,to_account,amount_cents,note) VALUES(?,?,?,?,?)",{now(),from,to,amount,note});movement(from,-amount,"FUNDS_TRANSFER",id,note);movement(to,amount,"FUNDS_TRANSFER",id,note);commit();return id;}catch(...){rollback();throw;}}
void Database::reverseFundsTransfer(int id){begin();try{auto q=query("SELECT from_account,to_account,amount_cents,status FROM account_transfers WHERE id=?",{id});if(!q.next()||q.value(3).toString()!="POSTED")fail("Funds transfer cannot be reversed");int from=q.value(0).toInt(),to=q.value(1).toInt();qint64 n=q.value(2).toLongLong();if(balance(to)<n)fail("Destination account lacks funds for reversal");movement(to,-n,"FUNDS_REVERSAL",id,"");movement(from,n,"FUNDS_REVERSAL",id,"");exec("UPDATE account_transfers SET status='REVERSED' WHERE id=?",{id});commit();}catch(...){rollback();throw;}}
int Database::addExpense(const QDate &date,int cat,int account,const QString &desc,qint64 cents,const QString &note){requireActive("expense_categories",cat);requireActive("payment_accounts",account);if(!date.isValid()||desc.trimmed().isEmpty()||cents<=0)fail("Complete all required expense fields");begin();try{if(balance(account)<cents)fail("Insufficient account funds");int id=insert("INSERT INTO expenses(expense_date,category_id,account_id,description,amount_cents,note) VALUES(?,?,?,?,?,?)",{date.toString(Qt::ISODate),cat,account,desc.trimmed(),cents,note});movement(account,-cents,"EXPENSE",id,desc);commit();return id;}catch(...){rollback();throw;}}
void Database::editExpense(int id,const QDate &date,int cat,int account,const QString &desc,qint64 cents,const QString &note){requireActive("expense_categories",cat);requireActive("payment_accounts",account);if(!date.isValid()||desc.trimmed().isEmpty()||cents<=0)fail("Invalid expense");begin();try{auto q=query("SELECT account_id,amount_cents,status FROM expenses WHERE id=?",{id});if(!q.next()||q.value(2).toString()!="POSTED")fail("Expense cannot be edited");int oldAccount=q.value(0).toInt();qint64 oldAmount=q.value(1).toLongLong();movement(oldAccount,oldAmount,"EXPENSE_EDIT_REVERSAL",id,"Old expense reversal");if(balance(account)<cents)fail("Insufficient account funds");movement(account,-cents,"EXPENSE_EDIT",id,desc);exec("UPDATE expenses SET expense_date=?,category_id=?,account_id=?,description=?,amount_cents=?,note=?,updated_at=CURRENT_TIMESTAMP WHERE id=?",{date.toString(Qt::ISODate),cat,account,desc.trimmed(),cents,note,id});commit();}catch(...){rollback();throw;}}
void Database::voidExpense(int id){begin();try{auto q=query("SELECT account_id,amount_cents,status FROM expenses WHERE id=?",{id});if(!q.next()||q.value(2).toString()!="POSTED")fail("Expense already voided or missing");movement(q.value(0).toInt(),q.value(1).toLongLong(),"EXPENSE_VOID",id,"Expense void");exec("UPDATE expenses SET status='VOIDED',updated_at=CURRENT_TIMESTAMP WHERE id=?",{id});commit();}catch(...){rollback();throw;}}
void Database::loadDemoData(){if(scalar("SELECT COUNT(*) FROM products").toInt())fail("Demo data can only be loaded into an empty product catalog");int a=createProduct("DEMO-001","Notebook","Stationery","pcs",6500,10000,5);int b=createProduct("DEMO-002","USB Cable","Electronics","pcs",12000,20000,3);postAdjustment(1,"Opening stock",{{a,25},{b,12}},"Demo records");if(scalar("SELECT COUNT(*) FROM payment_accounts").toInt()==0){createAccount("Cash Register","Cash",2000000);createAccount("Bank Account","Bank",10000000);}}
