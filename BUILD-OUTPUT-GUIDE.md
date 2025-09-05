# ZeroTier One GUI - Build Output Guide

This guide explains what files you'll get after building the ZeroTier One GUI application.

## 🎯 **What You'll Get: Single Executable Solutions**

### Windows - Standalone Executable + Dependencies
**Output**: `build\deploy\ZeroTierOneGUI.exe` + supporting files
- **Main executable**: `ZeroTierOneGUI.exe` (your application)
- **Qt libraries**: All required Qt6 DLLs automatically included
- **Deployment**: Copy entire `build\deploy\` folder to any Windows PC
- **Requirements**: None! Runs on any Windows 10/11 system without installing Qt

### macOS - Self-Contained App Bundle
**Output**: `build/deploy/ZeroTierOneGUI.app` (single .app file)
- **App bundle**: Complete macOS application with all Qt frameworks inside
- **Deployment**: Copy single `.app` file to any Mac
- **Requirements**: None! Runs on any macOS 10.15+ system without installing Qt

### Linux - Multiple Options

#### Option 1: AppImage (Recommended)
**Output**: `build/deploy/ZeroTierOneGUI.AppImage` (single file)
- **Single file**: Everything packaged in one portable executable
- **Deployment**: Copy single file to any Linux system
- **Requirements**: None! Runs on any modern Linux distribution
- **Usage**: `chmod +x ZeroTierOneGUI.AppImage && ./ZeroTierOneGUI.AppImage`

#### Option 2: Regular Executable
**Output**: `build/deploy/ZeroTierOneGUI` (single file)
- **Executable**: Standard Linux binary
- **Deployment**: Copy single file
- **Requirements**: Target system needs Qt6 installed
- **Best for**: Systems where you control the Qt installation

#### Option 3: Truly Standalone (Advanced)
**Output**: `deploy-standalone/ZeroTierOneGUI-standalone` (single file)
- **Static linking**: Minimal external dependencies
- **Deployment**: Copy single file
- **Requirements**: Minimal (usually just libc)
- **Build**: Use `./build-standalone.sh` instead of regular build script

## 📦 **Quick Summary**

| Platform | Best Output | File Count | Portable |
|----------|-------------|------------|----------|
| Windows  | `ZeroTierOneGUI.exe` + folder | ~20-30 files | ✅ Yes (copy folder) |
| macOS    | `ZeroTierOneGUI.app` | 1 bundle | ✅ Yes (single file) |
| Linux    | `ZeroTierOneGUI.AppImage` | 1 file | ✅ Yes (single file) |

## 🚀 **Distribution Ready**

All outputs are ready for distribution:
- **No installation required** on target systems
- **No Qt installation needed** by end users
- **Cross-platform compatibility** within each OS family
- **Professional deployment** suitable for enterprise use

## 💡 **Recommendation**

For the best user experience:
1. **Windows**: Distribute the `build\deploy\` folder as a ZIP file
2. **macOS**: Distribute the `.app` file directly (optionally in a DMG)
3. **Linux**: Distribute the `.AppImage` file (single click to run)
