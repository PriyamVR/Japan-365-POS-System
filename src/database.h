#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QPair>
#include <QDate>
#include <QMap>

struct StockLine {
    int productId = 0;
    double quantity = 0; // signed for adjustments; always positive for transfers
};

class Database {
public:
    Database();
    ~Database();
    void initialize();
    QSqlQuery query(const QString &sql, const QVariantList &args = {});
    void exec(const QString &sql, const QVariantList &args = {});
    QVariant scalar(const QString &sql, const QVariantList &args = {});
    QString path() const;
    void begin();
    void commit();
    void rollback();
    int insert(const QString &sql, const QVariantList &args = {});

    // Inventory
    int createProduct(const QString &sku, const QString &name, const QString &category,
                      const QString &unit, qint64 cost, qint64 price, double minimum);
    void editProduct(int id, const QString &sku, const QString &name, const QString &category,
                     const QString &unit, qint64 cost, qint64 price, double minimum);
    void archiveProduct(int id);
    int createWarehouse(const QString &name, const QString &code);
    void editWarehouse(int id, const QString &name, const QString &code);
    void archiveWarehouse(int id);
    double stockAt(int warehouseId, int productId);
    int postTransfer(int source, int dest, const QVector<StockLine> &lines, const QString &note);
    void reverseTransfer(int transferId);
    int postAdjustment(int warehouse, const QString &reason, const QVector<StockLine> &lines,
                       const QString &note);
    void reverseAdjustment(int adjustmentId);

    // Finance
    int createCategory(const QString &name);
    int createAccount(const QString &name, const QString &kind, qint64 openingCents);
    void editAccount(int id, const QString &name, const QString &kind);
    void archiveAccount(int id);
    qint64 balance(int accountId);
    void depositOrWithdraw(int accountId, qint64 signedAmount, const QString &description);
    int transferFunds(int source, int dest, qint64 amount, const QString &note);
    void reverseFundsTransfer(int id);
    int addExpense(const QDate &date, int categoryId, int accountId,
                   const QString &description, qint64 cents, const QString &note);
    void editExpense(int id, const QDate &date, int categoryId, int accountId,
                     const QString &description, qint64 cents, const QString &note);
    void voidExpense(int id);
    void loadDemoData();

private:
    QSqlDatabase db_;
    QString filePath_;
    QString now() const;
    void requireActive(const QString &table, int id);
    qint64 unitCost(int productId);
    void changeStock(int warehouseId, int productId, double delta);
    void stockEvent(const QString &kind, int documentId, int productId, int warehouseId,
                    double delta, qint64 cost, const QString &note);
    void movement(int accountId, qint64 amount, const QString &kind, int docId,
                  const QString &description);
    QMap<int,double> normalize(const QVector<StockLine> &lines, bool allowNegative);
};

QString money(qint64 cents);
QString quantity(double amount);
