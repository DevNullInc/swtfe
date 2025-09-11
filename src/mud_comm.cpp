/*****************************************************************************************
 *                                                                                       *
 *  ________  __  __  ________       ______  ______  __   __  ________  __              *
 * /_______/\/_/\/_/\/______/\     /_____/\/_____/\/_/\ /_/\/______/\/_/\             *
 * \__.::._\/\:\ \:\ \::::_\/     \::::_\/\:::_ \ \:\ \\ \ \::::_\/_\:\ \            *
 *   /::\ \  \:\ \:\ \:\/___/\     \:\/___/\:(_) ) )\:\ \\ \ \:\/___/\\:\ \           *
 *  _\::\ \__\:\ \:\ \::___\/      \:::_ \ \: __ `\ \:\_/.:\ \::___\/_\:\ \____       *
 * /__\::\__/\\:\_\:\ \:\____/\      \:\ \ \ \ \ `\ \ \ ..::/ /\:\____/\\:\/___/\      *
 * \________\/ \_____\/ \_____\/       \__\/ \_\  \__\/ \___/_/ \_____\/ \_____\/      *
 *                                                                                       *
 *                       ______  ______  ______  ______  ______  _____   ______        *
 *                      /_____/\/_____/\/_____/\/_____/\/_____/\/___/\/_____/\       *
 *                      \::::_\/\:::_ \ \:\ \ \ \::::_\/\::::_\/\::\ \\:::_ \ \      *
 *                       \:\/___/\:(_) )\\ \:\ \\ \:\/___/\:\/___/\::\/_\\:(_) ) )     *
 *                        \:::_ \ \: __ `\ \:\ \\ \::___\/ \::___\/ \::\ \\: __ `\ \   *
 *                         \:\ \ \ \ \ `\ \ \:\_\\ \:\____/\ \:\____/\ \::\ \ \ `\ \ \  *
 *                          \__\/ \_\  \__\/\_____\/_____\/ \_____\/ \__\/ \_\  \__\/ *
 *                                                                                       *
 *                               T H E   F I N A L   E P I S O D E                       *
 *                                                                                       *
 *****************************************************************************************
 *                                                                                       *
 * 2003 by Michael Ervin, Mark Gottselig, Gavin Mogan                                    *
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
 * Communication system for player interactions, channels, messaging, and social commands in the game. *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "mud.hpp"
#include "races.hpp"

ch_ret    simple_damage(CharData * ch, CharData * victim, int dam, int dt);
CharData *get_char_room_mp args((CharData * ch, char *argument));

bool      MOBtrigger;

char     *mprog_type_to_name(int type)
{
        switch (type)
        {
        case InFileProg:
                return "in_file_prog";
        case ActProg:
                return "act_prog";
        case SpeechProg:
                return "speech_prog";
        case RandProg:
                return "rand_prog";
        case FightProg:
                return "fight_prog";
        case HitprcntProg:
                return "hitprcnt_prog";
        case DeathProg:
                return "death_prog";
        case EntryProg:
                return "entry_prog";
        case GreetProg:
                return "greet_prog";
        case AllGreetProg:
                return "all_greet_prog";
        case GiveProg:
                return "give_prog";
        case BribeProg:
                return "bribe_prog";
        case HourProg:
                return "hour_prog";
        case TimeProg:
                return "time_prog";
        case WearProg:
                return "wear_prog";
        case RemoveProg:
                return "remove_prog";
        case SacProg:
                return "sac_prog";
        case LookProg:
                return "look_prog";
        case ExaProg:
                return "exa_prog";
        case ZapProg:
                return "zap_prog";
        case GetProg:
                return "get_prog";
        case DropProg:
                return "drop_prog";
        case RepairProg:
                return "repair_prog";
        case DamageProg:
                return "damage_prog";
        case PullProg:
                return "pull_prog";
        case PushProg:
                return "push_prog";
        case ScriptProg:
                return "script_prog";
        case SleepProg:
                return "sleep_prog";
        case RestProg:
                return "rest_prog";
        case LeaveProg:
                return "leave_prog";
        case UseProg:
                return "use_prog";
        default:
                return "ErrorProg";
        }
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MUDprograms which are set.
 */
CMDF do_mpstat(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        MProgData *mprg;
        CharData *victim;

        if (IsAffected(ch, AffCharm))
                return;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("MProg stat whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!IsNpc(victim))
        {
                send_to_char("Only Mobiles can have MobPrograms!\n\r", ch);
                return;
        }

        if (!(victim->pIndexData->progtypes))
        {
                send_to_char("That Mobile has no Programs set.\n\r", ch);
                return;
        }

        ch_printf(ch, "Name: %s.  Vnum: %d.\n\r",
                  victim->name, victim->pIndexData->vnum);

        ch_printf(ch, "Short description: %s.\n\rLong  description: %s",
                  victim->short_descr,
                  victim->long_descr[0] != '\0' ?
                  victim->long_descr : "(none).\n\r");

        ch_printf(ch, "Hp: %d/%d.  Endurance: %d/%d.\n\r",
                  victim->hit, victim->max_hit,
                  victim->endurance, victim->max_endurance);

        ch_printf(ch,
                  "Lv: %d.  Align: %d.  AC: %d.  Credits: %d.\n\r",
                  victim->top_level, victim->alignment,
                  GetAc(victim), victim->gold);

		int count = 0;
        for (mprg = victim->pIndexData->mudprogs; mprg; mprg = mprg->next)
		{
				count++;
                ch_printf(ch, "%d>%s %s\n\r%s\n\r",
						  count,
                          mprog_type_to_name(mprg->type),
                          mprg->arglist, mprg->comlist);
		}
        return;
}

/* Opstat - Scryn 8/12*/
CMDF do_opstat(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        MProgData *mprg;
        ObjData *obj;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("OProg stat what?\n\r", ch);
                return;
        }

        if ((obj = get_obj_world(ch, arg)) == NULL)
        {
                send_to_char("You cannot find that.\n\r", ch);
                return;
        }

        if (!(obj->pIndexData->progtypes))
        {
                send_to_char("That object has no programs set.\n\r", ch);
                return;
        }

        ch_printf(ch, "Name: %s.  Vnum: %d.\n\r",
                  obj->name, obj->pIndexData->vnum);

        ch_printf(ch, "Short description: %s.\n\r", obj->short_descr);

		int count = 0;
        for (mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next) {
				count++;
                ch_printf(ch, "%d>%s %s\n\r%s\n\r",
						  count,
                          mprog_type_to_name(mprg->type),
                          mprg->arglist, mprg->comlist);
		}
        return;

}

/* Rpstat - Scryn 8/12 */
CMDF do_rpstat(CharData * ch, char *argument)
{
		int count = 0;
        MProgData *mprg;

        argument = NULL;

        if (!(ch->in_room->progtypes))
        {
                send_to_char("This room has no programs set.\n\r", ch);
                return;
        }

        ch_printf(ch, "Name: %s.  Vnum: %d.\n\r",
                  ch->in_room->name, ch->in_room->vnum);

        for (mprg = ch->in_room->mudprogs; mprg; mprg = mprg->next) {
				count++;
                ch_printf(ch, "%d>%s %s\n\r%s\n\r",
						  count,
                          mprog_type_to_name(mprg->type),
                          mprg->arglist, mprg->comlist);
		}
        return;
}

/* Prints the argument to all the rooms around the mobile */
CMDF do_mpasound(CharData * ch, char *argument)
{
        RoomIndexData *was_in_room;
        ExitData *pexit;
        int       actflags;


        if (!ch)
        {
                bug("Nonexistent ch in do_mpasound!", 0);
                return;
        }

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                progbug("Mpasound - No argument", ch);
                return;
        }

        actflags = ch->act;
        RemoveBit(ch->act, ActSecretive);
        was_in_room = ch->in_room;
        for (pexit = was_in_room->first_exit; pexit; pexit = pexit->next)
        {
                if (pexit->to_room && pexit->to_room != was_in_room)
                {
                        ch->in_room = pexit->to_room;
                        MOBtrigger = FALSE;
                        act(AtSay, argument, ch, NULL, NULL, ToRoom);
                }
        }
        ch->act = actflags;
        ch->in_room = was_in_room;
        return;
}

/* lets the mobile kill any player or mobile without murder*/

CMDF do_mpkill(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;

        if (!ch)
        {
                bug("Nonexistent ch in do_mpkill!", 0);
                return;
        }

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                progbug("MpKill - no argument", ch);
                return;
        }

        if ((victim = get_char_room_mp(ch, arg)) == NULL)
        {
                progbug("MpKill - Victim not in room", ch);
                return;
        }

        if (victim == ch)
        {
                progbug("MpKill - Bad victim to attack", ch);
                return;
        }

        if (IsAffected(ch, AffCharm) && ch->master == victim)
        {
                progbug("MpKill - Charmed mob attacking master", ch);
                return;
        }

        if (ch->position == PosFighting)
        {
                progbug("MpKill - Already fighting", ch);
                return;
        }

        multi_hit(ch, victim, TypeUndefined);
        return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */

CMDF do_mpjunk(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ObjData *obj_next;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                progbug("Mpjunk - No argument", ch);
                return;
        }

        if (str_cmp(arg, "all") && str_prefix("all.", arg))
        {
                if ((obj = get_obj_wear(ch, arg)) != NULL)
                {
                        unequip_char(ch, obj);
                        extract_obj(obj);
                        return;
                }
                if ((obj = get_obj_carry(ch, arg)) == NULL)
                        return;
                extract_obj(obj);
        }
        else
                for (obj = ch->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        if (arg[3] == '\0' || is_name(&arg[4], obj->name))
                        {
                                if (obj->wear_loc != WearNone)
                                        unequip_char(ch, obj);
                                extract_obj(obj);
                        }
                }

        return;

}

/*
 * This function examines a text string to see if the first "word" is a
 * color indicator (e.g. _red, _whi_, _blu).  -  Gorog
 */
int get_color(char *argument)   /* get color code from command string */
{
        char      color[MaxInputLength];
        char     *cptr;
        static char const *color_list =
                "_bla_red_dgr_bro_dbl_pur_cya_cha_dch_ora_gre_yel_blu_pin_lbl_whi";
        static char const *blink_list =
                "*bla*red*dgr*bro*dbl*pur*cya*cha*dch*ora*gre*yel*blu*pin*lbl*whi";

        one_argument(argument, color);
        if (color[0] != '_' && color[0] != '*')
                return 0;
        if ((cptr = const_cast<char*>(strstr(color_list, color))))
                return (cptr - color_list) / 4;
        if ((cptr = const_cast<char*>(strstr(blink_list, color))))
                return (cptr - blink_list) / 4 + AtBlink;
        return 0;
}


/* prints the message to everyone in the room other than the mob and victim */

CMDF do_mpechoaround(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        int       actflags;
        sh_int    color;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                progbug("Mpechoaround - No argument", ch);
                return;
        }

        if (!(victim = get_char_room_mp(ch, arg)))
        {
                progbug("Mpechoaround - victim does not exist", ch);
                return;
        }

        actflags = ch->act;
        RemoveBit(ch->act, ActSecretive);

        if ((color = get_color(argument)))
        {
                argument = one_argument(argument, arg);
                act(color, argument, ch, NULL, victim, ToNotvict);
        }
        else
                act(AtAction, argument, ch, NULL, victim, ToNotvict);

        ch->act = actflags;
}


/* prints message only to victim */

CMDF do_mpechoat(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        int       actflags;
        sh_int    color;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                progbug("Mpechoat - No argument", ch);
                return;
        }

        if (!(victim = get_char_room_mp(ch, arg)))
        {
                progbug("Mpechoat - victim does not exist", ch);
                return;
        }

        actflags = ch->act;
        RemoveBit(ch->act, ActSecretive);

        if ((color = get_color(argument)))
        {
                argument = one_argument(argument, arg);
                act(color, argument, ch, NULL, victim, ToVict);
        }
        else
                act(AtAction, argument, ch, NULL, victim, ToVict);

        ch->act = actflags;
}


/* prints message to room at large. */

CMDF do_mpecho(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        sh_int    color;
        int       actflags;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                progbug("Mpecho - called w/o argument", ch);
                return;
        }

        actflags = ch->act;
        RemoveBit(ch->act, ActSecretive);

        if ((color = get_color(argument)))
        {
                argument = one_argument(argument, arg1);
                act(color, argument, ch, NULL, NULL, ToRoom);
        }
        else
                act(AtAction, argument, ch, NULL, NULL, ToRoom);

        ch->act = actflags;
}


/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */

CMDF do_mpmload(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        MobIndexData *pMobIndex;
        CharData *victim;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0' || !is_number(arg))
        {
                progbug("Mpmload - Bad vnum as arg", ch);
                return;
        }

        if ((pMobIndex = get_mob_index(atoi(arg))) == NULL)
        {
                progbug("Mpmload - Bad mob vnum", ch);
                return;
        }

        victim = create_mobile(pMobIndex);
        char_to_room(victim, ch->in_room);
        return;
}

CMDF do_mpoload(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        ObjIndexData *pObjIndex;
        ObjData *obj;
        int       level;
        int       timer = 0;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || !is_number(arg1))
        {
                progbug("Mpoload - Bad syntax", ch);
                return;
        }

        if (arg2[0] == '\0')
                level = get_trust(ch);
        else
        {
                /*
                 * New feature from Alander.
                 */
                if (!is_number(arg2))
                {
                        progbug("Mpoload - Bad level syntax", ch);
                        return;
                }
                level = atoi(arg2);
                if (level < 0 || level > get_trust(ch))
                {
                        progbug("Mpoload - Bad level", ch);
                        return;
                }

                /*
                 * New feature from Thoric.
                 */
                timer = atoi(argument);
                if (timer < 0)
                {
                        progbug("Mpoload - Bad timer", ch);
                        return;
                }
        }

        if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL)
        {
                progbug("Mpoload - Bad vnum arg", ch);
                return;
        }

        obj = create_object(pObjIndex, level);
        obj->timer = timer;
        if (CanWear(obj, ItemTake))
                obj_to_char(obj, ch);
        else
                obj_to_room(obj, ch->in_room);

        return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MUDprogram
   otherwise ugly stuff will happen */

CMDF do_mppurge(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        ObjData *obj;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                /*
                 * 'purge' 
                 */
                CharData *vnext;

                for (victim = ch->in_room->first_person; victim;
                     victim = vnext)
                {
                        vnext = victim->next_in_room;
                        if (IsNpc(victim) && victim != ch)
                                extract_char(victim, TRUE);
                }
                while (ch->in_room->first_content)
                        extract_obj(ch->in_room->first_content);

                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                if ((obj = get_obj_here(ch, arg)) != NULL)
                        extract_obj(obj);
                else
                        progbug("Mppurge - Bad argument", ch);
                return;
        }

        if (!IsNpc(victim))
        {
                progbug("Mppurge - Trying to purge a PC", ch);
                return;
        }

        if (IsNpc(victim) && victim->pIndexData->vnum == 3)
        {
                progbug("Mppurge: trying to purge supermob", ch);
                return;
        }

        extract_char(victim, TRUE);
        return;
}


/* Allow mobiles to go wizinvis with programs -- SB */

CMDF do_mpinvis(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        sh_int    level;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);
        if (arg && arg[0] != '\0')
        {
                if (!is_number(arg))
                {
                        progbug("Mpinvis - Non numeric argument ", ch);
                        return;
                }
                level = atoi(arg);
                if (level < 2 || level > 51)
                {
                        progbug("MPinvis - Invalid level ", ch);
                        return;
                }

                ch->mobinvis = level;
                ch_printf(ch, "Mobinvis level set to %d.\n\r", level);
                return;
        }

        if (ch->mobinvis < 2)
                ch->mobinvis = ch->top_level;

        if (IsSet(ch->act, ActMobinvis))
        {
                RemoveBit(ch->act, ActMobinvis);
                act(AtImmort, "$n slowly fades into existence.", ch, NULL,
                    NULL, ToRoom);
                send_to_char("You slowly fade back into existence.\n\r", ch);
        }
        else
        {
                SetBit(ch->act, ActMobinvis);
                act(AtImmort, "$n slowly fades into thin air.", ch, NULL,
                    NULL, ToRoom);
                send_to_char("You slowly vanish into thin air.\n\r", ch);
        }
        return;
}

/* lets the mobile goto any location it wishes that is not private */

CMDF do_mpgoto(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        RoomIndexData *location;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                progbug("Mpgoto - No argument", ch);
                return;
        }

        if ((location = find_location(ch, arg)) == NULL)
        {
                progbug("Mpgoto - No such location", ch);
                return;
        }

        if (ch->fighting)
                stop_fighting(ch, TRUE);

        char_from_room(ch);
        char_to_room(ch, location);

        return;
}

/* lets the mobile do a command at another location. Very useful */

CMDF do_mpat(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        RoomIndexData *location;
        RoomIndexData *original;
        CharData *wch;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                progbug("Mpat - Bad argument", ch);
                return;
        }

        if ((location = find_location(ch, arg)) == NULL)
        {
                progbug("Mpat - No such location", ch);
                return;
        }

        original = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, location);
        interpret(ch, argument);

        /*
         * See if 'ch' still exists before continuing!
         * Handles 'at XXXX quit' case.
         */
        for (wch = first_char; wch; wch = wch->next)
                if (wch == ch)
                {
                        char_from_room(ch);
                        char_to_room(ch, original);
                        break;
                }

        return;
}

/* allow a mobile to advance a player's level... very dangerous */
CMDF do_mpadvance(CharData * ch, char *argument)
{
        argument = NULL;
        if (IsAffected(ch, AffCharm))
                return;

        return;
}



/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */

CMDF do_mptransfer(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      buf[MaxStringLength];
        RoomIndexData *location;
        CharData *victim;
        CharData *nextinroom;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                progbug("Mptransfer - Bad syntax", ch);
                return;
        }

        /*
         * Put in the variable nextinroom to make this work right. -Narn 
         */
        if (!str_cmp(arg1, "all"))
        {
                for (victim = ch->in_room->first_person; victim;
                     victim = nextinroom)
                {
                        nextinroom = victim->next_in_room;
                        if (victim != ch
                            && !NotAuthed(victim) && can_see(ch, victim))
                        {
                                snprintf(buf, MSL, "%s %s", victim->name,
                                         arg2);
                                do_mptransfer(ch, buf);
                        }
                }
                return;
        }

        /*
         * Thanks to Grodyn for the optional location parameter.
         */
        if (arg2[0] == '\0')
        {
                location = ch->in_room;
        }
        else
        {
                if ((location = find_location(ch, arg2)) == NULL)
                {
                        progbug("Mptransfer - No such location", ch);
                        return;
                }

                if (room_is_private(ch, location))
                {
                        progbug("Mptransfer - Private room", ch);
                        return;
                }
        }

        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                progbug("Mptransfer - No such person", ch);
                return;
        }

        if (!victim->in_room)
        {
                progbug("Mptransfer - Victim in Limbo", ch);
                return;
        }

        if (NotAuthed(victim) && location->area != victim->in_room->area)
        {
                progbug("Mptransfer - transferring unauthorized player", ch);
                return;
        }


/* If victim not in area's level range, do not transfer */
        if (!in_hard_range(victim, location->area)
            && !IsSet(location->RoomFlags, RoomPrototype))
                return;

        if (victim->fighting)
                stop_fighting(victim, TRUE);

        char_from_room(victim);
        char_to_room(victim, location);

        return;
}

/* lets the mobile Force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

CMDF do_mpforce(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CMDType  *command;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || ch->desc)
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);
        command = find_command(arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                progbug("Mpforce - Bad syntax", ch);
                return;
        }

        if( !str_cmp( arg, "all" ) )
        {
            CharData *vch, *vch_next;

            for( vch = ch->in_room->first_person; vch; vch = vch_next )
            {
                vch_next = vch->next_in_room;
                if( get_trust( vch ) < get_trust( ch ) && can_see( ch, vch ) )
                    interpret( vch, argument );
            }
        }
        else
        {
                CharData *victim;

                if ((victim = get_char_room_mp(ch, arg)) == NULL)
                {
                        progbug("Mpforce - No such victim", ch);
                        return;
                }

                if (victim == ch)
                {
                        progbug("Mpforce - Forcing oneself", ch);
                        return;
                }

                if (!IsNpc(victim) && IsImmortal(victim))
                {
                        progbug("Mpforce - Attempting to Force immortal", ch);
                        return;
                }


                interpret(victim, argument);
        }

        return;
}



/*
 *  Haus' toys follow:
 */


/*
 * syntax:  mppractice victim spell_name max%
 *
 */
CMDF do_mp_practice(CharData * ch, char *argument)
{
        argument = NULL;
        if (IsAffected(ch, AffCharm))
                return;
}

/*
 * syntax: mpslay (character)
 */
CMDF do_mp_slay(CharData * ch, char *argument)
{
        argument = NULL;
        if (IsAffected(ch, AffCharm))
                return;

        return;
}

/*
 * syntax: mpdamage (character) (#hps)
 */
CMDF do_mp_damage(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        CharData *victim;
        int       dam;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || (ch->desc && get_trust(ch) < LevelImmortal))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("mpdamage whom?\n\r", ch);
                progbug("Mpdamage: invalid argument1", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                send_to_char("mpdamage inflict how many hps?\n\r", ch);
                progbug("Mpdamage: invalid argument2", ch);
                return;
        }

        if ((victim = get_char_room_mp(ch, arg1)) == NULL)
        {
                send_to_char("Victim must be in room.\n\r", ch);
                progbug("Mpdamage: victim not in room", ch);
                return;
        }

        dam = atoi(arg2);

        if ((dam < 0) || (dam > 32000))
        {
                send_to_char("Mpdamage how much?\n\r", ch);
                progbug("Mpdamage: invalid (nonexistent?) argument", ch);
                return;
        }

        /*
         * this is kinda begging for trouble        
         */
        /*
         * Note from Thoric to whoever put this in...
         * Wouldn't it be better to call damage(ch, ch, dam, dt)?
         * I hate redundant code
         */
        if (simple_damage(ch, victim, dam, TypeUndefined) == rVICT_DIED)
        {
                stop_fighting(ch, FALSE);
                stop_hating(ch);
                stop_fearing(ch);
                stop_hunting(ch);
        }

        return;
}


/*
 * syntax: mprestore (character) (#hps)                Gorog
 */
CMDF do_mp_restore(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        CharData *victim;
        int       hp;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || (ch->desc && get_trust(ch) < LevelImmortal))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("mprestore whom?\n\r", ch);
                progbug("Mprestore: invalid argument1", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                send_to_char("mprestore how many hps?\n\r", ch);
                progbug("Mprestore: invalid argument2", ch);
                return;
        }

        if ((victim = get_char_room_mp(ch, arg1)) == NULL)
        {
                send_to_char("Victim must be in room.\n\r", ch);
                progbug("Mprestore: victim not in room", ch);
                return;
        }

        hp = atoi(arg2);

        if ((hp < 0) || (hp > 32000))
        {
                send_to_char("Mprestore how much?\n\r", ch);
                progbug("Mprestore: invalid (nonexistent?) argument", ch);
                return;
        }
        hp += victim->hit;
        victim->hit = (hp > 32000 || hp < 0 || hp > victim->max_hit) ?
                victim->max_hit : hp;
}

CMDF do_mpgain(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        CharData *victim;
        long      experience;
        int       ability;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) && !IsImmAdmin(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: mpgain <who> <class> <amt>\n\r", ch);
                progbug("Mpgain: invalid argument1", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                send_to_char("Syntax: mpgain <who> <class> <amt>\n\r", ch);
                progbug("Mpgain: invalid argument2", ch);
                return;
        }

        if (arg3[0] == '\0')
        {
                send_to_char("Syntax: mpgain <who> <class> <amt>\n\r", ch);
                progbug("Mpgain: invalid argument3", ch);
                return;
        }

        if ((victim = get_char_room_mp(ch, arg1)) == NULL)
        {
                send_to_char("Victim must be in room.\n\r", ch);
                progbug("Mpgain: victim not in room", ch);
                return;
        }

        if (is_number(arg2))
        {
                ability = atoi(arg2);
        }
        else
        {
                int       i = 0;

                ability = 0;
                for (i = 0; i < MaxAbility; i++)
                {
                        if (!str_cmp(arg2, ability_name[i]))
                        {
                                ability = i;
                                break;
                        }
                }
        }
        experience = atoi(arg3);

        if (ability < 0 || ability >= MaxAbility)
        {
                send_to_char("Mpgain which ability?\n\r", ch);
                progbug("Mpgain: ability out of range", ch);
                return;
        }

        if ((experience < 1))
        {
                send_to_char("Mpgain how much?\n\r", ch);
                progbug("Mpgain: experience out of range", ch);
                return;
        }

        experience =
                URANGE(1, experience,
                       (exp_level(victim->skill_level[ability] + 1) -
                        exp_level(victim->skill_level[ability])));

        ch_printf(victim, "You gain %ld %s experience.\n\r", experience,
                  ability_name[ability]);
        gain_exp(victim, experience, ability);
        return;
}

/*
 * Syntax mp_open_passage x y z
 *
 * opens a 1-way passage from room x to room y in direction z
 *
 *  won't mess with existing exits
 */
CMDF do_mp_open_passage(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        RoomIndexData *targetRoom, *fromRoom;
        int       targetRoomVnum, fromRoomVnum, exit_num;
        ExitData *pexit;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || (ch->desc && get_trust(ch) < LevelImmortal))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
        {
                progbug("MpOpenPassage - Bad syntax", ch);
                return;
        }

        if (!is_number(arg1))
        {
                progbug("MpOpenPassage - Bad syntax", ch);
                return;
        }

        fromRoomVnum = atoi(arg1);
        if ((fromRoom = get_room_index(fromRoomVnum)) == NULL)
        {
                progbug("MpOpenPassage - Bad syntax", ch);
                return;
        }

        if (!is_number(arg2))
        {
                progbug("MpOpenPassage - Bad syntax", ch);
                return;
        }

        targetRoomVnum = atoi(arg2);
        if ((targetRoom = get_room_index(targetRoomVnum)) == NULL)
        {
                progbug("MpOpenPassage - Bad syntax", ch);
                return;
        }

        if (!is_number(arg3))
        {
                progbug("MpOpenPassage - Bad syntax", ch);
                return;
        }

        exit_num = atoi(arg3);
        if ((exit_num < 0) || (exit_num > MaxDir))
        {
                progbug("MpOpenPassage - Bad syntax", ch);
                return;
        }

        if ((pexit = get_exit(fromRoom, exit_num)) != NULL)
        {
                if (!IsSet(pexit->exit_info, ExPassage))
                        return;
                progbug("MpOpenPassage - Exit exists", ch);
                return;
        }

        pexit = make_exit(fromRoom, targetRoom, exit_num);
        pexit->keyword = STRALLOC("");
        pexit->description = STRALLOC("");
        pexit->key = -1;
        pexit->exit_info = ExPassage;

        act(AtPlain, "A passage opens!", ch, NULL, NULL, ToChar);
        act(AtPlain, "A passage opens!", ch, NULL, NULL, ToRoom);

        return;
}

/*
 * Syntax mp_close_passage x y 
 *
 * closes a passage in room x leading in direction y
 *
 * the exit must have ExPassage set
 */
CMDF do_mp_close_passage(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        RoomIndexData *fromRoom;
        int       fromRoomVnum, exit_num;
        ExitData *pexit;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || (ch->desc && get_trust(ch) < LevelImmortal))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg2[0] == '\0')
        {
                progbug("MpClosePassage - Bad syntax", ch);
                return;
        }

        if (!is_number(arg1))
        {
                progbug("MpClosePassage - Bad syntax", ch);
                return;
        }

        fromRoomVnum = atoi(arg1);
        if ((fromRoom = get_room_index(fromRoomVnum)) == NULL)
        {
                progbug("MpClosePassage - Bad syntax", ch);
                return;
        }

        if (!is_number(arg2))
        {
                progbug("MpClosePassage - Bad syntax", ch);
                return;
        }

        exit_num = atoi(arg2);
        if ((exit_num < 0) || (exit_num > MaxDir))
        {
                progbug("MpClosePassage - Bad syntax", ch);
                return;
        }

        if ((pexit = get_exit(fromRoom, exit_num)) == NULL)
        {
                return; /* already closed, ignore...  so rand_progs */
                /*
                 * can close without spam 
                 */
        }

        if (!IsSet(pexit->exit_info, ExPassage))
        {
                progbug("MpClosePassage - Exit not a passage", ch);
                return;
        }

        extract_exit(fromRoom, pexit);

        /*
         * act( AtPlain, "A passage closes!", ch, NULL, NULL, ToChar ); 
         */
        /*
         * act( AtPlain, "A passage closes!", ch, NULL, NULL, ToRoom ); 
         */

        return;
}



/*
 * Does nothing.  Used for scripts.
 */
CMDF do_mpnothing(CharData * ch, char *argument)
{
        argument = NULL;
        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || (ch->desc && get_trust(ch) < LevelImmortal))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }
        return;
}

CMDF do_mpsector(CharData * ch, char *argument)
{
        RoomIndexData *wroom;
        char      arg1[MaxInputLength];

        argument = one_argument(argument, arg1);
        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || (ch->desc && get_trust(ch) < LevelImmortal))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }
        if (arg1[0] == '\0')
        {
                progbug("Mpsector - Bad syntax", ch);
                return;
        }

        if (!is_number(arg1))
        {
                progbug("Mpsector - Bad syntax", ch);
                return;
        }

        if ((wroom = get_room_index(ch->in_room->vnum)) == NULL)
        {
                progbug("Mpsector - Bad room", ch);
                return;
        }

        wroom->sector_type = atoi(arg1);
        if (wroom->sector_type < 0 || wroom->sector_type >= SectMax)
        {
                wroom->sector_type = 1;
                progbug("Mpsector - Not a Valid sector", ch);
                return;
        }
        fold_area(wroom->area, wroom->area->filename, FALSE, TRUE);
        return;
}

/*
 *   Sends a message to sleeping character.  Should be fun
 *    with room sleep_progs
 *
 */
CMDF do_mpdream(CharData * ch, char *argument)
{
        char      arg1[MaxStringLength];
        CharData *vict;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch) || (ch->desc && get_trust(ch) < LevelImmortal))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);

        if ((vict = get_char_world(ch, arg1)) == NULL)
        {
                progbug("Mpdream: No such character", ch);
                return;
        }

        if (vict->position <= PosSleeping)
        {
                send_to_char(argument, vict);
                send_to_char("\n\r", vict);
        }
        return;
}

CMDF do_mpapply(CharData * ch, char *argument)
{
        CharData *victim;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                progbug("Mpapply - bad syntax", ch);
                return;
        }

        if ((victim = get_char_room_mp(ch, argument)) == NULL)
        {
                progbug("Mpapply - no such player in room.", ch);
                return;
        }

        if (!victim->desc)
        {
                send_to_char("Not on linkdeads.\n\r", ch);
                return;
        }

        if (!NotAuthed(victim))
                return;

        if (victim->PCData->AuthState >= 1)
                return;

        snprintf(log_buf, MSL, "%s@%s new %s applying for authorization...",
                 victim->name, victim->desc->host, victim->race->name());
        log_string(log_buf);
        victim->PCData->AuthState = 1;
        return;
}

CMDF do_mpapplyb(CharData * ch, char *argument)
{
        CharData *victim;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                progbug("Mpapplyb - bad syntax", ch);
                return;
        }

        if ((victim = get_char_room_mp(ch, argument)) == NULL)
        {
                progbug("Mpapplyb - no such player in room.", ch);
                return;
        }

        if (!victim->desc)
        {
                send_to_char("Not on linkdeads.\n\r", ch);
                return;
        }

        if (!NotAuthed(victim))
                return;

        if (get_timer(victim, TimerApplied) >= 1)
                return;

        switch (victim->PCData->AuthState)
        {
        case 0:
        case 1:
        default:
                send_to_char("You attempt to regain the gods' attention.\n\r",
                             victim);
                snprintf(log_buf, MSL,
                         "%s@%s new %s applying for authorization...",
                         victim->name, victim->desc->host,
                         victim->race->name());
                log_string(log_buf);
                add_timer(victim, TimerApplied, 10, NULL, 0);
                victim->PCData->AuthState = 1;
                break;

        case 2:
                send_to_char
                        ("Your name has been deemed unsuitable by the gods.  Please choose a more apropriate name with the 'name' command.\n\r",
                         victim);
                add_timer(victim, TimerApplied, 10, NULL, 0);
                break;

        case 3:
                send_to_char("The gods permit you to enter the SWR.\n\r",
                             victim);
                RemoveBit(victim->PCData->flags, PcflagUnauthed);
                if (victim->fighting)
                        stop_fighting(victim, TRUE);
                char_from_room(victim);
                char_to_room(victim, get_room_index(RoomVnumSchool));
                act(AtWhite,
                    "$n enters this world from within a column of blinding light!",
                    victim, NULL, NULL, ToRoom);
                do_look(victim, "auto");
                break;
        }

        return;
}

/*
 * Deposit some gold into the current area's economy		-Thoric
 */
CMDF do_mp_deposit(CharData * ch, char *argument)
{
        char      arg[MaxStringLength];
        int       gold;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                progbug("Mpdeposit - bad syntax", ch);
                return;
        }
        gold = atoi(arg);
        if (gold <= ch->gold && ch->in_room)
        {
                ch->gold -= gold;
                boost_economy(ch->in_room->area, gold);
        }
}


/*
 * Withdraw some gold from the current area's economy		-Thoric
 */
CMDF do_mp_withdraw(CharData * ch, char *argument)
{
        char      arg[MaxStringLength];
        int       gold;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                progbug("Mpwithdraw - bad syntax", ch);
                return;
        }
        gold = atoi(arg);
        if (ch->gold < 1000000000 && gold < 1000000000 && ch->in_room
            && economy_has(ch->in_room->area, gold))
        {
                ch->gold += gold;
                lower_economy(ch->in_room->area, gold);
        }
}


CMDF do_mppkset(CharData * ch, char *argument)
{
        argument = NULL;
        if (IsAffected(ch, AffCharm))
                return;

        send_to_char
                ("mppkset has been zapped into the realm of useless old code.\n\r",
                 ch);
        return;

}

CMDF do_mprat(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        RoomIndexData *location;
        RoomIndexData *original;
        int       Start, End, vnum;

        if (IsAffected(ch, AffCharm))
                return;

        if (!IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);


        if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
        {
                progbug("mprat: bad syntax \n\r", ch);
                return;
        }

        Start = atoi(arg1);
        End = atoi(arg2);

        if (Start < 1 || End < Start || Start > End || Start == End
            || End > MaxVnums)
        {
                progbug("mprat: Invalid range.\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "quit"))
        {
                return;
        }

        original = ch->in_room;
        for (vnum = Start; vnum <= End; vnum++)
        {
                if ((location = get_room_index(vnum)) == NULL)
                        continue;
                char_from_room(ch);
                char_to_room(ch, location);
                interpret(ch, argument);
        }

        char_from_room(ch);
        char_to_room(ch, original);
        return;
}

/*
 * Inflict damage from a mudprogram
 *
 *  note: should be careful about using victim afterwards
 */
ch_ret simple_damage(CharData * ch, CharData * victim, int dam, int dt)
{
        sh_int    dameq;
        bool      npcvict;
        ObjData *damobj;
        ch_ret    retcode;


        retcode = rNONE;

        if (!ch)
        {
                bug("Damage: null ch!", 0);
                return rERROR;
        }
        if (!victim)
        {
                progbug("Damage: null victim!", ch);
                return rVICT_DIED;
        }

        if (victim->position == PosDead)
        {
                return rVICT_DIED;
        }

        npcvict = IsNpc(victim);

        if (dam)
        {
                if (IsFire(dt))
                        dam = ris_damage(victim, dam, RisFire);
                else if (IsCold(dt))
                        dam = ris_damage(victim, dam, RisCold);
                else if (IsAcid(dt))
                        dam = ris_damage(victim, dam, RisAcid);
                else if (IsElectricity(dt))
                        dam = ris_damage(victim, dam, RisElectricity);
                else if (IsEnergy(dt))
                        dam = ris_damage(victim, dam, RisEnergy);
                else if (dt == gsn_poison)
                        dam = ris_damage(victim, dam, RisPoison);
                else if (dt == (TypeHit + 7) || dt == (TypeHit + 8))
                        dam = ris_damage(victim, dam, RisBlunt);
                else if (dt == (TypeHit + 2) || dt == (TypeHit + 11))
                        dam = ris_damage(victim, dam, RisPierce);
                else if (dt == (TypeHit + 1) || dt == (TypeHit + 3))
                        dam = ris_damage(victim, dam, RisSlash);
                if (dam < 0)
                        dam = 0;
        }

        if (victim != ch)
        {
                /*
                 * Damage modifiers.
                 */
                if (IsAffected(victim, AffSanctuary))
                        dam /= 2;

                if (IsAffected(victim, AffProtect) && IsEvil(ch))
                        dam -= (int) (dam / 4);

                if (dam < 0)
                        dam = 0;

                /*
                 * dam_message( ch, victim, dam, dt ); 
                 */
        }

        /*
         * Check for EQ damage.... ;)
         */

        if (dam > 10)
        {
                /*
                 * get a random body eq part 
                 */
                dameq = number_range(WearLight, WearEyes);
                damobj = get_eq_char(victim, dameq);
                if (damobj)
                {
                        if (dam > get_obj_resistance(damobj))
                        {
                                set_cur_obj(damobj);
                                damage_obj(damobj);
                        }
                }
        }

        /*
         * Hurt the victim.
         * Inform the victim of his new state.
         */
        victim->hit -= dam;
        if (!IsNpc(victim)
            && get_trust(victim) >= LevelImmortal && victim->hit < 1)
                victim->hit = 1;

        if (!npcvict
            && get_trust(victim) >= LevelImmortal
            && get_trust(ch) >= LevelImmortal && victim->hit < 1)
                victim->hit = 1;
        update_pos(victim);

        switch (victim->position)
        {
        case PosMortal:
                act(AtDying,
                    "$n is mortally wounded, and will die soon, if not aided.",
                    victim, NULL, NULL, ToRoom);
                act(AtDanger,
                    "You are mortally wounded, and will die soon, if not aided.",
                    victim, NULL, NULL, ToChar);
                break;

        case PosIncap:
                act(AtDying,
                    "$n is incapacitated and will slowly die, if not aided.",
                    victim, NULL, NULL, ToRoom);
                act(AtDanger,
                    "You are incapacitated and will slowly die, if not aided.",
                    victim, NULL, NULL, ToChar);
                break;

        case PosStunned:
                if (!IsAffected(victim, AffParalysis))
                {
                        act(AtAction,
                            "$n is stunned, but will probably recover.",
                            victim, NULL, NULL, ToRoom);
                        act(AtHurt,
                            "You are stunned, but will probably recover.",
                            victim, NULL, NULL, ToChar);
                }
                break;

        case PosDead:
                act(AtDead, "$n is DEAD!!", victim, 0, 0, ToRoom);
                act(AtDead, "You have been KILLED!!\n\r", victim, 0, 0,
                    ToChar);
                break;

        default:
                if (dam > victim->max_hit / 4)
                        act(AtHurt, "That really did HURT!", victim, 0, 0,
                            ToChar);
                if (victim->hit < victim->max_hit / 4)
                        act(AtDanger,
                            "You wish that your wounds would stop BLEEDING so much!",
                            victim, 0, 0, ToChar);
                break;
        }

        /*
         * Payoff for killing things.
         */
        if (victim->position == PosDead)
        {
                if (!npcvict)
                {
                        snprintf(log_buf, MSL, "%s killed by %s at %d",
                                 victim->name,
                                 (IsNpc(ch) ? ch->short_descr : ch->name),
                                 victim->in_room->vnum);
                        log_string(log_buf);


                }
                raw_kill(ch, victim);
                victim = NULL;

                return rVICT_DIED;
        }

        if (victim == ch)
                return rNONE;

        /*
         * Take care of link dead people.
         */
        if (!npcvict && !victim->desc)
        {
                if (number_range(0, victim->wait) == 0)
                {
                        do_recall(victim, "");
                        return rNONE;
                }
        }

        /*
         * Wimp out?
         */
        if (npcvict && dam > 0)
        {
                if ((IsSet(victim->act, ActWimpy) && number_bits(1) == 0
                     && victim->hit < victim->max_hit / 2)
                    || (IsAffected(victim, AffCharm) && victim->master
                        && victim->master->in_room != victim->in_room))
                {
                        start_fearing(victim, ch);
                        stop_hunting(victim);
                        do_flee(victim, "");
                }
        }

        if (!npcvict
            && victim->hit > 0
            && victim->hit <= victim->wimpy && victim->wait == 0)
                do_flee(victim, "");
        else if (!npcvict && IsSet(victim->act, PlrFlee))
                do_flee(victim, "");

        tail_chain();
        return rNONE;
}

CharData *get_char_room_mp(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *rch;
        int       number, count, vnum;

        number = number_argument(argument, arg);
        if (!str_cmp(arg, "self"))
                return ch;

        if (get_trust(ch) >= LevelSavior && is_number(arg))
                vnum = atoi(arg);
        else
                vnum = -1;

        count = 0;

        for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
                if ((nifty_is_name(arg, rch->name)
                     || (IsNpc(rch) && vnum == rch->pIndexData->vnum)))
                {
                        if (number == 0 && !IsNpc(rch))
                                return rch;
                        else if (++count == number)
                                return rch;
                }

        if (vnum != -1)
                return NULL;
        count = 0;
        for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
        {
                if (!nifty_is_name_prefix(arg, rch->name))
                        continue;
                if (number == 0 && !IsNpc(rch))
                        return rch;
                else if (++count == number)
                        return rch;
        }

        return NULL;
}
