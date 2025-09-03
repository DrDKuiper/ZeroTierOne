# ZeroTier Cross-Platform Deployment Guide
# Guia completo para todas as plataformas suportadas

## 🌍 Plataformas Suportadas

### 🖥️ Desktop
- ✅ **Windows** (7, 8, 10, 11)
- ✅ **macOS** (10.12+, Intel & Apple Silicon)  
- ✅ **Linux** (Ubuntu, Debian, CentOS, Fedora, Arch)
- ✅ **FreeBSD** (12+)

### 📱 Mobile
- ✅ **Android** (API 21+, ARM/x86)
- ✅ **iOS** (iOS 12+, iPadOS)
- ✅ **Windows Mobile** (UWP)

### 🏠 NAS & Storage
- ✅ **Synology DSM** (6.0+)
- ✅ **QNAP QTS** (4.3+)
- ✅ **TrueNAS** (Core/Scale)
- ✅ **OpenMediaVault**

### 🌐 Routers & Gateways
- ✅ **OpenWrt** (19.07+)
- ✅ **pfSense** (2.4+)
- ✅ **EdgeOS** (Ubiquiti)
- ✅ **MikroTik RouterOS**

### 🔧 IoT & Embedded
- ✅ **Raspberry Pi** (Raspbian/Ubuntu)
- ✅ **BeagleBone** (Debian)
- ✅ **NVIDIA Jetson** (JetPack)
- ✅ **Docker Containers**

---

## 🚀 Quick Deploy

### One-Line Install (Linux/macOS/FreeBSD)
```bash
curl -fsSL https://install.zerotier.com | sudo bash
```

### Windows PowerShell
```powershell
iwr -useb https://download.zerotier.com/install.ps1 | iex
```

### Docker Universal
```bash
docker run -d --name zerotier-manager \
  --cap-add=NET_ADMIN \
  --device=/dev/net/tun \
  -v /var/lib/zerotier-one:/var/lib/zerotier-one \
  -p 9993:9993 \
  zerotier/zerotier:latest
```

---

## 📱 Mobile Deployment

### Android APK Build
```bash
# Install buildozer
pip install buildozer

# Build APK
cd mobile/
buildozer android debug

# Install on device
adb install bin/zerotier_manager-0.1-debug.apk
```

### iOS App Build
```bash
# Install kivy-ios
pip install kivy-ios

# Build iOS app
cd mobile/
kivy-ios build python3 kivy requests
kivy-ios create ZeroTierManager .

# Open in Xcode
open ios-build/ZeroTierManager.xcodeproj
```

### Progressive Web App (PWA)
```bash
# Deploy web interface
python universal_builder.py --web

# Serve via HTTPS for PWA features
python -m http.server 8080 --bind 0.0.0.0
```

---

## 🏠 NAS Platform Deployment

### Synology DSM

#### Method 1: Docker (Recommended)
```bash
# Download ZeroTier Docker image
docker pull zerotier/zerotier

# Create container
docker run -d \
  --name=zerotier \
  --cap-add=NET_ADMIN \
  --cap-add=SYS_ADMIN \
  --device=/dev/net/tun \
  --net=host \
  --restart=unless-stopped \
  -v /volume1/zerotier:/var/lib/zerotier-one:Z \
  zerotier/zerotier
```

#### Method 2: Native Package
```bash
# Download SPK package
wget https://github.com/zerotier/ZeroTierOne/releases/download/1.x.x/zerotier_synology_x64.spk

# Install via Package Center
# Control Panel > Package Center > Manual Install
```

### QNAP QTS
```bash
# Download QPKG
wget https://github.com/zerotier/ZeroTierOne/releases/download/1.x.x/zerotier_qnap_x64.qpkg

# Install via App Center
# App Center > Install Manually
```

### TrueNAS Scale
```bash
# Create ZeroTier jail/container
iocage create -n zerotier -r 12.2-RELEASE
iocage set allow_tun=1 zerotier
iocage set vnet=on zerotier
iocage start zerotier

# Install inside jail
iocage exec zerotier pkg install zerotier
```

---

## 🌐 Router Platform Deployment

### OpenWrt
```bash
# Update package list
opkg update

# Install ZeroTier
opkg install zerotier

# Enable and start service
service zerotier enable
service zerotier start

# Configure via LuCI web interface
# Network > Services > ZeroTier
```

### pfSense
```bash
# Install via Package Manager
# System > Package Manager > Available Packages
# Search "zerotier" and install

# Configure via Web GUI
# Services > ZeroTier
```

### EdgeOS (Ubiquiti)
```bash
# Download Debian package
curl -O https://download.zerotier.com/debian/bullseye/pool/main/z/zerotier-one/zerotier-one_1.x.x_amd64.deb

# Install
sudo dpkg -i zerotier-one_1.x.x_amd64.deb

# Configure
configure
set service zerotier network YOUR_NETWORK_ID
commit
save
```

---

## 🔧 IoT & Embedded Deployment

### Raspberry Pi
```bash
# Automated install
curl -s https://install.zerotier.com | sudo bash

# Join network
sudo zerotier-cli join NETWORK_ID

# Enable at boot
sudo systemctl enable zerotier-one
```

### Docker on ARM
```bash
# Multi-arch build
docker buildx build --platform linux/arm64,linux/amd64 \
  -t zerotier-manager:latest .

# Deploy on ARM device
docker run -d --name zerotier-arm \
  --privileged \
  --net=host \
  zerotier-manager:latest
```

### Kubernetes Deployment
```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: zerotier
spec:
  selector:
    matchLabels:
      app: zerotier
  template:
    metadata:
      labels:
        app: zerotier
    spec:
      hostNetwork: true
      containers:
      - name: zerotier
        image: zerotier/zerotier:latest
        securityContext:
          privileged: true
        volumeMounts:
        - name: zerotier-data
          mountPath: /var/lib/zerotier-one
        - name: dev-net-tun
          mountPath: /dev/net/tun
      volumes:
      - name: zerotier-data
        hostPath:
          path: /var/lib/zerotier-one
      - name: dev-net-tun
        hostPath:
          path: /dev/net/tun
```

---

## ⚙️ Configuration Management

### Ansible Playbook
```yaml
---
- name: Deploy ZeroTier Universal
  hosts: all
  become: yes
  tasks:
    - name: Install ZeroTier (Debian/Ubuntu)
      apt:
        deb: https://download.zerotier.com/debian/bullseye/pool/main/z/zerotier-one/zerotier-one_1.x.x_amd64.deb
      when: ansible_os_family == "Debian"
    
    - name: Install ZeroTier (RedHat/CentOS)
      yum:
        name: https://download.zerotier.com/redhat/el/7/x86_64/zerotier-one-1.x.x.x86_64.rpm
      when: ansible_os_family == "RedHat"
    
    - name: Join ZeroTier network
      command: zerotier-cli join {{ zerotier_network_id }}
      
    - name: Enable ZeroTier service
      systemd:
        name: zerotier-one
        enabled: yes
        state: started
```

### Terraform Configuration
```hcl
# Deploy ZeroTier on cloud instances
resource "aws_instance" "zerotier_node" {
  ami           = "ami-0c55b159cbfafe1d0"
  instance_type = "t3.micro"
  key_name      = "my-key"
  
  user_data = <<-EOF
    #!/bin/bash
    curl -s https://install.zerotier.com | sudo bash
    sudo zerotier-cli join ${var.zerotier_network_id}
  EOF
  
  tags = {
    Name = "ZeroTier Node"
  }
}

variable "zerotier_network_id" {
  description = "ZeroTier network ID"
  type        = string
}
```

---

## 🔒 Security Configurations

### Firewall Rules (iptables)
```bash
# Allow ZeroTier traffic
iptables -A INPUT -p udp --dport 9993 -j ACCEPT
iptables -A OUTPUT -p udp --sport 9993 -j ACCEPT

# Allow ZeroTier interface
iptables -A INPUT -i zt+ -j ACCEPT
iptables -A OUTPUT -o zt+ -j ACCEPT
```

### SELinux Policy (Red Hat/CentOS)
```bash
# Allow ZeroTier through SELinux
setsebool -P zerotier_can_network_connect 1
semanage fcontext -a -t bin_t "/usr/sbin/zerotier-one"
restorecon -v /usr/sbin/zerotier-one
```

### Windows Firewall
```powershell
# Allow ZeroTier through Windows Firewall
New-NetFirewallRule -DisplayName "ZeroTier" -Direction Inbound -Protocol UDP -LocalPort 9993
New-NetFirewallRule -DisplayName "ZeroTier" -Direction Outbound -Protocol UDP -LocalPort 9993
```

---

## 📊 Monitoring & Management

### Systemd Service Monitoring
```bash
# Check status
systemctl status zerotier-one

# View logs
journalctl -u zerotier-one -f

# Restart service
systemctl restart zerotier-one
```

### Docker Health Checks
```dockerfile
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
  CMD zerotier-cli info || exit 1
```

### Prometheus Metrics
```yaml
# prometheus.yml
scrape_configs:
  - job_name: 'zerotier'
    static_configs:
      - targets: ['localhost:9993']
    metrics_path: '/metrics'
    headers:
      X-ZT1-Auth: ['YOUR_AUTH_TOKEN']
```

---

## 🛠️ Build from Source

### Universal Build Script
```bash
#!/bin/bash
# Build ZeroTier for current platform

# Detect platform
OS=$(uname -s)
ARCH=$(uname -m)

echo "Building ZeroTier for $OS-$ARCH"

# Install dependencies
case $OS in
  "Linux")
    sudo apt-get update
    sudo apt-get install -y build-essential cmake git
    ;;
  "Darwin")
    xcode-select --install
    brew install cmake
    ;;
  "FreeBSD")
    pkg install -y cmake git
    ;;
esac

# Clone and build
git clone https://github.com/zerotier/ZeroTierOne.git
cd ZeroTierOne
make

# Install
sudo make install
sudo systemctl enable zerotier-one
sudo systemctl start zerotier-one
```

### Cross-Compilation
```bash
# ARM64 build on x86_64
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export AR=aarch64-linux-gnu-ar
export STRIP=aarch64-linux-gnu-strip

make ZT_TARGET_ARCH=arm64
```

---

## 📱 Mobile App Stores

### Google Play Store (Android)
1. Build APK with: `buildozer android release`
2. Sign APK with your keystore
3. Upload to Google Play Console
4. Follow Play Store guidelines

### Apple App Store (iOS)
1. Build with Xcode
2. Sign with Apple Developer Certificate
3. Submit via App Store Connect
4. Follow App Store Review Guidelines

### F-Droid (Open Source Android)
1. Submit app metadata to F-Droid
2. Ensure reproducible builds
3. Follow F-Droid inclusion criteria

---

## 🚀 Deployment Automation

### GitHub Actions CI/CD
```yaml
name: Multi-Platform Build

on: [push, pull_request]

jobs:
  build-desktop:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v2
      - name: Setup Python
        uses: actions/setup-python@v2
        with:
          python-version: '3.9'
      - name: Build
        run: python universal_builder.py --desktop
  
  build-mobile:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build Android
        run: |
          sudo apt-get install -y android-sdk
          python universal_builder.py --mobile android
```

---

## 🎯 Quick Reference

### Essential Commands
```bash
# Status
zerotier-cli info

# List networks
zerotier-cli listnetworks

# Join network
zerotier-cli join NETWORK_ID

# Leave network  
zerotier-cli leave NETWORK_ID

# List peers
zerotier-cli listpeers
```

### Configuration Files
```
Linux:   /var/lib/zerotier-one/
macOS:   /Library/Application Support/ZeroTier/One/
Windows: C:\ProgramData\ZeroTier\One\
FreeBSD: /var/db/zerotier-one/
```

### Default Ports
- **UDP 9993** - ZeroTier protocol
- **TCP 443/80** - Web management (optional)
- **TCP 9993** - API/management interface

---

## 📞 Support & Community

- **Documentation**: https://docs.zerotier.com
- **Community**: https://discuss.zerotier.com  
- **GitHub**: https://github.com/zerotier/ZeroTierOne
- **Discord**: ZeroTier Community Server

---

*Este guia cobre todas as principais plataformas suportadas pelo ZeroTier. Para configurações específicas ou problemas, consulte a documentação oficial ou a comunidade.*
