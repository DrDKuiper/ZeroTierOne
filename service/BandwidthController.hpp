#ifndef ZT_BANDWIDTHCONTROLLER_HPP
#define ZT_BANDWIDTHCONTROLLER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <memory>

#include "../include/ZeroTierOne.h"
#include "../osdep/OSUtils.hpp"

namespace ZeroTier {

/**
 * Advanced Bandwidth Controller for ZeroTier
 * Provides traffic shaping, QoS, and bandwidth management
 */
class BandwidthController
{
public:
    struct BandwidthLimit {
        uint64_t maxBytesPerSecond;
        uint64_t burstSize;
        uint64_t currentTokens;
        uint64_t lastUpdate;
        bool enabled;
    };

    struct TrafficStats {
        uint64_t bytesIn;
        uint64_t bytesOut;
        uint64_t packetsIn;
        uint64_t packetsOut;
        uint64_t droppedPackets;
        uint64_t lastReset;
        double avgLatency;
        double packetLoss;
    };

    struct QoSClass {
        std::string name;
        int priority;           // 0-7, 7 = highest priority
        uint64_t minBandwidth;  // Guaranteed bandwidth in bytes/sec
        uint64_t maxBandwidth;  // Maximum bandwidth in bytes/sec
        double maxLatency;      // Maximum acceptable latency in ms
        bool enabled;
    };

    struct NetworkPolicy {
        std::string networkId;
        BandwidthLimit globalLimit;
        std::unordered_map<std::string, QoSClass> qosClasses;
        std::unordered_map<std::string, BandwidthLimit> peerLimits;
        bool trafficShaping;
        bool priorityQueuing;
        double congestionThreshold; // 0.0-1.0
    };

    BandwidthController();
    ~BandwidthController();

    /**
     * Set bandwidth limit for a network
     */
    bool setNetworkLimit(const std::string& networkId, uint64_t maxBytesPerSecond, uint64_t burstSize = 0);

    /**
     * Set bandwidth limit for a specific peer
     */
    bool setPeerLimit(const std::string& networkId, const std::string& peerId, uint64_t maxBytesPerSecond, uint64_t burstSize = 0);

    /**
     * Check if packet transmission is allowed (token bucket algorithm)
     */
    bool allowPacket(const std::string& networkId, const std::string& peerId, uint64_t packetSize, bool isOutbound = true);

    /**
     * Record traffic statistics
     */
    void recordTraffic(const std::string& networkId, const std::string& peerId, uint64_t bytes, bool isOutbound, double latency = 0.0);

    /**
     * Get traffic statistics for a network
     */
    TrafficStats getNetworkStats(const std::string& networkId) const;

    /**
     * Get traffic statistics for a peer
     */
    TrafficStats getPeerStats(const std::string& networkId, const std::string& peerId) const;

    /**
     * Configure QoS class for a network
     */
    bool setQoSClass(const std::string& networkId, const std::string& className, const QoSClass& qosClass);

    /**
     * Classify packet for QoS
     */
    std::string classifyPacket(const std::string& networkId, const void* packetData, size_t packetSize) const;

    /**
     * Apply traffic shaping
     */
    bool shouldDropPacket(const std::string& networkId, const std::string& qosClass, double currentLatency) const;

    /**
     * Get network policy
     */
    NetworkPolicy getNetworkPolicy(const std::string& networkId) const;

    /**
     * Set network policy
     */
    bool setNetworkPolicy(const std::string& networkId, const NetworkPolicy& policy);

    /**
     * Update bandwidth controller (called periodically)
     */
    void update();

    /**
     * Get congestion status
     */
    double getCongestionLevel(const std::string& networkId) const;

    /**
     * Enable/disable traffic shaping for network
     */
    bool setTrafficShaping(const std::string& networkId, bool enabled);

    /**
     * Get performance metrics
     */
    std::string getPerformanceMetrics() const;

    /**
     * Reset statistics
     */
    void resetStats(const std::string& networkId = "");

private:
    mutable Mutex _lock;
    std::unordered_map<std::string, NetworkPolicy> _networkPolicies;
    std::unordered_map<std::string, TrafficStats> _networkStats;
    std::unordered_map<std::string, std::unordered_map<std::string, TrafficStats>> _peerStats;
    std::unordered_map<std::string, std::unordered_map<std::string, BandwidthLimit>> _peerLimits;
    
    uint64_t _lastUpdate;
    uint64_t _updateInterval; // Update interval in milliseconds

    // Token bucket constants
    static constexpr uint64_t TOKEN_BUCKET_REFILL_INTERVAL = 100; // ms
    static constexpr double CONGESTION_EWMA_ALPHA = 0.1;
    static constexpr double DEFAULT_BURST_MULTIPLIER = 2.0;

    /**
     * Update token buckets
     */
    void updateTokenBuckets();

    /**
     * Calculate congestion level based on current metrics
     */
    double calculateCongestion(const std::string& networkId) const;

    /**
     * Apply exponential weighted moving average to latency
     */
    double updateLatencyEWMA(double currentLatency, double previousAvg) const;

    /**
     * Get default QoS classes
     */
    std::unordered_map<std::string, QoSClass> getDefaultQoSClasses() const;

    /**
     * Simple packet classification based on size and patterns
     */
    std::string classifyBySize(size_t packetSize) const;

    /**
     * Check if packet appears to be real-time traffic
     */
    bool isRealTimeTraffic(const void* packetData, size_t packetSize) const;
};

} // namespace ZeroTier

#endif
