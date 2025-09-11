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
 * Account system for managing multiple characters per user with shared resources.      *
 ****************************************************************************************/

// Always include mud.hpp first for type definitions and macros
#pragma once
#include "mud.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <array>

// =============================================================================
// Account System Constants
// =============================================================================

constexpr const char* ACCOUNT_DIR = "../account/";

// Note: These will be defined after including mud.hpp where BV00/BV01 are defined
#define ACCOUNT_SOUND ACCOUNT_MSP
#define ACCOUNT_MSP   BV00
#define ACCOUNT_MXP   BV01

// =============================================================================
// Forward Declarations
// =============================================================================

struct account_data;
using ACCOUNT_DATA = account_data;
struct char_data;
using CharData = char_data;
struct DescriptorData;
using DescriptorData = DescriptorData;

// =============================================================================
// Global Variables
// =============================================================================

extern ACCOUNT_DATA *first_account;
extern ACCOUNT_DATA *last_account;

// =============================================================================
// Account Data Structure
// =============================================================================

struct account_data
{
        ACCOUNT_DATA *prev{nullptr};
        ACCOUNT_DATA *next{nullptr};
        struct alias_data *first_alias{nullptr};
        struct alias_data *last_alias{nullptr};
        char     *name{nullptr};
        char     *password{nullptr};
        char     *character[MaxCharacters]{nullptr};
        long     rppoints{0};        // 64-bit signed for RP points (was int)
        long     rpcurrent{-1};      // 64-bit signed for current RP character (was int)
        long     qpoints{0};         // 64-bit signed for quest points (was int)
        long     inuse{1};           // 64-bit signed usage flag (was int)
        unsigned long flags{0};      // 64-bit unsigned for bit flags (was int)
        struct note_data *comments{nullptr};
        char     *email{nullptr};
};

// =============================================================================
// Function Prototypes (C++23 modernized)
// =============================================================================

[[nodiscard]] ACCOUNT_DATA *load_account(const char *name);
[[nodiscard]] ACCOUNT_DATA *load_account(std::string_view name);
[[nodiscard]] ACCOUNT_DATA *create_account() noexcept;
void save_account(ACCOUNT_DATA *account);
[[nodiscard]] bool add_to_account(ACCOUNT_DATA *account, CharData *ch);
[[nodiscard]] bool add_to_account(std::shared_ptr<ACCOUNT_DATA> account, std::shared_ptr<CharData> ch);
[[nodiscard]] bool del_from_account(ACCOUNT_DATA *account, CharData *ch);
[[nodiscard]] bool del_from_account(std::shared_ptr<ACCOUNT_DATA> account, std::shared_ptr<CharData> ch);
void show_account_characters(DescriptorData *d);
void show_account_characters(std::shared_ptr<DescriptorData> d);
void free_account(ACCOUNT_DATA *account);
void fread_account(ACCOUNT_DATA *account, FILE *fp);
