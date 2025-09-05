#!/bin/bash
# Build script for ZeroTier One GUI on Linux

echo "Building ZeroTier One GUI for Linux..."

# Function to check if package is installed (works for most distros)
check_package() {
    if command -v dpkg &> /dev/null; then
        # Debian/Ubuntu
        dpkg -l | grep -q "^ii.*$1" && return 0
    elif command -v rpm &> /dev/null; then
        # Red Hat/CentOS/Fedora
        rpm -q "$1" &> /dev/null && return 0
    elif command -v pacman &> /dev/null; then
        # Arch Linux
        pacman -Q "$1" &> /dev/null && return 0
    fi
    return 1
}

# Check for required packages
echo "Checking dependencies..."

if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo "Qt6 development packages not found."
    echo "Please install Qt6 development packages:"
    echo "  Ubuntu/Debian: sudo apt install qt6-base-dev qt6-tools-dev"
    echo "  Fedora: sudo dnf install qt6-qtbase-devel qt6-qttools-devel"
    echo "  Arch: sudo pacman -S qt6-base qt6-tools"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Please install CMake:"
    echo "  Ubuntu/Debian: sudo apt install cmake"
    echo "  Fedora: sudo dnf install cmake"
    echo "  Arch: sudo pacman -S cmake"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Find Qt6 installation
QT6_PATH=""
if [ -d "/usr/lib/x86_64-linux-gnu/cmake/Qt6" ]; then
    QT6_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt6"
elif [ -d "/usr/lib64/cmake/Qt6" ]; then
    QT6_PATH="/usr/lib64/cmake/Qt6"
elif [ -d "/usr/lib/cmake/Qt6" ]; then
    QT6_PATH="/usr/lib/cmake/Qt6"
fi

# Configure with CMake
if [ -n "$QT6_PATH" ]; then
    cmake .. -DCMAKE_PREFIX_PATH="$QT6_PATH" -DBUILD_GUI=ON -DCMAKE_BUILD_TYPE=Release
else
    cmake .. -DBUILD_GUI=ON -DCMAKE_BUILD_TYPE=Release
fi

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    cd ..
    exit 1
fi

# Build the project
cmake --build . --config Release -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    cd ..
    exit 1
fi

# Create standalone executable
echo "Creating standalone executable..."
mkdir -p deploy

# Copy the executable
cp gui/zerotier-gui deploy/ZeroTierOneGUI

# Try to create an AppImage (portable single file)
if command -v linuxdeploy &> /dev/null || command -v linuxdeploy-x86_64.AppImage &> /dev/null; then
    echo "Creating AppImage (single file executable)..."
    
    # Create AppImage directory structure
    mkdir -p AppDir/usr/bin
    mkdir -p AppDir/usr/share/applications
    mkdir -p AppDir/usr/share/pixmaps
    
    # Copy files
    cp gui/zerotier-gui AppDir/usr/bin/ZeroTierOneGUI
    
    # Create desktop file
    cat > AppDir/usr/share/applications/zerotier-one.desktop << EOF
[Desktop Entry]
Type=Application
Name=ZeroTier One
Comment=ZeroTier One Network Manager
Exec=ZeroTierOneGUI
Icon=zerotier
Categories=Network;
EOF
    
    # Copy icon
    cp ../artwork/ZeroTierIcon.png AppDir/usr/share/pixmaps/zerotier.png
    
    # Create AppImage
    if command -v linuxdeploy &> /dev/null; then
        linuxdeploy --appdir AppDir --output appimage
    else
        linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage
    fi
    
    # Move AppImage to deploy directory
    if [ -f *.AppImage ]; then
        mv *.AppImage deploy/ZeroTierOneGUI.AppImage
        echo "Successfully created standalone AppImage!"
        echo "Single file executable: build/deploy/ZeroTierOneGUI.AppImage"
        echo "This file contains all dependencies and can run on any Linux system"
    else
        echo "AppImage creation failed, but you still have the regular executable"
        echo "Regular executable: build/deploy/ZeroTierOneGUI"
    fi
else
    echo "linuxdeploy not found. Creating basic standalone executable..."
    echo "Regular executable: build/deploy/ZeroTierOneGUI"
    echo "Note: This may require Qt6 to be installed on target systems"
    echo "To create a fully portable AppImage, install linuxdeploy"
fi

cd ..
