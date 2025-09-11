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
 * This module was originally for SMAUG coded by Shaddai, but has since been modified for* 
 * SWTFE with enhanced time handling and security features.                               *
 *****************************************************************************************
 *                            Ban System Header                                          *
 ****************************************************************************************/


#pragma once

#include <ctime>
#include <string>
#include <string_view>
#include <memory>

// ============================================================================
// Forward Declarations
// ============================================================================
struct ban_data;
using BanData = ban_data;
struct reserve_data;
using ReserveData = reserve_data;

// ============================================================================
// Configuration Constants
// ============================================================================

// Ban types
#define BanSite        1
#define BanClass       2
#define BanRace        3
#define BanWarn        -1

// Ban levels
#define BanNewbie      1
#define BanMortal      50
#define BanAll         999

// Time constants (in seconds for consistency)
#define SecondsPerDay     86400
#define SecondsPerHour    3600
#define SecondsPerMinute  60

// Ban duration limits (in days)
#define MinBanDuration    1
#define MaxBanDuration    1000
#define PermanentBan       -1

// File constants
#define ReservedList       "reserved.lst"

// ============================================================================
// Data Structures
// ============================================================================

/*
 * Enhanced ban structure with proper time handling
 */
struct ban_data
{
    BanData* next;                 // Linked list pointers
    BanData* prev;
    
    // Ban target information
    char* name;                     // Name of site/class/race banned
    char* user;                     // Name of user from site (for site bans)
    int flag;                       // Class or Race number
    
    // Ban configuration
    int level;                      // Level that is banned
    bool warn;                      // Echo on warn channel
    bool prefix;                    // Use of *site wildcard
    bool suffix;                    // Use of site* wildcard
    
    // Time management (using time_t for consistency)
    time_t ban_time;                // When ban was created (FIXED: was char*)
    time_t unban_date;              // When ban expires (FIXED: was int)
    int duration_days;              // Original duration in days (FIXED: was sh_int)
    
    // Administrative information
    char* ban_by;                   // Who banned this site
    char* note;                     // Why it was banned
};

/*
 * Reserved name structure
 */
struct reserve_data
{
    ReserveData* next;
    ReserveData* prev;
    char* name;
};

// ============================================================================
// Global Variables
// ============================================================================
extern BanData* first_ban;
extern BanData* last_ban;
extern BanData* first_ban_class;
extern BanData* last_ban_class;
extern BanData* first_ban_race;
extern BanData* last_ban_race;

extern ReserveData* first_reserved;
extern ReserveData* last_reserved;

// ============================================================================
// Function Declarations
// ============================================================================

// Core ban management
void load_banlist();
void save_banlist();
bool check_total_bans(DescriptorData* d);
bool check_total_bans(std::shared_ptr<DescriptorData> d);
bool check_bans(CharData* ch, int type);
bool check_bans(std::shared_ptr<CharData> ch, int type);

// Ban operations
int add_ban(CharData* ch, const char* arg1, const char* arg2, int time, int type);
int add_ban(CharData* ch, std::string_view arg1, std::string_view arg2, int time, int type);
int add_ban(std::shared_ptr<CharData> ch, std::string_view arg1, std::string_view arg2, int time, int type);
void show_bans(CharData* ch, int type);
void show_bans(std::shared_ptr<CharData> ch, int type);
bool check_expire(BanData* ban);
void dispose_ban(BanData* ban, int type);
void free_ban(BanData* ban);

// Time utilities
std::time_t calculate_unban_time(int duration_days);
bool is_ban_expired(const BanData* ban);
std::string format_ban_time_remaining(const BanData* ban);
std::string format_ban_creation_time(const BanData* ban);

// Reserved names
void load_reserved();
void save_reserved();
bool is_reserved_name(const char* name);
bool is_reserved_name(std::string_view name);
void sort_reserved(ReserveData* pRes);
void sort_reserved(std::shared_ptr<ReserveData> pRes);

// Command functions
CMDF do_ban(CharData* ch, const char* argument);
CMDF do_ban(CharData* ch, std::string_view argument);
CMDF do_ban(std::shared_ptr<CharData> ch, std::string_view argument);
CMDF do_allow(CharData* ch, const char* argument);
CMDF do_allow(CharData* ch, std::string_view argument);
CMDF do_allow(std::shared_ptr<CharData> ch, std::string_view argument);
CMDF do_warn(CharData* ch, const char* argument);
CMDF do_warn(CharData* ch, std::string_view argument);
CMDF do_warn(std::shared_ptr<CharData> ch, std::string_view argument);
CMDF do_reserve(CharData* ch, const char* argument);
CMDF do_reserve(CharData* ch, std::string_view argument);
CMDF do_reserve(std::shared_ptr<CharData> ch, std::string_view argument);
