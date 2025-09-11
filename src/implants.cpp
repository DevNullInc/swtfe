/* vim: ts=8 et ft=c sw=8
 *****************************************************************************************
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
 *                          \__\/ \_\  \__\/\_____\\_____\/ \_____\/ \__\/ \_\  \__\/ *
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
 * Cybernetic implant system for player enhancements and character modifications. *
 ****************************************************************************************/
#include <string.h>
#include "mud.hpp"

int get_type_number args((char *name));

char     *const implant_names[] = {
        "Muscular Accuators",   /* Strength */
        "Ocular Reflex Amplifiers", /* Dexterity */
        "Cellular Regeneration Stimulator", /* Constitution */
        "Neural Path Stabilizer",   /* Intelligence */
        "Cognative Augmentation",   /* Wisdom */
        "Dental Revision",  /* Charisma */
        "Nutritional Reformation"   /* Hunger/Thirst */
};

/* Name of the affect, I think I wanna have the character field in an integer array check this, I dunno... */
char     *const implant_affect_names[] = {
        "Negative",
        "None",
        "Minor",
        "Major"
};

int get_implant_affect(CharData * ch, int implant)
{
        if (IsNpc(ch) || !ch->pcdata)
                return 0;
        else if (ch->pcdata->implants[implant] >= 1
                 && ch->pcdata->implants[implant] <= 3)
                return ch->pcdata->implants[implant] - 1;
        else
                return 0;
}

int get_type_number(char *name)
{
        int       count;

        for (count = 0; count < MaxImplantTypes; count++)
                if (!str_cmp(name, implant_names[count]))
                        return count;
        return -1;
}

CMDF do_makeimplant(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      buf[MaxStringLength];
        int       level, percentage, charge, type, count;
        bool      checktool, checkdura, checkbatt, checkoven, checkchem,
                checkcirc;
        ObjData *obj;
        ObjIndexData *pObjIndex;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makeimplant <type>\n\r&w", ch);
                        send_to_char("Availbable types are:\n\r", ch);
                        for (count = 0; count < MaxImplantTypes; count++)
                                ch_printf(ch, "\t&B%d&z) &w%s\n\r", count,
                                          implant_names[count]);
                        return;
                }

                checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkchem = FALSE;
                checkcirc = FALSE;

                if (is_number(arg))
                        type = atoi(arg);
                else
                        type = get_type_number(arg);

                if ((type < 0 || type >= MaxImplantTypes))
                {
                        send_to_char
                                ("That is an invalid type, your option are:\n\r",
                                 ch);
                        for (count = 0; count < MaxImplantTypes; count++)
                                ch_printf(ch, "\t&B%d&z) &w%s\n\r", count,
                                          implant_names[count]);
                        return;
                }
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
                        if (obj->item_type == ItemDurasteel
                            || obj->item_type == ItemDuraplast)
                                checkdura = TRUE;
                        if (obj->item_type == ItemBattery)
                                checkbatt = TRUE;
                        if (obj->item_type == ItemOven)
                                checkoven = TRUE;
                        if (obj->item_type == ItemChemical)
                                checkchem = TRUE;
                        if (obj->item_type == ItemCircuit)
                                checkcirc = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make an implant.\n\r",
                                 ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char
                                ("&RYou need some duraplast or durasteel to make it out of.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a power source for the implants construction.\n\r",
                                 ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char
                                ("&RYou need a small furnace to heat the metal.\n\r",
                                 ch);
                        return;
                }

                if (!checkchem)
                {
                        send_to_char
                                ("&RYou need some chemicals to aid in the biological Process.\n\r",
                                 ch);
                        return;
                }
                if (!checkcirc)

                {
                        send_to_char
                                ("&RYou need a circuit to control the electricals of the implant.\n\r",
                                 ch);
                        return;
                }
                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makeimplant]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou gather your supplies, and start to design the small implant.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n takes $s supplies and starts to work on something very small.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 35, do_makeimplant, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou gather your supplies, but cannot conceive of how your implant will work.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeimplant);
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
                        ("&RYou stop to do something else, and when you come back, your implant fell apart!.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;

        if (is_number(arg))
                type = atoi(arg);
        else
                type = get_type_number(arg);

        level = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makeimplant]);

        if ((pObjIndex = get_obj_index(ObjVnumImplant)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdura = FALSE;
        checkbatt = FALSE;
        checkoven = FALSE;
        checkchem = FALSE;
        checkcirc = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ItemToolkit)
                        checktool = TRUE;
                if (obj->item_type == ItemOven)
                        checkoven = TRUE;
                if ((obj->item_type == ItemDurasteel
                     || obj->item_type == ItemDuraplast)
                    && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ItemChemical && checkchem == FALSE)
                {
                        checkchem = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ItemCircuit && checkcirc == FALSE)
                {
                        checkcirc = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ItemBattery && checkbatt == FALSE)
                {
                        charge = UMAX(5, obj->value[0]);
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
        }

        percentage = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makeimplant]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura)
            || (!checkbatt) || (!checkoven) || (!checkcirc) || (!checkchem))
        {
                send_to_char("&RYou finish your work on the implant.\n\r",
                             ch);
                snprintf(buf, MSL,
                         "You hold up your new %s, and study it carefully.\n\r",
                         implant_names[type]);
                send_to_char(buf, ch);
                send_to_char
                        ("&RWhile you hold it up, it slips between your fingers, falls to the ground, and shatters.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeimplant);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ItemImplant;
        SetBit(obj->wear_flags, ItemTake);
        obj->level = level;
        obj->weight = 1;
        stralloc_printf(&obj->name, "%s implant", implant_names[type]);
        stralloc_printf(&obj->short_descr, "A %s", implant_names[type]);
        stralloc_printf(&obj->description, "A %s was left here",
                        implant_names[type]);
        /*
         * Quality of the Implant 
         */
        obj->value[0] =
                URANGE(0,
                       get_curr_int(ch) +
                       ch->pcdata->learned[gsn_makeimplant] / 5 +
                       ch->skill_level[MedicAbility], 100);
        /*
         * type of implant 
         */
        obj->value[1] = type;
        obj->cost = number_range(1000, 10000);

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou hold your implant up to the light to examine it.&w\n\r",
                 ch);
        send_to_char
                ("&GYou test the implant with a small piece of machinery, and conclude that the item is working.&w\n\r",
                 ch);
        act(AtPlain, "$n finishes crafting a small piece of technology.", ch,
            NULL, argument, ToRoom);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 2,
                              (exp_level
                               (ch->skill_level[EngineeringAbility] + 1) -
                               exp_level(ch->
                                         skill_level[EngineeringAbility])));
                gain_exp(ch, xpgain, EngineeringAbility);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_makeimplant);
}

CMDF do_implant(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      arg2[MaxInputLength];
        int       percentage;
        bool      checktool, checkneed, checkimpl;
        int       type = 0, quality = 0;
        ObjData *obj;
        CharData *victim = NULL;
        bool      npc = false;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0' || arg2[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: Implant <subject> <implant>\n\r&w",
                                 ch);
                        return;
                }

                checktool = FALSE;
                checkneed = FALSE;
                checkimpl = FALSE;

                if ((victim = get_char_room(ch, arg)) == NULL)
                {
                        send_to_char
                                ("That person is not around! How can you perform surgery on them?",
                                 ch);
                        return;
                }
                npc = IsNpc(victim);
                if (ch == victim)
                {
                        send_to_char
                                ("That person is not a suitable candidate for that implant.",
                                 ch);
                        return;
                }

                if (victim->position != PosSleeping)
                {
                        send_to_char
                                ("How do you expect to operate on them while they are awake?",
                                 ch);
                        send_to_char
                                ("They start to operate on you, but you are still awake. That will really hurt!",
                                 victim);
                        return;
                }

                if ((obj = get_obj_carry(ch, arg2)) == NULL
                    || obj->item_type != ItemImplant)
                {
                        send_to_char
                                ("How do you expect to do that without an implant?",
                                 ch);
                        return;
                }
                if (!npc)
                {
                        if (victim->pcdata->implants[obj->value[1]] >= 0
                            && victim->pcdata->implants[obj->value[1]] <= 3)
                        {
                                send_to_char
                                        ("They already have tha sort of implant.",
                                         ch);
                                return;
                        }
                }


                if (!xIS_SET(ch->in_room->RoomFlags, RoomSafe)
                    || !xIS_SET(ch->in_room->RoomFlags, RoomIndoors))
                {
                        send_to_char
                                ("&RYou must be indoors somewhere, where it is safe to operate.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ItemToolkit)
                                checktool = TRUE;
                        if (obj->item_type == ItemThread)
                                checkneed = TRUE;
                        if (obj->item_type == ItemImplant)
                                checkimpl = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to operate of them.\n\r",
                                 ch);
                        return;
                }

                if (!checkneed)
                {
                        send_to_char
                                ("&RHow do you expect to perform an operation with no surgical thread.\n\r",
                                 ch);
                        return;
                }

                if (!checkimpl)
                {
                        send_to_char
                                ("&RUmm, what are you trying to implant, exactly?\n\r",
                                 ch);
                        return;
                }

                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_implant]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou start to operate on them, keeping the implant close at hand.\n\r",
                                 ch);
                        send_to_char("&GThey begin the operation on you.\n\r",
                                     victim);
                        add_timer(ch, TimerDoFun, 35, do_implant, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou begin your work on them, but you slip at the very beginning, inflicting a wound.\n\r",
                         ch);
                send_to_char
                        ("&RThey begin the operation on you, but they slip and slice you.\n\r",
                         victim);
                damage(victim, victim, 100, TypeUndefined);
                learn_from_failure(ch, gsn_implant);
                return;

        case 1:
                if (!ch->dest_buf || !ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                break;

        case SubTimerDoAbort:
                victim = get_char_world(ch, (char *) ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SubNone;
                if (victim != NULL)
                {
                        if (victim->in_room == ch->in_room)
                        {
                                send_to_char
                                        ("&RYou stop doing your operation in the middle of it, causing severe damage to your subject!\n\r",
                                         ch);
                                send_to_char
                                        ("&RThey stopped in the middle of your surgery, and did nothing to heal you from your incisions.\n\r",
                                         victim);
                                damage(victim, victim, 600, TypeUndefined);
                        }
                        else
                        {
                                send_to_char
                                        ("&RYou stop doing your operation in the middle of it, causing severe damage to your subject!\n\r",
                                         ch);
                                send_to_char
                                        ("&RSince you were being operated on, and then got up and left without noticing your cuts, you suddenly feel a sharp pain as the anesthetic wears off.\n\r",
                                         victim);
                                damage(victim, victim, 600, TypeUndefined);
                        }
                }
                send_to_char
                        ("&RYou stop doing your operation in the middle of it.\n\r",
                         ch);
                send_to_char
                        ("&RHowever, your subject does not appear to be here any longer.\n\r",
                         ch);
                send_to_char("&RHow could you not notice?!?!.\n\r", ch);
                return;
        }

        ch->substate = SubNone;

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char
                        ("That person is not around! How can you perform surgery on them?",
                         ch);
                return;
        }
        npc = IsNpc(victim);

        if (victim->position != PosSleeping)
        {
                send_to_char
                        ("They woke up in the middl of the operation, and felt severe pain!",
                         ch);
                send_to_char
                        ("You awoke in the middle of surgey to the horror of feeling a knife cut your flesh!",
                         victim);
                damage(victim, victim, 300, TypeUndefined);
        }

        checktool = FALSE;
        checkneed = FALSE;
        checkimpl = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ItemToolkit)
                        checktool = TRUE;
                if (obj->item_type == ItemThread)
                        checkneed = TRUE;
                if (obj->item_type == ItemImplant
                    && get_obj_carry(ch, arg2) == obj)
                {
                        checkimpl = TRUE;
                        quality = obj->value[0];
                        type = obj->value[1];
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }

        percentage = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_implant]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkneed)
            || (!checkimpl))
        {
                send_to_char
                        ("&RYou finish your surget on the victim, and sew them up.\n\r",
                         ch);
                send_to_char
                        ("&RHowever, you notice that the implant is still on the table next to you.\n\r",
                         ch);
                send_to_char
                        ("&RThey finish the sergury, and you awake, but you feel no different.\n\r",
                         victim);
                damage(victim, victim, 100, TypeUndefined);
                learn_from_failure(ch, gsn_implant);
                return;
        }

        quality = quality - (30 - get_curr_dex(ch));

        if (quality <= 20)
                quality = 0;
        else if (quality <= 40)
                quality = 1;
        else if (quality <= 60)
                quality = 2;
        else
                quality = 3;

        if (!npc)
        {
                victim->pcdata->implants[type] = quality;
        }
        send_to_char
                ("&GYou finish your surgery on the victim, and sew them up.\n\r",
                 ch);
        do_wake(victim, "");
        send_to_char
                ("&GAfter they wake, you check their vitals, and with a little time, will be better than ever.&w\n\r",
                 ch);
        send_to_char
                ("&GYou feel slightly woozy, and a little weak, but otherwise fine. The surgery was a complete success.&w\n\r",
                 victim);

        {
                long      xpgain;

                xpgain = UMIN(victim->top_level * 100,
                              (exp_level
                               (ch->skill_level[MedicAbility] + 1) -
                               exp_level(ch->skill_level[MedicAbility])));
                if (!npc)
                {
                        xpgain -= 10 * (ch->skill_level[MedicAbility] + 1);
                }
                gain_exp(ch, xpgain, MedicAbility);
                ch_printf(ch, "You gain %d engineering experience.\n\r",
                          xpgain);
        }

        learn_from_success(ch, gsn_implant);
}
