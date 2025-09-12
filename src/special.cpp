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
 *                               Special Functions Module                                *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>
#include "mud.hpp"
#include "bounty.hpp"
#include "races.hpp"
#include "olc_bounty.hpp"
#include "installations.hpp"

/* jails for wanted flags */

#define RoomJailCoruscant        0

SpecList *first_specfun;
SpecList *last_specfun;

bool is_wielding_weapon args((CharData * ch));

/* Simple load function - no OLC support for now.
 * This is probably something you DONT want builders playing with.
 */
void load_specfuns(void)
{
        SpecList *specfun;
        FILE     *fp;
        char      filename[256];

        first_specfun = NULL;
        last_specfun = NULL;

        snprintf(filename, 256, "%sspecfuns.dat", SystemDir);
        if (!(fp = fopen(filename, "r")))
        {
                bug("%s",
                    "load_specfuns: FATAL - cannot load specfuns.dat, exiting.");
                perror(filename);
                exit(1);
        }
        else
        {
                for (;;)
                {
                        if (feof(fp))
                        {
                                bug("%s",
                                    "load_specfuns: Premature end of file!");
                                FCLOSE(fp);
                                return;
                        }
                        CREATE(specfun, SpecList, 1);
                        specfun->name = fread_string_nohash(fp);
                        if (!str_cmp(specfun->name, "$"))
                                break;
                        LINK(specfun, first_specfun, last_specfun, next,
                             prev);
                }
                DISPOSE(specfun->name);
                DISPOSE(specfun);
                FCLOSE(fp);
        }
        return;
}

/* Simple validation function to be sure a function can be used on mobs */
bool validate_spec_fun(char *name)
{
        SpecList *specfun;

        for (specfun = first_specfun; specfun; specfun = specfun->next)
        {
                if (!str_cmp(specfun->name, name))
                        return TRUE;
        }
        return FALSE;
}

bool remove_obj args((CharData * ch, int iWear, bool fReplace));

/*
 * Given a name, return the appropriate spec_fun.
 */
SpecFun *spec_lookup(char *name)
{
        void     *funHandle;
        const char *error;

        funHandle = dlsym(sysdata.dlHandle, name);
        if ((error = dlerror()) != NULL)
        {
                bug("spec_lookup: Error locating function %s in symbol table.", name);
                return NULL;
        }
        return (SpecFun *) funHandle;
}



SPECF spec_newbie_pilot(CharData * ch)
{
        int       home = 32149;
        CharData *victim;
        CharData *v_next;
        ObjData *obj;
        char      buf[MaxStringLength];
        bool      diploma = FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                        if (obj->pIndexData->vnum == ObjVnumSchoolDiploma)
                                diploma = TRUE;

                if (!diploma)
                        continue;

                if (victim->race && victim->race->home() != -1)
                {
                        RoomIndexData *room =
                                get_room_index(victim->race->home());
                        home = victim->race->home();
                        if (room && room->area && room->area->planet)
                        {
                                snprintf(buf, MSL,
                                         "After a brief journey you arrive at %s's spaceport.",
                                         room->area->planet->name);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "After a brief journey you arrive at %s's spaceport.",
                                         room->name);
                        }
                        echo_to_room(AtAction, ch->in_room, buf);
                }
                else
                {
                        snprintf(buf, MSL, "Hmm, a %s.",
                                 victim->race->name());
                        do_look(ch, victim->name);
                        do_say(ch, buf);
                        do_say(ch,
                               "You're home planet is a little hard to get to right now.");
                        do_say(ch, "I'll take you to the Pluogus instead.");
                        echo_to_room(AtAction, ch->in_room,
                                     "After a brief journey the shuttle docks with the Serin Pluogus.\n\r\n\r");
                }
                char_from_room(victim);
                char_to_room(victim, get_room_index(home));

                do_look(victim, "");

                snprintf(buf, MSL,
                         "%s steps out and the shuttle quickly returns to the academy.\n\r",
                         victim->name);
                echo_to_room(AtAction, ch->in_room, buf);
        }

        return FALSE;
}

SPECF spec_player_elite_guard(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;
                if (!is_clan_enemy(ch, victim))
                        continue;
                do_say(ch, "Hey you're not allowed in here!");
                multi_hit(ch, victim, TypeUndefined);
                return TRUE;
        }
        return FALSE;
}

SPECF spec_installation_guard(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;

                if (is_clan_ally(ch, victim))
                        continue;

                if (!is_clan_enemy(ch, victim))
                        continue;

                do_say(ch, "Hey you're not allowed in here!");
                multi_hit(ch, victim, TypeUndefined);
                return TRUE;
        }
        return FALSE;
}

SPECF spec_installation_entrance_guard(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        InstallationData *installation;
        ExitData *pexit;


        if (!IsAwake(ch) || ch->position == PosFighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

				if (!can_see(ch, victim))
                        continue;

                if ((installation =
                     installation_from_room(ch->in_room->vnum)) == NULL)
                        return FALSE;

                if (is_clan_ally(ch, victim) || is_same_clan(ch, victim))
                        continue;

                for (pexit =
                     get_room_index(installation->first_room)->first_exit;
                     pexit; pexit = pexit->next)
                        if (pexit->to_room->vnum > installation->last_room
                            || pexit->to_room->vnum <
                            installation->first_room)
                                break;

                do_say(ch, "Hey you're not allowed in here! Leave, NOW!");
                char_from_room(victim);
                char_to_room(victim, pexit->to_room);
                do_look(victim, "auto");
                return TRUE;

        }

        return FALSE;
}

SPECF spec_installation_doctor(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        InstallationData *installation;


        if (!IsAwake(ch) || ch->position == PosFighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

                if (!is_clan_ally(ch, victim))
                        continue;

                if ((installation =
                     installation_from_room(ch->in_room->vnum)) == NULL)
                        return FALSE;

                if (victim->hit < victim->max_hit)
                {
                        do_say(ch,
                               "Hello there, you clearly need healing, lemme see what I can do.");
                        ch_printf(victim,
                                  "The doctor heals you, replenishing %d hit points.\n\r ",
                                  victim->max_hit / 10);
                        victim->hit += victim->max_hit / 10;
                        if (victim->hit > victim->max_hit)
                                victim->hit = victim->max_hit;
                }
                return TRUE;
        }

        return FALSE;
}

SPECF spec_jedi(CharData * ch)
{
        ch = NULL;
        return FALSE;
}



SPECF spec_clan_guard(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;
        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;

                if (!is_clan_enemy(ch, victim))
                        continue;
                if (is_same_clan(ch, victim))
                        continue;
                do_say(ch, "Hey you're not allowed in here!");
                multi_hit(ch, victim, TypeUndefined);
                return TRUE;
        }

        return FALSE;
}

SPECF spec_player_guard(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;
                if (!IsNpc(victim) && victim->PCData && IsAwake(victim)
                    && str_cmp(ch->leader->name, victim->name))
                {
                        do_say(ch, "Hey you're not allowed in here!");
                        multi_hit(ch, victim, TypeUndefined);
                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_clan_patrol(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;
                if (!IsNpc(victim) && victim->PCData && victim->PCData->clan
                    && IsAwake(victim) && (victim->top_level > 29))
                {
                        if (victim->PCData->clan->mainclan
                            && get_clan(ch->mob_clan)->mainclan)
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->mainclan->name,
                                     victim->PCData->clan->mainclan->name))
                                        return FALSE;
                        }
                        else if (victim->PCData->clan->mainclan)
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->name,
                                     victim->PCData->clan->mainclan->name))
                                        return FALSE;
                        }
                        else if (get_clan(ch->mob_clan)->mainclan)
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->mainclan->name,
                                     victim->PCData->clan->name))
                                        return FALSE;
                        }
                        else
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->name,
                                     victim->PCData->clan->name))
                                        return FALSE;
                        }


                        do_say(ch, "Hey you're not allowed around here!");
                        multi_hit(ch, victim, TypeUndefined);
                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_player_patrol(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;
                if (!IsNpc(victim) && victim->PCData && IsAwake(victim)
                    && (victim->top_level > 29)
                    && str_cmp(ch->leader->name, victim->name))
                {
                        do_say(ch, "Hey you're not allowed around here!");
                        multi_hit(ch, victim, TypeUndefined);
                        return TRUE;
                }
        }

        return FALSE;
}



SPECF spec_customs_smut(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        ObjData *obj;
        char      buf[MaxStringLength];
        long      ch_exp;

        if (!IsAwake(ch) || ch->position == PosFighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                {
                        if (obj->pIndexData->item_type == ItemSmut)
                        {
                                if (victim != ch && can_see(ch, victim)
                                    && can_see_obj(ch, obj))
                                {
                                        snprintf(buf, MSL,
                                                 "%s is illegal contraband. I'm going to have to confiscate that.",
                                                 obj->short_descr);
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WearNone)
                                                remove_obj(victim,
                                                           obj->wear_loc,
                                                           TRUE);
                                        separate_obj(obj);
                                        obj_from_char(obj);
                                        act(AtAction,
                                            "$n confiscates $p from $N.", ch,
                                            obj, victim, ToNotvict);
                                        act(AtAction,
                                            "$n takes $p from you.", ch, obj,
                                            victim, ToVict);
                                        obj = obj_to_char(obj, ch);
                                        SetBit(obj->extra_flags,
                                                ItemContraband);
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You lose %ld experience.\n\r ",
                                                  ch_exp);
                                        gain_exp(victim, 0 - ch_exp,
                                                 SmugglingAbility);
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)
                                         && !IsSet(obj->extra_flags,
                                                    ItemContraband))
                                {
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SmugglingAbility);

                                        act(AtAction,
                                            "$n looks at $N suspiciously.",
                                            ch, NULL, victim, ToNotvict);
                                        act(AtAction,
                                            "$n look at you suspiciously.",
                                            ch, NULL, victim, ToVict);
                                        SetBit(obj->extra_flags,
                                                ItemContraband);

                                        return TRUE;
                                }
                                else if (!IsSet
                                         (obj->extra_flags, ItemContraband))
                                {
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SmugglingAbility);

                                        SetBit(obj->extra_flags,
                                                ItemContraband);
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ItemContainer)
                        {
                                ObjData *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if (content->pIndexData->item_type ==
                                            ItemSmut
                                            && !IsSet(content->extra_flags,
                                                       ItemContraband))
                                        {
                                                ch_exp = UMin(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SmugglingAbility);
                                                SetBit(content->extra_flags,
                                                        ItemContraband);
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_customs_weapons(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
		char *mob_clan;
        ObjData *obj,*obj_prev;
        char      buf[MaxStringLength];
        long      ch_exp;

        if (!IsAwake(ch) || ch->position == PosFighting)
                return FALSE;

        mob_clan = ch->mob_clan;
        if (ch->in_room && ch->in_room->area->planet && ch->in_room->area->planet->governed_by)
                ch->mob_clan = ch->in_room->area->planet->governed_by->name;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

                if (is_clan_ally(ch, victim))
                        continue;

                for (obj = victim->last_carrying; obj; obj = obj_prev)
                {
                        obj_prev = obj->prev_content;

                        if (obj->item_type == ItemHolster && obj->first_content != NULL)
                             obj = obj->first_content;

                        if (obj->pIndexData->item_type == ItemWeapon
                            || obj->pIndexData->item_type == ItemGrenade)
                        {
                                if (victim != ch && can_see(ch, victim)
                                    && can_see_obj(ch, obj))
                                {
                                        snprintf(buf, MSL,
                                                 "Weapons are banned from non-military usage. I'm going to have to confiscate %s.",
                                                 obj->short_descr);
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WearNone)
                                                remove_obj(victim,
                                                           obj->wear_loc,
                                                           TRUE);
                                        separate_obj(obj);
                                        if (obj->in_obj) 
                                                obj_from_obj(obj);
                                        else 
                                                obj_from_char(obj);
                                        act(AtAction,
                                            "$n confiscates $p from $N.", ch,
                                            obj, victim, ToNotvict);
                                        act(AtAction,
                                            "$n takes $p from you.", ch, obj,
                                            victim, ToVict);
                                        obj = obj_to_char(obj, ch);
                                        SetBit(obj->extra_flags,
                                                ItemContraband);
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You lose %ld experience.\n\r ",
                                                  ch_exp);
                                        gain_exp(victim, 0 - ch_exp,
                                                 SmugglingAbility);
										ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)
                                         && !IsSet(obj->extra_flags,
                                                    ItemContraband))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %d.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SmugglingAbility);

                                        act(AtAction,
                                            "$n looks at $N suspiciously.",
                                            ch, NULL, victim, ToNotvict);
                                        act(AtAction,
                                            "$n look at you suspiciously.",
                                            ch, NULL, victim, ToVict);
                                        SetBit(obj->extra_flags,
                                                ItemContraband);
										ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (!IsSet
                                         (obj->extra_flags, ItemContraband))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SmugglingAbility);

                                        SetBit(obj->extra_flags,
                                                ItemContraband);
										ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ItemContainer)
                        {
                                ObjData *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if ((content->pIndexData->item_type ==
                                             ItemWeapon
                                             || content->pIndexData->
                                             item_type == ItemGrenade)
                                            && !IsSet(content->extra_flags,
                                                       ItemContraband))
                                        {
                                                separate_obj(content);
                                                ch_exp = UMin(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SmugglingAbility);
                                                SetBit(content->extra_flags,
                                                        ItemContraband);
												ch->mob_clan = mob_clan;
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_customs_alcohol(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        ObjData *obj;
        char      buf[MaxStringLength];
        int       liquid;
        long      ch_exp;

        if (!IsAwake(ch) || ch->position == PosFighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                {
                        if (obj->pIndexData->item_type == ItemDrinkCon)
                        {
                                if ((liquid = obj->value[2]) >= LiqMax)
                                        liquid = obj->value[2] = 0;

                                if (liq_table[liquid].liq_affect[CondDrunk] >
                                    0)
                                {
                                        if (victim != ch
                                            && can_see(ch, victim)
                                            && can_see_obj(ch, obj))
                                        {
                                                snprintf(buf, MSL,
                                                         "%s is illegal contraband. I'm going to have to confiscate that.",
                                                         obj->short_descr);
                                                do_say(ch, buf);
                                                if (obj->wear_loc !=
                                                    WearNone)
                                                        remove_obj(victim,
                                                                   obj->
                                                                   wear_loc,
                                                                   TRUE);
                                                separate_obj(obj);
                                                obj_from_char(obj);
                                                act(AtAction,
                                                    "$n confiscates $p from $N.",
                                                    ch, obj, victim,
                                                    ToNotvict);
                                                act(AtAction,
                                                    "$n takes $p from you.",
                                                    ch, obj, victim, ToVict);
                                                obj = obj_to_char(obj, ch);
                                                SetBit(obj->extra_flags,
                                                        ItemContraband);
                                                ch_exp = UMin(obj->cost * 10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You lose %ld experience. \n\r",
                                                          ch_exp);
                                                gain_exp(victim, 0 - ch_exp,
                                                         SmugglingAbility);
                                                return TRUE;
                                        }
                                        else if (can_see(ch, victim)
                                                 && !IsSet(obj->extra_flags,
                                                            ItemContraband))
                                        {
                                                ch_exp = UMin(obj->cost * 10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %d. \n\r",
                                                          ch_exp,
                                                          obj->short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SmugglingAbility);

                                                act(AtAction,
                                                    "$n looks at $N suspiciously.",
                                                    ch, NULL, victim,
                                                    ToNotvict);
                                                act(AtAction,
                                                    "$n look at you suspiciously.",
                                                    ch, NULL, victim,
                                                    ToVict);
                                                SetBit(obj->extra_flags,
                                                        ItemContraband);
                                                return TRUE;
                                        }
                                        else if (!IsSet
                                                 (obj->extra_flags,
                                                  ItemContraband))
                                        {
                                                ch_exp = UMin(obj->cost * 10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %d. \n\r",
                                                          ch_exp,
                                                          obj->short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SmugglingAbility);

                                                SetBit(obj->extra_flags,
                                                        ItemContraband);
                                                return TRUE;
                                        }
                                }
                        }
                        else if (obj->item_type == ItemContainer)
                        {
                                ObjData *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if (content->pIndexData->item_type ==
                                            ItemDrinkCon
                                            && !IsSet(content->extra_flags,
                                                       ItemContraband))
                                        {
                                                if ((liquid =
                                                     obj->value[2]) >=
                                                    LiqMax)
                                                        liquid = obj->
                                                                value[2] = 0;
                                                if (liq_table[liquid].
                                                    liq_affect[CondDrunk] <=
                                                    0)
                                                        continue;
                                                ch_exp = UMin(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %d.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SmugglingAbility);
                                                SetBit(content->extra_flags,
                                                        ItemContraband);
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_customs_spice(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        ObjData *obj;
        char      buf[MaxStringLength];
        long      ch_exp;

        if (!IsAwake(ch) || ch->position == PosFighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                {
                        if (obj->pIndexData->item_type == ItemSpice
                            || obj->pIndexData->item_type == ItemRawspice)
                        {
                                if (victim != ch && can_see(ch, victim)
                                    && can_see_obj(ch, obj))
                                {
                                        snprintf(buf, MSL,
                                                 "%s is illegal contraband. I'm going to have to confiscate that.",
                                                 obj->short_descr);
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WearNone)
                                                remove_obj(victim,
                                                           obj->wear_loc,
                                                           TRUE);
                                        separate_obj(obj);
                                        obj_from_char(obj);
                                        act(AtAction,
                                            "$n confiscates $p from $N.", ch,
                                            obj, victim, ToNotvict);
                                        act(AtAction,
                                            "$n takes $p from you.", ch, obj,
                                            victim, ToVict);
                                        obj = obj_to_char(obj, ch);
                                        SetBit(obj->extra_flags,
                                                ItemContraband);
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You lose %ld experience. \n\r",
                                                  ch_exp);
                                        gain_exp(victim, 0 - ch_exp,
                                                 SmugglingAbility);
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)
                                         && !IsSet(obj->extra_flags,
                                                    ItemContraband))
                                {
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s. \n\r",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SmugglingAbility);

                                        act(AtAction,
                                            "$n looks at $N suspiciously.",
                                            ch, NULL, victim, ToNotvict);
                                        act(AtAction,
                                            "$n look at you suspiciously.",
                                            ch, NULL, victim, ToVict);
                                        SetBit(obj->extra_flags,
                                                ItemContraband);
                                        return TRUE;
                                }
                                else if (!IsSet
                                         (obj->extra_flags, ItemContraband))
                                {
                                        ch_exp = UMin(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SmugglingAbility] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SmugglingAbility])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s. \n\r",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SmugglingAbility);

                                        SetBit(obj->extra_flags,
                                                ItemContraband);
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ItemContainer)
                        {
                                ObjData *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if (content->pIndexData->item_type ==
                                            ItemSpice
                                            && !IsSet(content->extra_flags,
                                                       ItemContraband))
                                        {
                                                ch_exp = UMin(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SmugglingAbility);
                                                SetBit(content->extra_flags,
                                                        ItemContraband);
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_police(CharData * ch)
{
        RoomIndexData *jail = NULL;
        CharData *victim;
        CharData *v_next;
        char      buf[MaxStringLength];

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        if (!ch->in_room || !ch->in_room->area || !ch->in_room->area->planet)
                return FALSE;

        if (ch->in_room->area->planet->governed_by)
        {
                ClanData *clan = ch->in_room->area->planet->governed_by;

                jail = get_room_index(clan->jail);
        }

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (IsNpc(victim))
                        continue;
                if (!can_see(ch, victim))
                        continue;
                if (number_bits(1) == 0)
                        continue;

                if (is_wanted(victim, ch->in_room->area->planet))
                {
                        snprintf(buf, MSL, "Hey you're wanted on %s!",
                                 ch->in_room->area->planet->name);
                        do_say(ch, buf);

                        /*
                         * currently no jails 
                         */

                        if (jail
                            && (victim->position == PosIncap
                                || victim->position == PosStunned
                                || victim->position == PosSleeping))
                        {
                                act(AtAction, "$n ushers $N off to jail.",
                                    ch, NULL, victim, ToNotvict);
                                act(AtAction, "$n escorts you to jail.", ch,
                                    NULL, victim, ToVict);
                                char_from_room(victim);
                                char_to_room(victim, jail);
                                remove_wanted_planet(victim,
                                                     ch->in_room->area->
                                                     planet);
                        }
                        else if (ch->top_level >= victim->top_level
                                 && !IsSet(ch->in_room->RoomFlags,
                                             RoomSafe))
                        {
                                command_printf(ch,
                                               "yell Hey %s is wanted on %s!",
                                               victim->name,
                                               ch->in_room->area->planet->
                                               name);
                                multi_hit(ch, victim, TypeUndefined);
                                /*
                                 * remove_wanted_planet(victim,
                                 * ch->in_room->area->
                                 * planet);
                                 */
                        }
                        else
                        {
                                act(AtAction,
                                    "$n fines $N an enormous amount of money.",
                                    ch, NULL, victim, ToNotvict);
                                act(AtAction,
                                    "$n fines you an enourmous amount of money.",
                                    ch, NULL, victim, ToVict);
                                victim->gold *= 9;
                                victim->gold /= 10;
                                remove_wanted_planet(victim,
                                                     ch->in_room->area->
                                                     planet);
                        }

                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_jedi_healer(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (victim != ch && can_see(ch, victim)
                    && number_bits(1) == 0)
                        break;
        }

        if (!victim)
                return FALSE;

        switch (number_bits(12))
        {
        case 0:
                act(AtMagic, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, ToRoom);
                spell_smaug(skill_lookup("Armor"), ch->top_level, ch, victim);
                return TRUE;

        case 1:
                act(AtMagic, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, ToRoom);
                spell_smaug(skill_lookup("good fortune"), ch->top_level, ch,
                            victim);
                return TRUE;

        case 2:
                act(AtMagic, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, ToRoom);
                spell_cure_blindness(skill_lookup("cure blindness"),
                                     ch->top_level, ch, victim);
                return TRUE;

        case 3:
                act(AtMagic, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, ToRoom);
                spell_smaug(skill_lookup("cure light"), ch->top_level, ch,
                            victim);
                return TRUE;

        case 4:
                act(AtMagic, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, ToRoom);
                spell_cure_poison(skill_lookup("cure poison"), ch->top_level,
                                  ch, victim);
                return TRUE;

        case 5:
                act(AtMagic, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, ToRoom);
                spell_smaug(skill_lookup("refresh"), ch->top_level, ch,
                            victim);
                return TRUE;

        }

        return FALSE;
}



SPECF spec_dark_jedi(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        char     *spell;
        int       sn;


        if (ch->position != PosFighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (who_fighting(victim) && number_bits(2) == 0)
                        break;
        }

        if (!victim || victim == ch)
                return FALSE;

        for (;;)
        {
                int       min_level;

                switch (number_bits(4))
                {
                case 0:
                        min_level = 5;
                        spell = "blindness";
                        break;
                case 1:
                        min_level = 5;
                        spell = "fingers of the Force";
                        break;
                case 2:
                        min_level = 9;
                        spell = "choke";
                        break;
                case 3:
                        min_level = 8;
                        spell = "invade essence";
                        break;
                case 4:
                        min_level = 11;
                        spell = "Force projectile";
                        break;
                case 6:
                        min_level = 13;
                        spell = "drain essence";
                        break;
                case 7:
                        min_level = 4;
                        spell = "Force whip";
                        break;
                case 8:
                        min_level = 13;
                        spell = "harm";
                        break;
                case 9:
                        min_level = 9;
                        spell = "Force bolt";
                        break;
                case 10:
                        min_level = 1;
                        spell = "Force spray";
                        break;
                default:
                        return FALSE;
                }

                if (ch->top_level >= min_level)
                        break;
        }

        if ((sn = skill_lookup(spell)) < 0)
                return FALSE;
        (*skill_table[sn]->spell_fun) (sn, ch->top_level, ch, victim);
        return TRUE;
}



SPECF spec_fido(CharData * ch)
{
        ObjData *corpse;
        ObjData *c_next;
        ObjData *obj;
        ObjData *obj_next;

        if (!IsAwake(ch))
                return FALSE;

        for (corpse = ch->in_room->first_content; corpse; corpse = c_next)
        {
                c_next = corpse->next_content;
                if (corpse->item_type != ItemCorpseNpc)
                        continue;

                act(AtAction, "$n savagely devours a corpse.", ch, NULL,
                    NULL, ToRoom);
                for (obj = corpse->first_content; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        obj_from_obj(obj);
                        obj_to_room(obj, ch->in_room);
                }
                extract_obj(corpse);
                return TRUE;
        }

        return FALSE;
}

SPECF spec_stormtrooper(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;
                if ((IsNpc(victim) && nifty_is_name("republic", victim->name)
                     && victim->fighting && who_fighting(victim) != ch) ||
                    (!IsNpc(victim) && victim->PCData && victim->PCData->clan
                     && IsAwake(victim)
                     && nifty_is_name("republic",
                                      victim->PCData->clan->name)))
                {
                        do_say(ch, "Die Rebel Scum!");
                        multi_hit(ch, victim, TypeUndefined);
                        return TRUE;
                }

        }

        return FALSE;

}

SPECF spec_new_republic_trooper(CharData * ch)
{
        CharData *victim;
        CharData *v_next;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TimerRecentfight) > 0)
                        continue;
                if ((IsNpc(victim) && nifty_is_name("imperial", victim->name)
                     && victim->fighting && who_fighting(victim) != ch) ||
                    (!IsNpc(victim) && victim->PCData && victim->PCData->clan
                     && IsAwake(victim)
                     && nifty_is_name("empire", victim->PCData->clan->name)))
                {
                        do_say(ch, "Long live the New Republic!");
                        multi_hit(ch, victim, TypeUndefined);
                        return TRUE;
                }

        }

        return FALSE;

}


SPECF spec_guardian(CharData * ch)
{
        char      buf[MaxStringLength];
        CharData *victim;
        CharData *v_next;
        CharData *ech;
        char     *crime;
        int       max_evil;

        if (!IsAwake(ch) || ch->fighting)
                return FALSE;

        max_evil = 300;
        ech = NULL;
        crime = "";

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (victim->fighting
                    && who_fighting(victim) != ch
                    && victim->alignment < max_evil)
                {
                        max_evil = victim->alignment;
                        ech = victim;
                }
        }

        if (victim && IsSet(ch->in_room->RoomFlags, RoomSafe))
        {
                snprintf(buf, MSL, "%s is a %s!  As well as a COWARD!",
                         victim->name, crime);
                do_say(ch, buf);
                return TRUE;
        }

        if (victim)
        {
                snprintf(buf, MSL, "%s is a %s!  PROTECT THE INNOCENT!!",
                         victim->name, crime);
                do_shout(ch, buf);
                multi_hit(ch, victim, TypeUndefined);
                return TRUE;
        }

        if (ech)
        {
                act(AtYell, "$n screams 'PROTECT THE INNOCENT!!",
                    ch, NULL, NULL, ToRoom);
                multi_hit(ch, ech, TypeUndefined);
                return TRUE;
        }

        return FALSE;
}



SPECF spec_janitor(CharData * ch)
{
        ObjData *trash;
        ObjData *trash_next;

        if (!IsAwake(ch))
                return FALSE;

        for (trash = ch->in_room->first_content; trash; trash = trash_next)
        {
                trash_next = trash->next_content;
                if (!IsSet(trash->wear_flags, ItemTake)
                    || IsObjStat(trash, ItemBurried))
                        continue;
                if (trash->item_type == ItemDrinkCon
                    || trash->item_type == ItemTrash
                    || trash->cost < 10
                    || (trash->pIndexData->vnum == ObjVnumShoppingBag
                        && !trash->first_content))
                {
                        act(AtAction, "$n picks up some trash.", ch, NULL,
                            NULL, ToRoom);
                        obj_from_room(trash);
                        obj_to_char(trash, ch);
                        return TRUE;
                }
        }

        return FALSE;
}



SPECF spec_poison(CharData * ch)
{
        CharData *victim;

        if (ch->position != PosFighting
            || (victim = who_fighting(ch)) == NULL
            || number_percent() > 2 * ch->top_level)
                return FALSE;

        act(AtHit, "You bite $N!", ch, NULL, victim, ToChar);
        act(AtAction, "$n bites $N!", ch, NULL, victim, ToNotvict);
        act(AtPoison, "$n bites you!", ch, NULL, victim, ToVict);
        spell_poison(gsn_poison, ch->top_level, ch, victim);
        return TRUE;
}



SPECF spec_thief(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        int       gold, maxgold;

        if (ch->position != PosStanding)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || get_trust(victim) >= LevelImmortal || number_bits(2) != 0 || !can_see(ch, victim))   /* Thx Glop */
                        continue;

                if (IsAwake(victim) && number_range(0, ch->top_level) == 0)
                {
                        act(AtAction,
                            "You discover $n's hands in your wallet!", ch,
                            NULL, victim, ToVict);
                        act(AtAction,
                            "$N discovers $n's hands in $S wallet!", ch, NULL,
                            victim, ToNotvict);
                        return TRUE;
                }
                else
                {
                        maxgold = ch->top_level * ch->top_level * 1000;
                        gold = victim->gold
                                * number_range(1,
                                               URange(2, ch->top_level / 4,
                                                      10)) / 100;
                        ch->gold += 9 * gold / 10;
                        victim->gold -= gold;
                        if (ch->gold > maxgold)
                        {
                                boost_economy(ch->in_room->area,
                                              ch->gold - maxgold / 2);
                                ch->gold = maxgold / 2;
                        }
                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_auth(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
        char      buf[MaxStringLength];
        ObjIndexData *pObjIndex;
        ObjData *obj;
        bool      hasdiploma;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (IsNpc(victim))
                        continue;

                /*
                 * Prevent people who need name changes from being authed 
                 */
                if (victim->PCData->AuthState == 2)
                        continue;
                if (!IsNpc(victim)
                    && (pObjIndex =
                        get_obj_index(ObjVnumSchoolDiploma)) != NULL)
                {
                        hasdiploma = FALSE;

                        for (obj = victim->last_carrying; obj;
                             obj = obj->prev_content)
                                if (obj->pIndexData ==
                                    get_obj_index(ObjVnumSchoolDiploma))
                                        hasdiploma = TRUE;

                        if (!hasdiploma)
                        {
                                obj = create_object(pObjIndex, 1);
                                obj = obj_to_char(obj, victim);
                                send_to_char
                                        ("&cThe schoolmaster gives you a diploma, and shakes your hand.\n\r&w",
                                         victim);
                        }
                }

                if (IsNpc(victim)
                    || !IsSet(victim->PCData->flags, PcflagUnauthed))
                        continue;

                victim->PCData->AuthState = 3;
                RemoveBit(victim->PCData->flags, PcflagUnauthed);
                if (victim->PCData->authed_by)
                        STRFREE(victim->PCData->authed_by);
                victim->PCData->authed_by = QUICKLINK(ch->name);
                snprintf(buf, MSL, "%s authorized %s", ch->name,
                         victim->name);


        }
        return FALSE;

}

SPECF spec_questmaster(CharData * ch)
{
        if (!IsNpc(ch))
                return FALSE;
        else
                return TRUE;
}

/**
 * Checks to see if a mob is in the room, or the corpse is given to it?
 * if the mob is bound, then reward and dropped bounds,
 * reward is based on olc list (mob vnum, corpse vnum, bounty alive, bounty dead)
 * 
 * If the player enters the room and is bound by someone, and person is bountied, pay out bounder and jail bounty.
 *
 * (Halkeye) OOC: found bound mobs, its easy
 * (Halkeye) OOC: but for corpses, should it only be if they are given to the guy right? or should he automaticly assume any corpses in a players inv are for him?
 * (Halkeye) OOC: and since when can you put like a corpse in a backpack?
 * (Cinna) OOC: he should assume that the corpses are for him
 * (Cinna) OOC: because who else are they going too?
 * (Halkeye) OOC: yea .. well its kinda weird for a mob to just take your corpses from you
 * (Halkeye) OOC: and for that matter for you to pick up more than one corpse at all
 *
 * Decided that its a per mob list
 * map<vnum> => list
 *
 * @param npc -> CharData pointer to the npc with the spec flagged turned on
 */
SPECF spec_bountyhunter(CharData * npc)
{
        CharData *victim;
        CharData *v_next = NULL;
        OlcBountyData *bounty;
        CharData *imob;
        char      buf[MSL];

        /*
         * Only NPC suppport for now 
         */
        /*
         * Check for mobs that are being held by a player.
         */
        for (victim = npc->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (victim->holding == NULL)
                        continue;
                if (!IsNpc(victim->holding))
                        continue;
                if ((bounty = has_olc_bounty(victim->holding)) == NULL)
                        continue;
                if (bounty->type() == BountyAlive)
                {
                        imob = victim->holding;
                        do_release(victim, "");
                        command_printf(npc,
                                       "sayto %s Well done %s, you've captured %s.",
                                       victim->name, victim->race->name(),
                                       imob->short_descr);
                        act(AtAction, "$n ushers $N off to be processed.",
                            npc, NULL, imob, ToRoom);
                        command_printf(npc,
                                       "sayto %s Here is the payment for the capture.",
                                       victim->name);
                        sprintf(buf, "$n give you %d credits.",
                                bounty->amount());
                        act(AtAction, buf, npc, NULL, victim, ToVict);
                        victim->gold += bounty->amount();
                        gain_exp(victim, bounty->experience(),
                                 HuntingAbility);
                        ch_printf(victim,
                                  "You gain %d bounty hunting experience.",
                                  bounty->experience());
                        command_printf(npc, "mppurge %s", imob->short_descr);
                        return TRUE;
                }
        }
        return TRUE;
}

bool obj_is_contraband(ObjData * obj) 
{
        if (obj->pIndexData->item_type == ItemSmut) {
                return TRUE;
        }

        if (obj->pIndexData->item_type == ItemDrinkCon)
        {
                int liquid;
                if ((liquid = obj->value[2]) >= LiqMax)
                        liquid = obj->value[2] = 0;
                
                if (liq_table[liquid].liq_affect[CondDrunk] > 0)
                {
                        // Put message here
                        return TRUE;
                }
        }

        if (obj->pIndexData->item_type == ItemSpice || obj->pIndexData->item_type == ItemRawspice)
        {
                return TRUE;
        }

        if (obj->pIndexData->item_type == ItemWeapon || obj->pIndexData->item_type == ItemGrenade)
        {
                return TRUE;
        }
        return FALSE;
}

SPECF spec_customs(CharData * ch)
{
        CharData *victim;
        CharData *v_next;
		char *mob_clan;
        ObjData *obj,*obj_prev;
        char      buf[MaxStringLength];
        long      ch_exp;

        if (!IsAwake(ch) || ch->position == PosFighting)
                return FALSE;

        mob_clan = ch->mob_clan;
        if (ch->in_room && ch->in_room->area->planet && ch->in_room->area->planet->governed_by)
                ch->mob_clan = ch->in_room->area->planet->governed_by->name;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IsNpc(victim) || victim->position == PosFighting)
                        continue;

                if (is_clan_ally(ch, victim))
                        continue;

                for (obj = victim->last_carrying; obj; obj = obj_prev)
                {
                        obj_prev = obj->prev_content;

                        if (obj->item_type == ItemHolster && obj->first_content != NULL)
                             obj = obj->first_content;
                        
                        if (obj_is_contraband(obj)) {
                                if (victim != ch && can_see(ch, victim) && can_see_obj(ch, obj))
                                {
                                        if (ch->in_room->area->planet) {
                                                snprintf(buf, MSL,
                                                         "%s's are banned from this planet. I'm going to have to confiscate %s.",
                                                         capitalize(o_types[obj->item_type]), obj->short_descr);
                                        }
                                        else {
                                                snprintf(buf, MSL,
                                                         "%s's are banned from here. I'm going to have to confiscate %s.",
                                                         capitalize(o_types[obj->item_type]), obj->short_descr);
                                        }
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WearNone)
                                                remove_obj(victim,obj->wear_loc,TRUE);
                                        separate_obj(obj);
                                        if (obj->in_obj) 
                                                obj_from_obj(obj);
                                        else 
                                                obj_from_char(obj);
                                        act(AtAction,"$n confiscates $p from $N.", ch,obj, victim, ToNotvict);
                                        act(AtAction,"$n takes $p from you.", ch, obj,victim, ToVict);
                                        obj = obj_to_char(obj, ch);
                                        SetBit(obj->extra_flags,ItemContraband);
                                        ch_exp = UMin(obj->cost * 10,(exp_level(victim->skill_level[SmugglingAbility] +1) -exp_level(victim->skill_level[SmugglingAbility])));
                                        ch_printf(victim,"You lose %ld experience.\n\r ",ch_exp);
                                        gain_exp(victim, 0 - ch_exp,SmugglingAbility);
                                        ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)&& !IsSet(obj->extra_flags,ItemContraband))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMin(obj->cost * 10,(exp_level(victim->skill_level[SmugglingAbility] +1) -exp_level(victim->skill_level[SmugglingAbility])));
                                        ch_printf(victim,"You receive %ld experience for smuggling %d.\n\r ",ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,SmugglingAbility);

                                        act(AtAction,"$n looks at $N suspiciously.",ch, NULL, victim, ToNotvict);
                                        act(AtAction,"$n look at you suspiciously.",ch, NULL, victim, ToVict);
                                        SetBit(obj->extra_flags,ItemContraband);
                                        ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (!IsSet(obj->extra_flags, ItemContraband))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMin(obj->cost * 10,(exp_level(victim->skill_level[SmugglingAbility] +1) -exp_level(victim->skill_level[SmugglingAbility])));
                                        ch_printf(victim,"You receive %ld experience for smuggling %s.\n\r ",ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,SmugglingAbility);

                                        SetBit(obj->extra_flags,ItemContraband);
                                        ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ItemContainer)
                        {
                                ObjData *content;

                                for (content = obj->first_content; content; content = content->next_content)
                                {
                                        if (obj_is_contraband(content) && !IsSet(content->extra_flags, ItemContraband))
                                        {
                                                separate_obj(content);
                                                ch_exp = UMin(content->cost *10,(exp_level(victim->skill_level[SmugglingAbility] + 1) -
                                                               exp_level(victim->skill_level[SmugglingAbility])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,content->short_descr);
                                                gain_exp(victim, ch_exp,SmugglingAbility);
                                                SetBit(content->extra_flags,ItemContraband);
                                                ch->mob_clan = mob_clan;
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}
