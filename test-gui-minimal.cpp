#include <windows.h>
#include <iostream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create a simple message box for testing
    MessageBoxA(NULL, 
                "ZeroTier GUI Test Build\n\n"
                "This is a minimal test executable created when the main build fails.\n\n"
                "Features:\n"
                "- Windows-compatible executable\n"
                "- GUI subsystem\n"
                "- Test functionality\n\n"
                "Click OK to close.", 
                "ZeroTier One - Test Build", 
                MB_OK | MB_ICONINFORMATION);
    
    return 0;
}
