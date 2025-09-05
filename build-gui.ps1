# Master build script for ZeroTier One GUI (PowerShell version)

Write-Host "ZeroTier One GUI Build Script" -ForegroundColor Green
Write-Host "=============================" -ForegroundColor Green

# Check if we're on Windows
if ($IsWindows -or $env:OS -eq "Windows_NT") {
    Write-Host "Detected platform: Windows" -ForegroundColor Cyan
    
    # Check if build script exists
    if (Test-Path "build-gui-windows.bat") {
        Write-Host "Running Windows build script..." -ForegroundColor Yellow
        & cmd /c "build-gui-windows.bat"
    } else {
        Write-Host "build-gui-windows.bat not found!" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "This PowerShell script is for Windows only." -ForegroundColor Red
    Write-Host "Please use build-gui.sh on Linux/macOS." -ForegroundColor Yellow
    exit 1
}

Write-Host "Build script completed" -ForegroundColor Green
