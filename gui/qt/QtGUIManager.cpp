#include "QtGUIManager.hpp"
#include <QHeaderView>
#include <QMessageBox>
#include <QCloseEvent>
#include <QIcon>
#include <QPixmap>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

namespace ZeroTier {

// =============================================================================
// MainWindow Implementation
// =============================================================================

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("ZeroTier One");
    setMinimumSize(800, 600);
    
    // Widget central
    centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);
    
    // Área de Status
    statusGroup = new QGroupBox("Node Status");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    
    nodeIdLabel = new QLabel("Node ID: Loading...");
    statusLabel = new QLabel("Status: Connecting...");
    versionLabel = new QLabel("Version: 1.14.0");
    
    statusLayout->addWidget(nodeIdLabel);
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(versionLabel);
    mainLayout->addWidget(statusGroup);
    
    // Área de Redes
    networksGroup = new QGroupBox("Networks");
    QVBoxLayout* networksLayout = new QVBoxLayout(networksGroup);
    
    // Tabela de redes
    networksTable = new QTableWidget(0, 6);
    QStringList networkHeaders = {"Network ID", "Name", "Status", "Type", "Members", "Traffic"};
    networksTable->setHorizontalHeaderLabels(networkHeaders);
    networksTable->horizontalHeader()->setStretchLastSection(true);
    networksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    networksLayout->addWidget(networksTable);
    
    // Controles de rede
    networkControls = new QHBoxLayout;
    networkIdInput = new QLineEdit;
    networkIdInput->setPlaceholderText("Network ID (16 digits)");
    joinButton = new QPushButton("Join Network");
    leaveButton = new QPushButton("Leave Network");
    refreshButton = new QPushButton("Refresh");
    
    networkControls->addWidget(networkIdInput);
    networkControls->addWidget(joinButton);
    networkControls->addWidget(leaveButton);
    networkControls->addWidget(refreshButton);
    networksLayout->addLayout(networkControls);
    
    mainLayout->addWidget(networksGroup);
    
    // Área de Peers
    peersGroup = new QGroupBox("Peers");
    QVBoxLayout* peersLayout = new QVBoxLayout(peersGroup);
    
    peersTable = new QTableWidget(0, 5);
    QStringList peerHeaders = {"Address", "Name", "Status", "Latency", "Version"};
    peersTable->setHorizontalHeaderLabels(peerHeaders);
    peersTable->horizontalHeader()->setStretchLastSection(true);
    peersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    peersLayout->addWidget(peersTable);
    
    mainLayout->addWidget(peersGroup);
    
    // Conectar sinais
    connect(joinButton, &QPushButton::clicked, this, &MainWindow::onJoinClicked);
    connect(leaveButton, &QPushButton::clicked, this, &MainWindow::onLeaveClicked);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
}

void MainWindow::updateNetworks(const std::vector<NetworkInfo>& networks) {
    networksTable->setRowCount(networks.size());
    
    for (size_t i = 0; i < networks.size(); ++i) {
        const auto& net = networks[i];
        
        networksTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(net.networkId)));
        networksTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(net.name)));
        networksTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(net.status)));
        networksTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(net.type)));
        networksTable->setItem(i, 4, new QTableWidgetItem(QString::number(net.memberCount)));
        
        QString traffic = QString("↓%1 ↑%2")
            .arg(QString::number(net.bytesIn / 1024) + "KB")
            .arg(QString::number(net.bytesOut / 1024) + "KB");
        networksTable->setItem(i, 5, new QTableWidgetItem(traffic));
    }
}

void MainWindow::updatePeers(const std::vector<PeerInfo>& peers) {
    peersTable->setRowCount(peers.size());
    
    for (size_t i = 0; i < peers.size(); ++i) {
        const auto& peer = peers[i];
        
        peersTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(peer.address)));
        peersTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(peer.name)));
        peersTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(peer.status)));
        peersTable->setItem(i, 3, new QTableWidgetItem(QString::number(peer.latency) + "ms"));
        peersTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(peer.version)));
    }
}

void MainWindow::updateStatus(const std::string& status) {
    statusLabel->setText(QString("Status: %1").arg(QString::fromStdString(status)));
}

void MainWindow::onJoinClicked() {
    QString networkId = networkIdInput->text();
    if (!networkId.isEmpty()) {
        emit joinNetworkRequested(networkId);
        networkIdInput->clear();
    }
}

void MainWindow::onLeaveClicked() {
    int currentRow = networksTable->currentRow();
    if (currentRow >= 0) {
        QTableWidgetItem* item = networksTable->item(currentRow, 0);
        if (item) {
            emit leaveNetworkRequested(item->text());
        }
    }
}

void MainWindow::onRefreshClicked() {
    emit refreshRequested();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}

// =============================================================================
// QtGUIManager Implementation
// =============================================================================

QtGUIManager::QtGUIManager(QApplication* app) : _app(app) {
    _networkManager = new QNetworkAccessManager(this);
}

QtGUIManager::~QtGUIManager() {
    shutdown();
}

bool QtGUIManager::initialize() {
    if (!_app) {
        return false;
    }
    
    // Load authentication token
    loadAuthToken();
    if (_authToken.isEmpty()) {
        QMessageBox::warning(nullptr, "ZeroTier One", 
            "Could not find ZeroTier One authentication token.\n"
            "Make sure ZeroTier One service is running and you have proper permissions.");
        return false;
    }
    
    // Conectar ao serviço
    if (!connectToService()) {
        return false;
    }
    
    // Criar janela principal
    _mainWindow = new MainWindow;
    
    // Conectar sinais da janela
    connect(_mainWindow, &MainWindow::joinNetworkRequested,
            this, &QtGUIManager::onJoinNetworkRequested);
    connect(_mainWindow, &MainWindow::leaveNetworkRequested,
            this, &QtGUIManager::onLeaveNetworkRequested);
    connect(_mainWindow, &MainWindow::refreshRequested,
            this, &QtGUIManager::onRefreshRequested);
    
    // Criar ícone da bandeja
    createTrayIcon();
    createTrayMenu();
    
    // Timer para atualizações
    _updateTimer = new QTimer;
    connect(_updateTimer, &QTimer::timeout, this, &QtGUIManager::onUpdateTimer);
    _updateTimer->start(5000); // Atualizar a cada 5 segundos
    
    // Atualização inicial
    updateUI();
    
    return true;
}

void QtGUIManager::shutdown() {
    if (_updateTimer) {
        _updateTimer->stop();
        delete _updateTimer;
        _updateTimer = nullptr;
    }
    
    if (_trayIcon) {
        _trayIcon->hide();
        delete _trayIcon;
        _trayIcon = nullptr;
    }
    
    if (_mainWindow) {
        delete _mainWindow;
        _mainWindow = nullptr;
    }
}

void QtGUIManager::showMainWindow() {
    if (_mainWindow) {
        _mainWindow->show();
        _mainWindow->raise();
        _mainWindow->activateWindow();
    }
}

void QtGUIManager::hideMainWindow() {
    if (_mainWindow) {
        _mainWindow->hide();
    }
}

void QtGUIManager::showNotification(const std::string& title, const std::string& message) {
    if (_trayIcon && _trayIcon->isVisible()) {
        _trayIcon->showMessage(
            QString::fromStdString(title),
            QString::fromStdString(message),
            QSystemTrayIcon::Information,
            5000
        );
    }
}

void QtGUIManager::updateTrayIcon(const std::string& status) {
    if (!_trayIcon) return;
    
    // Criar ícone baseado no status
    QPixmap pixmap(16, 16);
    if (status == "ONLINE") {
        pixmap.fill(QColor(0, 255, 0)); // Verde
    } else if (status == "OFFLINE") {
        pixmap.fill(QColor(255, 0, 0)); // Vermelho
    } else {
        pixmap.fill(QColor(255, 255, 0)); // Amarelo
    }
    
    _trayIcon->setIcon(QIcon(pixmap));
    _trayIcon->setToolTip(QString("ZeroTier One - %1").arg(QString::fromStdString(status)));
}

void QtGUIManager::createTrayIcon() {
    _trayIcon = new QSystemTrayIcon;
    
    // Try to load icon from resources first, fallback to colored pixmap
    QIcon icon(":/icons/zerotier_32.png");
    if (icon.isNull()) {
        // Fallback to generated icon
        QPixmap pixmap(32, 32);
        pixmap.fill(QColor(0, 125, 255)); // ZeroTier blue
        icon = QIcon(pixmap);
    }
    _trayIcon->setIcon(icon);
    
    connect(_trayIcon, &QSystemTrayIcon::activated,
            this, &QtGUIManager::onTrayIconActivated);
    
    _trayIcon->show();
}

void QtGUIManager::createTrayMenu() {
    _trayMenu = new QMenu;
    
    _showAction = new QAction("Show ZeroTier One");
    _hideAction = new QAction("Hide");
    _quitAction = new QAction("Quit");
    
    connect(_showAction, &QAction::triggered, this, &QtGUIManager::onShowActionTriggered);
    connect(_hideAction, &QAction::triggered, this, &QtGUIManager::onHideActionTriggered);
    connect(_quitAction, &QAction::triggered, this, &QtGUIManager::onQuitActionTriggered);
    
    _trayMenu->addAction(_showAction);
    _trayMenu->addAction(_hideAction);
    _trayMenu->addSeparator();
    _trayMenu->addAction(_quitAction);
    
    _trayIcon->setContextMenu(_trayMenu);
}

void QtGUIManager::updateUI() {
    if (_mainWindow) {
        auto networks = getNetworks();
        auto peers = getPeers();
        auto status = getNodeStatus();
        
        _mainWindow->updateNetworks(networks);
        _mainWindow->updatePeers(peers);
        _mainWindow->updateStatus(status);
        updateTrayIcon(status);
    }
}

// Slots
void QtGUIManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        showMainWindow();
    }
}

void QtGUIManager::onShowActionTriggered() {
    showMainWindow();
}

void QtGUIManager::onHideActionTriggered() {
    hideMainWindow();
}

void QtGUIManager::onQuitActionTriggered() {
    if (_app) {
        _app->quit();
    }
}

void QtGUIManager::onUpdateTimer() {
    updateUI();
}

void QtGUIManager::onJoinNetworkRequested(const QString& networkId) {
    joinNetwork(networkId.toStdString());
    updateUI();
    showNotification("Network", "Attempting to join network " + networkId.toStdString());
}

void QtGUIManager::onLeaveNetworkRequested(const QString& networkId) {
    leaveNetwork(networkId.toStdString());
    updateUI();
    showNotification("Network", "Left network " + networkId.toStdString());
}

void QtGUIManager::onRefreshRequested() {
    updateUI();
}

void QtGUIManager::loadAuthToken() {
    // Try to load auth token from the standard ZeroTier location
    QString tokenPath;
    
#ifdef Q_OS_WIN
    tokenPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/ZeroTier/One/authtoken.secret";
#elif defined(Q_OS_MAC)
    tokenPath = "/Library/Application Support/ZeroTier/One/authtoken.secret";
#else
    tokenPath = "/var/lib/zerotier-one/authtoken.secret";
#endif

    QFile tokenFile(tokenPath);
    if (tokenFile.open(QIODevice::ReadOnly)) {
        _authToken = tokenFile.readAll().trimmed();
    } else {
        // Fallback: try local user directory
        QString userTokenPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.zeroTierOneAuthToken";
        QFile userTokenFile(userTokenPath);
        if (userTokenFile.open(QIODevice::ReadOnly)) {
            _authToken = userTokenFile.readAll().trimmed();
        }
    }
}

QNetworkRequest QtGUIManager::createApiRequest(const QString& endpoint) {
    QNetworkRequest request;
    request.setUrl(QUrl("http://127.0.0.1:9993" + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!_authToken.isEmpty()) {
        request.setRawHeader("X-ZT1-Auth", _authToken.toUtf8());
    }
    return request;
}

void QtGUIManager::makeApiCall(const QString& endpoint, std::function<void(QJsonDocument)> callback) {
    QNetworkRequest request = createApiRequest(endpoint);
    QNetworkReply* reply = _networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, [this, reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            callback(doc);
        } else {
            qWarning() << "API call failed:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

} // namespace ZeroTier

#include "QtGUIManager.moc"
