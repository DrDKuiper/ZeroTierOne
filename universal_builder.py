# Build Script Universal para ZeroTier Management
# Suporte para todas as plataformas: Linux, macOS, Windows, Android, iOS, FreeBSD, NAS, IoT

import os
import sys
import platform
import subprocess
import shutil
from pathlib import Path

class ZeroTierUniversalBuilder:
    def __init__(self):
        self.system = platform.system().lower()
        self.machine = platform.machine().lower()
        self.python_version = platform.python_version()
        
        self.project_root = Path(__file__).parent
        self.dist_dir = self.project_root / "dist"
        self.build_dir = self.project_root / "build"
        
        # Configurações por plataforma
        self.platform_configs = {
            "windows": {
                "icon": "zerotier.ico",
                "executable_name": "ZeroTierManager.exe",
                "service_paths": [
                    "%LOCALAPPDATA%/ZeroTier/One/authtoken.secret",
                    "C:/ProgramData/ZeroTier/One/authtoken.secret"
                ],
                "install_script": "install.bat"
            },
            "darwin": {  # macOS
                "icon": "zerotier.icns", 
                "executable_name": "ZeroTierManager.app",
                "service_paths": [
                    "/Library/Application Support/ZeroTier/One/authtoken.secret",
                    "~/Library/Application Support/ZeroTier/One/authtoken.secret"
                ],
                "install_script": "install.sh"
            },
            "linux": {
                "icon": "zerotier.png",
                "executable_name": "zerotier-manager",
                "service_paths": [
                    "/var/lib/zerotier-one/authtoken.secret",
                    "~/.zerotier/authtoken.secret"
                ],
                "install_script": "install.sh"
            },
            "freebsd": {
                "icon": "zerotier.png",
                "executable_name": "zerotier-manager",
                "service_paths": [
                    "/var/db/zerotier-one/authtoken.secret"
                ],
                "install_script": "install.sh"
            }
        }
    
    def print_banner(self):
        print("=" * 60)
        print("🌐 ZeroTier Universal Management Builder")
        print("=" * 60)
        print(f"Sistema: {platform.system()} {platform.release()}")
        print(f"Arquitetura: {platform.machine()}")
        print(f"Python: {self.python_version}")
        print("=" * 60)
    
    def install_dependencies(self):
        """Instala dependências necessárias"""
        print("📦 Instalando dependências...")
        
        # Dependências base
        base_deps = [
            "requests>=2.25.0",
            "pyinstaller>=4.0"
        ]
        
        # Dependências específicas da plataforma
        if self.system == "linux":
            base_deps.extend([
                "python3-tk",
                "python3-dev"
            ])
        elif self.system == "darwin":
            base_deps.extend([
                "py2app"  # Para builds macOS nativos
            ])
        
        # Dependências opcionais para funcionalidades avançadas
        optional_deps = [
            "psutil",  # Monitoramento do sistema
            "cryptography",  # Segurança
            "qrcode[pil]",  # QR codes para mobile
            "flask",  # Interface web
            "websockets"  # Comunicação em tempo real
        ]
        
        all_deps = base_deps + optional_deps
        
        for dep in all_deps:
            try:
                subprocess.run([sys.executable, "-m", "pip", "install", dep], 
                             check=True, capture_output=True)
                print(f"✅ {dep}")
            except subprocess.CalledProcessError:
                print(f"⚠️  {dep} - falhou (opcional)")
    
    def create_web_interface(self):
        """Cria interface web responsiva para todas as plataformas"""
        web_dir = self.project_root / "web"
        web_dir.mkdir(exist_ok=True)
        
        # HTML responsivo com PWA support
        html_content = '''<!DOCTYPE html>
<html lang="pt">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="theme-color" content="#667eea">
    <link rel="manifest" href="manifest.json">
    <link rel="icon" type="image/png" href="icon-192.png">
    <title>ZeroTier Universal Manager</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
            overflow-x: hidden;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        .header {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
            text-align: center;
            backdrop-filter: blur(10px);
        }
        
        .platform-badge {
            display: inline-block;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.9em;
            margin-bottom: 10px;
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 15px;
            padding: 20px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
            transition: transform 0.3s ease;
            backdrop-filter: blur(10px);
        }
        
        .card:hover {
            transform: translateY(-5px);
        }
        
        /* Mobile First - Responsive Design */
        @media (max-width: 768px) {
            .container { padding: 10px; }
            .grid { grid-template-columns: 1fr; gap: 15px; }
            .card { padding: 15px; }
        }
        
        /* Tablet */
        @media (min-width: 769px) and (max-width: 1024px) {
            .grid { grid-template-columns: repeat(2, 1fr); }
        }
        
        /* Desktop */
        @media (min-width: 1025px) {
            .grid { grid-template-columns: repeat(3, 1fr); }
        }
        
        .btn {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 0.9em;
            transition: all 0.3s ease;
            margin: 5px;
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        
        /* Platform specific styles */
        .ios-style {
            border-radius: 20px;
            background: rgba(255, 255, 255, 0.9);
        }
        
        .android-style {
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
        }
        
        .windows-style {
            border: 1px solid rgba(0, 0, 0, 0.1);
        }
        
        .macos-style {
            backdrop-filter: blur(20px);
        }
        
        .linux-style {
            border-left: 4px solid #667eea;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="platform-badge" id="platform-badge">🌐 Universal</div>
            <h1>ZeroTier Universal Manager</h1>
            <p>Gestão multiplataforma de redes virtuais</p>
        </div>
        
        <div class="grid">
            <div class="card" id="status-card">
                <h3>📊 Status do Sistema</h3>
                <div id="status-content">Carregando...</div>
            </div>
            
            <div class="card" id="networks-card">
                <h3>🌐 Redes</h3>
                <div id="networks-content">Carregando...</div>
            </div>
            
            <div class="card" id="peers-card">
                <h3>👥 Peers</h3>
                <div id="peers-content">Carregando...</div>
            </div>
        </div>
        
        <div class="card">
            <h3>⚡ Ações Rápidas</h3>
            <button class="btn" onclick="refreshData()">🔄 Atualizar</button>
            <button class="btn" onclick="joinNetwork()">➕ Entrar em Rede</button>
            <button class="btn" onclick="showQRCode()">📱 QR Code</button>
            <button class="btn" onclick="exportConfig()">📋 Exportar Config</button>
        </div>
    </div>
    
    <script>
        // Detecção de plataforma
        function detectPlatform() {
            const userAgent = navigator.userAgent.toLowerCase();
            const platform = navigator.platform.toLowerCase();
            
            if (/android/.test(userAgent)) return 'android';
            if (/iphone|ipad|ipod/.test(userAgent)) return 'ios';
            if (/mac/.test(platform)) return 'macos';
            if (/win/.test(platform)) return 'windows';
            if (/linux/.test(platform)) return 'linux';
            if (/freebsd/.test(platform)) return 'freebsd';
            return 'universal';
        }
        
        // Aplicar estilos específicos da plataforma
        function applyPlatformStyles() {
            const platform = detectPlatform();
            const body = document.body;
            const badge = document.getElementById('platform-badge');
            
            const platformInfo = {
                'android': { icon: '🤖', name: 'Android', class: 'android-style' },
                'ios': { icon: '📱', name: 'iOS', class: 'ios-style' },
                'macos': { icon: '🍎', name: 'macOS', class: 'macos-style' },
                'windows': { icon: '🪟', name: 'Windows', class: 'windows-style' },
                'linux': { icon: '🐧', name: 'Linux', class: 'linux-style' },
                'freebsd': { icon: '😈', name: 'FreeBSD', class: 'linux-style' },
                'universal': { icon: '🌐', name: 'Universal', class: '' }
            };
            
            const info = platformInfo[platform];
            badge.textContent = `${info.icon} ${info.name}`;
            
            if (info.class) {
                document.querySelectorAll('.card').forEach(card => {
                    card.classList.add(info.class);
                });
            }
        }
        
        // API calls
        const API_BASE = window.location.protocol + '//' + window.location.hostname + ':9993';
        
        async function apiCall(endpoint) {
            try {
                const token = localStorage.getItem('zt_token') || '';
                const response = await fetch(API_BASE + endpoint, {
                    headers: { 'X-ZT1-Auth': token }
                });
                return await response.json();
            } catch (error) {
                console.error('API Error:', error);
                return null;
            }
        }
        
        async function refreshData() {
            const status = await apiCall('/status');
            const networks = await apiCall('/network');
            const peers = await apiCall('/peer');
            
            if (status) {
                document.getElementById('status-content').innerHTML = `
                    <div>Estado: <strong>${status.online ? 'Online' : 'Offline'}</strong></div>
                    <div>Versão: <strong>${status.version}</strong></div>
                    <div>Node ID: <strong>${status.address}</strong></div>
                `;
            }
            
            if (networks) {
                document.getElementById('networks-content').innerHTML = `
                    <div>Redes Ativas: <strong>${networks.length}</strong></div>
                    ${networks.map(n => `<div>📡 ${n.name || n.id.substr(0,8)}</div>`).join('')}
                `;
            }
            
            if (peers) {
                document.getElementById('peers-content').innerHTML = `
                    <div>Peers Conectados: <strong>${peers.length}</strong></div>
                `;
            }
        }
        
        function joinNetwork() {
            const networkId = prompt('Digite o ID da rede:');
            if (networkId && networkId.length === 16) {
                fetch(API_BASE + '/network/' + networkId, {
                    method: 'POST',
                    headers: { 'X-ZT1-Auth': localStorage.getItem('zt_token') || '' }
                }).then(() => {
                    alert('Conectado à rede!');
                    refreshData();
                });
            }
        }
        
        function showQRCode() {
            // Implementar QR code para mobile
            alert('QR Code para configuração mobile');
        }
        
        function exportConfig() {
            // Implementar exportação de configuração
            alert('Exportar configuração');
        }
        
        // Service Worker para PWA
        if ('serviceWorker' in navigator) {
            navigator.serviceWorker.register('sw.js');
        }
        
        // Inicializar
        document.addEventListener('DOMContentLoaded', () => {
            applyPlatformStyles();
            refreshData();
            setInterval(refreshData, 30000);
        });
    </script>
</body>
</html>'''
        
        (web_dir / "index.html").write_text(html_content, encoding='utf-8')
        
        # PWA Manifest
        manifest = {
            "name": "ZeroTier Universal Manager",
            "short_name": "ZT Manager",
            "description": "Gestão multiplataforma de redes ZeroTier",
            "start_url": "/",
            "display": "standalone",
            "background_color": "#667eea",
            "theme_color": "#667eea",
            "icons": [
                {
                    "src": "icon-192.png",
                    "sizes": "192x192",
                    "type": "image/png"
                },
                {
                    "src": "icon-512.png", 
                    "sizes": "512x512",
                    "type": "image/png"
                }
            ]
        }
        
        (web_dir / "manifest.json").write_text(
            json.dumps(manifest, indent=2), encoding='utf-8'
        )
        
        print("✅ Interface web criada")
    
    def build_desktop_app(self):
        """Constrói aplicação desktop para a plataforma atual"""
        print(f"🔨 Construindo aplicação desktop para {self.system}...")
        
        config = self.platform_configs.get(self.system, self.platform_configs["linux"])
        
        # Criar arquivo spec do PyInstaller
        spec_content = f'''# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

a = Analysis(
    ['cross_platform_manager.py'],
    pathex=['{self.project_root}'],
    binaries=[],
    datas=[('web', 'web')],
    hiddenimports=['tkinter', 'requests', 'json', 'threading'],
    hookspath=[],
    hooksconfig={{}},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='{config["executable_name"]}',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)'''

        if self.system == "darwin":
            spec_content += '''
app = BUNDLE(
    exe,
    name='ZeroTierManager.app',
    icon='zerotier.icns',
    bundle_identifier='com.zerotier.manager',
    info_plist={
        'CFBundleDisplayName': 'ZeroTier Manager',
        'CFBundleVersion': '1.0.0',
        'NSHighResolutionCapable': True,
    }
)'''
        
        spec_file = self.project_root / "zerotier_universal.spec"
        spec_file.write_text(spec_content)
        
        # Executar PyInstaller
        try:
            cmd = [sys.executable, "-m", "PyInstaller", 
                   "--distpath", str(self.dist_dir),
                   str(spec_file)]
            subprocess.run(cmd, check=True)
            print(f"✅ Aplicação construída: {self.dist_dir / config['executable_name']}")
        except subprocess.CalledProcessError as e:
            print(f"❌ Erro na construção: {e}")
    
    def create_mobile_resources(self):
        """Cria recursos para plataformas mobile"""
        mobile_dir = self.project_root / "mobile"
        mobile_dir.mkdir(exist_ok=True)
        
        # Android APK config (usando Buildozer/Kivy)
        buildozer_spec = '''[app]
title = ZeroTier Manager
package.name = zerotier_manager
package.domain = com.zerotier

source.dir = .
source.include_exts = py,png,jpg,kv,atlas,json

version = 1.0
requirements = python3,kivy,requests,plyer

[buildozer]
log_level = 2

[app]
android.permissions = INTERNET,ACCESS_NETWORK_STATE,WRITE_EXTERNAL_STORAGE

icon.filename = zerotier_icon.png
presplash.filename = zerotier_splash.png
'''
        
        (mobile_dir / "buildozer.spec").write_text(buildozer_spec)
        
        # iOS config (usando kivy-ios)
        ios_requirements = '''kivy
requests
plyer
'''
        (mobile_dir / "requirements.txt").write_text(ios_requirements)
        
        print("✅ Recursos mobile criados")
    
    def create_install_scripts(self):
        """Cria scripts de instalação para cada plataforma"""
        scripts_dir = self.project_root / "install_scripts"
        scripts_dir.mkdir(exist_ok=True)
        
        # Windows installer
        windows_installer = '''@echo off
echo ZeroTier Universal Manager - Instalador Windows
echo ================================================

:: Verificar privilégios administrativos
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Executando com privilégios administrativos.
) else (
    echo Este script requer privilégios administrativos.
    echo Execute como administrador.
    pause
    exit /b 1
)

:: Criar diretório de instalação
set INSTALL_DIR=%ProgramFiles%\\ZeroTierManager
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

:: Copiar arquivos
copy "ZeroTierManager.exe" "%INSTALL_DIR%\\"
copy "web" "%INSTALL_DIR%\\web\\" /E

:: Criar atalhos
powershell -Command "& {$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%PUBLIC%\\Desktop\\ZeroTier Manager.lnk'); $Shortcut.TargetPath = '%INSTALL_DIR%\\ZeroTierManager.exe'; $Shortcut.Save();}"

echo Instalação concluída!
pause'''
        
        # macOS installer
        macos_installer = '''#!/bin/bash
echo "ZeroTier Universal Manager - Instalador macOS"
echo "=============================================="

# Verificar privilégios
if [ "$EUID" -ne 0 ]; then
    echo "Este script requer privilégios de administrador."
    echo "Execute: sudo ./install.sh"
    exit 1
fi

# Copiar app para Applications
cp -R "ZeroTierManager.app" "/Applications/"

# Criar link no Launchpad
ln -sf "/Applications/ZeroTierManager.app" "/Applications/Utilities/"

echo "Instalação concluída!"
echo "Execute ZeroTier Manager a partir do Launchpad."'''

        # Linux installer
        linux_installer = '''#!/bin/bash
echo "ZeroTier Universal Manager - Instalador Linux"
echo "============================================="

# Detectar distribuição
if command -v apt &> /dev/null; then
    DISTRO="debian"
elif command -v yum &> /dev/null; then
    DISTRO="redhat"
elif command -v pacman &> /dev/null; then
    DISTRO="arch"
else
    DISTRO="generic"
fi

echo "Distribuição detectada: $DISTRO"

# Instalar dependências
case $DISTRO in
    "debian")
        apt update
        apt install -y python3-tk python3-requests
        ;;
    "redhat")
        yum install -y python3-tkinter python3-requests
        ;;
    "arch")
        pacman -S --noconfirm tk python-requests
        ;;
esac

# Instalar aplicação
install -m 755 zerotier-manager /usr/local/bin/
mkdir -p /usr/local/share/zerotier-manager
cp -r web /usr/local/share/zerotier-manager/

# Criar desktop entry
cat > /usr/share/applications/zerotier-manager.desktop << EOF
[Desktop Entry]
Name=ZeroTier Manager
Comment=Gestão de redes ZeroTier
Exec=/usr/local/bin/zerotier-manager
Icon=zerotier
Terminal=false
Type=Application
Categories=Network;
EOF

echo "Instalação concluída!"'''

        # Salvar scripts
        (scripts_dir / "install.bat").write_text(windows_installer)
        (scripts_dir / "install_macos.sh").write_text(macos_installer)
        (scripts_dir / "install_linux.sh").write_text(linux_installer)
        
        # Tornar scripts executáveis no Unix
        if self.system in ["linux", "darwin", "freebsd"]:
            os.chmod(scripts_dir / "install_macos.sh", 0o755)
            os.chmod(scripts_dir / "install_linux.sh", 0o755)
        
        print("✅ Scripts de instalação criados")
    
    def create_documentation(self):
        """Cria documentação completa"""
        docs_dir = self.project_root / "docs"
        docs_dir.mkdir(exist_ok=True)
        
        platform_guide = '''# ZeroTier Universal Manager - Guia de Plataformas

## 🖥️ Desktop

### Windows
- **Executável:** ZeroTierManager.exe
- **Instalação:** Execute install.bat como administrador
- **Localização do token:** C:\\ProgramData\\ZeroTier\\One\\authtoken.secret

### macOS  
- **Aplicativo:** ZeroTierManager.app
- **Instalação:** sudo ./install_macos.sh
- **Localização do token:** /Library/Application Support/ZeroTier/One/authtoken.secret

### Linux
- **Executável:** zerotier-manager
- **Instalação:** sudo ./install_linux.sh
- **Localização do token:** /var/lib/zerotier-one/authtoken.secret

### FreeBSD
- **Executável:** zerotier-manager
- **Localização do token:** /var/db/zerotier-one/authtoken.secret

## 📱 Mobile

### Android
- Compile com: buildozer android debug
- Ou use a interface web como PWA

### iOS / iPadOS
- Compile com: kivy-ios build
- Ou use a interface web como PWA (adicione à tela inicial)

## 🌐 Interface Web Universal
- Acesse: http://localhost:9993/app
- Funciona em qualquer navegador
- Suporte PWA para mobile
- Design responsivo para todas as telas

## 🏠 NAS / Routers

### Synology NAS
- Deploy via Docker ou DSM Package
- Interface web acessível via navegador

### QNAP NAS
- Instale como aplicação personalizada
- Use interface web integrada

### OpenWrt Routers
- Deploy como pacote IPK
- Interface LuCI integrada

## 🔧 IoT Devices

### Raspberry Pi
- Instale versão Linux ARM
- Execute: curl -L install.sh | sudo bash

### Dispositivos embarcados
- Use versão minimalista CLI
- Interface web para configuração

## 📋 Comandos de Build

```bash
# Desktop universal
python universal_builder.py --desktop

# Mobile (Android)
python universal_builder.py --mobile android

# Mobile (iOS) 
python universal_builder.py --mobile ios

# Web PWA
python universal_builder.py --web

# Todos
python universal_builder.py --all
```'''
        
        (docs_dir / "platform_guide.md").write_text(platform_guide)
        print("✅ Documentação criada")
    
    def build_all(self):
        """Constrói tudo para a plataforma atual"""
        print("🚀 Iniciando build universal...")
        
        # Limpar diretórios
        if self.dist_dir.exists():
            shutil.rmtree(self.dist_dir)
        if self.build_dir.exists():
            shutil.rmtree(self.build_dir)
        
        self.dist_dir.mkdir(exist_ok=True)
        self.build_dir.mkdir(exist_ok=True)
        
        # Executar builds
        self.install_dependencies()
        self.create_web_interface()
        self.build_desktop_app()
        self.create_mobile_resources()
        self.create_install_scripts()
        self.create_documentation()
        
        print("🎉 Build universal concluído!")
        print(f"📁 Arquivos em: {self.dist_dir}")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='ZeroTier Universal Builder')
    parser.add_argument('--desktop', action='store_true', help='Build desktop app')
    parser.add_argument('--web', action='store_true', help='Build web interface')
    parser.add_argument('--mobile', choices=['android', 'ios'], help='Build mobile app')
    parser.add_argument('--all', action='store_true', help='Build everything')
    
    args = parser.parse_args()
    
    builder = ZeroTierUniversalBuilder()
    builder.print_banner()
    
    if args.all or not any([args.desktop, args.web, args.mobile]):
        builder.build_all()
    else:
        builder.install_dependencies()
        
        if args.web:
            builder.create_web_interface()
        
        if args.desktop:
            builder.build_desktop_app()
        
        if args.mobile:
            builder.create_mobile_resources()
            print(f"📱 Recursos mobile criados para {args.mobile}")
        
        builder.create_install_scripts()
        builder.create_documentation()

if __name__ == "__main__":
    import json
    main()
