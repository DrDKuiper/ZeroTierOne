#!/bin/bash
# Check system requirements for ZeroTier One GUI build

echo "ZeroTier One GUI - System Requirements Check"
echo "==========================================="
echo

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check package (works for most distros)
check_package() {
    if command_exists dpkg; then
        # Debian/Ubuntu
        dpkg -l | grep -q "^ii.*$1" && return 0
    elif command_exists rpm; then
        # Red Hat/CentOS/Fedora
        rpm -q "$1" >/dev/null 2>&1 && return 0
    elif command_exists pacman; then
        # Arch Linux
        pacman -Q "$1" >/dev/null 2>&1 && return 0
    fi
    return 1
}

# Detect platform
echo "🖥️  Platform Detection:"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "✓ Linux detected"
    PLATFORM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "✓ macOS detected"
    PLATFORM="macos"
else
    echo "❌ Unsupported platform: $OSTYPE"
    exit 1
fi
echo

# Check basic requirements
echo "🔧 Basic Requirements:"

# Check CMake
if command_exists cmake; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "✓ CMake found: $CMAKE_VERSION"
else
    echo "❌ CMake not found"
fi

# Check Qt6
if command_exists qmake6 || command_exists qmake; then
    if command_exists qmake6; then
        QT_VERSION=$(qmake6 -query QT_VERSION 2>/dev/null || echo "unknown")
    else
        QT_VERSION=$(qmake -query QT_VERSION 2>/dev/null || echo "unknown")
    fi
    echo "✓ Qt found: $QT_VERSION"
else
    echo "❌ Qt6 not found"
fi

# Check compiler
if command_exists gcc; then
    GCC_VERSION=$(gcc --version | head -n1)
    echo "✓ GCC found: $GCC_VERSION"
elif command_exists clang; then
    CLANG_VERSION=$(clang --version | head -n1)
    echo "✓ Clang found: $CLANG_VERSION"
else
    echo "❌ No suitable compiler found (gcc/clang)"
fi

echo

# Platform-specific checks
if [ "$PLATFORM" = "linux" ]; then
    echo "🐧 Linux-specific Requirements:"
    
    # Check for Qt6 development packages
    if check_package "qt6-base-dev" || check_package "qt6-qtbase-devel" || check_package "qt6-base"; then
        echo "✓ Qt6 development packages found"
    else
        echo "❌ Qt6 development packages not found"
        echo "   Install with:"
        echo "   Ubuntu/Debian: sudo apt install qt6-base-dev qt6-tools-dev cmake"
        echo "   Fedora: sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake"
        echo "   Arch: sudo pacman -S qt6-base qt6-tools cmake"
    fi
    
elif [ "$PLATFORM" = "macos" ]; then
    echo "🍎 macOS-specific Requirements:"
    
    # Check Xcode command line tools
    if xcode-select -p >/dev/null 2>&1; then
        echo "✓ Xcode command line tools installed"
    else
        echo "❌ Xcode command line tools not installed"
        echo "   Install with: xcode-select --install"
    fi
    
    # Check Homebrew
    if command_exists brew; then
        echo "✓ Homebrew found"
        
        # Check if Qt6 is installed via brew
        if brew list qt6 >/dev/null 2>&1; then
            echo "✓ Qt6 installed via Homebrew"
        else
            echo "❌ Qt6 not installed via Homebrew"
            echo "   Install with: brew install qt6 cmake"
        fi
    else
        echo "❌ Homebrew not found"
        echo "   Install from: https://brew.sh"
    fi
fi

echo

# Check ZeroTier service
echo "🌐 ZeroTier Service:"
if command_exists zerotier-cli; then
    ZT_STATUS=$(zerotier-cli info 2>/dev/null | cut -d' ' -f3 || echo "unknown")
    echo "✓ ZeroTier CLI found, status: $ZT_STATUS"
else
    echo "❌ ZeroTier CLI not found"
    echo "   Install ZeroTier One service first"
fi

echo

# Summary
echo "📋 Build Readiness Summary:"
READY=true

if ! command_exists cmake; then
    echo "❌ Missing: CMake"
    READY=false
fi

if ! (command_exists qmake6 || command_exists qmake); then
    echo "❌ Missing: Qt6"
    READY=false
fi

if ! (command_exists gcc || command_exists clang); then
    echo "❌ Missing: C++ Compiler"
    READY=false
fi

if [ "$READY" = true ]; then
    echo "✅ System is ready for GUI build!"
    echo ""
    echo "Next steps:"
    echo "1. Run the build script for your platform:"
    echo "   Linux: ./build-gui-linux.sh"
    echo "   macOS: ./build-gui-macos.sh"
    echo "2. Or use the universal script: ./build-gui.sh"
else
    echo "❌ System is NOT ready for GUI build"
    echo "   Please install the missing dependencies listed above"
fi
