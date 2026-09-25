"""SQLite schema + balance/stock integrity smoke tests (no Qt dependency)."""
from pathlib import Path
import sqlite3

root = Path(__file__).resolve().parents[1]
conn = sqlite3.connect(':memory:')
conn.execute('PRAGMA foreign_keys=ON')
conn.executescript((root / 'sql/schema.sql').read_text())
conn.execute("INSERT INTO warehouses(name,code) VALUES ('Main','M'),('Back','B')")
conn.execute("INSERT INTO products(sku,name,cost_cents) VALUES('T-001','Test item',250)")
conn.execute('INSERT INTO stock(warehouse_id,product_id,qty) VALUES(1,1,10)')
conn.execute('UPDATE stock SET qty=qty-3 WHERE warehouse_id=1 AND product_id=1')
conn.execute('INSERT INTO stock(warehouse_id,product_id,qty) VALUES(2,1,3)')
assert conn.execute('SELECT SUM(qty) FROM stock').fetchone()[0] == 10
conn.execute("INSERT INTO payment_accounts(name,kind) VALUES('Cash','Cash')")
conn.execute("INSERT INTO account_movements(occurred_at,account_id,amount_cents,kind) VALUES('2026-01-01',1,10000,'OPENING')")
conn.execute("INSERT INTO expense_categories(name) VALUES('Rent')")
conn.execute("INSERT INTO expenses(expense_date,category_id,account_id,description,amount_cents) VALUES('2026-01-01',1,1,'Office rent',2500)")
conn.execute("INSERT INTO account_movements(occurred_at,account_id,amount_cents,kind,document_id) VALUES('2026-01-01',1,-2500,'EXPENSE',1)")
assert conn.execute('SELECT SUM(amount_cents) FROM account_movements').fetchone()[0] == 7500
try:
    conn.execute('INSERT INTO stock(warehouse_id,product_id,qty) VALUES(1,999,5)')
    raise AssertionError('FK constraint was not enforced')
except sqlite3.IntegrityError:
    pass
try:
    conn.execute('UPDATE stock SET qty=-10 WHERE warehouse_id=1 AND product_id=1')
    raise AssertionError('Negative stock constraint was not enforced')
except sqlite3.IntegrityError:
    pass
print('PASS: SQL schema, stock conservation, account expense ledger, foreign keys and nonnegative stock')
