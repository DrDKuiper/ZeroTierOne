#!/bin/bash
# Pre-build setup script for ZeroTier One GUI

echo "Setting up ZeroTier One GUI build environment..."

# Create required directories
mkdir -p gui/resources/macos
mkdir -p gui/resources/linux  
mkdir -p gui/resources/windows

# Copy icons if they don't exist
if [ ! -f "gui/resources/macos/zerotier.icns" ] && [ -f "artwork/ZeroTierIcon.icns" ]; then
    echo "Copying macOS icon..."
    cp artwork/ZeroTierIcon.icns gui/resources/macos/zerotier.icns
fi

if [ ! -f "gui/resources/windows/zerotier.ico" ] && [ -f "artwork/ZeroTierIcon.ico" ]; then
    echo "Copying Windows icon..."
    cp artwork/ZeroTierIcon.ico gui/resources/windows/zerotier.ico
fi

if [ ! -f "gui/resources/linux/zerotier.png" ] && [ -f "artwork/ZeroTierIcon.png" ]; then
    echo "Copying Linux icon..."
    cp artwork/ZeroTierIcon.png gui/resources/linux/zerotier.png
fi

echo "GUI build environment setup complete!"
