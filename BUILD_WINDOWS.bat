@echo off
setlocal
cd /d "%~dp0"
echo Building Japan 365 POS (C++ / Qt 6)...
echo Run this from the Qt 6 MinGW 64-bit environment or a matching Qt Creator terminal.
where cmake >nul 2>nul || (echo ERROR: Install CMake and add it to PATH.& pause & exit /b 1)
where windeployqt >nul 2>nul || (echo ERROR: Qt 6 bin folder must be in PATH. Use your installed Qt 6 MinGW 64-bit kit.& pause & exit /b 1)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release || (pause & exit /b 1)
cmake --build build --config Release || (pause & exit /b 1)
if exist build\Release\Japan365POS.exe (set "BIN=build\Release\Japan365POS.exe") else (set "BIN=build\Japan365POS.exe")
if not exist "%BIN%" (echo ERROR: EXE not found.& pause & exit /b 1)
if not exist release mkdir release
copy /Y "%BIN%" release\Japan365POS.exe >nul
windeployqt --release --compiler-runtime release\Japan365POS.exe || (echo ERROR: Deployment failed.& pause & exit /b 1)
echo.
echo SUCCESS: release\Japan365POS.exe
start "" "release\Japan365POS.exe"
pause
