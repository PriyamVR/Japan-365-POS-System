# Japan 365 POS System — Inventory, Finance & Reporting (C++ / Qt 6 / SQLite)

Japan 365 POS System is a C++ desktop application developed using Qt 6 and SQLite as an academic project. It focuses on five assigned modules: Stock Transfers, Stock Adjustment, Expenses, Payment Accounts, and Reports.

The application has been successfully built and launched on Windows using Qt Creator, Qt 6.11.2, and MinGW 64-bit.

The GitHub repository includes the complete source code, database schema, and build instructions. Users can build and run the application using Qt Creator.

This is an independently developed academic project and is not the official Japan.365 software.

## Project identity

**Project title:** Japan 365 POS System  
**Qt Creator/CMake target:** `Japan365POS`  
**Executable after building:** `Japan365POS.exe`  
**Window title:** Japan 365 POS

This is an independently developed application named for the requested project; it is not affiliated with or endorsed by any commercial Japan.365 product owner.

## Features

- **Products:** create, view, search, edit, archive; SKU, category, unit, purchase and sale price, reorder level.
- **Warehouses:** create, edit, archive, search; prevents archiving a warehouse with stock.
- **Stock transfers:** post transfers between distinct warehouses; check available quantities; reverse while destination stock is sufficient; stock event audit history.
- **Stock adjustments:** add or remove stock (opening balances, damage, counts, returns); prevent negative stock; reverse adjustments.
- **Expenses:** categories, date, account, amount, descriptions; edit and void with compensating ledger entries.
- **Payment accounts:** cash, bank, mobile banking; create/edit/archive, opening balances, deposits, withdrawals, account transfers, reverse funds transfers.
- **Reports:** stock by warehouse, inventory valuation, low-stock alerts, stock movement audit, transfers, adjustments, expenses by category, detailed expenses, account balances, account ledger, funds transfers, monthly expense totals. Date filters where applicable; CSV export.
- SQLite is stored automatically in your user application data folder. **Transactions use database transactions.** Currency uses integer hundredths to avoid floating-point rounding of account balances.

### Limitations

This is a single-user **desktop starter application**, not a commercially audited accounting system. There is no login, tax compliance module, multi-user network sync, barcode hardware configuration, automated backup, or sales checkout screen. Forms currently post **one product line per stock transfer or adjustment**, although the database layer supports multi-line transactions. `Reports > Funds transfers` displays transfer IDs so you can use **Payment Accounts > Reverse funds**. Data is stored locally and is **not encrypted**. Test with demo records before entering real business data. Back up the `.sqlite` file regularly after exiting the program.

## Build and run on Windows

1. Install **Qt 6.11.2 MinGW 64-bit** (or a compatible Qt 6 MinGW kit), its matching **MinGW compiler**, **Qt Creator**, **CMake**, and **Ninja**. The Qt SQLite driver is required.
2. Open the project folder in **Qt Creator** (open `CMakeLists.txt`), select your installed Qt 6 MinGW 64-bit kit, **Configure**, and **Build**. Run the `Japan365POS` target from Qt Creator.
3. For an independently runnable release folder, open a command prompt with the selected Qt kit's `bin` folder and matching MinGW/CMake tools in `PATH`, then run `BUILD_WINDOWS.bat`. The script builds the executable and uses `windeployqt` to copy Qt DLLs and plugins, including the SQLite driver, to `release/`.
4. Run `release/Japan365POS.exe`. **Do not open an HTML file or browser.** On another PC, copy the entire deployed `release/` folder, not only the `.exe`.

If CMake cannot find Qt, set `CMAKE_PREFIX_PATH` to your Qt kit, e.g. `-DCMAKE_PREFIX_PATH=C:\Qt\6.11.2\mingw_64` when configuring. Use the version actually installed.

## First use

- Open the app and create a **payment account** with an opening balance.
- Create or edit products and use the two pre-created warehouses.
- Record **Stock Adjustment > Opening stock**, then post warehouse transfers.
- Add expenses and view the dashboard and reports; generate date-filtered CSV reports.
- **Sample data:** optional button on dashboard, only for an empty product catalog.

## Files

- `src/main.cpp`: Qt Widgets desktop frontend, forms, tables, search, reporting and CSV.
- `src/database.h`, `src/database.cpp`: SQLite-backed business logic, validations, transactions and reversible audit entries.
- `sql/schema.sql`: all persistent tables, constraints, and indexes, embedded as a Qt resource.
- `BUILD_WINDOWS.bat`: build and package on Windows.
- `tests/test_schema.py`: Python standard-library SQLite schema and ledger smoke test (no Qt needed).

## Project layout

```text
Japan_365_POS_CPP_Qt6/
  CMakeLists.txt
  BUILD_WINDOWS.bat
  README.md
  src/
    main.cpp
    database.cpp
    database.h
  sql/
    schema.sql
  tests/
    test_schema.py
```

## Data safety

The database file path is shown in the app's status bar, typically under `%LOCALAPPDATA%/Japan365POS/Japan 365 POS/japan365pos.sqlite` or your Qt platform's `AppDataLocation`. For backups, close the application first and copy the SQLite file to another location. Do not edit database tables while the app is running.

**Existing data:** If a previous RetailDesk build has an existing database at the documented old Windows AppData location, the new project attempts a one-time copy to the Japan 365 POS data directory without deleting the old file. Make a backup before first launch.

## GitHub submission

See [`GITHUB_SETUP_BN.md`](GITHUB_SETUP_BN.md) for a Bangla GitHub Desktop upload and two-contributor workflow. Keep `.gitignore`. Add your module screenshots to [`screenshots/`](screenshots/) before submitting.
