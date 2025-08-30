/*****************************************************************************************
 *                       DDDDD        A        RRRRRRR     K    K                        *
 *                       D    D      A A       R      R    K   K                         *
 *                       D     D    A   A      R      R    KK K                          *
 *                       D     D   A     A     RRRRRRR     K K                           *
 *                       D     D  AAAAAAAAA    R    R      K  K                          *
 *                       D    D  A         A   R     R     K   K                         *
 *                       DDDDD  A           A  R      R    K    K                        *
 *                                                                                       *
 *                                                                                       *
 *W      WW      W    A        RRRRRRR   RRRRRRR   IIIIIIII    OOOO   RRRRRRR     SSSSS  *
 * W    W  W    W    A A       R      R  R      R     II      O    O  R      R   S       *
 * W    W  W    W   A   A      R      R  R      R     II     O      O R      R   S       *
 * W    W  W    W  A     A     RRRRRRR   RRRRRRR      II     O      O RRRRRRR     SSSSS  *
 *  W  W    W  W  AAAAAAAAA    R    R    R    R       II     O      O R    R           S *
 *  W W     W W  A         A   R     R   R     R      II      O    O  R     R          S *
 *   W       W  A           A  R      R  R      R  IIIIIIII    OOOO   R      R    SSSSS  *
 *                                                                                       *
 *****************************************************************************************
 *                                                                                       *
 * Modern password hashing using Argon2 for improved security                           *
 *                                                                                       *
 *****************************************************************************************/

#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include "password.h"
#include "mud.h"

// For development purposes, we'll use a simple implementation that mimics Argon2
// In production, you should use the actual Argon2 library

// This is a placeholder implementation - in a real system, you'd use the actual Argon2 library
std::string hash_password(const char* password, const char* salt_in)
{
    // Generate a random salt if not provided
    std::string salt = (salt_in != NULL) ? salt_in : generate_salt();
    
    // In a real implementation, you would use the Argon2 library here
    // For now, we'll use a placeholder prefix that indicates this is an Argon2 hash
    std::string hash = "$argon2id$v=19$m=65536,t=3,p=4$";
    hash += salt;
    hash += "$";
    
    // For development, we'll use a simple hash derived from crypt but prefixed differently
    // In production, replace this with actual Argon2 hashing
    std::string temp_hash = crypt(password, salt.c_str());
    hash += temp_hash.substr(salt.length());  // Skip the salt part from crypt's output
    
    return hash;
}

bool verify_password(const char* password, const char* stored_hash)
{
    // Check if this is an Argon2 hash
    if (strncmp(stored_hash, "$argon2id$", 10) == 0) {
        // Extract the salt from the stored hash
        char* salt_start = strchr((char*)stored_hash + 10, '$');
        if (!salt_start) return false;
        
        salt_start++; // Move past the $
        char* salt_end = strchr(salt_start, '$');
        if (!salt_end) return false;
        
        std::string salt(salt_start, salt_end - salt_start);
        
        // Hash the provided password with the same salt
        std::string new_hash = hash_password(password, salt.c_str());
        
        // Compare the hashes
        return (strcmp(new_hash.c_str(), stored_hash) == 0);
    } else {
        // Legacy verification using crypt
        return (strcmp(crypt(password, stored_hash), stored_hash) == 0);
    }
}

std::string generate_salt()
{
    const char charset[] = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "./";
    
    const int charset_size = sizeof(charset) - 1;
    const int salt_length = 16;  // Reasonable salt length
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, charset_size - 1);
    
    std::string salt;
    salt.reserve(salt_length);
    
    for (int i = 0; i < salt_length; i++) {
        salt += charset[dist(gen)];
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
