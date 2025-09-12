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
 *                            Information handling                                       *
 *****************************************************************************************/

// =============================================================================
// SYSTEM INCLUDES
// =============================================================================
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <crypt.h>
#if defined(__CYGWIN__)
#include <crypt.h>
#endif

// =============================================================================
// LOCAL INCLUDES
// =============================================================================
#include "mud.hpp"
#include "mxp.hpp"
#include "msp.hpp"
#include "web-server.hpp"
#ifdef OlcShuttle
#include "olc-shuttle.hpp"
#endif
#include "editor.hpp"
#include "boards.hpp"
#include "account.hpp"
#include "astral.hpp"
#include "races.hpp"
#include "greet.hpp"
#include "password.hpp"

// =============================================================================
// CONSTANTS
// =============================================================================
namespace {
    constexpr int    MentalStabilityThreshold = 40;
    constexpr int    DrunkDivisor = 12;
    constexpr int    MaxHallucinationLevel = 20;
    constexpr int    MinHallucinationRange = 6;
    constexpr int    AgeSuffixThresholdLow = 4;
    constexpr int    AgeSuffixThresholdHigh = 20;
    constexpr int    MinPasswordLength = 8;
    constexpr int    PagerMinLines = 5;
    constexpr int    SocialColumns = 6;
    constexpr int    CommandColumns = 4;
    constexpr int    SkillListColumns = 3;
    constexpr size_t RevisionOffset = 11;
    constexpr size_t RevisionEndOffset = 2;
}

// Define practice restrictions outside namespace to avoid const issues
#define CantPrac "Tongue"

// =============================================================================
// EXTERNAL FUNCTION DECLARATIONS
// =============================================================================
void send_gmcp_event(DescriptorData* desc, const char* event, const char* json);
extern int top_help;
extern int top_area;

// =============================================================================
// LOCAL FUNCTION PROTOTYPES
// =============================================================================
char* trim(const char* str);
void show_visible_affects_to_char(CharData* victim, CharData* ch);
const char* halucinated_object(int ms, bool fShort);
RoomIndexData* generate_exit(RoomIndexData* in_room, ExitData** pexit);
HelpData* get_help(CharData* ch, char* argument);
char* help_fix(char* text);
int get_comfreq(CharData* ch);
void show_char_to_char_0(CharData* victim, CharData* ch);
void show_char_to_char_1(CharData* victim, CharData* ch);
void show_char_to_char(CharData* list, CharData* ch);
void show_ships_to_char(ShipData* ship, CharData* ch);
bool check_blind(CharData* ch);
void show_condition(CharData* ch, CharData* victim);
sh_int str_similarity(const char* astr, const char* bstr);
sh_int str_prefix_level(const char* astr, const char* bstr);
void similar_help_files(CharData* ch, char* argument);

// =============================================================================
// GLOBAL DATA
// =============================================================================

const char* const where_name[] = {
        "<used as light>     ",
        "<worn on finger>    ",
        "<worn on finger>    ",
        "<worn around neck>  ",
        "<worn around neck>  ",
        "<worn on body>      ",
        "<worn on head>      ",
        "<worn on legs>      ",
        "<worn on feet>      ",
        "<worn on hands>     ",
        "<worn on arms>      ",
        "<energy shield>     ",
        "<worn about body>   ",
        "<worn about waist>  ",
        "<worn around wrist> ",
        "<worn around wrist> ",
        "<wielded>           ",
        "<held>              ",
        "<dual wielded>      ",
        "<worn on ears>      ",
        "<worn on eyes>      ",
        "<missile wielded>   ",
        "<bound by>          ",
        "<left holster>      ",
        "<right holster>     "
};

// =============================================================================
// OBJECT FORMATTING FUNCTIONS
// =============================================================================

/*
 * Format an object for display to a character
 */
char* format_obj_to_char(ObjData* obj, CharData* ch, bool fShort)
{
        static char buf[MaxStringLength];

        buf[0] = '\0';
        if (IsObjStat(obj, ItemInvis))
                mudstrlcat(buf, "(Invis) ", MSL);
        if ((IsAffected(ch, AffDetectMagic) || IsImmortal(ch))
            && IsObjStat(obj, ItemMagic))
                mudstrlcat(buf, "&B(Blue Aura)&w ", MSL);
        if (IsObjStat(obj, ItemGlow))
                mudstrlcat(buf, "(Glowing) ", MSL);
        if (IsObjStat(obj, ItemHum))
                mudstrlcat(buf, "(Humming) ", MSL);
        if (IsObjStat(obj, ItemHidden))
                mudstrlcat(buf, "(Hidden) ", MSL);
        if (IsObjStat(obj, ItemBurried))
                mudstrlcat(buf, "(Burried) ", MSL);
        if (IsImmortal(ch) && IsObjStat(obj, ItemPrototype))
                mudstrlcat(buf, "(PROTO) ", MSL);
        if (IsAffected(ch, AffDetecttraps) && is_trapped(obj))
                mudstrlcat(buf, "(Trap) ", MSL);

        if (fShort)
        {
                if (obj->short_descr)
                        mudstrlcat(buf, obj->short_descr, MSL);
        }
        else
        {
                if (obj->description)
                        mudstrlcat(buf, obj->description, MSL);
        }

        return buf;
}

// =============================================================================
// HALLUCINATION SYSTEM
// =============================================================================

/*
 * Some increasingly freaky halucinated objects - Thoric
 */
const char* halucinated_object(int ms, bool fShort)
{
        int       sms = URange(1, (ms + 10) / 5, 20);

        if (fShort)
                switch (number_range(MinHallucinationRange - URange(1, sms / 2, 5), sms)) {
                case 1:
                        return "a sword";
                case 2:
                        return "a stick";
                case 3:
                        return "something shiny";
                case 4:
                        return "something";
                case 5:
                        return "something interesting";
                case 6:
                        return "something colorful";
                case 7:
                        return "something that looks cool";
                case 8:
                        return "a nifty thing";
                case 9:
                        return "a cloak of flowing colors";
                case 10:
                        return "a mystical flaming sword";
                case 11:
                        return "a swarm of insects";
                case 12:
                        return "a deathbane";
                case 13:
                        return "a figment of your imagination";
                case 14:
                        return "your gravestone";
                case 15:
                        return "the long lost boots of Ranger Thoric";
                case 16:
                        return "a glowing tome of arcane knowledge";
                case 17:
                        return "a long sought secret";
                case 18:
                        return "the meaning of it all";
                case 19:
                        return "the answer";
                case 20:
                        return "the key to life, the universe and everything";
                default:
                        return "-error";
                }
        switch (number_range(MinHallucinationRange - URange(1, sms / 2, 5), sms)) {
        case 1:
                return "A nice looking sword catches your eye.";
        case 2:
                return "The ground is covered in small sticks.";
        case 3:
                return "Something shiny catches your eye.";
        case 4:
                return "Something catches your attention.";
        case 5:
                return "Something interesting catches your eye.";
        case 6:
                return "Something colorful flows by.";
        case 7:
                return "Something that looks cool calls out to you.";
        case 8:
                return "A nifty thing of great importance stands here.";
        case 9:
                return "A cloak of flowing colors asks you to wear it.";
        case 10:
                return "A mystical flaming sword awaits your grasp.";
        case 11:
                return "A swarm of insects buzzes in your face!";
        case 12:
                return "The extremely rare Deathbane lies at your feet.";
        case 13:
                return "A figment of your imagination is at your command.";
        case 14:
                return "You notice a gravestone here... upon closer examination, it reads your name.";
        case 15:
                return "The long lost boots of Ranger Thoric lie off to the side.";
        case 16:
                return "A glowing tome of arcane knowledge hovers in the air before you.";
        case 17:
                return "A long sought secret of all mankind is now clear to you.";
        case 18:
                return "The meaning of it all, so simple, so clear... of course!";
        case 19:
                return "The answer.  One.  It's always been One.";
        case 20:
                return "The key to life, the universe and everything awaits your hand.";
        default:
                return "-error";
        }
        return "Whoa!!!";
}

// =============================================================================
// OBJECT LISTING FUNCTIONS
// =============================================================================

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(ObjData* list, CharData* ch, bool fShort, bool fShowNothing)
{
        char    **prgpstrShow;
        int      *prgnShow;
        int      *pitShow;
        char     *pstrShow;
        ObjData *obj;
        int       nShow;
        int       iShow;
        int       count, offcount, tmp, ms, cnt;
        bool      fCombine;

        if (!ch->desc)
                return;

        /*
         * if there's no list... then don't do all this crap!  -Thoric
         */
        if (!list)
        {
                if (fShowNothing)
                {
                        if (IsNpc(ch) || IsSet(ch->act, PlrCombine))
                                send_to_char("     ", ch);
                        send_to_char("Nothing.\n\r", ch);
                }
                return;
        }
        /*
         * Alloc space for output lines.
         */
        count = 0;
        for (obj = list; obj; obj = obj->next_content)
                count++;

        ms = (ch->mental_state ? ch->mental_state : 1)
                *
                (IsNpc(ch) ? 1
                 : (ch->PCData->
                    condition[CondDrunk] ? (ch->PCData->
                                             condition[CondDrunk] /
                                             12) : 1));

        /*
         * If not mentally stable...
         */
        if (abs(ms) > MentalStabilityThreshold) {
                offcount = URange(-(count), (count * ms) / 100, count * 2);
                if (offcount < 0)
                        offcount += number_range(0, abs(offcount));
                else if (offcount > 0)
                        offcount -= number_range(0, offcount);
        }
        else
                offcount = 0;

        if (count + offcount <= 0)
        {
                if (fShowNothing)
                {
                        if (IsNpc(ch) || IsSet(ch->act, PlrCombine))
                                send_to_char("     ", ch);
                        send_to_char("Nothing.\n\r", ch);
                }
                return;
        }

        CREATE(prgpstrShow, char *, static_cast<size_t>(count + ((offcount > 0) ? offcount : 0)));
        CREATE(prgnShow, int, static_cast<size_t>(count + ((offcount > 0) ? offcount : 0)));
        CREATE(pitShow, int, static_cast<size_t>(count + ((offcount > 0) ? offcount : 0)));

        nShow = 0;
        tmp = (offcount > 0) ? offcount : 0;
        cnt = 0;

        /*
         * Format the list of objects.
         */
        for (obj = list; obj; obj = obj->next_content)
        {
                if (offcount < 0 && ++cnt > (count + offcount))
                        break;
                if (tmp > 0 && number_bits(1) == 0)
                {
                        prgpstrShow[nShow] =
                                str_dup(halucinated_object(ms, fShort));
                        prgnShow[nShow] = 1;
                        pitShow[nShow] = number_range(ItemLight, ItemBook);
                        nShow++;
                        --tmp;
                }
                if (obj->wear_loc == WearNone
                    && can_see_obj(ch, obj)
                    && (obj->item_type != ItemTrap
                        || IsAffected(ch, AffDetecttraps)))
                {
                        pstrShow = format_obj_to_char(obj, ch, fShort);
                        fCombine = FALSE;

                        if (IsNpc(ch) || IsSet(ch->act, PlrCombine))
                        {
                                /*
                                 * Look for duplicates, case sensitive.
                                 * Matches tend to be near end so run loop backwords.
                                 */
                                for (iShow = nShow - 1; iShow >= 0; iShow--)
                                {
                                        if (!strcmp
                                            (prgpstrShow[iShow], pstrShow))
                                        {
                                                prgnShow[iShow] += obj->count;
                                                fCombine = TRUE;
                                                break;
                                        }
                                }
                        }

                        pitShow[nShow] = obj->item_type;
                        /*
                         * Couldn't combine, or didn't want to.
                         */
                        if (!fCombine)
                        {
                                prgpstrShow[nShow] = str_dup(pstrShow);
                                prgnShow[nShow] = obj->count;
                                nShow++;
                        }
                }
        }
        if (tmp > 0)
        {
                int       x;

                for (x = 0; x < tmp; x++)
                {
                        prgpstrShow[nShow] =
                                str_dup(halucinated_object(ms, fShort));
                        prgnShow[nShow] = 1;
                        pitShow[nShow] = number_range(ItemLight, ItemBook);
                        nShow++;
                }
        }

        /*
         * Output the formatted list.       -Color support by Thoric
         */
        for (iShow = 0; iShow < nShow; iShow++)
        {
                switch (pitShow[iShow])
                {
                default:
                        set_char_color(AtObject, ch);
                        break;
                case ItemBlood:
                        set_char_color(AtBlood, ch);
                        break;
                case ItemMoney:
                case ItemTreasure:
                        set_char_color(AtYellow, ch);
                        break;
                case ItemFood:
                        set_char_color(AtHungry, ch);
                        break;
                case ItemDrinkCon:
                case ItemFountain:
                        set_char_color(AtThirsty, ch);
                        break;
                case ItemFire:
                        set_char_color(AtFire, ch);
                        break;
                case ItemScroll:
                case ItemWand:
                case ItemStaff:
                        set_char_color(AtMagic, ch);
                        break;
                }
                if (fShowNothing)
                        send_to_char("     ", ch);
                send_to_char(prgpstrShow[iShow], ch);
/*	if ( IsNpc(ch) || IsSet(ch->act, PlrCombine) ) */
                {
                        if (prgnShow[iShow] != 1)
                                ch_printf(ch, " (%d)", prgnShow[iShow]);
                }

                send_to_char("\n\r", ch);
                DISPOSE(prgpstrShow[iShow]);
        }

        if (fShowNothing && nShow == 0)
        {
                if (IsNpc(ch) || IsSet(ch->act, PlrCombine))
                        send_to_char("     ", ch);
                send_to_char("Nothing.\n\r", ch);
        }

        /*
         * Clean up.
         */
        free(prgpstrShow);
        free(prgnShow);
        free(pitShow);
        return;
}

// =============================================================================
// CHARACTER DISPLAY FUNCTIONS
// =============================================================================

/*
 * Show fancy descriptions for certain spell affects - Thoric
 */
void show_visible_affects_to_char(CharData* victim, CharData* ch)
{
        char      buf[MaxStringLength];

        if (IsAffected(victim, AffSanctuary))
        {
                if (IsGood(victim))
                {
                        set_char_color(AtWhite, ch);
                        ch_printf(ch,
                                  "%s glows with an aura of divine radiance.\n\r",
                                  IsNpc(victim) ? capitalize(victim->
                                                              short_descr)
                                  : (victim->name));
                }
                else if (IsEvil(victim))
                {
                        set_char_color(AtWhite, ch);
                        ch_printf(ch,
                                  "%s shimmers beneath an aura of dark energy.\n\r",
                                  IsNpc(victim) ? capitalize(victim->
                                                              short_descr)
                                  : (victim->name));
                }
                else
                {
                        set_char_color(AtWhite, ch);
                        ch_printf(ch,
                                  "%s is shrouded in flowing shadow and light.\n\r",
                                  IsNpc(victim) ? capitalize(victim->
                                                              short_descr)
                                  : (victim->name));
                }
        }
        if (IsAffected(victim, AffFireshield))
        {
                set_char_color(AtFire, ch);
                ch_printf(ch,
                          "%s is engulfed within a blaze of mystical flame.\n\r",
                          IsNpc(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
        if (IsAffected(victim, AffShockshield))
        {
                set_char_color(AtBlue, ch);
                ch_printf(ch,
                          "%s is surrounded by cascading torrents of energy.\n\r",
                          IsNpc(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
/*Scryn 8/13*/
        if (IsAffected(victim, AffIceshield))
        {
                set_char_color(AtLblue, ch);
                ch_printf(ch,
                          "%s is ensphered by shards of glistening ice.\n\r",
                          IsNpc(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
        if (IsAffected(victim, AffCharm))
        {
                set_char_color(AtMagic, ch);
                ch_printf(ch, "%s looks ahead free of expression.\n\r",
                          IsNpc(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
        if (!IsNpc(victim) && !victim->desc
            && victim->switched && IsAffected(victim->switched, AffPossess))
        {
                set_char_color(AtMagic, ch);
                mudstrlcpy(buf, PERS(victim, ch), MSL);
                mudstrlcat(buf, " appears to be in a deep trance...\n\r",
                           MSL);
        }
}

/*
 * Display a character to another character (brief view)
 */
void show_char_to_char_0(CharData* victim, CharData* ch)
{
        char      buf[MaxStringLength];
        char      buf1[MaxStringLength];


        buf[0] = '\0';

        if (IsNpc(victim))
                mudstrlcat(buf, " ", MSL);

        if (!IsNpc(victim) && !victim->desc)
        {
                if (!victim->switched)
                        mudstrlcat(buf, "(Link Dead) ", MSL);
                else if (!IsAffected(victim->switched, AffPossess))
                        mudstrlcat(buf, "(Switched) ", MSL);
        }
        if (IsNpc(victim) && ch->questmob > 0
            && victim->pIndexData->vnum == ch->questmob)
                mudstrlcat(buf, "[TARGET] ", MSL);
        if (!IsNpc(victim) && IsSet(victim->act, PlrAfk))
                mudstrlcat(buf, "[AFK] ", MSL);

        if ((!IsNpc(victim) && IsSet(victim->act, PlrWizinvis))
            || (IsNpc(victim) && IsSet(victim->act, ActMobinvis)))
        {
                if (!IsNpc(victim))
                        snprintf(buf1, MSL, "(Invis %d) ",
                                 victim->PCData->wizinvis);
                else
                        snprintf(buf1, MSL, "(Mobinvis %d) ",
                                 victim->mobinvis);
                mudstrlcat(buf, buf1, MSL);
        }
        if (IsAffected(victim, AffInvisible))
                mudstrlcat(buf, "(Invis) ", MSL);
        if (IsAffected(victim, AffHide))
                mudstrlcat(buf, "(Hide) ", MSL);
        if (IsAffected(victim, AffPassDoor))
                mudstrlcat(buf, "(Translucent) ", MSL);
        if (IsAffected(victim, AffFaerieFire))
                mudstrlcat(buf, "&P(Pink Aura)&w ", MSL);
        if (IsEvil(victim) && IsAffected(ch, AffDetectEvil))
                mudstrlcat(buf, "&R(Red Aura)&w ", MSL);
        if ((victim->perm_frc > 1 && victim->endurance > 10)
            && (IsAffected(ch, AffDetectMagic) || IsImmortal(ch)))
                mudstrlcat(buf, "&B(Blue Aura)&w ", MSL);
        if (!IsNpc(victim) && IsSet(victim->act, PlrLitterbug))
                mudstrlcat(buf, "(LITTERBUG) ", MSL);
        if (IsNpc(victim) && IsImmortal(ch)
            && IsSet(victim->act, ActPrototype))
                mudstrlcat(buf, "(PROTO) ", MSL);
        if (victim->desc && victim->desc->connected == ConEditing)
                mudstrlcat(buf, "(Writing) ", MSL);

        set_char_color(AtPerson, ch);
        if (victim->position == victim->defposition
            && victim->long_descr[0] != '\0')
        {
                mudstrlcat(buf, victim->long_descr, MSL);
                send_to_char(buf, ch);
                show_visible_affects_to_char(victim, ch);
                return;
        }

        if (can_see(ch,victim)) {
                char * temp = get_char_desc(victim,ch);
                temp[0] = Upper(temp[0]);
                mudstrlcat(buf, temp ,MSL);
        }
        else if (IsImmortal(victim)) {
                mudstrlcat(buf, "An immortal", MSL);
        }
        else {
                mudstrlcat(buf, "Someone", MSL);
        }

        switch (victim->position)
        {
        case PosDead:
                mudstrlcat(buf, " is DEAD!!", MSL);
                break;
        case PosMortal:
                mudstrlcat(buf, " is mortally wounded.", MSL);
                break;
        case PosIncap:
                mudstrlcat(buf, " is incapacitated.", MSL);
                break;
        case PosStunned:
                mudstrlcat(buf, " is lying here stunned.", MSL);
                break;
        case PosSleeping:
                if (victim->on != NULL)
                {
                        if (victim->on->value[2] == SleepAt)
                        {
                                mudstrlcat(buf, " &wis asleep at ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == SleepOn)
                        {
                                mudstrlcat(buf, " &wis asleep on ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == SleepIn)
                        {
                                mudstrlcat(buf, " &wis asleep in ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                }
                else
                {
                        if (ch->position == PosSitting
                            || ch->position == PosResting)
                                mudstrlcat(buf, " is sleeping nearby.", MSL);
                        else
                                mudstrlcat(buf, " is deep in slumber here.",
                                           MSL);
                }
                break;

        case PosResting:

                if (victim->on != NULL)
                {
                        if (victim->on->value[2] == RestAt)
                        {
                                mudstrlcat(buf, " &wis resting at ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == RestOn)
                        {
                                mudstrlcat(buf, " &wis resting on ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else
                        {
                                mudstrlcat(buf, " &wis resting in ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                }
                else
                {
                        if (ch->position == PosResting)
                                mudstrlcat(buf,
                                           " &wis sprawled out alongside you.",
                                           MSL);
                        else if (ch->position == PosMounted)
                                mudstrlcat(buf,
                                           " &wis sprawled out at the foot of your mount.",
                                           MSL);
                        else
                                mudstrlcat(buf, " &wis sprawled out here.",
                                           MSL);
                }
                break;
        case PosSitting:
                if (victim->on != NULL)
                {
                        if (victim->on->value[2] == SitAt)
                        {
                                mudstrlcat(buf, " &wis sitting at ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == SitOn)
                        {
                                mudstrlcat(buf, " &wis sitting on ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else
                        {
                                mudstrlcat(buf, " &wis sitting in ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                }
                else
                        mudstrlcat(buf, " &wis sitting here.", MSL);
                break;


        case PosStanding:
                if (IsImmortal(victim))
                        mudstrlcat(buf, " is here before you.", MSL);
                else if ((victim->in_room->sector_type == SectUnderwater)
                         && !IsAffected(victim, AffAquaBreath)
                         && !IsNpc(victim))
                        mudstrlcat(buf, " is drowning here.", MSL);
                else if (victim->in_room->sector_type == SectUnderwater)
                        mudstrlcat(buf, " is here in the water.", MSL);
                else if ((victim->in_room->sector_type == SectOceanfloor)
                         && !IsAffected(victim, AffAquaBreath)
                         && !IsNpc(victim))
                        mudstrlcat(buf, " is drowning here.", MSL);
                else if (victim->in_room->sector_type == SectOceanfloor)
                        mudstrlcat(buf, " is standing here in the water.",
                                   MSL);
                else if (IsAffected(victim, AffFloating)
                         || IsAffected(victim, AffFlying))
                        mudstrlcat(buf, " is hovering here.", MSL);
                else
                        mudstrlcat(buf, " is standing here.", MSL);
                break;
        case PosShove:
                mudstrlcat(buf, " is being shoved around.", MSL);
                break;
        case PosDrag:
                mudstrlcat(buf, " is being dragged around.", MSL);
                break;
        case PosMounted:
                mudstrlcat(buf, " is here, upon ", MSL);
                if (!victim->mount)
                        mudstrlcat(buf, "thin air???", MSL);
                else if (victim->mount == ch)
                        mudstrlcat(buf, "your back.", MSL);
                else if (victim->in_room == victim->mount->in_room)
                {
                        mudstrlcat(buf, PERS(victim->mount, ch), MSL);
                        mudstrlcat(buf, ".", MSL);
                }
                else
                        mudstrlcat(buf, "someone who left??", MSL);
                break;
        case PosFighting:
                mudstrlcat(buf, " is here, fighting ", MSL);
                if (!victim->fighting)
                        mudstrlcat(buf, "thin air???", MSL);
                else if (who_fighting(victim) == ch)
                        mudstrlcat(buf, "YOU!", MSL);
                else if (victim->in_room == victim->fighting->who->in_room)
                {
                        mudstrlcat(buf, PERS(victim->fighting->who, ch), MSL);
                        mudstrlcat(buf, ".", MSL);
                }
                else
                        mudstrlcat(buf, "someone who left??", MSL);
                break;
        default:
                {
                }
        }

        mudstrlcat(buf, "\n\r", MSL);
        buf[0] = Upper(buf[0]);
        send_to_char(buf, ch);
        show_visible_affects_to_char(victim, ch);
        return;
}



void show_char_to_char_1(CharData * victim, CharData * ch)
{
        ObjData *obj;
        int       iWear;
        bool      found;

        if (can_see(victim, ch))
        {
                act(AtAction, "$n looks at you.", ch, NULL, victim, ToVict);
                act(AtAction, "$n looks at $N.", ch, NULL, victim ,ToNotvict);
        }
        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&z",
                 ch);
        if (victim->description[0] != '\0')
                ch_printf(ch, "&z%s\n\r", wordwrap(victim->description, 72));
        else
        {
                if (victim->race && victim->race->name())
                        ch_printf(ch,
                                  "You see nothing special about the %s.\n\r",
                                  victim->race->name());
        }
        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&w",
                 ch);
        show_condition(ch, victim);
        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&z",
                 ch);
        found = FALSE;
        for (iWear = 0; iWear < MaxWear; iWear++)
        {
                if ((obj = get_eq_char(victim, iWear)) != NULL
                    && can_see_obj(ch, obj))
                {
                        if (!found)
                        {
                                send_to_char("\n\r", ch);
                                act(AtPlain, "&w$N &zis using:&w", ch, NULL,
                                    victim, ToChar);
                                found = TRUE;
                        }
                        // Fix color bleed. -- Kasji
                        ch_printf(ch, "&w%s", where_name[iWear]);
/*                      send_to_char(where_name[iWear], ch); */
                        send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
                        send_to_char("\n\r", ch);
                }
        }

        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&z",
                 ch);
        /*
         * Crash fix here by Thoric
         */
        if (IsNpc(ch) || victim == ch)
                return;

        if (number_percent() < ch->PCData->learned[gsn_peek])
        {
                send_to_char("\n\r&BY&zou peek at the inventory:\n\r", ch);
                show_list_to_char(victim->first_carrying, ch, TRUE, TRUE);
                learn_from_success(ch, gsn_peek);
        }
        else if (ch->PCData->learned[gsn_peek])
                learn_from_failure(ch, gsn_peek);
        return;
}


void show_char_to_char(CharData * list, CharData * ch)
{
        CharData *rch;

        for (rch = list; rch; rch = rch->next_in_room)
        {
                if (rch == ch)
                        continue;

                if (can_see(ch, rch))
                {
                        show_char_to_char_0(rch, ch);
                }
                else if (!str_cmp(rch->race->name(), "defel"))
                {
                        set_char_color(AtBlood, ch);
                        send_to_char
                                ("You see a pair of red eyes staring back at you.\n\r",
                                 ch);
                }
                else if (room_is_dark(ch->in_room)
                         && IsAffected(rch, AffInfrared))
                {
                        set_char_color(AtBlood, ch);
                        send_to_char
                                ("The red form of a living creature is here.\n\r",
                                 ch);
                }
        }

        return;
}



bool check_blind(CharData * ch)
{
        if (!IsNpc(ch) && IsSet(ch->act, PlrHolylight))
                return TRUE;

        if (IsAffected(ch, AffTruesight))
                return TRUE;

        if (IsAffected(ch, AffBlind))
        {
                send_to_char("You can't see a thing!\n\r", ch);
                return FALSE;
        }

        return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door(char *arg)
{
        int       door;

        if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
                door = 0;
        else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
                door = 1;
        else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
                door = 2;
        else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
                door = 3;
        else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
                door = 4;
        else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
                door = 5;
        else if (!str_cmp(arg, "ne") || !str_cmp(arg, "northeast"))
                door = 6;
        else if (!str_cmp(arg, "nw") || !str_cmp(arg, "northwest"))
                door = 7;
        else if (!str_cmp(arg, "se") || !str_cmp(arg, "southeast"))
                door = 8;
        else if (!str_cmp(arg, "sw") || !str_cmp(arg, "southwest"))
                door = 9;
        else
                door = -1;
        return door;
}

// =============================================================================
// INFORMATION COMMANDS
// =============================================================================

/*
 * Look command - examine the environment and objects
 */
CMDF do_look(CharData* ch, const char* argument)
{
        char      arg[MaxInputLength];
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        ExitData *pexit;
        CharData *victim;
        ObjData *obj;
        RoomIndexData *original;
        char     *pdesc;
        bool      doexaprog;
        sh_int    door;
        int       number, cnt;

        if (!ch->desc)
                return;

        if (ch->position < PosSleeping)
        {
                send_to_char("You can't see anything but stars!\n\r", ch);
                return;
        }

        if (ch->position == PosSleeping)
        {
                send_to_char("You can't see anything, you're sleeping!\n\r",
                             ch);
                return;
        }

        if (!check_blind(ch))
                return;

        if (!IsNpc(ch)
            && !IsSet(ch->act, PlrHolylight)
            && !IsAffected(ch, AffTruesight) && room_is_dark(ch->in_room))
        {
                set_char_color(AtDgrey, ch);
                send_to_char("It is pitch black ... \n\r", ch);
                show_char_to_char(ch->in_room->first_person, ch);
                return;
        }

        const char* arg_ptr = one_argument(const_cast<char*>(argument), arg1);
        arg_ptr = one_argument(const_cast<char*>(arg_ptr), arg2);
        arg_ptr = one_argument(const_cast<char*>(arg_ptr), arg3);

        doexaprog = str_cmp("noprog", arg2) && str_cmp("noprog", arg3);

        if (arg1[0] == '\0' || !str_cmp(arg1, "auto"))
        {
                ShipData *ship;

                /*
                 * 'look' or 'look auto' 
                 */
                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);
                send_to_char("&w", ch);
                if (IsMxp(ch))
                        send_to_char(MXPTAG("RName"), ch);
                send_to_char(ch->in_room->name, ch);
                if (IsMxp(ch))
                        send_to_char(MXPTAG("/RName"), ch);
                send_to_char(" ", ch);

                if (!ch->desc->original)
                {

                        if (get_trust(ch) >= LevelImmortal)
                        {
                                if (IsSet(ch->act, PlrRoomvnum))
                                {
                                        /*
                                         * Added 10/17 by Kuran of 
                                         */
                                        send_to_char("&B{&w", ch);  /* SWReality */
                                        if (IsMxp(ch))
                                                send_to_char(MXPTAG("RNum"),
                                                             ch);
                                        ch_printf(ch, "%d",
                                                  ch->in_room->vnum);
                                        if (IsMxp(ch))
                                                send_to_char(MXPTAG("/RNum"),
                                                             ch);
                                        send_to_char("&B}}", ch);
                                }
                                if (IsSet(ch->PCData->flags, PcflagRoom))
                                {
                                        send_to_char("[&z", ch);
                                        send_to_char(ext_flag_string
                                                     (&ch->in_room->
                                                      RoomFlags, r_flags),
                                                     ch);
                                        send_to_char("&B]", ch);
                                }
                        }

                }

                send_to_char("\n\r", ch);
                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);
                send_to_char("&z", ch);
                if (arg1[0] == '\0'
                    || (!IsNpc(ch) && !IsSet(ch->act, PlrBrief)))
                {
                        if (IsMxp(ch))
                                send_to_char(MXPTAG("RDesc"), ch);
                        send_to_char(ch->in_room->description, ch);
                        if (IsMxp(ch))
                                send_to_char(MXPTAG("/RDesc"), ch);
                }

                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);
                if (!IsNpc(ch) && IsSet(ch->act, PlrAutoexit))
                        do_exits(ch, "");
                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);


                show_ships_to_char(ch->in_room->first_ship, ch);
#ifdef OlcShuttle
                show_shuttles_to_char(ch, ch->in_room->first_shuttle);
#endif
                show_list_to_char(ch->in_room->first_content, ch, FALSE,
                                  FALSE);
                show_char_to_char(ch->in_room->first_person, ch);
                if (ch->desc)
                {
                        /* Send detailed Room.Info for web navigation */
                        char room_buf[1024];
                        char desc_buf[512];
                        mudstrlcpy(desc_buf, ch->in_room->description, sizeof(desc_buf));
                        char *clean_desc = smash_color(desc_buf);
                        snprintf(room_buf, sizeof(room_buf),
                                 "{\"num\":%d,\"name\":\"%s\",\"desc\":\"%s\"}",
                                 ch->in_room->vnum, ch->in_room->name, clean_desc);
                        send_gmcp_event(ch->desc, "Core.Room.Info", room_buf);

                        /* Send Area.Weather GMCP/GCMP event */
                        /* If area weather support is added, send Area.Weather GMCP/GCMP event here. */
                        /* Currently, area_data has no weather member, so this block is disabled. */
                }

                if (str_cmp(arg1, "auto"))
                        if ((ship =
                             ship_from_cockpit(ch->in_room->vnum)) != NULL)
                        {
                                set_char_color(AtWhite, ch);
                                ch_printf(ch,
                                          "\n\rThrough the transparisteel windows you see:\n\r");
                                if (ship->starsystem)
                                {
                                        MissileData *missile = NULL;
                                        ShipData *target = NULL;
                                        BodyData *body = NULL;

                                        set_char_color(AtGreen, ch);
                                        /*
                                         * I really hate doing 3 for loops for sorting, we should time it 
                                         */
                                        ForEachList(BodyList,
                                                      ship->starsystem->
                                                      bodies, body)
                                        {
                                                if (body->type() == StarBody)
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  body->
                                                                  name());
                                        }
                                        ForEachList(BodyList,
                                                      ship->starsystem->
                                                      bodies, body)
                                        {
                                                if (body->type() ==
                                                    PlanetBody)
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  body->
                                                                  name());
                                        }
                                        ForEachList(BodyList,
                                                      ship->starsystem->
                                                      bodies, body)
                                        {
                                                if (body->type() == MoonBody)
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  body->
                                                                  name());
                                        }
                                        for (target =
                                             ship->starsystem->first_ship;
                                             target;
                                             target =
                                             target->next_in_starsystem)
                                        {
                                                if (target != ship
                                                    && !IsSet(target->flags,
                                                               ShipCloak))
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  target->
                                                                  name);
                                        }
                                        for (missile =
                                             ship->starsystem->first_missile;
                                             missile;
                                             missile =
                                             missile->next_in_starsystem)
                                        {
                                                ch_printf(ch, "%s\n\r",
                                                          missile->
                                                          missiletype ==
                                                          ConcussionMissile ?
                                                          "A Concusion Missile"
                                                          : (missile->
                                                             missiletype ==
                                                             ProtonTorpedo ?
                                                             "A Torpedo"
                                                             : (missile->
                                                                missiletype ==
                                                                HeavyRocket ?
                                                                "A Heavy Rocket"
                                                                :
                                                                "A Heavy Bomb")));
                                        }

                                }
                                else if (ship->location == ship->lastdoc)
                                {
                                        RoomIndexData *to_room;

                                        if ((to_room =
                                             get_room_index(ship->
                                                            location)) !=
                                            NULL)
                                        {
                                                ch_printf(ch, "\n\r");
                                                original = ch->in_room;
                                                char_from_room(ch);
                                                char_to_room(ch, to_room);
                                                do_glance(ch, "");
                                                char_from_room(ch);
                                                char_to_room(ch, original);
                                        }
                                }


                        }

                return;
        }

        if (!str_cmp(arg1, "under"))
        {
                int       count;

                /*
                 * 'look under' 
                 */
                if (arg2[0] == '\0')
                {
                        send_to_char("Look beneath what?\n\r", ch);
                        return;
                }

                if ((obj = get_obj_here(ch, arg2)) == NULL)
                {
                        send_to_char("You do not see that here.\n\r", ch);
                        return;
                }
                if (ch->carry_weight + obj->weight > can_carry_w(ch))
                {
                        send_to_char
                                ("It's too heavy for you to look under.\n\r",
                                 ch);
                        return;
                }
                count = obj->count;
                obj->count = 1;
                act(AtPlain, "You lift $p and look beneath it:", ch, obj,
                    NULL, ToChar);
                act(AtPlain, "$n lifts $p and looks beneath it:", ch, obj,
                    NULL, ToRoom);
                obj->count = static_cast<sh_int>(count);
                if (IsObjStat(obj, ItemCovering))
                        show_list_to_char(obj->first_content, ch, TRUE, TRUE);
                else
                        send_to_char("Nothing.\n\r", ch);
                if (doexaprog)
                        oprog_examine_trigger(ch, obj);
                return;
        }

        if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in"))
        {
                int       count;

                /*
                 * 'look in' 
                 */
                if (arg2[0] == '\0')
                {
                        send_to_char("Look in what?\n\r", ch);
                        return;
                }

                if ((obj = get_obj_here(ch, arg2)) == NULL)
                {
                        send_to_char("You do not see that here.\n\r", ch);
                        return;
                }

                switch (obj->item_type)
                {
                default:
                        send_to_char("That is not a container.\n\r", ch);
                        break;

                case ItemDrinkCon:
                        if (obj->value[1] <= 0)
                        {
                                send_to_char("It is empty.\n\r", ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                break;
                        }

                        ch_printf(ch, "It's %s full of a %s liquid.\n\r",
                                  obj->value[1] < obj->value[0] / 4
                                  ? "less than" :
                                  obj->value[1] < 3 * obj->value[0] / 4
                                  ? "about" : "more than",
                                  liq_table[obj->value[2]].liq_color);

                        if (doexaprog)
                                oprog_examine_trigger(ch, obj);
                        break;

                case ItemPortal:
                        for (pexit = ch->in_room->first_exit; pexit;
                             pexit = pexit->next)
                        {
                                if (pexit->vdir == DirPortal
                                    && IsSet(pexit->exit_info, ExPortal))
                                {
                                        if (room_is_private
                                            (ch, pexit->to_room)
                                            && get_trust(ch) <
                                            sysdata.level_override_private)
                                        {
                                                set_char_color(AtWhite, ch);
                                                send_to_char
                                                        ("That room is private buster!\n\r",
                                                         ch);
                                                return;
                                        }
                                        original = ch->in_room;
                                        char_from_room(ch);
                                        char_to_room(ch, pexit->to_room);
                                        do_look(ch, "auto");
                                        char_from_room(ch);
                                        char_to_room(ch, original);
                                        return;
                                }
                        }
                        send_to_char("You see a swirling chaos...\n\r", ch);
                        break;
                case ItemHolster:
                case ItemContainer:
                case ItemCorpseNpc:
                case ItemCorpsePc:
                case ItemDroidCorpse:
                        if (IsSet(obj->value[1], ContClosed))
                        {
                                send_to_char("It is closed.\n\r", ch);
                                break;
                        }

                        count = obj->count;
                        obj->count = 1;
                        act(AtPlain, "$p contains:", ch, obj, NULL, ToChar);
                        obj->count = static_cast<sh_int>(count);
                        show_list_to_char(obj->first_content, ch, TRUE, TRUE);
                        if (doexaprog)
                                oprog_examine_trigger(ch, obj);
                        break;
                }
                return;
        }

        if ((pdesc =
             get_extra_descr(arg1, ch->in_room->first_extradesc)) != NULL)
        {
                send_to_char(pdesc, ch);
                return;
        }

        door = static_cast<sh_int>(get_door(arg1));
        if ((pexit = find_door(ch, arg1, TRUE)) != NULL)
        {
                if (pexit->keyword)
                {
                        if (IsSet(pexit->exit_info, ExClosed)
                            && !IsSet(pexit->exit_info, ExWindow))
                        {
                                if (IsSet(pexit->exit_info, ExSecret)
                                    && door != -1)
                                        send_to_char
                                                ("Nothing special there.\n\r",
                                                 ch);
                                else
                                        act(AtPlain, "The $d is closed.", ch,
                                            NULL, pexit->keyword, ToChar);
                                return;
                        }
                        if (IsSet(pexit->exit_info, ExBashed))
                                act(AtRed,
                                    "The $d has been bashed from its hinges!",
                                    ch, NULL, pexit->keyword, ToChar);
                }

                if (pexit->description && pexit->description[0] != '\0')
                        send_to_char(pexit->description, ch);
                else
                        send_to_char("Nothing special there.\n\r", ch);

                /*
                 * Ability to look into the next room         -Thoric
                 */
                if (pexit->to_room
                    && (IsAffected(ch, AffScrying)
                        || IsSet(pexit->exit_info, EX_xLOOK)
                        || get_trust(ch) >= LevelImmortal))
                {
                        if (!IsSet(pexit->exit_info, EX_xLOOK)
                            && get_trust(ch) < LevelImmortal)
                        {
                                set_char_color(AtMagic, ch);
                                send_to_char("You attempt to scry...\n\r",
                                             ch);
                                /*
                                 * Change by Narn, Sept 96 to allow characters who don't have the
                                 * scry spell to benefit from objects that are affected by scry.
                                 */
                                if (!IsNpc(ch))
                                {
                                        int       percent =
                                                ch->PCData->
                                                learned[skill_lookup("scry")];
                                        if (!percent)
                                                percent = 99;

                                        if (number_percent() > percent)
                                        {
                                                send_to_char("You fail.\n\r",
                                                             ch);
                                                return;
                                        }
                                }
                        }
                        if (room_is_private(ch, pexit->to_room)
                            && get_trust(ch) < sysdata.level_override_private)
                        {
                                set_char_color(AtWhite, ch);
                                send_to_char
                                        ("That room is private buster!\n\r",
                                         ch);
                                return;
                        }
                        original = ch->in_room;
                        if (pexit->distance > 1)
                        {
                                RoomIndexData *to_room;

                                if ((to_room =
                                     generate_exit(ch->in_room,
                                                   &pexit)) != NULL)
                                {
                                        char_from_room(ch);
                                        char_to_room(ch, to_room);
                                }
                                else
                                {
                                        char_from_room(ch);
                                        char_to_room(ch, pexit->to_room);
                                }
                        }
                        else
                        {
                                char_from_room(ch);
                                char_to_room(ch, pexit->to_room);
                        }
                        do_look(ch, "auto");
                        char_from_room(ch);
                        char_to_room(ch, original);
                }
                return;
        }
        else if (door != -1)
        {
                send_to_char("Nothing special there.\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) != NULL)
        {
                show_char_to_char_1(victim, ch);
                return;
        }


        /*
         * finally fixed the annoying look 2.obj desc bug   -Thoric 
         */
        number = number_argument(arg1, arg);
        for (cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (can_see_obj(ch, obj))
                {
                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }

                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->pIndexData->
                                             first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }

                        if (nifty_is_name_prefix(arg, obj->name))
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                pdesc = get_extra_descr(obj->name,
                                                        obj->pIndexData->
                                                        first_extradesc);
                                if (!pdesc)
                                        pdesc = get_extra_descr(obj->name,
                                                                obj->
                                                                first_extradesc);
                                if (!pdesc)
                                        send_to_char
                                                ("You see nothing special.\r\n",
                                                 ch);
                                else
                                        send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }
                }
        }

        for (obj = ch->in_room->last_content; obj; obj = obj->prev_content)
        {
                if (can_see_obj(ch, obj))
                {
                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }

                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->pIndexData->
                                             first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }
                        if (nifty_is_name_prefix(arg, obj->name))
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                pdesc = get_extra_descr(obj->name,
                                                        obj->pIndexData->
                                                        first_extradesc);
                                if (!pdesc)
                                        pdesc = get_extra_descr(obj->name,
                                                                obj->
                                                                first_extradesc);
                                if (!pdesc)
                                        send_to_char
                                                ("You see nothing special.\r\n",
                                                 ch);
                                else
                                        send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }
                }
        }

        send_to_char("You do not see that here.\n\r", ch);
        return;
}

void show_condition(CharData * ch, CharData * victim)
{
        char      buf[MaxStringLength];
        int       percent;

        if (!victim || !victim->name)
                return;
        if (victim->max_hit > 0)
                percent = (100 * victim->hit) / victim->max_hit;
        else
                percent = -1;


        mudstrlcpy(buf, "&R&W", MSL);
        mudstrlcat(buf, capitalize(PERS(victim, ch)), MSL);
        if ((IsNpc(victim) && IsSet(victim->act, ActDroid))
            || !str_cmp(victim->race->name(), "droid"))
        {

                if (percent >= 100)
                        mudstrlcat(buf, " is in perfect condition.\n\r", MSL);
                else if (percent >= 90)
                        mudstrlcat(buf, " is slightly scratched.\n\r", MSL);
                else if (percent >= 80)
                        mudstrlcat(buf, " has a few scrapes.\n\r", MSL);
                else if (percent >= 70)
                        mudstrlcat(buf, " has some dents.\n\r", MSL);
                else if (percent >= 60)
                        mudstrlcat(buf,
                                   " has a couple holes in its plating.\n\r",
                                   MSL);
                else if (percent >= 50)
                        mudstrlcat(buf, " has a many broken pieces.\n\r",
                                   MSL);
                else if (percent >= 40)
                        mudstrlcat(buf, " has many exposed circuits.\n\r",
                                   MSL);
                else if (percent >= 30)
                        mudstrlcat(buf, " is leaking oil.\n\r", MSL);
                else if (percent >= 20)
                        mudstrlcat(buf, " has smoke coming out of it.\n\r",
                                   MSL);
                else if (percent >= 10)
                        mudstrlcat(buf, " is almost completely broken.\n\r",
                                   MSL);
                else
                        mudstrlcat(buf, " is about to EXPLODE.\n\r", MSL);

        }
        else
        {

                if (percent >= 100)
                        mudstrlcat(buf, " is in perfect health.\n\r", MSL);
                else if (percent >= 90)
                        mudstrlcat(buf, " is slightly scratched.\n\r", MSL);
                else if (percent >= 80)
                        mudstrlcat(buf, " has a few bruises.\n\r", MSL);
                else if (percent >= 70)
                        mudstrlcat(buf, " has some cuts.\n\r", MSL);
                else if (percent >= 60)
                        mudstrlcat(buf, " has several wounds.\n\r", MSL);
                else if (percent >= 50)
                        mudstrlcat(buf, " has many nasty wounds.\n\r", MSL);
                else if (percent >= 40)
                        mudstrlcat(buf, " is bleeding freely.\n\r", MSL);
                else if (percent >= 30)
                        mudstrlcat(buf, " is covered in blood.\n\r", MSL);
                else if (percent >= 20)
                        mudstrlcat(buf, " is leaking guts.\n\r", MSL);
                else if (percent >= 10)
                        mudstrlcat(buf, " is almost dead.\n\r", MSL);
                else
                        mudstrlcat(buf, " is DYING.\n\r", MSL);

        }
        buf[0] = Upper(buf[0]);
        send_to_char(buf, ch);
        return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
CMDF do_glance(CharData * ch, const char *argument)
{
        char      arg1[MaxInputLength];
        CharData *victim;
        int       save_act;

        if (!ch->desc)
                return;

        if (ch->position < PosSleeping)
        {
                send_to_char("You can't see anything but stars!\n\r", ch);
                return;
        }

        if (ch->position == PosSleeping)
        {
                send_to_char("You can't see anything, you're sleeping!\n\r",
                             ch);
                return;
        }

        if (!check_blind(ch))
                return;

        one_argument(const_cast<char*>(argument), arg1);

        if (arg1[0] == '\0')
        {
                save_act = ch->act;
                SetBit(ch->act, PlrBrief);
                do_look(ch, "auto");
                ch->act = save_act;
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They're not here.", ch);
                return;
        }
        else
        {
                if (can_see(victim, ch))
                {
                        act(AtAction, "$n glances at you.", ch, NULL, victim,
                            ToVict);
                        act(AtAction, "$n glances at $N.", ch, NULL, victim,
                            ToNotvict);
                }

                show_condition(ch, victim);
                return;
        }

        return;
}


/* New command to view a player's skills - Samson 4-13-98 */
CMDF do_viewskills(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      buf[MaxStringLength];
        CharData *victim;
        int       sn;
        int       col;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("&zSyntax: skills <player>.\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("No such person in the game.\n\r", ch);
                return;
        }

        col = 0;

        if (!IsNpc(victim))
        {
                sh_int    lasttype, cnt;

                col = cnt = 0;
                lasttype = SkillSpell;
                set_pager_color(AtMagic, ch);
                for (sn = 0; sn < top_sn; sn++)
                {
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                                continue;

                        if (strcmp(skill_table[sn]->name, "reserved") == 0
                            && (IsImmortal(victim)))
                        {
                                if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                send_to_pager
                                        ("&B--------------------------------[&zSpells&B]---------------------------------\n\r",
                                         ch);
                                send_to_pager("&w", ch);
                                col = 0;
                        }
                        if (skill_table[sn]->type != lasttype)
                        {
                                if (!cnt)
                                        send_to_pager
                                                ("                                (none)\n\r",
                                                 ch);
                                else if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                pager_printf(ch,
                                             "&B--------------------------------&B[&z%ss&B]---------------------------------\n\r",
                                             skill_tname[skill_table[sn]->
                                                         type]);
                                col = cnt = 0;
                        }

                        lasttype = skill_table[sn]->type;

                        if (skill_table[sn]->races
                            && skill_table[sn]->races[0] != '\0')
                        {
                                snprintf(buf, MSL, "%s", ch->race->name());
                                if (!is_name(buf, skill_table[sn]->races))
                                {
                                        continue;
                                }
                        }


                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                                continue;

                        if (victim->PCData->learned[sn] <= 0
                            && victim->skill_level[skill_table[sn]->guild] <
                            skill_table[sn]->min_level)
                                continue;

                        if (victim->PCData->learned[sn] == 0
                            && SpellFlag(skill_table[sn], SfSecretskill))
                                continue;

                        ++cnt;
                        pager_printf(ch, "&z%18.18s &W%3.3d%%  ",
                                     skill_table[sn]->name,
                                     victim->PCData->learned[sn]);
                        if (++col % 3 == 0)
                                send_to_pager("\n\r", ch);
                }

                if (col % 3 != 0)
                        send_to_pager("\n\r", ch);
        }
        return;
}



CMDF do_examine(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        char      arg[MaxInputLength];
        ObjData *obj;
        BoardData *board;
        sh_int    dam;

        if (!argument)
        {
                bug("do_examine: null argument.", 0);
                return;
        }

        if (!ch)
        {
                bug("do_examine: null ch.", 0);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Examine what?\n\r", ch);
                return;
        }

        snprintf(buf, MSL, "%s noprog", arg);
        do_look(ch, buf);

        /*
         * Support for looking at boards, checking equipment conditions,
         * and support for trigger positions by Thoric
         */
        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                if ((board = get_board(obj)) != NULL)
                {
                        if (board->num_posts)
                                ch_printf(ch,
                                          "There are about %d notes posted here.  Type 'note list' to list them.\n\r",
                                          board->num_posts);
                        else
                                send_to_char
                                        ("There aren't any notes posted here.\n\r",
                                         ch);
                }

                switch (obj->item_type)
                {
                default:
                        break;

                case ItemArmor:
                        if (obj->value[1] == 0)
                                obj->value[1] = obj->value[0];
                        if (obj->value[1] == 0)
                                obj->value[1] = 1;
                        dam = static_cast<sh_int>((obj->value[0] * 10) / obj->value[1]);
                        mudstrlcpy(buf,
                                   "As you look more closely, you notice that it is ",
                                   MSL);
                        if (dam >= 10)
                                mudstrlcat(buf, "in superb condition.", MSL);
                        else if (dam == 9)
                                mudstrlcat(buf, "in very good condition.",
                                           MSL);
                        else if (dam == 8)
                                mudstrlcat(buf, "in good shape.", MSL);
                        else if (dam == 7)
                                mudstrlcat(buf, "showing a bit of wear.",
                                           MSL);
                        else if (dam == 6)
                                mudstrlcat(buf, "a little run down.", MSL);
                        else if (dam == 5)
                                mudstrlcat(buf, "in need of repair.", MSL);
                        else if (dam == 4)
                                mudstrlcat(buf, "in great need of repair.",
                                           MSL);
                        else if (dam == 3)
                                mudstrlcat(buf, "in dire need of repair.",
                                           MSL);
                        else if (dam == 2)
                                mudstrlcat(buf, "very badly worn.", MSL);
                        else if (dam == 1)
                                mudstrlcat(buf, "practically worthless.",
                                           MSL);
                        else if (dam <= 0)
                                mudstrlcat(buf, "broken.", MSL);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        break;

                case ItemWeapon:
                        dam = static_cast<sh_int>(InitWeaponCondition - obj->value[0]);
                        mudstrlcpy(buf,
                                   "As you look more closely, you notice that it is ",
                                   MSL);
                        if (dam == 0)
                                mudstrlcat(buf, "in superb condition.", MSL);
                        else if (dam == 1)
                                mudstrlcat(buf, "in excellent condition.",
                                           MSL);
                        else if (dam == 2)
                                mudstrlcat(buf, "in very good condition.",
                                           MSL);
                        else if (dam == 3)
                                mudstrlcat(buf, "in good shape.", MSL);
                        else if (dam == 4)
                                mudstrlcat(buf, "showing a bit of wear.",
                                           MSL);
                        else if (dam == 5)
                                mudstrlcat(buf, "a little run down.", MSL);
                        else if (dam == 6)
                                mudstrlcat(buf, "in need of repair.", MSL);
                        else if (dam == 7)
                                mudstrlcat(buf, "in great need of repair.",
                                           MSL);
                        else if (dam == 8)
                                mudstrlcat(buf, "in dire need of repair.",
                                           MSL);
                        else if (dam == 9)
                                mudstrlcat(buf, "very badly worn.", MSL);
                        else if (dam == 10)
                                mudstrlcat(buf, "practically worthless.",
                                           MSL);
                        else if (dam == 11)
                                mudstrlcat(buf, "almost broken.", MSL);
                        else if (dam == 12)
                                mudstrlcat(buf, "broken.", MSL);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        if (obj->value[3] == WeaponBlaster)
                        {
                                if (obj->blaster_setting == BlasterFull)
                                        ch_printf(ch,
                                                  "It is set on FULL power.\n\r");
                                else if (obj->blaster_setting == BlasterHigh)
                                        ch_printf(ch,
                                                  "It is set on HIGH power.\n\r");
                                else if (obj->blaster_setting ==
                                         BlasterNormal)
                                        ch_printf(ch,
                                                  "It is set on NORMAL power.\n\r");
                                else if (obj->blaster_setting == BlasterHalf)
                                        ch_printf(ch,
                                                  "It is set on HALF power.\n\r");
                                else if (obj->blaster_setting == BlasterLow)
                                        ch_printf(ch,
                                                  "It is set on LOW power.\n\r");
                                else if (obj->blaster_setting == BlasterStun)
                                        ch_printf(ch,
                                                  "It is set on STUN.\n\r");
                                ch_printf(ch,
                                          "It has from %d to %d shots remaining.\n\r",
                                          obj->value[4] / 5, obj->value[4]);
                        }
                        else if ((obj->value[3] == WeaponLightsaber ||
                                  obj->value[3] == WeaponVibroBlade ||
                                  obj->value[3] == WeaponForcePike))
                        {
                                ch_printf(ch,
                                          "It has %d/%d units of charge remaining.\n\r",
                                          obj->value[4], obj->value[5]);
                        }
                        break;

                case ItemFood:
                        if (obj->timer > 0 && obj->value[1] > 0)
                                dam = static_cast<sh_int>((obj->timer * 10) / obj->value[1]);
                        else
                                dam = 10;
                        mudstrlcpy(buf,
                                   "As you examine it carefully you notice that it ",
                                   MSL);
                        if (dam >= 10)
                                mudstrlcat(buf, "is fresh.", MSL);
                        else if (dam == 9)
                                mudstrlcat(buf, "is nearly fresh.", MSL);
                        else if (dam == 8)
                                mudstrlcat(buf, "is perfectly fine.", MSL);
                        else if (dam == 7)
                                mudstrlcat(buf, "looks good.", MSL);
                        else if (dam == 6)
                                mudstrlcat(buf, "looks ok.", MSL);
                        else if (dam == 5)
                                mudstrlcat(buf, "is a little stale.", MSL);
                        else if (dam == 4)
                                mudstrlcat(buf, "is a bit stale.", MSL);
                        else if (dam == 3)
                                mudstrlcat(buf, "smells slightly off.", MSL);
                        else if (dam == 2)
                                mudstrlcat(buf, "smells quite rank.", MSL);
                        else if (dam == 1)
                                mudstrlcat(buf, "smells revolting.", MSL);
                        else if (dam <= 0)
                                mudstrlcat(buf, "is crawling with maggots.",
                                           MSL);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        break;

                case ItemSwitch:
                case ItemLever:
                case ItemPullchain:
                        if (IsSet(obj->value[0], TrigUp))
                                send_to_char
                                        ("You notice that it is in the up position.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("You notice that it is in the down position.\n\r",
                                         ch);
                        break;
                case ItemButton:
                        if (IsSet(obj->value[0], TrigUp))
                                send_to_char
                                        ("You notice that it is depressed.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("You notice that it is not depressed.\n\r",
                                         ch);
                        break;

/* Not needed due to check in do_look already
	case ItemPortal:
	    snprintf( buf, MSL, "in %s noprog", arg );
	    do_look( ch, buf );
	    break;
*/

                case ItemCorpsePc:
                case ItemCorpseNpc:
                        {
                                sh_int    timerfrac = obj->timer;

                                if (obj->item_type == ItemCorpsePc)
                                        timerfrac = static_cast<int>(obj->timer) / 8 + 1;

                                switch (timerfrac)
                                {
                                default:
                                        send_to_char
                                                ("This corpse has recently been slain.\n\r",
                                                 ch);
                                        break;
                                case 4:
                                        send_to_char
                                                ("This corpse was slain a little while ago.\n\r",
                                                 ch);
                                        break;
                                case 3:
                                        send_to_char
                                                ("A foul smell rises from the corpse, and it is covered in flies.\n\r",
                                                 ch);
                                        break;
                                case 2:
                                        send_to_char
                                                ("A writhing mass of maggots and decay, you can barely go near this corpse.\n\r",
                                                 ch);
                                        break;
                                case 1:
                                case 0:
                                        send_to_char
                                                ("Little more than bones, there isn't much left of this corpse.\n\r",
                                                 ch);
                                        break;
                                }
                        }
                        if (IsObjStat(obj, ItemCovering))
                                break;
                        send_to_char("When you look inside, you see:\n\r",
                                     ch);
                        snprintf(buf, MSL, "in %s noprog", arg);
                        do_look(ch, buf);
                        break;

                case ItemDroidCorpse:
                        {
                                sh_int    timerfrac = obj->timer;

                                switch (timerfrac)
                                {
                                default:
                                        send_to_char
                                                ("These remains are still smoking.\n\r",
                                                 ch);
                                        break;
                                case 4:
                                        send_to_char
                                                ("The parts of this droid have cooled down completely.\n\r",
                                                 ch);
                                        break;
                                case 3:
                                        send_to_char
                                                ("The broken droid components are beginning to rust.\n\r",
                                                 ch);
                                        break;
                                case 2:
                                        send_to_char
                                                ("The pieces are completely covered in rust.\n\r",
                                                 ch);
                                        break;
                                case 1:
                                case 0:
                                        send_to_char
                                                ("All that remains of it is a pile of crumbling rust.\n\r",
                                                 ch);
                                        break;
                                }
                        }
                        [[fallthrough]];

                case ItemContainer:
                        if (IsObjStat(obj, ItemCovering))
                                break;
                        [[fallthrough]];

                case ItemDrinkCon:
                        send_to_char("When you look inside, you see:\n\r",
                                     ch);
                        snprintf(buf, MSL, "in %s noprog", arg);
                        do_look(ch, buf);
                }
                if (IsObjStat(obj, ItemCovering))
                {
                        snprintf(buf, MSL, "under %s noprog", arg);
                        do_look(ch, buf);
                }
                oprog_examine_trigger(ch, obj);
                if (char_died(ch) || obj_extracted(obj))
                        return;

                check_for_trap(ch, obj, TrapExamine);
        }
        return;
}


CMDF do_exits(CharData * ch, const char *argument)
{
        char      buf[MaxStringLength];
        ExitData *pexit;
        bool      found;
        bool      fAuto;

        set_char_color(AtExits, ch);
        buf[0] = '\0';
        fAuto = !str_cmp(argument, "auto");

        if (!check_blind(ch))
                return;

        if (IsMxp(ch))
                send_to_char(MXPTAG("RExits"), ch);
        mudstrlcpy(buf, fAuto ? "&BE&xits:" : "&BObvious exits:\n\r&w", MSL);

        found = FALSE;
        for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
        {
                if (pexit->to_room && !IsSet(pexit->exit_info, ExHidden))
                {
                        found = TRUE;
                        if (!fAuto)
                        {
                                if (IsSet(pexit->exit_info, ExClosed))
                                {
                                        /*
                                         * I really dson't like copy and pasting, but its alot cleaner and easier right now than hacking together lines of mudstrlcat and stuff - Gavin 
                                         */
                                        if (IsMxp(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("Ex")
                                                         "%-5s" MXPTAG("/Ex")
                                                         " - (closed)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - (closed)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                }
                                else if (IsSet(pexit->exit_info, ExWindow))
                                {
                                        if (IsMxp(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("Ex")
                                                         "%-5s" MXPTAG("/Ex")
                                                         " - (window)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - (window)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                }
                                else if (IsSet(pexit->exit_info, EX_xAUTO))
                                {
                                        if (IsMxp(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("ex")
                                                         "%-5s" MXPTAG("/ex")
                                                         " - %s\n\r",
                                                         pexit->keyword,
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - %s\n\r",
                                                         pexit->keyword,
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                }
                                else
                                {
                                        if (IsMxp(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("Ex")
                                                         "%-5s" MXPTAG("/Ex")
                                                         " - %s\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]),
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "Too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - %s\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]),
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "Too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                }
                        }
                        else
                        {
                                if (IsMxp(ch))
                                        snprintf(buf + strlen(buf), MSL,
                                                 "&w " MXPTAG("Ex") "%s"
                                                 MXPTAG("/Ex"),
                                                 capitalize(dir_name
                                                            [pexit->vdir]));
                                else
                                        snprintf(buf + strlen(buf), MSL,
                                                 "&w %s",
                                                 capitalize(dir_name
                                                            [pexit->vdir]));
                        }
                }
        }

        if (!found)
                mudstrlcat(buf, fAuto ? " none.\n\r" : "None.\n\r", MSL);
        else if (fAuto)
                mudstrlcat(buf, ".\n\r", MSL);
        send_to_char(buf, ch);
        if (IsMxp(ch))
                send_to_char(MXPTAG("/RExits"), ch);
        return;
}

const char* const day_name[] = {
        "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
        "the Great Gods", "the Sun"
};

const char* const month_name[] = {
        "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
        "the Grand Struggle", "the Spring", "Nature", "Futility",
        "the Dragon",
        "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
        "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

extern char str_boot_time[];
extern char reboot_time[];
CMDF do_time(CharData * ch, [[maybe_unused]] const char *argument)
{
        const char *suf;
        int       day;
        time_t    current;
        time_t    diff;

        current = time(0);
        diff = current - boot_time;

        (void)argument;
        day = time_info.day + 1;

        if (day > AgeSuffixThresholdLow && day < AgeSuffixThresholdHigh)
                suf = "th";
        else if (day % 10 == 1)
                suf = "st";
        else if (day % 10 == 2)
                suf = "nd";
        else if (day % 10 == 3)
                suf = "rd";
        else
                suf = "th";

        set_char_color(AtYellow, ch);
        ch_printf(ch,
                  "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r"
                  "The mud started up at:    %s\r"
                  "The system time (M.S.T.): %s\r"
                  "Next Reboot is set for:   %s\r",
                  (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
                  time_info.hour >= 12 ? "pm" : "am",
                  day_name[day % 7],
                  day, suf,
                  month_name[time_info.month],
                  str_boot_time, const_cast<char *>(ctime(&current_time)), reboot_time);
        if (sysdata.CLEANPFILES)
                ch_printf(ch, "Next pfile cleanup :      %s",
                          const_cast<char *>(ctime(&new_pfile_time_t)));
        ch_printf(ch,
                  "The mud has been up for:  %d hours, %d minutes, %d seconds.\n\r",
                  diff / 3600, (diff / 60) % 60, diff % 60);

        return;
}



CMDF do_weather(CharData * ch, const char *argument)
{
        static const char* const sky_look[4] = {
                "cloudless",
                "cloudy",
                "rainy",
                "lit by flashes of lightning"
        };

        (void)argument; // Mark as unused
        if (!IsOutside(ch))
        {
                send_to_char("You can't see the sky from here.\n\r", ch);
                return;
        }

        set_char_color(AtBlue, ch);
        ch_printf(ch, "The sky is %s and %s.\n\r",
                  sky_look[weather_info.sky],
                  weather_info.change >= 0
                  ? "a warm southerly breeze blows"
                  : "a cold northern gust blows");
        return;
}


/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HelpData *get_help(CharData * ch, char *argument)
{
        char      argall[MaxInputLength];
        char      argone[MaxInputLength];
        char      argnew[MaxInputLength];
        HelpData *pHelp;
        int       lev;

        if (argument[0] == '\0')
        {
                mudstrlcpy(argnew, "summary", MaxInputLength);
                argument = argnew;
        }

        if (isdigit(argument[0]))
        {
                lev = number_argument(argument, argnew);
                argument = argnew;
        }
        else
                lev = -2;
        /*
         * Tricky argument handling so 'help a b' doesn't match a.
         */
        argall[0] = '\0';
        while (argument[0] != '\0')
        {
                argument = one_argument(argument, argone);
                if (argall[0] != '\0')
                        mudstrlcat(argall, " ", MIL);
                mudstrlcat(argall, argone, MIL);
        }

        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                if (pHelp->level > get_trust(ch))
                        continue;
                if (lev != -2 && pHelp->level != lev)
                        continue;

                if (is_name(argall, pHelp->keyword))
                        return pHelp;
        }

        return NULL;
}


/*  Ranks by number of matches between two whole words. Coded for the Similar Helpfiles */
/*  Snippet by Senir.*/
sh_int str_similarity(const char *astr, const char *bstr)
{
        sh_int    matches = 0;

        if (!astr || !bstr)
                return matches;

/*    for ( ; *astr; astr++) - Gavin */
        for (;;)
        {
                if (astr == NULL || bstr == NULL)
                        break;
                astr++;

                if (Lower((*astr)) == Lower((*bstr)))
                        matches++;
                ++bstr;

                if ((*astr) == '\0' || (*bstr) == '\0')
                        break;
        }

        return matches;
}

/*  Ranks by number of matches until there's a nonmatching character between two words.*/
/*  Coded for the Similar Helpfiles Snippet by Senir.*/
sh_int str_prefix_level(const char *astr, const char *bstr)
{
        sh_int    matches = 0;

        if (!astr || !bstr)
                return matches;

        for (; *astr; astr++)
        {
                if (Lower(*astr) == Lower(*bstr))
                        matches++;
                else
                        return matches;

                ++bstr;
                if (*bstr == '\0')
                        return matches;
        }

        return matches;
}

/* Main function of Similar Helpfiles Snippet by Senir. It loops through all of the*/
/* helpfiles, using the string matching function defined to find the closest matching*/
/* helpfiles to the argument. It then checks for singles. Then, if matching helpfiles*/
/* are found at all, it loops through and prints out the closest matching helpfiles.*/
/* If its a single(there's only one), it opens the helpfile.*/
void similar_help_files(CharData * ch, char *argument)
{
        HelpData *pHelp = NULL;
        char      buf[MaxStringLength];
        char     *extension;
        sh_int    lvl = 0;
        bool      single = FALSE;


        send_to_pager_color("&C&BSimilar Help Files:\n\r", ch);

        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                buf[0] = '\0';
                extension = pHelp->keyword;

                if (pHelp->level > get_trust(ch))
                        continue;

                while (extension[0] != '\0')
                {
                        extension = one_argument(extension, buf);

                        if (str_similarity(argument, buf) > lvl)
                        {
                                lvl = str_similarity(argument, buf);
                                single = TRUE;
                        }
                        else if (str_similarity(argument, buf) == lvl
                                 && lvl > 0)
                        {
                                single = FALSE;
                        }
                }
        }

        if (lvl == 0)
        {
                send_to_pager_color("&C&GNo similar help files.\n\r", ch);
                return;
        }

        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                buf[0] = '\0';
                extension = pHelp->keyword;

                while (extension[0] != '\0')
                {
                        extension = one_argument(extension, buf);

                        if (str_similarity(argument, buf) >= lvl
                            && pHelp->level <= get_trust(ch))
                        {
                                if (single)
                                {
                                        send_to_pager_color
                                                ("&C&GOpening only similar helpfile.&C\n\r",
                                                 ch);
                                        do_help(ch, buf);
                                        return;
                                }

                                pager_printf(ch, "&C&G   %s\n\r",
                                             pHelp->keyword);
                                break;

                        }

                }
        }
        return;
}




/*
 * Now this is cleaner
 *
CMDF do_help(CharData * ch, char *argument)
{
        HelpData *pHelp;
        int       skill_number;
        CMDType  *command;

        if ((pHelp = get_help(ch, argument)) == NULL)
        {
                append_file(ch, HelpFile, argument);

                ch_printf(ch, "&C&wNo help on \'%s\' found.\n\r", argument);
		if ( !is_number(argument)) - I don't think this works... - Gavin
			similar_help_files(ch, argument); 
                return;
        }
        set_pager_color(AtYellow, ch);
        if (pHelp->level >= 0 && str_cmp(argument, "imotd"))
        {
                send_to_pager(pHelp->keyword, ch);
                send_to_pager("\n\r", ch);
        }

        sound_to_char(ch, "help");
        if ((skill_number = skill_lookup(argument)) >= 0 &&
            nifty_is_name(skill_table[skill_number]->name, pHelp->keyword))
                pager_printf(ch, "Skill level: %d\n\r",
                             skill_table[skill_number]->min_level);
        if ((command = find_command(argument)) != NULL
            && nifty_is_name(command->name, pHelp->keyword))
                pager_printf(ch, "Command level: %d\n\r", command->level);

        set_pager_color(AtHelp, ch);
        if (pHelp->text[0] == '.')
                send_to_pager(pHelp->text + 1, ch);
        else
                send_to_pager(pHelp->text, ch);

        if (pHelp->author != NULL && pHelp->level >= 0 &&
            pHelp->date != NULL && str_cmp(argument, "imotd") &&
            str_cmp(argument, "amotd") && str_cmp(argument, "_diemsg_"))
        {
                send_to_pager("&B[ E&zdited by&B ]&R&W:&w", ch);
                send_to_pager(pHelp->author, ch);
                send_to_pager("     &B[ L&zast Edited on&B ]&R&W:&w ", ch);
                send_to_pager(pHelp->date, ch);
        }
        send_to_pager("\n\r", ch);
        return;
}*/

// =============================================================================
// HELP SYSTEM COMMANDS
// =============================================================================

/*
 * Help command - Updated version provided by Remcon of The Lands of Pabulum 03/20/2004
 */
void do_help(CharData* ch, char* argument)
{
        HelpData *pHelp;
        char     *keyword;
        char      oneword[MSL], lastmatch[MSL];
        sh_int    matched = 0, checked = 0, totalmatched = 0, found = 0;

        set_pager_color(AtHelp, ch);

        if (!argument || argument[0] == '\0')
        {
                static char help_default[] = "help";
                argument = help_default;
        }

        if (!(pHelp = get_help(ch, argument)))
        {
                pager_printf(ch, "&wNo help on \'%s\' found.&D\n\r",
                             argument);
                send_to_pager("&BSuggested Help Files:&D\n\r", ch);
                mudstrlcpy(lastmatch, " ", MSL);
                for (pHelp = first_help; pHelp; pHelp = pHelp->next)
                {
                        matched = 0;
                        if (!pHelp || !pHelp->keyword
                            || pHelp->keyword[0] == '\0'
                            || pHelp->level > get_trust(ch))
                                continue;
                        keyword = pHelp->keyword;
                        while (keyword && keyword[0] != '\0')
                        {
                                matched = 0;    /* Set to 0 for each time we check lol */
                                keyword = one_argument(keyword, oneword);
                                /*
                                 * Lets check only up to 10 spots 
                                 */
                                for (checked = 0; checked <= 10; checked++)
                                {
                                        if (!oneword[checked]
                                            || !argument[checked])
                                                break;
                                        if (Lower(oneword[checked]) ==
                                            Lower(argument[checked]))
                                                matched++;
                                }
                                if ((matched > 1 && matched > (checked / 2))
                                    || (matched > 0 && checked < 2))
                                {
                                        pager_printf(ch, "&G %-20s &D\n\r",
                                                     oneword);
                                        mudstrlcpy(lastmatch, oneword, MSL);
                                        totalmatched++;
                                        break;
                                }
                        }
                }
                if (totalmatched == 0)
                {
                        send_to_pager("&C&GNo suggested help files.\n\r", ch);
                        return;
                }
                if (totalmatched == 1 && lastmatch[0] != '\0')
                {
                        send_to_pager
                                ("&COpening only suggested helpfile.&D\n\r",
                                 ch);
                        do_help(ch, lastmatch);
                        return;
                }
                if (found > 0 && found <= 3)
                        send_to_pager("\n\r", ch);
                return;
        }

        if (IsImmortal(ch))
                pager_printf(ch, "Help level: %d\n\r", pHelp->level);

        set_pager_color(AtHelp, ch);

        /*
         * Strip leading '.' to allow initial blanks.
         */
        if (pHelp->text[0] == '.')
                send_to_pager(pHelp->text + 1, ch);
        else
                send_to_pager(pHelp->text, ch);

        if (pHelp->author != NULL && pHelp->level >= 0 &&
            pHelp->date != NULL && str_cmp(argument, "imotd") &&
            str_cmp(argument, "amotd") && str_cmp(argument, "_diemsg_"))
        {
                send_to_pager("&B[ E&zdited by&B ]&R&W:&w", ch);
                send_to_pager(pHelp->author, ch);
                send_to_pager("     &B[ L&zast Edited on&B ]&R&W:&w ", ch);
                send_to_pager(pHelp->date, ch);
        }
}


/*
 * Help editor							-Thoric
 */
CMDF do_hedit(CharData * ch, char *argument)
{
        HelpData *pHelp;

        if (!ch->desc)
        {
                send_to_char("You have no descriptor.\n\r", ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                break;
        case SubHelpEdit:
                if ((pHelp = static_cast<HelpData *>(ch->dest_buf)) == NULL)
                {
                        bug("hedit: sub_help_edit: NULL ch->dest_buf", 0);
                        stop_editing(ch);
                        return;
                }
/* Why not hash the author, date, and keywords? Some will show up more than once, why not?
   Plus copy_buffer returns a STRALLOC, not a str_dup, so we have to hash text as well, waste of time, but whatever */
                STRFREE(pHelp->text);
                pHelp->text = copy_buffer(ch);
                stop_editing(ch);
                stralloc_printf(&pHelp->author, const_cast<char*>("%s"), ch->name);
                stralloc_printf(&pHelp->date, const_cast<char*>("%s"), ctime(&current_time));
/*	  pHelp->author  = ch->name;
      pHelp->date    = ctime( &current_time );*/
                return;
        }
        if ((pHelp = get_help(ch, argument)) == NULL)   /* new help */
        {
                char      argnew[MaxInputLength];
                int       lev;

                if (isdigit(argument[0]))
                {
                        lev = number_argument(argument, argnew);
                        argument = argnew;
                }
                else
                        lev = get_trust(ch);
                CREATE(pHelp, HelpData, 1);
                pHelp->keyword = STRALLOC(strupper(argument));
                pHelp->text = STRALLOC(const_cast<char*>(""));
                pHelp->level = static_cast<sh_int>(lev);
                stralloc_printf(&pHelp->author, const_cast<char*>("%s"), ch->name);
                stralloc_printf(&pHelp->date, const_cast<char*>("%s"), ctime(&current_time));
/*	pHelp->author  = ch->name;
	pHelp->date    = ctime( &current_time );*/
                add_help(pHelp);
        }
        ch->substate = SubHelpEdit;
        ch->dest_buf = pHelp;
        start_editing(ch, pHelp->text);
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
char     *help_fix(char *text)
{
        char     *fixed;
        static char empty[] = "";

        if (!text)
                return empty;
        fixed = strip_cr(text);
        if (fixed[0] == ' ')
                fixed[0] = '.';
        return fixed;
}

void save_help(void)
{
        FILE     *fpout;
        HelpData *pHelp;
        char      date[25];

        rename("help.are", "help.are.bak");
        FCLOSE(fpReserve);
        if ((fpout = fopen("help.are", "w")) == NULL)
        {
                bug("save_help: fopen", 0);
                perror("help.are");
                fpReserve = fopen(NullFile, "r");
                return;
        }

        fprintf(fpout, "#HELPS\n\n");
        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                /*
                 * Hackish attempt to remove the extra lines from the date strings) 
                 */
                snprintf(date, 25, "%s", pHelp->date);
                fprintf(fpout, "%d %s~\n%s~\n%s~\n%s~\n\n",
                        pHelp->level, pHelp->keyword, help_fix(pHelp->text),
                        pHelp->author, date);
        }
        fprintf(fpout, "0 $~\n\n\n#$\n");
        FCLOSE(fpout);
        fpReserve = fopen(NullFile, "r");

}
CMDF do_hset(CharData * ch, char *argument)
{
        HelpData *pHelp;
        char      arg1[MaxInputLength];

        smash_tilde(argument);
        argument = one_argument(argument, arg1);
        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: hset <help page> <field> [value]\n\r",
                             ch);
                send_to_char("\n\r", ch);
                send_to_char("Field being one of:\n\r", ch);
                send_to_char
                        ("  level keyword remove save show addkeyword\n\r",
                         ch);
                return;
        }

        if (!str_cmp(arg1, "save"))
        {
                log_string_plus("Saving help.are...", LogNormal,
                                LevelGreater);
                save_help();
                send_to_char("Saved.\n\r", ch);
                return;
        }

        if ((pHelp = get_help(ch, arg1)) == NULL)
        {
                send_to_char("Cannot find help on that subject.\n\r", ch);
                return;
        }


        if (!str_cmp(argument, "remove"))
        {
                /*
                 * Since we removed it from delete_help, we need this here 
                 */
                UNLINK(pHelp, first_help, last_help, next, prev);
                delete_help(pHelp);
                send_to_char("Removed.\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "show"))
        {
                ch_printf(ch, "&BL&zevel&B:&w %d\n\r", pHelp->level);
                ch_printf(ch, "&BK&zeywords&B:&w %s\n\r", pHelp->keyword);
                ch_printf(ch, "&BA&zuthor&B:&w %s\n\r", pHelp->author);
                ch_printf(ch, "&BD&zate&B:&w %s\n\r", pHelp->date);
                return;
        }

        argument = one_argument(argument, arg1);
        if (!str_cmp(arg1, "level"))
        {
                pHelp->level = static_cast<sh_int>(atoi(argument));
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg1, "keyword"))
        {
                STRFREE(pHelp->keyword);
                pHelp->keyword = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg1, "addkeyword"))
        {
                char      buf[MSL];

                snprintf(buf, MSL, "%s %s", pHelp->keyword, argument);
                STRFREE(pHelp->keyword);
                pHelp->keyword = STRALLOC(buf);
                send_to_char("Done.\n\r", ch);
                return;
        }

        do_hset(ch, const_cast<char*>(""));
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 */
CMDF do_hlist(CharData * ch, char *argument)
{
        int       min, max, minlimit, maxlimit, cnt;
        char      arg[MaxInputLength];
        HelpData *help;

        maxlimit = get_trust(ch);
        minlimit = maxlimit >= LevelGreater ? -1 : 0;
        argument = one_argument(argument, arg);
        if (arg[0] != '\0')
        {
                min = URange(minlimit, atoi(arg), maxlimit);
                if (argument[0] != '\0')
                        max = URange(min, atoi(argument), maxlimit);
                else
                        max = maxlimit;
        }
        else
        {
                min = minlimit;
                max = maxlimit;
        }
        set_pager_color(AtGreen, ch);
        pager_printf(ch, "Help Topics in level range %d to %d:\n\r\n\r", min,
                     max);
        for (cnt = 0, help = first_help; help; help = help->next)
                if (help->level >= min && help->level <= max)
                {
                        one_argument(help->keyword, arg);
                        if (IsMxp(ch))
                                pager_printf(ch,
                                             "   %3d " MXPTAG("help '%s'")
                                             "%s" MXPTAG("/help") "\n\r",
                                             help->level, arg, help->keyword);
                        else
                                pager_printf(ch, "   %3d %s\n\r",
                                             help->level, help->keyword);
                        ++cnt;
                }
        if (cnt)
                pager_printf(ch, "\n\r%d pages found.\n\r", cnt);
        else
                send_to_char("None found.\n\r", ch);
}

// =============================================================================
// PLAYER INFORMATION COMMANDS
// =============================================================================

/* 
 * New do_who with WHO REQUEST, clan, race and homepage support. -Thoric
 * Latest version eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals. -Narn, Oct/96
 */
CMDF do_who(CharData* ch, [[maybe_unused]] const char* argument)
{
        char      buf[MaxStringLength];
        char      invis_str[MaxInputLength];
        char      extra_title[MaxStringLength];
        char      race_text[MaxInputLength];
        char      ClanName[MaxInputLength];
        DescriptorData *d;
        int       iLevelLower;
        int       iLevelUpper;
        int       nMatch;
        bool      fImmortalOnly;
        FILE     *whoout = NULL;

        /*
         * #define WtImm    0;
         * #define WtMortal 1;
         */

        WhoData *cur_who = NULL;
        WhoData *next_who = NULL;
        WhoData *first_mortal = NULL;
        WhoData *first_newbie = NULL;
        WhoData *first_imm = NULL;

        (void)argument;
        /*
         * Set default arguments.
         */
        iLevelLower = 0;
        iLevelUpper = MaxLevel;
        fImmortalOnly = FALSE;


        /*
         * Parse arguments.
         */


        /*
         * Now find matching chars.
         */
        nMatch = 0;
        buf[0] = '\0';
        if (ch)
                set_pager_color(AtGreen, ch);
#ifdef WEB
        else
        {
                whoout = fopen(WhoFile, "w");
                if (whoout)
                {
                        fprintf(whoout,
                                "<html><head><title> Who List</title><style> pre { display: inline; }  a:link { background-color: inherit; text-decoration: none; color: inherit; }\n\ra:visited { background-color: inherit; text-decoration: none; color: inherit; }\n\ra:hover { background-color: inherit; text-decoration: none; font-weight: bold; color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; color: inherit; }</style></head>\n\r");
                        fprintf(whoout,
                                "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r");
                        fprintf(whoout, "<h1>Who List</h1><br>\n\r");
                        fprintf(whoout,
                                "<table cellpadding=0 cellspacing=0>\n\r");
                }
        }
#endif		

/* start from last to first to get it in the proper order */
        for (d = last_descriptor; d; d = d->prev)
        {
                CharData *wch;
                char const *race;

                wch = CH(d);
                if ((!IsPlaying(d) && d->connected != ConEditing)
                    || (!can_see_ooc(ch, wch)
                        && IsImmortal(wch)) || d->original)
                        continue;
                if (wch->top_level < iLevelLower
                    || wch->top_level > iLevelUpper
                    || (fImmortalOnly && wch->top_level < LevelImmortal))
                        continue;
                if (IsNpc(wch))
                        continue;


                /*****************************************
                 **	Showing Members Of Wch's Clan    	**
                 **	Using "ClanName" in who list    	**
                 ******************************************/
                ClanName[0] = '\0'; /* Reset this so it won't print on others */
                if ( ch && !IsNpc(ch) && wch->PCData->clan && (ch->PCData->clan || IsImmortal(ch)))
                {
                        ClanData *pclan;
                        ClanData *zclan;

                        if ( wch->PCData->clan ) 
                        {
                                pclan = wch->PCData->clan;
                                if (pclan->mainclan) pclan = pclan->mainclan;
                        }

                        if ( ch->PCData->clan ) { 
                                zclan = ch->PCData->clan;
                                if (zclan->mainclan) zclan = zclan->mainclan;
                        }

                        if ( IsImmortal(ch) || (pclan && zclan && pclan == zclan) )  
                        {
                                if ( !str_cmp( wch->name, wch->PCData->clan->leader ) )
                                {
                                        snprintf(ClanName, MSL, " &R(&BL&zeader,&B %c&z%s&R)&W",
                                                        wch->PCData->clan->name[0],
                                                        (wch->PCData->clan->name+1)
                                                );
                                }
                                else
                                {
                                        snprintf(ClanName, MSL, " &R(&B%c&z%s,&B %c&z%s&R)&W",
                                                        wch->PCData->clan->rank[wch->PCData->clanrank][0],
                                                        (wch->PCData->clan->rank[wch->PCData->clanrank]+1),
                                                        wch->PCData->clan->name[0],
                                                        (wch->PCData->clan->name+1)
                                                );
                                } 

                        } 
                }
                nMatch++;

                snprintf(race_text, MSL, "(%s) ", wch->race->name());
                race = race_text;

                switch (wch->top_level)
                {
                default:
                        break;
                case 200:
                        race = "The Ghost in the Machine";
                        break;
                case MaxLevel - 0:
                        race = "Owner";
                        break;
                case MaxLevel - 1:
                        race = "Admin";
                        break;
                case MaxLevel - 2:
                        race = "Head Builder";
                        break;
                case MaxLevel - 3:
                        race = "Builder";
                        break;
                case MaxLevel - 4:
                        race = "Enforcer";
                        break;
                }

                if (!ch && wch->PCData->homepage
                    && wch->PCData->homepage[0] != '\0')
                {
                        if (get_trust(ch) > get_trust(wch)
                            && !nifty_is_name(wch->name,
                                              smash_color(wch->PCData->
                                                          title)))
                                snprintf(extra_title, MSL,
                                         "<a href=\"%s\" target=_blank>%s [%s]</a>",
                                         wch->PCData->homepage,
                                         wch->PCData->title, wch->name);
                        else
                                snprintf(extra_title, MSL,
                                         "<a href=\"%s\" target=_blank>%s</a>",
                                         wch->PCData->homepage,
                                         wch->PCData->title);
                }
                else
                {
                        if (get_trust(ch) > get_trust(wch)
                            && !nifty_is_name(wch->name,
                                              smash_color(wch->PCData->
                                                          title)))
                                snprintf(extra_title, MSL, "%s [%s]",
                                         wch->PCData->title, wch->name);
                        else
                                mudstrlcpy(extra_title, wch->PCData->title,
                                           MSL);
                }


                if (IsRetired(wch))
                        race = "Retired";
                else if (IsGuest(wch))
                        race = "Guest";
                else if (IsSet(wch->PCData->flags, PcflagNewbguide))
                        race = "&b[&zNewbie Guide&b]&D";
                else if (wch->PCData->rank && wch->PCData->rank[0] != '\0')
                        race = wch->PCData->rank;


                if (IsSet(wch->act, PlrWizinvis))
                        snprintf(invis_str, 10, "(%d) ",
                                 wch->PCData->wizinvis);
                else
                        invis_str[0] = '\0';

                /* 
                 * Build the string safely in multiple steps to avoid buffer overflow 
                 */
                char safe_buf[MaxStringLength];
                int len = 0;
                
                /* Start with race and basic status info (limited to 100 chars) */
                len += snprintf(safe_buf + len, static_cast<size_t>(MaxStringLength - len), "%.100s &W%.10s%.20s%.20s&W", 
                         race, 
                         invis_str,
                         NotAuthed(wch) ? "&BN&W " : "",
                         IsSet(wch->act, PlrAfk) ? "[AFK] " : "");
                
                /* Add titles and clan info (limited to 200 chars) */
                len += snprintf(safe_buf + len, static_cast<size_t>(MaxStringLength - len), "%.200s%.200s", 
                         extra_title, ClanName);
                
                /* Add status flags (limited space) */
                len += snprintf(safe_buf + len, static_cast<size_t>(MaxStringLength - len), "%.50s%.50s%.50s&w",
                         IsSet(wch->PCData->flags, PcflagWorking) ? "&Y [&RWORKING&Y]&W" : "&W",
                         IsSet(wch->act, PlrSilence) ? "&Y [&BS&zilenced&Y]&W" : "&W",
                         wch->desc->connected == ConEditing ? "&Y [&cWRITING&Y]" : 
                         (wch->desc->connected == ConIaForked || 
                          wch->desc->connected == ConForked) ? "&Y [&cCOMPILING&Y]" : "");
                
                /* Copy the safely built string to the main buffer */
                snprintf(buf, MSL, "%s", safe_buf);

                /*
                 * This is where the old code would display the found player to the ch.
                 * What we do instead is put the found data into a linked list
                 */

                /*
                 * First make the structure. 
                 */
                CREATE(cur_who, WhoData, 1);
                cur_who->text = str_dup(buf);
                if (IsImmortal(wch))
                        cur_who->type = WtImm;
                else if (get_trust(wch) <= 10)
                        cur_who->type = WtNewbie;
                else
                        cur_who->type = WtMortal;

                /*
                 * Then put it into the appropriate list. 
                 */
                switch (cur_who->type)
                {
                case WtMortal:
                        cur_who->next = first_mortal;
                        first_mortal = cur_who;
                        break;
                case WtImm:
                        cur_who->next = first_imm;
                        first_imm = cur_who;
                        break;
                case WtNewbie:
                        cur_who->next = first_newbie;
                        first_newbie = cur_who;
                        break;
                default:
                        {   /* Really should be using enums */
                        }
                }

        }
        buf[0] = '\0';
        race_text[0] = '\0';

        /*
         * Ok, now we have three separate linked lists and what remains is to 
         * * display the information and clean up.
         */

        /*
         * Deadly list removed for swr ... now only 2 lists 
         */

        if (first_newbie)
        {
                mudstrlcpy(race_text,
                           "\n\r&B----------------------------&z[&w Dark Warrior Newbies &z]&B---------------------------&W\n\r\n\r",
                           MIL);
                if (ch)
                        send_to_pager(race_text, ch);
#ifdef WEB
                else
                {
                        web_colourconv(buf, race_text);
                        if (whoout)
                        {
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                        }
                }
#endif
		}

        for (cur_who = first_newbie; cur_who; cur_who = next_who)
        {
#ifdef WEB
                if (!ch)
                {
                        web_colourconv(buf, cur_who->text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
                else
#endif
                {
                        send_to_pager(cur_who->text, ch);
                        send_to_pager("\n\r", ch);
                }
                next_who = cur_who->next;
                DISPOSE(cur_who->text);
                DISPOSE(cur_who);
        }


        if (first_mortal)
        {
                mudstrlcpy(race_text,
                           "\n\r&B----------------------------&z[&w Dark Warrior Players &z]&B---------------------------&W\n\r\n\r",
                           MIL);
                if (ch)
                        send_to_pager(race_text, ch);
#ifdef WEB
                else
                {
                        web_colourconv(buf, race_text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
#endif
        }

        for (cur_who = first_mortal; cur_who; cur_who = next_who)
        {
#ifdef WEB
                if (!ch)
                {
                        web_colourconv(buf, cur_who->text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
                else
#endif
                {
                        send_to_pager(cur_who->text, ch);
                        send_to_pager("\n\r", ch);
                }
                next_who = cur_who->next;
                DISPOSE(cur_who->text);
                DISPOSE(cur_who);
        }

        if (first_imm)
        {
                mudstrlcpy(race_text,
                           "\n\r&B----------------------------&z[&w Dark Warrior Immortals &z]&B-------------------------&W\n\r\n\r",
                           MIL);
                if (ch)
                        send_to_pager(race_text, ch);
#ifdef WEB
				else
                {
                        web_colourconv(buf, race_text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
#endif
        }

        for (cur_who = first_imm; cur_who; cur_who = next_who)
        {
#ifdef WEB
                if (!ch)
                {
                        web_colourconv(buf, cur_who->text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
                else
#endif
                {
                        send_to_pager(cur_who->text, ch);
                        send_to_pager("\n\r", ch);
                }
                next_who = cur_who->next;
                DISPOSE(cur_who->text);
                DISPOSE(cur_who);
        }
#ifdef WEB
        if (!ch && whoout)
        {
                fprintf(whoout, "</table>\n\r");
                fprintf(whoout,
                        "<font color=#FFFF00><pre> %d player%s</pre></font>\n",
                        nMatch, nMatch == 1 ? "" : "s");
                fprintf(whoout, "</body></html>\n\n");
                FCLOSE(whoout);
        }
#endif
        if (!ch)
                return;

        set_char_color(AtYellow, ch);
        ch_printf(ch, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s");
        return;
}


CMDF do_compare(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        ObjData *obj1;
        ObjData *obj2;
        int       value1;
        int       value2;
        const char *msg;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if (arg1[0] == '\0')
        {
                send_to_char("Compare what to what?\n\r", ch);
                return;
        }

        if ((obj1 = get_obj_carry(ch, arg1)) == NULL)
        {
                send_to_char("You do not have that item.\n\r", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                for (obj2 = ch->first_carrying; obj2;
                     obj2 = obj2->next_content)
                {
                        if (obj2->wear_loc != WearNone
                            && can_see_obj(ch, obj2)
                            && obj1->item_type == obj2->item_type
                            && (obj1->wear_flags & obj2->
                                wear_flags & ~ItemTake) != 0)
                                break;
                }

                if (!obj2)
                {
                        send_to_char
                                ("You aren't wearing anything comparable.\n\r",
                                 ch);
                        return;
                }
        }
        else
        {
                if ((obj2 = get_obj_carry(ch, arg2)) == NULL)
                {
                        send_to_char("You do not have that item.\n\r", ch);
                        return;
                }
        }

        msg = NULL;
        value1 = 0;
        value2 = 0;

        if (obj1 == obj2)
        {
                msg = "You compare $p to itself.  It looks about the same.";
        }
        else if (obj1->item_type != obj2->item_type)
        {
                msg = "You can't compare $p and $P.";
        }
        else
        {
                switch (obj1->item_type)
                {
                default:
                        msg = "You can't compare $p and $P.";
                        break;

                case ItemArmor:
                        value1 = obj1->value[0];
                        value2 = obj2->value[0];
                        break;

                case ItemWeapon:
                        value1 = obj1->value[1] + obj1->value[2];
                        value2 = obj2->value[1] + obj2->value[2];
                        break;
                }
        }

        if (!msg)
        {
                if (value1 == value2)
                        msg = "$p and $P look about the same.";
                else if (value1 > value2)
                        msg = "$p looks better than $P.";
                else
                        msg = "$p looks worse than $P.";
        }

        act(AtPlain, msg, ch, obj1, obj2, ToChar);
        return;
}



CMDF do_where(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        DescriptorData *d;
        bool      found;

        if (get_trust(ch) < LevelImmortal)
        {
                send_to_char("If only life were really that simple...\n\r",
                             ch);
                return;
        }

        one_argument(argument, arg);

        set_pager_color(AtPerson, ch);
        if (arg[0] == '\0')
        {
                if (get_trust(ch) >= LevelImmortal)
                        send_to_pager("Players logged in:\n\r", ch);
                else
                        pager_printf(ch, "Players near you in %s:\n\r",
                                     ch->in_room->area->name);
                found = FALSE;
                for (d = first_descriptor; d; d = d->next)
                        if ((IsPlaying(d) || d->connected == ConEditing)
                            && (victim = d->character) != NULL
                            && !IsNpc(victim)
                            && victim->in_room
                            && (victim->in_room->area == ch->in_room->area
                                || get_trust(ch) >= LevelImmortal)
                            && can_see(ch, victim))
                        {
                                found = TRUE;
                                pager_printf(ch, "%-28s %s\n\r",
                                             victim->name,
                                             victim->in_room->name);
                        }
                if (!found)
                        send_to_char("None\n\r", ch);
        }
        else
        {
                found = FALSE;
                for (victim = first_char; victim; victim = victim->next)
                        if (victim->in_room
                            && victim->in_room->area == ch->in_room->area
                            && !IsAffected(victim, AffHide)
                            && !IsAffected(victim, AffSneak)
                            && can_see(ch, victim)
                            && is_name(arg, victim->name))
                        {
                                found = TRUE;
                                pager_printf(ch, "%-28s %s\n\r",
                                             PERS(victim, ch),
                                             victim->in_room->name);
                                break;
                        }
                if (!found)
                        act(AtPlain, "You didn't find any $T.", ch, NULL,
                            arg, ToChar);
        }

        return;
}




CMDF do_consider(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        const char *msg;
        int       diff;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Consider killing whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }

        diff = (victim->top_level - ch->top_level) * 10;

        diff += static_cast<int>(victim->max_hit - ch->max_hit) / 10;

        if (diff <= -200)
                msg = "$N looks like a feather!";
        else if (diff <= -150)
                msg = "Hey! Where'd $N go?";
        else if (diff <= -100)
                msg = "Easy as picking off womp rats at beggers canyon!";
        else if (diff <= -50)
                msg = "$N is a wimp.";
        else if (diff <= 0)
                msg = "$N looks weaker than you.";
        else if (diff <= 50)
                msg = "$N looks about as strong as you.";
        else if (diff <= 100)
                msg = "It would take a bit of Luck...";
        else if (diff <= 150)
                msg = "It would take a lot of Luck, and a really big blaster!";
        else if (diff <= 200)
                msg = "Why don't you just attack a star destoyer with a vibroblade?";
        else
                msg = "$N is built like an AT-AT!";
        act(AtConsider, msg, ch, NULL, victim, ToChar);

        return;
}

// =============================================================================
// SKILL AND TRAINING COMMANDS
// =============================================================================

/*
 * Practice command - practice skills
 */
CMDF do_practice(CharData* ch, char* argument)
{
        char      buf[MaxStringLength];
        char      arg[MaxStringLength];
        int       sn, classtype = -1, iClass;
        bool      parts = FALSE;

        if (IsNpc(ch))
                return;

        one_argument(argument, arg);
        if (argument[0] == '\0' || !str_cmp("class", arg))
        {
                int       col;
                sh_int    lasttype, cnt;

                if (!str_cmp("class", arg))
                {
                        argument = one_argument(argument, arg); /* Strip away the word class into arg */
                        argument = one_argument(argument, arg); /* Fill arg with class name, what we want */
                        if (arg[0] != '\0')
                        {
                                for (iClass = 0; iClass < MaxAbility;
                                     iClass++)
                                {
                                        if (!str_prefix
                                            (arg, ability_name[iClass]))
                                        {
                                                classtype = iClass;
                                                break;
                                        }
                                }
                        }
                }
                col = cnt = 0;
                lasttype = SkillSpell;
                set_pager_color(AtMagic, ch);
                for (sn = 0; sn < top_sn; sn++)
                {
                        parts = FALSE;
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                                continue;

                        if (strcmp(skill_table[sn]->name, "reserved") == 0
                            && (IsImmortal(ch)))
                        {
                                if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                send_to_pager
                                        ("&B--------------------------------[&zSpells&B]---------------------------------\n\r",
                                         ch);
                                send_to_pager("&w", ch);
                                col = 0;
                        }
                        if (skill_table[sn]->type != lasttype)
                        {
                                if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                if (skill_table[sn]->type != SkillUnknown)
                                        pager_printf(ch,
                                                     "&B--------------------------------&B[&z%ss&B]---------------------------------\n\r",
                                                     skill_tname[skill_table
                                                                 [sn]->type]);
                                col = cnt = 0;
                        }

                        lasttype = skill_table[sn]->type;

                        if (skill_table[sn]->races
                            && skill_table[sn]->races[0] != '\0')
                        {
                                snprintf(buf, MSL, "%s", ch->race->name());
                                if (!is_name(buf, skill_table[sn]->races))
                                {
                                        continue;
                                }
                        }
                        if (!IsEmpty(skill_table[sn]->body_parts)
                            && !IsEmpty(ch->xflags))
                        {
                                sh_int    i = 0;

                                for (i = 0; i < MaxBits; i++)
                                        if (IsSet
                                            (skill_table[sn]->body_parts, i)
                                            && IsSet(ch->xflags, i))
                                                parts = TRUE;
                        }
                        if (!IsEmpty(skill_table[sn]->body_parts)
                            && parts == FALSE)
                                continue;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                                continue;

                        if (classtype != -1
                            && skill_table[sn]->guild != classtype)
                                continue;

                        if (ch->PCData->learned[sn] <= 0
                            && ch->skill_level[skill_table[sn]->guild] <
                            skill_table[sn]->min_level)
                                continue;

                        if (ch->PCData->learned[sn] == 0
                            && SpellFlag(skill_table[sn], SfSecretskill))
                                continue;

                        ++cnt;
                        if (ch->PCData->learned[sn] >= 100)
                        {
                                if (IsMxp(ch))
                                {
                                        pager_printf(ch,
                                                     "&w"
                                                     MXPTAG("practice %s")
                                                     "%18.18s"
                                                     MXPTAG("/practice")
                                                     " &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     skill_table[sn]->name,
                                                     ch->PCData->learned[sn]);
                                }
                                else
                                {
                                        pager_printf(ch,
                                                     "&w%18.18s &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     ch->PCData->learned[sn]);
                                }
                        }
                        else
                        {
                                if (IsMxp(ch))
                                {
                                        pager_printf(ch,
                                                     "&z"
                                                     MXPTAG("practice %s")
                                                     "%18.18s"
                                                     MXPTAG("/practice")
                                                     " &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     skill_table[sn]->name,
                                                     ch->PCData->learned[sn]);
                                }
                                else
                                {
                                        pager_printf(ch,
                                                     "&z%18.18s &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     ch->PCData->learned[sn]);
                                }
                        }
                        if (++col % 3 == 0)
                                send_to_pager("\n\r", ch);
                }

                if (col % 3 != 0)
                        send_to_pager("\n\r", ch);

        }
        else
        {
                CharData *mob;
                int       adept;
                bool      can_prac = TRUE;

                if (!IsAwake(ch))
                {
                        send_to_char("In your dreams, or what?\n\r", ch);
                        return;
                }

                for (mob = ch->in_room->first_person; mob;
                     mob = mob->next_in_room)
                        if (IsNpc(mob) && IsSet(mob->act, ActPractice))
                                break;

                if (!mob)
                {
                        send_to_char("You can't do that here.\n\r", ch);
                        return;
                }


                sn = skill_lookup(argument);

                if (sn == -1)
                {
                        act(AtTell,
                            "$n tells you 'I've never heard of that one...'",
                            mob, NULL, ch, ToVict);
                        return;
                }

                if (skill_table[sn]->guild < 0
                    || skill_table[sn]->guild >= MaxAbility)
                {
                        act(AtTell,
                            "$n tells you 'I cannot teach you that...'", mob,
                            NULL, ch, ToVict);
                        return;
                }

                if (can_prac && !IsNpc(ch)
                    && ch->skill_level[skill_table[sn]->guild] <
                    skill_table[sn]->min_level)
                {
                        act(AtTell,
                            "$n tells you 'You're not ready to learn that yet...'",
                            mob, NULL, ch, ToVict);
                        return;
                }

                if (is_name(skill_tname[skill_table[sn]->type], static_cast<char*>(const_cast<char*>(CantPrac))))
                {
                        act(AtTell,
                            "$n tells you 'I do not know how to teach that.'",
                            mob, NULL, ch, ToVict);
                        return;
                }

                /*
                 * Skill requires a special teacher
                 */
                if (skill_table[sn]->teachers
                    && skill_table[sn]->teachers[0] != '\0')
                {
                        snprintf(buf, MSL, "%d", mob->pIndexData->vnum);
                        if (!is_name(buf, skill_table[sn]->teachers))
                        {
                                act(AtTell,
                                    "$n tells you, 'I know not know how to teach that.'",
                                    mob, NULL, ch, ToVict);
                                return;
                        }
                }
                else
                {
                        act(AtTell,
                            "$n tells you, 'I know not know how to teach that.'",
                            mob, NULL, ch, ToVict);
                        return;
                }

                if (skill_table[sn]->races
                    && skill_table[sn]->races[0] != '\0')
                {
                        snprintf(buf, MSL, "%s", ch->race->name());
                        if (!is_name(buf, skill_table[sn]->races))
                        {
                                act(AtTell,
                                    "$n tells you, 'You are not the right race to learn that skill.'",
                                    mob, NULL, ch, ToVict);
                                return;
                        }
                }


                adept = 20;

                if (ch->gold < skill_table[sn]->min_level * 10)
                {
                        snprintf(buf, MSL,
                                 "$n tells you, 'I charge %d credits to teach that. You don't have enough.'",
                                 skill_table[sn]->min_level * 10);
                        act(AtTell,
                            "$n tells you 'You don't have enough credits.'",
                            mob, NULL, ch, ToVict);
                        return;
                }

                if (ch->PCData->learned[sn] >= adept)
                {
                        snprintf(buf, MSL,
                                 "$n tells you, 'I've taught you everything I can about %s.'",
                                 skill_table[sn]->name);
                        act(AtTell, buf, mob, NULL, ch, ToVict);
                        act(AtTell,
                            "$n tells you, 'You'll have to practice it on your own now...'",
                            mob, NULL, ch, ToVict);
                }
                else
                {
                        ch->gold -= skill_table[sn]->min_level * 10;
                        ch->PCData->learned[sn] +=
                                IntApp[get_curr_int(ch)].learn;
                        act(AtAction, "You practice $T.", ch, NULL,
                            skill_table[sn]->name, ToChar);
                        act(AtAction, "$n practices $T.", ch, NULL,
                            skill_table[sn]->name, ToRoom);
                        if (ch->PCData->learned[sn] >= adept)
                        {
                                ch->PCData->learned[sn] = static_cast<sh_int>(adept);
                                act(AtTell,
                                    "$n tells you. 'You'll have to practice it on your own now...'",
                                    mob, NULL, ch, ToVict);
                        }
                }
        }
        return;
}

CMDF do_teach(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        int       sn;
        char      arg[MaxInputLength];

        if (IsNpc(ch))
                return;

        switch (ch->tempnum)
        {
        default:
                argument = one_argument(argument, arg);

                if (argument[0] == '\0')
                {
                        send_to_char("Teach who, what?\n\r", ch);
                        return;
                }
                else
                {
                        CharData *victim;
                        int       adept;

                        if (!IsAwake(ch))
                        {
                                send_to_char("In your dreams, or what?\n\r",
                                             ch);
                                return;
                        }

                        if ((victim = get_char_room(ch, arg)) == NULL)
                        {
                                send_to_char
                                        ("They don't seem to be here...\n\r",
                                         ch);
                                return;
                        }

                        if (IsNpc(victim))
                        {
                                send_to_char
                                        ("You can't teach that to them!\n\r",
                                         ch);
                                return;
                        }

                        sn = skill_lookup(argument);

                        if (sn == -1)
                        {
                                act(AtTell, "You have no idea what that is.",
                                    victim, NULL, ch, ToVict);
                                return;
                        }

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                        {
                                act(AtTell,
                                    "Thats just not going to happen.", victim,
                                    NULL, ch, ToVict);
                                return;
                        }

                        if (is_name
                            (skill_tname[skill_table[sn]->type], static_cast<char*>(const_cast<char*>(CantPrac))))
                        {
                                act(AtTell,
                                    "You are unable to teach that skill.",
                                    victim, NULL, ch, ToVict);
                                return;
                        }

                        if (skill_table[sn]->races
                            && skill_table[sn]->races[0] != '\0')
                        {
                                snprintf(buf, MSL, "%s",
                                         victim->race->name());
                                if (!is_name(buf, skill_table[sn]->races))
                                {
                                        act(AtTell,
                                            "They are not the right race to learn that skill.",
                                            victim, NULL, ch, ToVict);
                                        return;
                                }
                        }

                        if (ch->PCData->learned[sn] < 100)
                        {
                                act(AtTell,
                                    "You must perfect that yourself before teaching others.",
                                    victim, NULL, ch, ToVict);
                                return;
                        }

                        add_request(ch, victim, argument, const_cast<char*>("teach"));
                        return;

        case TRUE: // Can't be reached without consent.
                        victim = static_cast<CharData *>(ch->dest_buf);    // Should exist, this is a triggered response caused by victim.
                        sn = skill_lookup(argument);    // argument has already been parsed, should be fine.

                        if (victim->skill_level[skill_table[sn]->guild] <
                            skill_table[sn]->min_level)
                        {
                                act(AtTell,
                                    "$n isn't ready to learn that yet.",
                                    victim, NULL, ch, ToVict);
                                return;
                        }



                        adept = 20;

                        if (victim->PCData->learned[sn] >= adept)
                        {
                                act(AtTell,
                                    "$n must practice that on their own.",
                                    victim, NULL, ch, ToVict);
                                return;
                        }
/*                else if (!str_cmp(ch->race->name(), "verpine"))
                {
                        victim->PCData->learned[sn] += 99;
                        snprintf(buf, MSL, "You teach %s $T.", victim->name);
                        act(AtAction, buf,
                            ch, NULL, skill_table[sn]->name, ToChar);
                        snprintf(buf, MSL, "%s teaches you $T.", ch->name);
                        act(AtAction, buf,
                            victim, NULL, skill_table[sn]->name, ToChar);
                }*/
                        else
                        {
                                victim->PCData->learned[sn] +=
                                        IntApp[get_curr_int(ch)].learn;
                                snprintf(buf, MSL, "You teach %s $T.",
                                         victim->name);
                                act(AtAction, buf, ch, NULL,
                                    skill_table[sn]->name, ToChar);
                                snprintf(buf, MSL, "%s teaches you $T.",
                                         ch->name);
                                act(AtAction, buf, victim, NULL,
                                    skill_table[sn]->name, ToChar);
                        }
                }
                return;
        }
}


CMDF do_wimpy(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       wimpy;

        one_argument(argument, arg);

        if (arg[0] == '\0')
                wimpy = static_cast<int>(ch->max_hit) / 5;
        else
                wimpy = atoi(arg);

        if (wimpy < 0)
        {
                send_to_char("Your courage exceeds your Wisdom.\n\r", ch);
                return;
        }

        if (wimpy > ch->max_hit)
        {
                send_to_char("Such cowardice ill becomes you.\n\r", ch);
                return;
        }

        ch->wimpy = static_cast<sh_int>(wimpy);
        ch_printf(ch, "Wimpy set to %d hit points.\n\r", wimpy);
        return;
}

// =============================================================================
// USER ACCOUNT COMMANDS
// =============================================================================

/*
 * Password command - change player password
 */
CMDF do_password(CharData* ch, char* argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char     *pArg;
        char     *p;
        char      cEnd;

        if (IsNpc(ch))
                return;

        /*
         * Can't use one_argument here because it smashes case.
         * So we just steal all its code.  Bleagh.
         */
        pArg = arg1;
        while (isspace(*argument))
                argument++;

        cEnd = ' ';
        if (*argument == '\'' || *argument == '"')
                cEnd = *argument++;

        while (*argument != '\0')
        {
                if (*argument == cEnd)
                {
                        argument++;
                        break;
                }
                *pArg++ = *argument++;
        }
        *pArg = '\0';

        pArg = arg2;
        while (isspace(*argument))
                argument++;

        cEnd = ' ';
        if (*argument == '\'' || *argument == '"')
                cEnd = *argument++;

        while (*argument != '\0')
        {
                if (*argument == cEnd)
                {
                        argument++;
                        break;
                }
                *pArg++ = *argument++;
        }
        *pArg = '\0';

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Syntax: password <old> <new>.\n\r", ch);
                return;
        }

        if (!verify_password(arg1, ch->PCData->pwd))
        {
                WaitState(ch, 40);
                send_to_char("Wrong password.  Wait 10 seconds.\n\r", ch);
                return;
        }

        if (strlen(arg2) < MinPasswordLength) {
                send_to_char
                        ("New password must be at least five characters long.\n\r",
                         ch);
                return;
        }
        if (arg1[0] == '!' || arg2[0] == '!')
        {
                send_to_char
                        ("New password cannot begin with the '!' character.",
                         ch);
                return;
        }

        /*
         * No tilde allowed because of player file format.
         */
        for (p = arg2; *p != '\0'; p++)
        {
                if (*p == '~')
                {
                        send_to_char
                                ("New password not acceptable, try again.\n\r",
                                 ch);
                        return;
                }
        }
        
        // Generate a strong Argon2 hash
        std::string new_hash = hash_password(arg2);

        DISPOSE(ch->PCData->pwd);
        ch->PCData->pwd = str_dup(new_hash.c_str());
        if (IsSet(sysdata.save_flags, SvPasschg))
                save_char_obj(ch);
        send_to_char("Ok.\n\r", ch);
        return;
}

CMDF do_ls(CharData * ch, [[maybe_unused]] const char *argument)
{
        send_to_char("This isn't your terminal dumbass.\n\r", ch);
        return;
}

// =============================================================================
// LIST AND REFERENCE COMMANDS
// =============================================================================

/*
 * Socials command - list available social commands
 */
CMDF do_socials(CharData* ch, char* argument)
{
        int       iHash;
        int       col = 0;
        SocialType *social;

        (void)argument;

        set_pager_color(AtPlain, ch);
        for (iHash = 0; iHash < 27; iHash++)
                for (social = social_index[iHash]; social;
                     social = social->next)
                {
                        if (social->minarousal == 0)
                        {
                                pager_printf(ch, "%-12s", social->name);
                                if (++col % SocialColumns == 0)
                                        send_to_pager("\n\r", ch);
                        }
                }

        if (col % SocialColumns != 0)
                send_to_pager("\n\r", ch);
        return;
}



CMDF do_commands(CharData * ch, char *argument)
{
        int       col;
        int       hash;
        CMDType  *command;

        (void)argument;

        col = 0;
        set_pager_color(AtPlain, ch);
        for (hash = 0; hash < 126; hash++)
                for (command = command_hash[hash]; command;
                     command = command->next)
                        if (command->level < LevelHero
                            && command->level <= get_trust(ch)
                            && (command->name[0] != 'm'
                                || command->name[1] != 'p'))
                        {
                                pager_printf(ch,
                                             "&B[&w%-3d&B][&z%s&B]&z %-12.12s",
                                             command->level, get_help(ch,
                                                                      command->
                                                                      name) ?
                                             "H" : " ", command->name);
                                if (++col % CommandColumns == 0)
                                        send_to_pager("\n\r", ch);
                        }
        if (col % CommandColumns != 0)
                send_to_pager("\n\r", ch);

        return;
}

/*
 * display WIZLIST file						-Thoric
 */
CMDF do_wizlist(CharData * ch, char *argument)
{
        (void)argument;
        set_pager_color(AtImmort, ch);
        show_file(ch, WizlistFile);
}

CMDF do_showhelp(CharData * ch, char *argument)
{
        (void)argument;
        set_pager_color(AtImmort, ch);
        show_file(ch, HelpFile);
}

CMDF do_showlog(CharData * ch, char *argument)
{
        bool      clear = FALSE;
        char      arg[MaxInputLength];

        argument = one_argument(argument, arg);
        set_pager_color(AtImmort, ch);

        if (arg[0] == '\0')
        {
                send_to_char("Show which log file?.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "clear"))
        {
                clear = TRUE;
                argument = one_argument(argument, arg);
        }

        if (!str_cmp(arg, "help"))
        {
                if (clear)
                        clear_file(ch, const_cast<char*>(HelpFile));
                else
                        show_file(ch, HelpFile);
        }
        else if (!str_cmp(arg, "boot"))
        {
                if (clear)
                        clear_file(ch, const_cast<char*>(BootlogFile));
                else
                        show_file(ch, BootlogFile);
        }
        else if (!str_cmp(arg, "usage"))
        {
                if (clear)
                        clear_file(ch, const_cast<char*>(UsageFile));
                else
                        show_file(ch, UsageFile);
        }
        else if (!str_cmp(arg, "log"))
        {
                if (clear)
                        clear_file(ch, const_cast<char*>(LogFile));
                else
                        show_file(ch, LogFile);
        }
        else if (!str_cmp(arg, "bug"))
        {
                if (clear)
                        clear_file(ch, const_cast<char*>(BugsFile));
                else
                        show_file(ch, BugsFile);
        }
        else if (!str_cmp(arg, "idea"))
        {
                if (clear)
                        clear_file(ch, const_cast<char*>(IdeaFile));
                else
                        show_file(ch, IdeaFile);
        }
        else if (!str_cmp(arg, "typo"))
        {
                if (clear)
                        clear_file(ch, const_cast<char*>(TypoFile));
                else
                        show_file(ch, TypoFile);
        }
        else
        {
                send_to_char("Unknown File.\n\r", ch);
        }
        return;

}

/*
 * Contributed by Grodyn.
 */
CMDF do_config(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];

        if (IsNpc(ch))
                return;

        one_argument(argument, arg);

        set_char_color(AtWhite, ch);
        if (arg[0] == '\0')
        {
                send_to_char("[ Keyword  ] Option\n\r", ch);

                send_to_char(IsSet(ch->act, PlrFlee)
                             ?
                             "[+FLEE     ] You flee if you get attacked.\n\r"
                             :
                             "[-flee     ] You fight back if you get attacked.\n\r",
                             ch);

                send_to_char(IsSet(ch->PCData->flags, PcflagNorecall)
                             ?
                             "[+NORECALL ] You fight to the death, link-dead or not.\n\r"
                             :
                             "[-norecall ] You try to recall if fighting link-dead.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrAutoexit)
                             ? "[+AUTOEXIT ] You automatically see exits.\n\r"
                             :
                             "[-autoexit ] You don't automatically see exits.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrAutoloot)
                             ?
                             "[+AUTOLOOT ] You automatically loot corpses.\n\r"
                             :
                             "[-autoloot ] You don't automatically loot corpses.\n\r",
                             ch);


                send_to_char(IsSet(ch->act, PlrAutogold)
                             ?
                             "[+AUTOCRED ] You automatically split credits from kills in groups.\n\r"
                             :
                             "[-autocred ] You don't automatically split credits from kills in groups.\n\r",
                             ch);

                send_to_char(IsSet(ch->PCData->flags, PcflagGag)
                             ?
                             "[+GAG      ] You see only necessary battle text.\n\r"
                             : "[-gag      ] You see full battle text.\n\r",
                             ch);

                send_to_char(IsSet(ch->PCData->flags, PcflagPageron)
                             ? "[+PAGER    ] Long output is page-paused.\n\r"
                             :
                             "[-pager    ] Long output scrolls to the end.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrBlank)
                             ?
                             "[+BLANK    ] You have a blank line before your prompt.\n\r"
                             :
                             "[-blank    ] You have no blank line before your prompt.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrBrief)
                             ? "[+BRIEF    ] You see brief descriptions.\n\r"
                             : "[-brief    ] You see long descriptions.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrCombine)
                             ?
                             "[+COMBINE  ] You see object lists in combined format.\n\r"
                             :
                             "[-combine  ] You see object lists in single format.\n\r",
                             ch);

                send_to_char(IsSet(ch->PCData->flags, PcflagNointro)
                             ?
                             "[+NOINTRO  ] You don't see the ascii intro screen on login.\n\r"
                             :
                             "[-nointro  ] You see the ascii intro screen on login.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrPrompt)
                             ? "[+PROMPT   ] You have a prompt.\n\r"
                             : "[-prompt   ] You don't have a prompt.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrTelnetGa)
                             ?
                             "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
                             :
                             "[-telnetga ] You don't receive a telnet GA sequence.\n\r",
                             ch);

                send_to_char(IsSet(ch->act, PlrAnsi)
                             ?
                             "[+ANSI     ] You receive ANSI color sequences.\n\r"
                             :
                             "[-ansi     ] You don't receive receive ANSI colors.\n\r",
                             ch);
#ifdef ACCOUNT
                if (ch->PCData->Account)
                        send_to_char(IsSet(ch->PCData->flags, AccountSound)
                                     ?
                                     "[+SOUND    ] You have MSP support.\n\r"
                                     :
                                     "[-sound    ] You don't have MSP support.\n\r",
                                     ch);
                else
#endif
                        send_to_char(IsSet(ch->act, PlrSound)
                                     ?
                                     "[+SOUND    ] You have MSP support.\n\r"
                                     :
                                     "[-sound    ] You don't have MSP support.\n\r",
                                     ch);
                send_to_char(IsSet(ch->act, PlrShovedrag) ?
                             "[+SHOVEDRAG] You allow yourself to be shoved and dragged around.\n\r"
                             :
                             "[-shovedrag] You'd rather not be shoved or dragged around.\n\r",
                             ch);

                send_to_char(IsSet(ch->PCData->flags, PcflagNosummon)
                             ?
                             "[+NOSUMMON ] You do not allow other players to summon you.\n\r"
                             :
                             "[-nosummon ] You allow other players to summon you.\n\r",
                             ch);
                
				send_to_char(IsSet(ch->PCData->flags, PcflagAutodraw)
                             ?
                             "[+AUTODRAW ] You autodraw your holsters and put them back away after a fight.\n\r"
                             :
                             "[-autodraw ] No autodrawing, buisness as usual.\n\r",
                             ch);

                if (IsImmortal(ch)) {
                        send_to_char(IsSet(ch->act, PlrRoomvnum)
                                     ?
                                     "[+VNUM     ] You can see the VNUM of a room.\n\r"
                                     :
                                     "[-vnum     ] You do not see the VNUM of a room.\n\r",
                                     ch);

                        send_to_char(IsSet(ch->PCData->flags, PcflagRoom)
                                     ?
                                     "[+ROOMFLAGS] You will see room flags.\n\r"
                                     :
                                     "[-roomflags] You will not see room flags.\n\r",
                                     ch);
				}

                /*
                 * Added 12/16/2003 by Gavin of DW 
                 */
                send_to_char(IsSet(ch->act, PlrMxp)
                             ? "[+MXP      ] You have MXP support.\n\r"
                             : "[-mxp      ] You don't have MXP support.\n\r",
                             ch);
                send_to_char(IsSet(ch->act, PlrSilence) ?
                             "[+SILENCE  ] You are silenced.\n\r" : "", ch);

                send_to_char(!IsSet(ch->act, PlrNoEmote)
                             ? "" : "[-emote    ] You can't emote.\n\r", ch);

                send_to_char(!IsSet(ch->act, PlrNoTell)
                             ? ""
                             : "[-tell     ] You can't use 'tell'.\n\r", ch);

                send_to_char(!IsSet(ch->act, PlrLitterbug)
                             ? ""
                             :
                             "[-litter  ] A convicted litterbug. You cannot drop anything.\n\r",
                             ch);
        }
        else
        {
                bool      fSet;
                int       bit = 0;

                if (arg[0] == '+')
                        fSet = TRUE;
                else if (arg[0] == '-')
                        fSet = FALSE;
                else
                {
                        send_to_char("Config -option or +option?\n\r", ch);
                        return;
                }

                if (!str_prefix(arg + 1, "autoexit"))
                        bit = PlrAutoexit;
                else if (!str_prefix(arg + 1, "autoloot"))
                        bit = PlrAutoloot;
                else if (!str_prefix(arg + 1, "autocred"))
                        bit = PlrAutogold;
                else if (!str_prefix(arg + 1, "blank"))
                        bit = PlrBlank;
                else if (!str_prefix(arg + 1, "brief"))
                        bit = PlrBrief;
                else if (!str_prefix(arg + 1, "combine"))
                        bit = PlrCombine;
                else if (!str_prefix(arg + 1, "prompt"))
                        bit = PlrPrompt;
                else if (!str_prefix(arg + 1, "telnetga"))
                        bit = PlrTelnetGa;
                else if (!str_prefix(arg + 1, "ansi"))
                        bit = PlrAnsi;
                else if (!str_prefix(arg + 1, "sound"))
                {
                        send_to_char("Please use sound command instead.", ch);
                        /*
                         * bit = PlrSound; 
                         */
                        return;
                }
                else if (!str_prefix(arg + 1, "flee"))
                        bit = PlrFlee;
                else if (!str_prefix(arg + 1, "nice"))
                        bit = PlrNice;
                else if (!str_prefix(arg + 1, "shovedrag"))
                        bit = PlrShovedrag;
                else if (IsImmortal(ch) && !str_prefix(arg + 1, "vnum"))
                        bit = PlrRoomvnum;
                else if (!str_prefix(arg + 1, "mxp"))
                {
                        bit = PlrMxp;  /* mxp */
                        if (fSet)
                                do_mxp(ch, "on");
                        else
                                do_mxp(ch, "off");
                }

                if (bit)
                {

                        if (fSet)
                                SetBit(ch->act, bit);
                        else
                                RemoveBit(ch->act, bit);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                else
                {
                        if (!str_prefix(arg + 1, "norecall"))
                                bit = PcflagNorecall;
                        else if (!str_prefix(arg + 1, "nointro"))
                                bit = PcflagNointro;
                        else if (!str_prefix(arg + 1, "nosummon"))
                                bit = PcflagNosummon;
                        else if (!str_prefix(arg + 1, "autodraw"))
                                bit = PcflagAutodraw;
                        else if (!str_prefix(arg + 1, "gag"))
                                bit = PcflagGag;
                        else if (!str_prefix(arg + 1, "pager"))
                                bit = PcflagPageron;
                        else if (!str_prefix(arg + 1, "roomflags")
                                 && (IsImmortal(ch)))
                                bit = PcflagRoom;
                        else
                        {
                                send_to_char("Config which option?\n\r", ch);
                                return;
                        }

                        if (fSet)
                                SetBit(ch->PCData->flags, bit);
                        else
                                RemoveBit(ch->PCData->flags, bit);

                        send_to_char("Ok.\n\r", ch);
                        return;
                }
        }

        return;
}


CMDF do_credits(CharData * ch, char *argument)
{
        (void)argument;
        do_help(ch, "credits");
}

// =============================================================================
// WORLD INFORMATION COMMANDS
// =============================================================================

/*
 * Areas command - list game areas
 */
CMDF do_areas(CharData* ch, char* argument)
{
        AreaData *pArea;

        (void)argument;

        set_pager_color(AtPlain, ch);
        send_to_pager
                ("\n\r   Author    |             Area                     | Recommended |  Enforced\n\r",
                 ch);
        send_to_pager
                ("-------------+--------------------------------------+-------------+-----------\n\r",
                 ch);

        for (pArea = first_area; pArea; pArea = pArea->next)
                pager_printf(ch,
                             "%-12s | %-36s | %4d - %-4d | %3d - %-3d \n\r",
                             pArea->author, pArea->name,
                             pArea->low_soft_range, pArea->hi_soft_range,
                             pArea->low_hard_range, pArea->hi_hard_range);
        return;
}

CMDF do_afk(CharData * ch, char *argument)
{
        (void)argument;
        if (IsNpc(ch))
                return;

        if IsSet
                (ch->act, PlrAfk)
        {
                RemoveBit(ch->act, PlrAfk);
                send_to_char("You are no longer afk.\n\r", ch);
                act(AtGrey, "$n is no longer afk.", ch, NULL, NULL, ToRoom);
        }
        else
        {
                SetBit(ch->act, PlrAfk);
                send_to_char("You are now afk.\n\r", ch);
                act(AtGrey, "$n is now afk.", ch, NULL, NULL, ToRoom);
                return;
        }

}

CMDF do_slist(CharData * ch, char *argument)
{
        int       sn, i;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
        int lFound = 0;
#pragma GCC diagnostic pop
        char      skn[MaxInputLength];
        int       col = 0;

/* C++ conversion - Greven 31/12/03 
   int ability, class, iClass;*/
        int       ability, classtype, iClass;

        if (IsNpc(ch))
                return;


        classtype = -1;
        for (iClass = 0; iClass < MaxAbility; iClass++)
        {
                if (!str_prefix(argument, ability_name[iClass]))
                        classtype = iClass;
        }
        set_pager_color(AtMagic, ch);
        send_to_pager("&BS&zPELL &w& &BS&zKILL &BL&zIST\n\r", ch);
        send_to_pager("------------------\n\r", ch);

        for (ability = -1; ability < MaxAbility; ability++)
        {

                if (argument[0] && (ability != classtype))
                        continue;

                if (ch->perm_frc < 1 && ability == 7
                    && ch->skill_level[7] < 500)
                        ability++;

                if (ability >= 0)
                        snprintf(skn, MSL, "\n\r&B[&z%s&B]\n\r",
                                 capitalize(ability_name[ability]));
                else
                        snprintf(skn, MSL, "%s",
                                 "\n\r&B[&zGeneral Skills&B]\n\r");

                send_to_pager(skn, ch);
                for (i = 0; i <= 150; i++)
                {
                        lFound = 0;
                        for (sn = 0; sn < top_sn; sn++)
                        {
                                if (!skill_table[sn]->name)
                                        break;

                                if (skill_table[sn]->guild != ability)
                                        continue;

                                if (ch->PCData->learned[sn] == 0
                                    && SpellFlag(skill_table[sn],
                                                  SfSecretskill))
                                        continue;

                                if (i == skill_table[sn]->min_level)
                                {
                                        if (skill_table[sn]->races
                                            && skill_table[sn]->races[0] !=
                                            '\0')
                                        {
                                                pager_printf(ch,
                                                             "&B(&w%3d&B)[&zR&B]&B[&z%s&B]&w%-18.18s ",
                                                             i, get_help(ch,
                                                                         skill_table
                                                                         [sn]->
                                                                         name)
                                                             ? "H" : " ",
                                                             skill_table[sn]->
                                                             name);
                                        }
                                        else
                                        {
                                                pager_printf(ch,
                                                             "&B(&w%3d&B)[&z &B]&B[&z%s&B]&w%-18.18s ",
                                                             i, get_help(ch,
                                                                         skill_table
                                                                         [sn]->
                                                                         name)
                                                             ? "H" : " ",
                                                             skill_table[sn]->
                                                             name);
                                        }
                                        if (++col == 3)
                                        {
                                                pager_printf(ch, "\n\r");
                                                col = 0;
                                        }
                                }
                        }
                }
                if (col != 0)
                {
                        pager_printf(ch, "\n\r");
                        col = 0;
                }
        }
        return;
}

CMDF do_whois(CharData * ch, char *argument)
{
        CharData *victim;
        char      buf[MaxStringLength];

        buf[0] = '\0';

        if (IsNpc(ch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char
                        ("You must input the name of a player online.\n\r",
                         ch);
                return;
        }

        mudstrlcat(buf, "0.", MSL);
        mudstrlcat(buf, argument, MSL);
        if (((victim = get_char_world(ch, buf)) == NULL))
        {
                send_to_char("No such player online.\n\r", ch);
                return;
        }

        if (IsNpc(victim))
        {
                send_to_char("That's not a player!\n\r", ch);
                return;
        }

/*        if (!victim->desc)
        {
                send_to_char("That person does not have a descriptor!\n\r",
                             ch);
                return;
        } - Why is this here? */

        send_to_char
                ("&B---------------------------------------------------------&B\n\r",
                 ch);
        ch_printf(ch, "&B| &BN&zame:&w %-20s &B| &BL&zevel: &w%-17d &B|\n\r",
                  victim->name, victim->top_level);
        ch_printf(ch, "&B| &BA&zge:&w %-21d &B| %s &w%-16s &B|\n\r",
                  get_age(victim),
                  victim->PCData->spouse[0] !=
                  '\0' ? (IsSet(victim->PCData->flags, PcflagMarried) ?
                          "&BS&zpouse:" : "&BF&ziance:") : "       ",
                  victim->PCData->spouse[0] !=
                  '\0' ? victim->PCData->spouse : "");
        ch_printf(ch, "&B| &BS&zex:&w %-21s &B| &BR&zace: &w%-18s &B|\n\r",
                  victim->sex == SexMale ? "male" : victim->sex ==
                  SexFemale ? "female" : "neutral", victim->race->name());
        if (IsImmortal(ch))
                ch_printf(ch,
                          "&B| &BI&zn Room:&w %-17d &B|                          &B|\n\r",
                          victim->in_room->vnum);
        if (victim->PCData->clan)
        {
                ch_printf(ch, "&B| &BC&zlan:&w %-47s &B|\n\r",
                          victim->PCData->clan->name);
        }
        send_to_char
                ("&B---------------------------------------------------------&B\n\r",
                 ch);

        if (victim->PCData->bio && victim->PCData->bio[0] != '\0')
        {
                ch_printf(ch, "&z%s's personal bio:\n\r", victim->name);
                send_to_char
                        ("&B---------------------------------------------------------&B\n\r",
                         ch);
                ch_printf(ch, "&w%s\n\r", victim->PCData->bio);
                send_to_char
                        ("&B---------------------------------------------------------&B\n\r",
                         ch);
        }
        if (IsImmortal(ch))
        {
                send_to_char("&zInfo for immortals:\n\r", ch);
                send_to_char
                        ("&B---------------------------------------------------------&B\n\r",
                         ch);

                if (victim->PCData->authed_by
                    && victim->PCData->authed_by[0] != '\0')
                        ch_printf(ch, "&z%s was authorized by %s.\n\r",
                                  victim->name, victim->PCData->authed_by);

                ch_printf(ch,
                          "&z%s has killed &w%d &zmobiles, and been killed by a mobile &w%d &ztimes.\n\r",
                          victim->name, victim->PCData->mkills,
                          victim->PCData->mdeaths);
                if (victim->PCData->pkills || victim->PCData->pdeaths)
                        ch_printf(ch,
                                  "&z%s has killed &w%d &zplayers, and been killed by a player &w%d &ztimes.\n\r",
                                  victim->name, victim->PCData->pkills,
                                  victim->PCData->pdeaths);
                if (victim->PCData->illegal_pk)
                        ch_printf(ch,
                                  "&z%s has committed &w%d &zillegal player kills.\n\r",
                                  victim->name, victim->PCData->illegal_pk);

                ch_printf(ch, "&z%s is &w%s&zhelled at the moment.\n\r",
                          victim->name,
                          (victim->PCData->release_date == 0) ? "not " : "");

                if (victim->PCData->release_date != 0)
                        ch_printf(ch,
                                  "&z%s was helled by &w%s&z, and will be released on &w%24.24s.\n\r",
                                  victim->sex ==
                                  SexMale ? "He" : victim->sex ==
                                  SexFemale ? "She" : "It",
                                  victim->PCData->helled_by,
                                  ctime(&victim->PCData->release_date));

                if (get_trust(victim) < get_trust(ch))
                {
                        snprintf(buf, MSL, "list %s", victim->name);
                        do_comment(ch, buf);
                }

                if (IsSet(victim->act, PlrSilence)
                    || IsSet(victim->act, PlrNoEmote)
                    || IsSet(victim->act, PlrNoTell))
                {
                        snprintf(buf, MSL, "%s",
                                 "&zThis player has the following flags set:");
                        if (IsSet(victim->act, PlrSilence))
                                mudstrlcat(buf, "&w silence", MSL);
                        if (IsSet(victim->act, PlrNoEmote))
                                mudstrlcat(buf, "&w noemote", MSL);
                        if (IsSet(victim->act, PlrNoTell))
                                mudstrlcat(buf, "&w notell", MSL);
                        mudstrlcat(buf, ".\n\r", MSL);
                        send_to_char(buf, ch);
                }
                if (victim->desc && victim->desc->host[0] != '\0')  /* added by Gorog */
                {
                        snprintf(buf, MSL, "&z%s's IP info:&w %s ",
                                 victim->name, victim->desc->host);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        if (victim->desc && victim->desc->Client[0] != '\0')
                        {
                                ch_printf(ch, "&z%s's Client is &w%s\n\r",
                                          victim->name, victim->desc->Client);
                        }
                }
                if (get_trust(ch) >= LevelGod
                    && get_trust(ch) >= get_trust(victim) && victim->PCData)
                {
                        ch_printf(ch, "&zEmail: &w%s\n\r",
                                  victim->PCData->email ? victim->PCData->email : "None");
                }
                if (victim->desc)
                {
                        ch_printf(ch, "&zMSP: &w%s  &zMXP:  &w%s"
#ifdef MCCP
                                  "  &zMCCP:   &w%s"
#endif
                                  "\n\r",
                                  victim->desc->MspDetected ? "On " : "Off",
                                  victim->desc->MxpDetected ? "On " : "Off"
#ifdef MCCP
                                  , victim->desc->Compressing ? "On " : "Off"
#endif
                                );
                }
        }
}


CMDF do_pager(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];

        if (IsNpc(ch))
                return;
        argument = one_argument(argument, arg);
        if (!*arg)
        {
                if (IsSet(ch->PCData->flags, PcflagPageron))
                        do_config(ch, const_cast<char*>("-pager"));
                else
                        do_config(ch, const_cast<char*>("+pager"));
                return;
        }
        if (!is_number(arg))
        {
                send_to_char("Set page pausing to how many lines?\n\r", ch);
                return;
        }
        ch->PCData->pagerlen = static_cast<sh_int>(atoi(arg));
        if (ch->PCData->pagerlen < PagerMinLines)
                ch->PCData->pagerlen = 5;
        ch_printf(ch, "Page pausing set to %d lines.\n\r",
                  ch->PCData->pagerlen);
        return;
}

CMDF do_steacher(CharData * ch, char *argument)
{

        CharData *victim;
        char      buf[MaxStringLength];
        char     *buf1;
        char      arg[MaxStringLength];
        int       sn, vnum;
        bool      fMob = FALSE, fSet = FALSE;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
        bool fMI = FALSE;
#pragma GCC diagnostic pop
        SkillType *skill = NULL;


        if (IsNpc(ch))
                return;

        set_pager_color(AtMagic, ch);
        send_to_pager("\n\r------------[ Missing Teachers ]-------------\n\r",
                      ch);
        if (str_cmp(argument, "existant"))
        {
                for (sn = 0;
                     sn < top_sn && skill_table[sn] && skill_table[sn]->name;
                     sn++)
                {
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                                continue;

                        fMob = FALSE;
                        fSet = TRUE;
                        skill = skill_table[sn];
                        if (skill->teachers && skill->teachers[0] != '\0')
                        {
                                buf1 = skill->teachers;
                                for (;;)
                                {
                                        buf1 = one_argument(buf1, arg);
                                        if (arg[0] != '\0')
                                        {
                                                vnum = atoi(arg);
                                                if (vnum > 0)
                                                {
                                                        if ((victim =
                                                             get_char_world
                                                             (ch,
                                                              arg)) != NULL)
                                                                fMob = TRUE;
                                                }
                                        }
                                        else
                                                break;
                                }
                        }
                        else if (!skill->teachers
                                 || skill->teachers[0] == '\0')
                                fSet = FALSE;
                        else
                                continue;

                        if (fMob)
                                continue;
                        if (fSet && fMob)
                                continue;

                        if (!fMob)
                                snprintf(buf, MSL,
                                         "Skill: %-25s    [No Invoked Teachers]\n\r",
                                         skill_table[sn]->name);
                        if (!fSet)
                                snprintf(buf, MSL,
                                         "Skill: %-25s    [No Designated Teachers]\n\r",
                                         skill_table[sn]->name);
                        send_to_char(buf, ch);
                        continue;
                }
        }
        else
        {
                for (sn = 0;
                     sn < top_sn && skill_table[sn] && skill_table[sn]->name;
                     sn++)
                {
                        fMI = FALSE;
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                                continue;

                        skill = skill_table[sn];
                        if (skill->teachers && skill->teachers[0] != '\0')
                        {
                                buf1 = skill->teachers;
                                for (;;)
                                {
                                        buf1 = one_argument(buf1, arg);
                                        if (arg[0] != '\0')
                                        {
                                                vnum = atoi(arg);
                                                if (vnum > 0)
                                                {
                                                        if (!get_mob_index
                                                            (vnum))
                                                                ch_printf(ch,
                                                                          "&BSkill: %-25s    [Invalid teacher: %7d]\n\r",
                                                                          skill_table
                                                                          [sn]->
                                                                          name,
                                                                          vnum);
                                                }
                                        }
                                        else
                                                break;
                                }
                        }
                }
        }

        return;
}

char     *revision(void)
{
        static char buf[MIL];
        char      buf1[MIL];
        int       i, j;

        mudstrlcpy(buf1, REVISION, MIL);
        for (i = 11, j = 0; i < static_cast<int>(strlen(buf1)) - 2; i++, j++)
                buf[j] = buf1[i];
        buf[i++] = '\0';


        return buf;

}

CMDF do_mudinfo(CharData * ch, char *argument)
{
        (void)argument;
        ch_printf(ch, "&BM&zud name:             &w%-20s&D\n\r",
                  sysdata.mud_name);
        ch_printf(ch, "&BM&zud email:            &w%-20s&D\n\r",
                  sysdata.mud_email);
        ch_printf(ch, "&BM&zud url:              &w%-20s&D\n\r",
                  sysdata.mud_url);
        ch_printf(ch, "&BM&zud name:             &w%-20s&D\n\r",
                  sysdata.mud_name);
        ch_printf(ch, "&BW&zeb server enabled:   &w%-20s&D\n\r",
                  sysdata.web ? "Enabled" : "Disabled");
        ch_printf(ch, "&BA&zmount of help files: &w%-5d&D\n\r", top_help);
        ch_printf(ch, "&BM&zax players ever:     &w%-5d&D\n\r",
                  sysdata.alltimemax);
        ch_printf(ch, "&BL&zast compile time:    &w%-20s&D\n\r", __TIME__);
        ch_printf(ch, "&BL&zast compile date:    &w%-20s&D\n\r", __DATE__);
#if defined(__cplusplus)
        ch_printf(ch, "&BC&zompile type:         &w%-20s&D\n\r", "C++");
#else
        ch_printf(ch, "&BC&zompile type:         &w%-20s&D\n\r", "C");
#endif

#if defined(MCCP)
        ch_printf(ch, "&BM&zCCP:                 &w%-20s&D\n\r", "Enabled");
#else
        ch_printf(ch, "&BM&zCCP:                 &w%-20s&D\n\r", "Disabled");
#endif

#if defined(IMC)
        ch_printf(ch, "&BI&zMC2:                 &w%-20s&D\n\r", "Enabled");
#else
        ch_printf(ch, "&BI&zMC2:                 &w%-20s&D\n\r", "Disabled");;
#endif

#if defined(WIN)
        ch_printf(ch, "&BW&zindows compile:      &w%-20s&D\n\r", "Yes");
#else
        ch_printf(ch, "&BW&zindows compile:      &w%-20s&D\n\r", "No");
#endif
        ch_printf(ch, "&BL&zast version:         &w%-20s&D\n\r", revision());
}
