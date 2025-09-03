#include "BandwidthController.hpp"
#include "../node/Utils.hpp"
#include "../osdep/OSUtils.hpp"

#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace ZeroTier {

BandwidthController::BandwidthController()
    : _lastUpdate(OSUtils::now())
    , _updateInterval(TOKEN_BUCKET_REFILL_INTERVAL)
{
}

BandwidthController::~BandwidthController()
{
    Mutex::Lock _l(_lock);
    _networkPolicies.clear();
    _networkStats.clear();
    _peerStats.clear();
    _peerLimits.clear();
}

bool BandwidthController::setNetworkLimit(const std::string& networkId, uint64_t maxBytesPerSecond, uint64_t burstSize)
{
    if (networkId.empty() || maxBytesPerSecond == 0) {
        return false;
    }

    Mutex::Lock _l(_lock);
    
    auto& policy = _networkPolicies[networkId];
    policy.networkId = networkId;
    policy.globalLimit.maxBytesPerSecond = maxBytesPerSecond;
    policy.globalLimit.burstSize = (burstSize > 0) ? burstSize : (maxBytesPerSecond * DEFAULT_BURST_MULTIPLIER);
    policy.globalLimit.currentTokens = policy.globalLimit.burstSize;
    policy.globalLimit.lastUpdate = OSUtils::now();
    policy.globalLimit.enabled = true;

    // Initialize default QoS classes if not present
    if (policy.qosClasses.empty()) {
        policy.qosClasses = getDefaultQoSClasses();
    }

    return true;
}

bool BandwidthController::setPeerLimit(const std::string& networkId, const std::string& peerId, uint64_t maxBytesPerSecond, uint64_t burstSize)
{
    if (networkId.empty() || peerId.empty() || maxBytesPerSecond == 0) {
        return false;
    }

    Mutex::Lock _l(_lock);
    
    BandwidthLimit limit;
    limit.maxBytesPerSecond = maxBytesPerSecond;
    limit.burstSize = (burstSize > 0) ? burstSize : (maxBytesPerSecond * DEFAULT_BURST_MULTIPLIER);
    limit.currentTokens = limit.burstSize;
    limit.lastUpdate = OSUtils::now();
    limit.enabled = true;

    _peerLimits[networkId][peerId] = limit;
    return true;
}

bool BandwidthController::allowPacket(const std::string& networkId, const std::string& peerId, uint64_t packetSize, bool isOutbound)
{
    Mutex::Lock _l(_lock);
    
    uint64_t now = OSUtils::now();
    
    // Check network-level limit
    auto networkPolicyIt = _networkPolicies.find(networkId);
    if (networkPolicyIt != _networkPolicies.end()) {
        BandwidthLimit& networkLimit = networkPolicyIt->second.globalLimit;
        
        if (networkLimit.enabled) {
            // Update tokens based on time elapsed
            uint64_t timeDelta = now - networkLimit.lastUpdate;
            uint64_t tokensToAdd = (networkLimit.maxBytesPerSecond * timeDelta) / 1000;
            networkLimit.currentTokens = std::min(networkLimit.burstSize, networkLimit.currentTokens + tokensToAdd);
            networkLimit.lastUpdate = now;
            
            // Check if we have enough tokens
            if (networkLimit.currentTokens < packetSize) {
                // Update drop statistics
                auto& stats = _networkStats[networkId];
                stats.droppedPackets++;
                return false;
            }
            
            networkLimit.currentTokens -= packetSize;
        }
    }
    
    // Check peer-level limit
    auto networkPeerLimits = _peerLimits.find(networkId);
    if (networkPeerLimits != _peerLimits.end()) {
        auto peerLimitIt = networkPeerLimits->second.find(peerId);
        if (peerLimitIt != networkPeerLimits->second.end()) {
            BandwidthLimit& peerLimit = peerLimitIt->second;
            
            if (peerLimit.enabled) {
                // Update tokens based on time elapsed
                uint64_t timeDelta = now - peerLimit.lastUpdate;
                uint64_t tokensToAdd = (peerLimit.maxBytesPerSecond * timeDelta) / 1000;
                peerLimit.currentTokens = std::min(peerLimit.burstSize, peerLimit.currentTokens + tokensToAdd);
                peerLimit.lastUpdate = now;
                
                // Check if we have enough tokens
                if (peerLimit.currentTokens < packetSize) {
                    // Update drop statistics
                    auto& stats = _peerStats[networkId][peerId];
                    stats.droppedPackets++;
                    return false;
                }
                
                peerLimit.currentTokens -= packetSize;
            }
        }
    }
    
    return true;
}

void BandwidthController::recordTraffic(const std::string& networkId, const std::string& peerId, uint64_t bytes, bool isOutbound, double latency)
{
    Mutex::Lock _l(_lock);
    
    uint64_t now = OSUtils::now();
    
    // Update network statistics
    auto& networkStats = _networkStats[networkId];
    if (isOutbound) {
        networkStats.bytesOut += bytes;
        networkStats.packetsOut++;
    } else {
        networkStats.bytesIn += bytes;
        networkStats.packetsIn++;
    }
    
    if (latency > 0.0) {
        networkStats.avgLatency = updateLatencyEWMA(latency, networkStats.avgLatency);
    }
    
    // Update peer statistics
    auto& peerStats = _peerStats[networkId][peerId];
    if (isOutbound) {
        peerStats.bytesOut += bytes;
        peerStats.packetsOut++;
    } else {
        peerStats.bytesIn += bytes;
        peerStats.packetsIn++;
    }
    
    if (latency > 0.0) {
        peerStats.avgLatency = updateLatencyEWMA(latency, peerStats.avgLatency);
    }
}

BandwidthController::TrafficStats BandwidthController::getNetworkStats(const std::string& networkId) const
{
    Mutex::Lock _l(_lock);
    
    auto it = _networkStats.find(networkId);
    if (it != _networkStats.end()) {
        return it->second;
    }
    
    return TrafficStats{}; // Return empty stats
}

BandwidthController::TrafficStats BandwidthController::getPeerStats(const std::string& networkId, const std::string& peerId) const
{
    Mutex::Lock _l(_lock);
    
    auto networkIt = _peerStats.find(networkId);
    if (networkIt != _peerStats.end()) {
        auto peerIt = networkIt->second.find(peerId);
        if (peerIt != networkIt->second.end()) {
            return peerIt->second;
        }
    }
    
    return TrafficStats{}; // Return empty stats
}

bool BandwidthController::setQoSClass(const std::string& networkId, const std::string& className, const QoSClass& qosClass)
{
    if (networkId.empty() || className.empty()) {
        return false;
    }

    Mutex::Lock _l(_lock);
    
    auto& policy = _networkPolicies[networkId];
    policy.qosClasses[className] = qosClass;
    return true;
}

std::string BandwidthController::classifyPacket(const std::string& networkId, const void* packetData, size_t packetSize) const
{
    Mutex::Lock _l(_lock);
    
    // Simple classification based on packet size and patterns
    if (isRealTimeTraffic(packetData, packetSize)) {
        return "realtime";
    }
    
    return classifyBySize(packetSize);
}

bool BandwidthController::shouldDropPacket(const std::string& networkId, const std::string& qosClass, double currentLatency) const
{
    Mutex::Lock _l(_lock);
    
    auto policyIt = _networkPolicies.find(networkId);
    if (policyIt == _networkPolicies.end()) {
        return false;
    }
    
    auto qosIt = policyIt->second.qosClasses.find(qosClass);
    if (qosIt == policyIt->second.qosClasses.end()) {
        return false;
    }
    
    const QoSClass& qos = qosIt->second;
    
    // Drop packet if latency exceeds maximum for this class
    if (currentLatency > qos.maxLatency) {
        return true;
    }
    
    // Check congestion level
    double congestion = getCongestionLevel(networkId);
    if (congestion > policyIt->second.congestionThreshold) {
        // Drop lower priority packets first during congestion
        return qos.priority < 4; // Drop packets with priority less than 4
    }
    
    return false;
}

BandwidthController::NetworkPolicy BandwidthController::getNetworkPolicy(const std::string& networkId) const
{
    Mutex::Lock _l(_lock);
    
    auto it = _networkPolicies.find(networkId);
    if (it != _networkPolicies.end()) {
        return it->second;
    }
    
    return NetworkPolicy{}; // Return empty policy
}

bool BandwidthController::setNetworkPolicy(const std::string& networkId, const NetworkPolicy& policy)
{
    if (networkId.empty()) {
        return false;
    }

    Mutex::Lock _l(_lock);
    _networkPolicies[networkId] = policy;
    return true;
}

void BandwidthController::update()
{
    uint64_t now = OSUtils::now();
    
    if (now - _lastUpdate >= _updateInterval) {
        updateTokenBuckets();
        _lastUpdate = now;
    }
}

double BandwidthController::getCongestionLevel(const std::string& networkId) const
{
    Mutex::Lock _l(_lock);
    return calculateCongestion(networkId);
}

bool BandwidthController::setTrafficShaping(const std::string& networkId, bool enabled)
{
    Mutex::Lock _l(_lock);
    
    auto& policy = _networkPolicies[networkId];
    policy.trafficShaping = enabled;
    return true;
}

std::string BandwidthController::getPerformanceMetrics() const
{
    Mutex::Lock _l(_lock);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "{\"networks\":[";
    bool first = true;
    
    for (const auto& networkPair : _networkStats) {
        if (!first) oss << ",";
        first = false;
        
        const std::string& networkId = networkPair.first;
        const TrafficStats& stats = networkPair.second;
        
        oss << "{";
        oss << "\"networkId\":\"" << networkId << "\",";
        oss << "\"bytesIn\":" << stats.bytesIn << ",";
        oss << "\"bytesOut\":" << stats.bytesOut << ",";
        oss << "\"packetsIn\":" << stats.packetsIn << ",";
        oss << "\"packetsOut\":" << stats.packetsOut << ",";
        oss << "\"droppedPackets\":" << stats.droppedPackets << ",";
        oss << "\"avgLatency\":" << stats.avgLatency << ",";
        oss << "\"packetLoss\":" << stats.packetLoss << ",";
        oss << "\"congestionLevel\":" << calculateCongestion(networkId);
        oss << "}";
    }
    
    oss << "]}";
    return oss.str();
}

void BandwidthController::resetStats(const std::string& networkId)
{
    Mutex::Lock _l(_lock);
    
    if (networkId.empty()) {
        // Reset all stats
        _networkStats.clear();
        _peerStats.clear();
    } else {
        // Reset specific network stats
        _networkStats.erase(networkId);
        _peerStats.erase(networkId);
    }
}

void BandwidthController::updateTokenBuckets()
{
    uint64_t now = OSUtils::now();
    
    // Update network-level token buckets
    for (auto& policyPair : _networkPolicies) {
        BandwidthLimit& limit = policyPair.second.globalLimit;
        if (limit.enabled) {
            uint64_t timeDelta = now - limit.lastUpdate;
            uint64_t tokensToAdd = (limit.maxBytesPerSecond * timeDelta) / 1000;
            limit.currentTokens = std::min(limit.burstSize, limit.currentTokens + tokensToAdd);
            limit.lastUpdate = now;
        }
    }
    
    // Update peer-level token buckets
    for (auto& networkPair : _peerLimits) {
        for (auto& peerPair : networkPair.second) {
            BandwidthLimit& limit = peerPair.second;
            if (limit.enabled) {
                uint64_t timeDelta = now - limit.lastUpdate;
                uint64_t tokensToAdd = (limit.maxBytesPerSecond * timeDelta) / 1000;
                limit.currentTokens = std::min(limit.burstSize, limit.currentTokens + tokensToAdd);
                limit.lastUpdate = now;
            }
        }
    }
}

double BandwidthController::calculateCongestion(const std::string& networkId) const
{
    auto statsIt = _networkStats.find(networkId);
    if (statsIt == _networkStats.end()) {
        return 0.0;
    }
    
    const TrafficStats& stats = statsIt->second;
    
    // Simple congestion calculation based on packet loss and latency
    double congestion = 0.0;
    
    if (stats.packetsIn + stats.packetsOut > 0) {
        double dropRate = static_cast<double>(stats.droppedPackets) / (stats.packetsIn + stats.packetsOut);
        congestion += dropRate * 0.7; // 70% weight for drops
    }
    
    if (stats.avgLatency > 0.0) {
        // Normalize latency (assume 100ms as high latency)
        double normalizedLatency = std::min(1.0, stats.avgLatency / 100.0);
        congestion += normalizedLatency * 0.3; // 30% weight for latency
    }
    
    return std::min(1.0, congestion);
}

double BandwidthController::updateLatencyEWMA(double currentLatency, double previousAvg) const
{
    if (previousAvg == 0.0) {
        return currentLatency;
    }
    
    return CONGESTION_EWMA_ALPHA * currentLatency + (1.0 - CONGESTION_EWMA_ALPHA) * previousAvg;
}

std::unordered_map<std::string, BandwidthController::QoSClass> BandwidthController::getDefaultQoSClasses() const
{
    std::unordered_map<std::string, QoSClass> classes;
    
    classes["realtime"] = {
        "realtime",
        7,              // Highest priority
        0,              // No minimum guarantee
        UINT64_MAX,     // No maximum limit
        50.0,           // 50ms max latency
        true
    };
    
    classes["interactive"] = {
        "interactive",
        5,              // High priority
        0,
        UINT64_MAX,
        100.0,          // 100ms max latency
        true
    };
    
    classes["bulk"] = {
        "bulk",
        2,              // Low priority
        0,
        UINT64_MAX,
        1000.0,         // 1s max latency
        true
    };
    
    classes["background"] = {
        "background",
        0,              // Lowest priority
        0,
        UINT64_MAX,
        5000.0,         // 5s max latency
        true
    };
    
    return classes;
}

std::string BandwidthController::classifyBySize(size_t packetSize) const
{
    if (packetSize < 128) {
        return "realtime";      // Small packets (likely control/ACK)
    } else if (packetSize < 512) {
        return "interactive";   // Medium packets (web browsing, chat)
    } else if (packetSize < 1400) {
        return "bulk";          // Large packets (file transfer)
    } else {
        return "background";    // Very large packets (backups, etc.)
    }
}

bool BandwidthController::isRealTimeTraffic(const void* packetData, size_t packetSize) const
{
    if (!packetData || packetSize < 20) {
        return false;
    }
    
    // Simple heuristics for real-time traffic detection
    const uint8_t* data = static_cast<const uint8_t*>(packetData);
    
    // Check for common real-time traffic patterns
    // This is a simplified implementation - in practice, you'd use DPI
    
    // Small packet size suggests real-time traffic
    if (packetSize < 128) {
        return true;
    }
    
    // Check for UDP patterns that might indicate VoIP/gaming
    // This would require deeper packet inspection in a real implementation
    
    return false;
}

} // namespace ZeroTier
