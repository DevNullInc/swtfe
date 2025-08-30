#ifndef _PASSWORD_H_
#define _PASSWORD_H_

#include <string>

/*
 * Argon2 password hashing functions for improved security
 */

// Hashes a password using Argon2id
std::string hash_password(const char* password, const char* salt = NULL);

// Verifies a password against a stored hash
bool verify_password(const char* password, const char* stored_hash);

// Generate a random salt
std::string generate_salt();

// Migration function to convert old crypt hashes to Argon2
std::string migrate_password(const char* password, const char* old_hash);

#endif /* _PASSWORD_H_ */
