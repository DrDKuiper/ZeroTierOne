#include "BaseGUIManager.hpp"
#include "GUIManager.hpp"
#include "../../service/OneService.hpp"
#include "../../node/Utils.hpp"
#include <thread>
#include <chrono>

namespace ZeroTier {

BaseGUIManager::~BaseGUIManager() {
    if (_running) {
        shutdown();
    }
}

// Conectar ao serviço ZeroTier
bool BaseGUIManager::connectToService() {
    // Aqui conectaríamos ao serviço via API local
    // Por enquanto, simular a conexão
    return true;
}

// Implementações comuns
std::vector<NetworkInfo> BaseGUIManager::getNetworks() const {
        std::vector<NetworkInfo> networks;
        
        // Simulação de dados - em implementação real,
        // faria requisição à API do serviço
        NetworkInfo net1;
        net1.networkId = "8056c2e21c000001";
        net1.name = "Earth";
        net1.status = "OK";
        net1.connected = true;
        net1.type = "Public";
        net1.memberCount = 12500;
        net1.bytesIn = 1024000;
        net1.bytesOut = 512000;
        networks.push_back(net1);

        NetworkInfo net2;
        net2.networkId = "a0cbf4b62a000001";
        net2.name = "Private Network";
        net2.status = "OK";
        net2.connected = true;
        net2.type = "Private";
        net2.memberCount = 15;
        net2.bytesIn = 2048000;
        net2.bytesOut = 1024000;
        networks.push_back(net2);

        return networks;
    }

std::vector<PeerInfo> BaseGUIManager::getPeers() const {
        std::vector<PeerInfo> peers;
        
        // Simulação de dados
        PeerInfo peer1;
        peer1.address = "12ac4a1e71";
        peer1.name = "planet1.zerotier.com";
        peer1.status = "ONLINE";
        peer1.latency = 25;
        peer1.version = "1.14.0";
        peers.push_back(peer1);

        PeerInfo peer2;
        peer2.address = "12ac4a1e72";
        peer2.name = "planet2.zerotier.com";
        peer2.status = "ONLINE";
        peer2.latency = 45;
        peer2.version = "1.14.0";
        peers.push_back(peer2);

        return peers;
    }

bool BaseGUIManager::joinNetwork(const std::string& networkId) {
        // Implementar join via API
        if (_networkCallback) {
            NetworkInfo info;
            info.networkId = networkId;
            info.name = "Joined Network";
            info.status = "REQUESTING_CONFIGURATION";
            info.connected = false;
            _networkCallback(info);
        }
        return true;
    }

bool BaseGUIManager::leaveNetwork(const std::string& networkId) {
        // Implementar leave via API
        if (_networkCallback) {
            NetworkInfo info;
            info.networkId = networkId;
            info.status = "NOT_FOUND";
            info.connected = false;
            _networkCallback(info);
        }
        return true;
    }

std::string BaseGUIManager::getNodeStatus() const {
        return "ONLINE";
    }

// Registrar callbacks
void BaseGUIManager::setNetworkCallback(NetworkCallback callback) {
        _networkCallback = callback;
    }

void BaseGUIManager::setStatusCallback(StatusCallback callback) {
        _statusCallback = callback;
    }

void BaseGUIManager::setErrorCallback(ErrorCallback callback) {
        _errorCallback = callback;
    }

// Thread para atualizações periódicas
void BaseGUIManager::startUpdateThread() {
    _running = true;
    _updateThread = std::thread([this]() {
        updateLoop();
    });
}

void BaseGUIManager::stopUpdateThread() {
    _running = false;
    if (_updateThread.joinable()) {
        _updateThread.join();
    }
}

void BaseGUIManager::updateLoop() {
    while (_running) {
        if (_statusCallback) {
            _statusCallback(getNodeStatus());
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

} // namespace ZeroTier
