#!/bin/bash
# Build script for ZeroTier One GUI on macOS

echo "Building ZeroTier One GUI for macOS..."

# Ensure macOS icon exists
mkdir -p gui/resources/macos
if [ ! -f "gui/resources/macos/zerotier.icns" ] && [ -f "artwork/ZeroTierIcon.icns" ]; then
    echo "Copying icon file for macOS..."
    cp artwork/ZeroTierIcon.icns gui/resources/macos/zerotier.icns
fi

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

# Create standalone app bundle using macdeployqt
echo "Creating standalone app bundle..."
mkdir -p deploy

# Copy the app bundle
cp -R "gui/ZeroTier One.app" "deploy/ZeroTierOneGUI.app"

# Deploy Qt frameworks and create standalone bundle
macdeployqt "deploy/ZeroTierOneGUI.app" -always-overwrite

if [ $? -eq 0 ]; then
    echo "Successfully created standalone app bundle!"
    echo "Standalone app: build/deploy/ZeroTierOneGUI.app"
    echo "All Qt frameworks included in the app bundle"
else
    echo "Warning: macdeployqt failed. App may require Qt runtime installed."
    echo "You can still use the app, but Qt6 must be installed on target systems."
    echo "Basic app bundle: build/deploy/ZeroTierOneGUI.app"
fi

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
