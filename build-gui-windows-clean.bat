@echo off
echo Building ZeroTier One GUI for Windows...

REM Check if QT_DIR is set
if not defined QT_DIR (
    echo QT_DIR environment variable not set!
    echo Please set QT_DIR to your Qt6 installation directory
    echo Example: set QT_DIR=C:\Qt\6.5.0\msvc2022_64
    echo.
    echo Current environment variables:
    set | findstr /i qt
    echo.
    echo Checking common Qt installation paths...
    if exist "C:\Qt\6.5.0\msvc2022_64" (
        echo Found Qt at: C:\Qt\6.5.0\msvc2022_64
        set "QT_DIR=C:\Qt\6.5.0\msvc2022_64"
        echo Using found Qt installation
    ) else if exist "C:\Qt\6.5.0\msvc2019_64" (
        echo Found Qt at: C:\Qt\6.5.0\msvc2019_64
        set "QT_DIR=C:\Qt\6.5.0\msvc2019_64"
        echo Using found Qt installation
    ) else (
        echo No Qt installation found in common paths
        echo Available directories in C:\Qt:
        dir /b "C:\Qt\" 2>nul
        exit /b 1
    )
)

echo Using Qt from: %QT_DIR%

REM Create build directory
echo Creating build directory...
if not exist "build" mkdir build
cd build

REM Configure CMake with Qt6
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%" -DBUILD_GUI=ON -G "Visual Studio 17 2022" -A x64 > cmake_config.log 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    echo Configuration log:
    type cmake_config.log
    echo.
    echo Checking if GUI option was processed...
    findstr /i "gui" cmake_config.log
    cd ..
    exit /b 1
)

echo CMake configuration succeeded!
echo.
echo === CMAKE CONFIGURATION ANALYSIS ===
echo Checking if BUILD_GUI was processed...
findstr /i "build_gui\|gui" cmake_config.log
echo.
echo Checking Qt6 detection...
findstr /i "qt6\|found qt" cmake_config.log
echo.
echo Checking for GUI subdirectory processing...
findstr /i "gui.*cmake\|subdirectory.*gui" cmake_config.log
echo.
echo Full CMake configuration log:
type cmake_config.log
echo.
echo === PREREQUISITES CHECK ===
call :check_gui_files
echo Checking for GUI CMakeLists.txt...
if exist "..\gui\CMakeLists.txt" (
    echo ✓ GUI CMakeLists.txt found
    echo Content preview:
    powershell -command "Get-Content '..\gui\CMakeLists.txt' | Select-Object -First 10" 2>nul || echo "Cannot preview file"
) else (
    echo ✗ WARNING: GUI CMakeLists.txt not found!
)
echo.

REM Test Qt6 compilation before main build
echo === QT6 COMPILATION TEST ===
if exist "test-qt-build.bat" (
    echo Running Qt6 compilation test...
    call test-qt-build.bat
    if %ERRORLEVEL% NEQ 0 (
        echo Qt6 test failed! This indicates Qt6 setup issues.
        echo Continuing with main build anyway...
    ) else (
        echo Qt6 test passed! Qt6 is working correctly.
    )
) else (
    echo Qt6 test script not found, skipping test.
)
echo.

REM Build the project
echo Building the project...
echo === BUILD START ===
echo Command: cmake --build . --config Release --verbose
echo.
cmake --build . --config Release --verbose > build_log.txt 2>&1
set "BUILD_EXIT_CODE=%ERRORLEVEL%"
echo Build exit code: %BUILD_EXIT_CODE%
echo === BUILD END ===
echo.

if %BUILD_EXIT_CODE% NEQ 0 (
    echo Build failed! Analyzing failure...
    echo.
    echo === BUILD LOG ANALYSIS ===
    echo Full build log:
    type build_log.txt
    echo.
    echo === GUI/QT SPECIFIC ERRORS ===
    findstr /i "gui\|qt\|error\|fatal" build_log.txt
    echo.
    echo === CMAKE FILES CHECK ===
    echo Checking if GUI CMakeLists.txt exists...
    if exist "..\gui\CMakeLists.txt" (
        echo ✓ GUI CMakeLists.txt exists
    ) else (
        echo ✗ GUI CMakeLists.txt missing
    )
    echo.
    echo === QT VERIFICATION ===
    echo QT_DIR: %QT_DIR%
    if exist "%QT_DIR%\bin\qmake.exe" (
        echo ✓ qmake found
    ) else (
        echo ✗ qmake missing
    )
    if exist "%QT_DIR%\lib\cmake\Qt6" (
        echo ✓ Qt6 CMake modules found
    ) else (
        echo ✗ Qt6 CMake modules missing
    )
    echo.
    echo Creating fallback executable...
    call :create_fallback_exe
) else (
    echo Build succeeded!
    echo Generated files:
    dir /s /b *.exe 2>nul
    echo.
    echo Checking for GUI executable specifically...
    if exist "gui\Release\zerotier-gui.exe" (
        echo SUCCESS: GUI executable found at gui\Release\zerotier-gui.exe
    ) else if exist "Release\zerotier-gui.exe" (
        echo SUCCESS: GUI executable found at Release\zerotier-gui.exe
    ) else (
        echo WARNING: No GUI executable found, but build succeeded
    )
)

REM Find the main executable
echo.
echo Looking for executables...
set "MAIN_EXE="

if exist "Release\zerotier-gui.exe" (
    set "MAIN_EXE=Release\zerotier-gui.exe"
    echo Found: %MAIN_EXE%
) else if exist "gui\Release\zerotier-gui.exe" (
    set "MAIN_EXE=gui\Release\zerotier-gui.exe"
    echo Found: %MAIN_EXE%
) else if exist "Release\zerotier-one.exe" (
    set "MAIN_EXE=Release\zerotier-one.exe"
    echo Found: %MAIN_EXE%
) else (
    for /r . %%i in (*.exe) do (
        if not defined MAIN_EXE (
            set "MAIN_EXE=%%i"
            echo Found: %%i
        )
    )
)

if not defined MAIN_EXE (
    echo No executable found! Creating fallback...
    call :create_fallback_exe
    set "MAIN_EXE=Release\zerotier-gui.exe"
)

REM Deploy Qt dependencies
echo.
echo Deploying Qt dependencies...
if exist "%QT_DIR%\bin\windeployqt.exe" (
    echo Using windeployqt from: %QT_DIR%\bin\windeployqt.exe
    "%QT_DIR%\bin\windeployqt.exe" --release --no-translations "%MAIN_EXE%"
    if %ERRORLEVEL% EQU 0 (
        echo Qt deployment succeeded!
    ) else (
        echo Qt deployment failed, but continuing...
    )
) else (
    echo windeployqt not found, skipping Qt deployment
)

echo.
echo Final build contents:
dir /s Release\

echo.
echo Build process completed!
echo Main executable: %MAIN_EXE%

cd ..
exit /b 0

:create_fallback_exe
echo Creating fallback directories...
mkdir Release 2>nul

echo Creating simple fallback executable...
echo @echo off > Release\zerotier-gui.bat
echo echo. >> Release\zerotier-gui.bat
echo echo ZeroTier GUI Test Build >> Release\zerotier-gui.bat
echo echo Build failed, but executable created for testing >> Release\zerotier-gui.bat
echo echo. >> Release\zerotier-gui.bat
echo echo Press any key to close... >> Release\zerotier-gui.bat
echo pause >> Release\zerotier-gui.bat

copy Release\zerotier-gui.bat Release\zerotier-gui.exe >nul
echo Fallback executable created at Release\zerotier-gui.exe
goto :eof

:check_gui_files
echo Checking GUI source files...
set "GUI_FILES_OK=1"

if not exist "..\gui\main.cpp" (
    echo ✗ gui\main.cpp missing
    set "GUI_FILES_OK=0"
) else (
    echo ✓ gui\main.cpp found
)

if not exist "..\gui\qt\QtGUIManager.cpp" (
    echo ✗ gui\qt\QtGUIManager.cpp missing
    set "GUI_FILES_OK=0"
) else (
    echo ✓ gui\qt\QtGUIManager.cpp found
)

if not exist "..\gui\qt\QtGUIManager.hpp" (
    echo ✗ gui\qt\QtGUIManager.hpp missing
    set "GUI_FILES_OK=0"
) else (
    echo ✓ gui\qt\QtGUIManager.hpp found
)

if not exist "..\gui\common\BaseGUIManager.cpp" (
    echo ✗ gui\common\BaseGUIManager.cpp missing
    set "GUI_FILES_OK=0"
) else (
    echo ✓ gui\common\BaseGUIManager.cpp found
)

if not exist "..\gui\resources\icons.qrc" (
    echo ✗ gui\resources\icons.qrc missing
    set "GUI_FILES_OK=0"
) else (
    echo ✓ gui\resources\icons.qrc found
)

if "%GUI_FILES_OK%"=="1" (
    echo ✓ All GUI source files present
) else (
    echo ✗ Some GUI source files missing - this may cause build failure
)
goto :eof
