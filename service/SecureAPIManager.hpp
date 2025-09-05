/*
 * Copyright (c)2025 ZeroTier, Inc.
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file in the project's root directory.
 */

#ifndef ZT_SECURE_API_MANAGER_HPP
#define ZT_SECURE_API_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include "../node/Utils.hpp"
#include "../osdep/OSUtils.hpp"

namespace ZeroTier {

/**
 * Enhanced security manager for API authentication and authorization
 */
class SecureAPIManager {
public:
    enum class SecurityLevel {
        INFO = 0,
        WARNING = 1,
        CRITICAL = 2
    };

    struct TokenInfo {
        std::string user_id;
        std::string token_hash;
        std::chrono::steady_clock::time_point created_at;
        std::chrono::steady_clock::time_point expires_at;
        std::chrono::steady_clock::time_point last_used;
        std::vector<std::string> scopes;
        uint32_t usage_count;
        bool is_revoked;
    };

    struct ClientMetrics {
        std::chrono::steady_clock::time_point last_request;
        uint32_t request_count;
        uint32_t burst_count;
        double reputation_score;
        std::vector<std::string> recent_endpoints;
        uint32_t failed_auth_count;
    };

private:
    std::unordered_map<std::string, TokenInfo> _active_tokens;
    std::unordered_map<std::string, ClientMetrics> _client_metrics;
    std::string _jwt_secret;
    uint32_t _base_rate_limit;
    std::chrono::minutes _token_lifetime;
    std::mutex _tokens_mutex;
    std::mutex _metrics_mutex;

public:
    SecureAPIManager();
    ~SecureAPIManager();

    /**
     * Authenticate HTTP request with enhanced security checks
     */
    bool authenticateRequest(const std::string& auth_header,
                           const std::string& client_ip,
                           const std::string& endpoint,
                           const std::string& required_scope);

    /**
     * Generate secure JWT token with specified scopes
     */
    std::string generateSecureToken(const std::string& user_id,
                                  const std::vector<std::string>& scopes);

    /**
     * Revoke token immediately
     */
    bool revokeToken(const std::string& token_id);

    /**
     * Check if request is allowed under rate limiting
     */
    bool allowRequest(const std::string& client_ip, 
                     const std::string& endpoint);

    /**
     * Log security event with appropriate severity
     */
    void logSecurityEvent(const std::string& event_type,
                         const std::string& details,
                         SecurityLevel level,
                         const std::string& client_ip = "");

    /**
     * Rotate tokens that are close to expiration
     */
    void rotateExpiredTokens();

    /**
     * Get client reputation score
     */
    double getClientReputation(const std::string& client_ip);

    /**
     * Get total number of security events
     */
    size_t getTotalSecurityEvents() const;

    /**
     * Get number of active tokens
     */
    size_t getActiveTokenCount() const;

    /**
     * Get rate-limited IPs
     */
    std::vector<std::string> getRateLimitedIPs() const;

private:
    std::string extractTokenFromHeader(const std::string& auth_header);
    bool validateJWTToken(const std::string& token, TokenInfo& token_info);
    bool checkPermissions(const TokenInfo& token_info, const std::string& required_scope);
    uint32_t getAdaptiveRateLimit(const std::string& client_ip, const std::string& endpoint);
    void updateClientMetrics(const std::string& client_ip, const std::string& endpoint, bool auth_success);
    std::string generateSecureRandomString(size_t length);
    void writeSecurityLog(const nlohmann::json& log_entry);
    void sendSecurityAlert(const nlohmann::json& alert_data);
    size_t _security_event_count;
};

} // namespace ZeroTier

#endif
