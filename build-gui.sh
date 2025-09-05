#!/bin/bash
# Master build script for ZeroTier One GUI - detects platform and builds accordingly

echo "ZeroTier One GUI Build Script"
echo "============================="

# Detect platform
PLATFORM=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

echo "Detected platform: $PLATFORM"

# Run platform-specific build script
case $PLATFORM in
    "linux")
        chmod +x build-gui-linux.sh
        ./build-gui-linux.sh
        ;;
    "macos")
        chmod +x build-gui-macos.sh
        ./build-gui-macos.sh
        ;;
    "windows")
        # On Windows, run the batch file
        cmd //c build-gui-windows.bat
        ;;
    *)
        echo "No build script available for platform: $PLATFORM"
        exit 1
        ;;
esac

echo "Build script completed for $PLATFORM"
