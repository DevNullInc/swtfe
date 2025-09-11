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
 * Medical system handling player health, injuries, healing, and medical interactions in the game. *
 ****************************************************************************************/

#include <string.h>
#include "mud.hpp"
#include "races.hpp"

CMDF do_autopsy(CharData * ch, char *argument)
{
        ObjData *obj;
        char      arg[MaxStringLength];
        char      buf[MaxStringLength];
        int       chance;
		char      armed_by[MaxStringLength];

        mudstrlcpy(arg, argument, MSL);

        if (IsNpc(ch))
                return;

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("Syntax: autopsy <corpsename>\n\r", ch);
                        return;
                }
                obj = get_obj_here(ch, arg);
                if (!obj
                    || (obj->item_type != ItemCorpseNpc
                        && obj->item_type != ItemCorpsePc))
                {
                        send_to_char("This only works on corpses!\n\r", ch);
                        return;
                }
                chance = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                             learned
                                                             [gsn_autopsy]);

                if (number_percent() < chance)
                {
                        send_to_char
                                ("&GYou begin the long Process doing an autopsy on the corpse.\n\r",
                                 ch);
                        act(AtPlain, "$n begins to work the corpse.", ch,
                            NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 10, do_autopsy, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out how to proceed.\n\r",
                             ch);
                learn_from_failure(ch, gsn_autopsy);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MSL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }
        ch->substate = SubNone;

        obj = get_obj_here(ch, arg);
        if (!obj
            || (obj->item_type != ItemCorpseNpc
                && obj->item_type != ItemCorpsePc))
        {
                send_to_char("This only works on corpses!\n\r", ch);
                return;
        }
        chance = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                     learned[gsn_autopsy]);
        act(AtImmort, "$n has finished $s work on the corpse.", ch, NULL,
            NULL, ToRoom);
		if (obj->armed_by && obj->armed_by[0] != '\0')
			strcpy(armed_by, obj->armed_by);
		else
			armed_by[0] = '\0';
        /*
         * required due to object grouping 
         */
		separate_obj(obj);
        /*
         * Then Remove from game 
         */
        extract_obj(obj);

        if (number_percent() < chance) 
        { 
			if (armed_by[0] == '\0') {
			/* Failed */
                send_to_char
                        ("You have finished your autopsy, but are unable to determine the killer.\n\r",
                         ch);
			}
			else {
				/* Success */
                send_to_char("You have finished your autopsy.\n\r", ch);
                {
                    /* Use stacked buffer approach to avoid truncation warnings */
                    char tmp[MSL*2]; /* Temporary buffer large enough for the format operation */
                    snprintf(tmp, sizeof(tmp),
                             "Only to find that the killer is none other than %s!\n\r",
                             armed_by);
                    tmp[sizeof(tmp)-1] = '\0'; /* Ensure null termination */
                    strncpy(buf, tmp, MSL-1);
                    buf[MSL-1] = '\0'; /* Ensure null termination */
                    send_to_char(buf, ch);
                }
			}
        }
		else {
			send_to_char("You fail to figure out the cause of death and mutilate the corpse beyond recognition.\n\r", ch);
			return;
		}


        {
                long      xpgain;

                xpgain = UMIN(15000,
                              (exp_level(ch->skill_level[MedicAbility] + 1) -
                               exp_level(ch->skill_level[MedicAbility])));
                gain_exp(ch, xpgain, MedicAbility);
                ch_printf(ch, "You gain %d medical experience.", xpgain);
        }
        learn_from_success(ch, gsn_autopsy);
}

CMDF do_diagnose(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      buf[MaxStringLength];
        int       percentage;
        bool      checkmedpac;
        ObjData *obj;
        CharData *victim;

        if (IsNpc(ch))
                return;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: diagnose <target>\n\r&w", ch);
                        return;
                }

                snprintf(buf, MSL, "0.%s", argument);

                if ((victim = get_char_room(ch, buf)) == NULL)
                {
                        send_to_char("You don't see that person here.\n\r",
                                     ch);
                        return;
                }

                if (IsNpc(victim))
                {
                        send_to_char("That's not a player!\n\r", ch);
                        return;
                }


                checkmedpac = FALSE;

                if (!xIS_SET(ch->in_room->RoomFlags, RoomSafe))
                {
                        send_to_char
                                ("&RYou need to be somewhere that you don't have to worry about danger.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                        if (obj->item_type == ItemMedpac)
                                checkmedpac = TRUE;

                if (!checkmedpac)
                {
                        send_to_char("&RYou need a medpac to do that.\n\r",
                                     ch);
                        return;
                }


                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_diagnose]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou grab your medpac and try to diagnose your subject.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n takes $s medpac and begins to examine $s subject.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 15, do_diagnose, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou try to start your diagnosis, but don't know where to begin.\n\r",
                         ch);
                learn_from_failure(ch, gsn_diagnose);
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
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;

        checkmedpac = FALSE;
        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ItemMedpac && checkmedpac == FALSE)
                {
                        checkmedpac = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }

        }

        snprintf(buf, MSL, "0.%s", arg);

        if ((victim = get_char_room(ch, buf)) == NULL)
        {
                send_to_char
                        ("You can't find that entry in the galactic net.\n\r",
                         ch);
                return;
        }

        if (IsNpc(victim))
        {
                send_to_char("That's not a player!\n\r", ch);
                return;
        }

        percentage = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_diagnose]);
        if (number_percent() > percentage * 2 || (!checkmedpac))
        {
                send_to_char("&RYou finish your notes on the patient.\n\r",
                             ch);
                send_to_char
                        ("&RYou start you reveiw that notes, but notice an error.\n\r",
                         ch);
                send_to_char
                        ("&RDue to your negligence, you diagnose them as the wrong race.\n\r",
                         ch);
                learn_from_failure(ch, gsn_diagnose);
                return;
        }

        send_to_char
                ("&GYou finish your diagnosis and look over your results.&w\n\r",
                 ch);
        act(AtPlain, "$n finishes the diagnosis and looks over the results.",
            ch, NULL, argument, ToRoom);

        /*
         * We already check to see if its an NPC above. - Gavin
         * if (!victim->pcdata)
         * return;
         */

        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        ch_printf(ch, "&b[&B|&cMedical Report for&C:&z %-68s\n\r",
                  victim->pcdata->title);
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        ch_printf(ch, "&b[&B|&cName&C:&z %-68s\n\r", victim->name);
        ch_printf(ch, "&b[&B|&cAge&C:&z %d\n\r", get_age(victim));
        ch_printf(ch, "&b[&B|&cCurrent HP&C:&z %d\n\r", victim->hit);
        ch_printf(ch, "&b[&B|&cCurrent ENDURANCE&C:&z %d\n\r",
                  victim->endurance);
        ch_printf(ch, "&b[&B|&cDisease&C:&z %s\n\r",
                  capitalize(illness_list[victim->pcdata->illness]));
        ch_printf(ch, "&b[&B|&cStrength&C:&z %d of a maximum %d\n\r",
                  get_curr_str(victim),
                  (victim->race->attr_modifier(AttrStrength) + 20));
        ch_printf(ch, "&b[&B|&cIntelligence&C:&z %d of a maximum %d\n\r",
                  get_curr_int(victim),
                  (victim->race->attr_modifier(AttrIntelligence) + 20));
        ch_printf(ch, "&b[&B|&cWisdom&C:&z %d of a maximum %d\n\r",
                  get_curr_wis(victim),
                  (victim->race->attr_modifier(AttrWisdom) + 20));
        ch_printf(ch, "&b[&B|&cConstitution&C:&z %d of a maximum %d\n\r",
                  get_curr_con(victim),
                  (victim->race->attr_modifier(AttrConstitution) + 20));
        ch_printf(ch, "&b[&B|&cCharisma&C:&z %d of a maximum %d\n\r",
                  get_curr_cha(victim),
                  (victim->race->attr_modifier(AttrCharisma) + 20));
        ch_printf(ch, "&b[&B|&cDexterity&C:&z %d of a maximum %d\n\r",
                  get_curr_dex(victim),
                  (victim->race->attr_modifier(AttrDexterity) + 20));
        ch_printf(ch, "&b[&B|&cBones Broken&C:&z %-60s\n\r",
                  flag_string(victim->bodyparts, body_parts));
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);

        {
                long      xpgain;

                xpgain = UMIN(victim->top_level * 100,
                              (exp_level(ch->skill_level[MedicAbility] + 1) -
                               exp_level(ch->skill_level[MedicAbility])));
                gain_exp(ch, xpgain, MedicAbility);
                ch_printf(ch, "You gain %d medic experience.", xpgain);
        }

        learn_from_success(ch, gsn_diagnose);
}

int       get_bodypart(char *flag);
CMDF do_splint(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      arg3[MaxInputLength];
        int       value, chance;
        CharData *victim;

        if (IsNpc(ch))
                return;

        argument = one_argument(argument, arg);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        ch_printf(ch, "Splint who's what?!?\n\r");
                        return;
                }

                if ((victim = get_char_room(ch, arg)) == NULL)
                {
                        send_to_char("They are not here.\n\r", ch);
                        return;
                }
                if (IsNpc(victim))
                {
                        ch_printf(ch, "UHHH RIIIIGHT!\n\r");
                        return;
                }
                if (victim == ch)
                {
                        send_to_char
                                ("How do you expect to splint yourself?\n\r",
                                 ch);
                        return;
                }

                if (argument[0] == '\0')
                {
                        ch_printf(ch, "Splint who's what?!?\n\r");
                        return;
                }
                chance = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                             learned
                                                             [gsn_splint]);
                if (number_percent() < chance)
                {
                        act(AtAction,
                            "You take out some plaster and gauss from your bag and begin to assemble a cast on $N's $t.",
                            ch, argument, victim, ToChar);
                        act(AtAction,
                            "$n takes out some plaster and gauss from $s's bag and begins to assemble a cast on your $t.",
                            ch, argument, victim, ToVict);
                        act(AtAction,
                            "$n takes out some plaster and gauss from $s's bag and begins to assemble a cast on $N's $t.",
                            ch, argument, victim, ToNotvict);
                        WaitState(victim, 10 * PulseViolence);
                        add_timer(ch, TimerDoFun, 10, do_splint, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(argument);
                        return;
                }
                send_to_char("&RYou don't know what todo.\n\r", ch);
                learn_from_failure(ch, gsn_splint);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg3, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }
        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }
        if (victim == ch)
        {
                send_to_char("How do you expect to splint yourself?\n\r", ch);
                return;
        }
        chance = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                     learned[gsn_splint]);

        if (number_percent() > chance * 2)
        {
                act(AtAction, "You fumble and mess up.", ch, NULL, NULL,
                    ToChar);
                act(AtAction, "$n fumbles and creats a big mess.", ch, NULL,
                    NULL, ToRoom);
                learn_from_failure(ch, gsn_splint);
                return;
        }

        value = get_bodypart(arg3);
        if (value == -1)
        {
                ch_printf(ch, "Unknown Body part: %s\n\r", arg3);
                return;
        }
        else if (!IsSet(victim->bodyparts, 1 << value))
        {
                send_to_char
                        ("Now why would you want to splint that? Its not even broken.\n\r",
                         ch);
                return;
        }
        else
        {
                act(AtAction,
                    "You finish and create a splended cast on $N's $t.", ch,
                    arg3, victim, ToChar);
                act(AtAction,
                    "$n finishes and create a splended cast on your $t.", ch,
                    arg3, victim, ToVict);
                act(AtAction,
                    "$n finishes and create a splended cast on  $N's $t.", ch,
                    arg3, victim, ToNotvict);
                RemoveBit(victim->bodyparts, 1 << value);
        }
        learn_from_success(ch, gsn_splint);
}

CMDF do_first_aid(CharData * ch, char *argument)
{
        ObjData *medpac;
        CharData *victim;
        int       heal;

        if (IsNpc(ch))
                return;

        if (ch->position == PosFighting)
        {
                send_to_char("You can't do that while fighting!\n\r", ch);
                return;
        }

        medpac = get_eq_char(ch, WearHold);
        if (!medpac || medpac->item_type != ItemMedpac)
        {
                send_to_char("You need to be holding a medpac.\n\r", ch);
                return;
        }

        if (medpac->value[0] <= 0)
        {
                send_to_char("Your medpac seems to be empty.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
                victim = ch;
        else
                victim = get_char_room(ch, argument);

        if (!victim)
        {
                ch_printf(ch, "I don't see any %s here...\n\r", argument);
                return;
        }

        heal = number_range(1, 150);

        if (heal > ch->pcdata->learned[gsn_first_aid] * 2)
        {
                ch_printf(ch, "You fail in your attempt at first aid.\n\r");
                learn_from_failure(ch, gsn_first_aid);
                return;
        }

        if (victim == ch)
        {
                send_to_char("You tend to your wounds.\n\r", ch);
                act(AtAction, "$n uses $t to help heal $s wounds.", ch,
                    medpac->short_descr, victim, ToRoom);
        }
        else
        {
                act(AtAction, "You tend to $N's wounds.", ch, NULL, victim,
                    ToChar);
                act(AtAction, "$n uses $t to help heal $N's wounds.", ch,
                    medpac->short_descr, victim, ToNotvict);
                act(AtAction, "$n uses $t to help heal your wounds.", ch,
                    medpac->short_descr, victim, ToVict);
        }

        --medpac->value[0];

        if (medpac->value[0] == 0)
        {
                separate_obj(medpac);
                obj_from_char(medpac);
                extract_obj(medpac);
                act(AtAction,
                    "While healing $N's wounds, you use up $t and discard it.",
                    ch, medpac->short_descr, victim, ToChar);
        }

        victim->hit += URANGE(0, heal, victim->max_hit - victim->hit);

        learn_from_success(ch, gsn_first_aid);
}

CMDF do_makemedkit(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      buf[MaxStringLength];
        int       level, chance, Strength = 0, weight = 0;
        bool      checktool, checkdrink, checkchem;
        ObjData *obj;
        ObjIndexData *pObjIndex;
        int       vnum;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makemedkit <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checkdrink = FALSE;
                checkchem = FALSE;

                if (!xIS_SET(ch->in_room->RoomFlags, RoomFactory))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ItemToolkit)
                                checktool = TRUE;
                        if (obj->item_type == ItemDrinkCon
                            && obj->value[1] == 0)
                                checkdrink = TRUE;
                        if (obj->item_type == ItemChemical)
                                checkchem = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a med kit.\n\r",
                                 ch);
                        return;
                }

                if (!checkdrink)
                {
                        send_to_char
                                ("&RYou will need an empty drink container to mix and hold the chemicals.\n\r",
                                 ch);
                        return;
                }

                if (!checkchem)
                {
                        send_to_char
                                ("&RSome chemicals would come in handy!\n\r",
                                 ch);
                        return;
                }

                chance = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                             learned
                                                             [gsn_makemedkit]);
                if (number_percent() < chance)
                {
                        send_to_char
                                ("&GYou begin the long Process of making a medkit.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n takes $s tools and a drink container and begins to work on something.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 25, do_makemedkit, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou your mixture fizles and dies.\n\r", ch);
                learn_from_failure(ch, gsn_makemedkit);
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
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;

        level = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makemedkit]);
        vnum = ObjVnumMedkit;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdrink = FALSE;
        checkchem = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ItemToolkit)
                        checktool = TRUE;
                if (obj->item_type == ItemDrinkCon && checkdrink == FALSE
                    && obj->value[1] == 0)
                {
                        checkdrink = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ItemChemical)
                {
                        Strength = URANGE(10, obj->value[0], level * 5);
                        weight = obj->weight;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkchem = TRUE;
                }
        }

        chance = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                     learned[gsn_makemedkit]);

        if (number_percent() > chance * 2 || (!checktool) || (!checkdrink)
            || (!checkchem))
        {
                send_to_char
                        ("&RJust as you are about to finish your work,\n\ryour newly created medkit bubles over and dies...doh!\n\r",
                         ch);
                learn_from_failure(ch, gsn_makemedkit);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ItemMedpac;
        obj->wear_flags = 0;
        SetBit(obj->wear_flags, ItemHold);
        SetBit(obj->wear_flags, ItemTake);
        obj->level = level;
        obj->weight = weight;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " medkit", MSL);
        obj->name = STRALLOC(buf);
        mudstrlcpy(buf, arg, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was carelessly left here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[0] = Strength / 2;
        obj->value[1] = Strength;
        obj->cost = obj->value[1] * 5;
        if (IsObjStat(obj, ItemPrototype))
                RemoveBit(obj->extra_flags, ItemPrototype);

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your superb new med kit.&w\n\r",
                 ch);
        act(AtPlain,
            "$g finishes making $s brand new medkit and holds it up for everyone to see.",
            ch, NULL, argument, ToRoom);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 50,
                              (exp_level(ch->skill_level[MedicAbility] + 1) -
                               exp_level(ch->skill_level[MedicAbility])));
                gain_exp(ch, xpgain, MedicAbility);
                ch_printf(ch, "You gain %d medical experience.", xpgain);
        }
        learn_from_success(ch, gsn_makemedkit);
}
