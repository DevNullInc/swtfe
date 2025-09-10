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
 *                                SWR Bounty module                                      *
 ****************************************************************************************/

// Modern C++ includes for incremental modernization
#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <memory>

struct bounty_data;
using BOUNTY_DATA = bounty_data;
struct wanted_data;
using WANTED_DATA = wanted_data;

extern BOUNTY_DATA *first_bounty;
extern BOUNTY_DATA *last_bounty;
extern BOUNTY_DATA *first_disintigration;
extern BOUNTY_DATA *last_disintigration;

#define BOUNTY_LIST     "bounty.lst"
typedef enum
{ BOUNTY_PLAYER, BOUNTY_POLICE } bounty_types;

struct bounty_data
{
        BOUNTY_DATA *next;
        BOUNTY_DATA *prev;
        char     *target;
        long int  amount;
        int       type;
        char     *source;
};

#define STARTING_WANTED 10
struct wanted_data
{
        WANTED_DATA *next;
        WANTED_DATA *prev;
        /*
         * This should be government instead 
         */
        CLAN_DATA *government;
        int       amount;
};

/* bounty.c */

BOUNTY_DATA *get_disintigration(const char *target);
BOUNTY_DATA *get_disintigration(std::string_view target);
BOUNTY_DATA *get_disintigration(std::shared_ptr<std::string> target);
void load_bounties();
void save_bounties();
void save_disintigrations();
void remove_wanted(CHAR_DATA *ch, CLAN_DATA *clan);
void remove_wanted(std::shared_ptr<CHAR_DATA> ch, std::shared_ptr<CLAN_DATA> clan);
void remove_wanted_planet(CHAR_DATA *ch, PLANET_DATA *planet);
void remove_wanted_planet(std::shared_ptr<CHAR_DATA> ch, std::shared_ptr<PLANET_DATA> planet);
void remove_disintigration(BOUNTY_DATA *bounty);
void remove_disintigration(std::shared_ptr<BOUNTY_DATA> bounty);
void claim_disintigration(CHAR_DATA *ch, CHAR_DATA *victim);
void claim_disintigration(std::shared_ptr<CHAR_DATA> ch, std::shared_ptr<CHAR_DATA> victim);
void add_police_bounty(CHAR_DATA *ch, PLANET_DATA *planet);
void add_police_bounty(std::shared_ptr<CHAR_DATA> ch, std::shared_ptr<PLANET_DATA> planet);
void add_wanted(CHAR_DATA *ch, PLANET_DATA *planet);
void add_wanted(std::shared_ptr<CHAR_DATA> ch, std::shared_ptr<PLANET_DATA> planet);
bool is_wanted(CHAR_DATA *ch, PLANET_DATA *pl);
bool is_wanted(std::shared_ptr<CHAR_DATA> ch, std::shared_ptr<PLANET_DATA> pl);
void fwrite_wanted(CHAR_DATA *ch, FILE *fp);
void fwrite_wanted(std::shared_ptr<CHAR_DATA> ch, FILE *fp);
void fread_wanted(CHAR_DATA *ch, FILE *fp);
void fread_wanted(std::shared_ptr<CHAR_DATA> ch, FILE *fp);

// ============================================================================
// Modern C++ Bounty Utilities (Incremental Modernization)
// ============================================================================
namespace BountyUtils {
    // Modern wrapper functions that delegate to legacy implementation
    // These provide type-safe, secure alternatives to legacy functions
    
    // String-safe bounty target checking
    inline bool is_target_valid(const std::string& target) {
        return !target.empty() && target.length() < 256; // Prevent buffer overflows
    }
    
    // Safe amount validation
    inline bool is_amount_valid(long amount) {
        return amount > 0 && amount <= 2000000000L; // Reasonable upper limit
    }
    
    // Modern string conversion helpers
    std::string legacy_to_string(const char* legacy_str);
    const char* string_to_legacy(const std::string& modern_str);
    
    // Modern bounty safety functions
    bool is_bounty_target_safe(const char* target);
    long sanitize_bounty_amount(long amount);
    std::string get_safe_bounty_target(BOUNTY_DATA* bounty);
    
    // Modern bounty creation (secure wrapper around legacy system)
    BOUNTY_DATA* create_secure_bounty(const std::string& target, long amount, 
                                     int type, const std::string& source = "");
    
    // Example function showing modern/legacy interoperability
    void display_bounty_info_secure(CHAR_DATA* ch, BOUNTY_DATA* bounty);
    
} // namespace BountyUtils
