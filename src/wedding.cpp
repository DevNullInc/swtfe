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
 * Marriage and wedding system for player relationships, ceremonies, and social events.  *
 ****************************************************************************************/
/***************************************************************************
*	MARRY.C written by Ryouga for Vilaross Mud (baby.indstate.edu 4000)	   *
*	Please leave this and all other credit include in this package.        *
*	Email questions/comments to ryouga@jessi.indstate.edu		           *
****************************************************************************
*	Modified By Gavin(ur_gavin@hotmail.com) for Unkown Regions             *
***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"


CMDF do_propose(CharData * ch, char *argument)
{
        CharData *victim;

        if (IsNpc(ch))
                return;

        if (IsSet(ch->pcdata->flags, PcflagMarried))
        {
                send_to_char("But you are already married!\n\r", ch);
                return;
        }

        if (ch->pcdata->spouse && ch->pcdata->spouse[0] != '\0')
        {
                send_to_char("But you are already engaged!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Who do you wish to propose marriage to?\n\r",
                             ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL && victim != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IsNpc(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (victim->pcdata->spouse && victim->pcdata->spouse[0] != '\0')
        {
                if (IsSet(victim->pcdata->flags, PcflagMarried))
                        send_to_char("But they are already married!\n\r", ch);
                else
                        send_to_char("But they are already engaged!\n\r", ch);
                return;
        }

/*	if ( get_age(ch) < race_table[ch->race]->consent )
	{
		send_to_char("You must mature a little more before you start thinking about that.\n\r",ch);
		return;
	}
	if ( get_age(victim) < race_table[victim->race]->consent )
	{
		send_to_char("They must mature a little more before they should start thinking about that.\n\r",ch);
		return;
	}*/

/*	if ( ch->sex == victim->sex && ch->sex != SexNeutral)
	{
		send_to_char("Not on this mud bub!\n\r",ch);
		return;
	}*/
        if (ch == victim)
        {
                send_to_char("Not on this mud bub!\n\r", ch);
                return;
        }

        ch->pcdata->propose = victim;
        victim->pcdata->propose = ch;
        act(AtWhite, "You propose marriage to $M.", ch, NULL, victim,
            ToChar);
        act(AtWhite, "$n gets down on one knee and proposes to $N.", ch,
            NULL, victim, ToNotvict);
        act(AtWhite, "$n asks you quietly 'Will you marry me?'", ch, NULL,
            victim, ToVict);
        return;
}

CMDF do_accept(CharData * ch, char *argument)
{
        CharData *victim;

        argument = NULL;

        if (IsNpc(ch))
                return;

        if ((victim = ch->pcdata->propose) == NULL)
        {
                send_to_char("Nobody proposed to you.", ch);
                return;
        }

        if (victim->pcdata->propose != ch)
        {
                send_to_char
                        ("They seemed to have proposed to someone else in the mean time.",
                         ch);
                return;
        }

        if (victim == NULL || victim->in_room != ch->in_room)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IsSet(ch->pcdata->flags, PcflagMarried))
        {
                send_to_char("You are already married!\n\r", ch);
                return;
        }

        if (IsSet(victim->pcdata->flags, PcflagMarried))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        victim->pcdata->propose = NULL;
        ch->pcdata->propose = NULL;
        RemoveBit(ch->pcdata->flags, PcflagMarried);
        RemoveBit(victim->pcdata->flags, PcflagMarried);
        if (victim->pcdata->spouse)
                STRFREE(victim->pcdata->spouse);
        if (ch->pcdata->spouse)
                STRFREE(ch->pcdata->spouse);
        victim->pcdata->spouse = STRALLOC(ch->name);
        ch->pcdata->spouse = STRALLOC(victim->name);
        act(AtWhite, "You accept $S offer of marriage.", ch, NULL, victim,
            ToChar);
        act(AtWhite, "$n accepts $N's offer of marriage.", ch, NULL, victim,
            ToNotvict);
        act(AtWhite, "$n accepts your offer of marriage.", ch, NULL, victim,
            ToVict);
        act(AtWhite, "$n and $N are now engaged!", ch, NULL, victim,
            ToNotvict);
        save_char_obj(victim);
        save_char_obj(ch);
        return;
}


CMDF do_refuse(CharData * ch, char *argument)
{
        CharData *victim;

        argument = NULL;

        if (IsNpc(ch))
                return;

        if ((victim = ch->pcdata->propose) == NULL)
        {
                send_to_char("Nobody proposed to you.", ch);
                return;
        }

        if (victim->pcdata->propose != ch)
        {
                send_to_char
                        ("They seemed to have proposed to someone else in the mean time.",
                         ch);
                return;
        }

        if (victim == NULL || victim->in_room != ch->in_room)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IsSet(ch->pcdata->flags, PcflagMarried))
        {
                send_to_char("You are already married!\n\r", ch);
                return;
        }

        if (IsSet(victim->pcdata->flags, PcflagMarried))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        victim->pcdata->propose = NULL;
        ch->pcdata->propose = NULL;

        act(AtWhite, "$N refused $n's offer of engagement!", ch, NULL,
            victim, ToNotvict);
        act(AtWhite, "$N refuses your offer of engagement!", ch, NULL,
            victim, ToChar);
        act(AtWhite, "You refuse $N's offer of engagement!", ch, NULL,
            victim, ToVict);
}

CMDF do_marry(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim1;
        CharData *victim2;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: &Gmarry &C<&cperson&C> &C<&cperson&C>\n\r",
                         ch);
                return;
        }

        if ((victim1 = get_char_room(ch, arg)) == NULL && victim1 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if ((victim2 = get_char_room(ch, argument)) == NULL && victim2 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IsNpc(victim1) || IsNpc(victim2))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (IsSet(victim1->pcdata->flags, PcflagMarried) ||
            IsSet(victim2->pcdata->flags, PcflagMarried))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        if (!str_cmp(victim1->name, victim2->pcdata->spouse) &&
            !str_cmp(victim2->name, victim1->pcdata->spouse))
        {
                SetBit(victim1->pcdata->flags, PcflagMarried);
                SetBit(victim2->pcdata->flags, PcflagMarried);
                save_char_obj(victim1);
                save_char_obj(victim2);
                act(AtWhite, "You are now married to $N! Congrats!", victim1,
                    NULL, victim2, ToVict);
                act(AtWhite, "You are now married to $n! Congrats!", victim1,
                    NULL, victim2, ToChar);
                act(AtWhite, "$n and $N are now married!", victim1, NULL,
                    victim2, ToNotvict);
                return;
        }

        send_to_char("You can't do that to them! They are not ready.\n\r",
                     ch);
        return;
}

CMDF do_divorce(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim1;
        CharData *victim2;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: &Gdivorce &C<&cperson&C> &C<&cperson&C>\n\r",
                         ch);
                return;
        }

        if ((victim1 = get_char_room(ch, arg)) == NULL && victim1 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if ((victim2 = get_char_room(ch, argument)) == NULL && victim2 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IsNpc(victim1) || IsNpc(victim2))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }


        if (!str_cmp(victim1->name, victim2->pcdata->spouse) &&
            !str_cmp(victim2->name, victim1->pcdata->spouse))
        {
                if (!IsSet(victim1->pcdata->flags, PcflagMarried) ||
                    !IsSet(victim2->pcdata->flags, PcflagMarried))
                {
                        send_to_char("They are not married!\n\r", ch);
                        return;
                }

                RemoveBit(victim1->pcdata->flags, PcflagMarried);
                RemoveBit(victim2->pcdata->flags, PcflagMarried);
                STRFREE(victim1->pcdata->spouse);
                STRFREE(victim2->pcdata->spouse);

                victim1->pcdata->spouse = STRALLOC("");
                victim2->pcdata->spouse = STRALLOC("");

                save_char_obj(victim1);
                save_char_obj(victim2);
                act(AtWhite, "You are now divorced from $N!", victim1, NULL,
                    victim2, ToVict);
                act(AtWhite, "You are now divorced from $n!", victim1, NULL,
                    victim2, ToChar);
                act(AtWhite, "$n and $N are now divorced!", victim1, NULL,
                    victim2, ToNotvict);
                return;
        }

        send_to_char("They are not married!\n\r", ch);
        return;
}
