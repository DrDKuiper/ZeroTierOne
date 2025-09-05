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

echo Build completed successfully!
echo Executable location: build\gui\Release\ZeroTier One.exe

cd ..
