#ifndef ZEROTIER_QT_GUI_MANAGER_HPP
#define ZEROTIER_QT_GUI_MANAGER_HPP

#include "../common/GUIManager.hpp"
#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace ZeroTier {

/**
 * Janela principal da aplicação
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    
    // Área de status
    QGroupBox* statusGroup;
    QLabel* nodeIdLabel;
    QLabel* statusLabel;
    QLabel* versionLabel;
    
    // Área de redes
    QGroupBox* networksGroup;
    QTableWidget* networksTable;
    QHBoxLayout* networkControls;
    QLineEdit* networkIdInput;
    QPushButton* joinButton;
    QPushButton* leaveButton;
    QPushButton* refreshButton;
    
    // Área de peers
    QGroupBox* peersGroup;
    QTableWidget* peersTable;

public:
    MainWindow(QWidget* parent = nullptr);
    void updateNetworks(const std::vector<NetworkInfo>& networks);
    void updatePeers(const std::vector<PeerInfo>& peers);
    void updateStatus(const std::string& status);

signals:
    void joinNetworkRequested(const QString& networkId);
    void leaveNetworkRequested(const QString& networkId);
    void refreshRequested();

private slots:
    void onJoinClicked();
    void onLeaveClicked();
    void onRefreshClicked();

protected:
    void closeEvent(QCloseEvent* event) override;
};

/**
 * Implementação Qt da GUI Manager
 */
class QtGUIManager : public QObject, public BaseGUIManager {
    Q_OBJECT

private:
    QApplication* _app = nullptr;
    MainWindow* _mainWindow = nullptr;
    QSystemTrayIcon* _trayIcon = nullptr;
    QMenu* _trayMenu = nullptr;
    QTimer* _updateTimer = nullptr;
    QNetworkAccessManager* _networkManager = nullptr;
    QString _authToken;
    
    // Ações do menu
    QAction* _showAction = nullptr;
    QAction* _hideAction = nullptr;
    QAction* _quitAction = nullptr;

public:
    QtGUIManager(QApplication* app);
    virtual ~QtGUIManager();

    // Implementação da interface GUIManager
    bool initialize() override;
    void shutdown() override;
    void showMainWindow() override;
    void hideMainWindow() override;
    void showNotification(const std::string& title, const std::string& message) override;
    void updateTrayIcon(const std::string& status) override;
    
    // Métodos de dados do ZeroTier
    std::vector<NetworkInfo> getNetworks() const override;
    std::vector<PeerInfo> getPeers() const override;
    bool joinNetwork(const std::string& networkId) override;
    bool leaveNetwork(const std::string& networkId) override;
    std::string getNodeStatus() const override;
    
    // Métodos de callback
    void setNetworkCallback(NetworkCallback callback) override;
    void setStatusCallback(StatusCallback callback) override;
    void setErrorCallback(ErrorCallback callback) override;
    
    // Método auxiliar
    bool connectToService();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowActionTriggered();
    void onHideActionTriggered();
    void onQuitActionTriggered();
    void onUpdateTimer();
    void onJoinNetworkRequested(const QString& networkId);
    void onLeaveNetworkRequested(const QString& networkId);
    void onRefreshRequested();

private:
    void createTrayIcon();
    void createTrayMenu();
    void updateUI();
    void loadAuthToken();
    QNetworkRequest createApiRequest(const QString& endpoint);
    void makeApiCall(const QString& endpoint, std::function<void(QJsonDocument)> callback);
};

} // namespace ZeroTier

#endif // ZEROTIER_QT_GUI_MANAGER_HPP
