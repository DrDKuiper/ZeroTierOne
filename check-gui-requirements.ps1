# ZeroTier One GUI - Windows System Requirements Check
# PowerShell script to check if the system is ready for GUI builds

Write-Host "ZeroTier One GUI - System Requirements Check" -ForegroundColor Cyan
Write-Host "===========================================" -ForegroundColor Cyan
Write-Host

# Function to check if command exists
function Test-Command {
    param($Command)
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    }
    catch {
        return $false
    }
}

# Function to check if software is installed via registry or common paths
function Test-Software {
    param($Name, $ExecutableName)
    
    # Check if executable is in PATH
    if (Test-Command $ExecutableName) {
        return $true
    }
    
    # Check common installation paths
    $commonPaths = @(
        "${env:ProgramFiles}\$Name",
        "${env:ProgramFiles(x86)}\$Name",
        "${env:LOCALAPPDATA}\$Name",
        "${env:USERPROFILE}\$Name"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            return $true
        }
    }
    
    return $false
}

# Platform Detection
Write-Host "🖥️  Platform Detection:" -ForegroundColor Yellow
Write-Host "✓ Windows detected" -ForegroundColor Green
$WindowsVersion = (Get-WmiObject -Class Win32_OperatingSystem).Version
Write-Host "   Version: $WindowsVersion" -ForegroundColor Gray
Write-Host

# Check basic requirements
Write-Host "🔧 Basic Requirements:" -ForegroundColor Yellow

# Check CMake
if (Test-Command "cmake") {
    $cmakeVersion = (cmake --version | Select-String "cmake version" | ForEach-Object { $_.ToString().Split(" ")[2] })
    Write-Host "✓ CMake found: $cmakeVersion" -ForegroundColor Green
} else {
    Write-Host "❌ CMake not found" -ForegroundColor Red
}

# Check Qt6
$qtFound = $false
$qtVersion = "unknown"

if (Test-Command "qmake") {
    try {
        $qtVersion = (qmake -query QT_VERSION 2>$null)
        if ($qtVersion) {
            Write-Host "✓ Qt found: $qtVersion" -ForegroundColor Green
            $qtFound = $true
        }
    } catch {}
}

if (-not $qtFound) {
    # Check for Qt6 in common installation paths
    $qtPaths = @(
        "${env:ProgramFiles}\Qt",
        "${env:ProgramFiles(x86)}\Qt",
        "C:\Qt"
    )
    
    foreach ($qtPath in $qtPaths) {
        if (Test-Path $qtPath) {
            $qtVersions = Get-ChildItem $qtPath -Directory | Where-Object { $_.Name -match "^6\." }
            if ($qtVersions) {
                $latestQt = $qtVersions | Sort-Object Name -Descending | Select-Object -First 1
                Write-Host "✓ Qt6 found: $($latestQt.Name)" -ForegroundColor Green
                Write-Host "   Path: $($latestQt.FullName)" -ForegroundColor Gray
                $qtFound = $true
                break
            }
        }
    }
}

if (-not $qtFound) {
    Write-Host "❌ Qt6 not found" -ForegroundColor Red
}

# Check Visual Studio Build Tools
$vsFound = $false
if (Test-Command "cl") {
    Write-Host "✓ Visual Studio C++ compiler found" -ForegroundColor Green
    $vsFound = $true
} else {
    # Check for Visual Studio installations
    $vsPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022",
        "${env:ProgramFiles}\Microsoft Visual Studio\2019",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019"
    )
    
    foreach ($vsPath in $vsPaths) {
        if (Test-Path $vsPath) {
            Write-Host "✓ Visual Studio installation found: $vsPath" -ForegroundColor Green
            $vsFound = $true
            break
        }
    }
}

if (-not $vsFound) {
    Write-Host "❌ Visual Studio C++ compiler not found" -ForegroundColor Red
}

Write-Host

# Windows-specific checks
Write-Host "🪟 Windows-specific Requirements:" -ForegroundColor Yellow

# Check Windows SDK
$sdkFound = $false
$sdkPaths = @(
    "${env:ProgramFiles(x86)}\Windows Kits\10",
    "${env:ProgramFiles}\Windows Kits\10"
)

foreach ($sdkPath in $sdkPaths) {
    if (Test-Path $sdkPath) {
        $sdkVersions = Get-ChildItem "$sdkPath\Include" -Directory | Sort-Object Name -Descending
        if ($sdkVersions) {
            Write-Host "✓ Windows SDK found: $($sdkVersions[0].Name)" -ForegroundColor Green
            $sdkFound = $true
            break
        }
    }
}

if (-not $sdkFound) {
    Write-Host "❌ Windows SDK not found" -ForegroundColor Red
}

# Check Git
if (Test-Command "git") {
    $gitVersion = (git --version | ForEach-Object { $_.ToString().Split(" ")[2] })
    Write-Host "✓ Git found: $gitVersion" -ForegroundColor Green
} else {
    Write-Host "❌ Git not found" -ForegroundColor Red
}

Write-Host

# Check ZeroTier service
Write-Host "🌐 ZeroTier Service:" -ForegroundColor Yellow
if (Test-Command "zerotier-cli") {
    try {
        $ztInfo = (zerotier-cli info 2>$null)
        if ($ztInfo) {
            $ztStatus = $ztInfo.Split(" ")[2]
            Write-Host "✓ ZeroTier CLI found, status: $ztStatus" -ForegroundColor Green
        } else {
            Write-Host "⚠️  ZeroTier CLI found but service may not be running" -ForegroundColor Yellow
        }
    } catch {
        Write-Host "⚠️  ZeroTier CLI found but unable to get status" -ForegroundColor Yellow
    }
} else {
    Write-Host "❌ ZeroTier CLI not found" -ForegroundColor Red
    Write-Host "   Install ZeroTier One service first from: https://www.zerotier.com/download/" -ForegroundColor Gray
}

Write-Host

# Summary
Write-Host "📋 Build Readiness Summary:" -ForegroundColor Yellow
$ready = $true

if (-not (Test-Command "cmake")) {
    Write-Host "❌ Missing: CMake" -ForegroundColor Red
    Write-Host "   Download from: https://cmake.org/download/" -ForegroundColor Gray
    $ready = $false
}

if (-not $qtFound) {
    Write-Host "❌ Missing: Qt6" -ForegroundColor Red
    Write-Host "   Download from: https://www.qt.io/download-qt-installer" -ForegroundColor Gray
    $ready = $false
}

if (-not $vsFound) {
    Write-Host "❌ Missing: Visual Studio C++ Build Tools" -ForegroundColor Red
    Write-Host "   Download from: https://visualstudio.microsoft.com/visual-cpp-build-tools/" -ForegroundColor Gray
    $ready = $false
}

if ($ready) {
    Write-Host "✅ System is ready for GUI build!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Cyan
    Write-Host "1. Open Developer Command Prompt for Visual Studio" -ForegroundColor White
    Write-Host "2. Run the build script: .\build-gui-windows.bat" -ForegroundColor White
    Write-Host "3. Or use PowerShell: .\build-gui.ps1" -ForegroundColor White
} else {
    Write-Host "❌ System is NOT ready for GUI build" -ForegroundColor Red
    Write-Host "   Please install the missing dependencies listed above" -ForegroundColor Gray
}

Write-Host
Write-Host "Press any key to continue..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
