@echo off
echo Testing Qt6 compilation...

REM Check Qt environment
if not defined QT_DIR (
    echo QT_DIR not set!
    exit /b 1
)

echo Using Qt from: %QT_DIR%

REM Create a simple test
mkdir qt_test 2>nul
cd qt_test

REM Create a minimal Qt CMakeLists.txt
echo cmake_minimum_required(VERSION 3.16) > CMakeLists.txt
echo project(qt_test) >> CMakeLists.txt
echo find_package(Qt6 REQUIRED COMPONENTS Core Widgets) >> CMakeLists.txt
echo set(CMAKE_AUTOMOC ON) >> CMakeLists.txt
echo add_executable(qt_test main.cpp) >> CMakeLists.txt
echo target_link_libraries(qt_test Qt6::Core Qt6::Widgets) >> CMakeLists.txt

REM Create a minimal Qt main.cpp
echo #include ^<QApplication^> > main.cpp
echo #include ^<QLabel^> >> main.cpp
echo int main(int argc, char **argv) { >> main.cpp
echo     QApplication app(argc, argv); >> main.cpp
echo     QLabel label("Qt6 Test"); >> main.cpp
echo     label.show(); >> main.cpp
echo     return 0; >> main.cpp
echo } >> main.cpp

REM Try to configure and build
echo Configuring Qt test...
cmake . -DCMAKE_PREFIX_PATH="%QT_DIR%" -G "Visual Studio 17 2022" -A x64 > config.log 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo Qt test configuration failed!
    type config.log
    cd ..
    exit /b 1
)

echo Building Qt test...
cmake --build . --config Release > build.log 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo Qt test build failed!
    type build.log
    cd ..
    exit /b 1
)

echo Qt test successful!
if exist "Release\qt_test.exe" (
    echo ✓ Qt test executable created successfully
) else (
    echo ✗ Qt test executable not found
)

cd ..
rmdir /s /q qt_test 2>nul
exit /b 0
