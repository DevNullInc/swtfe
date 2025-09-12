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
 * This module is for our very own occupation class                                      *
 *****************************************************************************************
 * Player occupation and job system for role-playing, progression, & character dev.      *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"

CMDF do_cook(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomCafe))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char("You must be in a cafe to cook.\n\r",
                                     ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while cooking.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_cook]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou grab your ingredients and start to cook.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n takes $s ingredients and starts to cook something.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 14, do_cook, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to make a delicious meal.\n\r",
                         ch);
                learn_from_failure(ch, gsn_cook);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char("&RYou fail to cook your meal.\n\r", ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_beg])
        {
                send_to_char
                        ("You tried to cook a delicious meal, but you burn it!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to cook something, but burnt it!.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_beg);
                return;
        }


        amount = number_percent() + 150;

        ch->gold += amount;
        send_to_char
                ("You make some money for cooking your delicious meal!\n\r",
                 ch);
        {
                int       amount, condition;

                condition = ch->PCData->condition[CondFull];
                amount = 48 - condition;
                gain_condition(ch, CondFull, amount);
        }
        learn_from_success(ch, gsn_cook);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;

}

CMDF do_beg(CharData * ch, char *argument)
{


        char      arg1[MaxInputLength];
        CharData *victim;
        int       percent, xp;
        int       amount;

        if (IsNpc(ch))
                return;

        argument = one_argument(argument, arg1);

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                send_to_char("Beg for money from whom?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (IsSet(ch->in_room->RoomFlags, RoomSafe))
        {
                set_char_color(AtMagic, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (ch->position == PosFighting)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (victim->position == PosFighting)
        {
                send_to_char("They're a little busy right now.\n\r", ch);
                return;
        }

        if (ch->position <= PosSleeping)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (victim->position <= PosSleeping)
        {
                send_to_char("You might want to wake them first...\n\r", ch);
                return;
        }

        if (!IsNpc(victim))
        {
                send_to_char("You beg them for money.\n\r", ch);
                act(AtAction, "$n begs you to give $s some change.\n\r", ch,
                    NULL, victim, ToVict);
                act(AtAction, "$n begs $N for change.\n\r", ch, NULL, victim,
                    ToNotvict);
                return;
        }

        WaitState(ch, skill_table[gsn_beg]->beats);
        percent =
                number_percent() - ch->skill_level[OccupationAbility] +
                victim->top_level;

        if (percent > ch->PCData->learned[gsn_beg])
        {
                /*
                 * 
                 * * Failure.
                 * 
                 */
                send_to_char("You beg them for money but don't get any!\n\r",
                             ch);
                act(AtAction,
                    "$n is really getting on your nerves with all this begging!\n\r",
                    ch, NULL, victim, ToVict);
                act(AtAction, "$n begs $N for money.\n\r", ch, NULL, victim,
                    ToNotvict);

                if (victim->alignment < 0
                    && victim->top_level >= ch->top_level + 5)
                {
                        command_printf(victim,
                                       "yell %s is an annoying beggar and needs to be taught a lesson!",
                                       ch->name);
                        global_retcode =
                                multi_hit(victim, ch, TypeUndefined);
                }

                learn_from_failure(ch, gsn_beg);

                return;
        }


        act(AtAction, "$n begs $N for money.\n\r", ch, NULL, victim,
            ToNotvict);
        act(AtAction, "$n begs you for money!\n\r", ch, NULL, victim,
            ToVict);

        amount = UMin(victim->gold, number_range(1, 10));
        if (amount <= 0)
        {
                do_look(victim, ch->name);
                do_say(victim, "Sorry I have nothing to spare.\n\r");
                learn_from_failure(ch, gsn_beg);
                return;
        }

        ch->gold += amount;
        victim->gold -= amount;
        ch_printf(ch, "%s gives you %d credits.\n\r", victim->short_descr,
                  amount);
        learn_from_success(ch, gsn_beg);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        act(AtAction, "$N gives $n some money.\n\r", ch, NULL, victim,
            ToNotvict);
        act(AtAction, "You give $n some money.\n\r", ch, NULL, victim,
            ToVict);

        return;
}

CMDF do_dishwasher(CharData * ch, char *argument)
{

        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomSafe))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be somewhere safe to wash dishes.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_dishwasher]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou grab some dishes and start to wash.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n takes $s dishes and starts to wash them.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 7, do_dishwasher, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to wash dishes.\n\r",
                             ch);
                learn_from_failure(ch, gsn_dishwasher);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to wash the dishes.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_dishwasher])
        {
                send_to_char
                        ("You tried to wash dishes, but you broke a plate!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to wash dishes, but broke a plate!.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_dishwasher);
                ch->gold -= 5;
                return;
        }


        amount = (int) (number_percent() * 0.75);

        ch->gold += amount;
        send_to_char("You make some money for washing the dishes!\n\r", ch);
        learn_from_success(ch, gsn_dishwasher);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_busser(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomSafe))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be somewhere safe to bus tables.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_busser]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou choose a section and start to bus tables.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n picks $s tables and starts to bus them.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 9, do_busser, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to bus tables.\n\r",
                             ch);
                learn_from_failure(ch, gsn_busser);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to bus your tables.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_busser])
        {
                send_to_char
                        ("You tried to bus tables, but you broke a plate!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to bus tables, but broke a plate!.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_busser);
                ch->gold -= 5;
                return;
        }


        amount = number_percent() + 25;

        ch->gold += amount;
        send_to_char("You make some money for bussing the tables!\n\r", ch);
        learn_from_success(ch, gsn_busser);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_waiter(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomSafe))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be somewhere safe to wait tables.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_waiter]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou choose a section and start to take orders.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n picks $s tables and starts to take orders.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 11, do_waiter, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to take orders.\n\r",
                             ch);
                learn_from_failure(ch, gsn_waiter);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to take your orders.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_waiter])
        {
                send_to_char
                        ("You tried to take orders, but messed one up!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to take orders, but messed one up!.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_waiter);
                ch->gold -= 5;
                return;
        }


        amount = number_percent() + 90;

        ch->gold += amount;
        send_to_char("You make some money for waiting the tables!\n\r", ch);
        learn_from_success(ch, gsn_waiter);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_chef(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomKitchen))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in a kitchen to prepare a meal.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_chef]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou get your ingredients and start to prepare an extravagent meal.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n gets $s ingredients and starts to prepare a extravagent meal.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 27, do_chef, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to prepare an extravagent meal.\n\r",
                         ch);
                learn_from_failure(ch, gsn_chef);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to prepare your meal.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_chef])
        {
                send_to_char
                        ("You tried to prepare an extravagent meal, but it came out plain!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to prepare an extravagent meal, but it came out plain!\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_chef);
                return;
        }


        amount = number_percent() * 3 + 300;
        {
                int       amount, condition;

                condition = ch->PCData->condition[CondFull];
                amount = 48 - condition;
                gain_condition(ch, CondFull, amount);
        }
        ch->gold += amount;
        send_to_char("You are paid well for your fabulous meal!\n\r", ch);
        learn_from_success(ch, gsn_chef);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_streetcleaner(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (IsSet(ch->in_room->RoomFlags, RoomIndoors))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be outside to clean streets.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_streetcleaner]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou get your equipment and start to clean the street.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n gets $s equipment and starts to clean the street.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 6, do_streetcleaner, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to clean the street.\n\r",
                         ch);
                learn_from_failure(ch, gsn_streetcleaner);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to clean the street.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_streetcleaner])
        {
                send_to_char
                        ("You tried to clean the street, but it is still dirty!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to clean the street, but it is still dirty!\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_streetcleaner);
                return;
        }


        amount = number_percent() / 2;

        ch->gold += amount;
        send_to_char("You get some money for cleaning the street!\n\r", ch);
        learn_from_success(ch, gsn_streetcleaner);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_bartender(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomSafe))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be somewhere safe to mix drinks.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_bartender]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou get your ingredients and start to mix drinks.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n gets $s ingredients and starts to mix drinks.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 14, do_bartender, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to mix drinks.\n\r",
                             ch);
                learn_from_failure(ch, gsn_bartender);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to mix drinks.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_bartender])
        {
                send_to_char
                        ("You tried to mix drinks, but they were horrible!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to mix drinks, but they were horrible!\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_bartender);
                return;
        }


        amount = number_percent() + 200;
        {
                int       amount, condition;

                condition = ch->PCData->condition[CondThirst];
                amount = 48 - condition;
                gain_condition(ch, CondThirst, amount);
        }
        ch->gold += amount;
        send_to_char("You get some money for mixing peoples drinks!\n\r", ch);
        learn_from_success(ch, gsn_bartender);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_interiorcleaner(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomIndoors))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char("You must be inside to clean rooms.\n\r",
                                     ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_interiorcleaner]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou get your equipment and start to clean the room.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n gets $s equipment and starts to clean the room.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 14, do_interiorcleaner,
                                  1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to clean the room.\n\r",
                         ch);
                learn_from_failure(ch, gsn_interiorcleaner);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to clean the room.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_interiorcleaner])
        {
                send_to_char
                        ("You tried to clean the room, but it is still dirty!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to clean the room, but it is still dirty!\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_interiorcleaner);
                return;
        }


        amount = number_percent() + 125;

        ch->gold += amount;
        send_to_char("You get some money for cleaning the room!\n\r", ch);
        learn_from_success(ch, gsn_interiorcleaner);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_hotelcleaner(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomInn))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char("You must be in an inn to do this.\n\r",
                                     ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_hotelcleaner]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou get your equipment and start to clean the hotel room.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n gets $s equipment and starts to clean the hotel room.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 11, do_hotelcleaner, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to clean the hotel room.\n\r",
                         ch);
                learn_from_failure(ch, gsn_hotelcleaner);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to clean the hotel room.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_hotelcleaner])
        {
                send_to_char
                        ("You tried to clean the hotel room, but it is still dirty!\n\r",
                         ch);
                act(AtAction,
                    "$n tried to clean the hotel room, but it is still dirty!\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_hotelcleaner);
                return;
        }


        amount = number_percent() + 100;

        ch->gold += amount;
        send_to_char("You get some money for cleaning the hotel room!\n\r",
                     ch);
        learn_from_success(ch, gsn_hotelcleaner);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_secretary(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomOffice))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an office to be a secretary.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_secretary]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sit down and start to file your nails.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n sits down and starts to file &s nails.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 19, do_secretary, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to be a secretary.\n\r",
                         ch);
                learn_from_failure(ch, gsn_secretary);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to file your nails.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_secretary])
        {
                send_to_char("You tried to be a secretary, but failed\n\r",
                             ch);
                act(AtAction, "$n tried to be a secretary, but failed\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_secretary);
                return;
        }


        amount = number_percent() + 125;

        ch->gold += amount;
        send_to_char("You get some money for being a secretary!\n\r", ch);
        learn_from_success(ch, gsn_secretary);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;

}

CMDF do_clerk(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomOffice))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an office to be a clerk.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_clerk]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sit down and start to go over some books.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n sits down and starts to go over some books.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 25, do_clerk, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to be a clerk.\n\r",
                             ch);
                learn_from_failure(ch, gsn_clerk);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to go over the books.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_clerk])
        {
                send_to_char("You tried to be a clerk, but failed\n\r", ch);
                act(AtAction, "$n tried to be a clerk, but failed\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_clerk);
                return;
        }


        amount = number_percent() + 400;

        ch->gold += amount;
        send_to_char("You get some money for being a clerk!\n\r", ch);
        learn_from_success(ch, gsn_clerk);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_commmarketer(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomSafe))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be somewhere safe to be a comm marketer.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_commmarketer]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou start to randomly dial comm numbers.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n starts to randomly dial comm numbers.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 7, do_commmarketer, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to be a comm marketer.\n\r",
                         ch);
                learn_from_failure(ch, gsn_commmarketer);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to sell anything.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_commmarketer])
        {
                send_to_char
                        ("You tried to sell things over the commnet, but nobody bought.\n\r",
                         ch);
                act(AtAction,
                    "$n tried to sell things over the commnet, but nobody bought.\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_commmarketer);
                return;
        }


        amount = number_percent() + 0;

        ch->gold += amount;
        send_to_char("You get some money for being selling an item!\n\r", ch);
        learn_from_success(ch, gsn_commmarketer);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_marketer(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomOffice))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an office to market products.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_marketer]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou think about how to market a product.\n\r",
                                 ch);
                        act(AtPlain, "$n sits down and thinks.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 14, do_marketer, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to market the product.\n\r",
                         ch);
                learn_from_failure(ch, gsn_marketer);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to market the product.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_marketer])
        {
                send_to_char
                        ("You couldn't think of how to market your product.\n\r",
                         ch);
                act(AtAction,
                    "$n couldn't think of how to market $s product.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_marketer);
                return;
        }


        amount = number_percent() + 200;

        ch->gold += amount;
        send_to_char("You get some money for marketing your product!\n\r",
                     ch);
        learn_from_success(ch, gsn_marketer);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_solicitor(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_solicitor]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou pull out a product and try to sell it.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n pulls out a product and tries to sell it.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 27, do_solicitor, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou dont manage to sell your product.\n\r",
                             ch);
                learn_from_failure(ch, gsn_solicitor);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to sell your product.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_solicitor])
        {
                send_to_char
                        ("You tried to sell your product, but failed.\n\r",
                         ch);
                act(AtAction, "$n tried to sell $s product, but failed.\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_solicitor);
                return;
        }


        amount = number_percent() * 2 + 800;

        ch->gold += amount;
        send_to_char("You get some money for selling your product!\n\r", ch);
        learn_from_success(ch, gsn_solicitor);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_advertiser(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomExecutive))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an executive office to be an advertiser.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_advertiser]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sit and start to sketch a new ad.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n sits down and thinks about how to advertise their product.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 40, do_advertiser, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to advertise your product.\n\r",
                         ch);
                learn_from_failure(ch, gsn_advertiser);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to think clearly.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_advertiser])
        {
                send_to_char
                        ("You couldn't think of a way to advertise your product.\n\r",
                         ch);
                act(AtAction,
                    "$n couldn't think of a way to advertise &s product.\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_advertiser);
                return;
        }


        amount = number_percent() * 4 + 1600;

        ch->gold += amount;
        send_to_char("You get some money for advertising your product!\n\r",
                     ch);
        learn_from_success(ch, gsn_advertiser);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_banker(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomBank))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in a bank to be a banker.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_banker]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sit down and start to make transactions.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n sits down and starts to make transactions.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 11, do_banker, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to be a banker.\n\r",
                             ch);
                learn_from_failure(ch, gsn_banker);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to make your transactions.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_banker])
        {
                send_to_char("You tried to make transactions, but made a mistake!\n\r",ch);
                act(AtAction,"$n tried to make transactions, but made a mistake!\n\r",ch, NULL, ch, ToRoom);
//				ch_printf("You accidently transfer %d of your credits to 
                learn_from_failure(ch, gsn_banker);
                return;
        }


        amount = number_percent() + 200;

        ch->gold += amount;
        send_to_char("You get some money for making the transactions!\n\r",
                     ch);
        learn_from_success(ch, gsn_banker);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_accountant(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomOffice))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an office to be an accountant.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_accountant]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou pick up a tax book and start to look for loopholes.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n picks up a tax book and starts to look for loopholes.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 17, do_accountant, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to be an accountant.\n\r",
                         ch);
                learn_from_failure(ch, gsn_accountant);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to find a tax loophole.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_accountant])
        {
                send_to_char
                        ("You tried to find a tax loophole, but failed.\n\r",
                         ch);
                act(AtAction,
                    "$n tried to find a tax loophole, but failed.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_accountant);
                return;
        }


        amount = number_percent() + 550;

        ch->gold += amount;
        send_to_char("You get some money for being an accountant!\n\r", ch);
        learn_from_success(ch, gsn_accountant);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_investor(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomExecutive))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an executive office to be an investor.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_investor]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou start to play the stock market.\n\r",
                                 ch);
                        act(AtPlain, "$n starts to play the stock market.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 23, do_investor, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to be an investor.\n\r",
                         ch);
                learn_from_failure(ch, gsn_investor);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to play the market.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_investor])
        {
                send_to_char
                        ("You tried to play the stockmarket, but failed.\n\r",
                         ch);
                act(AtAction,
                    "$n tried to play the stockmarket, but failed.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_investor);
                return;
        }


        amount = number_percent() * 2 + 800;

        ch->gold += amount;
        send_to_char("You get some money for being an investor!\n\r", ch);
        learn_from_success(ch, gsn_investor);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_broker(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomExecutive))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an executive office to be a broker.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_broker]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou start making comm calls and selling stocks.\n\r",
                                 ch);
                        act(AtPlain, "$n starts talking on the comm.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 42, do_broker, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to be a broker.\n\r",
                             ch);
                learn_from_failure(ch, gsn_broker);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to sell stocks.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_broker])
        {
                send_to_char
                        ("You tried to sell stocks and funds, but failed.\n\r",
                         ch);
                act(AtAction,
                    "$n tried to sell stocks and funds, but failed.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_broker);
                return;
        }


        amount = number_percent() * 10 + 900;

        ch->gold += amount;
        send_to_char("You get some money for being a broker!\n\r", ch);
        learn_from_success(ch, gsn_broker);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_boardmember(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomBoardroom))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in a boardroom to be a board member.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_boardmember]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sit down at the table and have a meeting.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n sits down at the table and participates in the meeting.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 60, do_boardmember, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to be a board member.\n\r",
                         ch);
                learn_from_failure(ch, gsn_boardmember);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to participate in the meeting.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_boardmember])
        {
                send_to_char
                        ("You tried join the meeting, but were shunned.\n\r",
                         ch);
                act(AtAction,
                    "$n tried to join the meeting, but were shunned.\n\r", ch,
                    NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_boardmember);
                return;
        }


        amount = number_percent() * 20 + 2000;

        ch->gold += amount;
        send_to_char("You get some money for being a board member!\n\r", ch);
        learn_from_success(ch, gsn_boardmember);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_ceo(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomBoardroom))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in a boardroom to be a ceo.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_ceo]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sit down and start to conduct the meeting.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n sits down and starts to conduct the meeting.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 120, do_ceo, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to be a ceo.\n\r",
                             ch);
                learn_from_failure(ch, gsn_ceo);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to conduct the meeting.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_ceo])
        {
                send_to_char
                        ("You tried to conduct the meeting, but was out politiced.\n\r",
                         ch);
                act(AtAction,
                    "$n tried to conduct the meeting, but was out politiced.\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_ceo);
                return;
        }


        amount = number_percent() * 10 + 9000;

        ch->gold += amount;
        send_to_char("You get some money for being a ceo!\n\r", ch);
        learn_from_success(ch, gsn_ceo);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}

CMDF do_yourmom(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent, xp, amount, percentage;

        mudstrlcpy(arg, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (IsNpc(ch))
                        return;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;

                if (!IsSet(ch->in_room->RoomFlags, RoomOffice))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You must be in an office to be a secretary.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == PosFighting)
                {
                        send_to_char("You can't do that while fighting.\n\r",
                                     ch);
                        return;
                }

                if (ch->position <= PosSleeping)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_secretary]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sit down and start to file your nails.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n sits down and starts to file &s nails.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 19, do_secretary, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to be a secretary.\n\r",
                         ch);
                learn_from_failure(ch, gsn_secretary);
                return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to file your nails.\n\r",
                         ch);
                return;

        }

        ch->substate = SubNone;


        percent = number_percent() - ch->skill_level[OccupationAbility];

        if (percent > ch->PCData->learned[gsn_secretary])
        {
                send_to_char("You tried to be a secretary, but failed\n\r",
                             ch);
                act(AtAction, "$n tried to be a secretary, but failed\n\r",
                    ch, NULL, ch, ToRoom);
                learn_from_failure(ch, gsn_secretary);
                return;
        }


        amount = number_percent() + 125;

        ch->gold += amount;
        send_to_char("You get some money for being a secretary!\n\r", ch);
        learn_from_success(ch, gsn_secretary);
        xp = UMin(amount * 10,
                  (exp_level(ch->skill_level[OccupationAbility] + 1) -
                   exp_level(ch->skill_level[OccupationAbility])));
        gain_exp(ch, xp, OccupationAbility);
        ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
        return;
}
