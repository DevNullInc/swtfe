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
 *                              Command Interpreter Module                               *
 ****************************************************************************************/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"
#include <stdlib.h>
#include "Account.hpp"
#include "alias.hpp"
#include "channels.hpp"

/*
 * Externals
 */

void      subtract_times(struct timeval *etime, struct timeval *systime);



void write_watch_files args((CharData * ch, CMDType * cmd, char *logline));
bool valid_watch args((char *logline));
bool check_social args((CharData * ch, char *command, char *argument));


/*
 * Log-all switch.
 */
bool      fLogAll = FALSE;


CMDType  *command_hash[126];    /* hash table for cmd_table */
SocialType *social_index[27];   /* hash table for socials   */

/*
 * Character not in position for command?
 */
bool check_pos(CharData * ch, sh_int position)
{
        if (ch->position < position)

        {
                switch (ch->position)
                {
                case PosDead:
                        send_to_char
                                ("A little difficult to do when you are DEAD...\n\r",
                                 ch);
                        break;

                case PosMortal:
                case PosIncap:
                        send_to_char("You are hurt far too bad for that.\n\r",
                                     ch);
                        break;

                case PosStunned:
                        send_to_char("You are too stunned to do that.\n\r",
                                     ch);
                        break;

                case PosSleeping:
                        send_to_char("In your dreams, or what?\n\r", ch);
                        break;

                case PosResting:
                        send_to_char("Nah... You feel too relaxed...\n\r",
                                     ch);
                        break;

                case PosSitting:
                        send_to_char("You can't do that sitting down.\n\r",
                                     ch);
                        break;

                case PosFighting:
                        send_to_char("No way!  You are still fighting!\n\r",
                                     ch);
                        break;

                }
                return FALSE;
        }
        return TRUE;
}

extern char lastplayercmd[MaxInputLength * 2];

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'Force'.
 */
void interpret(CharData * ch, char *argument)
{
        char      command[MaxInputLength];
        char      logline[MaxInputLength];
        char      logname[MaxInputLength];
        Timer    *timer = NULL;
        CMDType  *cmd = NULL;
        int       trust;
        int       loglvl;
        bool      found, chan;
        struct timeval time_used;
        long      tmptime;
        bool      ooc, held;
        int       string_count = allocated_strings();
        char      cmd_copy[MaxInputLength];
        char      buf[MaxStringLength];


        if (!ch)
        {
                bug("interpret: null ch!", 0);
                return;
        }

        found = FALSE;
        ooc = FALSE;
        held = FALSE;
        chan = FALSE;
        if (ch->substate == SubRepeatCmd)
        {
                DoFun   *fun;

                if ((fun = ch->last_cmd) == NULL)
                {
                        ch->substate = SubNone;
                        bug("interpret: SubRepeatCmd with NULL last_cmd", 0);
                        return;
                }
                else
                {
                        int       x;

                        /*
                         * yes... we lose out on the hashing speediness here...
                         * but the only REPEATCMDS are wizcommands (currently)
                         */
                        for (x = 0; x < 126; x++)
                        {
                                for (cmd = command_hash[x]; cmd;
                                     cmd = cmd->next)
                                        if (cmd->do_fun == fun)
                                        {
                                                found = TRUE;
                                                break;
                                        }
                                if (found)
                                        break;
                        }
                        if (!found)
                        {
                                cmd = NULL;
                                bug("interpret: SubRepeatCmd: last_cmd invalid", 0);
                                return;
                        }
                        snprintf(logline, MSL, "(%s) %s", cmd->name,
                                 argument);
                }
        }

        if (!cmd)
        {
                /*
                 * Changed the order of these ifchecks to prevent crashing. 
                 */
                if (!argument || !strcmp(argument, ""))
                {
                        bug("interpret: null argument!", 0);
                        return;
                }

                /*
                 * Strip leading spaces.
                 */
                while (isspace(*argument))
                        argument++;
                if (argument[0] == '\0')
                        return;

                timer = get_timerptr(ch, TimerDoFun);

                /*
                 * RemoveBit( ch->affected_by, AffHide ); 
                 */

                /*
                 * Implement freeze command.
                 */
                if (!IsNpc(ch) && IsSet(ch->act, PlrFreeze))
                {
                        send_to_char("You're totally frozen!\n\r", ch);
                        return;
                }

                mudstrlcpy(cmd_copy, argument, MIL);

                /*
                 * Grab the command word.
                 * Special parsing so ' can be a command,
                 *   also no spaces needed after punctuation.
                 */
                mudstrlcpy(logline, argument, MIL);
                if (argument[0] != '@' && !isalpha(argument[0])
                    && !isdigit(argument[0]))
                {
                        command[0] = argument[0];
                        command[1] = '\0';
                        argument++;
                        while (isspace(*argument))
                                argument++;
                }
                else
                        argument = one_argument(argument, command);

                /*
                 * Look for command in command table.
                 * Check for council powers and/or bestowments
                 */
                trust = get_trust(ch);
                for (cmd = command_hash[LOWER(command[0]) % 126]; cmd;
                     cmd = cmd->next)
                        if (!str_prefix(command, cmd->name)
                            && (check_command(ch, cmd)
                                || (!IsNpc(ch) && ch->pcdata->bestowments
                                    && ch->pcdata->bestowments[0] != '\0'
                                    && is_name(cmd->name,
                                               ch->pcdata->bestowments))))
                        {
                                found = TRUE;
                                break;
                        }

                /*
                 * Turn off afk bit when any command performed.
                 */
                if (IsSet(ch->act, PlrAfk) && (str_cmp(command, "AFK")))
                {
                        RemoveBit(ch->act, PlrAfk);
                        act(AtGrey, "$n is no longer afk.", ch, NULL, NULL,
                            ToRoom);
                }
        }

        /*
         * Log and snoop.
         */
                snprintf(lastplayercmd, MSL, "** %s: %s", ch->name, logline);

        if (found && cmd->log == LogNever)
                mudstrlcpy(logline, "XXXXXXXX XXXXXXXX XXXXXXXX", MIL);

        loglvl = found ? cmd->log : (int) LogNormal;

        /*
         * Write input line to watch files if applicable
         */
        if (!IsNpc(ch) && ch->desc && valid_watch(logline))
        {
                if (found && IsSet(cmd->flags, CmdWatch))
                        write_watch_files(ch, cmd, logline);
                else if (IsSet(ch->pcdata->flags, PcflagWatch))
                        write_watch_files(ch, NULL, logline);
        }

        if ((!IsNpc(ch) && IsSet(ch->act, PlrLog))
            || fLogAll
            || loglvl == LogBuild
            || loglvl == LogHigh || loglvl == LogAlways)
        {
                /*
                 * Added by Narn to show who is switched into a mob that executes
                 * a logged command.  Check for descriptor in case Force is used. 
                 */
                if (ch->desc && ch->desc->original)
                        snprintf(log_buf, MSL, "Log %s (%s): %s", ch->name,
                                 ch->desc->original->name, logline);
                else
                        snprintf(log_buf, MSL, "Log %s: %s", ch->name,
                                 logline);

                /*
                 * Make it so a 'log all' will send most output to the log
                 * file only, and not spam the log channel to death -Thoric
                 */
                if (fLogAll && loglvl == LogNormal
                    && (IsNpc(ch) || !IsSet(ch->act, PlrLog)))
                        loglvl = LogAll;

                log_string_plus(log_buf, loglvl, get_trust(ch));
        }

        if (ch->desc && ch->desc->snoop_by)
        {
                snprintf(logname, MSL, "%s", ch->name);
                write_to_buffer(ch->desc->snoop_by, logname, 0);
                write_to_buffer(ch->desc->snoop_by, "% ", 2);
                write_to_buffer(ch->desc->snoop_by, logline, 0);
                write_to_buffer(ch->desc->snoop_by, "\n\r", 2);
                append_file(ch, LogFile, logline);
        }



        if (found)
        {
                /*
                 * all imm commands are ooc 
                 */
                if (IsSet(cmd->flags, CmdOoc)
                    || cmd->level > LevelImmortal)
                        ooc = TRUE;
                if (IsSet(cmd->flags, CmdHeld))
                        held = TRUE;

				if (IsSet(cmd->flags, CmdFullname) && strcmp(command,cmd->name) ) {
					set_char_color(AtRed, ch);
					ch_printf(ch, "If you want to %s, you have to spell it out.\n\r",strupper(cmd->name));
					return;
				}


        }

        if (!found)
        {
                int       sn;

                sn = skill_lookup(command);
                if (sn != -1 && skill_table[sn]->held)
                        held = TRUE;
                if (find_social(command))
                        held = TRUE;
        }
        if (!found)
        {
                ChannelData *channel;

                if ((channel = get_channel(command)) != NULL
                    && channel->type == ChannelOoc)
                        chan = TRUE;
        }

        if (ch->held == TRUE && !ooc && !held)
        {
                send_to_char("You cannot do that while restrained!\n\r", ch);
                return;
        }

        if (timer && !ooc && !chan)
        {
                int       tempsub;

                tempsub = ch->substate;
                ch->substate = SubTimerDoAbort;
                (timer->do_fun) (ch, "");
                if (char_died(ch))
                        return;
                if (ch->substate != SubTimerCantAbort)
                {
                        ch->substate = tempsub;
                        extract_timer(ch, timer);
                }
                else
                {
                        ch->substate = tempsub;
                        return;
                }
        }

        /*
         * Look for command in skill and socials table.
         */
        if (!found)
        {
                if (!check_skill(ch, command, argument)
                    && !check_channel(ch, command, argument)
                    && !check_alias(ch, command, argument)
                    && !check_social(ch, command, argument)
#ifdef IMC
                    && !imc_command_hook(ch, command, argument)
#endif
                        )
                {
                        ExitData *pexit;

                        /*
                         * check for an auto-matic exit command 
                         */
                        if ((pexit = find_door(ch, command, TRUE)) != NULL
                            && IsSet(pexit->exit_info, EX_xAUTO))
                        {
                                if (IsSet(pexit->exit_info, ExClosed)
                                    && (!IsAffected(ch, AffPassDoor)
                                        || IsSet(pexit->exit_info,
                                                  ExNopassdoor)))
                                {
                                        if (!IsSet
                                            (pexit->exit_info, ExSecret))
                                                act(AtPlain,
                                                    "The $d is closed.", ch,
                                                    NULL, pexit->keyword,
                                                    ToChar);
                                        else
                                                send_to_char
                                                        ("You cannot do that here.\n\r",
                                                         ch);
                                        return;
                                }
                                move_char(ch, pexit, 0, FALSE);
                                return;
                        }
                        send_to_char("Huh?\n\r", ch);
                }
                return;
        }

        /*
         * Character not in position for command?
         */
        if (!check_pos(ch, cmd->position))
                return;

        /*
         * Berserk check for flee.. maybe add drunk to this?.. but too much
         * hardcoding is annoying.. -- Altrag 
         */
        if (!str_cmp(cmd->name, "flee") && IsAffected(ch, AffBerserk))
        {
                send_to_char("You aren't thinking very clearly..\n\r", ch);
                return;
        }

        /*
         * Dispatch the command.
         */
        ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
        ch->last_cmd = cmd->do_fun;
        start_timer(&time_used);
        (*cmd->do_fun) (ch, argument);
        end_timer(&time_used);
        /*
         * Update the record of how many times this command has been used (haus)
         */
        update_userec(&time_used, &cmd->userec);
        tmptime = UMIN(time_used.tv_sec, 19) * 1000000 + time_used.tv_usec;

        /*
         * laggy command notice: command took longer than 1.5 seconds 
         */
        if (tmptime > 1500000)
        {
                snprintf(log_buf, MSL,
                         "[*****] LAG: %s: %s %s (R:%d S:%d.%06d)", ch->name,
                         cmd->name,
                         (cmd->log == LogNever ? "XXX" : argument),
                         ch->in_room ? ch->in_room->vnum : 0,
                         (int) (time_used.tv_sec), (int) (time_used.tv_usec));
                log_string_plus(log_buf, LogNormal, get_trust(ch));
        }

        if (!sysdata.PORT && string_count < allocated_strings())
        {
                snprintf(buf, MSL,
                         "Memcheck : Increase in strings :: %s : %s",
                         ch->name, cmd_copy);
                log_string(buf);
        }

        tail_chain();
}

/* Register custom imm commands */
void register_custom_commands(void)
{
        /* Commands added here are immortal-level commands */
        /* Expecting add_cmd(name, function, position, level, log, show) helper exists; otherwise use existing mechanisms to add to command table */
        if (command_hash[0] == NULL)
                return; /* safe no-op if init order differs */
        /* Fallback: use existing command table insertion function if available */
        /* If your code has a build-time command table, you may instead add these statically. */
}

CMDType  *find_command(char *command)
{
        CMDType  *cmd;
        int       hash;

        hash = LOWER(command[0]) % 126;

        for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
                if (!str_prefix(command, cmd->name))
                        return cmd;

        return NULL;
}

SocialType *find_social(char *command)
{
        SocialType *social;
        int       hash;

        if (command[0] < 'a' || command[0] > 'z')
                hash = 0;
        else
                hash = (command[0] - 'a') + 1;

        for (social = social_index[hash]; social; social = social->next)
                if (!str_prefix(command, social->name)
                    && social->minarousal == 0)
                        return social;

        return NULL;
}

SocialType *find_xsocial(char *command)
{
        SocialType *social;
        int       hash;

        if (command[0] < 'a' || command[0] > 'z')
                hash = 0;
        else
                hash = (command[0] - 'a') + 1;

        for (social = social_index[hash]; social; social = social->next)
                if (!str_prefix(command, social->name))
                        return social;

        return NULL;
}

bool check_social(CharData * ch, char *command, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        SocialType *social;

        if ((social = find_social(command)) == NULL)
                return FALSE;

        if (!IsNpc(ch) && IsSet(ch->act, PlrNoEmote))
        {
                send_to_char("You are anti-social!\n\r", ch);
                return TRUE;
        }

        switch (ch->position)
        {
        case PosDead:
                send_to_char("Lie still; you are DEAD.\n\r", ch);
                return TRUE;

        case PosIncap:
        case PosMortal:
                send_to_char("You are hurt far too bad for that.\n\r", ch);
                return TRUE;

        case PosStunned:
                send_to_char("You are too stunned to do that.\n\r", ch);
                return TRUE;

        case PosSleeping:
                /*
                 * I just know this is the path to a 12" 'if' statement.  :(
                 * But two players asked for it already!  -- Furey
                 */
                if (!str_cmp(social->name, "snore"))
                        break;
                send_to_char("In your dreams, or what?\n\r", ch);
                return TRUE;

        }

        one_argument(argument, arg);
        victim = NULL;
        if (arg[0] == '\0')
        {
                act(AtSocial, social->others_no_arg, ch, NULL, victim,
                    ToRoom);
                act(AtSocial, social->char_no_arg, ch, NULL, victim,
                    ToChar);
        }
        else if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
        }
        else if (victim == ch)
        {
                act(AtSocial, social->others_auto, ch, NULL, victim,
                    ToRoom);
                act(AtSocial, social->char_auto, ch, NULL, victim, ToChar);
        }
        else
        {
                act(AtSocial, social->others_found, ch, NULL, victim,
                    ToNotvict);
                act(AtSocial, social->char_found, ch, NULL, victim, ToChar);
                act(AtSocial, social->vict_found, ch, NULL, victim, ToVict);

                if (!IsNpc(ch) && IsNpc(victim)
                    && !IsAffected(victim, AffCharm)
                    && IsAwake(victim)
                    && !IsSet(victim->pIndexData->progtypes, ActProg))
                {
                        switch (number_bits(4))
                        {
                        case 0:
                                if (!xIS_SET
                                    (ch->in_room->RoomFlags, RoomSafe)
                                    || IsEvil(ch))
                                        multi_hit(victim, ch, TypeUndefined);
                                else if (IsNeutral(ch))
                                {
                                        act(AtAction, "$n slaps $N.", victim,
                                            NULL, ch, ToNotvict);
                                        act(AtAction, "You slap $N.", victim,
                                            NULL, ch, ToChar);
                                        act(AtAction, "$n slaps you.",
                                            victim, NULL, ch, ToVict);
                                }
                                else
                                {
                                        act(AtAction,
                                            "$n acts like $N doesn't even exist.",
                                            victim, NULL, ch, ToNotvict);
                                        act(AtAction, "You just ignore $N.",
                                            victim, NULL, ch, ToChar);
                                        act(AtAction,
                                            "$n appears to be ignoring you.",
                                            victim, NULL, ch, ToVict);
                                }
                                break;

                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                                act(AtSocial, social->others_found,
                                    victim, NULL, ch, ToNotvict);
                                act(AtSocial, social->char_found,
                                    victim, NULL, ch, ToChar);
                                act(AtSocial, social->vict_found,
                                    victim, NULL, ch, ToVict);
                                break;

                        case 9:
                        case 10:
                        case 11:
                        case 12:
                                act(AtAction, "$n slaps $N.", victim, NULL,
                                    ch, ToNotvict);
                                act(AtAction, "You slap $N.", victim, NULL,
                                    ch, ToChar);
                                act(AtAction, "$n slaps you.", victim, NULL,
                                    ch, ToVict);
                                break;
                        }
                }
        }

        return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number(char *arg)
{
        if (*arg == '\0')
                return FALSE;

        for (; *arg != '\0'; arg++)
        {
                if (!isdigit(*arg))
                        return FALSE;
        }

        return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument(char *argument, char *arg)
{
        char     *pdot;
        int       number;

        for (pdot = argument; *pdot != '\0'; pdot++)
        {
                if (*pdot == '.')
                {
                        *pdot = '\0';
                        number = atoi(argument);
                        *pdot = '.';
                        mudstrlcpy(arg, pdot + 1, MIL);
                        return number;
                }
        }

        mudstrlcpy(arg, argument, MIL);
        return 1;
}

CMDF do_timecmd(CharData * ch, char *argument)
{
        struct timeval systime;
        struct timeval etime;
        static bool timing;
        extern CharData *timechar;
        char      arg[MaxInputLength];

        send_to_char("Timing\n\r", ch);
        if (timing)
                return;
        one_argument(argument, arg);
        if (!*arg)
        {
                send_to_char("No command to time.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "update"))
        {
                if (timechar)
                        send_to_char
                                ("Another person is already timing updates.\n\r",
                                 ch);
                else
                {
                        timechar = ch;
                        send_to_char
                                ("Setting up to record next update loop.\n\r",
                                 ch);
                }
                return;
        }
        set_char_color(AtPlain, ch);
        send_to_char("Starting timer.\n\r", ch);
        timing = TRUE;
        gettimeofday(&systime, NULL);
        interpret(ch, argument);
        gettimeofday(&etime, NULL);
        timing = FALSE;
        set_char_color(AtPlain, ch);
        send_to_char("Timing complete.\n\r", ch);
        subtract_times(&etime, &systime);
        ch_printf(ch, "Timing took %d.%06d seconds.\n\r",
                  etime.tv_sec, etime.tv_usec);
        return;
}

void start_timer(struct timeval *systime)
{
        if (!systime)
        {
                bug("Start_timer: NULL systime.", 0);
                return;
        }
        gettimeofday(systime, NULL);
        return;
}

time_t end_timer(struct timeval * systime)
{
        struct timeval etime;

        /*
         * Mark etime before checking systime, so that we get a better reading.. 
         */
        gettimeofday(&etime, NULL);
        if (!systime || (!systime->tv_sec && !systime->tv_usec))
        {
                bug("End_timer: bad systime.", 0);
                return 0;
        }
        subtract_times(&etime, systime);
        /*
         * systime becomes time used 
         */
        *systime = etime;
        return (etime.tv_sec * 1000000) + etime.tv_usec;
}

void send_timer(struct timerset *vtime, CharData * ch)
{
        struct timeval ntime;
        int       carry;

        if (vtime->num_uses == 0)
                return;
        ntime.tv_sec = vtime->total_time.tv_sec / vtime->num_uses;
        carry = (vtime->total_time.tv_sec % vtime->num_uses) * 1000000;
        ntime.tv_usec = (vtime->total_time.tv_usec + carry) / vtime->num_uses;
        ch_printf(ch, "Has been used %d times this boot.\n\r",
                  vtime->num_uses);
        ch_printf(ch,
                  "Time (in secs): min %d.%0.6d; avg: %d.%0.6d; max %d.%0.6d"
                  "\n\r", vtime->min_time.tv_sec, vtime->min_time.tv_usec,
                  ntime.tv_sec, ntime.tv_usec, vtime->max_time.tv_sec,
                  vtime->max_time.tv_usec);
        return;
}

void update_userec(struct timeval *time_used, struct timerset *userec)
{
        userec->num_uses++;
        if (!timerisset(&userec->min_time)
            || timercmp(time_used, &userec->min_time, <))
        {
                userec->min_time.tv_sec = time_used->tv_sec;
                userec->min_time.tv_usec = time_used->tv_usec;
        }
        if (!timerisset(&userec->max_time)
            || timercmp(time_used, &userec->max_time, >))
        {
                userec->max_time.tv_sec = time_used->tv_sec;
                userec->max_time.tv_usec = time_used->tv_usec;
        }
        userec->total_time.tv_sec += time_used->tv_sec;
        userec->total_time.tv_usec += time_used->tv_usec;
        while (userec->total_time.tv_usec >= 1000000)
        {
                userec->total_time.tv_sec++;
                userec->total_time.tv_usec -= 1000000;
        }
        return;
}

bool check_command(CharData * ch, CMDType * command)
{
        sh_int    i = 0;

/* Temporary fix here for switched character, is this what we want? - Grev */
        if (ch->desc && ch->desc->original)
                ch = ch->desc->original;

        if (!IsImmortal(ch))
        {
                if (command->level <= get_trust(ch))
                        return TRUE;
                else
                        return FALSE;
        }

        if (command->level < (MaxLevel - 4))
                return TRUE;

        if (!ch || IsNpc(ch) || !ch->pcdata || !ch->pcdata->godflags)
                return FALSE;

        if (command->perm_flags == 0 || IsSet(ch->pcdata->godflags, ImmAll)
            || IsSet(ch->pcdata->godflags, ImmOwner)
            || IsSet(command->perm_flags, CommandAll))
                return TRUE;

        for (i = 0; i < 32; i++)
                if (IsSet(ch->pcdata->godflags, 1 << i)
                    && IsSet(command->perm_flags, 1 << i))
                        return TRUE;

        return FALSE;
}

/*
 * Determine if this input line is eligible for writing to a watch file.
 * We don't want to write movement commands like (n, s, e, w, etc.)
 */
bool valid_watch(char *logline)
{
        int       len = strlen(logline);
        char      c = logline[0];

        if (len == 1
            && (c == 'n' || c == 's' || c == 'e' || c == 'w' || c == 'u'
                || c == 'd'))
                return FALSE;
        if (len == 2 && c == 'n' && (logline[1] == 'e' || logline[1] == 'w'))
                return FALSE;
        if (len == 2 && c == 's' && (logline[1] == 'e' || logline[1] == 'w'))
                return FALSE;

        return TRUE;
}


/*
 * Write input line to watch files if applicable
 */
void write_watch_files(CharData * ch, CMDType * cmd, char *logline)
{
        WatchData *pw;
        FILE     *fp;
        char      fname[MaxInputLength], buf[MaxStringLength];
        struct tm *t = localtime(&current_time);

        if (!first_watch)   /* no active watches */
                return;

        /*
         * if we're watching a command we need to do some special stuff 
         */
        /*
         * to avoid duplicating log lines - relies upon watch list being 
         */
        /*
         * sorted by imm name 
         */
        if (cmd)
        {
                char     *cur_imm;
                bool      found;

                pw = first_watch;
                while (pw)
                {
                        found = FALSE;

                        for (cur_imm = pw->imm_name;
                             pw && !strcmp(pw->imm_name, cur_imm);
                             pw = pw->next)
                        {

                                if (!found && ch->desc
                                    && get_trust(ch) < pw->imm_level
                                    &&
                                    ((pw->target_name
                                      && !strcmp(cmd->name, pw->target_name))
                                     || (pw->player_site
                                         && !str_prefix(pw->player_site,
                                                        ch->desc->host))
                                     || (ch->pcdata && ch->pcdata->Account
                                         && pw->player_account
                                         && !str_cmp(pw->player_account,
                                                     ch->pcdata->Account->
                                                     name))))
                                {
                                        sprintf(fname, "%s%s", WatchDir,
                                                strlower(pw->imm_name));
                                        if (!(fp = fopen(fname, "a+")))
                                        {
                                                sprintf(buf, "%s%s",
                                                        "Write_watch_files: Cannot open ",
                                                        fname);
                                                bug(buf, 0);
                                                perror(fname);
                                                return;
                                        }
                                        sprintf(buf,
                                                "%.2d/%.2d %.2d:%.2d %s: %s\n\r",
                                                t->tm_mon + 1, t->tm_mday,
                                                t->tm_hour, t->tm_min,
                                                ch->name, logline);
                                        fputs(buf, fp);
                                        fclose(fp);
                                        found = TRUE;
                                }
                        }
                }
        }
        else
        {
                for (pw = first_watch; pw; pw = pw->next)
                        if (((pw->target_name
                              && !str_cmp(pw->target_name, ch->name))
                             || (pw->player_site
                                 && !str_prefix(pw->player_site,
                                                ch->desc->host))
                             || (ch->pcdata && ch->pcdata->Account
                                 && pw->player_account
                                 && !str_cmp(pw->player_account,
                                             ch->pcdata->Account->name)))
                            && get_trust(ch) < pw->imm_level && ch->desc)
                        {
                                sprintf(fname, "%s%s", WatchDir,
                                        strlower(pw->imm_name));
                                if (!(fp = fopen(fname, "a+")))
                                {
                                        sprintf(buf, "%s%s",
                                                "Write_watch_files: Cannot open ",
                                                fname);
                                        bug(buf, 0);
                                        perror(fname);
                                        return;
                                }
                                sprintf(buf, "%.2d/%.2d %.2d:%.2d %s: %s\n\r",
                                        t->tm_mon + 1, t->tm_mday, t->tm_hour,
                                        t->tm_min, ch->name, logline);
                                fputs(buf, fp);
                                fclose(fp);
                        }
        }

        return;
}
