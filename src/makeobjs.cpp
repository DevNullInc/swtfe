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
 * Object creation and management system for generating and handling game items. *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"


/*
 * Make a fire.
 */
void make_fire(RoomIndexData * in_room, sh_int timer)
{
        ObjData *fire;

        fire = create_object(get_obj_index(ObjVnumFire), 0);
        fire->timer = number_fuzzy(timer);
        obj_to_room(fire, in_room);
        return;
}

/*
 * Make a trap.
 */
ObjData *make_trap(int v0, int v1, int v2, int v3)
{
        ObjData *trap;

        trap = create_object(get_obj_index(ObjVnumTrap), 0);
        trap->timer = 0;
        trap->value[0] = v0;
        trap->value[1] = v1;
        trap->value[2] = v2;
        trap->value[3] = v3;
        return trap;
}


/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps(ObjData * obj)
{
        ObjData *scraps, *tmpobj;
        CharData *ch = NULL;

        separate_obj(obj);
        scraps = create_object(get_obj_index(ObjVnumScraps), 0);
        scraps->timer = number_range(5, 15);

        /*
         * don't make scraps of scraps of scraps of ... 
         */
        if (obj->pIndexData->vnum == ObjVnumScraps)
        {
                STRFREE(scraps->short_descr);
                scraps->short_descr = STRALLOC("some debris");
                STRFREE(scraps->description);
                scraps->description =
                        STRALLOC("Bits of debris lie on the ground here.");
        }
        else
        {
                stralloc_printf(&scraps->short_descr, scraps->short_descr,
                                obj->short_descr);
                stralloc_printf(&scraps->description, scraps->description,
                                obj->short_descr);
        }

        if (obj->carried_by)
        {
                act(AtObject, "$p falls to the ground in scraps!",
                    obj->carried_by, obj, NULL, ToChar);
                if (obj == get_eq_char(obj->carried_by, WearWield)
                    && (tmpobj =
                        get_eq_char(obj->carried_by,
                                    WearDualWield)) != NULL)
                        tmpobj->wear_loc = WearWield;

                obj_to_room(scraps, obj->carried_by->in_room);
        }
        else if (obj->in_room)
        {
                if ((ch = obj->in_room->first_person) != NULL)
                {
                        act(AtObject,
                            "$p is reduced to little more than scraps.", ch,
                            obj, NULL, ToRoom);
                        act(AtObject,
                            "$p is reduced to little more than scraps.", ch,
                            obj, NULL, ToChar);
                }
                obj_to_room(scraps, obj->in_room);
        }
        if ((obj->item_type == ItemContainer
             || obj->item_type == ItemHolster
             || obj->item_type == ItemCorpsePc) && obj->first_content)
        {
                if (ch && ch->in_room)
                {
                        act(AtObject,
                            "The contents of $p fall to the ground.", ch, obj,
                            NULL, ToRoom);
                        act(AtObject,
                            "The contents of $p fall to the ground.", ch, obj,
                            NULL, ToChar);
                }
                if (obj->carried_by)
                        empty_obj(obj, NULL, obj->carried_by->in_room);
                else if (obj->in_room)
                        empty_obj(obj, NULL, obj->in_room);
                else if (obj->in_obj)
                        empty_obj(obj, obj->in_obj, NULL);
        }
        extract_obj(obj);
}


/*
 * Make a corpse out of a character.
 */
void make_corpse(CharData * ch, CharData * killer)
{
        ObjData *corpse;
        ObjData *obj;
        ObjData *obj_next;
        char     *name;

        if (IsNpc(ch))
        {
                name = ch->short_descr;
                if (IsSet(ch->act, ActDroid))
                        corpse = create_object(get_obj_index
                                               (ObjVnumDroidCorpse), 0);
                else
                        corpse = create_object(get_obj_index
                                               (ObjVnumCorpseNpc), 0);
                corpse->timer = 6;
                if (ch->gold > 0)
                {
                        if (ch->in_room)
                                ch->in_room->area->gold_looted += ch->gold;
                        obj_to_obj(create_money(ch->gold), corpse);
                        ch->gold = 0;
                }

                /*
                 * Using corpse cost to cheat, since corpses not sellable 
                 */
                corpse->cost = (-(int) ch->pIndexData->vnum);
                corpse->value[2] = corpse->timer;
        }
        else
        {
                name = ch->name;
                corpse = create_object(get_obj_index(ObjVnumCorpsePc), 0);
                corpse->timer = 40;
                corpse->value[2] = (int) (corpse->timer / 8);
                corpse->value[3] = 0;
                if (ch->gold > 0)
                {
                        if (ch->in_room)
                                ch->in_room->area->gold_looted += ch->gold;
                        obj_to_obj(create_money(ch->gold), corpse);
                        ch->gold = 0;
                }
        }
        if (killer != NULL)
        {
                STRFREE(corpse->armed_by);
                corpse->armed_by = STRALLOC(killer->name);
        }
        /*
         * Added corpse name - make locate easier , other skills 
         */
        stralloc_printf(&corpse->name, "corpse %s", name);
        stralloc_printf(&corpse->short_descr, corpse->short_descr, name);
        stralloc_printf(&corpse->description, corpse->description, name);

        for (obj = ch->first_carrying; obj; obj = obj_next)
        {
                obj_next = obj->next_content;
                obj_from_char(obj);
                if (IsObjStat(obj, ItemInventory)
                    || IsObjStat(obj, ItemDeathrot))
                        extract_obj(obj);
                else
                        obj_to_obj(obj, corpse);
        }
        obj_to_room(corpse, ch->in_room);
        return;
}



void make_blood(CharData * ch)
{
        ObjData *obj;

        obj = create_object(get_obj_index(ObjVnumBlood), 0);
        obj->timer = number_range(2, 4);
        obj->value[1] = number_range(3, UMIN(5, ch->top_level));
        obj_to_room(obj, ch->in_room);
}


void make_bloodstain(CharData * ch)
{
        ObjData *obj;

        obj = create_object(get_obj_index(ObjVnumBloodstain), 0);
        obj->timer = number_range(1, 2);
        obj_to_room(obj, ch->in_room);
}


/*
 * make some coinage
 */
ObjData *create_money(int amount)
{
        ObjData *obj;

        if (amount <= 0)
        {
                bug("Create_money: zero or negative money %d.", amount);
                amount = 1;
        }

        if (amount == 1)
        {
                obj = create_object(get_obj_index(ObjVnumMoneyOne), 0);
        }
        else
        {
                obj = create_object(get_obj_index(ObjVnumMoneySome), 0);
                stralloc_printf(&obj->short_descr, obj->short_descr, amount);
                obj->value[0] = amount;
        }

        return obj;
}
