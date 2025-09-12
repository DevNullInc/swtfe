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
 * Modernized for C++23 by Gemini 2.5 Pro for SMAUG style MUDs.                          *
 * Original copyrights respectfully retained.                                            *
 *                                                                                       *
 *****************************************************************************************
 *                        Modernized Utility Header File                                 *
 ****************************************************************************************/
#pragma once // Use include guards instead of managing them manually

#include <algorithm>
#include <bit> // For bit manipulation in C++20+
#include <cctype>
#include <cstdio> // C++ header for stdio.h
#include <iostream>
#include <list>
#include <memory>
#include <source_location> // C++20 feature for logging source info
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits> // For advanced template metaprogramming
#include <vector>
#include "account.hpp"

// Forward declare common MUD types to allow utils to have the correct signatures.
struct CharData;
struct DescriptorData;

// Declare MUD-specific logging functions.
void bug(const char *str, ...);
void log_string(const char *str);

namespace utils
{
    // --- Basic Math and Character Utilities ---

    template <typename T>
    constexpr T min(const T &a, const T &b) { return (a < b) ? a : b; }

    template <typename T>
    constexpr T max(const T &a, const T &b) { return (a > b) ? a : b; }

    template <typename T>
    constexpr T range(const T &a, const T &b, const T &c)
    {
        return (b < a) ? a : ((b > c) ? c : b);
    }

    constexpr char to_lower(char c)
    {
        if (c >= 'A' && c <= 'Z')
            return c + ('a' - 'A');
        return c;
    }

    constexpr char to_upper(char c)
    {
        if (c >= 'a' && c <= 'z')
            return c + ('A' - 'a');
        return c;
    }

    // --- String Utilities ---

    inline bool is_null_or_empty(const char *str) { return !str || str[0] == '\0'; }
    inline bool is_null_or_empty(const std::string &str) { return str.empty(); }
    inline bool is_null_or_empty(std::string_view str) { return str.empty(); }

    // --- Compile-time Array Item Counter ---

    template <typename T, std::size_t N>
    constexpr std::size_t array_size(const T (&)[N]) noexcept { return N; }

    // --- Type-Safe Bitwise Operations (replaces IsSet, SetBit, etc.) ---

    template <std::integral T, std::integral U>
    constexpr bool is_set(T flag, U bit)
    {
        return (flag & bit) == bit;
    }

    template <std::integral T, std::integral U>
    constexpr void set_bit(T &var, U bit)
    {
        var |= bit;
    }

    template <std::integral T, std::integral U>
    constexpr void remove_bit(T &var, U bit)
    {
        var &= ~bit;
    }

    template <std::integral T, std::integral U>
    constexpr void toggle_bit(T &var, U bit)
    {
        var ^= bit;
    }

    // --- Safe File Handling ---

    inline void safe_fclose(FILE *&fp, const std::source_location &loc = std::source_location::current())
    {
        if (fp)
        {
            fclose(fp);
            fp = nullptr;
        }
        else
        {
            bug("Trying to fclose a null file pointer at %s:%d", loc.file_name(), loc.line());
        }
    }
} // namespace utils
