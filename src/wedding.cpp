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

        if (IsSet(ch->PCData->flags, PcflagMarried))
        {
                send_to_char("But you are already married!\n\r", ch);
                return;
        }

        if (ch->PCData->spouse && ch->PCData->spouse[0] != '\0')
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

        if (victim->PCData->spouse && victim->PCData->spouse[0] != '\0')
        {
                if (IsSet(victim->PCData->flags, PcflagMarried))
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

        ch->PCData->propose = victim;
        victim->PCData->propose = ch;
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

        if ((victim = ch->PCData->propose) == NULL)
        {
                send_to_char("Nobody proposed to you.", ch);
                return;
        }

        if (victim->PCData->propose != ch)
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

        if (IsSet(ch->PCData->flags, PcflagMarried))
        {
                send_to_char("You are already married!\n\r", ch);
                return;
        }

        if (IsSet(victim->PCData->flags, PcflagMarried))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        victim->PCData->propose = NULL;
        ch->PCData->propose = NULL;
        RemoveBit(ch->PCData->flags, PcflagMarried);
        RemoveBit(victim->PCData->flags, PcflagMarried);
        if (victim->PCData->spouse)
                STRFREE(victim->PCData->spouse);
        if (ch->PCData->spouse)
                STRFREE(ch->PCData->spouse);
        victim->PCData->spouse = STRALLOC(ch->name);
        ch->PCData->spouse = STRALLOC(victim->name);
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

        if ((victim = ch->PCData->propose) == NULL)
        {
                send_to_char("Nobody proposed to you.", ch);
                return;
        }

        if (victim->PCData->propose != ch)
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

        if (IsSet(ch->PCData->flags, PcflagMarried))
        {
                send_to_char("You are already married!\n\r", ch);
                return;
        }

        if (IsSet(victim->PCData->flags, PcflagMarried))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        victim->PCData->propose = NULL;
        ch->PCData->propose = NULL;

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

        if (IsSet(victim1->PCData->flags, PcflagMarried) ||
            IsSet(victim2->PCData->flags, PcflagMarried))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        if (!str_cmp(victim1->name, victim2->PCData->spouse) &&
            !str_cmp(victim2->name, victim1->PCData->spouse))
        {
                SetBit(victim1->PCData->flags, PcflagMarried);
                SetBit(victim2->PCData->flags, PcflagMarried);
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


        if (!str_cmp(victim1->name, victim2->PCData->spouse) &&
            !str_cmp(victim2->name, victim1->PCData->spouse))
        {
                if (!IsSet(victim1->PCData->flags, PcflagMarried) ||
                    !IsSet(victim2->PCData->flags, PcflagMarried))
                {
                        send_to_char("They are not married!\n\r", ch);
                        return;
                }

                RemoveBit(victim1->PCData->flags, PcflagMarried);
                RemoveBit(victim2->PCData->flags, PcflagMarried);
                STRFREE(victim1->PCData->spouse);
                STRFREE(victim2->PCData->spouse);

                victim1->PCData->spouse = STRALLOC("");
                victim2->PCData->spouse = STRALLOC("");

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
