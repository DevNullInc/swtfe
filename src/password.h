#ifndef _PASSWORD_H_
#define _PASSWORD_H_

#include <string>

/**
 * @file password.h
 * @brief Modern Argon2-based password hashing for enhanced security
 * 
 * This module provides secure password hashing using Argon2id, which offers
 * significant security improvements over traditional crypt() functions:
 * - Memory-hard algorithm resistant to GPU/ASIC attacks
 * - Configurable time/memory costs for performance tuning
 * - Automatic salt generation and management
 * - Backward compatibility with legacy crypt() hashes
 */

/**
 * @brief Hash a password using Argon2id
 * @param password The password to hash (required)
 * @param salt Optional salt bytes; if NULL, generates random salt
 * @return Encoded Argon2id hash string, or empty string on failure
 */
std::string hash_password(const char* password, const char* salt = nullptr);

/**
 * @brief Verify a password against a stored hash
 * @param password The password to verify
 * @param stored_hash The stored hash (Argon2 or legacy crypt)
 * @return true if password matches, false otherwise
 */
bool verify_password(const char* password, const char* stored_hash);

/**
 * @brief Generate a cryptographically secure random salt
 * @return 16 bytes of random salt data
 */
std::string generate_salt();

/**
 * @brief Migrate a legacy crypt hash to Argon2
 * @param password The original password
 * @param old_hash The legacy hash to migrate from
 * @return New Argon2 hash if migration successful, original hash otherwise
 */
std::string migrate_password(const char* password, const char* old_hash);

/**
 * @brief Check if a hash should be upgraded to Argon2
 * @param hash The hash to check
 * @return true if hash needs upgrading, false if already Argon2
 */
bool should_upgrade_hash(const char* hash);

#endif /* _PASSWORD_H_ */
