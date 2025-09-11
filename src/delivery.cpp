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
 *                                   Delivery Module                                     *
 ****************************************************************************************/



#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"


void fwrite_delivery(CharData * ch, ObjData * obj)
{
        FILE     *fp = NULL;
        char      strsave[MaxInputLength];
        ExtraDescrData *ed;
        AffectData *paf;
        sh_int    wear, wear_loc, x;

        if (!obj)
        {
                bug("Fwrite_delivery: NULL object.", 0);
                bug(ch->name, 0);
                return;
        }

        snprintf(strsave, MSL, "%s%s", MailDir, capitalize(ch->name));

        if ((fp = fopen(strsave, "w")) != NULL)
        {

                fprintf(fp, "#OBJECT\n");

                if (obj->count > 1)
                        fprintf(fp, "Count        %d\n", obj->count);
                if (QUICKMATCH(obj->name, obj->pIndexData->name) == 0)
                        fprintf(fp, "Name         %s~\n", obj->name);
                if (QUICKMATCH(obj->short_descr, obj->pIndexData->short_descr)
                    == 0)
                        fprintf(fp, "ShortDescr   %s~\n", obj->short_descr);
                if (QUICKMATCH(obj->description, obj->pIndexData->description)
                    == 0)
                        fprintf(fp, "Description  %s~\n", obj->description);
                if (QUICKMATCH(obj->action_desc, obj->pIndexData->action_desc)
                    == 0)
                        fprintf(fp, "ActionDesc   %s~\n", obj->action_desc);
                fprintf(fp, "Vnum         %d\n", obj->pIndexData->vnum);
                if (obj->extra_flags != obj->pIndexData->extra_flags)
                        fprintf(fp, "ExtraFlags   %d\n", obj->extra_flags);
                if (obj->wear_flags != obj->pIndexData->wear_flags)
                        fprintf(fp, "WearFlags    %d\n", obj->wear_flags);
                wear_loc = -1;
                for (wear = 0; wear < MaxWear; wear++)
                        for (x = 0; x < MaxLayers; x++)
                                if (obj == save_equipment[wear][x])
                                {
                                        wear_loc = wear;
                                        break;
                                }
                                else if (!save_equipment[wear][x])
                                        break;
                if (wear_loc != -1)
                        fprintf(fp, "WearLoc      %d\n", wear_loc);
                if (obj->item_type != obj->pIndexData->item_type)
                        fprintf(fp, "ItemType     %d\n", obj->item_type);
                if (obj->weight != obj->pIndexData->weight)
                        fprintf(fp, "Weight       %d\n", obj->weight);
                if (obj->level)
                        fprintf(fp, "Level        %d\n", obj->level);
                if (obj->timer)
                        fprintf(fp, "Timer        %d\n", obj->timer);
                if (obj->cost != obj->pIndexData->cost)
                        fprintf(fp, "Cost         %d\n", obj->cost);
                if (obj->value[0] || obj->value[1] || obj->value[2]
                    || obj->value[3] || obj->value[4] || obj->value[5])
                        fprintf(fp, "Values       %d %d %d %d %d %d\n",
                                obj->value[0], obj->value[1], obj->value[2],
                                obj->value[3], obj->value[4], obj->value[5]);

                switch (obj->item_type)
                {
                case ItemPill:    /* was down there with staff and wand, wrongly - Scryn */
                case ItemPotion:
                case ItemScroll:
                        if (IsValidSn(obj->value[1]))
                                fprintf(fp, "Spell 1      '%s'\n",
                                        skill_table[obj->value[1]]->name);

                        if (IsValidSn(obj->value[2]))
                                fprintf(fp, "Spell 2      '%s'\n",
                                        skill_table[obj->value[2]]->name);

                        if (IsValidSn(obj->value[3]))
                                fprintf(fp, "Spell 3      '%s'\n",
                                        skill_table[obj->value[3]]->name);

                        break;

                case ItemStaff:
                case ItemWand:
                        if (IsValidSn(obj->value[3]))
                                fprintf(fp, "Spell 3      '%s'\n",
                                        skill_table[obj->value[3]]->name);

                        break;
                case ItemSalve:
                        if (IsValidSn(obj->value[4]))
                                fprintf(fp, "Spell 4      '%s'\n",
                                        skill_table[obj->value[4]]->name);

                        break;
                }

                for (paf = obj->first_affect; paf; paf = paf->next)
                {
                        /*
                         * Save extra object affects                -Thoric
                         */
                        if (paf->type < 0 || paf->type >= top_sn)
                        {
                                fprintf(fp, "Affect       %d %d %d %d %d\n",
                                        paf->type,
                                        paf->duration,
                                        ((paf->location == ApplyWeaponspell
                                          || paf->location == ApplyWearspell
                                          || paf->location ==
                                          ApplyRemovespell
                                          || paf->location == ApplyStripsn)
                                         && IsValidSn(paf->
                                                        modifier)) ?
                                        skill_table[paf->modifier]->
                                        slot : paf->modifier, paf->location,
                                        paf->bitvector);
                        }
                        else
                                fprintf(fp, "AffectData   '%s' %d %d %d %d\n",
                                        skill_table[paf->type]->name,
                                        paf->duration,
                                        ((paf->location == ApplyWeaponspell
                                          || paf->location == ApplyWearspell
                                          || paf->location ==
                                          ApplyRemovespell
                                          || paf->location == ApplyStripsn)
                                         && IsValidSn(paf->
                                                        modifier)) ?
                                        skill_table[paf->modifier]->
                                        slot : paf->modifier, paf->location,
                                        paf->bitvector);
                }

                for (ed = obj->first_extradesc; ed; ed = ed->next)
                        fprintf(fp, "ExtraDescr   %s~ %s~\n",
                                ed->keyword, ed->description);


                fprintf(fp, "End\n\n");

                if (obj->first_content)
                        fwrite_obj(ch, obj->last_content, fp, 0, OsCarry,
                                   FALSE);

        }
        fprintf(fp, "#END \n\r");
        FCLOSE(fp);
        return;
}



CMDF do_deliver(CharData * ch, char *argument)
{
        ObjData *obj;
        char      arg[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        char      strsave[MaxInputLength];
        char      mobbuf[MaxInputLength];
        CharData *victim;
        CharData *mob;
        FILE     *fp = NULL;
        int       cost;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (IsNpc(ch))
                return;

        for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room)
                if (IsNpc(mob) && IsSet(mob->act, ActMail))
                        break;

        if (!mob)
        {
                send_to_char("You need to visit the post office.\n\r", ch);
                return;
        }



        if (arg[0] == '\0')
        {
                send_to_char("Deliver what to who?\n\r", ch);
                send_to_char("Syntax: deliver <object> <player>\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "pickup"))
        {
                snprintf(strsave, MSL, "%s%s", MailDir,
                         capitalize(ch->name));

                if ((fp = fopen(strsave, "r")) != NULL)
                {
                        for (;;)
                        {
                                char      letter;
                                char     *word;

                                letter = fread_letter(fp);
                                if (letter == '#')
                                {
                                        word = fread_word(fp);

                                        if (!strcmp(word, "END"))
                                                break;

                                        if (!strcmp(word, "OBJECT"))
                                                fread_obj(ch, fp, OsCarry);
                                }
                        }
                        FCLOSE(fp);
                        if (!remove(strsave))
                        {
                                send_to_char("You retrive your delivery.\n\r",
                                             ch);
                                RemoveBit(ch->PCData->flags, PcflagGotmail);
                        }
                        else if (errno != ENOENT)
                                bug("Delivery unable to delete mailbox.\n\r",
                                    0);
                        return;
                }
                else
                {
                        send_to_char("You currently have no deliveries.\n\r",
                                     ch);
                        return;
                }
        }
        if (ms_find_obj(ch))
                return;

        if (!(obj = get_obj_carry(ch, arg)))
        {
                send_to_char("You do not have that item.\n\r", ch);
                return;
        }
        separate_obj(obj);
        if (IsObjStat(obj, ItemNodrop) || IsObjStat(obj, ItemPrototype))
        {
                snprintf(mobbuf, MSL,
                         "I'm sorry we dont handle objects of that nature.");
                do_say(mob, mobbuf);
                return;
        }

        if ((victim = get_char_world(ch, arg2)) == NULL || IsNpc(victim))
        {
                snprintf(mobbuf, MSL, "%s",
                         "They're not here right now... let me check their mailbox.");
                do_say(mob, mobbuf);
                snprintf(strsave, MSL, "%s%c/%s", PlayerDir,
                         tolower(arg2[0]), capitalize(arg2));
                if ((fp = fopen(strsave, "r")) != NULL);
                else
                {
                        snprintf(mobbuf, MSL,
                                 "I dont have a mailbox for %s... Are you sure thats the correct name?",
                                 capitalize(arg2));
                        do_say(mob, mobbuf);
                        return;
                }
                snprintf(strsave, MSL, "%s%s", MailDir, capitalize(arg2));
                if ((fp = fopen(strsave, "r")) != NULL)
                {
                        FCLOSE(fp);
                        snprintf(mobbuf, MSL, "%s",
                                 "Oh... I'm sorry, But that mailbox is already full. Anything else i can do for you?");
                        do_say(mob, mobbuf);
                        return;
                }
                else
                {
                        cost = obj->level * 300;

                        if (ch->gold - cost < 0)
                        {
                                send_to_char
                                        ("You cant afford to send that to them.\n\r",
                                         ch);
                                return;
                        }
                        ch->gold -= cost;
                        if ((fp = fopen(strsave, "w")) != NULL)
                        {
                                ExtraDescrData *ed;
                                AffectData *paf;
                                sh_int    wear, wear_loc, x;

                                /*
                                 * Yes its redundant... But it works 
                                 */
                                fprintf(fp, "#OBJECT\n");

                                if (obj->count > 1)
                                        fprintf(fp, "Count        %d\n",
                                                obj->count);
                                if (QUICKMATCH
                                    (obj->name, obj->pIndexData->name) == 0)
                                        fprintf(fp, "Name         %s~\n",
                                                obj->name);
                                if (QUICKMATCH
                                    (obj->short_descr,
                                     obj->pIndexData->short_descr) == 0)
                                        fprintf(fp, "ShortDescr   %s~\n",
                                                obj->short_descr);
                                if (QUICKMATCH
                                    (obj->description,
                                     obj->pIndexData->description) == 0)
                                        fprintf(fp, "Description  %s~\n",
                                                obj->description);
                                if (QUICKMATCH
                                    (obj->action_desc,
                                     obj->pIndexData->action_desc) == 0)
                                        fprintf(fp, "ActionDesc   %s~\n",
                                                obj->action_desc);
                                fprintf(fp, "Vnum         %d\n",
                                        obj->pIndexData->vnum);
                                if (obj->extra_flags !=
                                    obj->pIndexData->extra_flags)
                                        fprintf(fp, "ExtraFlags   %d\n",
                                                obj->extra_flags);
                                if (obj->wear_flags !=
                                    obj->pIndexData->wear_flags)
                                        fprintf(fp, "WearFlags    %d\n",
                                                obj->wear_flags);
                                wear_loc = -1;
                                for (wear = 0; wear < MaxWear; wear++)
                                        for (x = 0; x < MaxLayers; x++)
                                                if (obj ==
                                                    save_equipment[wear][x])
                                                {
                                                        wear_loc = wear;
                                                        break;
                                                }
                                                else if (!save_equipment[wear]
                                                         [x])
                                                        break;
                                if (wear_loc != -1)
                                        fprintf(fp, "WearLoc      %d\n",
                                                wear_loc);
                                if (obj->item_type !=
                                    obj->pIndexData->item_type)
                                        fprintf(fp, "ItemType     %d\n",
                                                obj->item_type);
                                if (obj->weight != obj->pIndexData->weight)
                                        fprintf(fp, "Weight       %d\n",
                                                obj->weight);
                                if (obj->level)
                                        fprintf(fp, "Level        %d\n",
                                                obj->level);
                                if (obj->timer)
                                        fprintf(fp, "Timer        %d\n",
                                                obj->timer);
                                if (obj->cost != obj->pIndexData->cost)
                                        fprintf(fp, "Cost         %d\n",
                                                obj->cost);
                                if (obj->value[0] || obj->value[1]
                                    || obj->value[2] || obj->value[3]
                                    || obj->value[4] || obj->value[5])
                                        fprintf(fp,
                                                "Values       %d %d %d %d %d %d\n",
                                                obj->value[0], obj->value[1],
                                                obj->value[2], obj->value[3],
                                                obj->value[4], obj->value[5]);

                                switch (obj->item_type)
                                {
                                case ItemPill:    /* was down there with staff and wand, wrongly - Scryn */
                                case ItemPotion:
                                case ItemScroll:
                                        if (IsValidSn(obj->value[1]))
                                                fprintf(fp,
                                                        "Spell 1      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [1]]->
                                                        name);

                                        if (IsValidSn(obj->value[2]))
                                                fprintf(fp,
                                                        "Spell 2      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [2]]->
                                                        name);

                                        if (IsValidSn(obj->value[3]))
                                                fprintf(fp,
                                                        "Spell 3      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [3]]->
                                                        name);

                                        break;

                                case ItemStaff:
                                case ItemWand:
                                        if (IsValidSn(obj->value[3]))
                                                fprintf(fp,
                                                        "Spell 3      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [3]]->
                                                        name);

                                        break;
                                case ItemSalve:
                                        if (IsValidSn(obj->value[4]))
                                                fprintf(fp,
                                                        "Spell 4      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [4]]->
                                                        name);

                                        break;
                                }

                                for (paf = obj->first_affect; paf;
                                     paf = paf->next)
                                {
                                        /*
                                         * Save extra object affects                -Thoric
                                         */
                                        if (paf->type < 0
                                            || paf->type >= top_sn)
                                        {
                                                fprintf(fp,
                                                        "Affect       %d %d %d %d %d\n",
                                                        paf->type,
                                                        paf->duration,
                                                        ((paf->location ==
                                                          ApplyWeaponspell
                                                          || paf->location ==
                                                          ApplyWearspell
                                                          || paf->location ==
                                                          ApplyRemovespell
                                                          || paf->location ==
                                                          ApplyStripsn)
                                                         && IsValidSn(paf->
                                                                        modifier))
                                                        ? skill_table[paf->
                                                                      modifier]->
                                                        slot : paf->modifier,
                                                        paf->location,
                                                        paf->bitvector);
                                        }
                                        else
                                                fprintf(fp,
                                                        "AffectData   '%s' %d %d %d %d\n",
                                                        skill_table[paf->
                                                                    type]->
                                                        name, paf->duration,
                                                        ((paf->location ==
                                                          ApplyWeaponspell
                                                          || paf->location ==
                                                          ApplyWearspell
                                                          || paf->location ==
                                                          ApplyRemovespell
                                                          || paf->location ==
                                                          ApplyStripsn)
                                                         && IsValidSn(paf->
                                                                        modifier))
                                                        ? skill_table[paf->
                                                                      modifier]->
                                                        slot : paf->modifier,
                                                        paf->location,
                                                        paf->bitvector);
                                }

                                for (ed = obj->first_extradesc; ed;
                                     ed = ed->next)
                                        fprintf(fp, "ExtraDescr   %s~ %s~\n",
                                                ed->keyword, ed->description);

                                fprintf(fp, "End\n\n");

                                if (obj->first_content)
                                        fwrite_obj(ch, obj->last_content, fp,
                                                   0, OsCarry, FALSE);

                        }
                        fprintf(fp, "#END \n\r");
                        FCLOSE(fp);
                }
                snprintf(mobbuf, MSL,
                         "Ok, I've put it in their mailbox for them. Thanks and may I assist you further %s?.",
                         ch->name);
                do_say(mob, mobbuf);
                extract_obj(obj);
                return;

        }
        if (!str_cmp(arg3, "express"))
        {
                cost = obj->level * 400;

                if (victim == ch)
                {
                        snprintf(mobbuf, MSL,
                                 "I'm not gonna waste my time delivering to you FROM you %s!",
                                 ch->name);
                        do_say(mob, mobbuf);
                        return;
                }

                if (ch->gold - cost < 0)
                {
                        send_to_char
                                ("You cant afford to send that to them.\n\r",
                                 ch);
                        return;
                }
                ch->gold -= cost;
                separate_obj(obj);
                obj_from_char(obj);
                obj_to_char(obj, victim);
                act(AtMagic,
                    "You give the postmaster $p to express deliver to $N", ch,
                    obj, victim, ToChar);
                snprintf(mobbuf, MSL, "Thanks %s! It's on its way to %s.",
                         ch->name, victim->name);
                do_say(mob, mobbuf);
                act(AtMagic,
                    "A courier materliazes and hands you $p from $n.", ch,
                    obj, victim, ToVict);
                save_char_obj(victim);
                save_char_obj(ch);
                return;
        }
        cost = obj->level * 200;

        if (ch->gold - cost < 0)
        {
                send_to_char("You cant afford to send that to them.\n\r", ch);
                return;
        }
        snprintf(strsave, MSL, "%s%s", MailDir, capitalize(victim->name));
        if ((fp = fopen(strsave, "r")) != NULL)
        {
                send_to_char("Their mailbox is already full.\n\r", ch);
                FCLOSE(fp);
                return;
        }

        ch->gold -= cost;
        act(AtMagic, "You give the postmaster $p to deliver to $N", ch, obj,
            victim, ToChar);
        fwrite_delivery(victim, obj);
        extract_obj(obj);
        snprintf(mobbuf, MSL,
                 "Thanks %s! I've put your package in %s's box and sent a memo to them to pickup.",
                 ch->name, victim->name);
        do_say(mob, mobbuf);
        act(AtMagic,
            "$n has sent you $p in the mail, and it is awaiting your pickup at the nearest post office.",
            ch, obj, victim, ToVict);
        SetBit(victim->PCData->flags, PcflagGotmail);
        return;
}
