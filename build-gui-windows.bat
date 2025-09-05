@echo off
REM Build script for ZeroTier One GUI on Windows

echo Building ZeroTier One GUI for Windows...

REM Check if Qt6 is installed
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    if defined Qt6_DIR (
        echo Qt6 found via Qt6_DIR environment variable: %Qt6_DIR%
        set PATH=%Qt6_DIR%\bin;%PATH%
    ) else (
        echo Qt6 not found in PATH and Qt6_DIR not set. Please install Qt6 and add it to your PATH.
        exit /b 1
    )
) else (
    echo Qt6 found in PATH
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
if exist "gui\Release\ZeroTier One.exe" (
    echo Copying GUI executable...
    copy "gui\Release\ZeroTier One.exe" "deploy\ZeroTierOneGUI.exe"
    if %ERRORLEVEL% NEQ 0 (
        echo Failed to copy GUI executable!
        exit /b 1
    )
) else (
    echo GUI executable not found at gui\Release\ZeroTier One.exe
    echo Checking alternate locations...
    
    if exist "gui\ZeroTier One.exe" (
        echo Found at gui\ZeroTier One.exe
        copy "gui\ZeroTier One.exe" "deploy\ZeroTierOneGUI.exe"
    ) else if exist "gui\Release\zerotier-gui.exe" (
        echo Found at gui\Release\zerotier-gui.exe
        copy "gui\Release\zerotier-gui.exe" "deploy\ZeroTierOneGUI.exe"
    ) else if exist "gui\zerotier-gui.exe" (
        echo Found at gui\zerotier-gui.exe
        copy "gui\zerotier-gui.exe" "deploy\ZeroTierOneGUI.exe"
    ) else (
        echo No GUI executable found! Build may have failed.
        exit /b 1
    )
)

REM Deploy Qt dependencies to create standalone package
cd deploy
windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw "ZeroTierOneGUI.exe"

if %ERRORLEVEL% NEQ 0 (
    echo Warning: windeployqt failed. Executable may require Qt runtime installed.
    echo You can still use the executable, but Qt6 must be installed on target systems.
) else (
    echo Successfully created standalone executable with all dependencies!
)

REM Check for NSIS (Nullsoft Scriptable Install System)
where makensis >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo NSIS not found in PATH. Skipping installer creation.
) else (
    echo Creating installer with NSIS...
    REM Use a proper quoted path to NSIS
    set "NSIS_PATH=%ProgramFiles(x86)%\NSIS"
    if exist "%NSIS_PATH%\makensis.exe" (
        echo Using NSIS from: %NSIS_PATH%
        "%NSIS_PATH%\makensis.exe" ..\..\windows\ZeroTierOne.nsi
    ) else (
        echo NSIS makensis.exe not found at %NSIS_PATH%
        echo Skipping installer creation.
    )
)

cd ..

echo Build completed successfully!
echo Standalone executable: build\deploy\ZeroTierOneGUI.exe
echo All Qt dependencies included in: build\deploy\

cd ..
