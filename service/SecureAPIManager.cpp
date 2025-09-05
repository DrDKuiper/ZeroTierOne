/*
 * Copyright (c)2013-2020 ZeroTier, Inc.
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file in the project's root directory.
 *
 * Change Date: 2026-01-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2.0 of the Apache License.
 */

#include "SecureAPIManager.hpp"
#include "../node/Utils.hpp"
#include "../osdep/OSUtils.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace ZeroTier {

SecureAPIManager::SecureAPIManager()
{
}

SecureAPIManager::~SecureAPIManager()
{
}

std::string SecureAPIManager::generateToken(const std::string& clientId, const std::string& scope, int expirationTime)
{
	// Generate a secure random token
	uint8_t randomBytes[32];
	Utils::getSecureRandom(randomBytes, sizeof(randomBytes));
	
	// Convert to hex string
	char tokenHex[65];
	Utils::hex(randomBytes, sizeof(randomBytes), tokenHex);
	
	// Create a structured token with metadata
	std::ostringstream tokenStream;
	tokenStream << clientId << "_" << scope << "_" << expirationTime << "_" << std::string(tokenHex);
	
	return tokenStream.str();
}

std::string SecureAPIManager::generateToken()
{
	// Generate a simple secure random token for legacy compatibility
	uint8_t randomBytes[24];
	Utils::getSecureRandom(randomBytes, sizeof(randomBytes));
	
	char tokenHex[49];
	Utils::hex(randomBytes, sizeof(randomBytes), tokenHex);
	
	return std::string(tokenHex);
}

bool SecureAPIManager::validateToken(const std::string& token)
{
	// Basic validation - check if token is not empty and has reasonable length
	if (token.empty() || token.length() < 16) {
		return false;
	}
	
	// Additional validation logic could be added here
	// For now, we'll accept any non-empty token with sufficient length
	return true;
}

bool SecureAPIManager::revokeToken(const std::string& token)
{
	// Token revocation logic would go here
	// For now, we'll just return true to indicate success
	// In a real implementation, this would remove the token from a blacklist/database
	return true;
}

} // namespace ZeroTier
