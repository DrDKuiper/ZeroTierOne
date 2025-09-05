# ZeroTier One - Global Area Networking with GUI

*This comprehensive document covers both the core ZeroTier One functionality and the new graphical user interface (GUI) implementation.*

## Overview

**ZeroTier** is a smart programmable Ethernet switch for planet Earth. It allows all networked devices, VMs, containers, and applications to communicate as if they all reside in the same physical data center or cloud region.

This is accomplished by combining a cryptographically addressed and secure peer to peer network (termed VL1) with an Ethernet emulation layer somewhat similar to VXLAN (termed VL2). Our VL2 Ethernet virtualization layer includes advanced enterprise SDN features like fine grained access control rules for network micro-segmentation and security monitoring.

All ZeroTier traffic is encrypted end-to-end using secret keys that only you control. Most traffic flows peer to peer, though we offer free (but slow) relaying for users who cannot establish peer to peer connections.

**NEW: GUI Interface** - This repository now includes a complete graphical user interface that transforms the ZeroTier One command-line application into a user-friendly desktop application with system tray support for Windows, macOS, and Linux.

## Table of Contents

1. [Getting Started](#getting-started)
2. [GUI Quick Start](#gui-quick-start) 
3. [GUI Features](#gui-features)
4. [Building from Source](#building-from-source)
5. [GUI Building Instructions](#gui-building-instructions)
6. [Installation](#installation)
7. [Usage](#usage)
8. [Troubleshooting](#troubleshooting)
9. [Project Layout](#project-layout)
10. [API & Metrics](#api--metrics)

---

## Getting Started

Everything in the ZeroTier world is controlled by two types of identifier: 40-bit/10-digit *ZeroTier addresses* and 64-bit/16-digit *network IDs*. These identifiers are easily distinguished by their length. A ZeroTier address identifies a node or "device" (laptop, phone, server, VM, app, etc.) while a network ID identifies a virtual Ethernet network that can be joined by devices.

ZeroTier addresses can be thought of as port numbers on an enormous planet-wide enterprise Ethernet smart switch supporting VLANs. Network IDs are VLAN IDs to which these ports may be assigned. A single port can be assigned to more than one VLAN.

A ZeroTier address looks like `8056c2e21c` and a network ID looks like `8056c2e21c000001`. 

Visit [ZeroTier's site](https://www.zerotier.com/) for more information and [pre-built binary packages](https://www.zerotier.com/download/). Apps for Android and iOS are available for free in the Google Play and Apple app stores.

---

## GUI Quick Start

### What is the GUI?

The ZeroTier One GUI transforms the command-line application into a user-friendly graphical interface with system tray support for Windows, macOS, and Linux.

### Quick Build & Run

**Windows:**
```batch
# Run this in PowerShell or Command Prompt
build-gui-windows.bat
```

**macOS:**
```bash
# Run this in Terminal
chmod +x build-gui-macos.sh
./build-gui-macos.sh
```

**Linux:**
```bash
# Run this in Terminal
chmod +x build-gui-linux.sh
./build-gui-linux.sh
```

**Any Platform:**
```bash
# Universal script that detects your platform
chmod +x build-gui.sh
./build-gui.sh
```

### What You Get

✅ **System Tray Icon** - Always accessible from your taskbar/menu bar  
✅ **Network Management** - Join/leave ZeroTier networks with a click  
✅ **Real-time Status** - See your networks, peers, and connection status  
✅ **Cross-Platform** - Works on Windows, macOS, and Linux  
✅ **Auto-Detection** - Automatically finds your ZeroTier service  

---

## GUI Features

### Main Window
- **Node Status**: Displays your ZeroTier node ID, status, and version
- **Networks Table**: Shows all joined networks with status, member count, and traffic
- **Network Controls**: Join new networks or leave existing ones
- **Peers Table**: Displays connected peers with latency and version information

### System Tray
- **Tray Icon**: Shows connection status (green=online, red=offline, yellow=connecting)
- **Context Menu**: Quick access to show/hide window and quit application
- **Notifications**: Shows important status changes and network events

### Advanced Features
- 🔄 **Auto-refresh** every 5 seconds
- 📊 **Network statistics** (traffic, members)
- 👥 **Peer information** (latency, status)
- 🔔 **Desktop notifications** for network events
- 🎨 **Status-aware tray icon** (green=online, red=offline)
- 🖥️ **Hide to tray** when closing window

---

## Building from Source

### Prerequisites for Core ZeroTier

To build on Mac and Linux just type `make`. On FreeBSD and OpenBSD `gmake` (GNU make) is required and can be installed from packages or ports. For Windows there is a Visual Studio solution in `windows/`.

**Platform-specific requirements:**

- **Mac**: Xcode command line tools for macOS 10.13 or newer are required.
- **Linux**: The minimum compiler versions required are GCC/G++ 8.x or CLANG/CLANG++ 5.x.
- **Windows**: Visual Studio 2022 on Windows 10 or newer.
- **FreeBSD**: GNU make is required. Type `gmake` to build. `binutils` is required.
- **OpenBSD**: GNU make is required. Type `gmake` to build.

Typing `make selftest` will build a *zerotier-selftest* binary which unit tests various internals.

---

## GUI Building Instructions

### Prerequisites for GUI

All platforms require:
- **ZeroTier One service** must be installed and running
- **CMake** 3.16 or later
- **Qt6** development libraries
- **C++11** compatible compiler

### Quick Install Prerequisites

**Windows:**
- Install Qt6 from qt.io
- Install Visual Studio Build Tools
- Install CMake

**macOS:**
```bash
brew install qt6 cmake
```

**Ubuntu/Debian:**
```bash
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-tools cmake gcc
```

### Building the GUI

#### Windows
1. Install Qt6 (download from qt.io or use vcpkg)
2. Install Visual Studio 2019 or later with C++ support
3. Install CMake
4. Run the build script:
   ```batch
   build-gui-windows.bat
   ```

The executable will be created at `build\gui\Release\ZeroTier One.exe`

#### macOS
1. Install Xcode command line tools:
   ```bash
   xcode-select --install
   ```
2. Install dependencies via Homebrew:
   ```bash
   brew install qt6 cmake
   ```
3. Run the build script:
   ```bash
   chmod +x build-gui-macos.sh
   ./build-gui-macos.sh
   ```

The application bundle will be created at `build/gui/ZeroTier One.app`

#### Linux
1. Install dependencies (see prerequisites above)
2. Run the build script:
   ```bash
   chmod +x build-gui-linux.sh
   ./build-gui-linux.sh
   ```

The executable will be created at `build/gui/zerotier-gui`

---

## Installation

### Core ZeroTier Installation
On most distributions, macOS, and Windows, the installer will start the service and set it up to start on boot.

### GUI Installation

#### Windows
1. Build the application using the build script
2. The executable can be run directly or installed to `Program Files`
3. For system-wide installation, copy to `C:\Program Files\ZeroTier\One\`

#### macOS
1. Build the application using the build script
2. Copy `ZeroTier One.app` to `/Applications/`
3. Grant necessary permissions in System Preferences > Security & Privacy

#### Linux
1. Build the application using the build script
2. Install system-wide:
   ```bash
   sudo cp build/gui/zerotier-gui /usr/local/bin/
   sudo cp gui/resources/linux/zerotier-one.desktop /usr/share/applications/
   sudo cp artwork/ZeroTierIcon.png /usr/share/pixmaps/zerotier.png
   ```

---

## Usage

### Command Line Usage

Running *zerotier-one* with `-h` option will show help.

On Linux and BSD, if you built from source, you can start the service with:
```bash
sudo ./zerotier-one -d
```

The service is controlled via the JSON API, which by default is available at `127.0.0.1:9993`. We include a *zerotier-cli* command line utility to make API calls for standard things like joining and leaving networks.

### GUI Usage

#### First Run
1. Ensure ZeroTier One service is running
2. Launch the GUI application
3. The application will automatically locate the authentication token
4. If the token cannot be found, check service status and permissions

#### Network Management
1. **Join a Network**: Enter the 16-character network ID and click "Join Network"
2. **Leave a Network**: Select a network in the table and click "Leave Network" 
3. **Refresh Data**: Click "Refresh" to update network and peer information

#### System Tray
- **Double-click**: Show/hide the main window
- **Right-click**: Access context menu with options to show, hide, or quit

### Home Folders

Here's where home folders live (by default) on each OS:

- **Linux**: `/var/lib/zerotier-one`
- **FreeBSD** / **OpenBSD**: `/var/db/zerotier-one`
- **Mac**: `/Library/Application Support/ZeroTier/One`
- **Windows**: `\ProgramData\ZeroTier\One`

### Authentication Token Locations

The GUI automatically connects to the local ZeroTier One service using:
- **API Endpoint**: `http://127.0.0.1:9993`
- **Authentication**: Reads token from standard ZeroTier locations

**Token Locations:**
- **Windows**: `%PROGRAMDATA%\ZeroTier\One\authtoken.secret`
- **macOS**: `/Library/Application Support/ZeroTier/One/authtoken.secret`
- **Linux**: `/var/lib/zerotier-one/authtoken.secret`

---

## Troubleshooting

### Core ZeroTier Troubleshooting

For most users, it just works.

If you are running a local system firewall, we recommend adding rules permitting zerotier. 

**Common Issues:**
- **Firewall**: Check if UDP port 9993 inbound is open for LAN discovery
- **NAT**: Users behind "symmetric" NAT may need "full cone" NAT mode
- **Fallback**: If UDP is blocked, ZeroTier falls back to TCP tunneling over port 443

### GUI Troubleshooting

#### "Failed to initialize ZeroTier One GUI"
- Ensure ZeroTier One service is running
- Check that the authentication token file exists and is readable
- Verify the service is listening on port 9993

#### "Could not find authentication token"
- The GUI cannot locate the ZeroTier authentication token
- Ensure you have proper permissions to read the token file
- Try running as administrator/root or check service status

#### GUI appears but shows no data
- The service might not be responding
- Check if firewall is blocking localhost connections
- Verify ZeroTier One service is properly started

#### Icons not displaying properly
- Ensure Qt6 is properly installed
- Check that the resource files are bundled with the application
- Try rebuilding with proper Qt6 paths

For additional help, see our [knowledge base](https://zerotier.atlassian.net/wiki/spaces/SD/overview) and [documentation site](https://docs.zerotier.com/zerotier/troubleshooting).

---

## Project Layout

The base path contains the ZeroTier One service main entry point (`one.cpp`), self test code, makefiles, etc.

- `artwork/`: icons, logos, etc.
- `attic/`: old stuff and experimental code that we want to keep around for reference.
- `controller/`: the reference network controller implementation
- `debian/`: files for building Debian packages on Linux.
- `doc/`: manual pages and other documentation.
- `ext/`: third party libraries, binaries that we ship for convenience on some platforms
- **`gui/`**: **NEW - Graphical user interface implementation**
  - `main.cpp`: Application entry point
  - `CMakeLists.txt`: Build configuration  
  - `qt/`: Qt6-specific implementation
  - `common/`: Platform-independent interface
  - `resources/`: Platform-specific resources (icons, etc.)
- `include/`: include files for the ZeroTier core.
- `java/`: a JNI wrapper used with our Android mobile app.
- `node/`: the ZeroTier virtual Ethernet switch core
- `osdep/`: code to support and integrate with OSes
- `rule-compiler/`: JavaScript rules language compiler
- `service/`: the ZeroTier One service
- `windows/`: Visual Studio solution files, Windows service code
- `zeroidc/`: OIDC implementation used by ZeroTier service

---

## API & Metrics

### JSON API

The service is controlled via the JSON API, which by default is available at `127.0.0.1:9993`. The *authtoken.secret* file in the home folder contains the secret token for accessing this API. See [service/README.md](service/README.md) for API documentation.

### Prometheus Metrics

Prometheus Metrics are available at the `/metrics` API endpoint. This endpoint is protected by an API key stored in `metricstoken.secret`.

**Access Examples:**
```bash
# Linux
curl -H "X-ZT1-Auth: $(sudo cat /var/lib/zerotier-one/metricstoken.secret)" http://localhost:9993/metrics

# macOS  
curl -H "X-ZT1-Auth: $(sudo cat /Library/Application\ Support/ZeroTier/One/metricstoken.secret)" http://localhost:9993/metrics

# Windows PowerShell (Admin)
Invoke-RestMethod -Headers @{'X-ZT1-Auth' = "$(Get-Content C:\ProgramData\ZeroTier\One\metricstoken.secret)"; } -Uri http://localhost:9993/metrics
```

#### Available Metrics

| Metric Name | Labels | Metric Type | Description |
| ---         | ---    | ---         | ---         |
| zt_packet | packet_type, direction | Counter | ZeroTier packet type counts |
| zt_packet_error | error_type, direction | Counter | ZeroTier packet errors|
| zt_data | protocol, direction | Counter | number of bytes ZeroTier has transmitted or received |
| zt_num_networks | | Gauge | number of networks this instance is joined to |
| zt_network_multicast_groups_subscribed | network_id | Gauge | number of multicast groups networks are subscribed to |
| zt_network_packets | network_id, direction | Counter | number of incoming/outgoing packets per network |
| zt_peer_latency | node_id | Histogram | peer latency (ms) |
| zt_peer_path_count | node_id, status | Gauge | number of paths to peer |
| zt_peer_packets | node_id, direction | Counter | number of packets to/from a peer |
| zt_peer_packet_errors | node_id | Counter | number of incoming packet errors from a peer |

### HTTP / App Server

There is a static http file server suitable for hosting Single Page Apps at http://localhost:9993/app/<app-path>

---

## Contributing

Please do pull requests off of the `dev` branch.

Releases are done by merging `dev` into `main` and then tagging and doing builds.

### Cleanup Old Documentation Files

If you have old documentation files from previous versions, you can run the cleanup script to remove them:

**Linux/macOS:**
```bash
chmod +x cleanup-old-docs.sh
./cleanup-old-docs.sh
```

**Windows:**
```powershell
.\cleanup-old-docs.ps1
```

This will remove the old `QUICK_START.md`, `GUI_README.md`, and `GUI_PROPOSAL.md` files since all their content has been consolidated into this README.

### GUI Development

The GUI is built using the Qt6 framework with the following components:

- **BaseGUIManager**: Abstract base class defining the GUI interface
- **QtGUIManager**: Qt6-specific implementation  
- **MainWindow**: Main application window with network and peer tables
- **API Integration**: Communicates with ZeroTier service via REST API

---

## License

ZeroTier is licensed under the [BSL version 1.1](https://mariadb.com/bsl11/). See [LICENSE.txt](LICENSE.txt) and the [ZeroTier pricing page](https://www.zerotier.com/pricing) for details. ZeroTier is free to use internally in businesses and academic institutions and for non-commercial purposes.

A small amount of third party code is also included in ZeroTier and is not subject to our BSL license. See [AUTHORS.md](AUTHORS.md) for a list of third party code, where it is included, and the licenses that apply to it. All of the third party code in ZeroTier is liberally licensed (MIT, BSD, Apache, public domain, etc.).

---

## Links

- **Website**: [zerotier.com](https://www.zerotier.com)
- **Documentation**: [docs.zerotier.com](https://docs.zerotier.com)
- **Discussion Forum**: [discuss.zerotier.com](https://discuss.zerotier.com)
- **Downloads**: [zerotier.com/download](https://www.zerotier.com/download)

---

*Transform your ZeroTier experience from command-line to point-and-click with the new GUI interface!*
