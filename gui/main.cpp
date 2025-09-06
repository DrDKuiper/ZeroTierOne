#include "qt/QtGUIManager.hpp"
#include "common/GUIManager.hpp"
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <iostream>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    // Configurações da aplicação
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("ZeroTier One");
    app.setApplicationVersion("1.14.0");
    app.setOrganizationName("ZeroTier, Inc.");
    
    // Verificar se o sistema suporta system tray
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "ZeroTier One",
            "This system does not support system tray icons.");
        return 1;
    }
    
    try {
        // Criar e inicializar o GUI Manager
        ZeroTier::QtGUIManager guiManager(&app);
        
        if (!guiManager.initialize()) {
            QMessageBox::warning(nullptr, "ZeroTier One",
                "Failed to connect to ZeroTier One service.\n"
                "The GUI will start in offline mode.\n"
                "Make sure the ZeroTier One service is running for full functionality.");
        }
        
        // Mostrar notificação de inicialização
        guiManager.showNotification("ZeroTier One", "ZeroTier One GUI started successfully");
        
        // Executar o loop principal da aplicação
        int result = app.exec();
        
        // Limpeza
        guiManager.shutdown();
        
        return result;
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "ZeroTier One",
            QString("An error occurred: %1").arg(e.what()));
        return 1;
    }
    catch (...) {
        QMessageBox::critical(nullptr, "ZeroTier One",
            "An unknown error occurred.");
        return 1;
    }
}
    catch (...) {
        QMessageBox::critical(nullptr, "ZeroTier One",
            "An unknown error occurred.");
        return 1;
    }
}
