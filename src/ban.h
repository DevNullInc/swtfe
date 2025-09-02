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

#ifndef __BAN_H__
#define __BAN_H__

#include <time.h>

// ============================================================================
// Forward Declarations
// ============================================================================
typedef struct ban_data BAN_DATA;
typedef struct reserve_data RESERVE_DATA;

// ============================================================================
// Configuration Constants
// ============================================================================

// Ban types
#define BAN_SITE        1
#define BAN_CLASS       2
#define BAN_RACE        3
#define BAN_WARN        -1

// Ban levels
#define BAN_NEWBIE      1
#define BAN_MORTAL      50
#define BAN_ALL         999

// Time constants (in seconds for consistency)
#define SECONDS_PER_DAY     86400
#define SECONDS_PER_HOUR    3600
#define SECONDS_PER_MINUTE  60

// Ban duration limits (in days)
#define MIN_BAN_DURATION    1
#define MAX_BAN_DURATION    1000
#define PERMANENT_BAN       -1

// File constants
#define RESERVED_LIST       "reserved.lst"

// ============================================================================
// Data Structures
// ============================================================================

/*
 * Enhanced ban structure with proper time handling
 */
struct ban_data
{
    BAN_DATA* next;                 // Linked list pointers
    BAN_DATA* prev;
    
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
    RESERVE_DATA* next;
    RESERVE_DATA* prev;
    char* name;
};

// ============================================================================
// Global Variables
// ============================================================================
extern BAN_DATA* first_ban;
extern BAN_DATA* last_ban;
extern BAN_DATA* first_ban_class;
extern BAN_DATA* last_ban_class;
extern BAN_DATA* first_ban_race;
extern BAN_DATA* last_ban_race;

extern RESERVE_DATA* first_reserved;
extern RESERVE_DATA* last_reserved;

// ============================================================================
// Function Declarations
// ============================================================================

// Core ban management
void load_banlist(void);
void save_banlist(void);
bool check_total_bans(DESCRIPTOR_DATA* d);
bool check_bans(CHAR_DATA* ch, int type);

// Ban operations
int add_ban(CHAR_DATA* ch, char* arg1, char* arg2, int time, int type);
void show_bans(CHAR_DATA* ch, int type);
bool check_expire(BAN_DATA* ban);
void dispose_ban(BAN_DATA* ban, int type);
void free_ban(BAN_DATA* ban);

// Time utilities
time_t calculate_unban_time(int duration_days);
bool is_ban_expired(const BAN_DATA* ban);
char* format_ban_time_remaining(const BAN_DATA* ban);
char* format_ban_creation_time(const BAN_DATA* ban);

// Reserved names
void load_reserved(void);
void save_reserved(void);
bool is_reserved_name(char* name);
void sort_reserved(RESERVE_DATA* pRes);

// Command functions
CMDF do_ban(CHAR_DATA* ch, char* argument);
CMDF do_allow(CHAR_DATA* ch, char* argument);
CMDF do_warn(CHAR_DATA* ch, char* argument);
CMDF do_reserve(CHAR_DATA* ch, char* argument);

#endif /* __BAN_H__ */
