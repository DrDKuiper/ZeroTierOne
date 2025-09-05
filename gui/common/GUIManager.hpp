#ifndef ZEROTIER_GUI_MANAGER_HPP
#define ZEROTIER_GUI_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace ZeroTier {

/**
 * Estrutura para informações de rede
 */
struct NetworkInfo {
    std::string networkId;
    std::string name;
    std::string status;
    bool connected;
    std::string type;
    int memberCount;
    uint64_t bytesIn;
    uint64_t bytesOut;
};

/**
 * Estrutura para informações de peer
 */
struct PeerInfo {
    std::string address;
    std::string name;
    std::string status;
    int latency;
    std::string version;
};

/**
 * Interface comum para gerenciamento da GUI
 */
class GUIManager {
public:
    virtual ~GUIManager() = default;

    // Callbacks para eventos
    using NetworkCallback = std::function<void(const NetworkInfo&)>;
    using StatusCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    // Métodos virtuais puros para implementação específica
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void showMainWindow() = 0;
    virtual void hideMainWindow() = 0;
    virtual void showNotification(const std::string& title, const std::string& message) = 0;
    virtual void updateTrayIcon(const std::string& status) = 0;

    // Métodos comuns
    virtual std::vector<NetworkInfo> getNetworks() const = 0;
    virtual std::vector<PeerInfo> getPeers() const = 0;
    virtual bool joinNetwork(const std::string& networkId) = 0;
    virtual bool leaveNetwork(const std::string& networkId) = 0;
    virtual std::string getNodeStatus() const = 0;

    // Registrar callbacks
    virtual void setNetworkCallback(NetworkCallback callback) = 0;
    virtual void setStatusCallback(StatusCallback callback) = 0;
    virtual void setErrorCallback(ErrorCallback callback) = 0;
};

/**
 * Factory para criar instâncias específicas da GUI
 */
class GUIManagerFactory {
public:
    enum class GUIType {
        QT,
        WEB,
        NATIVE
    };

    static std::unique_ptr<GUIManager> create(GUIType type);
};

} // namespace ZeroTier

#endif // ZEROTIER_GUI_MANAGER_HPP
