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
#include <ctime>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <argon2.h>
#include "password.h"
#include "mud.h"

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

std::string hash_password(const char* password, const char* salt_in)
{
    // Argon2id default parameters (tweak as needed)
    const uint32_t t_cost = 3;        // iterations
    const uint32_t m_cost = 1 << 16;  // memory in KiB (64 MiB)
    const uint32_t parallelism = 1;   // lanes
    const size_t salt_len_default = 16; // 128-bit salt
    const size_t hash_len = 32;       // 256-bit hash

    // Prepare salt bytes (if provided, use those bytes; else create random bytes)
    std::vector<uint8_t> salt_bytes;
    if (salt_in && *salt_in) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(salt_in);
        salt_bytes.assign(p, p + strlen(salt_in));
    } else {
        salt_bytes.resize(salt_len_default);
        int fd = ::open("/dev/urandom", O_RDONLY);
        if (fd >= 0) {
            ssize_t rd = ::read(fd, salt_bytes.data(), salt_bytes.size());
            ::close(fd);
            if (rd != (ssize_t)salt_bytes.size()) {
                for (size_t i = 0; i < salt_bytes.size(); ++i)
                    salt_bytes[i] = static_cast<uint8_t>(rand() & 0xFF);
            }
        } else {
            for (size_t i = 0; i < salt_bytes.size(); ++i)
                salt_bytes[i] = static_cast<uint8_t>(rand() & 0xFF);
        }
    }

    // Compute required encoded length and allocate string buffer
    size_t encoded_len = argon2_encodedlen(t_cost, m_cost, parallelism,
                                           salt_bytes.size(), hash_len, Argon2_id);
    std::string encoded;
    encoded.resize(encoded_len);

    int rc = argon2id_hash_encoded(t_cost, m_cost, parallelism,
                                   password ? password : "",
                                   password ? strlen(password) : 0,
                                   salt_bytes.data(), salt_bytes.size(),
                                   hash_len,
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
    // Check if this is an Argon2 hash
    if (strncmp(stored_hash, "$argon2id$", 10) == 0) {
        int rc = argon2id_verify(stored_hash,
                                 password ? password : "",
                                 password ? strlen(password) : 0);
        return rc == ARGON2_OK;
    } else {
        // Legacy verification using crypt
        return (strcmp(crypt(password, stored_hash), stored_hash) == 0);
    }
}

std::string generate_salt()
{
    // Produce 16 random bytes; argon2id_hash_encoded will take raw salt and encode it
    const size_t salt_len = 16;
    std::string salt;
    salt.resize(salt_len);
    int fd = ::open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t rd = ::read(fd, &salt[0], salt_len);
        ::close(fd);
        if (rd != (ssize_t)salt_len) {
            for (size_t i = 0; i < salt_len; ++i)
                salt[i] = static_cast<char>(rand() & 0xFF);
        }
    } else {
        for (size_t i = 0; i < salt_len; ++i)
            salt[i] = static_cast<char>(rand() & 0xFF);
    }
    return salt;
}

std::string migrate_password(const char* password, const char* old_hash)
{
    // First verify the password against the old hash
    if (strcmp(crypt(password, old_hash), old_hash) != 0) {
        // Password doesn't match the old hash
        return old_hash;
    }
    
    // Create a new Argon2 hash
    return hash_password(password);
}

// Utility function to check if we should upgrade a password hash
bool should_upgrade_hash(const char* hash)
{
    // If it doesn't start with the Argon2 prefix, it needs to be upgraded
    return (strncmp(hash, "$argon2id$", 10) != 0);
}
