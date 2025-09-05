#ifndef ZEROTIER_BASE_GUI_MANAGER_HPP
#define ZEROTIER_BASE_GUI_MANAGER_HPP

#include "GUIManager.hpp"
#include <thread>

namespace ZeroTier {

class OneService;

/**
 * Implementação base comum para todas as GUIs
 */
class BaseGUIManager : public GUIManager {
protected:
    OneService* _service = nullptr;
    std::thread _updateThread;
    bool _running = false;
    
    NetworkCallback _networkCallback;
    StatusCallback _statusCallback;
    ErrorCallback _errorCallback;

public:
    BaseGUIManager() = default;
    virtual ~BaseGUIManager();

    // Conectar ao serviço ZeroTier
    bool connectToService();

    // Implementações comuns
    std::vector<NetworkInfo> getNetworks() const override;
    std::vector<PeerInfo> getPeers() const override;
    bool joinNetwork(const std::string& networkId) override;
    bool leaveNetwork(const std::string& networkId) override;
    std::string getNodeStatus() const override;
    
    // Callbacks
    void setNetworkCallback(NetworkCallback callback) override;
    void setStatusCallback(StatusCallback callback) override;
    void setErrorCallback(ErrorCallback callback) override;

protected:
    // Métodos auxiliares
    void startUpdateThread();
    void stopUpdateThread();
    void updateLoop();
    
    // Métodos virtuais que devem ser implementados pelas classes filhas
    virtual void onNetworkUpdate(const NetworkInfo& network) {}
    virtual void onStatusUpdate(const std::string& status) {}
    virtual void onError(const std::string& error) {}
};

} // namespace ZeroTier

#endif // ZEROTIER_BASE_GUI_MANAGER_HPP
