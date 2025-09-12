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
 *                                Object scripting module                                *
 ****************************************************************************************/
// ============================================================================
// System Headers
// ============================================================================
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Local Headers  
// ============================================================================
#include "mud.hpp"
#include "bet.hpp"
#include "races.hpp"
#include "olc_bounty.hpp"

// ============================================================================
// Constants and Configuration
// ============================================================================
namespace {
    // Object resistance and durability constants
    constexpr int MagicItemResistanceBonus = 12;
    constexpr int BlessedItemResistanceBonus = 5;
    constexpr int InventoryItemResistanceBonus = 20;
    constexpr int LevelResistanceDivisor = 10;
    constexpr int MinResistance = 10;
    constexpr int MaxResistance = 99;
    
    // Bury and dig constants
    constexpr int MinBuryWeight = 5;
    constexpr int BuryWeightDivisor = 10;
    constexpr int BuryMoveMultiplier = 50;
    constexpr int ShovelMultiplier = 1;
    constexpr int NoShovelMultiplier = 5;
    constexpr int MinBuryMove = 2;
    constexpr int MaxBuryMove = 1000;
    constexpr int MinBuryWait = 10;
    constexpr int MaxBuryWait = 100;
    constexpr int BuryWaitDivisor = 2;
    
    // String parsing constants
    constexpr int PrefixLength = 4; // Length of "all." prefix
}

// ============================================================================
// Function Prototypes
// ============================================================================
// ============================================================================
// Function Prototypes
// ============================================================================
void get_obj args((CharData * ch, ObjData * obj, ObjData * container));
bool remove_obj args((CharData * ch, int iWear, bool fReplace));
void      wear_obj
args((CharData * ch, ObjData * obj, bool fReplace, sh_int wear_bit));
bool      could_dual(CharData * ch);
bool      can_dual(CharData * ch);
bool      can_layer(CharData * ch, ObjData * obj, sh_int wear_loc);

// ============================================================================
// Object Resistance and Durability Functions
// ============================================================================

/*
 * how resistant an object is to damage				-Thoric
 */
sh_int get_obj_resistance(ObjData * obj)
{
        sh_int    resist;

        resist = static_cast<sh_int>(number_fuzzy(MaxItemImpact));

        /*
         * magical items are more resistant 
         */
        if (IsObjStat(obj, ItemMagic))
                resist = static_cast<sh_int>(resist + number_fuzzy(MagicItemResistanceBonus));
        /*
         * blessed objects should have a little Bonus 
         */
        if (IsObjStat(obj, ItemBless))
                resist = static_cast<sh_int>(resist + number_fuzzy(BlessedItemResistanceBonus));
        /*
         * lets make store inventory pretty tough 
         */
        if (IsObjStat(obj, ItemInventory))
                resist = static_cast<sh_int>(resist + InventoryItemResistanceBonus);

        /*
         * okay... let's add some Bonus/penalty for item level... 
         */
        resist = static_cast<sh_int>(resist + (obj->level / LevelResistanceDivisor));

        /*
         * and lasty... take Armor or weapon's condition into consideration 
         */
        if (obj->item_type == ItemArmor || obj->item_type == ItemWeapon)
                resist = static_cast<sh_int>(resist + obj->value[0]);

        return static_cast<sh_int>(URange(MinResistance, resist, MaxResistance));
}

// ============================================================================
// Object Transfer and Manipulation Functions
// ============================================================================

void get_obj(CharData * ch, ObjData * obj, ObjData * container)
{
        ClanData *clan;
        int       weight;

        if (IsSet(ch->affected_by, AffRestrained))
        {
                send_to_char
                        ("How do you expect to do that while restrained?\n\r",
                         ch);
                return;
        }

        if (!CanWear(obj, ItemTake)
            && (ch->top_level < sysdata.level_getobjnotake))
        {
                send_to_char("You can't take that.\n\r", ch);
                return;
        }

        if (IsObjStat(obj, ItemPrototype) && !can_take_proto(ch))
        {
                send_to_char
                        ("A godly Force prevents you from getting close to it.\n\r",
                         ch);
                return;
        }

        if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch))
        {
                act(AtPlain, "$d: you can't carry that many items.",
                    ch, NULL, obj->name, ToChar);
                return;
        }

        if (IsObjStat(obj, ItemCovering))
                weight = obj->weight;
        else
                weight = get_obj_weight(obj);

        if (ch->carry_weight + weight > can_carry_w(ch))
        {
                act(AtPlain, "$d: you can't carry that much weight.",
                    ch, NULL, obj->name, ToChar);
                return;
        }

        if (container)
        {
                act(AtAction, IsObjStat(container, ItemCovering) ?
                    "You get $p from beneath $P." : "You get $p from $P",
                    ch, obj, container, ToChar);
                act(AtAction, IsObjStat(container, ItemCovering) ?
                    "$n gets $p from beneath $P." : "$n gets $p from $P",
                    ch, obj, container, ToRoom);
                obj_from_obj(obj);
        }
        else
        {
                act(AtAction, "You get $p.", ch, obj, container, ToChar);
                act(AtAction, "$n gets $p.", ch, obj, container, ToRoom);
                obj_from_room(obj);
        }

        /*
         * Clan storeroom checks 
         */
        if (IsSet(ch->in_room->RoomFlags, RoomClanstoreroom)
            && (!container || container->carried_by == NULL))
                for (clan = first_clan; clan; clan = clan->next)
                        if (clan->storeroom == ch->in_room->vnum)
                                save_clan_storeroom(ch, clan);

        if (obj->item_type != ItemContainer)
                check_for_trap(ch, obj, TrapGet);
        if (char_died(ch))
                return;

        if (obj->item_type == ItemMoney)
        {
                ch->gold += obj->value[0];
                extract_obj(obj);
        }
        else
        {
                obj = obj_to_char(obj, ch);
                if (obj->item_type == ItemBeacon)
                        obj->value[0] = 0;
        }

        if (char_died(ch) || obj_extracted(obj))
                return;
        oprog_get_trigger(ch, obj);
        return;
}

// ============================================================================
// Object Retrieval Commands
// ============================================================================

CMDF do_get(CharData * ch, const char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      mutable_argument[MaxInputLength];
        ObjData *obj;
        ObjData *obj_next;
        ObjData *container;
        sh_int    number;
        bool      found;

        strcpy(mutable_argument, argument);
        char *arg_ptr = one_argument(mutable_argument, arg1);
        if (is_number(arg1))
        {
                number = static_cast<sh_int>(atoi(arg1));
                if (number < 1)
                {
                        send_to_char("That was easy...\n\r", ch);
                        return;
                }
                if ((ch->carry_number + number) > can_carry_n(ch))
                {
                        send_to_char("You can't carry that many.\n\r", ch);
                        return;
                }
                arg_ptr = one_argument(mutable_argument, arg1);
                strcpy(mutable_argument, arg_ptr);
        }
        else
                number = 0;
        arg_ptr = one_argument(mutable_argument, arg2);
        strcpy(mutable_argument, arg_ptr);
        /*
         * munch optional words 
         */
        if (!str_cmp(arg2, "from") && mutable_argument[0] != '\0')
        {
                arg_ptr = one_argument(mutable_argument, arg2);
                strcpy(mutable_argument, arg_ptr);
        }

        /*
         * Get type. 
         */
        if (arg1[0] == '\0')
        {
                send_to_char("Get what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (arg2[0] == '\0')
        {
                if (number <= 1 && str_cmp(arg1, "all")
                    && str_prefix("all.", arg1))
                {
                        /*
                         * 'get obj' 
                         */
                        obj = get_obj_list(ch, arg1,
                                           ch->in_room->first_content);
                        if (!obj)
                        {
                                act(AtPlain, "I see no $T here.", ch, NULL,
                                    arg1, ToChar);
                                return;
                        }
                        separate_obj(obj);
                        get_obj(ch, obj, NULL);
                        if (char_died(ch))
                                return;
                        if (IsSet(sysdata.save_flags, SvGet))
                                save_char_obj(ch);
                }
                else
                {
                        sh_int    cnt = 0;
                        bool      fAll;
                        char     *chk;

                        if (IsSet(ch->in_room->RoomFlags, RoomDonation))
                        {
                                send_to_char
                                        ("The gods frown upon such a display of greed!\n\r",
                                         ch);
                                return;
                        }
                        if (!str_cmp(arg1, "all"))
                                fAll = TRUE;
                        else
                                fAll = FALSE;
                        if (number > 1)
                                chk = arg1;
                        else
                                chk = &arg1[PrefixLength];
                        /*
                         * 'get all' or 'get all.obj' 
                         */
                        found = FALSE;
                        for (obj = ch->in_room->first_content; obj;
                             obj = obj_next)
                        {
                                obj_next = obj->next_content;
                                if ((fAll || nifty_is_name(chk, obj->name))
                                    && can_see_obj(ch, obj))
                                {
                                        found = TRUE;
                                        if (number
                                            && (cnt + obj->count) > number)
                                                split_obj(obj, number - cnt);
                                        cnt += obj->count;
                                        get_obj(ch, obj, NULL);
                                        if (char_died(ch)
                                            || ch->carry_number >=
                                            can_carry_n(ch)
                                            || ch->carry_weight >=
                                            can_carry_w(ch) || (number
                                                                && cnt >=
                                                                number))
                                        {
                                                if (IsSet
                                                    (sysdata.save_flags,
                                                     SvGet)
                                                    && !char_died(ch))
                                                        save_char_obj(ch);
                                                return;
                                        }
                                }
                        }
                        if (!found)
                        {
                                if (fAll)
                                        send_to_char
                                                ("I see nothing here.\n\r",
                                                 ch);
                                else
                                        act(AtPlain, "I see no $T here.", ch,
                                            NULL, chk, ToChar);
                        }
                        else if (IsSet(sysdata.save_flags, SvGet))
                                save_char_obj(ch);
                }
        }
        else
        {
                /*
                 * 'get ... container' 
                 */
                if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }

                if ((container = get_obj_here(ch, arg2)) == NULL)
                {
                        act(AtPlain, "I see no $T here.", ch, NULL, arg2,
                            ToChar);
                        return;
                }

                switch (container->item_type)
                {
                default:
                        if (!IsObjStat(container, ItemCovering))
                        {
                                send_to_char("That's not a container.\n\r",
                                             ch);
                                return;
                        }
                        if (ch->carry_weight + container->weight >
                            can_carry_w(ch))
                        {
                                send_to_char
                                        ("It's too heavy for you to lift.\n\r",
                                         ch);
                                return;
                        }
                        break;

                case ItemContainer:
                case ItemDroidCorpse:
                case ItemCorpsePc:
                case ItemCorpseNpc:
                        break;
                }

                if (!IsObjStat(container, ItemCovering)
                    && IsSet(container->value[1], ContClosed))
                {
                        act(AtPlain, "The $d is closed.", ch, NULL,
                            container->name, ToChar);
                        return;
                }

                if (number <= 1 && str_cmp(arg1, "all")
                    && str_prefix("all.", arg1))
                {
                        /*
                         * 'get obj container' 
                         */
                        obj = get_obj_list(ch, arg1,
                                           container->first_content);
                        if (!obj)
                        {
                                act(AtPlain,
                                    IsObjStat(container,
                                                ItemCovering) ?
                                    "I see nothing like that beneath the $T."
                                    : "I see nothing like that in the $T.",
                                    ch, NULL, arg2, ToChar);
                                return;
                        }
                        separate_obj(obj);
                        get_obj(ch, obj, container);

                        check_for_trap(ch, container, TrapGet);
                        if (char_died(ch))
                                return;
                        if (IsSet(sysdata.save_flags, SvGet))
                                save_char_obj(ch);
                }
                else
                {
                        int       cnt = 0;
                        bool      fAll;
                        char     *chk;

                        /*
                         * 'get all container' or 'get all.obj container' 
                         */
                        if (IsObjStat(container, ItemDonation))
                        {
                                send_to_char
                                        ("The gods frown upon such an act of greed!\n\r",
                                         ch);
                                return;
                        }
                        if (!str_cmp(arg1, "all"))
                                fAll = TRUE;
                        else
                                fAll = FALSE;
                        if (number > 1)
                                chk = arg1;
                        else
                                chk = &arg1[PrefixLength];
                        found = FALSE;
                        for (obj = container->first_content; obj;
                             obj = obj_next)
                        {
                                obj_next = obj->next_content;
                                if ((fAll || nifty_is_name(chk, obj->name))
                                    && can_see_obj(ch, obj))
                                {
                                        found = TRUE;
                                        if (number
                                            && (cnt + obj->count) > number)
                                                split_obj(obj, number - cnt);
                                        cnt += obj->count;
                                        get_obj(ch, obj, container);
                                        if (char_died(ch)
                                            || ch->carry_number >=
                                            can_carry_n(ch)
                                            || ch->carry_weight >=
                                            can_carry_w(ch) || (number
                                                                && cnt >=
                                                                number))
                                                return;
                                }
                        }

                        if (!found)
                        {
                                if (fAll)
                                        act(AtPlain,
                                            IsObjStat(container,
                                                        ItemCovering) ?
                                            "I see nothing beneath the $T." :
                                            "I see nothing in the $T.", ch,
                                            NULL, arg2, ToChar);
                                else
                                        act(AtPlain,
                                            IsObjStat(container,
                                                        ItemCovering) ?
                                            "I see nothing like that beneath the $T."
                                            :
                                            "I see nothing like that in the $T.",
                                            ch, NULL, arg2, ToChar);
                        }
                        else
                                check_for_trap(ch, container, TrapGet);
                        if (char_died(ch))
                                return;
                        if (found && IsSet(sysdata.save_flags, SvGet))
                                save_char_obj(ch);
                }
        }
        return;
}

// ============================================================================
// Object Storage Commands
// ============================================================================

CMDF do_put(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        ObjData *container;
        ObjData *obj;
        ObjData *obj_next;
        ClanData *clan;
        sh_int    count;
        int       number;
        bool      save_char = FALSE;

        argument = one_argument(argument, arg1);
        if (is_number(arg1))
        {
                number = atoi(arg1);
                if (number < 1)
                {
                        send_to_char("That was easy...\n\r", ch);
                        return;
                }
                argument = one_argument(argument, arg1);
        }
        else
                number = 0;
        argument = one_argument(argument, arg2);
        /*
         * munch optional words 
         */
        if ((!str_cmp(arg2, "into") || !str_cmp(arg2, "inside")
             || !str_cmp(arg2, "in")) && argument[0] != '\0')
                argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Put what in what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2))
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        if ((container = get_obj_here(ch, arg2)) == NULL)
        {
                act(AtPlain, "I see no $T here.", ch, NULL, arg2, ToChar);
                return;
        }

        if (!container->carried_by && IsSet(sysdata.save_flags, SvPut))
                save_char = TRUE;

        if (IsObjStat(container, ItemCovering))
        {
                if (ch->carry_weight + container->weight > can_carry_w(ch))
                {
                        send_to_char("It's too heavy for you to lift.\n\r",
                                     ch);
                        return;
                }
        }
        else
        {
                if (container->item_type != ItemContainer)
                {
                        send_to_char("That's not a container.\n\r", ch);
                        return;
                }

                if (IsSet(container->value[1], ContClosed))
                {
                        act(AtPlain, "The $d is closed.", ch, NULL,
                            container->name, ToChar);
                        return;
                }
        }

        if (number <= 1 && str_cmp(arg1, "all") && str_prefix("all.", arg1))
        {
                /*
                 * 'put obj container' 
                 */
                if ((obj = get_obj_carry(ch, arg1)) == NULL)
                {
                        send_to_char("You do not have that item.\n\r", ch);
                        return;
                }

                if (obj == container)
                {
                        send_to_char("You can't fold it into itself.\n\r",
                                     ch);
                        return;
                }

                if (!can_drop_obj(ch, obj))
                {
                        send_to_char("You can't let go of it.\n\r", ch);
                        return;
                }

                if ((IsObjStat(container, ItemCovering)
                     && (get_obj_weight(obj) / obj->count)
                     > ((get_obj_weight(container) / container->count)
                        - container->weight)))
                {
                        send_to_char("It won't fit under there.\n\r", ch);
                        return;
                }

                if ((get_obj_weight(obj) / obj->count)
                    + (get_obj_weight(container) / container->count)
                    > container->value[0])
                {
                        send_to_char("It won't fit.\n\r", ch);
                        return;
                }

                separate_obj(obj);
                separate_obj(container);
                obj_from_char(obj);
                obj = obj_to_obj(obj, container);
                check_for_trap(ch, container, TrapPut);
                if (char_died(ch))
                        return;
                count = obj->count;
                obj->count = 1;
                act(AtAction, IsObjStat(container, ItemCovering)
                    ? "$n hides $p beneath $P." : "$n puts $p in $P.",
                    ch, obj, container, ToRoom);
                act(AtAction, IsObjStat(container, ItemCovering)
                    ? "You hide $p beneath $P." : "You put $p in $P.",
                    ch, obj, container, ToChar);
                obj->count = count;

                if (save_char)
                        save_char_obj(ch);
                /*
                 * Clan storeroom check 
                 */
                if (IsSet(ch->in_room->RoomFlags, RoomClanstoreroom)
                    && container->carried_by == NULL)
                        for (clan = first_clan; clan; clan = clan->next)
                                if (clan->storeroom == ch->in_room->vnum)
                                        save_clan_storeroom(ch, clan);
        }
        else
        {
                bool      found = FALSE;
                int       cnt = 0;
                bool      fAll;
                char     *chk;

                if (!str_cmp(arg1, "all"))
                        fAll = TRUE;
                else
                        fAll = FALSE;
                if (number > 1)
                        chk = arg1;
                else
                        chk = &arg1[PrefixLength];

                separate_obj(container);
                /*
                 * 'put all container' or 'put all.obj container' 
                 */
                for (obj = ch->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;

                        if ((fAll || nifty_is_name(chk, obj->name))
                            && can_see_obj(ch, obj)
                            && obj->wear_loc == WearNone
                            && obj != container
                            && can_drop_obj(ch, obj)
                            && get_obj_weight(obj) + get_obj_weight(container)
                            <= container->value[0])
                        {
                                if (number && (cnt + obj->count) > number)
                                        split_obj(obj, number - cnt);
                                cnt += obj->count;
                                obj_from_char(obj);
                                act(AtAction, "$n puts $p in $P.", ch, obj,
                                    container, ToRoom);
                                act(AtAction, "You put $p in $P.", ch, obj,
                                    container, ToChar);
                                obj = obj_to_obj(obj, container);
                                found = TRUE;

                                check_for_trap(ch, container, TrapPut);
                                if (char_died(ch))
                                        return;
                                if (number && cnt >= number)
                                        break;
                        }
                }

                /*
                 * Don't bother to save anything if nothing was dropped   -Thoric
                 */
                if (!found)
                {
                        if (fAll)
                                act(AtPlain,
                                    "You are not carrying anything.", ch,
                                    NULL, NULL, ToChar);
                        else
                                act(AtPlain, "You are not carrying any $T.",
                                    ch, NULL, chk, ToChar);
                        return;
                }

                if (save_char)
                        save_char_obj(ch);
                /*
                 * Clan storeroom check 
                 */
                if (IsSet(ch->in_room->RoomFlags, RoomClanstoreroom)
                    && container->carried_by == NULL)
                        for (clan = first_clan; clan; clan = clan->next)
                                if (clan->storeroom == ch->in_room->vnum)
                                        save_clan_storeroom(ch, clan);
        }

        return;
}

// ============================================================================
// Object Dropping and Disposal Commands
// ============================================================================

CMDF do_drop(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ObjData *obj_next;
        bool      found;
        ClanData *clan;
        int       number;

        argument = one_argument(argument, arg);
        if (is_number(arg))
        {
                number = atoi(arg);
                if (number < 1)
                {
                        send_to_char("That was easy...\n\r", ch);
                        return;
                }
                argument = one_argument(argument, arg);
        }
        else
                number = 0;

        if (arg[0] == '\0')
        {
                send_to_char("Drop what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (IsSet(ch->in_room->RoomFlags, RoomNodrop)
            || (!IsNpc(ch) && IsSet(ch->act, PlrLitterbug)))
        {
                set_char_color(AtMagic, ch);
                send_to_char("A magical Force stops you!\n\r", ch);
                set_char_color(AtTell, ch);
                send_to_char("Someone tells you, 'No littering here!'\n\r",
                             ch);
                return;
        }

        if (number > 0)
        {
                /*
                 * 'drop NNNN coins' 
                 */

                if (!str_cmp(arg, "credits") || !str_cmp(arg, "credit"))
                {
                        if (ch->gold < number)
                        {
                                send_to_char
                                        ("You haven't got that many credits.\n\r",
                                         ch);
                                return;
                        }

                        ch->gold -= number;

                        for (obj = ch->in_room->first_content; obj;
                             obj = obj_next)
                        {
                                obj_next = obj->next_content;

                                switch (obj->pIndexData->vnum)
                                {
                                case ObjVnumMoneyOne:
                                        number += 1;
                                        extract_obj(obj);
                                        break;

                                case ObjVnumMoneySome:
                                        number += obj->value[0];
                                        extract_obj(obj);
                                        break;
                                default:
                                        {
                                        }   /* Do Nothing */
                                }
                        }

                        act(AtAction, "$n drops some credits.", ch, NULL,
                            NULL, ToRoom);
                        obj_to_room(create_money(number), ch->in_room);
                        send_to_char("OK.\n\r", ch);
                        if (IsSet(sysdata.save_flags, SvDrop))
                                save_char_obj(ch);
                        return;
                }
        }

        if (number <= 1 && str_cmp(arg, "all") && str_prefix("all.", arg))
        {
                /*
                 * 'drop obj' 
                 */
                if ((obj = get_obj_carry(ch, arg)) == NULL)
                {
                        send_to_char("You do not have that item.\n\r", ch);
                        return;
                }

                if (!can_drop_obj(ch, obj))
                {
                        send_to_char("You can't let go of it.\n\r", ch);
                        return;
                }

                separate_obj(obj);
                act(AtAction, "$n drops $p.", ch, obj, NULL, ToRoom);
                act(AtAction, "You drop $p.", ch, obj, NULL, ToChar);

                obj_from_char(obj);
                obj = obj_to_room(obj, ch->in_room);
                oprog_drop_trigger(ch, obj);    /* mudprogs */

                if (char_died(ch) || obj_extracted(obj))
                        return;

                /*
                 * Clan storeroom saving 
                 */
                if (IsSet(ch->in_room->RoomFlags, RoomClanstoreroom))
                        for (clan = first_clan; clan; clan = clan->next)
                                if (clan->storeroom == ch->in_room->vnum)
                                        save_clan_storeroom(ch, clan);
        }
        else
        {
                int       cnt = 0;
                char     *chk;
                bool      fAll;

                if (!str_cmp(arg, "all"))
                        fAll = TRUE;
                else
                        fAll = FALSE;
                if (number > 1)
                        chk = arg;
                else
                        chk = &arg[PrefixLength];
                /*
                 * 'drop all' or 'drop all.obj' 
                 */
                if (IsSet(ch->in_room->RoomFlags, RoomNodropall))
                {
                        send_to_char("You can't seem to do that here...\n\r",
                                     ch);
                        return;
                }
                found = FALSE;
                for (obj = ch->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;

                        if ((fAll || nifty_is_name(chk, obj->name))
                            && can_see_obj(ch, obj)
                            && obj->wear_loc == WearNone
                            && can_drop_obj(ch, obj))
                        {
                                found = TRUE;
                                if (obj->pIndexData->progtypes & DropProg
                                    && obj->count > 1)
                                {
                                        ++cnt;
                                        separate_obj(obj);
                                        obj_from_char(obj);
                                        if (!obj_next)
                                                obj_next = ch->first_carrying;
                                }
                                else
                                {
                                        if (number
                                            && (cnt + obj->count) > number)
                                                split_obj(obj, number - cnt);
                                        cnt += obj->count;
                                        obj_from_char(obj);
                                }
                                act(AtAction, "$n drops $p.", ch, obj, NULL,
                                    ToRoom);
                                act(AtAction, "You drop $p.", ch, obj, NULL,
                                    ToChar);
                                obj = obj_to_room(obj, ch->in_room);
                                oprog_drop_trigger(ch, obj);    /* mudprogs */
                                if (char_died(ch))
                                        return;
                                if (number && cnt >= number)
                                        break;
                        }
                }

                if (IsSet(ch->in_room->RoomFlags, RoomClanstoreroom))
                        for (clan = first_clan; clan; clan = clan->next)
                                if (clan->storeroom == ch->in_room->vnum)
                                        save_clan_storeroom(ch, clan);

                if (!found)
                {
                        if (fAll)
                                act(AtPlain,
                                    "You are not carrying anything.", ch,
                                    NULL, NULL, ToChar);
                        else
                                act(AtPlain, "You are not carrying any $T.",
                                    ch, NULL, chk, ToChar);
                }
        }
        if (IsSet(sysdata.save_flags, SvDrop))
                save_char_obj(ch);  /* duping protector */
        return;
}

// ============================================================================
// Object Transfer Between Characters
// ============================================================================

CMDF do_give(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      buf[MaxInputLength];
        CharData *victim;
        ObjData *obj;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if (!str_cmp(arg2, "to") && argument[0] != '\0')
                argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Give what to whom?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (is_number(arg1))
        {
                /*
                 * 'give NNNN coins victim' 
                 */
                int       amount;

                amount = atoi(arg1);
                if (amount <= 0
                    || (str_cmp(arg2, "credits") && str_cmp(arg2, "credit")))
                {
                        send_to_char("Sorry, you can't do that.\n\r", ch);
                        return;
                }

                argument = one_argument(argument, arg2);
                if (!str_cmp(arg2, "to") && argument[0] != '\0')
                        argument = one_argument(argument, arg2);
                if (arg2[0] == '\0')
                {
                        send_to_char("Give what to whom?\n\r", ch);
                        return;
                }

                if ((victim = get_char_room(ch, arg2)) == NULL)
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return;
                }

                if (ch->gold < amount)
                {
                        send_to_char
                                ("Very generous of you, but you haven't got that many credits.\n\r",
                                 ch);
                        return;
                }

                ch->gold -= amount;
                victim->gold += amount;
                mudstrlcpy(buf, "$n gives you ", MIL);
                mudstrlcat(buf, arg1, MIL);
                mudstrlcat(buf, (amount > 1) ? " credits." : " credit.", MIL);

                act(AtAction, buf, ch, NULL, victim, ToVict);
                act(AtAction, "$n gives $N some credits.", ch, NULL, victim,
                    ToNotvict);
                act(AtAction, "You give $N some credits.", ch, NULL, victim,
                    ToChar);
                send_to_char("OK.\n\r", ch);
                mprog_bribe_trigger(victim, ch, amount);
                if (IsSet(sysdata.save_flags, SvGive) && !char_died(ch))
                        save_char_obj(ch);
                if (IsSet(sysdata.save_flags, SvReceive)
                    && !char_died(victim))
                        save_char_obj(victim);
                return;
        }

        if ((obj = get_obj_carry(ch, arg1)) == NULL)
        {
                send_to_char("You do not have that item.\n\r", ch);
                return;
        }

        if (obj->wear_loc != WearNone)
        {
                send_to_char("You must remove it first.\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg2)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!can_drop_obj(ch, obj))
        {
                send_to_char("You can't let go of it.\n\r", ch);
                return;
        }

        if (victim->carry_number + (get_obj_number(obj) / obj->count) >
            can_carry_n(victim))
        {
                act(AtPlain, "$N has $S hands full.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if (victim->carry_weight + (get_obj_weight(obj) / obj->count) >
            can_carry_w(victim))
        {
                act(AtPlain, "$N can't carry that much weight.", ch, NULL,
                    victim, ToChar);
                return;
        }

        if (!can_see_obj(victim, obj))
        {
                act(AtPlain, "$N can't see it.", ch, NULL, victim, ToChar);
                return;
        }

        if (IsObjStat(obj, ItemPrototype) && !can_take_proto(victim))
        {
                act(AtPlain, "You cannot give that to $N!", ch, NULL, victim,
                    ToChar);
                return;
        }

        separate_obj(obj);
        obj_from_char(obj);
        act(AtAction, "$n gives $p to $N.", ch, obj, victim, ToNotvict);
        act(AtAction, "$n gives you $p.", ch, obj, victim, ToVict);
        act(AtAction, "You give $p to $N.", ch, obj, victim, ToChar);
        obj = obj_to_char(obj, victim);
        if (obj->item_type == ItemBeacon)
                obj->value[0] = 0;

        mprog_give_trigger(victim, ch, obj);
        check_given_bounty(ch, victim, obj);
        if (IsSet(sysdata.save_flags, SvGive) && !char_died(ch))
                save_char_obj(ch);
        if (IsSet(sysdata.save_flags, SvReceive) && !char_died(victim))
                save_char_obj(victim);
        return;
}

/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */
obj_ret damage_obj(ObjData * obj)
{
        CharData *ch;
        obj_ret   objcode;

        ch = obj->carried_by;
        objcode = rNONE;

        separate_obj(obj);
        if (ch)
                act(AtObject, "($p gets damaged)", ch, obj, NULL, ToChar);
        else if (obj->in_room && (ch = obj->in_room->first_person) != NULL)
        {
                act(AtObject, "($p gets damaged)", ch, obj, NULL, ToRoom);
                act(AtObject, "($p gets damaged)", ch, obj, NULL, ToChar);
                ch = NULL;
        }

        oprog_damage_trigger(ch, obj);
        if (obj_extracted(obj))
                return global_objcode;

        switch (obj->item_type)
        {
        default:
                make_scraps(obj);
                objcode = rOBJ_SCRAPPED;
                break;
        case ItemHolster:
        case ItemContainer:
                if (--obj->value[3] <= 0)
                {
                        make_scraps(obj);
                        objcode = rOBJ_SCRAPPED;
                }
                break;
        case ItemArmor:
                if (ch && obj->value[0] >= 1)
                        ch->Armor = static_cast<sh_int>(ch->Armor + apply_ac(obj, obj->wear_loc));
                obj->value[0] = static_cast<sh_int>(obj->value[0] - 1);
                if (obj->value[0] <= 0)
                {
                        make_scraps(obj);
                        objcode = rOBJ_SCRAPPED;
                }
                else if (ch && obj->value[0] >= 1)
                        ch->Armor = static_cast<sh_int>(ch->Armor - apply_ac(obj, obj->wear_loc));
                break;
        case ItemWeapon:
                obj->value[0] = static_cast<sh_int>(obj->value[0] - 1);
                if (obj->value[0] <= 0)
                {
                        make_scraps(obj);
                        objcode = rOBJ_SCRAPPED;
                }
                break;
        }
        if (ch != NULL)
                save_char_obj(ch);  /* Stop scrap duping - Samson 1-2-00 */

        return objcode;
}

// ============================================================================
// Equipment and Wearing System Functions
// ============================================================================

/*
 * Remove an object.
 */
bool remove_obj(CharData * ch, int iWear, bool fReplace)
{
        ObjData *obj, *tmpobj;

        if ((obj = get_eq_char(ch, iWear)) == NULL)
                return TRUE;

        if (!fReplace
            && ch->carry_number + get_obj_number(obj) > can_carry_n(ch))
        {
                act(AtPlain, "$d: you can't carry that many items.",
                    ch, NULL, obj->name, ToChar);
                return FALSE;
        }

        if (!fReplace)
                return FALSE;

        if (IsObjStat(obj, ItemNoremove))
        {
                act(AtPlain, "You can't remove $p.", ch, obj, NULL, ToChar);
                return FALSE;
        }

        if (obj == get_eq_char(ch, WearWield)
            && (tmpobj = get_eq_char(ch, WearDualWield)) != NULL)
                tmpobj->wear_loc = WearWield;

        unequip_char(ch, obj);

        act(AtAction, "$n stops using $p.", ch, obj, NULL, ToRoom);
        act(AtAction, "You stop using $p.", ch, obj, NULL, ToChar);
        oprog_remove_trigger(ch, obj);
        return TRUE;
}

/*
 * See if char could be capable of dual-wielding		-Thoric
 */
bool could_dual(CharData * ch)
{
        if (IsNpc(ch))
                return TRUE;
        /*
         * Body Parts / Bone snippet 
         */
        if (IsSet(ch->bodyparts, BodyLHand)
            || IsSet(ch->bodyparts, BodyRHand))
                return FALSE;
        if (ch->PCData->learned[gsn_dual_wield])
                return TRUE;

        return FALSE;
}

/*
 * See if char can dual wield at this time			-Thoric
 */
bool can_dual(CharData * ch)
{
        if (!could_dual(ch))
                return FALSE;

        if (get_eq_char(ch, WearDualWield))
        {
                send_to_char("You are already wielding two weapons!\n\r", ch);
                return FALSE;
        }
        if (get_eq_char(ch, WearHold))
        {
                send_to_char
                        ("You cannot dual wield while holding something!\n\r",
                         ch);
                return FALSE;
        }
        return TRUE;
}


/*
 * Check to see if there is room to wear another object on this location
 * (Layered clothing support)
 */
bool can_layer(CharData * ch, ObjData * obj, sh_int wear_loc)
{
        ObjData *otmp;
        sh_int    bitlayers = 0;
        sh_int    objlayers = obj->pIndexData->layers;

        for (otmp = ch->first_carrying; otmp; otmp = otmp->next_content)
                if (otmp->wear_loc == wear_loc)
                {
                        if (!otmp->pIndexData->layers)
                                return FALSE;
                        else
                                bitlayers |= otmp->pIndexData->layers;
                }
        if ((bitlayers && !objlayers) || bitlayers > objlayers)
                return FALSE;
        if (!bitlayers || ((bitlayers & ~objlayers) == bitlayers))
                return TRUE;
        return FALSE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 * Restructured a bit to allow for specifying body location	-Thoric
 */
void wear_obj(CharData * ch, ObjData * obj, bool fReplace, sh_int wear_bit)
{
        char      buf[MaxStringLength];
        ObjData *tmpobj;
        sh_int    bit, tmp;
        bool      check_size;

        separate_obj(obj);

        if (wear_bit > -1)
        {
                bit = wear_bit;
                if (!CanWear(obj, 1 << bit))
                {
                        if (fReplace)
                        {
                                switch (1 << bit)
                                {
                                case ItemHold:
                                        send_to_char
                                                ("You cannot hold that.\n\r",
                                                 ch);
                                        break;
                                case ItemWield:
                                        send_to_char
                                                ("You cannot wield that.\n\r",
                                                 ch);
                                        break;
                                default:
                                        snprintf(buf, MSL,
                                                 "You cannot wear that on your %s.\n\r",
                                                 w_flags[bit]);
                                        send_to_char(buf, ch);
                                }
                        }
                        return;
                }
        }
        else
        {
                for (bit = -1, tmp = 1; tmp < 31; tmp++)
                {
                        if (CanWear(obj, 1 << tmp))
                        {
                                bit = tmp;
                                break;
                        }
                }
        }

        if (IsSet(ch->affected_by, AffRestrained))
        {
                send_to_char
                        ("How do you expect to do that while restrained?\n\r",
                         ch);
                return;
        }

        check_size = FALSE;

        if (1 << bit == ItemWield || 1 << bit == ItemHold
            || obj->item_type == ItemLight || 1 << bit == ItemWearShield)
                check_size = FALSE;
        else if (!str_cmp(ch->race->name(), "duinuogwuin")
                 || !str_cmp(ch->race->name(), "defel"))
                check_size = TRUE;
        else if (!IsNpc(ch))
        {
                if (!str_cmp(ch->race->name(), "trandoshan") ||
                    !str_cmp(ch->race->name(), "verpine") ||
                    !str_cmp(ch->race->name(), "human") ||
                    !str_cmp(ch->race->name(), "adarian") ||
                    !str_cmp(ch->race->name(), "rodian") ||
                    !str_cmp(ch->race->name(), "twi'lek"))
                {
                        if (!IsObjStat(obj, ItemHumanSize))
                                check_size = TRUE;
                }

                if (!str_cmp(ch->race->name(), "hutt"))
                {
                        if (!IsObjStat(obj, ItemHuttSize))
                                check_size = TRUE;
                }

                if (!str_cmp(ch->race->name(), "gamorrean") ||
                    !str_cmp(ch->race->name(), "mon calamari") ||
                    !str_cmp(ch->race->name(), "wookiee"))
                {
                        if (!IsObjStat(obj, ItemLargeSize))
                                check_size = TRUE;
                }

                if (!str_cmp(ch->race->name(), "ewok") ||
                    !str_cmp(ch->race->name(), "noghri") ||
                    !str_cmp(ch->race->name(), "jawa"))
                {
                        if (!IsObjStat(obj, ItemSmallSize))
                                check_size = TRUE;
                }

        }

        /*
         * this seems redundant but it enables both multiple sized objects to be 
         * used as well as objects with no size flags at all 
         */

        if (check_size)
        {
                if (!str_cmp(ch->race->name(), "duinuogwuin"))
                {
                        act(AtMagic,
                            "It will probably be impossible to find something that fits your enormous body.",
                            ch, NULL, NULL, ToChar);
                        act(AtAction, "$n tries to use $p, but is too big.",
                            ch, obj, NULL, ToRoom);
                        return;
                }

                if (!str_cmp(ch->race->name(), "defel")
                    && !IsSet(obj->extra_flags, ItemInvis))
                {
                        act(AtMagic,
                            "It is against your nature to wear anything that might make you visible.",
                            ch, NULL, NULL, ToChar);
                        act(AtAction, "$n wants to use $p, but doesn't.", ch,
                            obj, NULL, ToRoom);
                        return;
                }

                if (IsObjStat(obj, ItemHuttSize))
                {
                        act(AtMagic, "That item is too big for you.", ch,
                            NULL, NULL, ToChar);
                        act(AtAction,
                            "$n tries to use $p, but it is too big.", ch, obj,
                            NULL, ToRoom);
                        return;
                }

                if (IsObjStat(obj, ItemLargeSize)
                    || IsObjStat(obj, ItemHumanSize))
                {
                        act(AtMagic, "That item is the wrong size for you.",
                            ch, NULL, NULL, ToChar);
                        act(AtAction, "$n tries to use $p, but can't.", ch,
                            obj, NULL, ToRoom);
                        return;
                }

                if (IsObjStat(obj, ItemSmallSize))
                {
                        act(AtMagic, "That item is too small for you.", ch,
                            NULL, NULL, ToChar);
                        act(AtAction,
                            "$n tries to use $p, but it is too small.", ch,
                            obj, NULL, ToRoom);
                        return;
                }
        }

        /*
         * currently cannot have a light in non-light position 
         */
        if (obj->item_type == ItemLight)
        {
                if (!remove_obj(ch, WearLight, fReplace))
                        return;
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n holds $p as a light.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You hold $p as your light.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearLight);
                oprog_wear_trigger(ch, obj);
                return;
        }

        if (bit == -1)
        {
                if (fReplace)
                        send_to_char
                                ("You can't wear, wield, or hold that.\n\r",
                                 ch);
                return;
        }

        switch (1 << bit)
        {
        default:
                bug("wear_obj: uknown/unused item_wear bit %d", bit);
                if (fReplace)
                        send_to_char
                                ("You can't wear, wield, or hold that.\n\r",
                                 ch);
                return;

        case ItemWearHolster1:
                if (get_eq_char(ch, WearHolsterL))
                {
                        if (get_eq_char(ch, WearHolsterR))
                        {
                                send_to_char
                                        ("You can only wear two holsters!\n\r",
                                         ch);
                                return;
                        }
                        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                        {
                                if (!obj->action_desc
                                    || obj->action_desc[0] == '\0')
                                {
                                        act(AtAction,
                                            "$n wears $p on $s right side.",
                                            ch, obj, NULL, ToRoom);
                                        act(AtAction,
                                            "You wear $p on your right side.",
                                            ch, obj, NULL, ToChar);
                                }
                                else
                                        actiondesc(ch, obj, NULL);
                        }
                        equip_char(ch, obj, WearHolsterR);
                        oprog_wear_trigger(ch, obj);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wears $p on $s left side.",
                                    ch, obj, NULL, ToRoom);
                                act(AtAction,
                                    "You wear $p on your left side.", ch, obj,
                                    NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearHolsterL);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearFinger:
                if (get_eq_char(ch, WearFingerL)
                    && get_eq_char(ch, WearFingerR)
                    && !remove_obj(ch, WearFingerL, fReplace)
                    && !remove_obj(ch, WearFingerR, fReplace))
                        return;

                if (!get_eq_char(ch, WearFingerL))
                {
                        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                        {
                                if (!obj->action_desc
                                    || obj->action_desc[0] == '\0')
                                {
                                        act(AtAction,
                                            "$n slips $s left finger into $p.",
                                            ch, obj, NULL, ToRoom);
                                        act(AtAction,
                                            "You slip your left finger into $p.",
                                            ch, obj, NULL, ToChar);
                                }
                                else
                                        actiondesc(ch, obj, NULL);
                        }
                        equip_char(ch, obj, WearFingerL);
                        oprog_wear_trigger(ch, obj);
                        return;
                }

                if (!get_eq_char(ch, WearFingerR))
                {
                        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                        {
                                if (!obj->action_desc
                                    || obj->action_desc[0] == '\0')
                                {
                                        act(AtAction,
                                            "$n slips $s right finger into $p.",
                                            ch, obj, NULL, ToRoom);
                                        act(AtAction,
                                            "You slip your right finger into $p.",
                                            ch, obj, NULL, ToChar);
                                }
                                else
                                        actiondesc(ch, obj, NULL);
                        }
                        equip_char(ch, obj, WearFingerR);
                        oprog_wear_trigger(ch, obj);
                        return;
                }

                bug("Wear_obj: no free finger.", 0);
                send_to_char
                        ("You already wear something on both fingers.\n\r",
                         ch);
                return;

        case ItemWearNeck:
                if (get_eq_char(ch, WearNeck1) != NULL
                    && get_eq_char(ch, WearNeck2) != NULL
                    && !remove_obj(ch, WearNeck1, fReplace)
                    && !remove_obj(ch, WearNeck2, fReplace))
                        return;

                if (!get_eq_char(ch, WearNeck1))
                {
                        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                        {
                                if (!obj->action_desc
                                    || obj->action_desc[0] == '\0')
                                {
                                        act(AtAction,
                                            "$n wears $p around $s neck.", ch,
                                            obj, NULL, ToRoom);
                                        act(AtAction,
                                            "You wear $p around your neck.",
                                            ch, obj, NULL, ToChar);
                                }
                                else
                                        actiondesc(ch, obj, NULL);
                        }
                        equip_char(ch, obj, WearNeck1);
                        oprog_wear_trigger(ch, obj);
                        return;
                }

                if (!get_eq_char(ch, WearNeck2))
                {
                        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                        {
                                if (!obj->action_desc
                                    || obj->action_desc[0] == '\0')
                                {
                                        act(AtAction,
                                            "$n wears $p around $s neck.", ch,
                                            obj, NULL, ToRoom);
                                        act(AtAction,
                                            "You wear $p around your neck.",
                                            ch, obj, NULL, ToChar);
                                }
                                else
                                        actiondesc(ch, obj, NULL);
                        }
                        equip_char(ch, obj, WearNeck2);
                        oprog_wear_trigger(ch, obj);
                        return;
                }

                bug("Wear_obj: no free neck.", 0);
                send_to_char("You already wear two neck items.\n\r", ch);
                return;

        case ItemWearBody:
                /*
                 * if ( !remove_obj( ch, WearBody, fReplace ) )
                 * return;
                 */
                if (!can_layer(ch, obj, WearBody))
                {
                        send_to_char
                                ("It won't fit overtop of what you're already wearing.\n\r",
                                 ch);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n fits $p on $s body.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You fit $p on your body.", ch,
                                    obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearBody);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearBinding:
                if (!obj->action_desc || obj->action_desc[0] == '\0')
                {
                        act(AtAction, "$n fits $p on $s body.", ch, obj,
                            NULL, ToRoom);
                        act(AtAction, "You fit $p on your body.", ch, obj,
                            NULL, ToChar);
                }
                else
                        actiondesc(ch, obj, NULL);
                equip_char(ch, obj, ItemWearBinding);
                oprog_wear_trigger(ch, obj);
                return;
        case ItemWearHead:
                if (!str_cmp(ch->race->name(), "verpine")
                    || !str_cmp(ch->race->name(), "twi'lek"))
                {
                        send_to_char
                                ("You cant wear anything on your head.\n\r",
                                 ch);
                        return;
                }
                if (!remove_obj(ch, WearHead, fReplace))
                        return;
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n dons $p upon $s head.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You don $p upon your head.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearHead);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearEyes:
                if (!remove_obj(ch, WearEyes, fReplace))
                        return;
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n places $p on $s eyes.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You place $p on your eyes.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearEyes);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearEars:
                if (!str_cmp(ch->race->name(), "verpine"))
                {
                        send_to_char("What ears?.\n\r", ch);
                        return;
                }
                if (!remove_obj(ch, WearEars, fReplace))
                        return;
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wears $p on $s ears.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You wear $p on your ears.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearEars);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearLegs:
/*
	    if ( !remove_obj( ch, WearLegs, fReplace ) )
	      return;
*/
                if (!str_cmp(ch->race->name(), "hutt"))
                {
                        send_to_char("Hutts don't have legs.\n\r", ch);
                        return;
                }
                if (!can_layer(ch, obj, WearLegs))
                {
                        send_to_char
                                ("It won't fit overtop of what you're already wearing.\n\r",
                                 ch);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n slips into $p.", ch, obj,
                                    NULL, ToRoom);
                                act(AtAction, "You slip into $p.", ch, obj,
                                    NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearLegs);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearFeet:
/*
	    if ( !remove_obj( ch, WearFeet, fReplace ) )
	      return;
*/
                if (!str_cmp(ch->race->name(), "feet"))
                {
                        send_to_char("Hutts don't have feet!\n\r", ch);
                        return;
                }
                if (!can_layer(ch, obj, WearFeet))
                {
                        send_to_char
                                ("It won't fit overtop of what you're already wearing.\n\r",
                                 ch);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wears $p on $s feet.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You wear $p on your feet.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearFeet);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearHands:
/*
	    if ( !remove_obj( ch, WearHands, fReplace ) )
	      return;
*/
                if (!can_layer(ch, obj, WearHands))
                {
                        send_to_char
                                ("It won't fit overtop of what you're already wearing.\n\r",
                                 ch);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wears $p on $s hands.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You wear $p on your hands.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearHands);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearArms:
/*
	    if ( !remove_obj( ch, WearArms, fReplace ) )
	      return;
*/
                if (!can_layer(ch, obj, WearArms))
                {
                        send_to_char
                                ("It won't fit overtop of what you're already wearing.\n\r",
                                 ch);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wears $p on $s arms.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction, "You wear $p on your arms.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearArms);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearAbout:
                /*
                 * if ( !remove_obj( ch, WearAbout, fReplace ) )
                 * return;
                 */
                if (!can_layer(ch, obj, WearAbout))
                {
                        send_to_char
                                ("It won't fit overtop of what you're already wearing.\n\r",
                                 ch);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wears $p about $s body.",
                                    ch, obj, NULL, ToRoom);
                                act(AtAction, "You wear $p about your body.",
                                    ch, obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearAbout);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearWaist:
/*
	    if ( !remove_obj( ch, WearWaist, fReplace ) )
	      return;
*/
                if (!can_layer(ch, obj, WearWaist))
                {
                        send_to_char
                                ("It won't fit overtop of what you're already wearing.\n\r",
                                 ch);
                        return;
                }
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wears $p about $s waist.",
                                    ch, obj, NULL, ToRoom);
                                act(AtAction,
                                    "You wear $p about your waist.", ch, obj,
                                    NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearWaist);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWearWrist:
                if (get_eq_char(ch, WearWristL)
                    && get_eq_char(ch, WearWristR)
                    && !remove_obj(ch, WearWristL, fReplace)
                    && !remove_obj(ch, WearWristR, fReplace))
                        return;

                if (!get_eq_char(ch, WearWristL))
                {
                        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                        {
                                if (!obj->action_desc
                                    || obj->action_desc[0] == '\0')
                                {
                                        act(AtAction,
                                            "$n fits $p around $s left wrist.",
                                            ch, obj, NULL, ToRoom);
                                        act(AtAction,
                                            "You fit $p around your left wrist.",
                                            ch, obj, NULL, ToChar);
                                }
                                else
                                        actiondesc(ch, obj, NULL);
                        }
                        equip_char(ch, obj, WearWristL);
                        oprog_wear_trigger(ch, obj);
                        return;
                }

                if (!get_eq_char(ch, WearWristR))
                {
                        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                        {
                                if (!obj->action_desc
                                    || obj->action_desc[0] == '\0')
                                {
                                        act(AtAction,
                                            "$n fits $p around $s right wrist.",
                                            ch, obj, NULL, ToRoom);
                                        act(AtAction,
                                            "You fit $p around your right wrist.",
                                            ch, obj, NULL, ToChar);
                                }
                                else
                                        actiondesc(ch, obj, NULL);
                        }
                        equip_char(ch, obj, WearWristR);
                        oprog_wear_trigger(ch, obj);
                        return;
                }

                bug("Wear_obj: no free wrist.", 0);
                send_to_char("You already wear two wrist items.\n\r", ch);
                return;

        case ItemWearShield:
                if (!remove_obj(ch, WearShield, fReplace))
                        return;
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction,
                                    "$n uses $p as an energy shield.", ch,
                                    obj, NULL, ToRoom);
                                act(AtAction,
                                    "You use $p as an energy shield.", ch,
                                    obj, NULL, ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearShield);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemWield:
                if ((tmpobj = get_eq_char(ch, WearWield)) != NULL
                    && !could_dual(ch))
                {
                        send_to_char("You're already wielding something.\n\r",
                                     ch);
                        return;
                }

                if (tmpobj)
                {
                        if (can_dual(ch))
                        {
                                if (get_obj_weight(obj) +
                                    get_obj_weight(tmpobj) >
                                    StrApp[get_curr_str(ch)].wield)
                                {
                                        send_to_char
                                                ("It is too heavy for you to wield.\n\r",
                                                 ch);
                                        return;
                                }
                                if (!oprog_use_trigger
                                    (ch, obj, NULL, NULL, NULL))
                                {
                                        if (!obj->action_desc
                                            || obj->action_desc[0] == '\0')
                                        {
                                                act(AtAction,
                                                    "$n dual-wields $p.", ch,
                                                    obj, NULL, ToRoom);
                                                act(AtAction,
                                                    "You dual-wield $p.", ch,
                                                    obj, NULL, ToChar);
                                        }
                                        else
                                                actiondesc(ch, obj, NULL);
                                }
                                equip_char(ch, obj, WearDualWield);
                                oprog_wear_trigger(ch, obj);
                        }
                        return;
                }

                if (get_obj_weight(obj) > StrApp[get_curr_str(ch)].wield)
                {
                        send_to_char("It is too heavy for you to wield.\n\r",
                                     ch);
                        return;
                }

                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        if (!obj->action_desc || obj->action_desc[0] == '\0')
                        {
                                act(AtAction, "$n wields $p.", ch, obj, NULL,
                                    ToRoom);
                                act(AtAction, "You wield $p.", ch, obj, NULL,
                                    ToChar);
                        }
                        else
                                actiondesc(ch, obj, NULL);
                }
                equip_char(ch, obj, WearWield);
                oprog_wear_trigger(ch, obj);
                return;

        case ItemHold:
                if (get_eq_char(ch, WearDualWield))
                {
                        send_to_char
                                ("You cannot hold something AND two weapons!\n\r",
                                 ch);
                        return;
                }
                if (!remove_obj(ch, WearHold, fReplace))
                        return;
                if (obj->item_type == ItemDevice
                    || obj->item_type == ItemGrenade
                    || obj->item_type == ItemFood
                    || obj->item_type == ItemPill
                    || obj->item_type == ItemPotion
                    || obj->item_type == ItemDrinkCon
                    || obj->item_type == ItemSalve
                    || obj->item_type == ItemKey
                    || !oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        act(AtAction, "$n holds $p in $s hands.", ch, obj,
                            NULL, ToRoom);
                        act(AtAction, "You hold $p in your hands.", ch, obj,
                            NULL, ToChar);
                }
                equip_char(ch, obj, WearHold);
                oprog_wear_trigger(ch, obj);
                return;
        }
}


CMDF do_wear(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        ObjData *obj;
        sh_int    wear_bit;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if ((!str_cmp(arg2, "on") || !str_cmp(arg2, "upon")
             || !str_cmp(arg2, "around")) && argument[0] != '\0')
                argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Wear, wield, or hold what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (!str_cmp(arg1, "all"))
        {
                ObjData *obj_next;

                for (obj = ch->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        if (obj->wear_loc == WearNone
                            && can_see_obj(ch, obj))
                                wear_obj(ch, obj, FALSE, -1);
                }
                return;
        }
        else
        {
                if ((obj = get_obj_carry(ch, arg1)) == NULL)
                {
                        send_to_char("You do not have that item.\n\r", ch);
                        return;
                }
                if (arg2[0] != '\0')
                        wear_bit = static_cast<sh_int>(get_wflag(arg2));
                else
                        wear_bit = static_cast<sh_int>(-1);
                wear_obj(ch, obj, TRUE, wear_bit);
        }

        return;
}



CMDF do_remove(CharData * ch, const char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj, *obj_next;


        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
        {
                send_to_char("Remove what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (!str_cmp(arg, "all"))   /* SB Remove all */
        {
                for (obj = ch->first_carrying; obj != NULL; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        if (obj->wear_loc != WearNone
                            && can_see_obj(ch, obj))
                                remove_obj(ch, obj->wear_loc, TRUE);
                }
                return;
        }

        if ((obj = get_obj_wear(ch, arg)) == NULL)
        {
                send_to_char("You are not using that item.\n\r", ch);
                return;
        }
        if ((obj_next = get_eq_char(ch, obj->wear_loc)) != obj)
        {
                act(AtPlain, "You must remove $p first.", ch, obj_next, NULL,
                    ToChar);
                return;
        }

        remove_obj(ch, obj->wear_loc, TRUE);
        return;
}

// ============================================================================
// Object Burial and Recovery Commands
// ============================================================================

CMDF do_bury(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        bool      shovel;
        sh_int    move;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("What do you wish to bury?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        shovel = FALSE;
        for (obj = ch->first_carrying; obj; obj = obj->next_content)
                if (obj->item_type == ItemShovel)
                {
                        shovel = TRUE;
                        break;
                }

        obj = get_obj_list_rev(ch, arg, ch->in_room->last_content);
        if (!obj)
        {
                send_to_char("You can't find it.\n\r", ch);
                return;
        }

        separate_obj(obj);
        if (!CanWear(obj, ItemTake))
        {
                act(AtPlain, "You cannot bury $p.", ch, obj, 0, ToChar);
                return;
        }

        switch (ch->in_room->sector_type)
        {
        case SectCity:
        case SectInside:
                send_to_char("The floor is too hard to dig through.\n\r", ch);
                return;
        case SectWaterSwim:
        case SectWaterNoswim:
        case SectUnderwater:
                send_to_char("You cannot bury something here.\n\r", ch);
                return;
        case SectAir:
                send_to_char("What?  In the air?!\n\r", ch);
                return;
        default:
                {
                }
        }

        if (obj->weight > (UMax(MinBuryWeight, (can_carry_w(ch) / BuryWeightDivisor))) && !shovel)
        {
                send_to_char
                        ("You'd need a shovel to bury something that big.\n\r",
                         ch);
                return;
        }

        move = static_cast<sh_int>((obj->weight * BuryMoveMultiplier * (shovel ? ShovelMultiplier : NoShovelMultiplier)) / UMax(1,
                                                            can_carry_w(ch)));
        move = static_cast<sh_int>(URange(MinBuryMove, move, MaxBuryMove));
        if (move > ch->endurance)
        {
                send_to_char
                        ("You don't have the energy to bury something of that size.\n\r",
                         ch);
                return;
        }
        ch->endurance -= move;

        act(AtAction, "You solemnly bury $p...", ch, obj, NULL, ToChar);
        act(AtAction, "$n solemnly buries $p...", ch, obj, NULL, ToRoom);
        SetBit(obj->extra_flags, ItemBurried);
        WaitState(ch, URange(MinBuryWait, move / BuryWaitDivisor, MaxBuryWait));
        return;
}

// ============================================================================
// Magic Item Commands
// ============================================================================

CMDF do_zap(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        ObjData *wand;
        ObjData *obj;
        ch_ret    retcode;

        one_argument(argument, arg);
        if (arg[0] == '\0' && !ch->fighting)
        {
                send_to_char("Zap whom or what?\n\r", ch);
                return;
        }

        if ((wand = get_eq_char(ch, WearHold)) == NULL)
        {
                send_to_char("You hold nothing in your hand.\n\r", ch);
                return;
        }

        if (wand->item_type != ItemWand)
        {
                send_to_char("You can zap only with a wand.\n\r", ch);
                return;
        }

        obj = NULL;
        if (arg[0] == '\0')
        {
                if (ch->fighting)
                {
                        victim = who_fighting(ch);
                }
                else
                {
                        send_to_char("Zap whom or what?\n\r", ch);
                        return;
                }
        }
        else
        {
                if ((victim = get_char_room(ch, arg)) == NULL
                    && (obj = get_obj_here(ch, arg)) == NULL)
                {
                        send_to_char("You can't find it.\n\r", ch);
                        return;
                }
        }

        WaitState(ch, 1 * PulseViolence);

        if (wand->value[2] > 0)
        {
                if (victim)
                {
                        if (!oprog_use_trigger(ch, wand, victim, NULL, NULL))
                        {
                                act(AtMagic, "$n aims $p at $N.", ch, wand,
                                    victim, ToRoom);
                                act(AtMagic, "You aim $p at $N.", ch, wand,
                                    victim, ToChar);
                        }
                }
                else
                {
                        if (!oprog_use_trigger(ch, wand, NULL, obj, NULL))
                        {
                                act(AtMagic, "$n aims $p at $P.", ch, wand,
                                    obj, ToRoom);
                                act(AtMagic, "You aim $p at $P.", ch, wand,
                                    obj, ToChar);
                        }
                }

                retcode =
                        obj_cast_spell(wand->value[3], wand->value[0], ch,
                                       victim, obj);
                if (retcode == rCHAR_DIED || retcode == rBOTH_DIED)
                {
                        bug("do_zap: char died", 0);
                        return;
                }
        }

        if (--wand->value[2] <= 0)
        {
                act(AtMagic, "$p explodes into fragments.", ch, wand, NULL,
                    ToRoom);
                act(AtMagic, "$p explodes into fragments.", ch, wand, NULL,
                    ToChar);
                extract_obj(wand);
        }

        return;
}

// ============================================================================
// Clan Storage System
// ============================================================================

/*
 * Save items in a clan storage room			-Scryn & Thoric
 */
void save_clan_storeroom(CharData * ch, ClanData * clan)
{
        FILE     *fp;
        char      filename[256];
        sh_int    templvl;
        ObjData *contents;

        if (!clan)
        {
                bug("save_clan_storeroom: Null clan pointer!", 0);
                return;
        }

        if (!ch)
        {
                bug("save_clan_storeroom: Null ch pointer!", 0);
                return;
        }

        snprintf(filename, MSL, "%s%s.vault", ClanDir, clan->filename);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_clan_storeroom: fopen", 0);
                perror(filename);
        }
        else
        {
                templvl = ch->top_level;
                ch->top_level = LevelHero; /* make sure EQ doesn't get lost */
                contents = ch->in_room->last_content;
                if (contents && contents->item_type != ItemCorpsePc)
                        fwrite_obj(ch, contents, fp, 0, OsCarry, FALSE);
                fprintf(fp, "#END\n");
                ch->top_level = templvl;
                FCLOSE(fp);
                return;
        }
        return;
}

/* put an item on auction, or see the stats on the current item or bet */
CMDF do_auction(CharData * ch, const char *argument)
{
        ObjData *obj;
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      buf[MaxStringLength];
        char      mutable_argument[MaxInputLength];

        strcpy(mutable_argument, argument);
        one_argument(mutable_argument, arg1);

        if (IsNpc(ch)) /* NPC can be extracted at any time and thus can't auction! */
                return;

        if (!IsSet(ch->in_room->RoomFlags, RoomAuction) &&
            !IsSet(ch->in_room->RoomFlags, RoomHotel) &&
            !IsSet(ch->in_room->RoomFlags, RoomSafe))
        {
                set_char_color(AtLblue, ch);
                send_to_char
                        ("\n\rYou must go to an auction hall to do that!\n\r",
                         ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                if (auction->item != NULL)
                {
                        AffectData *paf;

                        obj = auction->item;

                        /*
                         * show item data here 
                         */
                        if (auction->bet > 0)
                                snprintf(buf, MSL,
                                         "Current bid on this item is %d credits.\n\r",
                                         auction->bet);
                        else
                                snprintf(buf, MSL,
                                         "No bids on this item have been received.\n\r");
                        set_char_color(AtBlue, ch);
                        send_to_char(buf, ch);
/*          spell_identify (0, LevelHero - 1, ch, auction->item); */

                        snprintf(buf, MSL,
                                 "Object '%s' is %s, special properties: %s %s.\n\rIts weight is %d, value is %d.\n\r",
                                 obj->name,
                                 aoran(item_type_name(obj)),
                                 extra_bit_name(obj->extra_flags),
                                 magic_bit_name(obj->magic_flags),
                                 obj->weight, obj->cost);
                        set_char_color(AtLblue, ch);
                        send_to_char(buf, ch);

                        snprintf(buf, MSL, "Worn on: %s\n\r",
                                 flag_string(obj->wear_flags - 1, w_flags));
                        send_to_char(buf, ch);

                        set_char_color(AtBlue, ch);

                        /*
                         * Gavin - No real need for a switch here 
                         */
                        if (obj->item_type == ItemArmor)
                        {
                                ch_printf(ch,
                                          "Current Armor class is %d. ( based on current condition )\n\r",
                                          obj->value[0]);
                                ch_printf(ch,
                                          "Maximum Armor class is %d. ( based on top condition )\n\r",
                                          obj->value[1]);
                        }

                        for (paf = obj->pIndexData->first_affect; paf;
                             paf = paf->next)
                                showaffect(ch, paf);

                        for (paf = obj->first_affect; paf; paf = paf->next)
                                showaffect(ch, paf);
                        if ((obj->item_type == ItemContainer ||
                             obj->item_type == ItemHolster)
                            && (obj->first_content))
                        {
                                set_char_color(AtObject, ch);
                                send_to_char("Contents:\n\r", ch);
                                show_list_to_char(obj->first_content, ch,
                                                  TRUE, FALSE);
                        }

                        if (IsImmortal(ch))
                        {
                                snprintf(buf, MSL,
                                         "Seller: %s.  Bidder: %s.  Round: %d.\n\r",
                                         auction->seller->name,
                                         auction->buyer->name,
                                         (auction->going + 1));
                                send_to_char(buf, ch);
                                snprintf(buf, MSL,
                                         "Time left in round: %d.\n\r",
                                         auction->pulse);
                                send_to_char(buf, ch);
                        }
                        return;
                }
                else
                {
                        set_char_color(AtLblue, ch);
                        send_to_char
                                ("\n\rThere is nothing being auctioned right now.  What would you like to auction?\n\r",
                                 ch);
                        return;
                }
        }

        if (IsImmortal(ch) && !str_cmp(arg1, "stop"))
        {
                if (auction->item == NULL)
                {
                        send_to_char("There is no auction to stop.\n\r", ch);
                        return;
                }
                else    /* stop the auction */
                {
                        set_char_color(AtLblue, ch);
                        snprintf(buf, MSL,
                                 "Sale of %s has been stopped by an Immortal.",
                                 auction->item->short_descr);
                        talk_auction(buf);
                        obj_to_char(auction->item, auction->seller);
                        if (auction->item->item_type == ItemBeacon)
                                auction->item->value[0] = 0;
                        if (IsSet(sysdata.save_flags, SvAuction))
                                save_char_obj(auction->seller);
                        auction->item = NULL;
                        if (auction->buyer != NULL && auction->buyer != auction->seller)    /* return money to the buyer */
                        {
                                auction->buyer->gold += auction->bet;
                                send_to_char
                                        ("Your money has been returned.\n\r",
                                         auction->buyer);
                        }
                        return;
                }
        }
        if (!str_cmp(arg1, "bid"))
        {
                if (auction->item != NULL)
                {
                        int       newbet;

                        if (ch == auction->seller)
                        {
                                send_to_char
                                        ("You can't bid on your own item!\n\r",
                                         ch);
                                return;
                        }

                        /*
                         * make - perhaps - a bet now 
                         */
                        if (argument[0] == '\0')
                        {
                                send_to_char("Bid how much?\n\r", ch);
                                return;
                        }

                        newbet = parsebet(auction->bet, const_cast<char*>(argument));
/*	    ch_printf( ch, "Bid: %d\n\r",newbet);	*/

                        if (newbet < auction->starting)
                        {
                                send_to_char
                                        ("You must place a bid that is higher than the starting bet.\n\r",
                                         ch);
                                return;
                        }

                        /*
                         * to avoid slow auction, use a bigger amount than 100 if the bet
                         * is higher up - changed to 10000 for our high economy
                         */

                        if (newbet < (auction->bet + 10000))
                        {
                                send_to_char
                                        ("You must at least bid 10000 credits over the current bid.\n\r",
                                         ch);
                                return;
                        }

                        if (newbet > ch->gold)
                        {
                                send_to_char
                                        ("You don't have that much money!\n\r",
                                         ch);
                                return;
                        }

                        if (newbet > 2000000000)
                        {
                                send_to_char
                                        ("You can't bid over 2 billion credits.\n\r",
                                         ch);
                                return;
                        }

                        /*
                         * the actual bet is OK! 
                         */

                        /*
                         * return the gold to the last buyer, if one exists 
                         */
                        if (auction->buyer != NULL
                            && auction->buyer != auction->seller)
                                auction->buyer->gold += auction->bet;

                        ch->gold -= newbet; /* substract the gold - important :) */
                        if (IsSet(sysdata.save_flags, SvAuction))
                                save_char_obj(ch);
                        auction->buyer = ch;
                        auction->bet = newbet;
                        auction->going = 0;
                        auction->pulse = PulseAuction; /* start the auction over again */

                        snprintf(buf, MSL,
                                 "A bid of %d credits has been received on %s.\n\r",
                                 newbet, auction->item->short_descr);
                        talk_auction(buf);
                        return;


                }
                else
                {
                        send_to_char
                                ("There isn't anything being auctioned right now.\n\r",
                                 ch);
                        return;
                }
        }
/* finally... */
        if (ms_find_obj(ch))
                return;

        obj = get_obj_carry(ch, arg1);  /* does char have the item ? */

        if (obj == NULL)
        {
                send_to_char("You aren't carrying that.\n\r", ch);
                return;
        }

        if (obj->timer > 0)
        {
                send_to_char
                        ("You can't auction objects that are decaying.\n\r",
                         ch);
                return;
        }

        strcpy(mutable_argument, argument);
        one_argument(mutable_argument, arg2);

        if (arg2[0] == '\0')
        {
                auction->starting = 0;
                mudstrlcpy(arg2, "0", MIL);
        }

        if (!is_number(arg2))
        {
                send_to_char
                        ("You must input a number at which to start the auction.\n\r",
                         ch);
                return;
        }

        if (atoi(arg2) < 0)
        {
                send_to_char
                        ("You can't auction something for less than 0 credits!\n\r",
                         ch);
                return;
        }

        if (auction->item == NULL)
                switch (obj->item_type)
                {

                default:
                        act(AtTell, "You cannot auction $Ts.", ch, NULL,
                            item_type_name(obj), ToChar);
                        return;

/* insert any more item types here... items with a timer MAY NOT BE 
   AUCTIONED! 
*/
                case ItemLight:
                case ItemTreasure:
                case ItemRareMetal:
                case ItemCrystal:
                case ItemBook:
                case ItemFabric:
                case ItemArmor:
                        separate_obj(obj);
                        obj_from_char(obj);
                        if (IsSet(sysdata.save_flags, SvAuction))
                                save_char_obj(ch);
                        auction->item = obj;
                        auction->bet = 0;
                        auction->buyer = ch;
                        auction->seller = ch;
                        auction->pulse = PulseAuction;
                        auction->going = 0;
                        auction->starting = atoi(arg2);

                        if (auction->starting > 0)
                                auction->bet = auction->starting;

                        snprintf(buf, MSL,
                                 "A new item is being auctioned: %s at %d credits.",
                                 obj->short_descr, auction->starting);
                        talk_auction(buf);

                        return;

                }   /* switch */
        else
        {
                act(AtTell,
                    "Try again later - $p is being auctioned right now!", ch,
                    auction->item, NULL, ToChar);
                WaitState(ch, static_cast<sh_int>(1.5 * PulseViolence));
                return;
        }
}



/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */

void obj_fall(ObjData * obj, bool through)
{
        ExitData *pexit;
        RoomIndexData *to_room;
        static int fall_count;
        char      buf[MaxStringLength];
        static bool is_falling; /* Stop loops from the call to obj_to_room()  -- Altrag */

        if (!obj->in_room || is_falling)
                return;

        if (fall_count > 30)
        {
                bug("object falling in loop more than 30 times", 0);
                extract_obj(obj);
                fall_count = 0;
                return;
        }

        if (IsSet(obj->in_room->RoomFlags, RoomNofloor)
            && CanGo(obj, DirDown) && !IsObjStat(obj, ItemMagic))
        {

                pexit = get_exit(obj->in_room, DirDown);
                to_room = pexit->to_room;

                if (through)
                        fall_count++;
                else
                        fall_count = 0;

                if (obj->in_room == to_room)
                {
                        snprintf(buf, MSL,
                                 "Object falling into same room, room %d",
                                 to_room->vnum);
                        bug(buf, 0);
                        extract_obj(obj);
                        return;
                }

                if (obj->in_room->first_person)
                {
                        act(AtPlain, "$p falls far below...",
                            obj->in_room->first_person, obj, NULL, ToRoom);
                        act(AtPlain, "$p falls far below...",
                            obj->in_room->first_person, obj, NULL, ToChar);
                }
                obj_from_room(obj);
                is_falling = TRUE;
                obj = obj_to_room(obj, to_room);
                is_falling = FALSE;

                if (obj->in_room->first_person)
                {
                        act(AtPlain, "$p falls from above...",
                            obj->in_room->first_person, obj, NULL, ToRoom);
                        act(AtPlain, "$p falls from above...",
                            obj->in_room->first_person, obj, NULL, ToChar);
                }

                if (!IsSet(obj->in_room->RoomFlags, RoomNofloor)
                    && through)
                {
/*		int dam = (int)9.81*sqrt(fall_count*2/9.81)*obj->weight/2;
*/ int dam = fall_count * obj->weight
                                / 2;

                        /*
                         * Damage players 
                         */
                        if (obj->in_room->first_person
                            && number_percent() > 15)
                        {
                                CharData *rch;
                                CharData *vch = NULL;
                                int       chcnt = 0;

                                for (rch = obj->in_room->first_person; rch;
                                     rch = rch->next_in_room, chcnt++)
                                        if (number_range(0, chcnt) == 0)
                                                vch = rch;
                                act(AtWhite, "$p falls on $n!", vch, obj,
                                    NULL, ToRoom);
                                act(AtWhite, "$p falls on you!", vch, obj,
                                    NULL, ToChar);
                                damage(vch, vch, dam * vch->top_level,
                                       TypeUndefined);
                        }
                        /*
                         * Damage objects 
                         */
                        switch (obj->item_type)
                        {
                        case ItemWeapon:
                        case ItemArmor:
                                if ((obj->value[0] - dam) <= 0)
                                {
                                        if (obj->in_room->first_person)
                                        {
                                                act(AtPlain,
                                                    "$p is destroyed by the fall!",
                                                    obj->in_room->
                                                    first_person, obj, NULL,
                                                    ToRoom);
                                                act(AtPlain,
                                                    "$p is destroyed by the fall!",
                                                    obj->in_room->
                                                    first_person, obj, NULL,
                                                    ToChar);
                                        }
                                        make_scraps(obj);
                                }
                                else
                                        obj->value[0] -= dam;
                                break;
                        default:
                                if ((dam * 15) > get_obj_resistance(obj))
                                {
                                        if (obj->in_room->first_person)
                                        {
                                                act(AtPlain,
                                                    "$p is destroyed by the fall!",
                                                    obj->in_room->
                                                    first_person, obj, NULL,
                                                    ToRoom);
                                                act(AtPlain,
                                                    "$p is destroyed by the fall!",
                                                    obj->in_room->
                                                    first_person, obj, NULL,
                                                    ToChar);
                                        }
                                        make_scraps(obj);
                                }
                                break;
                        }
                }
                obj_fall(obj, TRUE);
        }
        return;
}

CMDF do_hold(CharData * ch, char *argument)
{
        CharData *victim = NULL;

        if (argument[0] == '\0')
        {
                send_to_char("Hold what, or who?\n\r", ch);
                return;
        }
        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                do_wear(ch, argument);
                return;
        }
        else
        {
                do_hold_person(ch, argument);
                return;
        }
}
CMDF do_setcode(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      arg2[MaxInputLength];
        ObjData *obj = NULL;
        CharData *victim = NULL;

        if (argument[0] == '\0')
        {
                send_to_char
                        ("Usage: setcode <target> <old code> <new code>\n\r",
                         ch);
                send_to_char
                        ("Currently used to change the code on a binding with a keypad.\n\r",
                         ch);
                return;
        }
        argument = one_argument(argument, arg);
        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("You do not see that person here.\n\r", ch);
                return;
        }
        if ((obj = get_eq_char(victim, WearBinding)) == NULL)
        {
                act(AtAction, "But $E is not even bound!", ch, NULL, victim,
                    ToChar);
                return;
        }
        if (obj->value[4] == 0)
        {
                act(AtAction, "$N's $p lacks a keypad.", ch, obj, victim,
                    ToChar);
                return;
        }
        argument = one_argument(argument, arg2);
        if (arg2[0] == '\0' || argument[0] == '\0')
        {
                do_setcode(ch, const_cast<char*>(""));
                return;
        }
        if (obj->value[3] != atoi(arg2))
        {
                send_to_char("That doesn't seem to be the right code.\n\r",
                             ch);
                act(AtAction,
                    "$n presses a few buttons on $p, but nothing happens.",
                    ch, obj, victim, ToRoom);
                return;
        }
        else
        {
                obj->value[3] = atoi(argument);
                act(AtAction, "You press a few buttons on $p.", ch, obj,
                    victim, ToChar);
                ch_printf(ch, "Code changed to %d.\n\r", atoi(argument));
                act(AtAction,
                    "$n presses a few buttons on $p, and it beeps.", ch, obj,
                    victim, ToRoom);
                return;
        }
}

// ============================================================================
// Object Memory Management
// ============================================================================

void free_object(ObjData * obj)
{

        AffectData *paf, *next_paf;
        ExtraDescrData *ed, *next_ed;

        for (paf = obj->first_affect; paf; paf = next_paf)
        {
                next_paf = paf->next;
                DISPOSE(paf);
        }
        obj->first_affect = obj->last_affect = NULL;

        for (ed = obj->first_extradesc; ed; ed = next_ed)
        {
                next_ed = ed->next;
                STRFREE(ed->description);
                STRFREE(ed->keyword);
                DISPOSE(ed);
        }
        obj->first_extradesc = obj->last_extradesc = NULL;
        UNLINK(obj, first_object, last_object, next, prev);

        STRFREE(obj->name);
        STRFREE(obj->description);
        STRFREE(obj->short_descr);
        STRFREE(obj->armed_by);
        STRFREE(obj->action_desc);
        DISPOSE(obj);
}
