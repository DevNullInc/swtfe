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
 *		                Bounty Hunter Module    		                 *   
 *                        (  and area capturing as well  )                               * 
 *****************************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "mud.hpp"
#include "bounty.hpp"
#include "olc_bounty.hpp"

// ============================================================================
// Modern C++ Bounty Utilities Implementation
// ============================================================================
namespace BountyUtils {
    
    std::string legacy_to_string(const char* legacy_str) {
        if (!legacy_str) return "";
        return std::string(legacy_str);
    }
    
    const char* string_to_legacy(const std::string& modern_str) {
        // Note: This returns a temporary pointer. In full modernization,
        // we'd use a different approach, but for incremental compatibility
        // this works with existing STRALLOC patterns
        static thread_local std::string temp_storage;
        temp_storage = modern_str;
        return temp_storage.c_str();
    }
    
    bool is_bounty_target_safe(const char* target) {
        if (!target) return false;
        size_t len = strlen(target);
        if (len == 0 || len >= 256) return false; // Prevent buffer overflows
        
        // Basic security check: no control characters
        for (size_t i = 0; i < len; ++i) {
            if (target[i] < 32 && target[i] != '\n' && target[i] != '\r' && target[i] != '\t') {
                return false;
            }
        }
        return true;
    }
    
    long sanitize_bounty_amount(long amount) {
        // Prevent integer overflow and negative amounts
        if (amount < 0) return 0;
        if (amount > 2000000000L) return 2000000000L;
        return amount;
    }
    
    std::string get_safe_bounty_target(BountyData* bounty) {
        if (!bounty || !bounty->target) return "<unknown>";
        if (!is_bounty_target_safe(bounty->target)) return "<invalid>";
        return legacy_to_string(bounty->target);
    }
    
    BountyData* create_secure_bounty(const std::string& target, long amount, 
                                     int type, const std::string& source) {
        // Input validation
        if (!is_target_valid(target)) return nullptr;
        amount = sanitize_bounty_amount(amount);
        if (amount <= 0) return nullptr;
        
        // Create legacy structure using secure methods
        BountyData* bounty = nullptr;
        CREATE(bounty, BountyData, 1);
        if (!bounty) return nullptr;
        
        // Use STRALLOC for memory management (legacy-compatible)
        bounty->target = STRALLOC(const_cast<char*>(target.c_str()));
        bounty->amount = amount;
        bounty->type = type;
        
        if (!source.empty()) {
            bounty->source = STRALLOC(const_cast<char*>(source.c_str()));
        } else {
            bounty->source = STRALLOC(const_cast<char*>(""));
        }
        
        bounty->next = nullptr;
        bounty->prev = nullptr;
        
        return bounty;
    }
    
    void display_bounty_info_secure(CharData* ch, BountyData* bounty) {
        if (!ch || !bounty) return;
        
        // Use modern string safety functions
        std::string safe_target = get_safe_bounty_target(bounty);
        long safe_amount = sanitize_bounty_amount(bounty->amount);
        
        // Use legacy output functions with modern safety
        set_char_color(AtWhite, ch);
        ch_printf(ch, "Bounty Target: %s\n\r", safe_target.c_str());
        ch_printf(ch, "Amount: %ld credits\n\r", safe_amount);
        ch_printf(ch, "Type: %s\n\r", 
                 (bounty->type == BountyPolice) ? "Police" : "Player");
        
        if (bounty->source && strlen(bounty->source) > 0) {
            std::string safe_source = legacy_to_string(bounty->source);
            ch_printf(ch, "Source: %s\n\r", safe_source.c_str());
        }
    }
    
} // namespace BountyUtils

// ============================================================================
// Legacy Implementation (Unchanged)
// ============================================================================


BountyData *first_bounty;
BountyData *last_bounty;
BountyData *first_disintigration;
BountyData *last_disintigration;

#define BountyVersion 1

void disintigration args((CharData * ch, CharData * victim, long amount));
void nodisintigration args((CharData * ch, CharData * victim, long amount));
int       xp_compute(CharData * ch, CharData * victim);

bool char_exists(char *player)
{
        char      fname[MSL];
        struct stat fst;

        if (!player)
                return FALSE;
        snprintf(fname, MSL, "%s%c/%s", PlayerDir, tolower(player[0]),
                 capitalize(player));
        return (stat(fname, &fst) != -1);
}

void save_disintigrations()
{
        BountyData *tbounty;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", SystemDir, DisintigrationList);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open disintigration.lst for writing!\n\r",
                    0);
                return;
        }

        fprintf(fpout, "#VERSION %d\n", BountyVersion);
        for (tbounty = first_disintigration; tbounty; tbounty = tbounty->next)
        {
                fprintf(fpout, "#BOUNTY\n");
                fprintf(fpout, "Target %s~\n", tbounty->target);
                fprintf(fpout, "Type %d\n", tbounty->type);
                fprintf(fpout, "Amount %ld\n", tbounty->amount);
                if (tbounty->type == BountyPolice)
                        fprintf(fpout, "Source %s~\n", tbounty->source);
                fprintf(fpout, "End\n\n");
        }
        fprintf(fpout, "#END\n");
        FCLOSE(fpout);

}

BountyData *get_disintigration(char *target)
{
        BountyData *bounty;

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (!str_cmp(target, bounty->target))
                        return bounty;
        return NULL;
}

void fread_bounty(FILE * fp, int version [[maybe_unused]])
{
        const char *word;
        BountyData *bounty = NULL;
        bool fMatch [[maybe_unused]];

        CREATE(bounty, BountyData, 1);

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);

                switch (Upper(word[0]))
                {
                case '*':
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Amount", bounty->amount, fread_number(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!bounty->target
                                    || !char_exists(bounty->target)
                                    || (bounty->type == BountyPolice
                                        && (bounty->source == NULL
                                            || !get_clan(bounty->source))))
                                {
                                        if (bounty->target)
                                                STRFREE(bounty->target);

                                        if (bounty->source != NULL)
                                                STRFREE(bounty->source);

                                        DISPOSE(bounty);
                                }
                                else
                                {
                                        LINK(bounty, first_disintigration,
                                             last_disintigration, next, prev);

                                        if (bounty->source == NULL)
                                                bounty->source = STRALLOC(const_cast<char*>(""));
                                }

                                return;
                        }
                        break;
                case 'S':
                        KEY("Source", bounty->source, fread_string(fp));
                        break;

                case 'T':
                        KEY("Target", bounty->target, fread_string(fp));
                        KEY("Type", bounty->type, fread_number(fp));
                        break;
                }
        }
        version = 0;
}

void load_bounties()
{
        FILE     *fpList;
        const char *target;
        char      bountylist[256];
        BountyData *bounty;
        long int  amount;
        int       version = 0;
        char      letter;

        first_disintigration = NULL;
        last_disintigration = NULL;

        snprintf(bountylist, MSL, "%s%s", SystemDir, DisintigrationList);
        FCLOSE(fpReserve);
        if ((fpList = fopen(bountylist, "r")) == NULL)
        {
                perror(bountylist);
                exit(1);
        }


        letter = fread_letter(fpList);
        fseek(fpList, 0, 0);
        if (letter != '#')
        {
                for (;;)
                {
                        target = feof(fpList) ? "$" : fread_word(fpList);
                        if (target[0] == '$')
                                break;
                        bounty = get_disintigration(const_cast<char *>(target));
                        if (!bounty)
                        {
                                CREATE(bounty, BountyData, 1);
                                LINK(bounty, first_disintigration,
                                     last_disintigration, next, prev);
                                bounty->target = STRALLOC(const_cast<char *>(target));
                                bounty->amount = 0;
                        }
                        amount = fread_number(fpList);
                        bounty->amount += amount;
                        bounty->type = BountyPlayer;
                }
        }
        else
        {
                char     *word = NULL;

                for (;;)
                {
                        fread_letter(fpList);
                        word = fread_word(fpList);
                        if (!str_cmp(word, "VERSION"))
                                version = fread_number(fpList);
                        else if (!str_cmp(word, "BOUNTY"))
                                fread_bounty(fpList, version);
                        else if (!str_cmp(word, "END"))
                                break;  /* Done */
                        else
                        {
                                bug("load_bounties: bad section: %s", word);
                                break;
                        }
                }
        }

        FCLOSE(fpList);
        boot_log(" Done bounties ");

        fpReserve = fopen(NullFile, "r");
        return;
}

CMDF do_bounties(CharData * ch, char *argument [[maybe_unused]])
{
        BountyData *bounty;
        int       count = 0;

        argument = NULL;

        set_char_color(AtWhite, ch);
        send_to_char("\n\rBounty                      Amount\n\r", ch);
        if (!first_disintigration && !check_olc_bounties(ch->in_room))
        {
                set_char_color(AtGrey, ch);
                send_to_char("There are no bounties set at this time.\n\r",
                             ch);
                return;
        }

        if (first_disintigration)
                for (bounty = first_disintigration; bounty;
                     bounty = bounty->next)
                {
                        set_char_color(AtRed, ch);
                        if (bounty->type == BountyPolice)
                                ch_printf(ch,
                                          "%-26s   %-14ld &B[&RJailing Only&B] &B[&RPolice&B]&w %s&w\n\r",
                                          bounty->target, bounty->amount,
                                          bounty->source);
                        else
                                ch_printf(ch, "%-26s   %-14ld\n\r",
                                          bounty->target, bounty->amount);
                        count++;
                }
        count += print_olc_bounties(ch);
        set_char_color(AtGrey, ch);
        ch_printf(ch, "&BT&zhere are &W%d&z bounties at this time.\n\r",
                  count);

}

void disintigration(CharData * ch, char *name, long amount)
{
        BountyData *bounty;
        bool      found;
        char      buf[MaxStringLength];

        found = FALSE;

        if (IsNpc(ch))
        {
                send_to_char("You must be authorized to post a bounty.\n\r",
                             ch);
                return;
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
        {
                if (bounty->type == BountyPlayer
                    && !str_cmp(bounty->target, name))
                {
                        found = TRUE;
                        break;
                }
        }

        if (!found)
        {
                CREATE(bounty, BountyData, 1);
                LINK(bounty, first_disintigration, last_disintigration, next,
                     prev);

                bounty->target = STRALLOC(name);
                bounty->amount = 0;
        }

        bounty->type = BountyPlayer;
        bounty->amount = bounty->amount + amount;
        save_disintigrations();

        snprintf(buf, MSL,
                 "%s has added %ld credits to the bounty on %s.",
                 ch->name, amount, name);
        log_string(buf);
        snprintf(buf, MSL, "%ld credits has been added to the bounty on %s.",
                 amount, name);
        echo_to_all(AtRed, buf, 0);
}

CMDF do_addbounty(CharData * ch, char *argument)
{
        char      arg[MaxStringLength];
        long int  amount;
        CharData *victim;
        char      name[256];

        if (!argument || argument[0] == '\0')
        {
                do_bounties(ch, argument);
                return;
        }

        argument = one_argument(argument, arg);

        if (argument[0] == '\0')
        {
                send_to_char("Usage: Addbounty <target> <amount>\n\r", ch);
                return;
        }

/*        if (ch->PCData && ch->PCData->clan
            && !str_cmp(ch->PCData->clan->name, "the hunters guild"))
        {
                send_to_char("Your job is to collect bounties not post them.",
                             ch);
                return;
        }*/

        if (!ch->in_room || !IsSet(ch->in_room->RoomFlags, RoomBounty))
        {
                send_to_char
                        ("You will have to go to your local Hunters Guild office to add a new bounty.",
                         ch);
                return;
        }

        if (argument[0] == '\0')
                amount = 0;
        else
                amount = atoi(argument);

        if (amount < 5000)
        {
                send_to_char("A bounty should be at least 5000 credits.\n\r",
                             ch);
                return;
        }

        if (!(victim = get_char_world(ch, arg)))
        {
                char      fname[1024];
                struct stat fst;

                arg[0] = Upper(arg[0]);
                sprintf(fname, "%s%c/%s", PlayerDir, tolower(arg[0]),
                        capitalize(arg));

                if (stat(fname, &fst) != -1)
                {
                        strcpy(name, arg);
                }
                else
                {
                        send_to_char("No player exists by that name.\n\r",
                                     ch);
                        return;
                }
        }
        else
        {
                strcpy(name, victim->name);
                if (IsNpc(victim))
                {
                        send_to_char
                                ("You can only set bounties on other players .. not mobs!\n\r",
                                 ch);
                        return;
                }
        }

        if (amount <= 0)
        {
                send_to_char
                        ("Nice try! How about 1 or more credits instead...\n\r",
                         ch);
                return;
        }

        if (ch->gold < amount)
        {
                send_to_char("You don't have that many credits!\n\r", ch);
                return;
        }

        ch->gold = ch->gold - amount;

        disintigration(ch, name, amount);

}

void remove_bounties(char *target)
{
        BountyData *bounty, *bt_next;

        for (bounty = first_disintigration; bounty; bounty = bt_next)
        {
                bt_next = bounty->next;
                if (!str_cmp(bounty->target, target))
                {
                        UNLINK(bounty, first_disintigration,
                               last_disintigration, next, prev);
                        STRFREE(bounty->target);
                        if (bounty->source)
                                STRFREE(bounty->source);
                        DISPOSE(bounty);
                }
        }

        save_disintigrations();
}

void remove_disintigration(BountyData * bounty)
{
        UNLINK(bounty, first_disintigration, last_disintigration, next, prev);
        STRFREE(bounty->target);
        if (bounty->source)
                STRFREE(bounty->source);
        DISPOSE(bounty);

        save_disintigrations();
}

void claim_disintigration(CharData * ch, CharData * victim)
{
        BountyData *bounty;
        long int  experience;
        char      buf[MaxStringLength];

        if (IsNpc(victim))
                return;

        if (IsNpc(ch))
                return;

        bounty = get_disintigration(victim->name);

        if (ch == victim)
        {
                if (bounty != NULL)
                        remove_bounties(bounty->target);
                snprintf(buf, MSL, "%s is Dead!", victim->name);
                echo_to_all(AtRed, buf, 0);
                return;
        }

        if (bounty
            && (!ch->PCData || !ch->PCData->clan
                || str_cmp(ch->PCData->clan->name, "the hunters guild")))
        {
			    bug("%s is not in the hunters guild", ch->name);
                remove_bounties(bounty->target);
                bounty = NULL;
        }

        if (bounty == NULL || bounty->type == BountyPolice)
        {
                if (IsSet(victim->act, PlrKiller) && !IsNpc(ch))
                {
                        experience =
                                URange(1, xp_compute(ch, victim),
                                       (exp_level
                                        (ch->skill_level[HuntingAbility] +
                                         1) -
                                        exp_level(ch->
                                                  skill_level
                                                  [HuntingAbility])));
						experience *= 5;
                        gain_exp(ch, static_cast<int>(experience), HuntingAbility);
                        set_char_color(AtBlood, ch);
                        ch_printf(ch,
                                  "You receive %ld hunting experience for executing a wanted killer.\n\r",
                                  experience);
                }
                else if (!IsNpc(ch)
                         && (!ch->PCData || !ch->PCData->clan
                             || str_cmp(ch->PCData->clan->name,
                                        "the hunters guild")))
                {
                        SetBit(ch->act, PlrKiller);
                        ch_printf(ch,
                                  "You are now wanted for the murder of %s.\n\r",
                                  victim->name);
                }
                snprintf(buf, MSL, "%s is Dead!", victim->name);
                echo_to_all(AtRed, buf, 0);
                if (bounty)
                        remove_bounties(bounty->target);
                return;
        }

        ch->gold += bounty->amount;

        experience =
                URange(1, bounty->amount + xp_compute(ch, victim),
                       (exp_level(ch->skill_level[HuntingAbility] + 1) -
                        exp_level(ch->skill_level[HuntingAbility])));
        gain_exp(ch, static_cast<int>(experience), HuntingAbility);

        experience *= 5;

        set_char_color(AtBlood, ch);
        ch_printf(ch,
                  "You receive %ld bounty experience and %ld credits,\n\r from the bounty on %s\n\r",
                  experience, bounty->amount, bounty->target);

        snprintf(buf, MSL, "%s has claimed the disintigration bounty on %s!",
                 ch->name, victim->name);
        echo_to_all(AtRed, buf, 0);
        snprintf(buf, MSL, "%s is Dead!", victim->name);
        echo_to_all(AtRed, buf, 0);

        if (!IsSet(victim->act, PlrKiller)
            && (!ch->PCData || !ch->PCData->clan
                || str_cmp(ch->PCData->clan->name, "the hunters guild")))
                SetBit(ch->act, PlrKiller);
        remove_bounties(bounty->target);
}

void add_wanted(CharData * ch, PlanetData * planet)
{
        WantedData *wanted = NULL;

        if (planet == NULL || planet->governed_by == NULL)
                return;

        if (IsNpc(ch))
                return;

        for (wanted = ch->PCData->first_wanted; wanted; wanted = wanted->next)
        {
                if (wanted->government == planet->governed_by)
                        break;
        }

        if (!wanted)
        {
                CREATE(wanted, WantedData, 1);
                LINK(wanted, ch->PCData->first_wanted,
                     ch->PCData->last_wanted, next, prev);
                /*
                 * If we are now wanted 
                 */
                ch_printf(ch, "&YYou are now wanted on %s.&w\n\r",
                          planet->name);
                wanted->amount = StartingWanted;
        }
        else
        {
                wanted->amount = UMax(wanted->amount + 10, 100);
        }
        /*
         * if we hit a certain amount. add_bounty_police (which just does add_bounty and sets a flag?) 
         */
        if (wanted->amount >= 50)
                add_police_bounty(ch, planet);
        wanted->government = planet->governed_by;
}

CMDF do_payfee(CharData * ch, char *argument)
{
        ClanData *clan;
        BountyData *bounty;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: payfeee <clan>\n\r", ch);
                return;
        }

        if (!ch->in_room || !IsSet(ch->in_room->RoomFlags, RoomBounty))
        {
                send_to_char
                        ("You will have to go to your local Hunters Guild office to pay the fee for a bounty.\n\r",
                         ch);
                return;
        }

        if ((clan = get_clan(argument)) == NULL)
        {
                send_to_char("There is no government by that name!\n\r", ch);
                return;
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
        {
                if (bounty->type != BountyPolice)
                        continue;
                if (!str_cmp(ch->name, bounty->target)
                    && !str_cmp(clan->name, bounty->source))
                        break;
        }

        if (!bounty)
        {
                send_to_char("There is no bounty for you by that clan.\n\r",
                             ch);
                return;
        }

        if (ch->gold < bounty->amount * 3)
        {
                send_to_char("You don't have that many credits!\n\r", ch);
                return;
        }

        ch->gold -= bounty->amount * 3;
        clan->funds += bounty->amount * 3;
        remove_wanted(ch, clan);
        save_disintigrations();
        send_to_char
                ("You pay the fee, and the Hunters Guild removes your bounty.",
                 ch);

}

void add_police_bounty(CharData * ch, PlanetData * planet)
{
        BountyData *bounty;

        if (planet == NULL)
                return;
        if (planet->governed_by == NULL)
                return;
        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (bounty->type == BountyPolice &&
                    !str_cmp(ch->name, bounty->target) &&
                    !str_cmp(planet->governed_by->name, bounty->source))
                        break;

        if (bounty == NULL)
        {
                CREATE(bounty, BountyData, 1);
                LINK(bounty, first_disintigration, last_disintigration, next,
                     prev);

                bounty->target = STRALLOC(ch->name);
                bounty->amount = 1000;
        }
        else
        {
                bounty->amount = bounty->amount + 500;
        }
        bounty->source = STRALLOC(planet->governed_by->name);
        bounty->type = BountyPolice;
        save_disintigrations();
        return;
}

bool is_wanted(CharData * ch, PlanetData * pl)
{
        WantedData *wanted;

        if (IsNpc(ch))
                return FALSE;
        if (!ch->PCData)
                return FALSE;
        if (pl->governed_by == NULL)
                return FALSE;

        for (wanted = ch->PCData->first_wanted; wanted; wanted = wanted->next)
        {
                if (wanted->government == pl->governed_by)
                        return TRUE;
        }
        return FALSE;
}

void fwrite_wanted(CharData * ch, FILE * fp)
{
        WantedData *wanted;

        if (!ch->PCData)
                return;

        for (wanted = ch->PCData->first_wanted; wanted; wanted = wanted->next)
        {
                fprintf(fp, "#WANTED\n");
                fprintf(fp, "Amount %d\n", wanted->amount);
                if (wanted->government)
                        fprintf(fp, "Government %s~\n",
                                wanted->government->name);
                fprintf(fp, "End\n\n");
        }
}

void fread_wanted(CharData * ch, FILE * fp)
{
        const char *word;
        WantedData *wanted;
        bool fMatch [[maybe_unused]];

        CREATE(wanted, WantedData, 1);

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);

                switch (Upper(word[0]))
                {
                case '*':
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Amount", wanted->amount, fread_number(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!wanted->government)
                                        DISPOSE(wanted);
                                else
                                        LINK(wanted, ch->PCData->first_wanted,
                                             ch->PCData->last_wanted, next,
                                             prev);
                                return;
                        }
                        break;
                case 'G':
                        if (!str_cmp(word, "Government"))
                        {
                                char     *temp = fread_string(fp);

                                wanted->government = get_clan(temp);
                                STRFREE(temp);
                        }
                        break;
                }
        }
}

void remove_wanted_planet(CharData * ch, PlanetData * planet)
{
        remove_wanted(ch, planet->governed_by);
}

void remove_wanted(CharData * ch, ClanData * clan)
{
        WantedData *wanted;
        BountyData *bounty;

        if (IsNpc(ch))
                return;
        if (!clan)
                return;


        for (wanted = ch->PCData->first_wanted; wanted; wanted = wanted->next)
        {
                if (clan == wanted->government)
                        break;
        }

        if (wanted)
        {
                UNLINK(wanted, ch->PCData->first_wanted,
                       ch->PCData->last_wanted, next, prev);
                DISPOSE(wanted);
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (bounty->type == BountyPolice &&
                    !str_cmp(ch->name, bounty->target) &&
                    !str_cmp(clan->name, bounty->source))
                        break;

        if (bounty)
        {
                UNLINK(bounty, first_disintigration, last_disintigration,
                       next, prev);
                STRFREE(bounty->target);
                STRFREE(bounty->source);
                DISPOSE(bounty);
        }
}

CMDF do_imprison(CharData * ch, char *argument)
{
        CharData *victim = NULL;
        ClanData *clan = NULL;
        RoomIndexData *jail = NULL;
        BountyData *bounty = NULL;
        int       chance = 0;

        if (IsNpc(ch))
                return;
        if (!ch->in_room || !ch->in_room->area || !ch->in_room->area->planet)
                return;
        clan = ch->in_room->area->planet->governed_by;

        if (argument[0] == '\0')
        {
                send_to_char("Jail who?\n\r", ch);
                return;
        }

        if (ch->position == PosFighting)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (ch->position <= PosSleeping)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (!clan)
        {
                send_to_char
                        ("Nobody owns this planet. How do you expect to turn them into the authorities",
                         ch);
                return;
        }

        jail = get_room_index(clan->jail);

        if (!jail)
        {
                send_to_char
                        ("This government does not have a suitable prison.\n\r",
                         ch);
                return;
        }

        if (is_wanted(ch, ch->in_room->area->planet))
        {
                send_to_char("You are wanted by the current government.\n\r",
                             ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (IsNpc(victim))
        {
                send_to_char("That would be a waste of time.\n\r", ch);
                return;
        }

        if (victim->position != PosIncap &&
            victim->position != PosStunned &&
            victim->position != PosSleeping && !victim->held)
        {
                send_to_char("You will have to incapacitate them first.\n\r",
                             ch);
                return;
        }

        chance = static_cast<int>(ch->PCData->learned[gsn_imprison]);
        if (IsSet(ch->in_room->RoomFlags, RoomSafe) && chance < 80)
        {
                set_char_color(AtMagic, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }
        if (number_percent() >= chance)
        {
                act(AtAction,
                    "Your failed attempt to jail $n has allowed them to recover.",
                    ch, NULL, victim, ToChar);
                act(AtAction,
                    "$n's failed attempt to jail you has allowed time for you to recover.",
                    ch, NULL, victim, ToVict);
                act(AtAction,
                    "$n's failed attempt to jail $N has allowed time for them to recover.",
                    ch, NULL, victim, ToNotvict);
                WaitState(ch, 10 * PulseViolence);
                victim->hit = 75;
                victim->position = PosStanding;
                update_pos(victim);
                return;
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (bounty->type == BountyPolice &&
                    !str_cmp(victim->name, bounty->target) &&
                    !str_cmp(clan->name, bounty->source))
                        break;

        if (!bounty)
        {
                send_to_char
                        ("They are not wanted by the current government.\n\r",
                         ch);
                return;
        }

        if (victim->hit < 0)
                victim->hit = 100;
        victim->position = PosStanding;
        update_pos(victim);

        ch_printf(ch,
                  "You collect the %ld bounty and escort them off to jail.\n\r",
                  bounty->amount);
        ch->gold += bounty->amount;

        act(AtAction,
            "You have a strange feeling that you've been moved.\n\r", ch,
            NULL, victim, ToVict);
        act(AtAction, "$n has $N escorted away.\n\r", ch, NULL, victim,
            ToNotvict);

        char_from_room(victim);
        char_to_room(victim, jail);
        do_look(victim, "");
        act(AtAction,
            "The door opens briefly as $n is shoved into the room.\n\r",
            victim, NULL, NULL, ToRoom);

        learn_from_success(ch, gsn_imprison);
        gain_exp(ch, static_cast<int>(bounty->amount * 3), HuntingAbility);
        remove_wanted_planet(victim, ch->in_room->area->planet);

        return;
}

CMDF do_rembounty(CharData * ch, char *argument)
{
        BountyData *bounty;

        if (!argument || argument[0] == '\0')
        {
                do_bounties(ch, argument);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Usage: rembounty <target>\n\r", ch);
                return;
        }

//  argument[0] = Upper(argument[0]);
        bounty = get_disintigration(argument);
        if (bounty)
        {
                remove_disintigration(bounty);
                ch_printf(ch, "Done\n\r");
        }
        else
        {
                ch_printf(ch, "No Such Bounty\n\r");
                return;
        }
}

void free_bounty(BountyData * bounty)
{
        if (bounty->target)
                STRFREE(bounty->target);
        if (bounty->source)
                STRFREE(bounty->source);
        DISPOSE(bounty);
        bounty = NULL;
}

/*
 *  Sharpen skill
 *  Author: Rantic (supfly@geocities.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */
/*
#include <stdio.h>
#include "mud.hpp"
Moved to bounty.c since it's much more appropriate
*/

CMDF do_sharpen(CharData * ch, char *argument)
{
        ObjData *obj;
        ObjData *pobj;
        char      arg[MaxInputLength];
        AffectData *paf;
        int       percent;
        int       level;

        one_argument(argument, arg);


        if (arg[0] == '\0')
        {
                send_to_char("What do you wish to sharpen?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (!(obj = get_obj_carry(ch, arg)))
        {
                send_to_char("You do not have that weapon.\n\r", ch);
                return;
        }

        if (obj->item_type != ItemWeapon)
        {
                send_to_char
                        ("You can't sharpen something that's not a weapon.\n\r",
                         ch);
                return;
        }

        /*
         * Let's not allow people to sharpen bludgeons and the like ;) 
         */
        /*
         * small mods to make it more generic.. --Cronel 
         */
        if (obj->value[3] != WeaponKnife)
        {
                send_to_char("You can't sharpen that type of weapon!\n\r",
                             ch);
                return;
        }

        if (obj->value[5] == 1) /* see reason below when setting */
        {
                send_to_char
                        ("It is already as sharp as it's going to get.\n\r",
                         ch);
                return;
        }

        for (pobj = ch->first_carrying; pobj; pobj = pobj->next_content)
        {
                if (pobj->pIndexData->vnum == ObjVnumSharpen)
                        break;
        }

        if (!pobj)
        {
                send_to_char("You do not have a sharpening stone.\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_sharpen]->beats);
        /*
         * Character must have the Dexterity to sharpen the weapon nicely, 
         * * if not, damage weapon 
         */
        if (!IsNpc(ch) && get_curr_dex(ch) < 17)
        {
                separate_obj(obj);
                if (obj->value[0] <= 1)
                {
                        act(AtObject,
                            "$p breaks apart and falls to the ground in pieces!.",
                            ch, obj, NULL, ToChar);
                        extract_obj(obj);
                        learn_from_failure(ch, gsn_sharpen);
                        return;
                }
                else
                {
                        obj->value[0]--;
                        act(AtGreen, "You clumsily slip and damage $p!", ch,
                            obj, NULL, ToChar);
                        return;
                }
        }

        percent = (number_percent() - get_curr_lck(ch) - 15);   /* too low a chance to damage? */

        separate_obj(pobj);
        if (!IsNpc(ch) && percent > ch->PCData->learned[gsn_sharpen])
        {
                act(AtObject,
                    "You fail to sharpen $p correctly, damaging the stone.",
                    ch, obj, NULL, ToChar);
                if (pobj->value[0] <= 1)
                {
                        act(AtObject,
                            "The sharpening stone crumbles apart from misuse.",
                            ch, pobj, NULL, ToChar);
                        extract_obj(pobj);
                        learn_from_failure(ch, gsn_sharpen);
                        return;
                }
                pobj->value[0]--;
                learn_from_failure(ch, gsn_sharpen);
                return;
        }

        level = ch->top_level;
        separate_obj(obj);
        act(AtSkill,
            "With skill and precision, you sharpen $p to a fine edge.", ch,
            obj, NULL, ToChar);
        act(AtSkill, "With skill and precision, $n sharpens $p.", ch, obj,
            NULL, ToRoom);
        CREATE(paf, AffectData, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = ApplyDamroll;
        paf->modifier = level / 10;
        paf->bitvector = 0;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);

        obj->value[5] = 1;
        /*
         * originaly a sharpened object flag was used, but took up a BV, 
         * * so I switched to giving it a value5, which is not used in weapons 
         * * besides to check for this 
         */
        learn_from_success(ch, gsn_sharpen);
		learn_from_success(ch, gsn_hone);
        return;

}
