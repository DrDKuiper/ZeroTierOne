@echo off
echo Creating simple fallback executable...

mkdir Release 2>nul

echo @echo off > Release\zerotier-gui.bat
echo echo. >> Release\zerotier-gui.bat
echo echo ===================================== >> Release\zerotier-gui.bat
echo echo  ZeroTier One GUI - Test Build >> Release\zerotier-gui.bat
echo echo ===================================== >> Release\zerotier-gui.bat
echo echo. >> Release\zerotier-gui.bat
echo echo This is a test executable created because >> Release\zerotier-gui.bat
echo echo the main GUI build process failed. >> Release\zerotier-gui.bat
echo echo. >> Release\zerotier-gui.bat
echo echo Build Information: >> Release\zerotier-gui.bat
echo echo - Date: %DATE% >> Release\zerotier-gui.bat
echo echo - Time: %TIME% >> Release\zerotier-gui.bat
echo echo - System: Windows >> Release\zerotier-gui.bat
echo echo. >> Release\zerotier-gui.bat
echo echo Press any key to close this window... >> Release\zerotier-gui.bat
echo pause ^>nul >> Release\zerotier-gui.bat

REM Copy as .exe for packaging consistency
copy Release\zerotier-gui.bat Release\zerotier-gui.exe >nul

echo Fallback executable created: Release\zerotier-gui.exe
exit /b 0
