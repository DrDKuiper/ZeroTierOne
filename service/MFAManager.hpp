#ifndef ZT_MFAMANAGER_HPP
#define ZT_MFAMANAGER_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <chrono>

#include "../include/ZeroTierOne.h"
#include "../osdep/OSUtils.hpp"

namespace ZeroTier {

/**
 * Multi-Factor Authentication Manager for ZeroTier
 * Provides TOTP (Time-based One-Time Password) support for administrative operations
 */
class MFAManager
{
public:
    struct MFAChallenge {
        std::string challengeId;
        std::string clientId;
        std::string secretKey;
        uint64_t timestamp;
        int attempts;
        bool verified;
        std::string endpoint;
    };

    struct TOTPConfig {
        std::string secret;
        uint64_t timeStep;
        int digits;
        std::string algorithm;
    };

    MFAManager();
    ~MFAManager();

    /**
     * Initialize MFA for a client with a secret key
     */
    bool initializeMFA(const std::string& clientId, const std::string& secretKey);

    /**
     * Generate a new MFA challenge for administrative operations
     */
    std::string generateChallenge(const std::string& clientId, const std::string& endpoint);

    /**
     * Verify MFA challenge response
     */
    bool verifyChallenge(const std::string& challengeId, const std::string& totpCode);

    /**
     * Check if endpoint requires MFA
     */
    bool requiresMFA(const std::string& endpoint) const;

    /**
     * Generate TOTP code for testing/verification
     */
    std::string generateTOTP(const std::string& secret, uint64_t timestamp = 0) const;

    /**
     * Verify TOTP code against secret
     */
    bool verifyTOTP(const std::string& secret, const std::string& code, uint64_t timestamp = 0) const;

    /**
     * Get MFA setup information for client
     */
    std::string getMFASetupInfo(const std::string& clientId) const;

    /**
     * Remove expired challenges
     */
    void cleanupExpiredChallenges();

    /**
     * Get MFA status for client
     */
    bool isMFAEnabled(const std::string& clientId) const;

    /**
     * Get number of clients with MFA enabled
     */
    size_t getMFAEnabledClientsCount() const;

private:
    mutable Mutex _lock;
    std::unordered_map<std::string, TOTPConfig> _clientConfigs;
    std::unordered_map<std::string, MFAChallenge> _activeChallenges;
    std::vector<std::string> _mfaRequiredEndpoints;
    
    // Constants
    static constexpr uint64_t CHALLENGE_EXPIRY_MS = 300000; // 5 minutes
    static constexpr int MAX_ATTEMPTS = 3;
    static constexpr uint64_t TOTP_TIME_STEP = 30;
    static constexpr int TOTP_DIGITS = 6;

    /**
     * Generate a secure random secret
     */
    std::string generateSecret() const;

    /**
     * Convert base32 string to bytes
     */
    std::vector<uint8_t> base32Decode(const std::string& encoded) const;

    /**
     * Convert bytes to base32 string
     */
    std::string base32Encode(const std::vector<uint8_t>& data) const;

    /**
     * HMAC-SHA1 implementation
     */
    std::vector<uint8_t> hmacSha1(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) const;

    /**
     * Generate HOTP value
     */
    uint32_t generateHOTP(const std::vector<uint8_t>& secret, uint64_t counter) const;
};

} // namespace ZeroTier

#endif
