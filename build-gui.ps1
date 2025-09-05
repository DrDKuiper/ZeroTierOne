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

Write-Host ""
Write-Host "Build completed!" -ForegroundColor Green
Write-Host "Standalone executable created with all Qt dependencies included." -ForegroundColor Cyan
Write-Host ""
Write-Host "Output files:" -ForegroundColor Yellow
Write-Host "- Standalone executable: build\deploy\ZeroTierOneGUI.exe" -ForegroundColor White
Write-Host "- All dependencies folder: build\deploy\" -ForegroundColor White
Write-Host ""
Write-Host "You can copy the entire 'build\deploy' folder to any Windows system" -ForegroundColor Green
Write-Host "The executable will run without requiring Qt to be installed." -ForegroundColor Green
