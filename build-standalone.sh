#!/bin/bash
# Create a truly standalone ZeroTier One GUI executable
# This script attempts to create a single executable with minimal dependencies

echo "Creating standalone ZeroTier One GUI executable..."

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Please run this script from the ZeroTier One root directory"
    exit 1
fi

# Create build directory
mkdir -p build-standalone
cd build-standalone

# Configure for static linking
echo "Configuring for static build..."

# Try to use static Qt6 if available
QT6_STATIC_PATH=""
if [ -d "/opt/qt6-static" ]; then
    QT6_STATIC_PATH="/opt/qt6-static"
elif [ -d "$HOME/qt6-static" ]; then
    QT6_STATIC_PATH="$HOME/qt6-static"
fi

if [ -n "$QT6_STATIC_PATH" ]; then
    echo "Using static Qt6 from: $QT6_STATIC_PATH"
    cmake .. \
        -DCMAKE_PREFIX_PATH="$QT6_STATIC_PATH" \
        -DBUILD_GUI=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DQT_FEATURE_static_runtime=ON
else
    echo "Static Qt6 not found, using regular build with static linking flags..."
    cmake .. \
        -DBUILD_GUI=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"
fi

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    cd ..
    exit 1
fi

# Build the project
echo "Building..."
cmake --build . --config Release -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    cd ..
    exit 1
fi

# Copy the executable to a deployment directory
mkdir -p ../deploy-standalone
if [ -f "gui/zerotier-gui" ]; then
    cp gui/zerotier-gui ../deploy-standalone/ZeroTierOneGUI-standalone
    echo "Build completed successfully!"
    echo "Standalone executable: deploy-standalone/ZeroTierOneGUI-standalone"
    
    # Check dependencies
    echo ""
    echo "Checking dependencies:"
    ldd ../deploy-standalone/ZeroTierOneGUI-standalone || echo "ldd not available"
    
    # Get file size
    SIZE=$(du -h ../deploy-standalone/ZeroTierOneGUI-standalone | cut -f1)
    echo "File size: $SIZE"
    
    echo ""
    echo "The executable has been created with minimal dependencies."
    echo "Copy 'deploy-standalone/ZeroTierOneGUI-standalone' to any Linux system to run."
else
    echo "Error: GUI executable not found!"
    exit 1
fi

cd ..
