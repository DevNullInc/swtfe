/*****************************************************************************************
 *                      .___________. __    __   _______                                 *
 *                      |           ||  |  |  | |   ____|                                *
 *                      `---|  |----`|  |__|  | |  |__                                   *
 *                          |  |     |   __   | |   __|                                  *
 *                          |  |     |  |  |  | |  |____                                 *
 *                          |__|     |__|  |__| |_______|                                *
 *                                                                                       *
 *                _______  __  .__   __.      ___       __                               *
 *               |   ____||  | |  \ |  |     /   \     |  |                              *
 *               |  |__   |  | |   \|  |    /  ^  \    |  |                              *
 *               |   __|  |  | |  . `  |   /  /_\  \   |  |                              *
 *               |  |     |  | |  |\   |  /  _____  \  |  `----.                         *
 *               |__|     |__| |__| \__| /__/     \__\ |_______|                         *
 *                                                                                       *
 *      _______ .______    __       _______.  ______    _______   _______                *
 *     |   ____||   _  \  |  |     /       | /  __  \  |       \ |   ____|               *
 *     |  |__   |  |_)  | |  |    |   (----`|  |  |  | |  .--.  ||  |__                  *
 *     |   __|  |   ___/  |  |     \   \    |  |  |  | |  |  |  ||   __|                 *
 *     |  |____ |  |      |  | .----)   |   |  `--'  | |  '--'  ||  |____                *
 *     |_______|| _|      |__| |_______/     \______/  |_______/ |_______|               *
 *****************************************************************************************
 *                                                                                       *
 * Modern password hashing using Argon2 for improved security - /dev/null Industries     *
 * Argon2 - https://argon2-cffi.readthedocs.io/en/stable/                                *
 *  Commented fully for better understanding on how this works.                          *
 *                                                                                       *
 *****************************************************************************************/

#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <argon2.h>
#include "password.hpp"
#include "mud.hpp"

/*
 * Security Notes:
 * This implementation uses Argon2id for password hashing, which provides significantly
 * better security than traditional crypt() functions. Key advantages include:
 *
 * 1. Memory-hard algorithm: Argon2 requires substantial memory usage, making it resistant
 *    to hardware-accelerated attacks using GPUs or ASICs.
 *
 * 2. Configurable parameters: Time cost (iterations), memory cost, and parallelism can be
 *    tuned to balance security with performance based on system capabilities.
 *
 * 3. Resistance to rainbow tables: Unique salts and the algorithm's complexity make
 *    precomputed attacks impractical.
 *
 * 4. Future-proof: Argon2 was selected as the winner of the Password Hashing Competition
 *    and is designed to remain secure against emerging threats.
 *
 * 5. Migration support: Legacy crypt() hashes are automatically verified and can be
 *    upgraded to Argon2 during password changes for backward compatibility.
 *
 * Unlike basic crypt() which uses simple hashing algorithms vulnerable to brute-force
 * and dictionary attacks, Argon2 provides robust protection against modern attack vectors.
 */

namespace {
    // Argon2id configuration constants
    constexpr uint32_t ARGON2_TIME_COST = 3;        // iterations
    constexpr uint32_t ARGON2_MEMORY_COST = 1 << 16; // memory in KiB (64 MiB)
    constexpr uint32_t ARGON2_PARALLELISM = 1;       // lanes
    constexpr size_t SALT_LENGTH = 16;               // 128-bit salt
    constexpr size_t HASH_LENGTH = 32;               // 256-bit hash
    constexpr const char* ARGON2_PREFIX = "$argon2id$";
    
    /**
     * @brief Generate cryptographically secure random bytes
     * @param buffer Buffer to fill with random data
     * @param length Number of bytes to generate
     * @return true on success, false on failure
     */
    bool generate_secure_random(uint8_t* buffer, size_t length) {
        int fd = ::open("/dev/urandom", O_RDONLY);
        if (fd >= 0) {
            ssize_t bytes_read = ::read(fd, buffer, length);
            ::close(fd);
            if (bytes_read == static_cast<ssize_t>(length)) {
                return true;
            }
        }
        
        // Fallback to stdlib rand (less secure but functional)
        for (size_t i = 0; i < length; ++i) {
            buffer[i] = static_cast<uint8_t>(rand() & 0xFF);
        }
        return false;
    }
}

std::string hash_password(const char* password, const char* salt_in)
{
    if (!password) {
        password = "";
    }

    // Prepare salt bytes (if provided, use those bytes; else create random bytes)
    std::vector<uint8_t> salt_bytes;
    if (salt_in && *salt_in) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(salt_in);
        salt_bytes.assign(p, p + strlen(salt_in));
    } else {
        salt_bytes.resize(SALT_LENGTH);
        generate_secure_random(salt_bytes.data(), SALT_LENGTH);
    }

    // Compute required encoded length and allocate string buffer
    size_t encoded_len = argon2_encodedlen(ARGON2_TIME_COST, ARGON2_MEMORY_COST, 
                                           ARGON2_PARALLELISM, salt_bytes.size(), 
                                           HASH_LENGTH, Argon2_id);
    std::string encoded;
    encoded.resize(encoded_len);

    int rc = argon2id_hash_encoded(ARGON2_TIME_COST, ARGON2_MEMORY_COST, ARGON2_PARALLELISM,
                                   password, strlen(password),
                                   salt_bytes.data(), salt_bytes.size(),
                                   HASH_LENGTH,
                                   &encoded[0], encoded_len);
    if (rc != ARGON2_OK) {
        bug("argon2id_hash_encoded failed: %s", argon2_error_message(rc));
        return std::string();
    }

    // Resize to actual length (encoded is NUL-terminated C string)
    encoded.resize(strlen(encoded.c_str()));
    return encoded;
}

bool verify_password(const char* password, const char* stored_hash)
{
    if (!password || !stored_hash) {
        return false;
    }

    // Check if this is an Argon2 hash
    if (strncmp(stored_hash, ARGON2_PREFIX, strlen(ARGON2_PREFIX)) == 0) {
        int rc = argon2id_verify(stored_hash, password, strlen(password));
        return rc == ARGON2_OK;
    } else {
        // Legacy verification using crypt
        const char* encrypted = crypt(password, stored_hash);
        return encrypted && (strcmp(encrypted, stored_hash) == 0);
    }
}

std::string generate_salt()
{
    std::string salt;
    salt.resize(SALT_LENGTH);
    generate_secure_random(reinterpret_cast<uint8_t*>(&salt[0]), SALT_LENGTH);
    return salt;
}

std::string migrate_password(const char* password, const char* old_hash)
{
    if (!password || !old_hash) {
        return old_hash ? old_hash : "";
    }

    // First verify the password against the old hash
    const char* encrypted = crypt(password, old_hash);
    if (!encrypted || strcmp(encrypted, old_hash) != 0) {
        // Password doesn't match the old hash, return unchanged
        return old_hash;
    }
    
    // Create a new Argon2 hash
    return hash_password(password);
}

// Utility function to check if we should upgrade a password hash
bool should_upgrade_hash(const char* hash)
{
    // If it doesn't start with the Argon2 prefix, it needs to be upgraded
    return !hash || strncmp(hash, ARGON2_PREFIX, strlen(ARGON2_PREFIX)) != 0;
}
