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
 *                                Combat Module                                          *
 ****************************************************************************************/
 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#if defined(__CYGWIN__)
#include <io.h>
#else
#include <sys/dir.h>
#endif

#include "mud.hpp"
#ifdef ACCOUNT
#include "Account.hpp"
#endif
#include "msp.hpp"
#include "bounty.hpp"
#include "races.hpp"

extern char lastplayercmd[MaxInputLength];
extern CharData *gch_prev;

/* From Skills.c */
int       ris_save(CharData * ch, int percent_chance, int ris);

/* From arena.c */
bool arena_can_fight args((CharData * ch, CharData * victim));
void win_fight args((CharData * winner, CharData * looser));
bool in_arena args((CharData * ch));

/*
 * Local functions.
 */
void dam_message args((CharData * ch, CharData * victim, int dam, int dt));
void group_gain args((CharData * ch, CharData * victim));
int xp_compute args((CharData * gch, CharData * victim));
int align_compute args((CharData * gch, CharData * victim));
ch_ret one_hit args((CharData * ch, CharData * victim, int dt));
int obj_hitroll args((ObjData * obj));
bool      get_cover(CharData * ch);
bool      dual_flip = FALSE;

DeclareDoFun(do_draw);
DeclareDoFun(do_holster);
/* 
 * Check to see if someone is weilding a weapon
 */
                
#define IsWeapon(obj) ((obj)->pIndexData->item_type == ItemWeapon || (obj)->pIndexData->item_type == ItemGrenade)

bool is_wielding_weapon(CharData * ch)
{
        ObjData *obj;
        if ((obj = get_eq_char(ch, WearDualWield)) != NULL)
                if (IsWeapon(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WearWield)) != NULL)
                if (IsWeapon(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WearHold)) != NULL)
                if (IsWeapon(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WearMissileWield)) != NULL)
                if (IsWeapon(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WearLight)) != NULL)
                if (IsWeapon(obj))
                        return TRUE;
        return FALSE;  /* Return FALSE if no weapon was found in any slot */
}
/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned(CharData * ch)
{
        ObjData *obj;

        if ((obj = get_eq_char(ch, WearWield))
            && (IsSet(obj->extra_flags, ItemPoisoned)))
                return TRUE;
        if ((obj = get_eq_char(ch, WearDualWield))
            && (IsSet(obj->extra_flags, ItemPoisoned)))
                return TRUE;

        return FALSE;

}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting(CharData * ch, CharData * victim)
{
        if (!ch->hunting || ch->hunting->who != victim)
                return FALSE;

        return TRUE;
}

bool is_hating(CharData * ch, CharData * victim)
{
        if (!ch->hating || ch->hating->who != victim)
                return FALSE;

        return TRUE;
}

bool is_fearing(CharData * ch, CharData * victim)
{
        if (!ch->fearing || ch->fearing->who != victim)
                return FALSE;

        return TRUE;
}

void stop_hunting(CharData * ch)
{
        if (ch->hunting)
        {
                STRFREE(ch->hunting->name);
                DISPOSE(ch->hunting);
                ch->hunting = NULL;
        }
        return;
}

void stop_hating(CharData * ch)
{
        if (ch->hating)
        {
                STRFREE(ch->hating->name);
                DISPOSE(ch->hating);
                ch->hating = NULL;
        }
        return;
}

void stop_fearing(CharData * ch)
{
        if (ch->fearing)
        {
                STRFREE(ch->fearing->name);
                DISPOSE(ch->fearing);
                ch->fearing = NULL;
        }
        return;
}

void start_hunting(CharData * ch, CharData * victim)
{
        if (ch->hunting)
                stop_hunting(ch);

        CREATE(ch->hunting, HHFData, 1);
        ch->hunting->name = QUICKLINK(victim->name);
        ch->hunting->who = victim;
        return;
}

void start_hating(CharData * ch, CharData * victim)
{
        if (ch->hating)
                stop_hating(ch);

        CREATE(ch->hating, HHFData, 1);
        ch->hating->name = QUICKLINK(victim->name);
        ch->hating->who = victim;
        return;
}

void start_fearing(CharData * ch, CharData * victim)
{
        if (ch->fearing)
                stop_fearing(ch);

        CREATE(ch->fearing, HHFData, 1);
        ch->fearing->name = QUICKLINK(victim->name);
        ch->fearing->who = victim;
        return;
}


int max_fight(CharData * ch)
{
        ch = NULL;
        return 8;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 */
void violence_update(void)
{
        char      buf[MaxStringLength];
        CharData *ch;
        CharData *lst_ch;
        CharData *victim;
        CharData *rch, *rch_next;
        AffectData *paf, *paf_next;
        Timer    *timer, *timer_next;
        ch_ret    retcode;
        SkillType *skill;
        int       sn;

        lst_ch = NULL;
        for (ch = last_char; ch; lst_ch = ch, ch = gch_prev)
        {

                if (ch == first_char && ch->prev)
                {
                        bug("ERROR: first_char->prev != NULL, fixing...", 0);
                        ch->prev = NULL;
                }

                gch_prev = ch->prev;

                if (gch_prev && gch_prev->next != ch)
                {
                        snprintf(buf, MSL,
                                 "FATAL: violence_update: %s->prev->next doesn't point to ch.",
                                 ch->name);
                        bug(buf, 0);
                        bug("Short-cutting here", 0);
                        ch->prev = NULL;
                        gch_prev = NULL;
                        do_shout(ch, "Thoric says, 'Prepare for the worst!'");
                }

                /*
                 * See if we got a pointer to someone who recently died...
                 * if so, either the pointer is bad... or it's a player who
                 * "died", and is back at the healer...
                 * Since he/she's in the char_list, it's likely to be the later...
                 * and should not already be in another fight already
                 */
                if (char_died(ch))
                        continue;

                /*
                 * See if we got a pointer to some bad looking data...
                 */
                if (!ch->in_room || !ch->name)
                {
                        log_string
                                ("violence_update: bad ch record!  (Shortcutting.)");
                        snprintf(buf, MSL,
                                 "ch: %p  ch->in_room: %p  ch->prev: %p  ch->next: %p",
                                 (void*)ch, (void*)ch->in_room, (void*)ch->prev,
                                 (void*)ch->next);
                        log_string(buf);
                        log_string(lastplayercmd);
                        if (lst_ch)
                                snprintf(buf, MSL,
                                         "lst_ch: %p  lst_ch->prev: %p  lst_ch->next: %p",
                                         (void*)lst_ch, (void*)lst_ch->prev,
                                         (void*)lst_ch->next);
                        else
                                mudstrlcpy(buf, "lst_ch: NULL", MSL);
                        log_string(buf);
                        gch_prev = NULL;
                        continue;
                }

                /*
                 * Experience gained during battle deceases as battle drags on
                 */
                if (ch->fighting)
                        if ((++ch->fighting->duration % 24) == 0)
                                ch->fighting->xp =
                                        ((ch->fighting->xp * 9) / 10);


                for (timer = ch->first_timer; timer; timer = timer_next)
                {
                        timer_next = timer->next;

                        for (sn = 0; sn < top_sn && skill_table[sn]; sn++)
                                if (*skill_table[sn]->skill_fun ==
                                    timer->do_fun)
                                        break;

                        /*
                         * There's a bug here, about sn not being Valid, but i can't think of how its explitable 
                         */
                        if (--timer->count <= 0
                            || (!IsNpc(ch) &&
                                skill_table[sn] && skill_table[sn]->guild ==
                                EngineeringAbility
                                && IsSet(ch->PCData->flags,
                                          PcflagFastengineer)))
                        {
                                if (timer->type == TimerDoFun)
                                {
                                        int       tempsub;

                                        tempsub = ch->substate;
                                        ch->substate = timer->value;
                                        (timer->do_fun) (ch, "");
                                        if (char_died(ch))
                                                break;
                                        ch->substate = tempsub;
                                }
                                /*
                                 * if ( timer->count == 0 ) 
                                 */
                                extract_timer(ch, timer);
                        }
                }

                if (char_died(ch))
                        continue;

                /*
                 * We need spells that have shorter durations than an hour.
                 * So a melee round sounds good to me... -Thoric
                 */
                for (paf = ch->first_affect; paf; paf = paf_next)
                {
                        paf_next = paf->next;
                        if (paf->duration > 0)
                                paf->duration--;
                        else if (paf->duration < 0)
                                ;
                        else
                        {
                                if (!paf_next
                                    || paf_next->type != paf->type
                                    || paf_next->duration > 0)
                                {
                                        skill = get_skilltype(paf->type);
                                        if (paf->type > 0 && skill
                                            && skill->msg_off)
                                        {
                                                set_char_color(AtWearoff,
                                                               ch);
                                                send_to_char(skill->msg_off,
                                                             ch);
                                                send_to_char("\n\r", ch);
                                        }
                                }
                                if (paf->type == gsn_possess)
                                {
                                        ch->desc->character =
                                                ch->desc->original;
                                        ch->desc->original = NULL;
                                        ch->desc->character->desc = ch->desc;
                                        ch->desc->character->switched = NULL;
                                        ch->desc = NULL;
                                }
                                affect_remove(ch, paf);
                        }
                }

                if ((victim = who_fighting(ch)) == NULL
                    || IsAffected(ch, AffParalysis))
                        continue;

                retcode = rNONE;

                if (IsSet(ch->in_room->RoomFlags, RoomSafe))
                {
                        snprintf(buf, MSL,
                                 "violence_update: %s fighting %s in a SAFE room.",
                                 ch->name, victim->name);
                        log_string(buf);
                        stop_fighting(ch, TRUE);
                }
                else if (IsAwake(ch) && ch->in_room == victim->in_room)
                        retcode = multi_hit(ch, victim, TypeUndefined);
                else
                        stop_fighting(ch, FALSE);

                if (char_died(ch))
                        continue;

                if (retcode == rCHAR_DIED
                    || (victim = who_fighting(ch)) == NULL)
                        continue;

                /*
                 *  Mob triggers
                 */
                rprog_rfight_trigger(ch);
                if (char_died(ch))
                        continue;
                mprog_hitprcnt_trigger(ch, victim);
                if (char_died(ch))
                        continue;
                mprog_fight_trigger(ch, victim);
                if (char_died(ch))
                        continue;

                /*
                 * Fun for the whole family!
                 */
                for (rch = ch->in_room->first_person; rch; rch = rch_next)
                {
                        rch_next = rch->next_in_room;

                        if (IsAwake(rch) && !rch->fighting)
                        {
                                /*
                                 * PC's auto-assist others in their group.
                                 */
                                if (!IsNpc(ch) || IsAffected(ch, AffCharm))
                                {
                                        if ((!IsNpc(rch)
                                             || IsAffected(rch, AffCharm))
                                            && is_same_group(ch, rch))
                                                multi_hit(rch, victim,
                                                          TypeUndefined);
                                        continue;
                                }

                                /*
                                 * NPC's assist NPC's of same type or 12.5% chance regardless.
                                 */
                                if (IsNpc(rch)
                                    && !IsAffected(rch, AffCharm)
                                    && !IsSet(rch->act, ActNoassist))
                                {
                                        if (char_died(ch))
                                                break;
                                        if (rch->pIndexData == ch->pIndexData
                                            || number_bits(3) == 0)
                                        {
                                                CharData *vch;
                                                CharData *target;
                                                int       number;

                                                target = NULL;
                                                number = 0;
                                                for (vch =
                                                     ch->in_room->
                                                     first_person; vch;
                                                     vch = vch->next_in_room)
                                                {
                                                        if (can_see(rch, vch)
                                                            &&
                                                            is_same_group(vch,
                                                                          victim)
                                                            && number_range(0,
                                                                            number)
                                                            == 0)
                                                        {
                                                                target = vch;
                                                                number++;
                                                        }
                                                }

                                                if (target)
                                                        multi_hit(rch, target,
                                                                  TypeUndefined);
                                        }
                                }
                        }
                }
        }

        return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit(CharData * ch, CharData * victim, int dt)
{
        int       percent_chance;
        int       dual_bonus;
        int       chance = 0;
        ch_ret    retcode;

        /*
         * add timer if player is attacking another player 
         */
        if (!IsNpc(ch) && !IsNpc(victim))
                add_timer(ch, TimerRecentfight, 20, NULL, 0);

        if (!IsNpc(ch) && IsSet(ch->act, PlrNice) && !IsNpc(victim))
                return rNONE;


        if (!arena_can_fight(ch, victim))
        {
                send_to_char
                        ("You can't do that yet. Wait for betting to close.\n\r",
                         ch);
                return rNONE;
        }
        /*
         *  Check if either of the combatants are holding someone, and if so
         *  there is a chance the fight will release them. This will allow for
         *  even weak characters to become martyrs, or the toughest combatants
         *  to one-round their enemy without losing their quarry.
         *                                                       -Ulthrax
         */
        if (ch->holding != NULL)
        {
                chance = IsNpc(victim) ? victim->top_level : number_range(10,
                                                                           80);
                if (number_percent() < chance)
                {
                        CharData *holding = NULL;

                        if ((holding = ch->holding) == NULL)
                        {
                                bug("%s with null holding structure in multi_hit", ch->name);
                                return rNONE;
                        }
                        send_to_char
                                ("The fight distracts you, and you lose hold of your victim.\n\r",
                                 ch);
                        act(AtAction,
                            "In the midst of the fight, $n lets go of $N!",
                            ch, NULL, holding, ToNotvict);
                        do_release(ch, "");
                }

        }
        if (victim->holding != NULL)
        {
                chance = IsNpc(victim) ? victim->top_level : number_range(10,
                                                                           80);
                if (number_percent() < chance)
                {
                        CharData *holding = NULL;

                        if ((holding = victim->holding) == NULL)
                        {
                                bug("%s with null holding structure in multi_hit", victim->name);
                                return rNONE;
                        }
                        send_to_char
                                ("The fight distracts you, and you lose hold of your victim.\n\r",
                                 ch);
                        act(AtAction,
                            "In the midst of the fight, $n lets go of $N!",
                            victim, NULL, holding, ToNotvict);
                        do_release(victim, "");
                }

        }
        if (!IsNpc(ch) && IsSet(ch->PCData->flags, PcflagAutodraw))
        {
                ObjData *holster1 = get_eq_char(ch, WearHolsterL),
                         *holster2 = get_eq_char(ch, WearHolsterR);
                if ((holster1 && holster1->item_type == ItemHolster && holster1->first_content) ||
                                (holster2 && holster2->item_type == ItemHolster && holster2->first_content))
                        do_draw(ch, "");
        }
        if ((retcode = one_hit(ch, victim, dt)) != rNONE)
                return retcode;

        if (who_fighting(ch) != victim || dt == gsn_backstab
            || dt == gsn_circle || dt == gsn_jab)
                return rNONE;

        /*
         * Very high percent_chance of hitting compared to percent_chance of going berserk 
         */
        /*
         * 40% or higher is always hit.. don't learn anything here though. 
         */
        /*
         * -- Altrag 
         */
        percent_chance =
                IsNpc(ch) ? 100 : (ch->PCData->learned[gsn_berserk] * 5 / 2);
        if (IsAffected(ch, AffBerserk) && number_percent() < percent_chance)
                if ((retcode = one_hit(ch, victim, dt)) != rNONE ||
                    who_fighting(ch) != victim)
                        return retcode;

        if (get_eq_char(ch, WearDualWield))
        {
                dual_bonus =
                        IsNpc(ch) ? (ch->skill_level[CombatAbility] /
                                      10) : (ch->PCData->
                                             learned[gsn_dual_wield] / 10);
                percent_chance =
                        IsNpc(ch) ? ch->top_level : ch->PCData->
                        learned[gsn_dual_wield];
                if (number_percent() < percent_chance)
                {
                        learn_from_success(ch, gsn_dual_wield);
                        retcode = one_hit(ch, victim, dt);
                        if (retcode != rNONE || who_fighting(ch) != victim)
                                return retcode;
                }
                else
                        learn_from_failure(ch, gsn_dual_wield);
        }
        else
                dual_bonus = 0;

        if (ch->endurance < 10)
                dual_bonus = -20;

        /*
         * NPC predetermined number of attacks          -Thoric
         */
        if (IsNpc(ch) && ch->numattacks > 0)
        {
                for (percent_chance = 0; percent_chance <= ch->numattacks;
                     percent_chance++)
                {
                        retcode = one_hit(ch, victim, dt);
                        if (retcode != rNONE || who_fighting(ch) != victim)
                                return retcode;
                }
                return retcode;
        }

        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) ((ch->PCData->learned[gsn_second_attack] +
                          dual_bonus) / 1.5);
        if (number_percent() < percent_chance)
        {
                learn_from_success(ch, gsn_second_attack);
                retcode = one_hit(ch, victim, dt);
                if (retcode != rNONE || who_fighting(ch) != victim)
                        return retcode;
        }
        else
                learn_from_failure(ch, gsn_second_attack);

        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) ((ch->PCData->learned[gsn_third_attack] +
                          (dual_bonus * 1.5)) / 2);
        if (number_percent() < percent_chance)
        {
                learn_from_success(ch, gsn_third_attack);
                retcode = one_hit(ch, victim, dt);
                if (retcode != rNONE || who_fighting(ch) != victim)
                        return retcode;
        }
        else
                learn_from_failure(ch, gsn_third_attack);

        retcode = rNONE;

        percent_chance = IsNpc(ch) ? (int) (ch->top_level / 4) : 0;
        if (number_percent() < percent_chance)
                retcode = one_hit(ch, victim, dt);

        if (retcode == rNONE)
        {
                int       endurance;

                if (!IsAffected(ch, AffFlying)
                    && !IsAffected(ch, AffFloating))
                        endurance =
                                encumbrance(ch,
                                            movement_loss[UMIN
                                                          (SectMax - 1,
                                                           ch->in_room->
                                                           sector_type)]);
                else
                        endurance = encumbrance(ch, 1);
                if (ch->endurance)
                        ch->endurance = UMAX(0, ch->endurance - endurance);
        }

        return retcode;
}


/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check(CharData * ch, ObjData * wield, int *gsn_ptr)
{
        int       Bonus;

        Bonus = 0;
        *gsn_ptr = -1;
        if (!IsNpc(ch) && wield)
        {
                switch (wield->value[3])
                {
                default:
                        *gsn_ptr = -1;
                        break;
                case 3:
                        *gsn_ptr = gsn_lightsabers;
                        break;
                case 2:
                        *gsn_ptr = gsn_vibro_blades;
                        break;
                case 4:
                        *gsn_ptr = gsn_flexible_arms;
                        break;
                case 5:
                        *gsn_ptr = gsn_knives;
                        break;
                case 6:
                        *gsn_ptr = gsn_blasters;
                        break;
                case 8:
                        *gsn_ptr = gsn_bludgeons;
                        break;
                case 9:
                        *gsn_ptr = gsn_bowcasters;
                        break;
                case 11:
                        *gsn_ptr = gsn_force_pikes;
                        break;

                }
                if (*gsn_ptr != -1)
                        Bonus = (int) (ch->PCData->learned[*gsn_ptr]);

        }
        if (IsNpc(ch) && wield)
                Bonus = get_trust(ch);
        return Bonus;
}

/*
 * Calculate the tohit Bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll(ObjData * obj)
{
        int       tohit = 0;
        AffectData *paf;

        for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
                if (paf->location == ApplyHitroll)
                        tohit += paf->modifier;
        for (paf = obj->first_affect; paf; paf = paf->next)
                if (paf->location == ApplyHitroll)
                        tohit += paf->modifier;
        return tohit;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl(CharData * ch, CharData * victim)
{
        sh_int    lvl;

        if (!IsNpc(ch))    /* players get much less effect */
        {
                lvl = UMAX(1, (ch->skill_level[ForceAbility]));
                if (number_percent() +
                    (victim->skill_level[CombatAbility] - lvl) < 35)
                        return lvl;
                else
                        return 0;
        }
        else
        {
                lvl = ch->top_level;
                if (number_percent() +
                    (victim->skill_level[CombatAbility] - lvl) < 70)
                        return lvl;
                else
                        return 0;
        }
}

/*
 * Hit one guy once.
 */
ch_ret one_hit(CharData * ch, CharData * victim, int dt)
{
        ObjData *wield;
        int       victim_ac;
        int       thac0;
        int       thac0_00;
        int       thac0_32;
        int       plusris;
        int       dam, x;
        int       diceroll;
        int       attacktype, cnt;
        int       prof_bonus;
        int       prof_gsn;
        ch_ret    retcode = 0;
        int       percent_chance;
        bool      fail;
        AffectData af;


        /*
         * Can't beat a dead char!
         * Guard against weird room-leavings.
         */
        if (victim->position == PosDead || ch->in_room != victim->in_room)
                return rVICT_DIED;
        if ((get_eq_char(ch, WearBinding)) != NULL)
        {
                send_to_char("You are bound, and cannot fight!\n\r", ch);
                return rNONE;
        }

        /*
         * Figure out the weapon doing the damage           -Thoric
         */
        if ((wield = get_eq_char(ch, WearDualWield)) != NULL)
        {
                if (dual_flip == FALSE)
                {
                        dual_flip = TRUE;
                        wield = get_eq_char(ch, WearWield);
                }
                else
                        dual_flip = FALSE;
        }
        else
                wield = get_eq_char(ch, WearWield);

        prof_bonus = weapon_prof_bonus_check(ch, wield, &prof_gsn);

        if (ch->fighting    /* make sure fight is already started */
            && dt == TypeUndefined && IsNpc(ch) && ch->attacks != 0)
        {
                cnt = 0;
                for (;;)
                {
                        x = number_range(0, 6);
                        attacktype = 1 << x;
                        if (IsSet(ch->attacks, attacktype))
                                break;
                        if (cnt++ > 16)
                        {
                                attacktype = 0;
                                break;
                        }
                }
                if (attacktype == AtckBackstab)
                        attacktype = 0;
                if (wield && number_percent() > 25)
                        attacktype = 0;
                switch (attacktype)
                {
                default:
                        break;
                case AtckBite:
                        do_bite(ch, "");
                        retcode = global_retcode;
                        break;
                case AtckClaws:
                        do_claw(ch, "");
                        retcode = global_retcode;
                        break;
                case AtckTail:
                        do_tail(ch, "");
                        retcode = global_retcode;
                        break;
                case AtckSting:
                        do_sting(ch, "");
                        retcode = global_retcode;
                        break;
                case AtckPunch:
                        do_punch(ch, "");
                        retcode = global_retcode;
                        break;
                case AtckKick:
                        do_kick(ch, "");
                        retcode = global_retcode;
                        break;
                case AtckTrip:
                        attacktype = 0;
                        break;
                }
                if (attacktype)
                        return retcode;
        }

        if (dt == TypeUndefined)
        {
                dt = TypeHit;
                if (wield && wield->item_type == ItemWeapon)
                        dt += wield->value[3];
        }

        /*
         * Calculate to-hit-Armor-class-0 versus Armor.
         */
        thac0_00 = 20;
        thac0_32 = 10;
        thac0 = interpolate(ch->skill_level[CombatAbility], thac0_00,
                            thac0_32) - GetHitroll(ch);
        victim_ac = (int) (GetAc(victim) / 10);

        /*
         * if you can't see what's coming... 
         */
        if (wield && !can_see_obj(victim, wield))
                victim_ac += 1;
        if (!can_see(ch, victim))
                victim_ac -= 4;

        if (!str_cmp(ch->race->name(), "defel"))
                victim_ac += 2;

        if (!IsAwake(victim))
                victim_ac += 5;

        /*
         * Weapon proficiency Bonus 
         */
        victim_ac += prof_bonus / 20;

        /*
         * The moment of excitement!
         */
        diceroll = number_range(1, 20);

        if (diceroll == 1 || (diceroll < 20 && diceroll < thac0 - victim_ac))
        {
                /*
                 * Miss. 
                 */
                if (prof_gsn != -1)
                        learn_from_failure(ch, prof_gsn);
                damage(ch, victim, 0, dt);
                tail_chain();
                return rNONE;
        }

        /*
         * Hit.
         * Calc damage.
         */

        if (!wield) /* dice formula fixed by Thoric */
                dam = number_range(ch->barenumdie,
                                   ch->baresizedie * ch->barenumdie);
        else
                dam = number_range(wield->value[1], wield->value[2]);

        /*
         * Bonuses.
         */

        dam += GetDamroll(ch);

        if (IsSet(ch->affected_by, AffRestrained))
        {
                dam /= 100;
        }

        if (prof_bonus)
                dam *= (1 + prof_bonus / 100);


        if (!IsNpc(ch) && ch->PCData->learned[gsn_enhanced_damage] > 0)
        {
                dam += (int) (dam * ch->PCData->learned[gsn_enhanced_damage] /
                              120);
                learn_from_success(ch, gsn_enhanced_damage);
        }


        if (!IsAwake(victim))
                dam *= 2;
        if (dt == gsn_backstab)
                dam *= (2 +
                        URANGE(2,
                               ch->skill_level[HuntingAbility] -
                               (victim->skill_level[CombatAbility] / 4),
                               30) / 16);

        if (dt == gsn_circle)
                dam *= (2 +
                        URANGE(2,
                               ch->skill_level[HuntingAbility] -
                               (victim->skill_level[CombatAbility] / 4),
                               30) / 32);

        if (dt == gsn_jab)
                dam *= (2 +
                        URANGE(2,
                               ch->skill_level[HuntingAbility] -
                               (victim->skill_level[CombatAbility] / 4),
                               30) / 16);

        plusris = 0;

        if (wield)
        {
                if (IsSet(wield->extra_flags, ItemMagic))
                        dam = ris_damage(victim, dam, RisMagic);
                else
                        dam = ris_damage(victim, dam, RisNonmagic);

                /*
                 * Handle PLUS1 - PLUS6 ris bits vs. weapon Hitroll   -Thoric
                 */
                plusris = obj_hitroll(wield);
        }
        else
                dam = ris_damage(victim, dam, RisNonmagic);

        /*
         * check for RIS_PLUSx                  -Thoric 
         */
        if (dam)
        {
                int       res, imm, sus, Mod;

                if (plusris)
                        plusris = RisPlus1 << UMIN(plusris, 7);

                /*
                 * initialize values to handle a zero plusris 
                 */
                imm = res = -1;
                sus = 1;

                /*
                 * find high ris 
                 */
                for (x = RisPlus1; x <= RisPlus6; x <<= 1)
                {
                        if (IsSet(victim->immune, x))
                                imm = x;
                        if (IsSet(victim->resistant, x))
                                res = x;
                        if (IsSet(victim->susceptible, x))
                                sus = x;
                }
                Mod = 10;
                if (imm >= plusris)
                        Mod -= 10;
                if (res >= plusris)
                        Mod -= 2;
                if (sus <= plusris)
                        Mod += 2;

                /*
                 * check if immune 
                 */
                if (Mod <= 0)
                        dam = -1;
                if (Mod != 10)
                        dam = (dam * Mod) / 10;
        }

        /*
         * race modifier 
         */

        if (!str_cmp(victim->race->name(), "duinuogwuin"))
                dam /= 5;

        /*
         * check to see if weapon is charged 
         */

        if (dt == (TypeHit + WeaponBlaster) && wield
            && wield->item_type == ItemWeapon)
        {
                if (wield->value[4] < 1)
                {
                        act(AtYellow,
                            "$n points their blaster at you but nothing happens.",
                            ch, NULL, victim, ToVict);
                        act(AtYellow,
                            "*CLICK* ... your blaster needs a new ammunition cell!",
                            ch, NULL, victim, ToChar);
                        if (IsNpc(ch))
                        {
                                do_remove(ch, wield->name);
                        }
                        return rNONE;
                }
                else if (wield->blaster_setting == BlasterFull
                         && wield->value[4] >= 5)
                {
                        dam *= (sh_int) 1.5;
                        wield->value[4] -= 5;
                }
                else if (wield->blaster_setting == BlasterHigh
                         && wield->value[4] >= 4)
                {
                        dam *= (sh_int) 1.25;
                        wield->value[4] -= 4;
                }
                else if (wield->blaster_setting == BlasterNormal
                         && wield->value[4] >= 3)
                {
                        wield->value[4] -= 3;
                }
                else if (wield->blaster_setting == BlasterStun
                         && wield->value[4] >= 5)
                {
                        dam /= 10;
                        wield->value[4] -= 3;
                        fail = FALSE;
                        percent_chance =
                                ris_save(victim,
                                         ch->skill_level[CombatAbility],
                                         RisParalysis);
                        if (percent_chance == 1000)
                                fail = TRUE;
                        else
                                fail = saves_para_petri(percent_chance,
                                                        victim);
                        if (victim->was_stunned > 0)
                        {
                                fail = TRUE;
                                victim->was_stunned--;
                        }
                        percent_chance =
                                100 - get_curr_con(victim) -
                                victim->skill_level[CombatAbility] / 2;
                        /*
                         * harder for player to stun another player 
                         */
                        if (!IsNpc(ch) && !IsNpc(victim))
                                percent_chance -= sysdata.stun_plr_vs_plr;
                        else
                                percent_chance -= sysdata.stun_regular;
                        percent_chance = URANGE(5, percent_chance, 95);
                        if (!fail && number_percent() < percent_chance)
                        {
                                WaitState(victim, PulseViolence);
                                act(AtBlue,
                                    "Blue rings of energy from &R$N's &Bblaster knock you down leaving you stunned!",
                                    victim, NULL, ch, ToChar);
                                act(AtBlue,
                                    "Blue rings of energy from your blaster strike &R$N&B, leaving $M &Bstunned!",
                                    ch, NULL, victim, ToChar);
                                act(AtBlue,
                                    "Blue rings of energy from &R$n's &Bblaster hit &R$N, &Bleaving $M stunned!",
                                    ch, NULL, victim, ToNotvict);
                                stop_fighting(victim, TRUE);
                                if (!IsAffected(victim, AffParalysis))
                                {
                                        af.type = gsn_stun;
                                        af.location = ApplyAc;
                                        af.modifier = 20;
                                        af.duration = 7;
                                        af.bitvector = AffParalysis;
                                        affect_to_char(victim, &af);
                                        update_pos(victim);
                                        if (IsNpc(victim))
                                        {
                                                start_hating(victim, ch);
                                                start_hunting(victim, ch);
                                                victim->was_stunned = 10;
                                        }
                                }
                        }
                        else
                        {
                                act(AtBlue,
                                    "Blue rings of energy from &R$N's &Bblaster hit you but have little effect",
                                    victim, NULL, ch, ToChar);
                                act(AtBlue,
                                    "Blue rings of energy from your blaster hit &R$N,&B but nothing seems to happen!",
                                    ch, NULL, victim, ToChar);
                                act(AtBlue,
                                    "Blue rings of energy from $n's blaster hit &R$N,&B but nothing seems to happen!",
                                    ch, NULL, victim, ToNotvict);

                        }
                }
                else if (wield->blaster_setting == BlasterHalf
                         && wield->value[4] >= 2)
                {
                        dam *= (sh_int) 0.75;
                        wield->value[4] -= 2;
                }
                else
                {
                        dam *= (sh_int) 0.5;
                        wield->value[4] -= 1;
                }

        }
        else if (dt == (TypeHit + WeaponVibroBlade)
                 && wield && wield->item_type == ItemWeapon)
        {
                if (wield->value[4] < 1)
                {
                        act(AtYellow,
                            "Your vibro-blade needs recharging ...", ch, NULL,
                            victim, ToChar);
                        dam /= 3;
                }
        }
        else if (dt == (TypeHit + WeaponForcePike)
                 && wield && wield->item_type == ItemWeapon)
        {
                if (wield->value[4] < 1)
                {
                        act(AtYellow, "Your Force-pike needs recharging ...",
                            ch, NULL, victim, ToChar);
                        dam /= 2;
                }
                else
                        wield->value[4]--;
        }
        else if (dt == (TypeHit + WeaponLightsaber)
                 && wield && wield->item_type == ItemWeapon)
        {
                if (wield->value[4] < 1)
                {
                        act(AtYellow,
                            "$n waves a dead hand grip around in the air.",
                            ch, NULL, victim, ToVict);
                        act(AtYellow,
                            "You need to recharge your lightsaber ... it seems to be lacking a blade.",
                            ch, NULL, victim, ToChar);
                        if (IsNpc(ch))
                        {
                                do_remove(ch, wield->name);
                        }
                        return rNONE;
                }
        }
        else if (dt == (TypeHit + WeaponBowcaster) && wield
                 && wield->item_type == ItemWeapon)
        {
                if (wield->value[4] < 1)
                {
                        act(AtYellow,
                            "$n points their bowcaster at you but nothing happens.",
                            ch, NULL, victim, ToVict);
                        act(AtYellow,
                            "*CLICK* ... your bowcaster needs a new bolt cartridge!",
                            ch, NULL, victim, ToChar);
                        if (IsNpc(ch))
                        {
                                do_remove(ch, wield->name);
                        }
                        return rNONE;
                }
                else
                        wield->value[4]--;
        }

        if (dam <= 0)
                dam = 1;

        if (prof_gsn != -1)
        {
                if (dam > 0)
                        learn_from_success(ch, prof_gsn);
                else
                        learn_from_failure(ch, prof_gsn);
        }

        /*
         * immune to damage 
         */
        if (dam == -1)
        {
                if (dt >= 0 && dt < top_sn)
                {
                        SkillType *skill = skill_table[dt];
                        bool      found = FALSE;

                        if (skill->imm_char && skill->imm_char[0] != '\0')
                        {
                                act(AtHit, skill->imm_char, ch, NULL, victim,
                                    ToChar);
                                found = TRUE;
                        }
                        if (skill->imm_vict && skill->imm_vict[0] != '\0')
                        {
                                act(AtHitme, skill->imm_vict, ch, NULL,
                                    victim, ToVict);
                                found = TRUE;
                        }
                        if (skill->imm_room && skill->imm_room[0] != '\0')
                        {
                                act(AtAction, skill->imm_room, ch, NULL,
                                    victim, ToNotvict);
                                found = TRUE;
                        }
                        if (found)
                                return rNONE;
                }
                dam = 0;
        }
        if ((retcode = damage(ch, victim, dam, dt)) != rNONE)
                return retcode;
        if (char_died(ch))
                return rCHAR_DIED;
        if (char_died(victim))
                return rVICT_DIED;

        retcode = rNONE;
        if (dam == 0)
                return retcode;

/* weapon spells	-Thoric */
        if (wield
            && !IsSet(victim->immune, RisMagic)
            && !IsSet(victim->in_room->RoomFlags, RoomNoMagic))
        {
                AffectData *aff;

                for (aff = wield->pIndexData->first_affect; aff;
                     aff = aff->next)
                        if (aff->location == ApplyWeaponspell
                            && IsValidSn(aff->modifier)
                            && skill_table[aff->modifier]->spell_fun)
                                retcode =
                                        (*skill_table[aff->modifier]->
                                         spell_fun) (aff->modifier,
                                                     (wield->level + 3) / 3,
                                                     ch, victim);
                if (retcode != rNONE || char_died(ch) || char_died(victim))
                        return retcode;
                for (aff = wield->first_affect; aff; aff = aff->next)
                        if (aff->location == ApplyWeaponspell
                            && IsValidSn(aff->modifier)
                            && skill_table[aff->modifier]->spell_fun)
                                retcode =
                                        (*skill_table[aff->modifier]->
                                         spell_fun) (aff->modifier,
                                                     (wield->level + 3) / 3,
                                                     ch, victim);
                if (retcode != rNONE || char_died(ch) || char_died(victim))
                        return retcode;
        }

        /*
         * magic shields that retaliate             -Thoric
         */
        if (IsAffected(victim, AffFireshield)
            && !IsAffected(ch, AffFireshield))
                retcode =
                        spell_fireball(gsn_fireball, off_shld_lvl(victim, ch),
                                       victim, ch);
        if (retcode != rNONE || char_died(ch) || char_died(victim))
                return retcode;

        if (retcode != rNONE || char_died(ch) || char_died(victim))
                return retcode;

        if (IsAffected(victim, AffShockshield)
            && !IsAffected(ch, AffShockshield))
                retcode =
                        spell_lightning_bolt(gsn_lightning_bolt,
                                             off_shld_lvl(victim, ch), victim,
                                             ch);
        if (retcode != rNONE || char_died(ch) || char_died(victim))
                return retcode;

        /*
         *   folks with blasters move and snipe instead of getting neatin up in one spot.
         */
        if (IsNpc(victim) && !IsSet(victim->act, ActNorunsnipe))
        {
                ObjData *blaster_wield;

                blaster_wield = get_eq_char(victim, WearWield);
                if (blaster_wield != NULL
                    && blaster_wield->value[3] == WeaponBlaster
                    && get_cover(victim) == TRUE)
                {
                        start_hating(victim, ch);
                        start_hunting(victim, ch);
                }
        }

        tail_chain();
        return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage(CharData * ch, sh_int dam, int ris)
{
        sh_int    modifier;

        modifier = 10;
        if (IsSet(ch->immune, ris))
                modifier -= 10;
        if (IsSet(ch->resistant, ris))
                modifier -= 2;
        if (IsSet(ch->susceptible, ris))
                modifier += 2;
        if (modifier <= 0)
                return -1;
        if (modifier == 10)
                return dam;
        return (dam * modifier) / 10;
}


/*
 * Inflict damage from a hit.
 */
ch_ret damage(CharData * ch, CharData * victim, int dam, int dt)
{
        char      buf1[MaxStringLength];
        sh_int    dameq;
        bool      npcvict;
        bool      loot;
        int       xp_gain;
        ObjData *damobj;
        ch_ret    retcode;
        sh_int    dampmod;
        CharData *gch;

        int       init_gold, new_gold, gold_diff;

        retcode = rNONE;

        if (!ch)
        {
                bug("Damage: null ch!", 0);
                return rERROR;
        }
        if (!victim)
        {
                bug("Damage: null victim!", 0);
                return rVICT_DIED;
        }

        if (victim->position == PosDead)
                return rVICT_DIED;

        npcvict = IsNpc(victim);

        /*
         * Check damage types for RIS               -Thoric
         */
        if (dam && dt != TypeUndefined)
        {
                if (IsFire(dt))
                        dam = ris_damage(victim, dam, RisFire);
                else if (IsCold(dt))
                        dam = ris_damage(victim, dam, RisCold);
                else if (IsAcid(dt))
                        dam = ris_damage(victim, dam, RisAcid);
                else if (IsElectricity(dt))
                        dam = ris_damage(victim, dam, RisElectricity);
                else if (IsEnergy(dt))
                        dam = ris_damage(victim, dam, RisEnergy);
                else if (IsDrain(dt))
                        dam = ris_damage(victim, dam, RisDrain);
                else if (dt == gsn_poison || IsPoison(dt))
                        dam = ris_damage(victim, dam, RisPoison);
                else if (dt == (TypeHit + 7) || dt == (TypeHit + 8))
                        dam = ris_damage(victim, dam, RisBlunt);
                else if (dt == (TypeHit + 2) || dt == (TypeHit + 11)
                         || dt == (TypeHit + 10))
                        dam = ris_damage(victim, dam, RisPierce);
                else if (dt == (TypeHit + 1) || dt == (TypeHit + 3)
                         || dt == (TypeHit + 4) || dt == (TypeHit + 5))
                        dam = ris_damage(victim, dam, RisSlash);

                if (dam == -1)
                {
                        if (dt >= 0 && dt < top_sn)
                        {
                                bool      found = FALSE;
                                SkillType *skill = skill_table[dt];

                                if (skill->imm_char
                                    && skill->imm_char[0] != '\0')
                                {
                                        act(AtHit, skill->imm_char, ch, NULL,
                                            victim, ToChar);
                                        found = TRUE;
                                }
                                if (skill->imm_vict
                                    && skill->imm_vict[0] != '\0')
                                {
                                        act(AtHitme, skill->imm_vict, ch,
                                            NULL, victim, ToVict);
                                        found = TRUE;
                                }
                                if (skill->imm_room
                                    && skill->imm_room[0] != '\0')
                                {
                                        act(AtAction, skill->imm_room, ch,
                                            NULL, victim, ToNotvict);
                                        found = TRUE;
                                }
                                if (found)
                                        return rNONE;
                        }
                        dam = 0;
                }
        }

        if (dam && npcvict && ch != victim)
        {
                if (!IsSet(victim->act, ActSentinel))
                {
                        if (victim->hunting)
                        {
                                if (victim->hunting->who != ch)
                                {
                                        STRFREE(victim->hunting->name);
                                        victim->hunting->name =
                                                QUICKLINK(ch->name);
                                        victim->hunting->who = ch;
                                }
                        }
                        else
                                start_hunting(victim, ch);
                }

                if (victim->hating)
                {
                        if (victim->hating->who != ch)
                        {
                                STRFREE(victim->hating->name);
                                victim->hating->name = QUICKLINK(ch->name);
                                victim->hating->who = ch;
                        }
                }
                else
                        start_hating(victim, ch);
        }

        if (victim != ch)
        {
                /*
                 * Certain attacks are forbidden.
                 * Most other attacks are returned.
                 */
                if (is_safe(ch, victim))
                        return rNONE;


                if (victim->position > PosStunned)
                {
                        if (!victim->fighting)
                                set_fighting(victim, ch);
                        if (victim->fighting)
                                victim->position = PosFighting;
                }

                if (victim->position > PosStunned)
                {
                        if (!ch->fighting)
                                set_fighting(ch, victim);

                        /*
                         * If victim is charmed, ch might attack victim's master.
                         */
                        if (IsNpc(ch)
                            && npcvict
                            && IsAffected(victim, AffCharm)
                            && victim->master
                            && victim->master->in_room == ch->in_room
                            && number_bits(3) == 0)
                        {
                                stop_fighting(ch, FALSE);
                                retcode =
                                        multi_hit(ch, victim->master,
                                                  TypeUndefined);
                                return retcode;
                        }
                }


                /*
                 * More charm stuff.
                 */
                if (victim->master == ch)
                        stop_follower(victim);


                /*
                 * Inviso attacks ... not.
                 */
                if (IsAffected(ch, AffInvisible)
                    && str_cmp(ch->race->name(), "defel"))
                {
                        affect_strip(ch, gsn_invis);
                        affect_strip(ch, gsn_mass_invis);
                        RemoveBit(ch->affected_by, AffInvisible);
                        act(AtMagic, "$n fades into existence.", ch, NULL,
                            NULL, ToRoom);
                }

                if (IsAffected(ch, AffSecretive))
                {
                        affect_strip(ch, gsn_slight);
                        RemoveBit(ch->affected_by, AffSecretive);
                        RemoveBit(ch->act, PlrSecretive);
                        act(AtMagic, "$n's action become noticable!", ch,
                            NULL, NULL, ToRoom);
                }

                /*
                 * Take away Hide 
                 */
                if (IsAffected(ch, AffHide))
                {
                        affect_strip(ch, gsn_hide);
                        RemoveBit(ch->affected_by, AffHide);
                        act(AtMagic, "$n's appears out of no where!", ch,
                            NULL, NULL, ToRoom);
                }

                /*
                 * Damage modifiers.
                 */
                if (IsAffected(victim, AffSanctuary))
                        dam /= 2;

                if (IsAffected(victim, AffProtect) && IsEvil(ch))
                        dam -= (int) (dam / 4);

                if (dam < 0)
                        dam = 0;

                /*
                 * Check for disarm, trip, parry, and dodge.
                 */
                if (dt >= TypeHit)
                {
                        if (IsNpc(ch)
                            && IsSet(ch->attacks, AtckTrip)
                            && number_percent() <
                            ch->skill_level[CombatAbility])
                                trip(ch, victim);

                        if (check_parry(ch, victim))
                                return rNONE;
                        if (check_dodge(ch, victim))
                                return rNONE;
                }



                /*
                 * Check control panel settings and modify damage
                 */
                if (IsNpc(ch))
                {
                        if (npcvict)
                                dampmod = sysdata.dam_mob_vs_mob;
                        else
                                dampmod = sysdata.dam_mob_vs_plr;
                }
                else
                {
                        if (npcvict)
                                dampmod = sysdata.dam_plr_vs_mob;
                        else
                                dampmod = sysdata.dam_plr_vs_plr;
                }
                if (dampmod > 0)
                        dam = (dam * dampmod) / 100;

                dam_message(ch, victim, dam, dt);
        }


        /*
         * Code to handle equipment getting damaged, and also support  -Thoric
         * bonuses/penalties for having or not having equipment where hit
         */
        if (dam > 10 && dt != TypeUndefined &&
            !IsSet(victim->in_room->RoomFlags, RoomArena))
        {
                /*
                 * get a random body eq part 
                 */
                dameq = number_range(WearLight, WearEyes);
                damobj = get_eq_char(victim, dameq);
                if (damobj)
                {
                        if (dam > get_obj_resistance(damobj))
                        {
                                set_cur_obj(damobj);
                                damage_obj(damobj);
                        }
                        dam -= 5;   /* add a Bonus for having something to block the blow */
                }
                else
                        dam += 5;   /* add penalty for bare skin! */
        }

        /*
         * Hurt the victim.
         * Inform the victim of his new state.
         */

        victim->hit -= dam;

        /*
         * Get experience based on % of damage done         -Thoric
         */
        if (dam && ch != victim
            && !IsNpc(ch) && ch->fighting && ch->fighting->xp)
        {
                xp_gain =
                        (int) (xp_compute(ch, victim) * 0.1 * dam) /
                        victim->max_hit;
                gain_exp(ch, xp_gain, CombatAbility);
        }

        if (IsSet(ch->in_room->RoomFlags, RoomArena) && victim->hit < 1
            && !IsNpc(victim))
        {
                stop_fighting(ch, TRUE);
                stop_fighting(victim, TRUE);
                win_fight(ch, victim);
                return rNONE;
        }

        if (!IsNpc(victim)
            && victim->top_level >= LevelImmortal && victim->hit < 1)
                victim->hit = 1;

        /*
         * Make sure newbies dont die 
         */

        if (!IsNpc(victim) && NotAuthed(victim) && victim->hit < 1)
                victim->hit = 1;

        if (dam > 0 && dt > TypeHit
            && !IsAffected(victim, AffPoison)
            && is_wielding_poisoned(ch)
            && !IsSet(victim->immune, RisPoison)
            && !saves_poison_death(ch->skill_level[CombatAbility], victim))
        {
                AffectData af;

                af.type = gsn_poison;
                af.duration = 20;
                af.location = ApplyStr;
                af.modifier = -2;
                af.bitvector = AffPoison;
                affect_join(victim, &af);
                victim->mental_state =
                        URANGE(20, victim->mental_state + 2, 100);
        }

        if (!npcvict
            && get_trust(victim) >= LevelImmortal
            && get_trust(ch) >= LevelImmortal && victim->hit < 1)
                victim->hit = 1;
        update_pos(victim);

        switch (victim->position)
        {
        case PosMortal:
                act(AtDying,
                    "$n is mortally wounded, and will die soon, if not aided.",
                    victim, NULL, NULL, ToRoom);
                send_to_char
                        ("&RYou are mortally wounded, and will die soon, if not aided.",
                         victim);
                break;

        case PosIncap:
                act(AtDying,
                    "$n is incapacitated and will slowly die, if not aided.",
                    victim, NULL, NULL, ToRoom);
                send_to_char
                        ("&RYou are incapacitated and will slowly die, if not aided.",
                         victim);
                break;

        case PosStunned:
                if (!IsAffected(victim, AffParalysis))
                {
                        act(AtAction,
                            "$n is stunned, but will probably recover.",
                            victim, NULL, NULL, ToRoom);
                        send_to_char
                                ("&RYou are stunned, but will probably recover.",
                                 victim);
                }
                break;

        case PosDead:
                if (dt >= 0 && dt < top_sn)
                {
                        SkillType *skill = skill_table[dt];

                        if (skill->die_char && skill->die_char[0] != '\0')
                                act(AtDead, skill->die_char, ch, NULL,
                                    victim, ToChar);
                        if (skill->die_vict && skill->die_vict[0] != '\0')
                                act(AtDead, skill->die_vict, ch, NULL,
                                    victim, ToVict);
                        if (skill->die_room && skill->die_room[0] != '\0')
                                act(AtDead, skill->die_room, ch, NULL,
                                    victim, ToNotvict);
                }
                if (IsNpc(victim) && IsSet(victim->act, ActNokill))
                        act(AtYellow,
                            "$n flees for $s life ... barely escaping certain death!",
                            victim, 0, 0, ToRoom);
                else if (IsNpc(victim) && IsSet(victim->act, ActDroid))
                        act(AtDead, "$n EXPLODES into many small pieces!",
                            victim, 0, 0, ToRoom);
                else if (str_cmp(victim->in_room->area->filename, "arena.are")
                         || !IsSet(victim->in_room->RoomFlags, RoomArena))
                        act(AtDead, "$n is DEAD!", victim, 0, 0, ToRoom);
                send_to_char("&WYou have been KILLED!\n\r", victim);
                break;

        default:
                if (dam > victim->max_hit / 4)
                {
                        act(AtHurt, "That really did HURT!", victim, 0, 0,
                            ToChar);
                        if (number_bits(3) == 0)
                                worsen_mental_state(victim, 1);
                }
                if (victim->hit < victim->max_hit / 4)

                {
                        act(AtDanger,
                            "You wish that your wounds would stop BLEEDING so much!",
                            victim, 0, 0, ToChar);
                        if (number_bits(2) == 0)
                                worsen_mental_state(victim, 1);
                }
                break;
        }

        /*
         * Sleep spells and extremely wounded folks.
         */
        if (!IsAwake(victim)   /* lets make NPC's not slaughter PC's */
            && !IsAffected(victim, AffParalysis))
        {
                if (victim->fighting
                    && victim->fighting->who->hunting
                    && victim->fighting->who->hunting->who == victim)
                        stop_hunting(victim->fighting->who);

                if (victim->fighting
                    && victim->fighting->who->hating
                    && victim->fighting->who->hating->who == victim)
                        stop_hating(victim->fighting->who);

                stop_fighting(victim, TRUE);
        }

        if (victim->hit <= 0 && !IsNpc(victim))
        {
                ObjData *obj;
                ObjData *obj_next;
                int       cnt = 0;

                RemoveBit(victim->act, PlrAttacker);

                stop_fighting(victim, TRUE);

                if ((obj = get_eq_char(victim, WearDualWield)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WearWield)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WearHold)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WearMissileWield)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WearLight)) != NULL)
                        unequip_char(victim, obj);

                for (obj = victim->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;

                        if (obj->wear_loc == WearNone)
                        {
                                if (obj->pIndexData->progtypes & DropProg
                                    && obj->count > 1)
                                {
                                        ++cnt;
                                        separate_obj(obj);
                                        obj_from_char(obj);
                                        if (!obj_next)
                                                obj_next =
                                                        victim->
                                                        first_carrying;
                                }
                                else
                                {
                                        cnt += obj->count;
                                        obj_from_char(obj);
                                }
                                act(AtAction, "$n drops $p.", victim, obj,
                                    NULL, ToRoom);
                                act(AtAction, "You drop $p.", victim, obj,
                                    NULL, ToChar);
                                obj = obj_to_room(obj, victim->in_room);
                        }
                }

                if (IsNpc(ch) && !IsNpc(victim))
                {
                        long      lose_exp;

                        lose_exp =
                                UMAX((victim->experience[CombatAbility] -
                                      exp_level(victim->
                                                skill_level[CombatAbility])),
                                     0);
                        ch_printf(victim, "You lose %ld experience.\n\r",
                                  lose_exp);
                        victim->experience[CombatAbility] -= lose_exp;
                }

                add_timer(victim, TimerRecentfight, 100, NULL, 0);

        }

        /*
         * Payoff for killing things.
         */
        if (victim->position == PosDead)
        {
                group_gain(ch, victim);

                if (!npcvict)
                {
                        if (!IsSet(victim->in_room->RoomFlags, RoomArena))
                        {
                                snprintf(log_buf, MSL,
                                         "%s killed by %s at %d",
                                         victim->name,
                                         (IsNpc(ch) ? ch->short_descr : ch->
                                          name), victim->in_room->vnum);
                                log_string(log_buf);
                        }
                }
                else if (!IsNpc(ch) && IsNpc(victim)) /* keep track of mob vnum killed */
                {
                        add_kill(ch, victim);

                        /*
                         * Add to kill tracker for grouped chars, as well. -Halcyon
                         */
                        for (gch = ch->in_room->first_person; gch;
                             gch = gch->next_in_room)
                                if (is_same_group(gch, ch) && !IsNpc(gch)
                                    && gch != ch)
                                        add_kill(gch, victim);
                }

                check_killer(ch, victim);

                if (!IsNpc(victim) || !IsSet(victim->act, ActNokill))
                        loot = legal_loot(ch, victim);
                else
                        loot = FALSE;

                raw_kill(ch, victim);
                victim = NULL;

                if (!IsNpc(ch) && loot)
                {
                        /*
                         * Autogold by Scryn 8/12 
                         */
                        if (IsSet(ch->act, PlrAutogold))
                        {
                                init_gold = ch->gold;
                                do_get(ch, "credits corpse");
                                new_gold = ch->gold;
                                gold_diff = (new_gold - init_gold);
                                if (gold_diff > 0)
                                {
                                        snprintf(buf1, MSL, "%d", gold_diff);
                                        do_split(ch, buf1);
                                }
                        }
                        if (IsSet(ch->act, PlrAutoloot))
                                do_get(ch, "all corpse");
                        else
                                do_look(ch, "in corpse");

                }

                if (IsSet(sysdata.save_flags, SvKill))
                        save_char_obj(ch);
                return rVICT_DIED;
        }

        if (victim == ch)
                return rNONE;

        /*
         * Take care of link dead people.
         */
        if (!npcvict && !victim->desc && !victim->switched)
        {
                if (number_range(0, victim->wait) == 0)
                {
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_hail(victim, "");
                        do_quit(victim, "");
                        return rNONE;
                }
        }

        /*
         * Wimp out?
         */
        if (npcvict && dam > 0)
        {
                if ((IsSet(victim->act, ActWimpy) && number_bits(1) == 0
                     && victim->hit < victim->max_hit / 2)
                    || (IsAffected(victim, AffCharm) && victim->master
                        && victim->master->in_room != victim->in_room))
                {
                        start_fearing(victim, ch);
                        stop_hunting(victim);
                        do_flee(victim, "");
                }
        }

        if (!npcvict
            && victim->hit > 0
            && victim->hit <= victim->wimpy && victim->wait == 0)
                do_flee(victim, "");
        else if (!npcvict && IsSet(victim->act, PlrFlee))
                do_flee(victim, "");

        tail_chain();
        return rNONE;
}

bool is_safe(CharData * ch, CharData * victim)
{
        if (!victim)
                return FALSE;

        /*
         * Thx Josh! 
         */
        if (who_fighting(ch) == ch)
                return FALSE;

        if (IsSet(victim->in_room->RoomFlags, RoomSafe))
        {
                set_char_color(AtMagic, ch);
                send_to_char("You'll have to do that elswhere.\n\r", ch);
                return TRUE;
        }

        if (get_trust(ch) > LevelHero)
                return FALSE;

        if (IsNpc(ch) || IsNpc(victim))
                return FALSE;


        return FALSE;

}

/* checks is_safe but without the output 
   cuts out imms and safe rooms as well 
   for info only */

bool is_safe_nm(CharData * ch, CharData * victim)
{
        ch = NULL;
        victim = NULL;
        return FALSE;
}


/*
 * just verify that a corpse looting is legal
 */
bool legal_loot(CharData * ch, CharData * victim)
{
        victim = NULL;
        /*
         * pc's can now loot .. why not .. death is pretty final 
         */
        if (!IsNpc(ch))
                return TRUE;
        /*
         * non-charmed mobs can loot anything 
         */
        if (IsNpc(ch) && !ch->master)
                return TRUE;

        return FALSE;
}

/*
see if an attack justifies a KILLER flag --- edited so that none do but can't 
murder a no pk person. --- edited again for planetary wanted flags -- well will be soon :p
 */

void check_killer(CharData * ch, CharData * victim)
{

        /*
         * Charm-o-rama.
         */
        if (IsSet(ch->affected_by, AffCharm))
        {
                if (!ch->master)
                {
                        char      buf[MaxStringLength];

                        snprintf(buf, MSL, "Check_killer: %s bad AffCharm",
                                 IsNpc(ch) ? ch->short_descr : ch->name);
                        bug(buf, 0);
                        affect_strip(ch, gsn_charm_person);
                        RemoveBit(ch->affected_by, AffCharm);
                        return;
                }

                /*
                 * stop_follower( ch ); 
                 */
                if (ch->master)
                        check_killer(ch->master, victim);
        }

        if (IsNpc(victim))
        {
                if (!IsNpc(ch))
                {
                        if (IsSet(victim->act, ActCitizen))
                        {
                                if (victim->in_room && victim->in_room->area
                                    && victim->in_room->area->planet)
                                {
                                        PlanetData *planet =
                                                victim->in_room->area->planet;
                                        add_wanted(ch, planet);
                                }
                        }
                        if (ch->PCData->clan)
                                ch->PCData->clan->mkills++;
                        ch->PCData->mkills++;
                        ch->in_room->area->mkills++;
                }
                return;
        }

        if (!IsNpc(ch) && !IsNpc(victim))
        {
                if (ch->PCData->clan)
                        ch->PCData->clan->pkills++;
                ch->PCData->pkills++;
                update_pos(victim);
                if (victim->PCData->clan)
                        victim->PCData->clan->pdeaths++;
        }


        if (IsNpc(ch))
                if (!IsNpc(victim))
                        victim->in_room->area->mdeaths++;

        return;
}



/*
 * Set position of a victim.
 */
void update_pos(CharData * victim)
{
        if (!victim)
        {
                bug("update_pos: null victim", 0);
                return;
        }

        if (victim->hit > 0)
        {
                if (victim->position <= PosStunned
                    && !IsAffected(victim, AffParalysis))
                        victim->position = PosStanding;
                if (IsAffected(victim, AffParalysis))
                {
                        victim->position = PosStunned;
                        stop_hating(victim);
                        stop_hunting(victim);
                }
                return;
        }

        if (IsNpc(victim) || victim->hit <= -500)
        {
                if (victim->mount)
                {
                        act(AtAction, "$n falls from $N.",
                            victim, NULL, victim->mount, ToRoom);
                        RemoveBit(victim->mount->act, ActMounted);
                        victim->mount = NULL;
                }
                victim->position = PosDead;
                return;
        }

        if (victim->hit <= -400)
                victim->position = PosMortal;
        else if (victim->hit <= -200)
                victim->position = PosIncap;
        else
                victim->position = PosStunned;

        if (victim->position > PosStunned
            && IsAffected(victim, AffParalysis))
                victim->position = PosStunned;

        if (victim->mount)
        {
                act(AtAction, "$n falls unconscious from $N.",
                    victim, NULL, victim->mount, ToRoom);
                RemoveBit(victim->mount->act, ActMounted);
                victim->mount = NULL;
        }
        return;
}


/*
 * Start fights.
 */
void set_fighting(CharData * ch, CharData * victim)
{
        FightData *fight;

        if (ch->fighting)
        {
                char      buf[MaxStringLength];

                snprintf(buf, MSL,
                         "Set_fighting: %s -> %s (already fighting %s)",
                         ch->name, victim->name, ch->fighting->who->name);
                bug(buf, 0);
                return;
        }

        if (IsAffected(ch, AffSleep))
                affect_strip(ch, gsn_sleep);

        /*
         * Limit attackers -Thoric 
         */
        if (victim->num_fighting > max_fight(victim))
        {
                send_to_char
                        ("There are too many people fighting for you to join in.\n\r",
                         ch);
                return;
        }

        CREATE(fight, FightData, 1);
        fight->who = victim;
        fight->xp = (int) xp_compute(ch, victim);
        fight->align = align_compute(ch, victim);
        if (!IsNpc(ch) && IsNpc(victim))
                fight->timeskilled = times_killed(ch, victim);
        ch->num_fighting = 1;
        ch->fighting = fight;
        ch->position = PosFighting;
        victim->num_fighting++;
        if (victim->switched && IsAffected(victim->switched, AffPossess))
        {
                send_to_char("You are disturbed!\n\r", victim->switched);
                do_return(victim->switched, "");
        }
        return;
}


CharData *who_fighting(CharData * ch)
{
        if (!ch)
        {
                bug("who_fighting: null ch", 0);
                return NULL;
        }
        if (!ch->fighting)
                return NULL;
        return ch->fighting->who;
}

void free_fight(CharData * ch)
{
        if (!ch)
        {
                bug("Free_fight: null ch!", 0);
                return;
        }
        if (ch->fighting)
        {
                if (!char_died(ch->fighting->who))
                        --ch->fighting->who->num_fighting;
                DISPOSE(ch->fighting);
        }
        ch->fighting = NULL;
        if (ch->mount)
                ch->position = PosMounted;
        else
                ch->position = PosStanding;
        /*
         * Berserk wears off after combat. -- Altrag 
         */
        if (IsAffected(ch, AffBerserk))
        {
                affect_strip(ch, gsn_berserk);
                set_char_color(AtWearoff, ch);
                send_to_char(skill_table[gsn_berserk]->msg_off, ch);
                send_to_char("\n\r", ch);
        }
        return;
}


/*
 * Stop fights.
 */
void stop_fighting(CharData * ch, bool fBoth)
{
        CharData *fch;

        free_fight(ch);
        update_pos(ch);

        if (!IsNpc(ch) && IsSet(ch->PCData->flags, PcflagAutodraw))
        {
                ObjData *holster1 = get_eq_char(ch, WearHolsterL),
                         *holster2 = get_eq_char(ch, WearHolsterR);
                if ((holster1 && holster1->item_type == ItemHolster && !holster1->first_content) ||
                                (holster2 && holster2->item_type == ItemHolster && !holster2->first_content))
                        do_holster(ch, "");
        }

        if (!fBoth) /* major short cut here by Thoric */
                return;

        for (fch = first_char; fch; fch = fch->next)
        {
                if (who_fighting(fch) == ch)
                {
                        /* cleaner than copying code */
                        stop_fighting(fch, FALSE);
                        /* free_fight(fch);
                        update_pos(fch); */
                }
        }
        return;
}



void death_cry(CharData * ch)
{
        ch = NULL;
        return;
}



void raw_kill(CharData * ch, CharData * victim)
{

        CharData *victmp;

        char      buf[MaxStringLength];
        char      buf2[MaxStringLength];
        char      arg[MaxStringLength];
        ObjData *obj, *obj_next;
        ShipData *ship;

        if (!victim)
        {
                bug("raw_kill: null victim!", 0);
                return;
        }

        mudstrlcpy(arg, victim->name, MSL);

        stop_fighting(victim, TRUE);

        if (!str_cmp(victim->in_room->area->filename, "arena.are")
            || IsSet(victim->in_room->RoomFlags, RoomArena))
        {
                victim->hit = 1;
                update_pos(victim);
                return;
        }

        if (ch && !IsNpc(ch) && !IsNpc(victim))
                claim_disintigration(ch, victim);

/* Take care of polymorphed chars */
        if (IsNpc(victim) && IsSet(victim->act, ActPolymorphed))
        {
                char_from_room(victim->desc->original);
                char_to_room(victim->desc->original, victim->in_room);
                victmp = victim->desc->original;
                do_revert(victim, "");
                raw_kill(ch, victmp);
                return;
        }

        if (victim->in_room && IsNpc(victim)
            && IsSet(victim->act, ActCitizen) && victim->in_room->area
            && victim->in_room->area->planet)
        {
                victim->in_room->area->planet->population--;
                victim->in_room->area->planet->population =
                        UMAX(victim->in_room->area->planet->population, 0);
                victim->in_room->area->planet->pop_support -=
                        (1 +
                         1 / (victim->in_room->area->planet->population + 1));
                if (victim->in_room->area->planet->pop_support < -100)
                        victim->in_room->area->planet->pop_support = -100;
        }

        if (!IsNpc(victim) || !IsSet(victim->act, ActNokill))
                mprog_death_trigger(ch, victim);
        if (char_died(victim))
                return;

        if (!IsNpc(victim) || !IsSet(victim->act, ActNokill))
                rprog_death_trigger(ch, victim);
        if (char_died(victim))
                return;

        if (!IsNpc(victim))
        {
                snprintf(buf, MSL, "%s%c/%s", PlayerDir, tolower(arg[0]),
                         capitalize(arg));
                snprintf(buf2, MSL, "%s%c/%s", BackupDir, tolower(arg[0]),
                         capitalize(arg));

                rename(buf, buf2);
        }

        if (!IsNpc(victim)
            || (!IsSet(victim->act, ActNokill)
                && !IsSet(victim->act, ActNocorpse)))
                make_corpse(victim, ch);
        else
        {
                for (obj = victim->last_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->prev_content;
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }


/*    make_blood( victim ); */

        if (IsNpc(victim))
        {
                victim->pIndexData->killed++;
                extract_char(victim, TRUE);
                victim = NULL;
                return;
        }

        set_char_color(AtDiemsg, victim);
        do_help(victim, "_DIEMSG_");


/* swreality chnages begin here */

/* Check if they have an ongoing auction */
        if (auction->item && (auction->seller == victim))
        {
                talk_auction("Auction has been halted.");
                obj_to_char(auction->item, auction->seller);
                auction->item = NULL;

                if (auction->buyer != NULL
                    && auction->buyer != auction->seller)
                {
                        auction->buyer->gold += auction->bet;
                        send_to_char("&YYour money has been returned.\n\r",
                                     auction->buyer);
                }
        }
        for (ship = first_ship; ship; ship = ship->next)
        {
                if (!str_cmp(ship->owner, victim->name))
                {
                        STRFREE(ship->owner);
                        ship->owner = STRALLOC("");
                        STRFREE(ship->pilot);
                        ship->pilot = STRALLOC("");
                        STRFREE(ship->copilot);
                        ship->copilot = STRALLOC("");

                        save_ship(ship);
                }

        }


        if (victim->plr_home)
        {
                RoomIndexData *room = victim->plr_home;

                STRFREE(room->name);
                room->name = STRALLOC("An Empty Apartment");

                RemoveBit(room->RoomFlags, RoomPlrHome);
                SetBit(room->RoomFlags, RoomEmptyHome);

                fold_area(room->area, room->area->filename, FALSE, TRUE);
        }

        if (victim->PCData && victim->PCData->clan)
        {
                if (!str_cmp(victim->name, victim->PCData->clan->leader))
                {
                        STRFREE(victim->PCData->clan->leader);
                        if (victim->PCData->clan->number1)
                        {
                                victim->PCData->clan->leader =
                                        STRALLOC(victim->PCData->clan->
                                                 number1);
                                STRFREE(victim->PCData->clan->number1);
                                victim->PCData->clan->number1 = STRALLOC("");
                        }
                        else if (victim->PCData->clan->number2)
                        {
                                victim->PCData->clan->leader =
                                        STRALLOC(victim->PCData->clan->
                                                 number2);
                                STRFREE(victim->PCData->clan->number2);
                                victim->PCData->clan->number2 = STRALLOC("");
                        }
                        else
                                victim->PCData->clan->leader = STRALLOC("");
                }

                if (!str_cmp(victim->name, victim->PCData->clan->number1))
                {
                        STRFREE(victim->PCData->clan->number1);
                        if (victim->PCData->clan->number2)
                        {
                                victim->PCData->clan->number1 =
                                        STRALLOC(victim->PCData->clan->
                                                 number2);
                                STRFREE(victim->PCData->clan->number2);
                                victim->PCData->clan->number2 = STRALLOC("");
                        }
                        else
                                victim->PCData->clan->number1 = STRALLOC("");
                }

                if (!str_cmp(victim->name, victim->PCData->clan->number2))
                {
                        STRFREE(victim->PCData->clan->number2);
                        victim->PCData->clan->number1 = STRALLOC("");
                }

                victim->PCData->clan->members--;
                if (victim->PCData->clan->roster)
                        if (hasname
                            (victim->PCData->clan->roster, victim->name))
                                removename(&victim->PCData->clan->roster,
                                           victim->name);
        }

        snprintf(buf, MSL, "%s%c/%s.clone", PlayerDir, tolower(arg[0]),
                 capitalize(arg));
        snprintf(buf2, MSL, "%s%c/%s", PlayerDir, tolower(arg[0]),
                 capitalize(arg));
#ifdef ACCOUNT
        if (rename(buf, buf2) != 0)
        {
                if (victim && victim->PCData && victim->PCData->Account)
                {
                        if (!del_from_account(victim->PCData->Account, victim))
                        {
                                bug("Failed to remove character from Account on death", 0);
                        }
                }
        }
#else
        rename(buf, buf2);
#endif
        if (!victim)
        {
                DescriptorData *d;

                /*
                 * Make sure they aren't halfway logged in. 
                 */
                for (d = first_descriptor; d; d = d->next)
                        if ((victim = d->character) && !IsNpc(victim))
                                break;
                if (d)
                        close_socket(d, TRUE);
        }
        else
        {
                int       x, y;

                quitting_char = victim;
                /*
                 * save_char_obj( victim );
                 */
                saving_char = NULL;
                extract_char(victim, TRUE);
                for (x = 0; x < MaxWear; x++)
                        for (y = 0; y < MaxLayers; y++)
                                save_equipment[x][y] = NULL;
        }
        return;


/* original player kill started here
    
    extract_char( victim, FALSE );
    if ( !victim )
    {
      bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
      return;
    }
    while ( victim->first_affect )
	affect_remove( victim, victim->first_affect );
    victim->affected_by	= race_table[victim->race].affected;
    victim->resistant   = 0;
    victim->susceptible = 0;
    victim->immune      = 0;
    victim->carry_weight= 0;
    victim->Armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck   	= 0;
    victim->Damroll	= 0;
    victim->Hitroll	= 0;
    victim->mental_state = -10;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->SavingSpellStaff = 0;
    victim->position	= PosResting;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->endurance	= UMAX( 1, victim->endurance );
    victim->endurance	= UMAX( 1, victim->endurance );
    
    victim->PCData->condition[CondFull]   = 12;
    victim->PCData->condition[CondThirst] = 12;
    
    if ( IsSet( sysdata.save_flags, SvDeath ) )
	save_char_obj( victim );
    return;

*/

}



void group_gain(CharData * ch, CharData * victim)
{
        char      buf[MaxStringLength];
        CharData *gch;
        CharData *lch;
        int       xp;
        int       members;

        /*
         * Monsters don't get kill xp's or alignment changes.
         * Dying of mortal wounds or poison doesn't give xp to anyone!
         */
        if (IsNpc(ch) || victim == ch)
                return;

        members = 0;

        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                if (is_same_group(gch, ch))
                        members++;
        }

        if (members == 0)
        {
                bug("Group_gain: members.", members);
                members = 1;
        }

        lch = ch->leader ? ch->leader : ch;

        if (IsSet(ch->act, PlrQuestor) && IsNpc(victim))
        {
                if (ch->questmob == victim->pIndexData->vnum)
                {
                        send_to_char
                                ("You have almost completed your QUEST!\n\r",
                                 ch);
                        send_to_char
                                ("Return to the questmaster before your time runs out!\n\r",
                                 ch);
                        ch->questmob = -1;
                }
        }


        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                ObjData *obj;
                ObjData *obj_next;

                if (!is_same_group(gch, ch))
                        continue;

                xp = (int) (xp_compute(gch, victim) / members);

                gch->alignment = align_compute(gch, victim);

                if (!IsNpc(gch) && IsNpc(victim) && gch->PCData
                    && gch->PCData->clan
                    && !str_cmp(gch->PCData->clan->name, victim->mob_clan))
                {
                        xp = 0;
                        snprintf(buf, MSL,
                                 "You receive no experience for killing your organizations resources.\n\r");
                        send_to_char(buf, gch);
                }
                else
                {
                        snprintf(buf, MSL,
                                 "You receive %d combat experience.\n\r", xp);
                        send_to_char(buf, gch);
                }

                gain_exp(gch, xp, CombatAbility);

                if (lch == gch && members > 1)
                {
                        xp = URANGE(members, xp * members,
                                    (exp_level
                                     (gch->skill_level[LeadershipAbility] +
                                      1) -
                                     exp_level(gch->
                                               skill_level
                                               [LeadershipAbility]) / 10));
                        snprintf(buf, MSL,
                                 "You get %d leadership experience for leading your group to victory.\n\r",
                                 xp);
                        send_to_char(buf, gch);
                        gain_exp(gch, xp, LeadershipAbility);
                }


                for (obj = ch->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        if (obj->wear_loc == WearNone)
                                continue;

                        if ((IsObjStat(obj, ItemAntiEvil) && IsEvil(ch))
                            || (IsObjStat(obj, ItemAntiGood)
                                && IsGood(ch))
                            || (IsObjStat(obj, ItemAntiNeutral)
                                && IsNeutral(ch)))
                        {
                                act(AtMagic, "You are zapped by $p.", ch,
                                    obj, NULL, ToChar);
                                act(AtMagic, "$n is zapped by $p.", ch, obj,
                                    NULL, ToRoom);

                                obj_from_char(obj);
                                obj = obj_to_room(obj, ch->in_room);
                                oprog_zap_trigger(ch, obj); /* mudprogs */
                                if (char_died(ch))
                                        return;
                        }
                }
        }

        return;
}


int align_compute(CharData * gch, CharData * victim)
{

/* never cared much for this system

    int align, newalign;

    align = gch->alignment - victim->alignment;

    if ( align >  500 )
	newalign  = UMIN( gch->alignment + (align-500)/4,  1000 );
    else
    if ( align < -500 )
	newalign  = UMAX( gch->alignment + (align+500)/4, -1000 );
    else
	newalign  = gch->alignment - (int) (gch->alignment / 4);
    
    return newalign;

make it simple instead */

        return URANGE(-1000,
                      (int) (gch->alignment - victim->alignment / 5), 1000);

}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */
int xp_compute(CharData * gch, CharData * victim)
{
        int       align;
        int       xp;

        xp = (get_exp_worth(victim)
              * URANGE(1,
                       (victim->skill_level[CombatAbility] -
                        gch->skill_level[CombatAbility]) + 10, 20)) / 10;
        align = gch->alignment - victim->alignment;

        /*
         * Bonus for attacking opposite alignment 
         */
        if (align > 990 || align < -990)
                xp = (xp * 5) >> 2;
        else
                /*
                 * penalty for good attacking same alignment 
                 */
        if (gch->alignment > 300 && align < 250)
                xp = (xp * 3) >> 2;

        xp = number_range((xp * 3) >> 2, (xp * 5) >> 2);

        /*
         * reduce exp for killing the same mob repeatedly       -Thoric 
         */
        if (!IsNpc(gch) && IsNpc(victim))
        {
                int       times = times_killed(gch, victim);

                if (times >= 3)
                        xp = 0;
                else if (times)
                        xp = (xp * (3 - times)) / 3;
        }

        /*
         * new xp cap for swreality 
         */

        return URANGE(1, xp,
                      (exp_level(gch->skill_level[CombatAbility] + 1) -
                       exp_level(gch->skill_level[CombatAbility])) / 5);
}


/*
 * Revamped by Thoric to be more realistic
 */
void dam_message(CharData * ch, CharData * victim, int dam, int dt)
{
        char      buf1[256], buf2[256], buf3[256];
        const char *vs;
        const char *vp;
        const char *attack;
        char      punct;
        sh_int    dampc;
        struct skill_type *skill = NULL;
        bool      gcflag = FALSE;
        bool      gvflag = FALSE;

        if (!dam)
                dampc = 0;
        else
                dampc = ((dam * 1000) / victim->max_hit) +
                        (50 - ((victim->hit * 50) / victim->max_hit));

        /*
         * 10 * percent                  
         */
        if (dam == 0)
        {
                vs = "&Rmiss&R&W";
                vp = "&Rmisses&R&W";
        }
        else if (dampc <= 5)
        {
                vs = "&Rbarely scratch&R&W";
                vp = "&Rbarely scratches&R&W";
        }
        else if (dampc <= 10)
        {
                vs = "&Rscratch&R&W";
                vp = "&Rscratches&R&W";
        }
        else if (dampc <= 20)
        {
                vs = "&Rnick&R&W";
                vp = "&Rnicks&R&W";
        }
        else if (dampc <= 30)
        {
                vs = "&Rgraze&R&W";
                vp = "&Rgrazes&R&W";
        }
        else if (dampc <= 40)
        {
                vs = "&Rbruise&R&W";
                vp = "&Rbruises&R&W";
        }
        else if (dampc <= 50)
        {
                vs = "&Rhit&R&W";
                vp = "&Rhits&R&W";
        }
        else if (dampc <= 60)
        {
                vs = "&Rinjure&R&W";
                vp = "&Rinjures&R&W";
        }
        else if (dampc <= 75)
        {
                vs = "&Rthrash&R&W";
                vp = "&Rthrashes&R&W";
        }
        else if (dampc <= 80)
        {
                vs = "&Rwound&R&W";
                vp = "&Rwounds&R&W";
        }
        else if (dampc <= 90)
        {
                vs = "&Rmaul&R&W";
                vp = "&Rmauls&R&W";
        }
        else if (dampc <= 125)
        {
                vs = "&Rdecimate&R&W";
                vp = "&Rdecimates&R&W";
        }
        else if (dampc <= 150)
        {
                vs = "&Rdevastate&R&W";
                vp = "&Rdevastates&R&W";
        }
        else if (dampc <= 200)
        {
                vs = "&Rmaim&R&W";
                vp = "&Rmaims&R&W";
        }
        else if (dampc <= 300)
        {
                vs = "&Rmutilate&R&W";
                vp = "&Rmutilates&R&W";
        }
        else if (dampc <= 400)
        {
                vs = "&Rdisembowels&R&W";
                vp = "&Rdisembowels&R&W";
        }
        else if (dampc <= 500)
        {
                vs = "&Rmassacre&R&W";
                vp = "&Rmassacres&R&W";
        }
        else if (dampc <= 600)
        {
                vs = "&Rpulverizes&R&W";
                vp = "&Rpulverizes&R&W";
        }
        else if (dampc <= 750)
        {
                vs = "&Rerviscerate&R&W";
                vp = "&Reviscerate&R&W";
        }
        else if (dampc <= 990)
        {
                vs = "&Robliterte&R&W";
                vp = "&Robliterates&R&W";
        }
        else
        {
                vs = "&Rannihilate&R&W";
                vp = "&Rannihilates&R&W";
        }
        punct = (dampc <= 30) ? '.' : '!';

        if (dam == 0 && (!IsNpc(ch) &&
                         (IsSet(ch->PCData->flags, PcflagGag))))
                gcflag = TRUE;

        if (dam == 0 && (!IsNpc(victim) &&
                         (IsSet(victim->PCData->flags, PcflagGag))))
                gvflag = TRUE;

        if (dt >= 0 && dt < top_sn)
                skill = skill_table[dt];

        if (dt == (TypeHit + WeaponBlaster))
        {
                char      sound[MaxStringLength];
                int       vol = number_range(20, 80);

                /*
                 * FIXME 
                 */
                snprintf(sound, MSL, "!!SOUND(blaster V=%d)", vol);
                sound_to_room(ch->in_room, "blaster");
        }

        if (dt == TypeHit || dam == 0)
        {
                snprintf(buf1, MSL, "&c&C$n %s &c&C$N%c", vp, punct);
                snprintf(buf2, MSL, "&c&CYou %s &c&C$N%c", vs, punct);
                snprintf(buf3, MSL, "&c&C$n %s &c&Cyou%c", vp, punct);
        }
        else if (dt > TypeHit && is_wielding_poisoned(ch))
        {
                if (dt <
                    TypeHit +
                    (int) sizeof(attack_table) /
                    (int) sizeof(attack_table[0]))
                        attack = attack_table[dt - TypeHit];
                else
                {
                        bug("Dam_message: bad dt %d.", dt);
                        dt = TypeHit;
                        attack = attack_table[0];
                }

                snprintf(buf1, MSL, "$n's poisoned %s %s $N%c", attack, vp,
                         punct);
                snprintf(buf2, MSL, "Your poisoned %s %s $N%c", attack, vp,
                         punct);
                snprintf(buf3, MSL, "$n's poisoned %s %s you%c", attack, vp,
                         punct);
        }
        else
        {
                if (skill)
                {
                        attack = skill->noun_damage;
                        if (dam == 0)
                        {
                                bool      found = FALSE;

                                if (skill->miss_char
                                    && skill->miss_char[0] != '\0')
                                {
                                        act(AtHit, skill->miss_char, ch,
                                            NULL, victim, ToChar);
                                        found = TRUE;
                                }
                                if (skill->miss_vict
                                    && skill->miss_vict[0] != '\0')
                                {
                                        act(AtHitme, skill->miss_vict, ch,
                                            NULL, victim, ToVict);
                                        found = TRUE;
                                }
                                if (skill->miss_room
                                    && skill->miss_room[0] != '\0')
                                {
                                        act(AtAction, skill->miss_room, ch,
                                            NULL, victim, ToNotvict);
                                        found = TRUE;
                                }
                                if (found)  /* miss message already sent */
                                        return;
                        }
                        else
                        {
                                if (skill->hit_char
                                    && skill->hit_char[0] != '\0')
                                        act(AtHit, skill->hit_char, ch, NULL,
                                            victim, ToChar);
                                if (skill->hit_vict
                                    && skill->hit_vict[0] != '\0')
                                        act(AtHitme, skill->hit_vict, ch,
                                            NULL, victim, ToVict);
                                if (skill->hit_room
                                    && skill->hit_room[0] != '\0')
                                        act(AtAction, skill->hit_room, ch,
                                            NULL, victim, ToNotvict);
                        }
                }
                else if (dt >= TypeHit
                         && dt <
                         TypeHit +
                         (int) sizeof(attack_table) /
                         (int) sizeof(attack_table[0]))
                        attack = attack_table[dt - TypeHit];
                else
                {
                        bug("Dam_message: bad dt %d.", dt);
                        dt = TypeHit;
                        attack = attack_table[0];
                }

                snprintf(buf1, MSL, "&c&C$n's %s %s &c&C$N%c&R&W", attack, vp,
                         punct);
                snprintf(buf2, MSL, "&c&CYour %s %s &c&C$N%c&R&W", attack, vp,
                         punct);
                snprintf(buf3, MSL, "&c&C$n's %s %s &c&Cyou%c&R&W", attack,
                         vp, punct);
        }

        if (ch->skill_level[CombatAbility] >= 100)
        {
                char      tmp_buf[MSL];

                /* Use step-by-step buffer building to avoid truncation warnings */
                mudstrlcpy(tmp_buf, buf2, MSL);
                size_t len = 0;
                buf2[0] = '\0';
                len += snprintf(buf2 + len, MSL - len, "%s", tmp_buf);
                len += snprintf(buf2 + len, MSL - len, " &WYou do &R%d", dam);
                len += snprintf(buf2 + len, MSL - len, " &R&Wpoints of damage.&R&W");
        }
        act(AtAction, buf1, ch, NULL, victim, ToNotvict);
        if (!gcflag)
                act(AtHit, buf2, ch, NULL, victim, ToChar);
        if (!gvflag)
                act(AtHitme, buf3, ch, NULL, victim, ToVict);

        return;
}


CMDF do_kill(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Kill whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!IsNpc(victim))
        {
                send_to_char("You must MURDER a player.\n\r", ch);
                return;
        }

        /*
         *
         else
         {
         if ( IsAffected(victim, AffCharm) && victim->master != NULL )
         {
         send_to_char( "You must MURDER a charmed creature.\n\r", ch );
         return;
         }
         }
         *
         */

        if (victim == ch)
        {
                send_to_char("You hit yourself.  Ouch!\n\r", ch);
                multi_hit(ch, ch, TypeUndefined);
                return;
        }

        if (is_safe(ch, victim))
                return;

        if (IsAffected(ch, AffCharm) && ch->master == victim)
        {
                act(AtPlain, "$N is your beloved master.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if (ch->position == PosFighting)
        {
                send_to_char("You do the best you can!\n\r", ch);
                return;
        }

        if (IsSet(victim->act, ActCitizen))
                ch->alignment -= 10;

        WaitState(ch, 1 * PulseViolence);
        multi_hit(ch, victim, TypeUndefined);
        return;
}



CMDF do_murde(CharData * ch, char *argument)
{
        argument = NULL;
        send_to_char("If you want to MURDER, spell it out.\n\r", ch);
        return;
}



CMDF do_murder(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Murder whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("Suicide is a mortal sin.\n\r", ch);
                return;
        }

        if (is_safe(ch, victim))
                return;

        if (IsAffected(ch, AffCharm))
        {
                if (ch->master == victim)
                {
                        act(AtPlain, "$N is your beloved master.", ch, NULL,
                            victim, ToChar);
                        return;
                }
        }

        if (ch->position == PosFighting)
        {
                send_to_char("You do the best you can!\n\r", ch);
                return;
        }

        if (!IsNpc(victim) && IsSet(ch->act, PlrNice))
        {
                send_to_char("You feel too nice to do that!\n\r", ch);
                return;
        }

        ch->alignment -= 10;

        WaitState(ch, 1 * PulseViolence);
        multi_hit(ch, victim, TypeUndefined);
        return;
}


CMDF do_flee(CharData * ch, char *argument)
{
        RoomIndexData *was_in;
        RoomIndexData *now_in;
        char      buf[MaxStringLength];
        int       attempt;
        sh_int    door;
        ExitData *pexit;

        argument = NULL;

        if (!who_fighting(ch))
        {
                if (ch->position == PosFighting)
                {
                        if (ch->mount)
                                ch->position = PosMounted;
                        else
                                ch->position = PosStanding;
                }
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        if (ch->endurance <= 0)
        {
                send_to_char("You're too exhausted to flee from combat!\n\r",
                             ch);
                return;
        }

        /*
         * No fleeing while stunned. - Narn 
         */
        if (ch->position < PosFighting)
                return;

        was_in = ch->in_room;
        for (attempt = 0; attempt < 8; attempt++)
        {

                door = number_door();
                if ((pexit = get_exit(was_in, door)) == NULL
                    || !pexit->to_room
                    || (IsSet(pexit->exit_info, ExClosed)
                        && !IsAffected(ch, AffPassDoor))
                    || (IsNpc(ch)
                        && IsSet(pexit->to_room->RoomFlags, RoomNoMob)))
                        continue;

                affect_strip(ch, gsn_sneak);
                RemoveBit(ch->affected_by, AffSneak);
                if (ch->mount && ch->mount->fighting)
                        stop_fighting(ch->mount, TRUE);
                move_char(ch, pexit, 0, FALSE);
                if ((now_in = ch->in_room) == was_in)
                        continue;

                ch->in_room = was_in;
                act(AtFlee, "$n runs for cover!", ch, NULL, NULL, ToRoom);
                ch->in_room = now_in;
                act(AtFlee, "$n glances around for signs of pursuit.", ch,
                    NULL, NULL, ToRoom);
                snprintf(buf, MSL, "You run for cover!");
                send_to_char(buf, ch);

                stop_fighting(ch, TRUE);
                return;
        }

        snprintf(buf, MSL, "You attempt to run for cover!");
        send_to_char(buf, ch);
        return;
}

bool get_cover(CharData * ch)
{
        RoomIndexData *was_in;
        RoomIndexData *now_in;
        int       attempt;
        sh_int    door;
        ExitData *pexit;

        if (!who_fighting(ch))
                return FALSE;

        if (ch->position < PosFighting)
                return FALSE;

        was_in = ch->in_room;
        for (attempt = 0; attempt < 10; attempt++)
        {

                door = number_door();
                if ((pexit = get_exit(was_in, door)) == NULL
                    || !pexit->to_room
                    || (IsSet(pexit->exit_info, ExClosed)
                        && !IsAffected(ch, AffPassDoor))
                    || (IsNpc(ch)
                        && IsSet(pexit->to_room->RoomFlags, RoomNoMob)))
                        continue;

                affect_strip(ch, gsn_sneak);
                RemoveBit(ch->affected_by, AffSneak);
                if (ch->mount && ch->mount->fighting)
                        stop_fighting(ch->mount, TRUE);
                move_char(ch, pexit, 0, FALSE);
                if ((now_in = ch->in_room) == was_in)
                        continue;

                ch->in_room = was_in;
                act(AtFlee, "$n sprints for cover!", ch, NULL, NULL,
                    ToRoom);
                ch->in_room = now_in;
                act(AtFlee, "$n spins around and takes aim.", ch, NULL, NULL,
                    ToRoom);

                stop_fighting(ch, TRUE);

                return TRUE;
        }

        return FALSE;
}



CMDF do_sla(CharData * ch, char *argument)
{
        argument = NULL;
        send_to_char("If you want to SLAY, spell it out.\n\r", ch);
        return;
}



CMDF do_slay(CharData * ch, char *argument)
{
        CharData *victim;
        char      arg[MaxInputLength];
        char      arg2[MaxInputLength];

        argument = one_argument(argument, arg);
        one_argument(argument, arg2);
        if (arg[0] == '\0')
        {
                send_to_char("Slay whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (ch == victim)
        {
                send_to_char("Suicide is a mortal sin.\n\r", ch);
                return;
        }

        if (!IsNpc(victim)
            && (get_trust(victim) == 103 || get_trust(ch) < 103))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "immolate"))
        {
                act(AtFire, "Your fireball turns $N into a blazing inferno.",
                    ch, NULL, victim, ToChar);
                act(AtFire,
                    "$n releases a searing fireball in your direction.", ch,
                    NULL, victim, ToVict);
                act(AtFire,
                    "$n points at $N, who bursts into a flaming inferno.", ch,
                    NULL, victim, ToNotvict);
        }

        else if (!str_cmp(arg2, "shatter"))
        {
                act(AtLblue,
                    "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",
                    ch, NULL, victim, ToChar);
                act(AtLblue,
                    "$n freezes you with a glance and shatters your frozen body into tiny shards.",
                    ch, NULL, victim, ToVict);
                act(AtLblue,
                    "$n freezes $N with a glance and shatters the frozen body into tiny shards.",
                    ch, NULL, victim, ToNotvict);
        }

        else if (!str_cmp(arg2, "demon"))
        {
                act(AtImmort,
                    "You gesture, and a slavering demon appears.  With a horrible grin, the",
                    ch, NULL, victim, ToChar);
                act(AtImmort,
                    "foul creature turns on $N, who screams in panic before being eaten alive.",
                    ch, NULL, victim, ToChar);
                act(AtImmort,
                    "$n gestures, and a slavering demon appears.  The foul creature turns on",
                    ch, NULL, victim, ToVict);
                act(AtImmort,
                    "you with a horrible grin.   You scream in panic before being eaten alive.",
                    ch, NULL, victim, ToVict);
                act(AtImmort,
                    "$n gestures, and a slavering demon appears.  With a horrible grin, the",
                    ch, NULL, victim, ToNotvict);
                act(AtImmort,
                    "foul creature turns on $N, who screams in panic before being eaten alive.",
                    ch, NULL, victim, ToNotvict);
        }

        else if (!str_cmp(arg2, "pounce") && get_trust(ch) >= LevelAscendant)
        {
                act(AtBlood,
                    "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",
                    ch, NULL, victim, ToChar);
                act(AtBlood,
                    "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...",
                    ch, NULL, victim, ToVict);
                act(AtBlood,
                    "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",
                    ch, NULL, victim, ToNotvict);
        }

        else if (!str_cmp(arg2, "slit") && get_trust(ch) >= LevelAscendant)
        {
                act(AtBlood, "You calmly slit $N's throat.", ch, NULL,
                    victim, ToChar);
                act(AtBlood,
                    "$n reaches out with a clawed finger and calmly slits your throat.",
                    ch, NULL, victim, ToVict);
                act(AtBlood, "$n calmly slits $N's throat.", ch, NULL,
                    victim, ToNotvict);
        }

        else
        {
                act(AtImmort, "You slay $N in cold blood!", ch, NULL, victim,
                    ToChar);
                act(AtImmort, "$n slays you in cold blood!", ch, NULL,
                    victim, ToVict);
                act(AtImmort, "$n slays $N in cold blood!", ch, NULL, victim,
                    ToNotvict);
        }

        raw_kill(ch, victim);
        return;
}

/* In fight.c */
CMDF do_trip(CharData * ch, char *argument)
{
        CharData *victim;

        argument = NULL;
        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if ((victim = who_fighting(ch)) == NULL)
        {
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_trip]->beats);
        if (IsNpc(ch) || number_percent() < ch->PCData->learned[gsn_trip])
        {
                act(AtHit, "You pull $N's legs from underneath $M.", ch,
                    NULL, victim, ToChar);
                act(AtHitme, "$n pulls your legs from underneath you.", ch,
                    NULL, victim, ToVict);
                act(AtAction, "$n pulls $N's legs from underneath $M.", ch,
                    NULL, victim, ToNotvict);
                learn_from_success(ch, gsn_trip);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_trip);
        }
        else
        {
                act(AtHit, "You swing your foot but connect with nothing.",
                    ch, NULL, victim, ToChar);
                act(AtHitme, "$n swing $s foot but connect with nothing.",
                    ch, NULL, NULL, ToRoom);
                learn_from_failure(ch, gsn_trip);
                global_retcode = damage(ch, victim, 0, gsn_trip);
        }

        if (number_percent() <= 25 && !IsImmortal(ch))
        {
                if (!IsNpc(victim))
                {
                        if (!IsSet(victim->bodyparts, BodyLLeg))
                        {
                                act(AtHitme, "You hit $N's legs!", ch, NULL,
                                    victim, ToChar);
                                act(AtHitme, "Ouch!", ch, NULL, victim,
                                    ToVict);
                                SetBit(victim->bodyparts, BodyLLeg);
                        }
                        else if (!IsSet(victim->bodyparts, BodyRLeg))
                        {
                                act(AtHitme, "You hit $N's legs!", ch, NULL,
                                    victim, ToChar);
                                act(AtHitme, "Ouch!", ch, NULL, victim,
                                    ToVict);
                                SetBit(victim->bodyparts, BodyRLeg);
                        }
                }
        }
        return;
}
