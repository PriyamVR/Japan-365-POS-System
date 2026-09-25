# Japan 365 POS Qt 6 — Compiler Fix

Fixed the five C++ `auto` declaration errors in `src/main.cpp` at the original lines 79, 80, 81, 82, and 85. The original code mixed `auto x = pointer` and `*y = pointer` in one declaration; C++ deduces conflicting underlying types. Each pointer is now declared separately.

## If you already opened the old project in Qt Creator
1. Close the old project in Qt Creator or use File > Close Project.
2. Extract this fixed archive into a **new directory** (do not mix old and new files).
3. In Qt Creator choose File > Open File or Project and select the new `Japan_365_POS_CPP_Qt6/CMakeLists.txt`.
4. Choose Desktop Qt 6.11.2 MinGW 64-bit, Debug, then Configure Project.
5. Press Ctrl+B to build, then click the green Run triangle.

Alternatively, replace just your original `src/main.cpp` with the fixed copy and press Ctrl+B.

**Build status:** These five compiler errors have been corrected in the source. The archive's SQLite schema tests are executed in this environment. A full Windows Qt/MinGW build must be performed on your PC; it has not been verified in this Linux environment.
