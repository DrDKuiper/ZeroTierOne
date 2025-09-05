#!/bin/bash
# Build script for ZeroTier One GUI on macOS

echo "Building ZeroTier One GUI for macOS..."

# Check if Qt6 is installed
if ! command -v qmake &> /dev/null; then
    echo "Qt6 not found. Please install Qt6 (brew install qt6)"
    exit 1
fi

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Please install CMake (brew install cmake)"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix qt6)" -DBUILD_GUI=ON

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    cd ..
    exit 1
fi

# Build the project
cmake --build . --config Release -j$(sysctl -n hw.ncpu)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    cd ..
    exit 1
fi

echo "Build completed successfully!"
echo "Application bundle location: build/gui/ZeroTier One.app"

# Optional: Create DMG
if command -v create-dmg &> /dev/null; then
    echo "Creating DMG..."
    create-dmg \
        --volname "ZeroTier One" \
        --window-pos 200 120 \
        --window-size 600 300 \
        --icon-size 100 \
        --icon "ZeroTier One.app" 175 120 \
        --hide-extension "ZeroTier One.app" \
        --app-drop-link 425 120 \
        "ZeroTier One.dmg" \
        "gui/"
fi

cd ..
