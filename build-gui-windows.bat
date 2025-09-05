@echo off
REM Build script for ZeroTier One GUI on Windows

echo Building ZeroTier One GUI for Windows...

REM Check if Qt6 is installed
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Qt6 not found in PATH. Please install Qt6 and add it to your PATH.
    exit /b 1
)

REM Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo CMake not found in PATH. Please install CMake and add it to your PATH.
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="%Qt6_DIR%" -DBUILD_GUI=ON

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build the project
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    cd ..
    exit /b 1
)

REM Create standalone executable using windeployqt
echo Creating standalone executable...
if not exist "deploy" mkdir deploy

REM Copy the main executable
copy "gui\Release\ZeroTier One.exe" "deploy\ZeroTierOneGUI.exe"

REM Deploy Qt dependencies to create standalone package
cd deploy
windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw "ZeroTierOneGUI.exe"

if %ERRORLEVEL% NEQ 0 (
    echo Warning: windeployqt failed. Executable may require Qt runtime installed.
    echo You can still use the executable, but Qt6 must be installed on target systems.
) else (
    echo Successfully created standalone executable with all dependencies!
)

cd ..

echo Build completed successfully!
echo Standalone executable: build\deploy\ZeroTierOneGUI.exe
echo All Qt dependencies included in: build\deploy\

cd ..
