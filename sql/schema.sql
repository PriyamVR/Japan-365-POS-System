-- Japan 365 POS v1: monetary amounts stored as INTEGER paise (1/100 BDT).
-- Enable foreign keys on every SQLite connection.
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS warehouses (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL COLLATE NOCASE UNIQUE,
  code TEXT NOT NULL COLLATE NOCASE UNIQUE,
  active INTEGER NOT NULL DEFAULT 1 CHECK (active IN (0,1)),
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS products (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  sku TEXT NOT NULL COLLATE NOCASE UNIQUE,
  name TEXT NOT NULL,
  category TEXT NOT NULL DEFAULT 'General',
  unit TEXT NOT NULL DEFAULT 'pcs',
  cost_cents INTEGER NOT NULL DEFAULT 0 CHECK(cost_cents >= 0),
  price_cents INTEGER NOT NULL DEFAULT 0 CHECK(price_cents >= 0),
  min_stock REAL NOT NULL DEFAULT 0 CHECK(min_stock >= 0),
  active INTEGER NOT NULL DEFAULT 1 CHECK(active IN (0,1)),
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS stock (
  warehouse_id INTEGER NOT NULL REFERENCES warehouses(id) ON DELETE RESTRICT,
  product_id INTEGER NOT NULL REFERENCES products(id) ON DELETE RESTRICT,
  qty REAL NOT NULL DEFAULT 0 CHECK(qty >= -0.000001),
  PRIMARY KEY (warehouse_id, product_id)
);
CREATE TABLE IF NOT EXISTS stock_transfers (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  reference TEXT NOT NULL UNIQUE,
  occurred_at TEXT NOT NULL,
  from_warehouse INTEGER NOT NULL REFERENCES warehouses(id),
  to_warehouse INTEGER NOT NULL REFERENCES warehouses(id),
  note TEXT NOT NULL DEFAULT '',
  status TEXT NOT NULL DEFAULT 'POSTED' CHECK(status IN ('POSTED','REVERSED')),
  CHECK(from_warehouse <> to_warehouse)
);
CREATE TABLE IF NOT EXISTS stock_transfer_lines (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  transfer_id INTEGER NOT NULL REFERENCES stock_transfers(id),
  product_id INTEGER NOT NULL REFERENCES products(id),
  qty REAL NOT NULL CHECK(qty > 0),
  cost_cents INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS stock_adjustments (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  reference TEXT NOT NULL UNIQUE,
  occurred_at TEXT NOT NULL,
  warehouse_id INTEGER NOT NULL REFERENCES warehouses(id),
  reason TEXT NOT NULL,
  note TEXT NOT NULL DEFAULT '',
  status TEXT NOT NULL DEFAULT 'POSTED' CHECK(status IN ('POSTED','REVERSED'))
);
CREATE TABLE IF NOT EXISTS stock_adjustment_lines (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  adjustment_id INTEGER NOT NULL REFERENCES stock_adjustments(id),
  product_id INTEGER NOT NULL REFERENCES products(id),
  delta REAL NOT NULL CHECK(delta != 0),
  cost_cents INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS stock_events (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  occurred_at TEXT NOT NULL,
  kind TEXT NOT NULL,
  document_id INTEGER NOT NULL,
  product_id INTEGER NOT NULL REFERENCES products(id),
  warehouse_id INTEGER NOT NULL REFERENCES warehouses(id),
  delta REAL NOT NULL,
  cost_cents INTEGER NOT NULL DEFAULT 0,
  note TEXT NOT NULL DEFAULT ''
);
CREATE TABLE IF NOT EXISTS expense_categories (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL COLLATE NOCASE UNIQUE,
  active INTEGER NOT NULL DEFAULT 1 CHECK(active IN (0,1))
);
CREATE TABLE IF NOT EXISTS payment_accounts (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL COLLATE NOCASE UNIQUE,
  kind TEXT NOT NULL DEFAULT 'Cash',
  active INTEGER NOT NULL DEFAULT 1 CHECK(active IN (0,1)),
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS account_transfers (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  occurred_at TEXT NOT NULL,
  from_account INTEGER NOT NULL REFERENCES payment_accounts(id),
  to_account INTEGER NOT NULL REFERENCES payment_accounts(id),
  amount_cents INTEGER NOT NULL CHECK(amount_cents > 0),
  note TEXT NOT NULL DEFAULT '',
  status TEXT NOT NULL DEFAULT 'POSTED' CHECK(status IN ('POSTED','REVERSED')),
  CHECK(from_account <> to_account)
);
CREATE TABLE IF NOT EXISTS expenses (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  expense_date TEXT NOT NULL,
  category_id INTEGER NOT NULL REFERENCES expense_categories(id),
  account_id INTEGER NOT NULL REFERENCES payment_accounts(id),
  description TEXT NOT NULL,
  amount_cents INTEGER NOT NULL CHECK(amount_cents > 0),
  note TEXT NOT NULL DEFAULT '',
  status TEXT NOT NULL DEFAULT 'POSTED' CHECK(status IN ('POSTED','VOIDED')),
  created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS account_movements (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  occurred_at TEXT NOT NULL,
  account_id INTEGER NOT NULL REFERENCES payment_accounts(id),
  amount_cents INTEGER NOT NULL CHECK(amount_cents <> 0),
  kind TEXT NOT NULL,
  document_id INTEGER,
  description TEXT NOT NULL DEFAULT ''
);
CREATE INDEX IF NOT EXISTS idx_stock_events_date ON stock_events(occurred_at);
CREATE INDEX IF NOT EXISTS idx_stock_events_product ON stock_events(product_id,warehouse_id);
CREATE INDEX IF NOT EXISTS idx_expenses_date ON expenses(expense_date);
CREATE INDEX IF NOT EXISTS idx_account_movements_date ON account_movements(occurred_at);
CREATE INDEX IF NOT EXISTS idx_account_movements_account ON account_movements(account_id);
PRAGMA user_version = 1;
