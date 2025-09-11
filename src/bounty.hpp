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
using BountyData = bounty_data;
struct wanted_data;
using WantedData = wanted_data;

extern BountyData *first_bounty;
extern BountyData *last_bounty;
extern BountyData *first_disintigration;
extern BountyData *last_disintigration;

#define BountyList     "bounty.lst"
typedef enum
{ BountyPlayer, BountyPolice } bounty_types;

struct bounty_data
{
        BountyData *next;
        BountyData *prev;
        char     *target;
        long int  amount;
        int       type;
        char     *source;
};

#define StartingWanted 10
struct wanted_data
{
        WantedData *next;
        WantedData *prev;
        /*
         * This should be government instead 
         */
        ClanData *government;
        int       amount;
};

/* bounty.c */

BountyData *get_disintigration(const char *target);
BountyData *get_disintigration(std::string_view target);
BountyData *get_disintigration(std::shared_ptr<std::string> target);
void load_bounties();
void save_bounties();
void save_disintigrations();
void remove_wanted(CharData *ch, ClanData *clan);
void remove_wanted(std::shared_ptr<CharData> ch, std::shared_ptr<ClanData> clan);
void remove_wanted_planet(CharData *ch, PlanetData *planet);
void remove_wanted_planet(std::shared_ptr<CharData> ch, std::shared_ptr<PlanetData> planet);
void remove_disintigration(BountyData *bounty);
void remove_disintigration(std::shared_ptr<BountyData> bounty);
void claim_disintigration(CharData *ch, CharData *victim);
void claim_disintigration(std::shared_ptr<CharData> ch, std::shared_ptr<CharData> victim);
void add_police_bounty(CharData *ch, PlanetData *planet);
void add_police_bounty(std::shared_ptr<CharData> ch, std::shared_ptr<PlanetData> planet);
void add_wanted(CharData *ch, PlanetData *planet);
void add_wanted(std::shared_ptr<CharData> ch, std::shared_ptr<PlanetData> planet);
bool is_wanted(CharData *ch, PlanetData *pl);
bool is_wanted(std::shared_ptr<CharData> ch, std::shared_ptr<PlanetData> pl);
void fwrite_wanted(CharData *ch, FILE *fp);
void fwrite_wanted(std::shared_ptr<CharData> ch, FILE *fp);
void fread_wanted(CharData *ch, FILE *fp);
void fread_wanted(std::shared_ptr<CharData> ch, FILE *fp);

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
    std::string get_safe_bounty_target(BountyData* bounty);
    
    // Modern bounty creation (secure wrapper around legacy system)
    BountyData* create_secure_bounty(const std::string& target, long amount, 
                                     int type, const std::string& source = "");
    
    // Example function showing modern/legacy interoperability
    void display_bounty_info_secure(CharData* ch, BountyData* bounty);
    
} // namespace BountyUtils
