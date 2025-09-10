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
 * Star Wars: The Final Episode additions and changes from the Star Wars Reality code    *
 * copyright (c) 2025 /dev/null Industries - StygianRenegade                             *
 *                                                                                       *
 * Star Wars Reality Code Additions and changes from the Smaug Code copyright (c) 1997   *
 * by Sean Cooper                                                                        *
 *                                                                                       *
 * Starwars and Starwars Names copyright(c) Lucas Film Ltd.                              *
 *****************************************************************************************
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                                Utility macros                                         *
 *****************************************************************************************
 * Modernized Utility Header File for Game Operations                                   *
 ****************************************************************************************/
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <iostream>
#include <stdexcept>

namespace utils {

// Min and Max
constexpr auto umin(auto a, auto b) { return (a < b) ? a : b; }
constexpr auto umax(auto a, auto b) { return (a > b) ? a : b; }

// Range
constexpr auto urange(auto a, auto b, auto c) { return (b < a) ? a : ((b > c) ? c : b); }

// Character Case Conversion
constexpr char to_lower(char c) { return (c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c; }
constexpr char to_upper(char c) { return (c >= 'a' && c <= 'z') ? (c + 'A' - 'a') : c; }

// String Utilities
inline bool is_null_or_empty(const std::string_view str) { return str.empty(); }

// Memory Management
template <typename T, typename... Args>
std::unique_ptr<T> create_unique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
std::shared_ptr<T> create_shared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// Linked List Utilities
// Use std::list or std::vector instead of manual linked list management

// Logging Utility
inline void log_error(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}

inline void log_debug(const std::string& message) {
    std::cout << "Debug: " << message << std::endl;
}

// Conversion Helpers
constexpr short int_to_shint(int value) {
    if (value < std::numeric_limits<short>::min() || value > std::numeric_limits<short>::max()) {
        throw std::out_of_range("Value out of range for short int");
    }
    return static_cast<short>(value);
}

} // namespace utils
