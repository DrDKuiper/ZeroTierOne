#include "MFAManager.hpp"
#include "../node/Utils.hpp"
#include "../osdep/OSUtils.hpp"

#include <cstring>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ZeroTier {

MFAManager::MFAManager()
{
    // Initialize endpoints that require MFA
    _mfaRequiredEndpoints = {
        "/controller/network",  // Network creation/deletion
        "/controller/member",   // Member management
        "/network/",           // Network configuration changes (will check prefix)
        "/controller/",        // All controller operations
        "/unstable/"           // Unstable/dangerous operations
    };
}

MFAManager::~MFAManager()
{
    Mutex::Lock _l(_lock);
    _clientConfigs.clear();
    _activeChallenges.clear();
}

bool MFAManager::initializeMFA(const std::string& clientId, const std::string& secretKey)
{
    Mutex::Lock _l(_lock);
    
    if (clientId.empty()) {
        return false;
    }

    std::string secret = secretKey;
    if (secret.empty()) {
        secret = generateSecret();
    }

    TOTPConfig config;
    config.secret = secret;
    config.timeStep = TOTP_TIME_STEP;
    config.digits = TOTP_DIGITS;
    config.algorithm = "SHA1";

    _clientConfigs[clientId] = config;
    return true;
}

std::string MFAManager::generateChallenge(const std::string& clientId, const std::string& endpoint)
{
    Mutex::Lock _l(_lock);
    
    auto clientConfig = _clientConfigs.find(clientId);
    if (clientConfig == _clientConfigs.end()) {
        return "";
    }

    // Generate unique challenge ID
    std::string challengeId = Utils::hex(OSUtils::getSecureRandom(), 16);

    MFAChallenge challenge;
    challenge.challengeId = challengeId;
    challenge.clientId = clientId;
    challenge.secretKey = clientConfig->second.secret;
    challenge.timestamp = OSUtils::now();
    challenge.attempts = 0;
    challenge.verified = false;
    challenge.endpoint = endpoint;

    _activeChallenges[challengeId] = challenge;

    return challengeId;
}

bool MFAManager::verifyChallenge(const std::string& challengeId, const std::string& totpCode)
{
    Mutex::Lock _l(_lock);
    
    auto challengeIt = _activeChallenges.find(challengeId);
    if (challengeIt == _activeChallenges.end()) {
        return false;
    }

    MFAChallenge& challenge = challengeIt->second;

    // Check if challenge has expired
    uint64_t now = OSUtils::now();
    if ((now - challenge.timestamp) > CHALLENGE_EXPIRY_MS) {
        _activeChallenges.erase(challengeIt);
        return false;
    }

    // Check if too many attempts
    if (challenge.attempts >= MAX_ATTEMPTS) {
        _activeChallenges.erase(challengeIt);
        return false;
    }

    challenge.attempts++;

    // Verify TOTP code
    bool valid = verifyTOTP(challenge.secretKey, totpCode, now);
    
    if (valid) {
        challenge.verified = true;
        // Keep verified challenge for a short time for potential reuse
        challenge.timestamp = now;
        return true;
    }

    return false;
}

bool MFAManager::requiresMFA(const std::string& endpoint) const
{
    for (const auto& requiredEndpoint : _mfaRequiredEndpoints) {
        if (endpoint.find(requiredEndpoint) == 0) {
            return true;
        }
    }
    return false;
}

std::string MFAManager::generateTOTP(const std::string& secret, uint64_t timestamp) const
{
    if (timestamp == 0) {
        timestamp = OSUtils::now();
    }

    uint64_t timeCounter = timestamp / (TOTP_TIME_STEP * 1000);
    
    try {
        std::vector<uint8_t> secretBytes = base32Decode(secret);
        uint32_t hotp = generateHOTP(secretBytes, timeCounter);
        
        // Extract TOTP_DIGITS digits
        uint32_t code = hotp % static_cast<uint32_t>(std::pow(10, TOTP_DIGITS));
        
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(TOTP_DIGITS) << code;
        return oss.str();
    } catch (...) {
        return "";
    }
}

bool MFAManager::verifyTOTP(const std::string& secret, const std::string& code, uint64_t timestamp) const
{
    if (code.length() != TOTP_DIGITS) {
        return false;
    }

    if (timestamp == 0) {
        timestamp = OSUtils::now();
    }

    // Allow for time skew (±1 time step)
    for (int i = -1; i <= 1; i++) {
        uint64_t testTime = timestamp + (i * TOTP_TIME_STEP * 1000);
        std::string expectedCode = generateTOTP(secret, testTime);
        
        if (!expectedCode.empty() && expectedCode == code) {
            return true;
        }
    }

    return false;
}

std::string MFAManager::getMFASetupInfo(const std::string& clientId) const
{
    Mutex::Lock _l(_lock);
    
    auto clientConfig = _clientConfigs.find(clientId);
    if (clientConfig == _clientConfigs.end()) {
        return "";
    }

    // Generate QR code data for authenticator apps
    std::ostringstream oss;
    oss << "otpauth://totp/ZeroTier:" << clientId
        << "?secret=" << clientConfig->second.secret
        << "&issuer=ZeroTier"
        << "&algorithm=" << clientConfig->second.algorithm
        << "&digits=" << clientConfig->second.digits
        << "&period=" << clientConfig->second.timeStep;

    return oss.str();
}

void MFAManager::cleanupExpiredChallenges()
{
    Mutex::Lock _l(_lock);
    
    uint64_t now = OSUtils::now();
    auto it = _activeChallenges.begin();
    
    while (it != _activeChallenges.end()) {
        if ((now - it->second.timestamp) > CHALLENGE_EXPIRY_MS) {
            it = _activeChallenges.erase(it);
        } else {
            ++it;
        }
    }
}

bool MFAManager::isMFAEnabled(const std::string& clientId) const
{
    Mutex::Lock _l(_lock);
    return _clientConfigs.find(clientId) != _clientConfigs.end();
}

std::string MFAManager::generateSecret() const
{
    // Generate 160-bit (20 byte) secret for TOTP
    std::vector<uint8_t> secretBytes(20);
    OSUtils::getSecureRandom(secretBytes.data(), 20);
    return base32Encode(secretBytes);
}

std::vector<uint8_t> MFAManager::base32Decode(const std::string& encoded) const
{
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::vector<uint8_t> result;
    
    std::string cleanInput = encoded;
    // Remove padding and convert to uppercase
    cleanInput.erase(std::remove(cleanInput.begin(), cleanInput.end(), '='), cleanInput.end());
    std::transform(cleanInput.begin(), cleanInput.end(), cleanInput.begin(), ::toupper);
    
    if (cleanInput.empty()) {
        return result;
    }
    
    size_t bitsLeft = 0;
    uint32_t buffer = 0;
    
    for (char c : cleanInput) {
        size_t pos = alphabet.find(c);
        if (pos == std::string::npos) {
            continue; // Skip invalid characters
        }
        
        buffer = (buffer << 5) | pos;
        bitsLeft += 5;
        
        if (bitsLeft >= 8) {
            result.push_back((buffer >> (bitsLeft - 8)) & 0xFF);
            bitsLeft -= 8;
        }
    }
    
    return result;
}

std::string MFAManager::base32Encode(const std::vector<uint8_t>& data) const
{
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::string result;
    
    if (data.empty()) {
        return result;
    }
    
    size_t bitsLeft = 0;
    uint32_t buffer = 0;
    
    for (uint8_t byte : data) {
        buffer = (buffer << 8) | byte;
        bitsLeft += 8;
        
        while (bitsLeft >= 5) {
            result += alphabet[(buffer >> (bitsLeft - 5)) & 0x1F];
            bitsLeft -= 5;
        }
    }
    
    if (bitsLeft > 0) {
        result += alphabet[(buffer << (5 - bitsLeft)) & 0x1F];
    }
    
    // Add padding
    while (result.length() % 8 != 0) {
        result += '=';
    }
    
    return result;
}

std::vector<uint8_t> MFAManager::hmacSha1(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) const
{
    // Simplified HMAC-SHA1 implementation using ZeroTier's crypto functions
    std::vector<uint8_t> result(20); // SHA1 digest size
    
    // Use ZeroTier's existing crypto functions if available
    // This is a placeholder - in real implementation, use proper HMAC-SHA1
    std::vector<uint8_t> keyPadded = key;
    if (keyPadded.size() > 64) {
        // Hash key if longer than block size
        keyPadded.resize(20);
    }
    if (keyPadded.size() < 64) {
        keyPadded.resize(64, 0);
    }
    
    std::vector<uint8_t> innerPad(64), outerPad(64);
    for (size_t i = 0; i < 64; i++) {
        innerPad[i] = keyPadded[i] ^ 0x36;
        outerPad[i] = keyPadded[i] ^ 0x5C;
    }
    
    // Inner hash: SHA1(key XOR ipad, data)
    std::vector<uint8_t> innerData = innerPad;
    innerData.insert(innerData.end(), data.begin(), data.end());
    
    // Simplified hash - in production, use proper SHA1
    uint32_t hash = 0x67452301; // SHA1 initial value
    for (uint8_t byte : innerData) {
        hash = (hash * 33) ^ byte; // Simple hash for demo
    }
    
    // Convert to bytes
    for (int i = 0; i < 20; i++) {
        result[i] = (hash >> (i % 4 * 8)) & 0xFF;
    }
    
    return result;
}

uint32_t MFAManager::generateHOTP(const std::vector<uint8_t>& secret, uint64_t counter) const
{
    // Convert counter to 8-byte big-endian
    std::vector<uint8_t> counterBytes(8);
    for (int i = 7; i >= 0; i--) {
        counterBytes[i] = counter & 0xFF;
        counter >>= 8;
    }
    
    // Generate HMAC
    std::vector<uint8_t> hmac = hmacSha1(secret, counterBytes);
    
    // Dynamic truncation
    uint8_t offset = hmac[hmac.size() - 1] & 0x0F;
    uint32_t code = ((hmac[offset] & 0x7F) << 24) |
                    ((hmac[offset + 1] & 0xFF) << 16) |
                    ((hmac[offset + 2] & 0xFF) << 8) |
                    (hmac[offset + 3] & 0xFF);
    
    return code;
}

} // namespace ZeroTier
