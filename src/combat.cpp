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
 *                             Advanced Combat Module                                    *
 ****************************************************************************************/
/*
 * New combatant stuff, or just general combat related stuff.
 */

#include <string.h>
#include "mud.hpp"

extern bool could_dual(CharData * ch);
extern void separate_obj(ObjData * obj);

CMDF do_draw(CharData * ch, char *argument)
{
        bool      silent = FALSE;
        ObjData *holster1 = NULL, *holster2 = NULL;
        ObjData *wield1, *wield2;
        ObjData *hold;

        if (!strcmp(argument, "-silence"))
                silent = TRUE;

        holster1 = get_eq_char(ch, WearHolsterL);
        if (!holster1)
        {
                holster1 = get_eq_char(ch, WearHolsterR);
                if (!holster1)
                {
                        if (!silent)
                                send_to_char
                                        ("You aren't wearing a holster.\n\r",
                                         ch);
                        return;
                }
        }
        else
                holster2 = get_eq_char(ch, WearHolsterR);

        if ((holster1->item_type != ItemHolster &&
             (holster2 && holster2->item_type != ItemHolster)) ||
            (holster1->item_type != ItemHolster && !holster2))
        {
                if (!silent)
                        send_to_char
                                ("That's not a holster you're wearing!\n\r",
                                 ch);
                return;
        }

        if ((!holster1->first_content
             && (holster2 && !holster2->first_content))
            || (!holster1->first_content && !holster2))
        {
                if (!silent)
                        send_to_char("Your holster(s) are empty!\n\r", ch);
                return;
        }

        wield1 = get_eq_char(ch, WearWield);
        wield2 = get_eq_char(ch, WearDualWield);

        if (wield1 != NULL)
                unequip_char(ch, wield1);
        if (wield2 != NULL)
                unequip_char(ch, wield2);

        wield1 = holster1->first_content;
        separate_obj(wield1);
        if (holster2)
        {
                wield2 = holster2->first_content;
                separate_obj(wield2);
        }

        if (wield1 != NULL)
        {
                obj_from_obj(wield1);
                wield1 = obj_to_char(wield1, ch);
                equip_char(ch, wield1, WearWield);
                act(AtAction, "$n draws $p.", ch, wield1, holster1, ToRoom);
                act(AtAction, "You draw $p.", ch, wield1, holster1, ToChar);
        }

        if (wield2 != NULL)
        {
                hold = get_eq_char(ch, WearHold);
                if (hold != NULL)
                        unequip_char(ch, hold);

                obj_from_obj(wield2);
                wield2 = obj_to_char(wield2, ch);
                equip_char(ch, wield2, WearDualWield);
                act(AtAction, "$n draws $p.", ch, wield2, holster2, ToRoom);
                act(AtAction, "You draw $p.", ch, wield2, holster2, ToChar);
        }

        return;
}

CMDF do_holster(CharData * ch, char * argument)
{
        static_cast<void>(argument);    /* Unused parameter */
        
        ObjData *wield1 = get_eq_char(ch, WearWield),
                *wield2 = get_eq_char(ch, WearDualWield);
        ObjData *holster1 = get_eq_char(ch, WearHolsterL),
                *holster2 = get_eq_char(ch, WearHolsterR);

        if (!(holster1 = get_eq_char(ch, WearHolsterL)))
        {
                if (!(holster1 = get_eq_char(ch, WearHolsterR)))
                {
                        send_to_char("You aren't wearing a holster!\n\r", ch);
                        return;
                }
        }
        else
                holster2 = get_eq_char(ch, WearHolsterR);

        if (holster1->item_type != ItemHolster &&
            (holster2 && holster2->item_type != ItemHolster))
        {
                send_to_char("That's not a holster you're wearing.\n\r", ch);
                return;
        }

        if (holster1->first_content && holster2 && holster2->first_content)
        {
                send_to_char("There's already something in the holster.\n\r",
                             ch);
                return;
        }

        if (!wield1 && !wield2)
        {
                send_to_char("You aren't wielding anything to holster.\n\r",
                             ch);
                return;
        }

        if (wield1 != NULL)
        {
                char      buf[MaxStringLength], buf2[MaxStringLength];

                if (!holster1->first_content)
                {
                        unequip_char(ch, wield1);
                        obj_from_char(wield1);
                        wield1 = obj_to_obj(wield1, holster1);
                        separate_obj(wield1);
                        if (holster1->wear_loc == WearHolsterL)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s left holster.");
                                sprintf(buf2,
                                        "You holster $p in your left holster.");
                        }
                        else if (holster1->wear_loc == WearHolsterR)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s right holster.");
                                sprintf(buf2,
                                        "You holster $p in your right holster.");
                        }
                        else
                        {
                                sprintf(buf, "$n holsters $p in $s holster.");
                                sprintf(buf2, "You holster $p in a holster.");
                        }
                        act(AtAction, buf, ch, wield1, holster1, ToRoom);
                        act(AtAction, buf2, ch, wield1, holster1, ToChar);
                }
                else if (holster2 && !holster2->first_content)
                {
                        unequip_char(ch, wield1);
                        obj_from_char(wield1);
                        wield1 = obj_to_obj(wield1, holster2);
                        separate_obj(wield1);
                        act(AtAction, "$n holsters $p in $s right holster.",
                            ch, wield1, holster2, ToRoom);
                        act(AtAction,
                            "You holster $p in your right holster.", ch,
                            wield1, holster2, ToRoom);
                }
                else
                {
                        send_to_char("Your holster(s) are full!\n\r", ch);
                        return;
                }
        }

        if (wield2 != NULL)
        {
                char      buf[MaxStringLength], buf2[MaxStringLength];

                if (!holster1->first_content)
                {
                        unequip_char(ch, wield2);
                        obj_from_char(wield2);
                        wield2 = obj_to_obj(wield2, holster1);
                        separate_obj(wield2);
                        if (holster1->wear_loc == WearHolsterL)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s left holster.\n\r");
                                sprintf(buf2,
                                        "You holster $p in your left holster.\n\r");
                        }
                        else if (holster1->wear_loc == WearHolsterR)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s right holster.\n\r");
                                sprintf(buf2,
                                        "You holster $p in your right holster.\n\r");
                        }
                        else
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s holster.\n\r");
                                sprintf(buf2,
                                        "You holster $p in a holster.\n\r");
                        }
                        act(AtAction, buf, ch, wield2, holster1, ToRoom);
                        act(AtAction, buf2, ch, wield2, holster1, ToChar);
                }
                else if (holster2 && !holster2->first_content)
                {
                        unequip_char(ch, wield2);
                        obj_from_char(wield2);
                        wield2 = obj_to_obj(wield2, holster2);
                        separate_obj(wield2);
                        act(AtAction, "$n holsters $p in $s right holster.",
                            ch, wield2, holster2, ToRoom);
                        act(AtAction,
                            "You holster $p in your right holster.", ch,
                            wield2, holster2, ToChar);
                }
                else
                {
                        unequip_char(ch, wield2);
                        equip_char(ch, wield2, WearWield);
                        send_to_char("Your holster(s) are full!\n\r", ch);
                        return;
                }
        }

        return;
}
