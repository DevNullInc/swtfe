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
 * Action and communication commands for player interactions and social gameplay. *
 ****************************************************************************************/

// =============================================================================
// SYSTEM INCLUDES
// =============================================================================
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// =============================================================================
// LOCAL INCLUDES
// =============================================================================
#include "mud.hpp"
#include "mxp.hpp"
#include "msp.hpp"
#include "Account.hpp"
#include "races.hpp"

// =============================================================================
// CONSTANTS
// =============================================================================
namespace {
    constexpr int    DefaultComchan = 0;
    constexpr int    NpcComfreq = -1;
    constexpr size_t LogBufferSize = MaxStringLength;
    constexpr size_t SocialNameSize = 256;
    constexpr size_t SocialDescSize = 512;
    constexpr size_t CommunicationBuffer = MaxInputLength;
    constexpr int    MinSplitMembers = 2;
    constexpr int    MaxFollowDistance = 3;
    constexpr int    MinLanguageSkill = 60;
    constexpr int    MaxLanguageSkill = 99;
    constexpr int    ArousalBase = 70;
    constexpr double ArousalMultiplier = 0.5;
    constexpr double RandomPercent = 0.1;
}

// =============================================================================
// EXTERNAL FUNCTION DECLARATIONS
// =============================================================================
void send_obj_page_to_char(CharData* ch, ObjIndexData* idx, char page);
void send_room_page_to_char(CharData* ch, RoomIndexData* idx, char page);
void send_page_to_char(CharData* ch, MobIndexData* idx, char page);
void send_control_page_to_char(CharData* ch, char page);
extern bool is_ignoring(CharData* ch, CharData* victim);

// =============================================================================
// LOCAL FUNCTION PROTOTYPES
// =============================================================================
void talk_channel(CharData* ch, char* argument, int channel, const char* verb);
char* scramble(const char* argument, LanguageData* language);
char* drunk_speech(const char* argument, CharData* ch);

// =============================================================================
// COMMUNICATION HELPER FUNCTIONS
// =============================================================================

/*
 * Check if character has a communication device
 */
bool has_comlink(CharData* ch)
{
    ObjData* obj;

    if (IsImmortal(ch) || IsNpc(ch))
        return TRUE;

#ifdef CYBER
    if (ch->pcdata->cyber & CyberComm)
        return TRUE;
#endif

    if (ch->race && !str_cmp(ch->race->name(), "droid"))
        return TRUE;

    for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
        if (obj->pIndexData->item_type == ItemComlink)
            return TRUE;
    }
    return FALSE;
}

/*
 * Get the communication frequency for a player
 */
int get_comfreq(CharData* ch)
{
    if (IsNpc(ch))
        return NpcComfreq;

    if (!has_comlink(ch))
        return DefaultComchan;
    
    return ch->pcdata->comchan ? ch->pcdata->comchan : DefaultComchan;
}

/*
 * Get the comlink object from character's inventory
 */
ObjData* get_comlink(CharData* ch)
{
    for (ObjData* obj = ch->last_carrying; obj; obj = obj->prev_content) {
        if (obj->item_type == ItemComlink)
            return obj;
    }
    return nullptr;
}

// =============================================================================
// SOCIAL SYSTEM FUNCTIONS
// =============================================================================

/*
 * Extended social command added by Atrox
 */
CMDF do_xsocial(CharData* ch, char* argument)
{
        char      arg[MaxInputLength], buf[MaxInputLength];
        CharData *victim;
        SocialType *social;
        char      command[MaxInputLength];
        int       iHash;
        int       col = 0;


        if (IsNpc(ch))
        {
                return;
        }

        if (ch->pcdata->realage != 1)
        {
                send_to_char
                        ("You must set your legal status before you can use this command",
                         ch);
                send_to_char("see: setself legal", ch);
                return;
        }

        argument = one_argument(argument, command);

        if (command[0] == '\0')
        {
                set_pager_color(AtPlain, ch);
                send_to_pager
                        ("&w<>&B----------------------------------------------------------------------&w<>\n\r",
                         ch);
                send_to_pager
                        ("&B||----------------------------------------------------------------------||\n\r&B|",
                         ch);

                for (iHash = 0; iHash < 27; iHash++)
                        for (social = social_index[iHash]; social;
                             social = social->next)
                        {
                                if (social->minarousal != 0
                                    || social->arousal != 0)
                                {
                                        pager_printf(ch, "&B[&w%-22s&B]",
                                                     center_str(social->name,
                                                                22));
                                        if (++col % 3 == 0)
                                                send_to_pager("&B|\n\r&B|",
                                                              ch);
                                }
                        }
                if (col % 3 != 0)
                        pager_printf(ch, "%s&B]|\n\r",
                                     center_str(" ", (24 * (3 - (col % 3)))));
                send_to_pager
                        ("&B||----------------------------------------------------------------------||\n\r",
                         ch);
                send_to_pager
                        ("&w<>&B----------------------------------------------------------------------&w<>\n\r",
                         ch);
                return;
        }

        xREMOVE_BIT(ch->deaf, ChannelTells);
        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("&BY&zou can't do that here.\n\r", ch);
                return;
        }

        if (!IsNpc(ch) && (IsSet(ch->act, PlrSilence)))
        {
                send_to_char("&BY&zou can't do that.\n\r", ch);
                return;
        }

        if ((social = find_xsocial(command)) == NULL)
        {
                send_to_char("&BP&zick a correct xsocial to use1.\n\r", ch);
                return;
        }
        if (social->arousal == 0)
        {
                send_to_char("&BP&zick a correct xsocial to use2.\n\r", ch);
                return;
        }

        switch (ch->position)
        {
        case PosDead:
                send_to_char("&BL&zie still; you are DEAD.\n\r", ch);
                return;
        case PosIncap:
        case PosMortal:
                send_to_char("&BY&zou are hurt far too bad for that.\n\r",
                             ch);
                return;
        case PosStunned:
                send_to_char("&BY&zou are too stunned to do that.\n\r", ch);
                return;
        case PosSleeping:
                break;
        default:
                {
                }
        }

        one_argument(argument, arg);
        victim = NULL;

        if (arg[0] == '\0')
        {
                snprintf(buf, MaxInputLength, "%s", social->others_no_arg);
                act(AtSocial, buf, ch, NULL, victim, ToRoom);
                snprintf(buf, MaxInputLength, "%s", social->char_no_arg);
                act(AtSocial, buf, ch, NULL, victim, ToChar);
                ch->pcdata->arousal += static_cast<sh_int>(social->arousal * .25);
                return;
        }
        victim = get_char_room(ch, arg);

        if (victim == NULL || (!can_see(ch, victim)))
        {
                send_to_char("&BT&zhat person isn't here on!\n\r", ch);
                return;
        }

        if (IsNpc(victim))
        {
                send_to_char("&BT&zhat person isn't here on!\n\r", ch);
                return;
        }

        if (IsSet(victim->act, PlrAfk) || victim->pcdata->realage != 1)
        {
                send_to_char("&BY&zou cannot do that to that person!", ch);
                return;
        }

        if (social->minarousal > victim->pcdata->arousal)
        {
                send_to_char("&BT&zhat person is not aroused enough!", ch);
                return;
        }

        if (victim->sex != social->sex && social->sex != 3)
        {
                send_to_char("&BT&zhey are not the right sex for that!", ch);
                return;
        }

        if (victim == ch && social->arousal > 4)
        {
                send_to_char("&BY&zou can't do that to yourself!", ch);
                return;
        }

        if (victim == ch)
        {
                snprintf(buf, MaxInputLength, "%s", social->others_auto);
                act(AtSocial, buf, ch, NULL, victim, ToRoom);
                snprintf(buf, MaxInputLength, "%s", social->char_auto);
                act(AtSocial, buf, ch, NULL, victim, ToChar);
                ch->pcdata->arousal += static_cast<sh_int>(social->arousal * ArousalMultiplier);
                /*
                 * FIXME - 
                 * * Gavin - Uh... all 3 of these victim->sex lines had ; at the end.
                 * * ie, if (victim->sex == 0);
                 * * which if thats the case, could be alot easier to write another way.
                 */
                if (victim->sex == SexNeutral) {
                        if (victim->pcdata->arousal >=
                            (ArousalBase +
                             (2 * (get_curr_con(victim)) +
                              (RandomPercent * number_percent())))) {
                        }
                }
                if (victim->sex == SexMale) {
                        if (victim->pcdata->arousal >=
                            (ArousalBase +
                             (3 * (get_curr_con(victim)) +
                              (RandomPercent * number_percent())))) {
                        }
                }
                if (victim->sex == SexFemale) {
                        if (victim->pcdata->arousal >=
                            (ArousalBase +
                             (1 * (get_curr_con(victim)) +
                              (RandomPercent * number_percent())))) {
                        }
                }
                return;
        }
        else
        {
                snprintf(buf, MaxInputLength, "%s", social->others_found);
                act(AtSocial, buf, ch, NULL, victim, ToNotvict);
                snprintf(buf, MaxInputLength, "%s", social->char_found);
                act(AtSocial, buf, ch, NULL, victim, ToChar);
                snprintf(buf, MaxInputLength, "%s", social->vict_found);
                act(AtSocial, buf, ch, NULL, victim, ToVict);
                victim->pcdata->arousal += social->arousal;
                ch->pcdata->arousal += static_cast<sh_int>(social->arousal * .25);
                if (victim->sex == SexNeutral)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (2 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                if (victim->sex == SexMale)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (3 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                if (victim->sex == SexFemale)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (1 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                return;
        }
}

// =============================================================================
// COMMUNICATION COMMANDS
// =============================================================================

/*
 * Beep command - send a communication signal to another player
 */
CMDF do_beep(CharData* ch, char* argument)
{
        CharData *victim;
        char      arg[MaxStringLength];

        argument = one_argument(argument, arg);

        xREMOVE_BIT(ch->deaf, ChannelTells);
        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IsNpc(ch) &&
            (IsSet(ch->act, PlrSilence) || IsSet(ch->act, PlrNoTell)))
        {
                send_to_char("&BY&zou can't do that.\n\r", ch);
                return;
        }

        if (arg[0] == '\0')
        {
                send_to_char("&BB&zeep who?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL
            || (IsNpc(victim) && victim->in_room != ch->in_room)
            || (!NotAuthed(ch) && NotAuthed(victim) && !IsImmortal(ch)))
        {
                send_to_char("&BT&zhey aren't here.\n\r", ch);
                return;
        }

        if (NotAuthed(ch) && !NotAuthed(victim) && !IsImmortal(victim))
        {
                send_to_char
                        ("&BT&zhey can't hear you because you are not authorized.\n\r",
                         ch);
                return;
        }

        if (!IsNpc(victim) && (victim->switched)
            && (get_trust(ch) > LevelAvatar))
        {
                send_to_char("&BT&zhat player is switched.\n\r", ch);
                return;
        }

        else if (!IsNpc(victim) && (!victim->desc))
        {
                send_to_char("&BT&zhat player is link-dead.\n\r", ch);
                return;
        }

        if (xIS_SET(victim->deaf, ChannelTells)
            && (!IsImmortal(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AtPlain, "$E has $S tells turned off.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if (IsSet(victim->pcdata->flags, PcflagWorking) && !IsImmortal(ch))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }

        if (!IsNpc(victim) && (IsSet(victim->act, PlrSilence)))
        {
                send_to_char
                        ("That player is silenced.  They will receive your message but can not respond.\n\r",
                         ch);
        }

        if ((!IsImmortal(ch) && !IsAwake(victim))
            || (!IsNpc(victim)
                && xIS_SET(victim->in_room->RoomFlags, RoomSilence)))
        {
                act(AtPlain, "$E can't hear you.", ch, 0, victim, ToChar);
                return;
        }

        if (victim->desc    /* make sure desc exists first  -Thoric */
            && victim->desc->connected == ConEditing
            && get_trust(ch) < LevelGod)
        {
                act(AtPlain,
                    "$E is currently in a writing buffer.  Please try again in a few minutes.",
                    ch, 0, victim, ToChar);
                return;
        }

        if (IsMxp(ch))
                act(AtWhite,
                    "\aYou beep " MXPTAG("player $G") "$N" MXPTAG("/player")
                    ": '$t'", ch, argument, victim, ToCharOoc);
        else
                act(AtWhite, "\aYou beep $N: '$t'", ch, argument, victim,
                    ToCharOoc);
        if (IsMxp(victim))
                act(AtWhite,
                    "\a" MXPTAG("player $g") "$n" MXPTAG("/player")
                    " beeps: '$t'", ch, argument, victim, ToVictOoc);
        else
                act(AtWhite, "\a$n beeps: '$t'", ch, argument, victim,
                    ToVictOoc);
}

/* Text scrambler -- Altrag */
char     *scramble(const char *argument, [[maybe_unused]] LanguageData * language)
{
        static char arg[MaxInputLength];
        sh_int    position, modifier;
        sh_int    conversion = 0;

        language = NULL;

        modifier = static_cast<sh_int>(number_range(80, 300));   /* Bitvectors get way too large #s */
        for (position = 0; position < MaxInputLength; position++)
        {
                if (argument[position] == '\0')
                {
                        arg[position] = '\0';
                        return arg;
                }
                else if (argument[position] >= 'A'
                         && argument[position] <= 'Z')
                {
                        conversion = static_cast<sh_int>(
                                -conversion + position - modifier +
                                argument[position] - 'A');
                        conversion = static_cast<sh_int>(
                                number_range(conversion - 5, conversion + 5));
                        while (conversion > 25)
                                conversion -= 26;
                        while (conversion < 0)
                                conversion += 26;
                        arg[position] = static_cast<char>(conversion + 'A');
                }
                else if (argument[position] >= 'a'
                         && argument[position] <= 'z')
                {
                        conversion = static_cast<sh_int>(
                                -conversion + position - modifier +
                                argument[position] - 'a');
                        conversion = static_cast<sh_int>(
                                number_range(conversion - 5, conversion + 5));
                        while (conversion > 25)
                                conversion -= 26;
                        while (conversion < 0)
                                conversion += 26;
                        arg[position] = static_cast<char>(conversion + 'a');
                }
                else if (argument[position] >= '0'
                         && argument[position] <= '9')
                {
                        conversion = static_cast<sh_int>(
                                -conversion + position - modifier +
                                argument[position] - '0');
                        conversion = static_cast<sh_int>(
                                number_range(conversion - 2, conversion + 2));
                        while (conversion > 9)
                                conversion -= 10;
                        while (conversion < 0)
                                conversion += 10;
                        arg[position] = static_cast<char>(conversion + '0');
                }
                else
                        arg[position] = argument[position];
        }
        arg[position] = '\0';
        return arg;
}

/* I'll rewrite this later if its still needed.. -- Altrag */
char     *translate([[maybe_unused]] CharData * ch, [[maybe_unused]] CharData * victim, [[maybe_unused]] const char *argument)
{
        static char empty_string[] = "";
        return empty_string;
}

char     *drunk_speech(const char *argument, CharData * ch)
{
        const char *arg = argument;
        static char buf[MaxInputLength * 2];
        char      buf1[MaxInputLength * 2];
        sh_int    drunk;
        char     *txt;
        char     *txt1;

        if (IsNpc(ch) || !ch->pcdata)
        {
                mudstrlcpy(buf, argument, MIL * 2);
                return buf;
        }

        drunk = ch->pcdata->condition[CondDrunk];
        /*
         * Speak funny if you have a broken jaw - Gavin 
         */
        if (IsSet(ch->bodyparts, BodyJaw))
                drunk += 15;

        if (drunk <= 0)
        {
                mudstrlcpy(buf, argument, MIL * 2);
                return buf;
        }

        buf[0] = '\0';
        buf1[0] = '\0';

        if (!argument)
        {
                bug("Drunk_speech: NULL argument", 0);
                static char empty_string[] = "";
                return empty_string;
        }

        /*
         * if ( *arg == '\0' )
         * return (char *) argument;
         */

        txt = buf;
        txt1 = buf1;

        while (*arg != '\0')
        {
                if (toupper(*arg) == 'S')
                {
                        if (number_percent() < (drunk * 2)) /* add 'h' after an 's' */
                        {
                                *txt++ = *arg;
                                *txt++ = 'h';
                        }
                        else
                                *txt++ = *arg;
                }
                else if (toupper(*arg) == 'X')
                {
                        if (number_percent() < (drunk * 2 / 2))
                        {
                                *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
                        }
                        else
                                *txt++ = *arg;
                }
                else if (number_percent() < (drunk * 2 / 5))    /* slurred letters */
                {
                        sh_int    slurn = static_cast<sh_int>(number_range(1, 2));
                        sh_int    currslur = 0;

                        while (currslur < slurn)
                                *txt++ = *arg, currslur++;
                }
                else
                        *txt++ = *arg;

                arg++;
        };

        *txt = '\0';

        txt = buf;

        while (*txt != '\0')    /* Let's mess with the string's caps */
        {
                if (number_percent() < (2 * drunk / 2.5))
                {
                        if (isupper(*txt))
                                *txt1 = static_cast<char>(tolower(*txt));
                        else if (islower(*txt))
                                *txt1 = static_cast<char>(toupper(*txt));
                        else
                                *txt1 = *txt;
                }
                else
                        *txt1 = *txt;

                txt1++, txt++;
        };

        *txt1 = '\0';
        txt1 = buf1;
        txt = buf;

        while (*txt1 != '\0')   /* Let's make them stutter */
        {
                if (*txt1 == ' ')   /* If there's a space, then there's gotta be a */
                {   /* along there somewhere soon */

                        while (*txt1 == ' ')    /* Don't stutter on spaces */
                                *txt++ = *txt1++;

                        if ((number_percent() < (2 * drunk / 4))
                            && *txt1 != '\0')
                        {
                                sh_int    offset = static_cast<sh_int>(number_range(0, 2));
                                sh_int    pos = 0;

                                while (*txt1 != '\0' && pos < offset)
                                        *txt++ = *txt1++, pos++;

                                if (*txt1 == ' ')   /* Make sure not to stutter a space after */
                                {   /* the initial offset into the word */
                                        *txt++ = *txt1++;
                                        continue;
                                }

                                pos = 0;
                                offset = static_cast<sh_int>(number_range(2, 4));
                                while (*txt1 != '\0' && pos < offset)
                                {
                                        *txt++ = *txt1;
                                        pos++;
                                        if (*txt1 == ' ' || pos == offset)  /* Make sure we don't stick */
                                        {   /* A hyphen right before a space */
                                                txt1--;
                                                break;
                                        }
                                        *txt++ = '-';
                                }
                                if (*txt1 != '\0')
                                        txt1++;
                        }
                }
                else
                        *txt++ = *txt1++;
        }

        *txt = '\0';

        return buf;
}

void info_chan(char *argument)
{
        char      buf[MaxStringLength];

        snprintf(buf, MSL, "&B[&zINFO&B] &w%s&R&W", argument);
        echo_to_all(AtGossip, buf, EchoTarAll);
}

void to_channel(const char *argument, int channel, const char *verb,
                sh_int level)
{
        char      buf[MaxStringLength];
        DescriptorData *d;

        if (!first_descriptor || argument[0] == '\0')
                return;

        snprintf(buf, MSL, "%s: %s\r\n", verb, argument);

        for (d = first_descriptor; d; d = d->next)
        {
                CharData *och;
                CharData *vch;

                och = d->original ? d->original : d->character;
                vch = d->character;

                if (!och || !vch)
                        continue;
                if (!IsImmortal(vch)
                    || (get_trust(vch) < sysdata.build_level
                        && channel == ChannelBuild)
                    || (get_trust(vch) < sysdata.log_level
                        && (channel == ChannelLog
                            || channel == ChannelComm)))
                        continue;

                if (IsPlaying(d)
                    && !xIS_SET(och->deaf, channel)
                    && get_trust(vch) >= level)
                {
                        set_char_color(AtLog, vch);
                        send_to_char(buf, vch);
                }
        }

        return;
}

CMDF do_shout(CharData * ch, const char *argument)
{
        RoomIndexData *room;
        ExitData *pexit = NULL;

        if (!ch->in_room)
        {
                bug("%s is not in a room", ch->name);
                send_to_char("Huh?\n\r", ch);
                return;
        }
        room = ch->in_room;

        /*
         * if (NotAuthed(ch))
         * {
         * send_to_char("Huh?\n\r", ch);
         * return;
         * } - I don't think we need this - Gavin 
         */

        /*
         * Need to scrable 
         */
        act(AtShout, "You shout '$t&c'", ch, drunk_speech(argument, ch), ch,
            ToChar);
        act(AtShout, "$n shouts '$t&c'", ch, drunk_speech(argument, ch), ch,
            ToRoom);
        char_from_room(ch);
        for (pexit = room->first_exit; pexit; pexit = pexit->next)
        {
                if (pexit->to_room && room != pexit->to_room
                    && !IsSet(pexit->exit_info, ExClosed))
                {
                        char_to_room(ch, pexit->to_room);
                        act(AtShout, "$n shouts '$t&c'", ch,
                            drunk_speech(argument, ch), ch, ToRoom);
                        char_from_room(ch);
                }
        }
        char_to_room(ch, room);
/*	talk_channel( ch, drunk_speech( argument, ch ), ChannelShout, "shout" );*/
        WaitState(ch, 12);
        return;
}


CMDF do_osay(CharData * ch, char *argument)
{
        if (argument[0] == '\0')
        {
                send_to_char("OSay what?\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        act(AtFlee, "$n osays &R[&Y$t&R]", ch, argument, ch, ToRoomOoc);
        act(AtSay, "You osay &R[&Y$t&R]", ch, argument, ch, ToCharOoc);
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                char      buf[MaxStringLength];

                snprintf(buf, MSL, "%s: %s",
                         IsNpc(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LogFile, buf);
        }
}



CMDF do_whisper(CharData * ch, char *argument)
{
        char      arg[MaxInputLength], _last_char;
        CharData *victim;
        int       arglen;
        int       actflags;
        char     *sbuf = NULL;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Say what to whom?\n\r", ch);
                return;
        }
        if ((victim = get_char_room(ch, arg)) == NULL
            || IsNpc(victim)
            || (!NotAuthed(ch) && NotAuthed(victim) && !IsImmortal(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        arglen = static_cast<int>(strlen(argument)) - 1;
        /*
         * Remove whitespace and tabs. 
         */
        while (argument[arglen] == ' ' || argument[arglen] == '\t')
                --arglen;
        _last_char = argument[arglen];

        actflags = ch->act;
        MOBtrigger = FALSE;

        sbuf = argument;

        if (IsNpc(ch))
                RemoveBit(ch->act, ActSecretive);

        if (!knows_language(victim, ch->speaking, ch)
            && (!IsNpc(ch) || ch->speaking != 0))
                sbuf = scramble(argument, ch->speaking);

        switch (_last_char)
        {
        case '?':
                act(AtWhite, "You whisper to $N, '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, ToChar);
                act(AtWhite, "$n quietly asks you '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, ToVict);
                break;

        default:
                act(AtWhite, "You whisper to $N '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, ToChar);
                act(AtWhite, "$n whispers to you '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, ToVict);
                break;
        }
        act(AtWhite, "$n whispers something to $N", ch, NULL, victim,
            ToNotvict);
        ch->act = actflags;
        MOBtrigger = TRUE;

        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                char      buf[MaxStringLength];

                snprintf(buf, MSL, "%s: %s",
                         IsNpc(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LogFile, buf);
        }
        mprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        oprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        rprog_speech_trigger(argument, ch);
        return;
}

// =============================================================================
// TELL SYSTEM COMMANDS
// =============================================================================

/*
 * Tell command - send a private message to another player
 */
CMDF do_tell(CharData* ch, char* argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        int       position;
        char     *sbuf = argument;

        if (xIS_SET(ch->deaf, ChannelTells) && !IsImmortal(ch))
        {
                act(AtPlain,
                    "You have tells turned off... try chan +tells first", ch,
                    NULL, NULL, ToChar);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IsNpc(ch) &&
            (IsSet(ch->act, PlrSilence) || IsSet(ch->act, PlrNoTell)))
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Tell whom what?\n\r", ch);
                return;
        }
        sbuf = argument;

        if ((victim = get_char_world(ch, arg)) == NULL
            || IsNpc(victim)
            || (!NotAuthed(ch) && NotAuthed(victim) && !IsImmortal(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }


        if (ch == victim)
        {
                send_to_char("You have a nice little chat with yourself.\n\r",
                             ch);
                return;
        }

        if (NotAuthed(ch) && !NotAuthed(victim) && !IsImmortal(victim))
        {
                send_to_char
                        ("They can't hear you because you are not authorized.\n\r",
                         ch);
                return;
        }

        if (!IsNpc(victim) && (victim->switched) &&
            (get_trust(ch) > LevelAvatar) &&
            !IsSet(victim->switched->act, ActPolymorphed) &&
            !IsAffected(victim->switched, AffPossess))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }
        else if (!IsNpc(victim) && (victim->switched) &&
                 (IsSet(victim->switched->act, ActPolymorphed) ||
                  IsAffected(victim->switched, AffPossess)))
                victim = victim->switched;
        else if (!IsNpc(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (!has_comlink(ch) && !IsImmortal(ch) && !IsImmortal(victim))
        {
                send_to_char("You do not have a comlink.", ch);
                return;
        }

        if (!has_comlink(victim) && !IsImmortal(victim) && !IsImmortal(ch))
        {
                send_to_char("They don't have a comlink.", ch);
                return;
        }


        if (IsSet(victim->pcdata->flags, PcflagWorking)
            && get_trust(ch) <= get_trust(victim))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }


        if (!IsNpc(victim) && (IsSet(victim->act, PlrAfk)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }

        if (xIS_SET(victim->deaf, ChannelTells)
            && (!IsImmortal(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AtPlain, "$E has $S tells turned off.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if (!IsNpc(victim) && (IsSet(victim->act, PlrSilence)))
        {
                send_to_char
                        ("That player is silenced.  They will receive your message but can not respond.\n\r",
                         ch);
        }

        if ((!IsImmortal(ch) && !IsAwake(victim))
            || (!IsNpc(victim)
                && xIS_SET(victim->in_room->RoomFlags, RoomSilence)))
        {
                act(AtPlain, "$E can't hear you.", ch, 0, victim, ToChar);
                return;
        }

        if (victim->desc    /* make sure desc exists first  -Thoric */
            && victim->desc->connected == ConEditing
            && get_trust(ch) < LevelGod)
        {
                act(AtPlain,
                    "$E is currently in a writing buffer.  Please try again in a few minutes.",
                    ch, 0, victim, ToChar);
                return;
        }


        if (IsMxp(ch))
                act(AtTell,
                    "You tell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " '$t'", ch, argument, victim, ToChar);
        else
                act(AtTell, "You tell $N '$t'", ch, argument, victim,
                    ToChar);

        position = victim->position;
        victim->position = PosStanding;
        if (!IsImmortal(victim) && !knows_language(victim, ch->speaking, ch)
            && (!IsNpc(ch) || ch->speaking != 0))
                sbuf = scramble(argument, ch->speaking);

        if (IsMxp(victim))
                act(AtTell,
                    MXPTAG("player $g") "$n" MXPTAG("/player")
                    " tells you '$t'", ch, drunk_speech(sbuf, ch), victim,
                    ToVict);
        else
                act(AtTell, "$n tells you '$t'", ch, drunk_speech(sbuf, ch),
                    victim, ToVict);

        victim->position = static_cast<sh_int>(position);
        victim->reply = ch;
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                char      buf[MaxInputLength];

                snprintf(buf, MaxInputLength, "%s: %s (tell to) %s.",
                         IsNpc(ch) ? ch->short_descr : ch->name,
                         argument,
                         IsNpc(victim) ? victim->short_descr : victim->name);
                append_to_file(LogFile, buf);
        }
        mprog_speech_trigger(argument, ch);
        return;
}



CMDF do_reply(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        CharData *victim;
        int       position;
        char     *sbuf = argument;


        xREMOVE_BIT(ch->deaf, ChannelTells);
        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IsNpc(ch) && IsSet(ch->act, PlrSilence))
        {
                send_to_char("Your message didn't get through.\n\r", ch);
                return;
        }


        if (((victim = ch->reply) == NULL) || IsNpc(victim))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!IsNpc(victim) && (victim->switched)
            && can_see(ch, victim) && (get_trust(ch) > LevelAvatar))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }
        else if (!IsNpc(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (!IsNpc(victim) && (IsSet(victim->act, PlrAfk)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }

        if (IsSet(victim->pcdata->flags, PcflagWorking))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }


        if (xIS_SET(victim->deaf, ChannelTells)
            && (!IsImmortal(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AtPlain, "$E has $S tells turned off.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if ((!IsImmortal(ch) && !IsAwake(victim))
            || (!IsNpc(victim)
                && xIS_SET(victim->in_room->RoomFlags, RoomSilence)))
        {
                act(AtPlain, "$E can't hear you.", ch, 0, victim, ToChar);
                return;
        }

        if (!IsImmortal(victim) && !knows_language(victim, ch->speaking, ch)
            && (!IsNpc(ch) || ch->speaking != 0))
                sbuf = scramble(argument, ch->speaking);

        if (IsMxp(ch))
                act(AtTell,
                    "You tell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " '$t'", ch, argument, victim, ToChar);
        else
                act(AtTell, "You tell $N '$t'", ch, argument, victim,
                    ToChar);
        position = victim->position;
        victim->position = PosStanding;
        if (IsMxp(victim))
                act(AtTell,
                    MXPTAG("player $g") "$n" MXPTAG("/player")
                    " tells you '$t'", ch, drunk_speech(sbuf, ch), victim,
                    ToVict);
        else
                act(AtTell, "$n tells you '$t'", ch,
                    drunk_speech(sbuf, ch), victim, ToVict);
        victim->position = static_cast<sh_int>(position);
        victim->reply = ch;
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                snprintf(buf, MSL, "%s: %s (reply to) %s.",
                         IsNpc(ch) ? ch->short_descr : ch->name,
                         argument,
                         IsNpc(victim) ? victim->short_descr : victim->name);
                append_to_file(LogFile, buf);
        }

        return;
}

CMDF do_otell(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      buf[MaxInputLength];
        CharData *victim;
        int       position;
        CharData *switched_victim;

        switched_victim = NULL;

        if (xIS_SET(ch->deaf, ChannelTells) && !IsImmortal(ch))
        {
                send_to_char
                        ("You have tells turned off... try chan +tells first",
                         ch);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IsNpc(ch) &&
            (IsSet(ch->act, PlrSilence) || IsSet(ch->act, PlrNoTell)))
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("OTell whom what?\n\r", ch);
                return;
        }
        if ((victim = get_char_world(ch, arg)) == NULL
            || IsNpc(victim)
            || (!NotAuthed(ch) && NotAuthed(victim) && !IsImmortal(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (ch == victim)
        {
                send_to_char
                        ("You have a nice little out of character chat with yourself.\n\r",
                         ch);
                return;
        }

        if (NotAuthed(ch) && !NotAuthed(victim) && !IsImmortal(victim))
        {
                send_to_char
                        ("They can't hear you because you are not authorized.\n\r",
                         ch);
                return;
        }

        if (!IsNpc(victim) && (victim->switched)
            && (get_trust(ch) > LevelAvatar)
            && !IsSet(victim->switched->act, ActPolymorphed)
            && !IsAffected(victim->switched, AffPossess))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }

        else if (!IsNpc(victim) && (victim->switched)
                 && (IsSet(victim->switched->act, ActPolymorphed)
                     || IsAffected(victim->switched, AffPossess)))
                switched_victim = victim->switched;

        else if (!IsNpc(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (!IsNpc(victim) && (IsSet(victim->act, PlrAfk)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }
        if (IsSet(victim->pcdata->flags, PcflagWorking))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }

        if (xIS_SET(victim->deaf, ChannelTells)
            && (!IsImmortal(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AtPlain, "$E has $S tells turned off.", ch, NULL, victim,
                    ToCharOoc);
                return;
        }

        if (!IsNpc(victim) && (IsSet(victim->act, PlrSilence)))
        {
                send_to_char
                        ("That player is silenced.  They will receive your message but can not respond.\n\r",
                         ch);
        }

        if ((!IsImmortal(ch) && !IsAwake(victim))
            || (!IsNpc(victim)
                && xIS_SET(victim->in_room->RoomFlags, RoomSilence)))
        {
                act(AtPlain, "$E can't hear you.", ch, 0, victim,
                    ToCharOoc);
                return;
        }

        if (victim->desc    /* make sure desc exists first  -Thoric */
            && victim->desc->connected == ConEditing
            && get_trust(ch) < LevelGod)
        {
                act(AtPlain,
                    "$E is currently in a writing buffer.  Please try again in a few minutes.",
                    ch, 0, victim, ToCharOoc);
                return;
        }

        if (is_ignoring(victim, ch))
        {
                snprintf(buf, MaxInputLength, "%s is ignoring you.\n\r", victim->name);
                send_to_char(buf, ch);
                return;
        }

        if (switched_victim)
                victim = switched_victim;

        position = victim->position;
        victim->position = PosStanding;
        if (IsMxp(ch))
                act(AtTell,
                    "&zYou Otell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " &B'&w$t&B'", ch, argument, victim, ToCharOoc);
        else
                act(AtTell, "&zYou Otell $N &B'&w$t&B'", ch, argument,
                    victim, ToCharOoc);
        if (IsMxp(victim))
                act(AtTell,
                    "&B" MXPTAG("player $g") "$n" MXPTAG("/player")
                    " &zOtells you &B'&w$t&B'", ch, argument, victim,
                    ToVictOoc);
        else
                act(AtTell, "&B$n &zOtells you &B'&w$t&B'", ch, argument,
                    victim, ToVictOoc);
        victim->position = static_cast<sh_int>(position);
        victim->oreply = ch;
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                snprintf(buf, MaxInputLength, "%s: %s (otell to) %s.",
                         IsNpc(ch) ? ch->short_descr : ch->name,
                         argument,
                         IsNpc(victim) ? victim->short_descr : victim->name);
                append_to_file(LogFile, buf);
        }
        return;
}

CMDF do_oreply(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        CharData *victim;
        int       position;

        xREMOVE_BIT(ch->deaf, ChannelTells);
        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IsNpc(ch) && IsSet(ch->act, PlrSilence))
        {
                send_to_char("Your message didn't get through.\n\r", ch);
                return;
        }

        if (((victim = ch->oreply) == NULL) || IsNpc(victim))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!IsNpc(victim) && (victim->switched)
            && can_see(ch, victim) && (get_trust(ch) > LevelAvatar))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }
        else if (!IsNpc(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (IsSet(victim->pcdata->flags, PcflagWorking))
        {
                send_to_char
                        ("That person is working right now. Don't bother them!\n\r",
                         ch);
                return;
        }

        if (!IsNpc(victim) && (IsSet(victim->act, PlrAfk)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }

        if (xIS_SET(victim->deaf, ChannelTells)
            && (!IsImmortal(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AtPlain, "$E has $S tells turned off.", ch, NULL, victim,
                    ToCharOoc);
                return;
        }

        if ((!IsImmortal(ch) && !IsAwake(victim))
            || (!IsNpc(victim)
                && xIS_SET(victim->in_room->RoomFlags, RoomSilence)))
        {
                act(AtPlain, "$E can't hear you.", ch, 0, victim,
                    ToCharOoc);
                return;
        }

        if (is_ignoring(victim, ch))
        {
                snprintf(buf, MSL, "%s is ignoring you.\n\r", victim->name);
                send_to_char(buf, ch);
                return;
        }
        position = victim->position;
        victim->position = PosStanding;
        if (IsMxp(ch))
                act(AtTell,
                    "&zYou Otell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " &B'&w$t&B'", ch, argument, victim, ToCharOoc);
        else
                act(AtTell, "&zYou Otell $N &B'&w$t&B'", ch, argument,
                    victim, ToCharOoc);
        if (IsMxp(victim))
                act(AtTell,
                    "&B" MXPTAG("player $g") "$n" MXPTAG("/player")
                    " &zOtells you &B'&w$t&B'", ch, argument, victim,
                    ToVictOoc);
        else
                act(AtTell, "&B$n &zOtells you &B'&w$t&B'", ch, argument,
                    victim, ToVictOoc);
        victim->position = static_cast<sh_int>(position);
        victim->oreply = ch;
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                snprintf(buf, MSL, "%s: %s (oreply to) %s.",
                         IsNpc(ch) ? ch->short_descr : ch->name,
                         argument,
                         IsNpc(victim) ? victim->short_descr : victim->name);
                append_to_file(LogFile, buf);
        }

        return;
}

// =============================================================================
// EMOTE AND ROLEPLAY COMMANDS
// =============================================================================

/*
 * Emote command - express character actions and emotions
 */
CMDF do_emote(CharData* ch, const char* argument)
{
        char      buf[MaxStringLength];
        int       actflags;
        char     *plast;

        if (!IsNpc(ch) && IsSet(ch->act, PlrNoEmote))
        {
                send_to_char("You can't show your emotions.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Emote what?\n\r", ch);
                return;
        }

        actflags = ch->act;
        if (IsNpc(ch))
                RemoveBit(ch->act, ActSecretive);
        for (plast = const_cast<char*>(argument); *plast != '\0'; plast++);

        mudstrlcpy(buf, argument, MSL);
        if (isalpha(plast[-1]))
                mudstrlcat(buf, ".", MSL);

        MOBtrigger = FALSE;
        if (buf[0] == 's' && buf[1] == ' ') 
        {
                act(AtSocial, "$n'$T", ch, NULL, buf, ToRoom);
                act(AtSocial, "$n'$T", ch, NULL, buf, ToChar);
        }
        else {
                act(AtSocial, "$n $T", ch, NULL, buf, ToRoom);
                act(AtSocial, "$n $T", ch, NULL, buf, ToChar);
        }
        ch->act = actflags;
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                snprintf(buf, MSL, "%s %s (emote)",
                         IsNpc(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LogFile, buf);
        }
        return;
}


// =============================================================================
// REPORTING COMMANDS (BUG/IDEA/TYPO)
// =============================================================================

/*
 * Bug report command
 */
CMDF do_bug(CharData* ch, char* argument)
{
        set_char_color(AtPlain, ch);
        if (argument[0] == '\0')
        {
                send_to_char("\n\rUsage:  'bug <message>'\n\r", ch);
                if (get_trust(ch) >= LevelAscendant)
                        send_to_char
                                ("Usage:  'bug list' or 'bug clear now'\n\r",
                                 ch);
                return;
        }
        if (!str_cmp(argument, "clear now")
            && get_trust(ch) >= LevelAscendant)
        {
                FILE     *fp = fopen(BugsFile, "w");

                if (fp)
                {
                        FCLOSE(fp);
                }
                send_to_char("Bug file cleared.\n\r", ch);
                return;
        }
        if (!str_cmp(argument, "list"))
        {
                if (get_trust(ch) >= LevelAscendant)
                {
                        send_to_char("\n\r VNUM \n\r.......\n\r", ch);
                        show_file(ch, BugsFile);
                }
                else
                {
                        send_to_char("Huh?\n\r", ch);
                        return;
                }
        }
        else
        {
                append_file(ch, BugsFile, smash_color((argument)));
                send_to_char("Ok, thanks..\n\r", ch);
        }
        return;
}

CMDF do_idea(CharData * ch, char *argument)
{
        set_char_color(AtPlain, ch);
        if (argument[0] == '\0')
        {
                send_to_char("\n\rUsage:  'idea <message>'\n\r", ch);
                if (get_trust(ch) >= LevelAscendant)
                        send_to_char
                                ("Usage:  'idea list' or 'idea clear now'\n\r",
                                 ch);
                return;
        }
        if (!str_cmp(argument, "clear now")
            && get_trust(ch) >= LevelAscendant)
        {
                FILE     *fp = fopen(IdeaFile, "w");

                if (fp)
                {
                        FCLOSE(fp);
                }
                send_to_char("Idea file cleared.\n\r", ch);
                return;
        }
        if (!str_cmp(argument, "list"))
        {
                if (get_trust(ch) >= LevelAscendant)
                {
                        send_to_char("\n\r VNUM \n\r.......\n\r", ch);
                        show_file(ch, IdeaFile);
                }
                else
                {
                        send_to_char("Huh?\n\r", ch);
                        return;
                }
        }
        else
        {
                append_file(ch, IdeaFile, smash_color((argument)));
                send_to_char("Ok, thanks..\n\r", ch);
        }
        return;
}




CMDF do_typo(CharData * ch, char *argument)
{
        set_char_color(AtPlain, ch);
        if (argument[0] == '\0')
        {
                send_to_char
                        ("\n\rUsage:  'typo <message>'  (your location is automatically recorded)\n\r",
                         ch);
                if (get_trust(ch) >= LevelAscendant)
                        send_to_char
                                ("Usage:  'typo list' or 'typo clear now'\n\r",
                                 ch);
                return;
        }
        if (!str_cmp(argument, "clear now") && get_trust(ch) >= LevelSupreme)
        {
                FILE     *fp = fopen(TypoFile, "w");

                if (fp)
                {
                        FCLOSE(fp);
                }
                send_to_char("Typo file cleared.\n\r", ch);
                return;
        }
        if (!str_cmp(argument, "list") && get_trust(ch) >= LevelAscendant)
        {
                send_to_char("VNUM \n\r.......\n\r", ch);
                show_file(ch, TypoFile);
        }
        else
        {
                append_file(ch, TypoFile, smash_color((argument)));
                send_to_char("Ok, thanks.\n\r", ch);
        }
        return;
}

CMDF do_qui(CharData * ch, [[maybe_unused]] char *argument)
{
        argument = NULL;
        set_char_color(AtRed, ch);
        send_to_char("If you want to QUIT, you have to spell it out.\n\r",
                     ch);
        return;
}

// =============================================================================
// SESSION COMMANDS (QUIT/SAVE/ANSI)
// =============================================================================

/*
 * Quit command - leave the game
 */
CMDF do_quit(CharData* ch, [[maybe_unused]] const char* argument)
{

        char      buf[MaxInputLength];

/*   int x, y; */
        int       level;

        argument = NULL;

        if (IsNpc(ch) && IsSet(ch->act, ActPolymorphed))
        {
                send_to_char("You can't quit while polymorphed.\n\r", ch);
                return;
        }

        if (IsNpc(ch))
                return;

        if (ch->position == PosFighting)
        {
                set_char_color(AtRed, ch);
                send_to_char("No way! You are fighting.\n\r", ch);
                return;
        }

        if (ch->position < PosStunned)
        {
                set_char_color(AtBlood, ch);
                send_to_char("You're not DEAD yet.\n\r", ch);
                return;
        }

        if (auction->item != NULL
            && ((ch == auction->buyer) || (ch == auction->seller)))
        {
                send_to_char
                        ("Wait until you have bought/sold the item on auction.\n\r",
                         ch);
                return;
        }

        if (!IsImmortal(ch) && ch->in_room
            && !xIS_SET(ch->in_room->RoomFlags, RoomHotel)
            && !NotAuthed(ch))
        {
                send_to_char("You may not quit here.\n\r", ch);
                send_to_char
                        ("You will have to find a safer resting place such as a hotel...\n\r",
                         ch);
                send_to_char("Maybe you could HAIL a speeder.\n\r", ch);
                return;
        }

        set_char_color(AtWhite, ch);
        send_to_char
                ("You close your eyes, and you slowly get tired. Lights change and swirl\n\ras your consiousness floats away.\n\r\n\r",
                 ch);
        act(AtSay,
            "You feel someone or something calling to you, '$n... come back.\n\rYou're our only hope'",
            ch, NULL, NULL, ToChar);
        act(AtBye, "$n has left the game.", ch, NULL, NULL, ToRoom);
        set_char_color(AtGrey, ch);
        if (!IsSet(ch->act, PlrWizinvis) && ch->desc)
        {
                snprintf(buf, MaxInputLength, "%s has left %s", ch->name,
                         sysdata.mud_name);
                info_chan(buf);
        }
        snprintf(log_buf, MSL, "%s has quit.", ch->name);
        quitting_char = ch;
        save_char_obj(ch);
        save_home(ch);
        saving_char = NULL;

        level = get_trust(ch);
        /*
         * After extract_char the ch is no longer Valid!
         */
#ifdef ACCOUNT
        if (!IsNpc(ch) && ch->pcdata && ch->pcdata->Account)
        {
                free_account(ch->pcdata->Account);
                ch->pcdata->Account = NULL;
                if (ch->desc && ch->desc->Account)
                        ch->desc->Account = NULL;
        }
#endif
        extract_char(ch, TRUE);
/*    for ( x = 0; x < MaxWear; x++ )
	for ( y = 0; y < MaxLayers; y++ )
	    save_equipment[x][y] = NULL; - WTF */

        /*
         * don't show who's logging off to leaving player 
         */
        log_string_plus(log_buf, LogComm, static_cast<sh_int>(level));
        return;
}

CMDF do_ansi(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("ANSI ON or OFF?\n\r", ch);
                return;
        }
        if ((strcmp(arg, "on") == 0) || (strcmp(arg, "ON") == 0))
        {
                SetBit(ch->act, PlrAnsi);
                set_char_color(AtWhite + AtBlink, ch);
                send_to_char("ANSI ON!!!\n\r", ch);
                return;
        }

        if ((strcmp(arg, "off") == 0) || (strcmp(arg, "OFF") == 0))
        {
                RemoveBit(ch->act, PlrAnsi);
                send_to_char("Okay... ANSI support is now off\n\r", ch);
                return;
        }
}

CMDF do_save(CharData * ch, const char *argument)
{
        bool      silent = FALSE;

        if (!str_cmp(argument, "-silentsave"))
                silent = TRUE;
        argument = NULL;
        if (IsNpc(ch) && IsSet(ch->act, ActPolymorphed))
        {
                send_to_char("You can't save while polymorphed.\n\r", ch);
                return;
        }

        if (IsNpc(ch))
                return;

        if (!IsSet(ch->affected_by, ch->race->affected()))
                SetBit(ch->affected_by, ch->race->affected());

        if (NotAuthed(ch))
        {
                send_to_char
                        ("You can't save until after you've graduated from the acadamey.\n\r",
                         ch);
                return;
        }
        if (IsImmortal(ch) && ch->pcdata && ch->pcdata->area
            && IsSet(ch->pcdata->area->status, AreaLoaded))
                fold_area(ch->pcdata->area, ch->pcdata->area->filename, FALSE,
                          FALSE);
        save_char_obj(ch);
        save_home(ch);
#ifdef ACCOUNT
        if (ch->pcdata && ch->pcdata->Account)
                save_account(ch->pcdata->Account);
#endif
        saving_char = NULL;
        save_finger(ch);
        if (silent == FALSE)
                send_to_char("Ok.\n\r", ch);

        return;
}

void auto_save(CharData * ch)
{
        if (IsNpc(ch) && IsSet(ch->act, ActPolymorphed))
        {
                return;
        }

        if (IsNpc(ch))
                return;

        if (!IsSet(ch->affected_by, ch->race->affected()))
                SetBit(ch->affected_by, ch->race->affected());

        if (NotAuthed(ch))
        {
                return;
        }
        save_char_obj(ch);
#ifdef ACCOUNT
        if (ch->pcdata && ch->pcdata->Account)
                save_account(ch->pcdata->Account);
#endif
        save_home(ch);
        saving_char = NULL;
        save_finger(ch);
        return;
}



/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow(CharData * ch, CharData * victim)
{
        CharData *tmp;

        for (tmp = victim; tmp; tmp = tmp->master)
                if (tmp == ch)
                        return TRUE;
        return FALSE;
}


// =============================================================================
// GROUP AND PARTY COMMANDS
// =============================================================================

/*
 * Follow command - follow another character
 */
CMDF do_follow(CharData* ch, char* argument)
{
        char      arg[MaxInputLength];
        CharData *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Follow whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IsAffected(ch, AffCharm) && ch->master)
        {
                act(AtPlain, "But you'd rather follow $N!", ch, NULL,
                    ch->master, ToChar);
                return;
        }

        if (victim == ch)
        {
                if (!ch->master)
                {
                        send_to_char("You already follow yourself.\n\r", ch);
                        return;
                }
                stop_follower(ch);
                return;
        }

        if (circle_follow(ch, victim))
        {
                send_to_char
                        ("Following in loops is not allowed... sorry.\n\r",
                         ch);
                return;
        }

        if (ch->master)
                stop_follower(ch);

        add_follower(ch, victim);
        return;
}



void add_follower(CharData * ch, CharData * master)
{
        if (ch->master)
        {
                bug("Add_follower: non-null master.", 0);
                return;
        }

        ch->master = master;
        ch->leader = NULL;

        if (can_see(master, ch))
                act(AtAction, "$n now follows you.", ch, NULL, master,
                    ToVict);

        act(AtAction, "You now follow $N.", ch, NULL, master, ToChar);

        return;
}



void stop_follower(CharData * ch)
{
        if (!ch->master)
        {
                bug("Stop_follower: null master.", 0);
                return;
        }

        if (IsAffected(ch, AffCharm))
        {
                RemoveBit(ch->affected_by, AffCharm);
                affect_strip(ch, gsn_charm_person);
        }

        if (can_see(ch->master, ch) && !char_died(ch->master))
                act(AtAction, "$n stops following you.", ch, NULL,
                    ch->master, ToVict);
        act(AtAction, "You stop following $N.", ch, NULL, ch->master,
            ToChar);

        ch->master = NULL;
        ch->leader = NULL;
        if (ch->holding)
        {
                ch->holding->heldby = NULL;
                ch->holding->held = FALSE;
        }
        ch->holding = NULL;
        return;
}



void die_follower(CharData * ch)
{
        CharData *fch;

        if (ch->master)
                stop_follower(ch);

        ch->leader = NULL;

        CheckLinks(first_char, last_char, next, prev, CharData);
        for (fch = first_char; fch; fch = fch->next)
        {
                if (fch->master == ch)
                        stop_follower(fch);
                if (fch->leader == ch)
                        fch->leader = fch;
        }
        if (ch->holding)
        {
                ch->holding->heldby = NULL;
                ch->holding->held = FALSE;
        }
        ch->holding = NULL;
        return;
}



CMDF do_order(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      argbuf[MaxInputLength];
        CharData *victim;
        CharData *och;
        CharData *och_next;
        bool      found;
        bool      fAll;

        mudstrlcpy(argbuf, argument, MIL);
        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Order whom to do what?\n\r", ch);
                return;
        }

        if (IsAffected(ch, AffCharm))
        {
                send_to_char("You feel like taking, not giving, orders.\n\r",
                             ch);
                return;
        }

        if (!str_cmp(arg, "all"))
        {
                fAll = TRUE;
                victim = NULL;
        }
        else
        {
                fAll = FALSE;
                if ((victim = get_char_room(ch, arg)) == NULL)
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return;
                }

                if (victim == ch)
                {
                        send_to_char("Aye aye, right away!\n\r", ch);
                        return;
                }

                if (!IsAffected(victim, AffCharm) || victim->master != ch)
                {
                        send_to_char("Do it yourself!\n\r", ch);
                        return;
                }
        }

        found = FALSE;
        for (och = ch->in_room->first_person; och; och = och_next)
        {
                och_next = och->next_in_room;

                if (IsAffected(och, AffCharm)
                    && och->master == ch && (fAll || och == victim))
                {
                        found = TRUE;
                        act(AtAction, "$n orders you to '$t'.", ch, argument,
                            och, ToVict);
                        interpret(och, argument);
                }
        }

        if (found)
        {
                snprintf(log_buf, MSL, "%s: order %s.", ch->name, argbuf);
                log_string_plus(log_buf, LogNormal, ch->top_level);
                send_to_char("Ok.\n\r", ch);
                WaitState(ch, 12);
        }
        else
                send_to_char("You have no followers here.\n\r", ch);
        return;
}

CMDF do_group(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim = NULL;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                CharData *gch;
                CharData *leader;

                leader = ch->leader ? ch->leader : ch;
                set_char_color(AtGreen, ch);
                ch_printf(ch, "%s's group:\n\r", PERS(leader, ch));

/* Changed so that no info revealed on possess */
                for (gch = first_char; gch; gch = gch->next)
                {
                        if (is_same_group(gch, ch))
                        {
                                set_char_color(AtDgreen, ch);
                                if (IsAffected(gch, AffPossess))
                                        ch_printf(ch,
                                                  "[%2d %s] %-16s %4s/%4s hp %4s/%4s mv %5s xp\n\r",
                                                  gch->top_level,
                                                  IsNpc(gch) ? "Mob" : gch->
                                                  race->name(),
                                                  capitalize(PERS(gch, ch)),
                                                  "????", "????", "????",
                                                  "????", "?????");

                                else
                                        ch_printf(ch,
                                                  "[%2d %s] %-16s %4d/%4d hp %4d/%4d mv\n\r",
                                                  gch->top_level,
                                                  IsNpc(gch) ? "Mob" : gch->
                                                  race->name(),
                                                  capitalize(PERS(gch, ch)),
                                                  gch->hit, gch->max_hit,
                                                  gch->endurance,
                                                  gch->max_endurance);
                        }
                }
                return;
        }

        if (!strcmp(arg, "disband"))
        {
                CharData *gch;
                int       count = 0;

                if (ch->leader || ch->master)
                {
                        send_to_char
                                ("You cannot disband a group if you're following someone.\n\r",
                                 ch);
                        return;
                }

                for (gch = first_char; gch; gch = gch->next)
                {
                        if (is_same_group(ch, gch) && (ch != gch))
                        {
                                gch->leader = NULL;
                                gch->master = NULL;
                                count++;
                                send_to_char("Your group is disbanded.\n\r",
                                             gch);
                        }
                }

                if (count == 0)
                        send_to_char
                                ("You have no group members to disband.\n\r",
                                 ch);
                else
                        send_to_char("You disband your group.\n\r", ch);

                return;
        }

        if (!strcmp(arg, "all"))
        {
                CharData *rch;
                int       count = 0;

                for (rch = ch->in_room->first_person; rch;
                     rch = rch->next_in_room)
                {
                        if (ch != rch
                            && !IsNpc(rch)
                            && rch->master == ch
                            && !ch->master
                            && !ch->leader && !is_same_group(rch, ch))
                        {
                                rch->leader = ch;
                                count++;
                        }
                }

                if (count == 0)
                        send_to_char
                                ("You have no eligible group members.\n\r",
                                 ch);
                else
                {
                        /*
                         * FIXME - victim is un-initalized 
                         */
                        /*
                         * act( AtAction, "$n groups $s followers.", ch, NULL, victim, ToRoom ); - Gavin. lets try this 
                         */
                        act(AtAction, "$n groups $s followers.", ch, NULL,
                            NULL, ToRoom);
                        send_to_char("You group your followers.\n\r", ch);
                }
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (ch->master || (ch->leader && ch->leader != ch))
        {
                send_to_char("But you are following someone else!\n\r", ch);
                return;
        }

        if (victim->master != ch && ch != victim)
        {
                act(AtPlain, "$N isn't following you.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if (is_same_group(victim, ch) && ch != victim)
        {
                victim->leader = NULL;
                act(AtAction, "$n removes $N from $s group.", ch, NULL,
                    victim, ToNotvict);
                act(AtAction, "$n removes you from $s group.", ch, NULL,
                    victim, ToVict);
                act(AtAction, "You remove $N from your group.", ch, NULL,
                    victim, ToChar);
                return;
        }

        victim->leader = ch;
        act(AtAction, "$N joins $n's group.", ch, NULL, victim, ToNotvict);
        act(AtAction, "You join $n's group.", ch, NULL, victim, ToVict);
        act(AtAction, "$N joins your group.", ch, NULL, victim, ToChar);
        return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
CMDF do_split(CharData * ch, const char *argument)
{
        char      buf[MaxStringLength];
        char      arg[MaxInputLength];
        CharData *gch;
        int       members;
        int       amount;
        int       share;
        int       extra;

        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
        {
                send_to_char("Split how much?\n\r", ch);
                return;
        }

        amount = atoi(arg);

        if (amount < 0)
        {
                send_to_char("Your group wouldn't like that.\n\r", ch);
                return;
        }

        if (amount == 0)
        {
                send_to_char
                        ("You hand out zero credits, but no one notices.\n\r",
                         ch);
                return;
        }

        if (ch->gold < amount)
        {
                send_to_char("You don't have that many credits.\n\r", ch);
                return;
        }

        members = 0;
        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                if (is_same_group(gch, ch))
                        members++;
        }


        if ((IsSet(ch->act, PlrAutogold)) && (members < 2))
                return;

        if (members < 2)
        {
                send_to_char("Just keep it all.\n\r", ch);
                return;
        }

        share = amount / members;
        extra = amount % members;

        if (share == 0)
        {
                send_to_char("Don't even bother, cheapskate.\n\r", ch);
                return;
        }

        ch->gold -= amount;
        ch->gold += share + extra;

        set_char_color(AtGold, ch);
        ch_printf(ch,
                  "You split %d credits.  Your share is %d credits.\n\r",
                  amount, share + extra);

        snprintf(buf, MSL, "$n splits %d credits.  Your share is %d credits.",
                 amount, share);

        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                if (gch != ch && is_same_group(gch, ch))
                {
                        act(AtGold, buf, ch, NULL, gch, ToVict);
                        gch->gold += share;
                }
        }
        return;
}



/*
 * Group tell command - send message to all group members
 */
CMDF do_gtell(CharData* ch, char* argument)
{
        CharData *gch;

        if (argument[0] == '\0')
        {
                send_to_char("Tell your group what?\n\r", ch);
                return;
        }

        if (IsSet(ch->act, PlrNoTell))
        {
                send_to_char("Your message didn't get through!\n\r", ch);
                return;
        }

        /*
         * Note use of send_to_char, so gtell works on sleepers.
         */

        for (gch = first_char; gch; gch = gch->next)
        {
                if (is_same_group(gch, ch))
                {
                        set_char_color(AtGtell, gch);
                        /*
                         * Groups unscrambled regardless of clan language.  Other languages
                         * still garble though. -- Altrag 
                         */
                        if (knows_language(gch, ch->speaking, gch)
                            || (IsNpc(ch) && !ch->speaking))
                                ch_printf(gch, "%s tells the group '%s'.\n\r",
                                          ch->name, argument);
                        else
                                ch_printf(gch, "%s tells the group '%s'.\n\r",
                                          ch->name, scramble(argument,
                                                             ch->speaking));
                }
        }

        return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group(CharData * ach, CharData * bch)
{
        if (ach->leader)
                ach = ach->leader;
        if (bch->leader)
                bch = bch->leader;
        return ach == bch;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction(char *argument)
{
        DescriptorData *d;
        char      buf[MaxStringLength];
        CharData *original;

        snprintf(buf, MSL, "Auction: %s", argument);    /* last %s to reset color */

        for (d = first_descriptor; d; d = d->next)
        {
                original = d->original ? d->original : d->character;    /* if switched */
                if (IsPlaying(d) && !xIS_SET(original->deaf, ChannelAuction)
                    && !xIS_SET(original->in_room->RoomFlags, RoomSilence)
                    && !NotAuthed(original))
                        act(AtGossip, buf, original, NULL, NULL, ToChar);
        }
}

// =============================================================================
// LANGUAGE SYSTEM FUNCTIONS
// =============================================================================

/*
 * Language support functions. -- Altrag
 * 07/01/96
 */
bool knows_language(CharData* ch, LanguageData* lang, CharData* cch)
{
        sh_int    sn;

        if (!IsNpc(ch) && IsImmortal(ch))
        {
                return TRUE;
        }

        if (IsImmortal(ch) || IsImmortal(cch))
                return TRUE;

/* Hack for OLC Languages, Force mobs to understand everything */
/* Allows certain mobs to speak EVERYTHING, like in the academy for newbies. */
        if (IsNpc(ch) || (IsNpc(cch) && IsSet(cch->act, ActSpeaksall)))
        {
                return TRUE;
        }
        if (!str_cmp(lang->name, "clan"))
        {
                /*
                 * Clan = basic for mobs.. snicker.. -- Altrag 
                 */
                if (IsNpc(ch) || IsNpc(cch))
                        return TRUE;
                if (ch->pcdata->clan == cch->pcdata->clan &&
                    ch->pcdata->clan != NULL)
                        return TRUE;
        }
        if (!IsNpc(ch))
        {
                /*
                 * Racial languages for PCs 
                 */
                /*
                 * Major Overhaul by Greven for OLC languages 
                 */
                if (ch->race->language() == lang)
                        return TRUE;
                if (ch->speaking == lang)
                        return TRUE;

                if ((sn = static_cast<sh_int>(skill_lookup(lang->name))) != -1
                    && ch->pcdata->learned[sn] >= MinLanguageSkill)
                        return TRUE;
        }
        return FALSE;
}

bool can_learn_lang(CharData * ch, LanguageData * lang)
{
        if (!str_cmp(lang->name, "clan"))
                return FALSE;
        if (IsNpc(ch))
                return FALSE;
        {
                int       sn;

                if ((sn = skill_lookup(lang->name)) < 0)
                {
                        bug("Can_learn_lang: Valid language without sn: %s",
                            lang->name);
                }
                if (ch->pcdata->learned[sn] >= MaxLanguageSkill)
                        return FALSE;
        }

        return TRUE;
}

const char* const lang_names[] =
        { "basic", "wookiee", "twilek", "rodian", "hutt",
        "mon calamari", "noghri", "gamorrean",
        "jawa", "adarian", "ewok", "verpine", "defel",
        "trandoshan", "shistavanan", "binary", "duinuogwuin", "csillian",
        "kel dor",
        "bothan", "barabel", "ithorian", "devaronian", "durosian", "gotal",
        "talzzi", "ho'din",
        "falleen", "givin", "clan", ""
};

/*
 * Speak command - change the language being spoken
 */
CMDF do_speak(CharData* ch, char* argument)
{
        LanguageData *language;
        char      arg[MaxInputLength];

        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "all") && IsImmortal(ch))
        {
                set_char_color(AtSay, ch);
                send_to_char("Now speaking all languages.\n\r", ch);
                return;
        }
        if (!str_prefix(arg, "basic")
            && !str_cmp(ch->race->name(), "wookiee"))
        {
                set_char_color(AtSay, ch);
                send_to_char
                        ("Wookiees cannot speak basic even though some can understand it.\n\r",
                         ch);
                return;
        }
        if (!str_prefix(arg, "twilek")
            && str_cmp(ch->race->name(), "twi'lek"))
        {
                set_char_color(AtSay, ch);
                send_to_char
                        ("To speak the Twi'lek language requires body parts that you don't have.\n\r",
                         ch);
                return;
        }
        if (!str_prefix(arg, "binary") && str_cmp(ch->race->name(), "droid"))
        {
                set_char_color(AtSay, ch);
                send_to_char
                        ("To speak binary, your brain would have to be a billion times faster.\n\r",
                         ch);
                return;
        }
        for (language = first_language; language; language = language->next)
        {
                if (!str_prefix(arg, language->name))
                {
                        if (knows_language(ch, language, ch))
                        {
                                if (!str_cmp(language->name, "clan") &&
                                    (IsNpc(ch) || !ch->pcdata->clan))
                                        continue;
                                ch->speaking = language;
                                set_char_color(AtSay, ch);
                                ch_printf(ch, "You now speak %s.\n\r",
                                          language->name);
                                return;
                        }
                }
        }
        set_char_color(AtSay, ch);
        send_to_char("You do not know that language.\n\r", ch);
}

CMDF do_languages(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        LanguageData *language = NULL, *lang2 = NULL;
        int       sn2, sn;
        int       numlang = 0;
        CharData *victim = NULL;

        if (IsNpc(ch))
                return;
        argument = one_argument(argument, arg);
        if (arg[0] != '\0' && !str_prefix(arg, "learn") &&
            !IsImmortal(ch) && !IsNpc(ch))
        {
                CharData *sch;
                char      arg2[MaxInputLength];
                int       prct;

                argument = one_argument(argument, arg2);
                if (arg2[0] == '\0')
                {
                        send_to_char("Learn which language?\n", ch);
                        return;
                }

                if (!str_prefix(arg2, "clan"))
                {
                        send_to_char("Can't learn clan\n", ch);
                        return;
                }

                for (lang2 = first_language; lang2; lang2 = lang2->next)
                {
                        if (!str_cmp(lang2->name, "clan"))
                                continue;

                        if (!str_prefix(arg2, lang2->name))
                                language = lang2;

                        if (str_prefix(arg2, lang2->name) &&
                            (sn2 = skill_lookup(lang2->name)) != -1)
                        {
                                if (ch->pcdata->learned[sn2] >= 1)
                                        numlang++;
                        }
                }

                if (numlang >= max_languages(ch))
                {
                        send_to_char
                                ("You are not smart enough to learn any more languages.\n\r",
                                 ch);
                        return;
                }
                if (language == NULL)
                {
                        send_to_char("That is not a language.\n\r", ch);
                        return;
                }
                if ((sn = skill_lookup(language->name)) < 0)
                {
                        send_to_char("That is not a language.\n\r", ch);
                        return;
                }
                if (ch->race->language() == language ||
                    ch->pcdata->learned[sn] >= MaxLanguageSkill)
                {
                        act(AtPlain, "You are already fluent in $t.", ch,
                            language->name, NULL, ToChar);
                        return;
                }
                for (sch = ch->in_room->first_person; sch;
                     sch = sch->next_in_room)
                        if (IsNpc(sch) && IsSet(sch->act, ActScholar))
                                break;
                if (!sch)
                {
                        send_to_char
                                ("There is no one who can teach that language here.\n\r",
                                 ch);
                        return;
                }
                if (ch->gold < 25)
                {
                        send_to_char
                                ("language lessons cost 25 credits... you don't have enough.\n\r",
                                 ch);
                        return;
                }
                ch->gold -= 25;
                /*
                 * Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag 
                 */
                prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);
                ch->pcdata->learned[sn] += static_cast<sh_int>(prct);
                ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn], 99);
                if (ch->pcdata->learned[sn] == prct)
                        act(AtPlain, "You begin lessons in $t.", ch,
                            language->name, NULL, ToChar);
                else if (ch->pcdata->learned[sn] < 60)
                        act(AtPlain, "You continue lessons in $t.", ch,
                            language->name, NULL, ToChar);
                else if (ch->pcdata->learned[sn] < 60 + prct)
                        act(AtPlain,
                            "You feel you can start communicating in $t.", ch,
                            language->name, NULL, ToChar);
                else if (ch->pcdata->learned[sn] < 99)
                        act(AtPlain, "You become more fluent in $t.", ch,
                            language->name, NULL, ToChar);
                else
                        act(AtPlain, "You now speak perfect $t.", ch,
                            language->name, NULL, ToChar);
                return;
        }

        if (arg[0] != '\0')
                victim = get_char_world(ch, arg);
        if (!victim || !IsImmortal(ch))
                victim = ch;
        if (IsNpc(victim))
                return;

        for (language = first_language; language; language = language->next)
        {
                if (!str_cmp(language->name, "clan"))
                        continue;
                if ((sn = skill_lookup(language->name)) < 0)
                        send_to_char("&B(&w  0&B) ", ch);
                else
                        ch_printf(ch, "&B(&w%3d&B) ",
                                  victim->pcdata->learned[sn]);
                if (victim->speaking == language)
                        ch_printf(ch, "&R%s", capitalize(language->name));
                else
                        ch_printf(ch, "&B%c&z%s", UPPER(language->name[0]),
                                  (language->name + 1));
                send_to_char("\n\r", ch);
        }
        send_to_char("\n\r", ch);
        return;
}

bool is_ignoring(CharData * ch, CharData * victim)
{
        int       pos;
        CharData *rch;

        if (ch->desc == NULL)
                rch = ch;
        else
                rch = ch->desc->original ? ch->desc->original : ch;

        if (IsNpc(rch) || IsNpc(victim))
                return FALSE;

        for (pos = 0; pos < MaxIgnore; pos++)
        {
                if (!IsNpc(rch))
                        if (rch->pcdata->ignore[pos] == NULL)
                                break;

                if (!str_cmp(rch->pcdata->ignore[pos], victim->name))
                        return TRUE;
        }

        return FALSE;
}

CMDF do_ignore(CharData * ch, char *argument)
{
        CharData *victim, *rch;
        int       pos;

        if (ch->desc == NULL)
                rch = ch;
        else
                rch = ch->desc->original ? ch->desc->original : ch;

        if (IsNpc(rch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char("Who do you want to ignore?\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "list"))
        {
                for (pos = 0; pos < MaxIgnore; pos++)
                {
                        if (rch->pcdata->ignore[pos] == NULL)
                                break;
                        ch_printf(ch, "&C[&c%d&C] &G%s&w\n\r", pos,
                                  rch->pcdata->ignore[pos]);
                }
                return;
        }


        if ((victim = get_char_world(rch, argument)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IsNpc(victim))
        {
                send_to_char("Ignore a mob?  I don't think so.\n\r", ch);
                return;
        }

        if (ch == victim)
        {
                send_to_char
                        ("I don't think you really want to ignore yourself.\n\r",
                         ch);
                return;
        }

        for (pos = 0; pos < MaxIgnore; pos++)
        {
                if (rch->pcdata->ignore[pos] == NULL)
                        break;

                if (!str_cmp(argument, rch->pcdata->ignore[pos]))
                {
                        STRFREE(rch->pcdata->ignore[pos]);
                        rch->pcdata->ignore[pos] = NULL;
                        ch_printf(ch, "You stop ignoring %s.\n\r",
                                  victim->name);
                        return;
                }
        }

        if (pos >= MaxIgnore)
        {
                send_to_char("You can't ignore anymore people\n\r", ch);
                return;
        }

        rch->pcdata->ignore[pos] = STRALLOC(argument);
        ch_printf(ch, "You now ignore %s.\n\r", victim->name);
        return;

}

/* tunes a characters comlink to a certain channel */
CMDF do_tune(CharData * ch, char *argument)
{

        char      arg[MaxInputLength];
        int       station;

        if (IsNpc(ch))
                return;

        if (!has_comlink(ch))
        {
                send_to_char("&RYou don't have a comlink to set!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                ch_printf(ch, "&BYour comlink display reads&G&W %d &B.\n\r",
                          ch->pcdata->comchan);
                return;
        }

        argument = one_argument(argument, arg);
        station = atoi(arg);

        if (station > 101 || station < 0)
        {
                send_to_char("&RAvailable stations are 0 through 100.\n\r",
                             ch);
                return;
        }

        if (station == 101 && !IsImmortal(ch))
        {
                send_to_char("&RAvailable stations are 0 through 100.\n\r",
                             ch);
                return;
        }

        ch_printf(ch, "&BYour comlink clicks as you set it to &W%d&B.\n\r",
                  station);
        act(AtSocial, "&B$n fiddles with a comlink for a second.", ch, NULL,
            NULL, ToRoom);

        ch->pcdata->comchan = station;
        return;
}

/* sends a message to the station your tuned too */
CMDF do_talk(CharData * ch, char *argument)
{
        DescriptorData *d;
        int       station;
        char      buf[MaxStringLength];

        if (IsNpc(ch))
                return;

        if (!has_comlink(ch))
        {
                send_to_char("&RYou don't seem to have a comlink!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Talk what?\n\r", ch);
                return;
        }

        if (!IsNpc(ch) && IsSet(ch->act, PlrSilence))
        {
                send_to_char("Your silenced.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        station = ch->pcdata->comchan;

        if (station <= 0)
        {
                send_to_char("&RYour comlink is not tuned to a station!\n\r",
                             ch);
                return;
        }
        ch_printf(ch,
                  "&B[&Woutgoing message&B]&W:&B [&W%d&B] &Wreads&B, &W'%s'\n\r",
                  station, drunk_speech(argument, ch));

        snprintf(buf, MSL,
                 "&B[&Wincoming message&B]&W:&B [&W%d&B/&W$n&B] &Wreads&B, &W'$t'",
                 station);
        for (d = first_descriptor; d; d = d->next)
        {
                [[maybe_unused]] CharData *och;
                CharData *vch;

                och = d->original ? d->original : d->character;
                vch = d->character;

                if (IsPlaying(d) && vch != ch && !IsNpc(vch))
                {
                        char     *sbuf = argument;

                        if (!has_comlink(vch))
                                continue;

                        if (xIS_SET(vch->in_room->RoomFlags, RoomSilence))
                                continue;

                        if (vch->pcdata && vch->pcdata->comchan != station
                            && vch->pcdata->comchan != 101)
                                continue;

                        if (!knows_language(vch, ch->speaking, ch) &&
                            (!IsNpc(ch) || ch->speaking != 0))
                                sbuf = scramble(argument, ch->speaking);

                        act(AtGossip, buf, ch, drunk_speech(sbuf, ch), vch,
                            ToVict);
                }
        }

        return;
}

char     *append_lang(const char *argument, CharData * ch,
                      CharData * victim)
{
        static char buf[MaxInputLength * 2];

        buf[0] = '\0';
        if (!IsNpc(victim) && !IsSet(victim->act, PlrBrief)
            && knows_language(victim, ch->speaking, ch))
        {
                snprintf(buf, MaxInputLength * 2, "(%s) ", ch->speaking->name);
        }
        mudstrlcat(buf, argument, MIL * 2);
        return buf;
}


CMDF do_say_to_char(CharData * ch, char *argument)
{
        char      arg[MaxInputLength], _last_char;
        char      buf[MaxStringLength];
        CharData *vch;
        CharData *victim;
        int       actflags;
        int       arglen;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Say what to whom?\n\r", ch);
                return;
        }
        if ((victim = get_char_room(ch, arg)) == NULL
            || (IsNpc(victim) && victim->in_room != ch->in_room)
            || (!NotAuthed(ch) && NotAuthed(victim) && !IsImmortal(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        arglen = static_cast<int>(strlen(argument)) - 1;
        /*
         * Remove whitespace and tabs. 
         */
        while (argument[arglen] == ' ' || argument[arglen] == '\t')
                --arglen;
        _last_char = argument[arglen];

        actflags = ch->act;
        if (IsNpc(ch))
                RemoveBit(ch->act, ActSecretive);
        MOBtrigger = FALSE;
        {
                char     *sbuf = argument;

                if (!knows_language(victim, ch->speaking, ch)
                    && (!IsNpc(ch) || ch->speaking != 0))
                        sbuf = scramble(argument, ch->speaking);

                switch (_last_char)
                {
                case '?':
                        act(AtSay, "You ask $N, '$t&c'", ch,
                            drunk_speech(argument, ch), victim, ToChar);
                        act(AtSay, "$n asks you '$t&c'", ch,
                            drunk_speech(sbuf, ch), victim, ToVict);
                        break;

                case '!':
                        act(AtSay, "You exclaim at $N, '$t&c'", ch,
                            drunk_speech(argument, ch), victim, ToChar);
                        act(AtSay, "$n exclaims to you, '$t&c'", ch,
                            drunk_speech(sbuf, ch), victim, ToVict);
                        break;

                default:
                        act(AtSay, "You say to $N '$t&c'", ch,
                            drunk_speech(argument, ch), victim, ToChar);
                        act(AtSay, "$n says to you '$t&c'", ch,
                            drunk_speech(sbuf, ch), victim, ToVict);
                        break;
                }
        }

        for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
        {
                char     *sbuf = argument;

                if (vch == ch || vch == victim)
                        continue;

                if (!knows_language(vch, ch->speaking, ch)
                    && (!IsNpc(ch) || ch->speaking != 0))
                        sbuf = scramble(argument, ch->speaking);

                switch (_last_char)
                {
                case '?':
                        snprintf(buf, MSL, "$n asks %s, '$t&c'",
                                 can_see(vch,
                                         victim) ? victim->name : "someone");
                        act(AtSay, buf, ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            ToVict);
                        break;

                case '!':
                        snprintf(buf, MSL, "$n exclaims at %s, '$t&c'",
                                 can_see(vch,
                                         victim) ? victim->name : "someone");
                        act(AtSay, buf, ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            ToVict);
                        break;

                default:
                        snprintf(buf, MSL, "$n says to %s, '$t&c'",
                                 can_see(vch,
                                         victim) ? victim->name : "someone");
                        act(AtSay, buf, ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            ToVict);
                        break;
                }
        }

        ch->act = actflags;
        MOBtrigger = TRUE;
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                snprintf(buf, MSL, "%s: %s",
                         IsNpc(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LogFile, buf);
        }
        mprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        oprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        rprog_speech_trigger(argument, ch);
        return;
}

// =============================================================================
// SPEECH COMMANDS
// =============================================================================

/*
 * Say command - speak to everyone in the room
 */
CMDF do_say(CharData* ch, const char* argument)
{
        CharData *vch;
        char      _last_char;
        int       actflags;
        int       arglen;

        if (argument[0] == '\0')
        {
                send_to_char("Say what?\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSilence))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        arglen = static_cast<int>(strlen(argument)) - 1;
        /*
         * Remove whitespace and tabs. 
         */
        while (argument[arglen] == ' ' || argument[arglen] == '\t')
                --arglen;
        _last_char = argument[arglen];

        actflags = ch->act;

        if (IsNpc(ch))
                RemoveBit(ch->act, ActSecretive);
        MOBtrigger = FALSE;
        switch (_last_char)
        {
        case '?':
                act(AtSay, "You ask, '$t&c'", ch, drunk_speech(argument, ch),
                    ch, ToChar);
                break;

        case '!':
                act(AtSay, "You exclaim, '$t&c'", ch,
                    drunk_speech(argument, ch), ch, ToChar);
                break;

        default:
                act(AtSay, "You say, '$t&c'", ch, drunk_speech(argument, ch),
                    ch, ToChar);
                break;
        }
        for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
        {
                const char *sbuf = argument;

                if (vch == ch)
                        continue;
                if (!knows_language(vch, ch->speaking, ch) &&
                    (!IsNpc(ch) || ch->speaking != 0))
                        sbuf = scramble(argument, ch->speaking);
                else
                {
                        ;
                }

                switch (_last_char)
                {
                case '?':
                        act(AtSay, "$n asks, '$t&c'", ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            ToVict);
                        break;

                case '!':
                        act(AtSay, "$n exclaims, '$t&c'", ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            ToVict);
                        break;

                default:
                        act(AtSay, "$n says, '$t&c'", ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            ToVict);
                        break;
                }

        }


        ch->act = actflags;
        MOBtrigger = FALSE;
        if (xIS_SET(ch->in_room->RoomFlags, RoomLogspeech))
        {
                char      buf[MaxStringLength];

                snprintf(buf, MSL, "%s: %s",
                         IsNpc(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LogFile, buf);
        }
        mprog_speech_trigger(const_cast<char*>(argument), ch);
        if (char_died(ch))
                return;
        oprog_speech_trigger(const_cast<char*>(argument), ch);
        if (char_died(ch))
                return;
        rprog_speech_trigger(const_cast<char*>(argument), ch);
        return;
}
