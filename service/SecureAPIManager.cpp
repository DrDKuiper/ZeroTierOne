/*
 * Copyright (c)2025 ZeroTier, Inc.
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file in the project's root directory.
 */

#include "SecureAPIManager.hpp"
#include "../node/SHA.hpp"
#include "../node/Utils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

namespace ZeroTier {

SecureAPIManager::SecureAPIManager() 
    : _base_rate_limit(100)
    , _token_lifetime(std::chrono::hours(24))
{
    // Generate secure JWT secret on startup
    uint8_t random_bytes[32];
    Utils::getSecureRandom(random_bytes, sizeof(random_bytes));
    _jwt_secret = Utils::hex(random_bytes, sizeof(random_bytes));
}

SecureAPIManager::~SecureAPIManager() {
    // Clean up any resources
}

bool SecureAPIManager::authenticateRequest(const std::string& auth_header,
                                         const std::string& client_ip,
                                         const std::string& endpoint,
                                         const std::string& required_scope) {
    
    // First check rate limiting
    if (!allowRequest(client_ip, endpoint)) {
        logSecurityEvent("rate_limit_exceeded", 
                        "Client exceeded rate limit for endpoint: " + endpoint,
                        SecurityLevel::WARNING, client_ip);
        return false;
    }
    
    // Extract and validate token
    std::string token = extractTokenFromHeader(auth_header);
    if (token.empty()) {
        updateClientMetrics(client_ip, endpoint, false);
        logSecurityEvent("missing_auth_token", 
                        "Request without authentication token",
                        SecurityLevel::WARNING, client_ip);
        return false;
    }
    
    TokenInfo token_info;
    if (!validateJWTToken(token, token_info)) {
        updateClientMetrics(client_ip, endpoint, false);
        logSecurityEvent("invalid_token", 
                        "Invalid or expired authentication token",
                        SecurityLevel::WARNING, client_ip);
        return false;
    }
    
    // Check permissions
    if (!checkPermissions(token_info, required_scope)) {
        updateClientMetrics(client_ip, endpoint, false);
        logSecurityEvent("insufficient_permissions", 
                        "Token lacks required scope: " + required_scope,
                        SecurityLevel::WARNING, client_ip);
        return false;
    }
    
    // Update token usage
    {
        std::lock_guard<std::mutex> lock(_tokens_mutex);
        auto it = _active_tokens.find(token_info.token_hash);
        if (it != _active_tokens.end()) {
            it->second.last_used = std::chrono::steady_clock::now();
            it->second.usage_count++;
        }
    }
    
    updateClientMetrics(client_ip, endpoint, true);
    logSecurityEvent("successful_auth", 
                    "Authenticated request for endpoint: " + endpoint,
                    SecurityLevel::INFO, client_ip);
    
    return true;
}

std::string SecureAPIManager::generateSecureToken(const std::string& user_id,
                                                const std::vector<std::string>& scopes) {
    
    auto now = std::chrono::steady_clock::now();
    auto expires = now + _token_lifetime;
    
    // Create JWT header
    nlohmann::json header = {
        {"alg", "HS256"},
        {"typ", "JWT"}
    };
    
    // Create JWT payload
    nlohmann::json payload = {
        {"sub", user_id},
        {"iat", std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count()},
        {"exp", std::chrono::duration_cast<std::chrono::seconds>(expires.time_since_epoch()).count()},
        {"scopes", scopes},
        {"jti", generateSecureRandomString(16)} // JWT ID for revocation
    };
    
    // Base64 encode header and payload
    std::string header_b64 = Utils::base64Encode(header.dump());
    std::string payload_b64 = Utils::base64Encode(payload.dump());
    
    // Create signature
    std::string to_sign = header_b64 + "." + payload_b64;
    uint8_t signature_bytes[32];
    HMACSHA256(_jwt_secret.data(), _jwt_secret.length(),
               to_sign.data(), to_sign.length(),
               signature_bytes);
    std::string signature_b64 = Utils::base64Encode(signature_bytes, sizeof(signature_bytes));
    
    std::string token = to_sign + "." + signature_b64;
    
    // Store token info
    TokenInfo token_info;
    token_info.user_id = user_id;
    token_info.token_hash = Utils::sha256(token);
    token_info.created_at = now;
    token_info.expires_at = expires;
    token_info.last_used = now;
    token_info.scopes = scopes;
    token_info.usage_count = 0;
    token_info.is_revoked = false;
    
    {
        std::lock_guard<std::mutex> lock(_tokens_mutex);
        _active_tokens[token_info.token_hash] = token_info;
    }
    
    logSecurityEvent("token_generated", 
                    "Generated new token for user: " + user_id,
                    SecurityLevel::INFO);
    
    return token;
}

bool SecureAPIManager::allowRequest(const std::string& client_ip, 
                                  const std::string& endpoint) {
    
    std::lock_guard<std::mutex> lock(_metrics_mutex);
    auto now = std::chrono::steady_clock::now();
    auto& metrics = _client_metrics[client_ip];
    
    // Reset counters if enough time has passed
    if (now - metrics.last_request >= std::chrono::minutes(1)) {
        metrics.request_count = 0;
        metrics.burst_count = 0;
    }
    
    // Get adaptive rate limit
    uint32_t limit = getAdaptiveRateLimit(client_ip, endpoint);
    
    // Check if over limit
    if (metrics.request_count >= limit) {
        metrics.burst_count++;
        
        // Exponential backoff for repeated violations
        if (metrics.burst_count > 5) {
            metrics.reputation_score = std::max(0.1, metrics.reputation_score * 0.8);
        }
        
        return false;
    }
    
    // Update metrics
    metrics.request_count++;
    metrics.last_request = now;
    metrics.recent_endpoints.push_back(endpoint);
    
    // Keep only recent endpoints (last 10)
    if (metrics.recent_endpoints.size() > 10) {
        metrics.recent_endpoints.erase(metrics.recent_endpoints.begin());
    }
    
    return true;
}

double SecureAPIManager::getClientReputation(const std::string& client_ip) {
    std::lock_guard<std::mutex> lock(_metrics_mutex);
    auto it = _client_metrics.find(client_ip);
    if (it != _client_metrics.end()) {
        return it->second.reputation_score;
    }
    return 1.0; // Default good reputation
}

size_t SecureAPIManager::getSecurityEventCount() const {
    return _security_event_count;
}

size_t SecureAPIManager::getActiveTokenCount() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(_tokens_mutex));
    return _active_tokens.size();
}

size_t SecureAPIManager::getRateLimitedIPsCount() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(_metrics_mutex));
    size_t count = 0;
    for (const auto& pair : _client_metrics) {
        if (pair.second.reputation_score < 0.5) { // Example threshold for being "rate-limited"
            count++;
        }
    }
    return count;
}

void SecureAPIManager::logSecurityEvent(const std::string& event_type,
                                      const std::string& details,
                                      SecurityLevel level,
                                      const std::string& client_ip) {
    
    nlohmann::json log_entry = {
        {"timestamp", OSUtils::now()},
        {"event_type", event_type},
        {"severity", static_cast<int>(level)},
        {"details", details},
        {"client_ip", client_ip},
        {"node_id", "local"}, // Should be actual node ID
        {"process_id", OSUtils::getProcessId()}
    };
    
    _security_event_count++;

    // Write to security log
    writeSecurityLog(log_entry);
    
    // Send alert for critical events
    if (level == SecurityLevel::CRITICAL) {
        sendSecurityAlert(log_entry);
    }
}

void SecureAPIManager::rotateExpiredTokens() {
    std::lock_guard<std::mutex> lock(_tokens_mutex);
    auto now = std::chrono::steady_clock::now();
    
    auto it = _active_tokens.begin();
    while (it != _active_tokens.end()) {
        if (it->second.expires_at < now || it->second.is_revoked) {
            logSecurityEvent("token_expired", 
                           "Removed expired/revoked token for user: " + it->second.user_id,
                           SecurityLevel::INFO);
            it = _active_tokens.erase(it);
        } else {
            ++it;
        }
    }
}

std::string SecureAPIManager::extractTokenFromHeader(const std::string& auth_header) {
    const std::string bearer_prefix = "Bearer ";
    if (auth_header.length() > bearer_prefix.length() && 
        auth_header.substr(0, bearer_prefix.length()) == bearer_prefix) {
        return auth_header.substr(bearer_prefix.length());
    }
    return "";
}

bool SecureAPIManager::validateJWTToken(const std::string& token, TokenInfo& token_info) {
    // Simple JWT validation (in production, use a proper JWT library)
    std::string token_hash = Utils::sha256(token);
    
    std::lock_guard<std::mutex> lock(_tokens_mutex);
    auto it = _active_tokens.find(token_hash);
    if (it == _active_tokens.end()) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    if (it->second.expires_at < now || it->second.is_revoked) {
        return false;
    }
    
    token_info = it->second;
    return true;
}

bool SecureAPIManager::checkPermissions(const TokenInfo& token_info, 
                                       const std::string& required_scope) {
    if (required_scope.empty()) {
        return true; // No specific scope required
    }
    
    return std::find(token_info.scopes.begin(), token_info.scopes.end(), required_scope) 
           != token_info.scopes.end();
}

uint32_t SecureAPIManager::getAdaptiveRateLimit(const std::string& client_ip, 
                                               const std::string& endpoint) {
    uint32_t base = _base_rate_limit;
    
    // Adjust based on endpoint cost
    if (endpoint.find("/peer") != std::string::npos) base /= 10;      // Expensive peer queries
    if (endpoint.find("/network") != std::string::npos) base /= 5;    // Network operations
    if (endpoint.find("/controller") != std::string::npos) base /= 2; // Controller operations
    
    // Adjust based on client reputation
    auto metrics_it = _client_metrics.find(client_ip);
    if (metrics_it != _client_metrics.end()) {
        base = static_cast<uint32_t>(base * metrics_it->second.reputation_score);
    }
    
    return std::max(1u, base); // Minimum 1 request allowed
}

void SecureAPIManager::updateClientMetrics(const std::string& client_ip, 
                                          const std::string& endpoint, 
                                          bool auth_success) {
    std::lock_guard<std::mutex> lock(_metrics_mutex);
    auto& metrics = _client_metrics[client_ip];
    
    if (!auth_success) {
        metrics.failed_auth_count++;
        // Decrease reputation for failed auth
        metrics.reputation_score = std::max(0.1, metrics.reputation_score - 0.1);
    } else {
        // Slowly improve reputation for successful requests
        metrics.reputation_score = std::min(1.0, metrics.reputation_score + 0.01);
    }
    
    // Initialize reputation score if not set
    if (metrics.reputation_score == 0.0) {
        metrics.reputation_score = 1.0;
    }
}

std::string SecureAPIManager::generateSecureRandomString(size_t length) {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.reserve(length);
    
    uint8_t random_bytes[length];
    Utils::getSecureRandom(random_bytes, length);
    
    for (size_t i = 0; i < length; ++i) {
        result += chars[random_bytes[i] % chars.length()];
    }
    
    return result;
}

void SecureAPIManager::writeSecurityLog(const nlohmann::json& log_entry) {
    // Write to rotating log file
    std::ofstream log_file("security.log", std::ios::app);
    if (log_file.is_open()) {
        log_file << log_entry.dump() << std::endl;
        log_file.close();
    }
}

void SecureAPIManager::sendSecurityAlert(const nlohmann::json& alert_data) {
    // In production, this would send to SIEM/monitoring system
    // For now, just log at higher severity
    std::ofstream alert_file("security_alerts.log", std::ios::app);
    if (alert_file.is_open()) {
        alert_file << "SECURITY ALERT: " << alert_data.dump() << std::endl;
        alert_file.close();
    }
}

} // namespace ZeroTier
