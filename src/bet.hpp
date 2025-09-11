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
 *                            Enhanced Betting System Header                             *
 * FEATURES: Advanced number parsing with k/m notation and relative betting              *
 ****************************************************************************************/


#pragma once

#include <cctype>
#include <climits>
#include <string>
#include <string_view>

// ============================================================================
// Betting System Constants
// ============================================================================
namespace BettingSystem {
    constexpr int DefaultPercentageIncrease = 25;     // Default +% increase
    constexpr int DefaultMultiplier = 2;               // Default multiplier (x2)
    constexpr int MaxPercentage = 10000;               // Maximum percentage (100x)
    constexpr int MaxMultiplier = 1000;                // Maximum multiplier
    constexpr int OverflowSafetyDivisor = 10;         // Safety margin for overflow checks
}

// ============================================================================
// Function Declarations
// ============================================================================

/*
 * Advanced number parsing supporting k/m notation
 * Examples: 14k = 14000, 5m = 5000000, 14k42 = 14420
 * Returns 0 on invalid input or overflow conditions
 */
int advatoi(const char* s);
int advatoi(std::string_view s);

/*
 * Enhanced betting parser supporting relative and absolute bets
 * Examples: 
 *   - Absolute: 1000, 5k, 2m
 *   - Relative: +25 (add 25%), x2 (multiply by 2)
 * Returns parsed amount or 0 on invalid input
 */
int parsebet(const int currentbet, const char* s);
int parsebet(const int currentbet, std::string_view s);

