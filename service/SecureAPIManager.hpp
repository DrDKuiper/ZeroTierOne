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

#ifndef ZT_SECUREAPIMANAGER_HPP
#define ZT_SECUREAPIMANAGER_HPP

#include <string>

namespace ZeroTier {

/**
 * Secure API Manager for handling authentication tokens
 */
class SecureAPIManager
{
public:
	SecureAPIManager();
	~SecureAPIManager();

	/**
	 * Generate a secure authentication token
	 * 
	 * @param clientId Client identifier
	 * @param scope Token scope or permissions
	 * @param expirationTime Token expiration time in seconds
	 * @return Generated authentication token
	 */
	static std::string generateToken(const std::string& clientId, const std::string& scope, int expirationTime);

	/**
	 * Generate a secure authentication token (legacy version without parameters)
	 * 
	 * @return Generated authentication token
	 */
	static std::string generateToken();

	/**
	 * Validate an authentication token
	 * 
	 * @param token Token to validate
	 * @return True if token is valid, false otherwise
	 */
	static bool validateToken(const std::string& token);

	/**
	 * Revoke an authentication token
	 * 
	 * @param token Token to revoke
	 * @return True if token was successfully revoked
	 */
	static bool revokeToken(const std::string& token);

private:
	// Static class, no instance variables needed
};

} // namespace ZeroTier

#endif
