# Script para criar executável da aplicação ZeroTier Management
# Build-ZeroTierManagement.ps1

param(
    [string]$OutputDir = ".\dist",
    [switch]$Clean = $false
)

Write-Host "🔧 ZeroTier Management - Build Script" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan

# Verificar se Python está instalado
try {
    $pythonVersion = python --version 2>$null
    Write-Host "✅ Python encontrado: $pythonVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ Python não encontrado. Instale Python 3.7+ primeiro." -ForegroundColor Red
    exit 1
}

# Criar diretório de output
if ($Clean -and (Test-Path $OutputDir)) {
    Write-Host "🧹 Limpando diretório de saída..." -ForegroundColor Yellow
    Remove-Item $OutputDir -Recurse -Force
}

if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

# Instalar dependências
Write-Host "📦 Instalando dependências..." -ForegroundColor Blue
pip install pyinstaller requests pillow

# Criar arquivo de especificação PyInstaller
$specContent = @"
# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

a = Analysis(
    ['management_app.py'],
    pathex=[],
    binaries=[],
    datas=[],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
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
    name='ZeroTierManager',
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
    icon='icon.ico'  # Adicione um ícone se disponível
)
"@

# Salvar arquivo spec
$specContent | Out-File -FilePath "zerotier_manager.spec" -Encoding UTF8

# Compilar aplicação
Write-Host "🔨 Compilando aplicação..." -ForegroundColor Blue
pyinstaller --distpath $OutputDir zerotier_manager.spec

if (Test-Path "$OutputDir\ZeroTierManager.exe") {
    Write-Host "✅ Aplicação compilada com sucesso!" -ForegroundColor Green
    Write-Host "📁 Arquivo: $OutputDir\ZeroTierManager.exe" -ForegroundColor Green
    
    # Criar script de instalação
    $installScript = @"
@echo off
echo Installing ZeroTier Management Console...

:: Verificar privilégios administrativos
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running with administrative privileges.
) else (
    echo This script requires administrative privileges.
    echo Please run as administrator.
    pause
    exit /b 1
)

:: Criar diretório de instalação
set INSTALL_DIR=%ProgramFiles%\ZeroTierManager
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

:: Copiar arquivos
copy "ZeroTierManager.exe" "%INSTALL_DIR%\"

:: Criar atalho na área de trabalho
powershell -Command "& {
    `$WshShell = New-Object -comObject WScript.Shell;
    `$Shortcut = `$WshShell.CreateShortcut('%PUBLIC%\Desktop\ZeroTier Manager.lnk');
    `$Shortcut.TargetPath = '%INSTALL_DIR%\ZeroTierManager.exe';
    `$Shortcut.WorkingDirectory = '%INSTALL_DIR%';
    `$Shortcut.Description = 'ZeroTier Network Management Console';
    `$Shortcut.Save();
}"

:: Criar entrada no menu iniciar
powershell -Command "& {
    `$WshShell = New-Object -comObject WScript.Shell;
    `$Shortcut = `$WshShell.CreateShortcut('%ALLUSERSPROFILE%\Microsoft\Windows\Start Menu\Programs\ZeroTier Manager.lnk');
    `$Shortcut.TargetPath = '%INSTALL_DIR%\ZeroTierManager.exe';
    `$Shortcut.WorkingDirectory = '%INSTALL_DIR%';
    `$Shortcut.Description = 'ZeroTier Network Management Console';
    `$Shortcut.Save();
}"

echo Installation completed successfully!
echo You can now run ZeroTier Manager from the desktop or start menu.
pause
"@
    
    $installScript | Out-File -FilePath "$OutputDir\install.bat" -Encoding ASCII
    
    Write-Host "📦 Script de instalação criado: $OutputDir\install.bat" -ForegroundColor Green
    
} else {
    Write-Host "❌ Falha na compilação!" -ForegroundColor Red
    exit 1
}

# Limpeza
Remove-Item "zerotier_manager.spec" -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "🎉 Build concluído com sucesso!" -ForegroundColor Green
Write-Host "📋 Próximos passos:" -ForegroundColor Yellow
Write-Host "   1. Execute install.bat como administrador para instalar" -ForegroundColor White
Write-Host "   2. Ou execute ZeroTierManager.exe diretamente" -ForegroundColor White
Write-Host "   3. Certifique-se que o ZeroTier One esteja rodando" -ForegroundColor White
