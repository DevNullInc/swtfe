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
 * Game update and maintenance routines for periodic tasks and world state management. *
 ****************************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.hpp"
#include "homes.hpp"
#include "olc-shuttle.hpp"
#include "account.hpp"
#include "races.hpp"
#include "space2.hpp"
#include "installations.hpp"

/* from swskills.c
 * Local functions.
 */
int hit_gain args((CharData * ch));
int mana_gain args((CharData * ch));
int move_gain args((CharData * ch));
void gain_addiction args((CharData * ch));
void mobile_update args((void));
void weather_update args((void));
void update_taxes args((void));
void update_salaries args((void));
void      update_baccounts();

void update_orbit args((void));

void battalion_update args((void));
void char_update args((void));
void obj_update args((void));
void aggr_update args((void));
void room_act_update args((void));
void obj_act_update args((void));
void char_check args((void));
void bacta_update args((void));
void quest_update args((void)); /* Vassago - quest.c */
void drunk_randoms args((CharData * ch));
void halucinations args((CharData * ch));
void subtract_times args((struct timeval * etime, struct timeval * systime));
void init_crashover args((void));
void arena_update args((void));

/*
 * Global Variables
 */
CharData *gch_prev;
ObjData *gobj_prev;

CharData *timechar;

char     *corpse_descs[] = {
        "The corpse of %s will soon be gone.",
        "The corpse of %s lies here.",
        "The corpse of %s lies here.",
        "The corpse of %s lies here.",
        "The corpse of %s lies here."
};

char     *d_corpse_descs[] = {
        "The shattered remains %s will soon be gone.",
        "The shattered remains %s are here.",
        "The shattered remains %s are here.",
        "The shattered remains %s are here.",
        "The shattered remains %s are here."
};

extern int top_exit;

/*
 * Advancement stuff.
 */
int max_level(CharData * ch, int ability)
{
        int       level = 0;

        if (IsNpc(ch))
                return 100;

        if (IsImmortal(ch))
                return 200;

        if (ability == CombatAbility)
        {
                if (ch->main_ability == CombatAbility)
                        level = 100;
                if (ch->main_ability == ForceAbility)
                        level = 25;
                if (ch->main_ability == HuntingAbility)
                        level = 25;
                if (ch->main_ability == ForceAbility)
                        level = 50;
                if (ch->sex == SexMale)
                        level += 5;
                if (ch->sex == SexFemale)
                        level -= 5;
                level += ch->perm_con + ch->perm_dex + ch->perm_str;
        }
        if (ability == PilotingAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == EngineeringAbility)
                        level = 25;
                if (ch->main_ability == HuntingAbility)
                        level = 25;
                if (ch->main_ability == SmugglingAbility)
                        level = 50;
                if (ch->main_ability == ForceAbility)
                        level = 30;
                if (ch->main_ability == PiracyAbility)
                        level = 50;
                if (ch->sex == SexMale)
                        level -= 10;
                if (ch->sex == SexFemale)
                        level += 10;
                level += ch->perm_dex * 2;
        }
        if (ability == EngineeringAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PilotingAbility)
                        level = 20;
                if (ch->main_ability == ForceAbility)
                        level = 30;
                if (ch->sex == SexMale)
                        level -= 5;
                if (ch->sex == SexFemale)
                        level += 5;
                level += ch->perm_int * 2;
        }
        if (ability == HuntingAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == ForceAbility)
                        level = 40;
                if (ch->sex == SexMale)
                        level += 10;
                if (ch->sex == SexFemale)
                        level -= 10;
        }
        if (ability == SmugglingAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PilotingAbility)
                        level = 20;
                if (ch->main_ability == EngineeringAbility)
                        level = 25;
                if (ch->main_ability == PiracyAbility)
                        level = 40;
                if (ch->main_ability == ForceAbility)
                        level = 50;
                level += ch->perm_lck * 2;
        }
        if (ability == LeadershipAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == CombatAbility)
                        level = 50;
                if (ch->main_ability == DiplomacyAbility)
                        level = 50;
                if (ch->main_ability == ForceAbility)
                        level = 30;
                if (ch->sex == SexMale)
                        level += 5;
                if (ch->sex == SexFemale)
                        level -= 5;
                level += ch->perm_wis + ch->perm_cha + ch->perm_int;
        }
        if (ability == DiplomacyAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PilotingAbility)
                        level = 10;
                if (ch->main_ability == LeadershipAbility)
                        level = 50;
                if (ch->main_ability == ForceAbility)
                        level = 30;
                if (ch->sex == SexMale)
                        level -= 5;
                if (ch->sex == SexFemale)
                        level += 5;
                level += ch->perm_cha * 3;
        }
        if (ability == OccupationAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == CombatAbility)
                        level = 60;
                if (ch->main_ability == PilotingAbility)
                        level = 50;
                if (ch->main_ability == EngineeringAbility)
                        level = 80;
                if (ch->main_ability == HuntingAbility)
                        level = 30;
                if (ch->main_ability == SmugglingAbility)
                        level = 40;
                if (ch->main_ability == DiplomacyAbility)
                        level = 100;
                if (ch->main_ability == LeadershipAbility)
                        level = 90;
                if (ch->main_ability == ForceAbility)
                        level = 70;
                if (ch->main_ability == PiracyAbility)
                        level = 40;
                if (ch->main_ability == MedicAbility)
                        level = 90;
        }
        if (ability == PiracyAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PilotingAbility)
                        level = 20;
                if (ch->main_ability == HuntingAbility)
                        level = 30;
                if (ch->main_ability == SmugglingAbility)
                        level = 40;
                if (ch->main_ability == ForceAbility)
                        level = 35;
                level += ch->perm_dex + ch->perm_lck;
        }
        if (ability == MedicAbility)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == CombatAbility)
                        level = 20;
                if (ch->main_ability == ForceAbility)
                        level = 50;
                level += ch->perm_dex + ch->perm_int + ch->perm_lck;
        }
        level += ch->race->class_modifier(ability);
        level = URANGE(1, level, 150);
        if (ability == ForceAbility)
        {
                level = ch->perm_frc * 5;
        }
        return level;
}

void advance_level(CharData * ch, int ability)
{

        if (ch->top_level < ch->skill_level[ability] && ch->top_level < 100)
        {
                ch->top_level = URANGE(1, ch->skill_level[ability], 100);
        }

        if (!IsNpc(ch))
                RemoveBit(ch->act, PlrBoughtPet);
        return;
}

void gain_exp_new(CharData * ch, int gain, int ability, bool outtext)
{
        /*
         * why were these global variables? - Gavin 
         */
        int       hpgain;
        int       mvgain;

        if (IsNpc(ch))
                return;

        if (IsSet(ch->act, PlrForsaken))
        {
                send_to_char
                        ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                         ch);
                return;
        }

        ch->experience[ability] = UMAX(0, ch->experience[ability] + gain);

        if (NotAuthed(ch)
            && ch->experience[ability] >=
            exp_level(ch->skill_level[ability] + 1))
        {
                send_to_char
                        ("You can not ascend to a higher level until you are authorized.\n\r",
                         ch);
                ch->experience[ability] =
                        (exp_level(ch->skill_level[ability] + 1) - 1);
                return;
        }

        while (ch->experience[ability] >=
               exp_level(ch->skill_level[ability] + 1))
        {
                if (ch->skill_level[ability] >= max_level(ch, ability))
                {
                        ch->experience[ability] =
                                (exp_level(ch->skill_level[ability] + 1) - 1);
                        return;
                }
                ++ch->skill_level[ability];
                if (outtext)
                {
                        set_char_color(AtWhite + AtBlink, ch);
                        ch_printf(ch,
                                  "You have now obtained %s level %d!\n\r",
                                  ability_name[ability],
                                  ch->skill_level[ability]);
                }
                advance_level(ch, ability);

                if (ability == CombatAbility)
                {
                        hpgain = (int) (1 + (ch->perm_con * 0.25));
                        ch->max_hit += hpgain;
                        if (outtext)
                                send_to_char
                                        ("Your maximum hit points have increased!.\n\r",
                                         ch);
                }

                if (ability == HuntingAbility)
                {
                        mvgain = 0 + (ch->perm_dex);
                        ch->max_endurance += mvgain;
                        if (outtext)
                                send_to_char
                                        ("Your maximum movement points have increased!.\n\r",
                                         ch);
                }

                if (ability == ForceAbility)
                {
                        ch->max_endurance += 20;
                        if (outtext)
                                send_to_char
                                        ("Your maximum endurance has increased!.\n\r",
                                         ch);
                }

                if (ch->skill_level[PilotingAbility] == 100)
                {
                        ch->bonus_dex = 1;
                        if (outtext)
                                send_to_char
                                        ("Your Dexterity has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[PilotingAbility] == 140)
                {
                        ch->bonus_dex = 4;
                        if (outtext)
                                send_to_char
                                        ("Your Dexterity has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[SmugglingAbility] == 50)
                {
                        ch->bonus_lck = 1;
                        if (outtext)
                                send_to_char("Your Luck has increased!\n\r",
                                             ch);
                }

                if (ch->skill_level[SmugglingAbility] == 100)
                {
                        ch->bonus_lck = 2;
                        if (outtext)
                                send_to_char("Your Luck has increased!\n\r",
                                             ch);
                }

                if (ch->skill_level[SmugglingAbility] == 150)
                {
                        ch->bonus_lck = 3;
                        if (outtext)
                                send_to_char("Your Luck has increased!\n\r",
                                             ch);
                }

                if (ch->skill_level[EngineeringAbility] == 100)
                {
                        ch->bonus_int = 1;
                        if (outtext)
                                send_to_char
                                        ("Your Intelligence has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[EngineeringAbility] == 140)
                {
                        ch->bonus_int = 4;
                        if (outtext)
                                send_to_char
                                        ("Your Intelligence has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[DiplomacyAbility] == 100)
                {
                        ch->bonus_cha = 1;
                        if (outtext)
                                send_to_char
                                        ("Your Charisma has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[DiplomacyAbility] == 140)
                {
                        ch->bonus_cha = 4;
                        if (outtext)
                                send_to_char
                                        ("Your Charisma has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[PiracyAbility] == 100)
                {
                        ch->bonus_con = 1;
                        if (outtext)
                                send_to_char
                                        ("Your Constitution has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[PiracyAbility] == 150)
                {
                        ch->bonus_con = 3;
                        if (outtext)
                                send_to_char
                                        ("Your Constitution has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[MedicAbility] == 100)
                {
                        ch->bonus_wis = 1;
                        if (outtext)
                                send_to_char
                                        ("Your Wisdom has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[MedicAbility] == 150)
                {
                        ch->bonus_wis = 3;
                        if (outtext)
                                send_to_char
                                        ("Your Wisdom has increased by one point!\n\r",
                                         ch);
                }
        }
        return;
}

void gain_exp(CharData * ch, int gain, int ability)
{
        gain_exp_new(ch, gain, ability, TRUE);
        return;
}

/*
 * Regeneration stuff.
 */
int hit_gain(CharData * ch)
{
        int       gain;

        if (IsNpc(ch))
        {
                gain = ch->top_level;
        }
        else
        {
                if (IsSet(ch->act, PlrForsaken))
                {
                        send_to_char
                                ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                                 ch);
                        return 0;
                }

                gain = UMIN(5, ch->top_level);

                switch (ch->position)
                {
                case PosDead:
                        return 0;
                case PosMortal:
                        return -25;
                case PosIncap:
                        return -20;
                case PosStunned:
                        return get_curr_con(ch) * 2;
                case PosSleeping:
                        gain += (int) (get_curr_con(ch) * 1.5);
                        break;
                case PosResting:
                        gain += get_curr_con(ch);
                        break;
                }

                if (ch->PCData->condition[CondFull] == 0)
                        gain /= 2;

                if (ch->PCData->condition[CondThirst] == 0)
                        gain /= 2;

        }

        if (IsAffected(ch, AffPoison))
                gain /= 4;

        if (ch->race && ch->race->death_age() != -1)
        {
                if (get_age(ch) > ch->race->death_age())
                {
                        send_to_char
                                ("You are very old.\n\rYou are becoming weaker with every moment.\n\rSoon you will die.\n\r",
                                 ch);
                        return -10;
                }
                else if (get_age(ch) > (ch->race->death_age() / 1.6))
                        gain /= 10;
                else if (get_age(ch) > (ch->race->death_age() / 2))
                        gain /= 5;
                else if (get_age(ch) > (ch->race->death_age() / 2.6))
                        gain /= 2;
        }

        /*
         * FIXME - shouldn't be hardcoded 
         */
        if (ch->race && !str_cmp(ch->race->name(), "trandoshan"))
                gain *= 4;

        return UMIN(gain, ch->max_hit - ch->hit);
}


int mana_gain(CharData * ch)
{
        int       gain;

        if (IsNpc(ch))
        {
                gain = ch->top_level;
        }
        else
        {
                if (IsSet(ch->act, PlrForsaken))
                {
                        send_to_char
                                ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                                 ch);
                        return 0;
                }
                if (ch->skill_level[ForceAbility] <= 1)
                        return (0);

                gain = UMIN(5, ch->skill_level[ForceAbility] / 2);

                if (ch->position < PosSleeping)
                        return 0;
                switch (ch->position)
                {
                case PosSleeping:
                        gain += get_curr_int(ch) * 3;
                        break;
                case PosResting:
                        gain += (int) (get_curr_int(ch) * 1.5);
                        break;
                }

                if (ch->PCData->condition[CondFull] == 0)
                        gain /= 2;

                if (ch->PCData->condition[CondThirst] == 0)
                        gain /= 2;

        }

        if (IsAffected(ch, AffPoison))
                gain /= 4;

        return UMIN(gain, ch->max_endurance - ch->endurance);
}

int move_gain(CharData * ch)
{
        int       gain;

        if (IsNpc(ch))
        {
                gain = ch->top_level;
        }
        else
        {
                if (IsSet(ch->act, PlrForsaken))
                {
                        send_to_char
                                ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                                 ch);
                        return 0;
                }
                gain = UMAX(15, 2 * ch->top_level);
                switch (ch->position)
                {
                case PosDead:
                        return 0;
                case PosMortal:
                        return -1;
                case PosIncap:
                        return -1;
                case PosStunned:
                        return 1;
                case PosSleeping:
                        gain += get_curr_dex(ch) * 2;
                        break;
                case PosResting:
                        gain += get_curr_dex(ch);
                        break;
                }
                if (ch->PCData->condition[CondFull] == 0)
                        gain /= 2;
                if (ch->PCData->condition[CondThirst] == 0)
                        gain /= 2;
        }
        if (IsSet(ch->bodyparts, BodyRLeg))
                gain -= 10;
        if (ch->race && ch->race->death_age() != -1)
        {
                if (get_age(ch) > ch->race->death_age())
                {
                        send_to_char
                                ("You are very old.\n\rYou are becoming weaker with every moment.\n\rSoon you will die.\n\r",
                                 ch);
                        return -10;
                }
                else if (get_age(ch) > (ch->race->death_age() / 1.6))
                        gain /= 10;
                else if (get_age(ch) > (ch->race->death_age() / 2))
                        gain /= 5;
                else if (get_age(ch) > (ch->race->death_age() / 2.6))
                        gain /= 2;

        }
        if (IsSet(ch->bodyparts, BodyLLeg))
                gain -= 10;
        if (IsSet(ch->bodyparts, BodyRLeg))
                gain -= 10;

        /*
         * Both are broken? HAHAHAHA okay, screw them over more 
         */
        if (IsSet(ch->bodyparts, BodyLLeg)
            && IsSet(ch->bodyparts, BodyRLeg))
                gain -= 10;

        return UMIN(gain, ch->max_endurance - ch->endurance);
}

void gain_addiction(CharData * ch)
{
        short     drug;
        ch_ret    retcode;
        AffectData af;

        for (drug = 0; drug <= 9; drug++)
        {

                if (ch->PCData->addiction[drug] <
                    ch->PCData->drug_level[drug])
                        ch->PCData->addiction[drug]++;

                if (ch->PCData->addiction[drug] >
                    ch->PCData->drug_level[drug] + 150)
                {
                        switch (ch->PCData->addiction[drug])
                        {
                        default:
                        case SpiceGlitterstim:
                                if (!IsAffected(ch, AffBlind))
                                {
                                        af.type = gsn_blindness;
                                        af.location = ApplyAc;
                                        af.modifier = 10;
                                        af.duration =
                                                ch->PCData->addiction[drug];
                                        af.bitvector = AffBlind;
                                        affect_to_char(ch, &af);
                                }
                        case SpiceCarsanum:
                                if (!IsAffected(ch, AffWeaken))
                                {
                                        af.type = -1;
                                        af.location = ApplyDamroll;
                                        af.modifier = -10;
                                        af.duration =
                                                ch->PCData->addiction[drug];
                                        af.bitvector = AffWeaken;
                                        affect_to_char(ch, &af);
                                }
                        case SpiceRyll:
                                if (!IsAffected(ch, AffWeaken))
                                {
                                        af.type = -1;
                                        af.location = ApplyDex;
                                        af.modifier = -5;
                                        af.duration =
                                                ch->PCData->addiction[drug];
                                        af.bitvector = AffWeaken;
                                        affect_to_char(ch, &af);
                                }
                        case SpiceAndris:
                                if (!IsAffected(ch, AffWeaken))
                                {
                                        af.type = -1;
                                        af.location = ApplyCon;
                                        af.modifier = -5;
                                        af.duration =
                                                ch->PCData->addiction[drug];
                                        af.bitvector = AffWeaken;
                                        affect_to_char(ch, &af);
                                }
                        }
                }

                if (ch->PCData->addiction[drug] >
                    ch->PCData->drug_level[drug] + 200)
                {
                        ch_printf(ch,
                                  "You feel like you are going to die. You NEED %s\n\r.",
                                  spice_table[drug]);
                        worsen_mental_state(ch, 2);
                        retcode = damage(ch, ch, 5, TypeUndefined);
                }
                else if (ch->PCData->addiction[drug] >
                         ch->PCData->drug_level[drug] + 100)
                {
                        ch_printf(ch, "You need some %s.\n\r",
                                  spice_table[drug]);
                        worsen_mental_state(ch, 2);
                }
                else if (ch->PCData->addiction[drug] >
                         ch->PCData->drug_level[drug] + 50)
                {
                        ch_printf(ch, "You really crave some %s.\n\r",
                                  spice_table[drug]);
                        worsen_mental_state(ch, 1);
                }
                else if (ch->PCData->addiction[drug] >
                         ch->PCData->drug_level[drug] + 25)
                {
                        ch_printf(ch,
                                  "Some more %s would feel quite nice.\n\r",
                                  spice_table[drug]);
                }
                else if (ch->PCData->addiction[drug] <
                         ch->PCData->drug_level[drug] - 50)
                {
                        act(AtPoison, "$n bends over and vomits.\n\r", ch,
                            NULL, NULL, ToRoom);
                        act(AtPoison, "You vomit.\n\r", ch, NULL, NULL,
                            ToChar);
                        ch->PCData->drug_level[drug] -= 10;
                }

                if (ch->PCData->drug_level[drug] > 1)
                        ch->PCData->drug_level[drug] -= 2;
                else if (ch->PCData->drug_level[drug] > 0)
                        ch->PCData->drug_level[drug] -= 1;
                else if (ch->PCData->addiction[drug] > 0
                         && ch->PCData->drug_level[drug] <= 0)
                        ch->PCData->addiction[drug]--;
        }

}

void gain_condition(CharData * ch, int iCond, int value)
{
        int       condition;
        ch_ret    retcode = rNONE;

        if (value == 0 || IsNpc(ch) || get_trust(ch) >= LevelImmortal ||
            NotAuthed(ch) || !str_cmp("droid", ch->race->name())
            || IsSet(ch->PCData->flags, PcflagNohunger))
                return;

        condition = ch->PCData->condition[iCond];
		/* For decreasing checks */
		if (value <= 0 && 
			(iCond == CondFull || iCond == CondThirst) &&
			get_implant_affect(ch,ImplantHunger))
				return;
        ch->PCData->condition[iCond] = URANGE(0, condition + value, 48);

        if (ch->PCData->condition[iCond] == 0)
        {
                switch (iCond)
                {
                case CondFull:
                        if (ch->top_level <= LevelAvatar)
                        {
                                set_char_color(AtHungry, ch);
                                send_to_char("You are STARVING!\n\r", ch);
                                act(AtHungry, "$n is starved half to death!",
                                    ch, NULL, NULL, ToRoom);
                                worsen_mental_state(ch, 1);
                                retcode = damage(ch, ch, 5, TypeUndefined);
                        }
                        break;

                case CondThirst:
                        if (ch->top_level < LevelAvatar)
                        {
                                set_char_color(AtThirsty, ch);
                                send_to_char("You are DYING of THIRST!\n\r",
                                             ch);
                                act(AtThirsty, "$n is dying of thirst!", ch,
                                    NULL, NULL, ToRoom);
                                worsen_mental_state(ch, 2);
                                retcode = damage(ch, ch, 5, TypeUndefined);
                        }
                        break;

                case CondDrunk:
                        if (condition != 0)
                        {
                                set_char_color(AtSober, ch);
                                send_to_char("You are sober.\n\r", ch);
                        }
                        retcode = rNONE;
                        break;
                default:
                        bug("Gain_condition: invalid condition type %d",
                            iCond);
                        retcode = rNONE;
                        break;
                }
        }

        if (retcode != rNONE)
                return;

        if (ch->PCData->condition[iCond] == 1)
        {
                switch (iCond)
                {
                case CondFull:
                        if (ch->top_level < LevelAvatar)
                        {
                                set_char_color(AtHungry, ch);
                                send_to_char("You are really hungry.\n\r",
                                             ch);
                                act(AtHungry,
                                    "You can hear $n's stomach growling.", ch,
                                    NULL, NULL, ToRoom);
                                if (number_bits(1) == 0)
                                        worsen_mental_state(ch, 1);
                        }
                        break;

                case CondThirst:
                        if (ch->top_level < LevelAvatar)
                        {
                                set_char_color(AtThirsty, ch);
                                send_to_char("You are really thirsty.\n\r",
                                             ch);
                                worsen_mental_state(ch, 1);
                                act(AtThirsty, "$n looks a little parched.",
                                    ch, NULL, NULL, ToRoom);
                        }
                        break;

                case CondDrunk:
                        if (condition != 0)
                        {
                                set_char_color(AtSober, ch);
                                send_to_char
                                        ("You are feeling a little less light headed.\n\r",
                                         ch);
                        }
                        break;
                }
        }
        if (ch->PCData->condition[iCond] == 2)
        {
                switch (iCond)
                {
                case CondFull:
                        if (ch->top_level < LevelAvatar)
                        {
                                set_char_color(AtHungry, ch);
                                send_to_char("You are hungry.\n\r", ch);
                        }
                        break;

                case CondThirst:
                        if (ch->top_level < LevelAvatar)
                        {
                                set_char_color(AtThirsty, ch);
                                send_to_char("You are thirsty.\n\r", ch);
                        }
                        break;

                }
        }

        if (ch->PCData->condition[iCond] == 3)
        {
                switch (iCond)
                {
                case CondFull:
                        if (ch->top_level < LevelAvatar)
                        {
                                set_char_color(AtHungry, ch);
                                send_to_char("You are a mite peckish.\n\r",
                                             ch);
                        }
                        break;

                case CondThirst:
                        if (ch->top_level < LevelAvatar)
                        {
                                set_char_color(AtThirsty, ch);
                                send_to_char
                                        ("You could use a sip of something refreshing.\n\r",
                                         ch);
                        }
                        break;

                }
        }
        return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update(void)
{
        char      buf[MaxStringLength];
        CharData *ch;
        ExitData *pexit;
        int       door;
        ch_ret    retcode;

        retcode = rNONE;

        /*
         * Examine all mobs. 
         */
        for (ch = last_char; ch; ch = gch_prev)
        {

                if (ch == first_char && ch->prev)
                {
                        bug("mobile_update: first_char->prev != NULL... fixed", 0);
                        ch->prev = NULL;
                }

                gch_prev = ch->prev;

                if (gch_prev && gch_prev->next != ch)
                {
                        snprintf(buf, MSL,
                                 "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
                                 ch->name);
                        bug(buf, 0);
                        bug("Short-cutting here", 0);
                        gch_prev = NULL;
                        ch->prev = NULL;
                        do_shout(ch, "Thoric says, 'Prepare for the worst!'");
                }

                if (!IsNpc(ch))
                {
                        drunk_randoms(ch);
                        halucinations(ch);
                        continue;
                }

                if (!ch->in_room
                    || IsAffected(ch, AffCharm)
                    || IsAffected(ch, AffParalysis))
                        continue;

/* Clean up 'animated corpses' that are not charmed' - Scryn */

                if (ch->pIndexData->vnum == 5 && !IsAffected(ch, AffCharm))
                {
                        if (ch->in_room->first_person)
                                act(AtMagic,
                                    "$n returns to the dust from whence $e came.",
                                    ch, NULL, NULL, ToRoom);

                        if (IsNpc(ch)) /* Guard against purging switched? */
                                extract_char(ch, TRUE);
                        continue;
                }

                if (!IsSet(ch->act, ActSentinel)
                    && !ch->fighting && ch->hunting)
                {
                        if (ch->top_level < 20)
                                WaitState(ch, 6 * PulsePerSecond);
                        else if (ch->top_level < 40)
                                WaitState(ch, 5 * PulsePerSecond);
                        else if (ch->top_level < 60)
                                WaitState(ch, 4 * PulsePerSecond);
                        else if (ch->top_level < 80)
                                WaitState(ch, 3 * PulsePerSecond);
                        else if (ch->top_level < 100)
                                WaitState(ch, 2 * PulsePerSecond);
                        else
                                WaitState(ch, 1 * PulsePerSecond);
                        hunt_victim(ch);
                        continue;
                }
                else if (!ch->fighting && !ch->hunting
                         && !IsSet(ch->act, ActRunning)
                         && ch->was_sentinel && ch->position >= PosStanding)
                {
                        act(AtAction, "$n leaves.", ch, NULL, NULL, ToRoom);
                        char_from_room(ch);
                        char_to_room(ch, ch->was_sentinel);
                        act(AtAction, "$n arrives.", ch, NULL, NULL,
                            ToRoom);
                        SetBit(ch->act, ActSentinel);
                        ch->was_sentinel = NULL;
                }

                /*
                 * Examine call for special procedure 
                 */
                if (!IsSet(ch->act, ActRunning) && ch->spec_fun)
                {
                        if ((*ch->spec_fun) (ch))
                                continue;
                        if (char_died(ch))
                                continue;
                }

                if (!IsSet(ch->act, ActRunning) && ch->spec_2)
                {
                        if ((*ch->spec_2) (ch))
                                continue;
                        if (char_died(ch))
                                continue;
                }

                /*
                 * Check for mudprogram script on mob 
                 */
                if (IsSet(ch->pIndexData->progtypes, ScriptProg))
                {
                        mprog_script_trigger(ch);
                        continue;
                }
                /*
                 * That's all for sleeping / busy monster 
                 */
                if (ch->position != PosStanding)
                        continue;


                if (IsSet(ch->act, ActMounted))
                {
                        if (IsSet(ch->act, ActAggressive))
                                do_emote(ch, "snarls and growls.");
                        continue;
                }

                if (IsSet(ch->in_room->RoomFlags, RoomSafe)
                    && IsSet(ch->act, ActAggressive))
                        do_emote(ch, "glares around and snarls.");  /* MOBprogram random trigger */
                if (ch->in_room->area->nplayer > 0)
                {
                        mprog_random_trigger(ch);
                        if (char_died(ch))
                                continue;
                        if (ch->position < PosStanding)
                                continue;
                }

                /*
                 * MOBprogram hour trigger: do something for an hour 
                 */
                mprog_hour_trigger(ch);

                if (char_died(ch))
                        continue;

                rprog_hour_trigger(ch);
                if (char_died(ch))
                        continue;

                if (ch->position < PosStanding)
                        continue;

                /*
                 * Scavenge 
                 */
                if (IsSet(ch->act, ActScavenger)
                    && ch->in_room->first_content && number_bits(2) == 0)
                {
                        ObjData *obj;
                        ObjData *obj_best;
                        int       max;

                        max = 1;
                        obj_best = NULL;
                        for (obj = ch->in_room->first_content; obj;
                             obj = obj->next_content)
                        {
                                if (CanWear(obj, ItemTake)
                                    && obj->cost > max
                                    && !IsObjStat(obj, ItemBurried))
                                {
                                        obj_best = obj;
                                        max = obj->cost;
                                }
                        }

                        if (obj_best)
                        {
                                obj_from_room(obj_best);
                                obj_to_char(obj_best, ch);
                                if (auction && auction->item
                                    && auction->item->item_type ==
                                    ItemBeacon)
                                        auction->item->value[0] = 0;
                                act(AtAction, "$n gets $p.", ch, obj_best,
                                    NULL, ToRoom);
                        }
                }

                /*
                 * Wander 
                 */ if (!IsSet(ch->act, ActSentinel)
                        && !IsSet(ch->act, ActPrototype)
                        && !ch->held
                        && (door = number_bits(5)) <= 9
                        && (pexit = get_exit(ch->in_room, door)) != NULL
                        && pexit->to_room
                        && !IsSet(pexit->exit_info, ExClosed)
                        && !IsSet(pexit->to_room->RoomFlags, RoomNoMob)
                        && (!IsSet(ch->act, ActStayArea)
                            || pexit->to_room->area == ch->in_room->area))
                {
                        retcode = move_char(ch, pexit, 0, FALSE);
                        /*
                         * If ch changes position due
                         * to it's or someother mob's
                         * movement via MOBProgs,
                         * continue - Kahn 
                         */
                        if (char_died(ch))
                                continue;
                        if (retcode != rNONE || IsSet(ch->act, ActSentinel)
                            || ch->position < PosStanding)
                                continue;
                }

                /*
                 * Flee 
                 */
                if (ch->hit < ch->max_hit / 2
                    && !ch->held
                    && (door = number_bits(4)) <= 9
                    && (pexit = get_exit(ch->in_room, door)) != NULL
                    && pexit->to_room
                    && !IsSet(pexit->exit_info, ExClosed)
                    && !IsSet(pexit->to_room->RoomFlags, RoomNoMob))
                {
                        CharData *rch;
                        bool      found;

                        found = FALSE;
                        for (rch = ch->in_room->first_person;
                             rch; rch = rch->next_in_room)
                        {
                                if (is_fearing(ch, rch))
                                {
                                        switch (number_bits(2))
                                        {
                                        case 0:
                                                snprintf(buf, MSL,
                                                         "yell Get away from me, %s!",
                                                         rch->name);
                                                break;
                                        case 1:
                                                snprintf(buf, MSL,
                                                         "yell Leave me be, %s!",
                                                         rch->name);
                                                break;
                                        case 2:
                                                snprintf(buf, MSL,
                                                         "yell %s is trying to kill me!  Help!",
                                                         rch->name);
                                                break;
                                        case 3:
                                                snprintf(buf, MSL,
                                                         "yell Someone save me from %s!",
                                                         rch->name);
                                                break;
                                        }
                                        interpret(ch, buf);
                                        found = TRUE;
                                        break;
                                }
                        }
                        if (found)
                                retcode = move_char(ch, pexit, 0, FALSE);
                }
        }

        return;
}

void update_salaries(void)
{
        CharData *ch;
        ClanData *clan;
        DescriptorData *d;
        int       amount;

        for (d = first_descriptor; d; d = d->next)
        {
                if (d->connected != ConPlaying)
                        continue;
                ch = d->character;
                /*
                 * Not as likely this way, but can be switched 
                 */
                if (IsNpc(ch))
                        continue;   /* Not likely, but just in case */
                if (ch->PCData && ch->PCData->clan)
                {
                        clan = ch->PCData->clan;
                        amount = clan->salary[ch->PCData->clanrank];
                        if (amount <= 0)
                                continue;
                        ch->gold += amount;
                        ch->PCData->clan->funds -= amount;
                        act(AtAction,
                            "Someone quickly appears and hands $n a package.",
                            ch, NULL, NULL, ToRoom);
                        act(AtAction,
                            "A clan messenger appears and hands you a salary package.",
                            ch, NULL, NULL, ToChar);
                }
        }
}

void update_taxes(void)
{
        PlanetData *planet;
        int       i, iv, ev;
        ClanData *clan;
        char      buf[MaxStringLength];

        for (planet = first_planet; planet; planet = planet->next)
        {
                for (i = 1; i < CargoMax; i++)
                {
                        /*
                         * Set resources value (iv)
                         */
                        iv = 0;
                        ev = 0;
                        if (planet->resource[i] > 1
                            && planet->resource[i] < 10)
                        {
                                iv = 10;
                                ev = 1;
                        }
                        if (planet->resource[i] > 10
                            && planet->resource[i] < 100)
                        {
                                iv = 9;
                                ev = 2;
                        }
                        if (planet->resource[i] > 100
                            && planet->resource[i] < 1000)
                        {
                                iv = 8;
                                ev = 3;
                        }
                        if (planet->resource[i] > 1000
                            && planet->resource[i] < 10000)
                        {
                                iv = 7;
                                ev = 4;
                        }
                        if (planet->resource[i] > 10000
                            && planet->resource[i] < 50000)
                        {
                                iv = 6;
                                ev = 5;
                        }
                        if (planet->resource[i] > 50000
                            && planet->resource[i] < 100000)
                        {
                                iv = 5;
                                ev = 6;
                        }
                        if (planet->resource[i] > 100000
                            && planet->resource[i] < 500000)
                        {
                                iv = 4;
                                ev = 7;
                        }
                        if (planet->resource[i] > 500000
                            && planet->resource[i] < 1000000)
                        {
                                iv = 3;
                                ev = 8;
                        }
                        if (planet->resource[i] > 1000000)
                        {
                                iv = 2;
                                ev = 9;
                        }
                        if (iv > ev)
                        {
                                /*
                                 * Adjust import and export costs
                                 */
                                planet->cargoimport[i] =
                                        ((10 * i) * (iv)) +
                                        (int) (number_percent() * (0.1));
                                planet->cargoexport[i] = 0;
                                /*
                                 * Adjust production and consumtion
                                 */
                                planet->produces[i] =
                                        (int) (1.6 * number_percent() *
                                               (iv - ev)) +
                                        (number_percent());
                                planet->consumes[i] =
                                        (number_percent() * (iv - ev)) +
                                        (number_percent());
                        }
                        else    /* if ( iv < ev ) */
                        {
                                /*
                                 * Adjust import and export costs
                                 */
                                planet->cargoexport[i] =
                                        ((10 * i) * (iv)) +
                                        (int) (number_percent() * (0.1));
                                planet->cargoimport[i] = 0;
                                /*
                                 * Adjust production and consumtion
                                 */
                                planet->produces[i] =
                                        (number_percent() * (ev - iv)) +
                                        (number_percent());
                                planet->consumes[i] =
                                        (int) (1.6 * number_percent() *
                                               (ev - iv)) +
                                        (number_percent());
                        }
                        /*
                         * Implement production and consumtion values 
                         */

                        planet->resource[i] += planet->produces[i];

                        planet->resource[i] -= planet->consumes[i];
                        /*
                         * Broadcast urgent import messages 
                         */
                        if (planet->resource[i] < 10000
                            && planet->resource[i] > 0)
                        {
                                snprintf(buf, MSL,
                                         "Trade Alert: %s is in need of imports!",
                                         planet->name);
                                echo_to_all(AtGold, buf, 0);
                        }
                        /*
                         * Reset negative values to 1 
                         */
                        /*
                         * and do emergency import stuff 
                         */
                        if (planet->resource[i] < 0)
                        {
                                planet->resource[i] = 1;
                                planet->cargoimport[i] =
                                        ((100 * i) +
                                         (int) (number_percent() * (0.1)));
                                planet->cargoexport[i] = 0;
                                snprintf(buf, MSL,
                                         "Trade Alert: %s is in need of emergency imports!",
                                         planet->name);
                                echo_to_all(AtGold, buf, 0);
                        }
                        /*
                         * Save the planet file 
                         */
                        save_planet(planet, FALSE);
                }
                clan = planet->governed_by;
                if (clan)
                {
                        clan->funds += get_taxes(planet) / 360;
                        save_clan(clan);
                        save_planet(planet, FALSE);
                }
        }
}

/*
 * Update the weather.
 */
void weather_update(void)
{
        char      buf[MaxStringLength];
        DescriptorData *d;
        int       diff;
        sh_int    AtTemp = AtPlain;

        buf[0] = '\0';

        switch (++time_info.hour)
        {
        case 5:
                weather_info.sunlight = SunLight;
                mudstrlcat(buf, "The day has begun.", MSL);
                AtTemp = AtYellow;
                break;

        case 6:
                weather_info.sunlight = SunRise;
                mudstrlcat(buf, "The sun rises in the east.", MSL);
                AtTemp = AtOrange;
                break;

        case 12:
                weather_info.sunlight = SunLight;
                mudstrlcat(buf, "It's noon.", MSL);
                AtTemp = AtYellow;
                break;

        case 19:
                weather_info.sunlight = SunSet;
                mudstrlcat(buf, "The sun slowly disappears in the west.",
                           MSL);
                AtTemp = AtBlood;
                break;

        case 20:
                weather_info.sunlight = SunDark;
                mudstrlcat(buf, "The night has begun.", MSL);
                AtTemp = AtDgrey;
                break;

        case 24:
                time_info.hour = 0;
                time_info.day++;
                break;
        }

        if (time_info.day >= 29)
        {
                time_info.day = 0;
                time_info.month++;
        }

        if (time_info.month >= 17)
        {
                time_info.month = 0;
                time_info.year++;
        }

        if (buf[0] != '\0')
        {
                for (d = first_descriptor; d; d = d->next)
                {
                        if (d->connected == ConPlaying
                            && IsOutside(d->character)
                            && IsAwake(d->character)
                            && d->character->in_room
                            && d->character->in_room->sector_type !=
                            SectUnderwater
                            && d->character->in_room->sector_type !=
                            SectOceanfloor
                            && d->character->in_room->sector_type !=
                            SectUnderground)
                                act(AtTemp, buf, d->character, 0, 0,
                                    ToChar);
                }
                buf[0] = '\0';
        }
        /*
         * Weather change.
         */
        if (time_info.month >= 9 && time_info.month <= 16)
                diff = weather_info.mmhg > 985 ? -2 : 2;
        else
                diff = weather_info.mmhg > 1015 ? -2 : 2;

        weather_info.change += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
        weather_info.change = UMAX(weather_info.change, -12);
        weather_info.change = UMIN(weather_info.change, 12);

        weather_info.mmhg += weather_info.change;
        weather_info.mmhg = UMAX(weather_info.mmhg, 960);
        weather_info.mmhg = UMIN(weather_info.mmhg, 1040);

        AtTemp = AtGrey;
        switch (weather_info.sky)
        {
        default:
                bug("Weather_update: bad sky %d.", weather_info.sky);
                weather_info.sky = SkyCloudless;
                break;

        case SkyCloudless:
                if (weather_info.mmhg < 990
                    || (weather_info.mmhg < 1010 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "The sky is getting cloudy.", MSL);
                        weather_info.sky = SkyCloudy;
                        AtTemp = AtGrey;
                }
                break;

        case SkyCloudy:
                if (weather_info.mmhg < 970
                    || (weather_info.mmhg < 990 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "It starts to rain.", MSL);
                        weather_info.sky = SkyRaining;
                        AtTemp = AtBlue;
                }

                if (weather_info.mmhg > 1030 && number_bits(2) == 0)
                {
                        mudstrlcat(buf, "The clouds disappear.", MSL);
                        weather_info.sky = SkyCloudless;
                        AtTemp = AtWhite;
                }
                break;

        case SkyRaining:
                if (weather_info.mmhg < 970 && number_bits(2) == 0)
                {
                        mudstrlcat(buf, "Lightning flashes in the sky.", MSL);
                        weather_info.sky = SkyLightning;
                        AtTemp = AtYellow;
                }

                if (weather_info.mmhg > 1030
                    || (weather_info.mmhg > 1010 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "The rain stopped.", MSL);
                        weather_info.sky = SkyCloudy;
                        AtTemp = AtWhite;
                }
                break;

        case SkyLightning:
                if (weather_info.mmhg > 1010
                    || (weather_info.mmhg > 990 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "The lightning has stopped.", MSL);
                        weather_info.sky = SkyRaining;
                        AtTemp = AtGrey;
                        break;
                }
                break;
        }

        if (buf[0] != '\0')
        {
                for (d = first_descriptor; d; d = d->next)
                {
                        if (d->connected == ConPlaying
                            && IsOutside(d->character)
                            && IsAwake(d->character))
                                act(AtTemp, buf, d->character, 0, 0,
                                    ToChar);
                }
        }

        return;
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update(void)
{
        CharData *ch;

        char      buf[MSL];

        CharData *ch_save;
        int       value;
        sh_int    save_count = 0;
        ObjData *binding = NULL;

        ch_save = NULL;
        for (ch = last_char; ch; ch = gch_prev)
        {
                if (ch == first_char && ch->prev)
                {
                        bug("char_update: first_char->prev != NULL... fixed",
                            0);
                        ch->prev = NULL;
                }
                gch_prev = ch->prev;
                if (gch_prev && gch_prev->next != ch)
                {
                        bug("char_update: ch->prev->next != ch", 0);
                        return;
                }
                /*
                 * erflink Erflink ERFLINK did you add the player fields for it? If so, I dunno where they are, gotta look.
                 */
                if (ch->PCData)
                {
/*
     value = number_range( 1,10000 );
     if ( ( value >= 1 ) && ( value <= 3 ) && ( ch->PCData->illness < 15 ) )
        ch->PCData->illness = 15;
     if ( ( value >= 4 ) && ( value <= 7 ) && ( ch->PCData->illness < 14 ) )
        ch->PCData->illness = 14;
     if ( ( value >= 8 ) && ( value <= 20 ) && ( ch->PCData->illness < 13 ) )
        ch->PCData->illness = 13;
     if ( ( value >= 21 ) && ( value <= 31 ) && ( ch->PCData->illness < 12 ) )
        ch->PCData->illness = 12;
     if ( ( value >= 32 ) && ( value <= 42 ) && ( ch->PCData->illness < 11 ) )
        ch->PCData->illness = 11;
     if ( ( value >= 43 ) && ( value <= 53 ) && ( ch->PCData->illness < 10 ) )
        ch->PCData->illness = 10;
     if ( ( value >= 54 ) && ( value <= 64 ) && ( ch->PCData->illness < 9 ) )
        ch->PCData->illness = 9;
     if ( ( value >= 65 ) && ( value <= 75 ) && ( ch->PCData->illness < 8 ) )
        ch->PCData->illness = 8;
     if ( ( value >= 76 ) && ( value <= 80 ) && ( ch->PCData->illness < 7 ) )
        ch->PCData->illness = 7;
     if ( ( value >= 81 ) && ( value <= 99 ) && ( ch->PCData->illness < 6 ) )
        ch->PCData->illness = 6;
     if ( ( value >= 100 ) && ( value <= 119 ) && ( ch->PCData->illness < 5 ) )
        ch->PCData->illness = 5;
     if ( ( value >= 120 ) && ( value <= 149 ) && ( ch->PCData->illness < 4 ) )
        ch->PCData->illness = 4;
     if ( ( value >= 150 ) && ( value <= 189 ) && ( ch->PCData->illness < 3 ) )
        ch->PCData->illness = 3;
     if ( ( value >= 190 ) && ( value <= 239 ) && ( ch->PCData->illness < 2 ) )
        ch->PCData->illness = 2;
     if ( ( value >= 240 ) && ( value <= 300 ) && ( ch->PCData->illness < 1 ) )
        ch->PCData->illness = 1;
*/
                        if (ch->PCData->illness > 0)
                        {
                                switch (ch->PCData->illness)
                                {
                                case 1:
                                        ch->endurance -= 140;
                                        send_to_char
                                                ("&P&WYour nose is runny\n\r",
                                                 ch);
                                        break;

                                case 2:
                                        ch->endurance -= 170;
                                        send_to_char
                                                ("&P&WYou have bad indigestion\n\r",
                                                 ch);
                                        interpret(ch, "burp");
                                        break;

                                case 3:
                                        ch->endurance -= 200;
                                        send_to_char
                                                ("&P&WYour throat is kind of scratchy.\n\r",
                                                 ch);
                                        interpret(ch, "cough");
                                        break;

                                case 4:
                                        ch->endurance -= 200;
                                        send_to_char
                                                ("&P&WYour sinuses are quite stuffy.\n\r",
                                                 ch);
                                        interpret(ch, "sneeze");
                                        break;

                                case 5:
                                        ch->endurance -= 300;
                                        ch->hit -= 10;
                                        send_to_char
                                                ("&P&WYou feel very nauseaus\n\r",
                                                 ch);
                                        if (number_percent() >= 25)
                                        {
                                                interpret(ch, "puke");
                                                ch->hit -= 30;
                                                gain_condition(ch, CondFull,
                                                               -2);
                                        }
                                        if (ch->position == PosResting)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 5;
                                        }
                                        if (ch->position == PosSleeping)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 10;
                                        }
                                        break;

                                case 6:
                                        ch->endurance -= 350;
                                        ch->hit -= 20;
                                        send_to_char
                                                ("&P&WYour throat burns like fire.\n\r",
                                                 ch);
                                        interpret(ch, "cough");
                                        gain_condition(ch, CondThirst, -3);
                                        if (ch->position == PosResting)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 5;
                                        }
                                        if (ch->position == PosSleeping)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 10;
                                        }
                                        break;

                                case 7:
                                        ch->endurance -= 400;
                                        ch->hit -= 40;
                                        send_to_char
                                                ("&P&WYour head aches and your sinuses feel ready to explode.\n\r",
                                                 ch);
                                        send_to_char
                                                ("&P&WYour stomach growls and your nausea is barely controlable.\n\r",
                                                 ch);
                                        if (number_percent() >= 50)
                                        {
                                                interpret(ch, "puke");
                                                ch->hit -= 10;
                                                gain_condition(ch, CondFull,
                                                               -1);
                                        }
                                        gain_condition(ch, CondFull, -1);
                                        if (ch->position == PosResting)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 10;
                                        }
                                        if (ch->position == PosSleeping)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 20;
                                        }
                                        break;

                                case 8:
                                        ch->endurance -= 200;
                                        ch->hit -= 5;
                                        send_to_char
                                                ("&P&WThere must be something in your eye, it burns.\n\r",
                                                 ch);
                                        break;

                                case 9:
                                        ch->endurance -= 200;
                                        ch->hit -= 5;
                                        send_to_char
                                                ("&P&WYour ears feel funny, and you find it hard to balance.\n\r",
                                                 ch);
                                        break;

                                case 10:
                                        ch->endurance -= 200;
                                        ch->hit -= 5;
                                        send_to_char
                                                ("&P&WYour throat feels like it is full of phlem.\n\r",
                                                 ch);
                                        break;

                                case 11:
                                        ch->endurance -= 400;
                                        ch->hit -= 90;
                                        send_to_char
                                                ("&P&WYour whole body feels week, as if you had been beaten.\n\r",
                                                 ch);
                                        gain_condition(ch, CondFull, -2);
                                        gain_condition(ch, CondThirst, -2);
                                        worsen_mental_state(ch, 1);
                                        if (ch->position == PosResting)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 20;
                                        }
                                        if (ch->position == PosSleeping)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 40;
                                        }
                                        if ((number_percent() <= 15)
                                            && (ch->position != PosSleeping))
                                        {
                                                interpret(ch, "faint");
                                                ch->hit -= 10;
                                                ch->position = PosSleeping;
                                        }
                                        break;

                                case 12:
                                        ch->endurance -= 200;
                                        ch->hit -= 40;
                                        send_to_char
                                                ("&P&WYou feel a sharp pain.\n\r",
                                                 ch);
                                        worsen_mental_state(ch, 1);
                                        value = (number_percent());
                                        if (value <= 15)
                                        {
                                        }
                                        if ((value >= 16) && (value <= 30))
                                        {
                                        }
                                        if ((value >= 31) && (value <= 45))
                                        {
                                        }
                                        if ((value >= 46) && (value <= 60))
                                        {
                                        }
                                        if ((value >= 61) && (value <= 75))
                                        {
                                        }
                                        if ((value >= 76) && (value <= 100))
                                        {
                                        }
                                        break;

                                case 13:
                                        ch->endurance -= 200;
                                        send_to_char
                                                ("&P&WYou don't feel like doing much.\n\r",
                                                 ch);
                                        worsen_mental_state(ch, 2);
                                        if (ch->position == PosSleeping)
                                        {
                                                better_mental_state(ch, 2);
                                        }
                                        if (ch->position == PosResting)
                                        {
                                                better_mental_state(ch, 1);
                                        }
                                        break;

                                case 14:
                                        ch->endurance -= 400;
                                        ch->hit -= 100;
                                        send_to_char
                                                ("&P&WYour whole body feels tired.\n\r",
                                                 ch);
                                        send_to_char
                                                ("&P&WPain jolts through your body for no aparent reason.\n\r",
                                                 ch);
                                        worsen_mental_state(ch, 2);
                                        gain_condition(ch, CondFull, -2);
                                        gain_condition(ch, CondThirst, -2);
                                        if (ch->position == PosSleeping)
                                        {
                                                better_mental_state(ch, 2);
                                                gain_condition(ch, CondFull,
                                                               +2);
                                                gain_condition(ch,
                                                               CondThirst,
                                                               +2);
                                                ch->hit += 30;
                                                ch->endurance += 100;
                                        }
                                        if (ch->position == PosResting)
                                        {
                                                gain_condition(ch, CondFull,
                                                               +1);
                                                gain_condition(ch,
                                                               CondThirst,
                                                               +1);
                                                better_mental_state(ch, 1);
                                                ch->hit += 15;
                                                ch->endurance += 50;
                                        }
                                        break;

                                case 15:
                                        ch->endurance -= 400;
                                        ch->hit -= 120;
                                        worsen_mental_state(ch, 2);
                                        gain_condition(ch, CondFull, -2);
                                        gain_condition(ch, CondThirst, -2);
                                        send_to_char
                                                ("&P&WYou feel as if your body is burning up.\n\r",
                                                 ch);
                                        send_to_char
                                                ("&P&WThe pain is excrutiating.\n\r",
                                                 ch);
                                        if (ch->position == PosSleeping)
                                        {
                                                better_mental_state(ch, 2);
                                                gain_condition(ch, CondFull,
                                                               +2);
                                                gain_condition(ch,
                                                               CondThirst,
                                                               +2);
                                                ch->hit += 30;
                                                ch->endurance += 100;
                                        }
                                        if (ch->position == PosResting)
                                        {
                                                gain_condition(ch, CondFull,
                                                               +1);
                                                gain_condition(ch,
                                                               CondThirst,
                                                               +1);
                                                better_mental_state(ch, 1);
                                                ch->hit += 15;
                                                ch->endurance += 50;
                                        }
                                        break;

                                default:
                                        break;
                                }
                        }
                }
                /*
                 * if (!IsNpc(ch))
                 * fix_char(ch);
                 */

                /*
                 *  Do a room_prog rand check right off the bat
                 *   if ch disappears (rprog might wax npc's), continue
                 */
                if (!IsNpc(ch))
                        rprog_random_trigger(ch);

                if (char_died(ch))
                        continue;

                if (IsNpc(ch))
                        mprog_time_trigger(ch);

                if (char_died(ch))
                        continue;

                rprog_time_trigger(ch);

                if (char_died(ch))
                        continue;
                /*
                 * Check to ensure you don't magically free yourself from bindings. -Ulthrax 
                 */

                if ((binding = get_eq_char(ch, WearBinding)) != NULL
                    && !ch->held)
                {
                        ch->held = TRUE;
                        bug("%s is bound, but not held. Re-applying held state.\n\r", ch->name);
                }
                /*
                 * See if player should be auto-saved.
                 */
                if (!IsNpc(ch)
                    && !NotAuthed(ch)
                    && current_time - ch->save_time >
                    (sysdata.save_frequency * 60))
                        ch_save = ch;
                else
                        ch_save = NULL;

                if (ch->position >= PosStunned)
                {
                        if (ch->hit < ch->max_hit)
                                ch->hit += hit_gain(ch);

                        /*
                         * Psst, wtf is this checking for Force ability to be.. 1? that makes no sense... should ignore that - gavin 
                         */
                        if (ch->endurance < ch->max_endurance
                            /*
                             * || ch->skill_level[ForceAbility] == 1 
                             */ )
                                ch->endurance += mana_gain(ch);

                        if (ch->endurance < ch->max_endurance)
                                ch->endurance += move_gain(ch);
                }

                if (ch->position == PosStunned)
                        update_pos(ch);

                if (ch->PCData)
                        gain_addiction(ch);


                if (!IsNpc(ch) && ch->top_level < LevelImmortal)
                {
                        ObjData *obj;

                        if ((obj = get_eq_char(ch, WearLight)) != NULL
                            && obj->item_type == ItemLight
                            && obj->value[2] > 0)
                        {
                                if (--obj->value[2] == 0 && ch->in_room)
                                {
                                        ch->in_room->light -= obj->count;
                                        act(AtAction, "$p goes out.", ch,
                                            obj, NULL, ToRoom);
                                        act(AtAction, "$p goes out.", ch,
                                            obj, NULL, ToChar);

                                        extract_obj(obj);
                                }
                        }

                        if (str_cmp("droid", ch->race->name()))
                        {
                                if (ch->PCData->condition[CondDrunk] > 8)
                                        worsen_mental_state(ch,
                                                            ch->PCData->
                                                            condition
                                                            [CondDrunk] / 8);
                                if (ch->PCData->condition[CondFull] > 1)
                                {
                                        switch (ch->position)
                                        {
                                        case PosSleeping:
                                                better_mental_state(ch, 4);
                                                break;
                                        case PosResting:
                                                better_mental_state(ch, 3);
                                                break;
                                        case PosSitting:
                                        case PosMounted:
                                                better_mental_state(ch, 2);
                                                break;
                                        case PosStanding:
                                                better_mental_state(ch, 1);
                                                break;
                                        case PosFighting:
                                                if (number_bits(2) == 0)
                                                        better_mental_state
                                                                (ch, 1);
                                                break;
                                        }
                                }
                                if (ch->PCData->condition[CondThirst] > 1)
                                {
                                        switch (ch->position)
                                        {
                                        case PosSleeping:
                                                better_mental_state(ch, 5);
                                                break;
                                        case PosResting:
                                                better_mental_state(ch, 3);
                                                break;
                                        case PosSitting:
                                        case PosMounted:
                                                better_mental_state(ch, 2);
                                                break;
                                        case PosStanding:
                                                better_mental_state(ch, 1);
                                                break;
                                        case PosFighting:
                                                if (number_bits(2) == 0)
                                                        better_mental_state
                                                                (ch, 1);
                                                break;
                                        }
                                }
                                gain_condition(ch, CondDrunk, -1);
                                /*
                                 * OLC Races - some races will be better tan others now :D 
                                 */
                                gain_condition(ch, CondFull,-1 + (ch->race ? ch->race->hunger_mod() : 0));

                                if (ch->in_room) {
                                        switch (ch->in_room->sector_type)
                                        {
                                        default:
                                                gain_condition(ch,CondThirst,-1 +(ch->race ? ch->race->thirst_mod() : 0));
                                                break;
                                        case SectDesert:
                                                gain_condition(ch,CondThirst,-2 +(ch->race ? ch->race->thirst_mod() : 0));
                                                break;
                                        case SectUnderwater:
                                        case SectOceanfloor:
                                                if (number_bits(1) == 0)
                                                        gain_condition(ch,CondThirst,-1 +(ch->race ?ch->race->thirst_mod() : 0));
                                                break;
                                         }  
								}

                        }
                }

                if (!char_died(ch))
                {
                        /*
                         * Careful with the damages here,
                         *   MUST NOT refer to ch after damage taken,
                         *   as it may be lethal damage (on NPC).
                         */
                        if (IsAffected(ch, AffPoison))
                        {
                                act(AtPoison, "$n shivers and suffers.", ch,
                                    NULL, NULL, ToRoom);
                                act(AtPoison, "You shiver and suffer.", ch,
                                    NULL, NULL, ToChar);
                                ch->mental_state =
                                        URANGE(20, ch->mental_state + 4, 100);
                                damage(ch, ch, 6, gsn_poison);
                        }
                        else if (ch->position == PosIncap)
                                damage(ch, ch, 1, TypeUndefined);
                        else if (ch->position == PosMortal)
                                damage(ch, ch, 4, TypeUndefined);
                        if (char_died(ch))
                                continue;
                        if (ch->mental_state >= 30)
                                switch ((ch->mental_state + 5) / 10)
                                {
                                case 3:
                                        send_to_char("You feel feverish.\n\r",
                                                     ch);
                                        act(AtAction,
                                            "$n looks kind of out of it.", ch,
                                            NULL, NULL, ToRoom);
                                        break;
                                case 4:
                                        send_to_char
                                                ("You do not feel well at all.\n\r",
                                                 ch);
                                        act(AtAction,
                                            "$n doesn't look too good.", ch,
                                            NULL, NULL, ToRoom);
                                        break;
                                case 5:
                                        send_to_char("You need help!\n\r",
                                                     ch);
                                        act(AtAction,
                                            "$n looks like $e could use your help.",
                                            ch, NULL, NULL, ToRoom);
                                        break;
                                case 6:
                                        send_to_char
                                                ("Seekest thou a cleric.\n\r",
                                                 ch);
                                        act(AtAction,
                                            "Someone should fetch a healer for $n.",
                                            ch, NULL, NULL, ToRoom);
                                        break;
                                case 7:
                                        send_to_char
                                                ("You feel reality slipping away...\n\r",
                                                 ch);
                                        act(AtAction,
                                            "$n doesn't appear to be aware of what's going on.",
                                            ch, NULL, NULL, ToRoom);
                                        break;
                                case 8:
                                        send_to_char
                                                ("You begin to understand... everything.\n\r",
                                                 ch);
                                        act(AtAction,
                                            "$n starts ranting like a madman!",
                                            ch, NULL, NULL, ToRoom);
                                        break;
                                case 9:
                                        send_to_char
                                                ("You are ONE with the universe.\n\r",
                                                 ch);
                                        act(AtAction,
                                            "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...",
                                            ch, NULL, NULL, ToRoom);
                                        break;
                                case 10:
                                        send_to_char
                                                ("You feel the end is near.\n\r",
                                                 ch);
                                        act(AtAction,
                                            "$n is muttering and ranting in tongues...",
                                            ch, NULL, NULL, ToRoom);
                                        break;
                                }
                        if (ch->mental_state <= -30)
                                switch ((abs(ch->mental_state) + 5) / 10)
                                {
                                case 10:
                                        if (ch->position > PosSleeping)
                                        {
                                                if ((ch->position ==
                                                     PosStanding
                                                     || ch->position <
                                                     PosFighting)
                                                    && number_percent() + 10 <
                                                    abs(ch->mental_state))
                                                        do_sleep(ch, "");
                                                else
                                                        send_to_char
                                                                ("You're barely conscious.\n\r",
                                                                 ch);
                                        }
                                        break;
                                case 9:
                                        if (ch->position > PosSleeping)
                                        {
                                                if ((ch->position ==
                                                     PosStanding
                                                     || ch->position <
                                                     PosFighting)
                                                    && (number_percent() +
                                                        20) <
                                                    abs(ch->mental_state))
                                                        do_sleep(ch, "");
                                                else
                                                        send_to_char
                                                                ("You can barely keep your eyes open.\n\r",
                                                                 ch);
                                        }
                                        break;
                                case 8:
                                        if (ch->position > PosSleeping)
                                        {
                                                if (ch->position < PosSitting
                                                    && (number_percent() +
                                                        30) <
                                                    abs(ch->mental_state))
                                                        do_sleep(ch, "");
                                                else
                                                        send_to_char
                                                                ("You're extremely drowsy.\n\r",
                                                                 ch);
                                        }
                                        break;
                                case 7:
                                        if (ch->position > PosResting)
                                                send_to_char
                                                        ("You feel very unmotivated.\n\r",
                                                         ch);
                                        break;
                                case 6:
                                        if (ch->position > PosResting)
                                                send_to_char
                                                        ("You feel sedated.\n\r",
                                                         ch);
                                        break;
                                case 5:
                                        if (ch->position > PosResting)
                                                send_to_char
                                                        ("You feel sleepy.\n\r",
                                                         ch);
                                        break;
                                case 4:
                                        if (ch->position > PosResting)
                                                send_to_char
                                                        ("You feel tired.\n\r",
                                                         ch);
                                        break;
                                case 3:
                                        if (ch->position > PosResting)
                                                send_to_char
                                                        ("You could use a rest.\n\r",
                                                         ch);
                                        break;
                                }

                        if (ch->backup_wait > 0)
                        {
                                --ch->backup_wait;
                                /*
                                 * This is why you want to make sure to use brackets 
                                 */
                                if (IsImmortal(ch))
                                {
                                        snprintf(buf, MSL,
                                                 "time left for reins: %d",
                                                 ch->backup_wait);
                                        send_to_char(buf, ch);
                                }
                                if (ch->backup_wait == 0)
                                        add_reinforcements(ch);
                        }

                        if (!IsNpc(ch))
                        {
                                if (++ch->timer > 15 && !ch->desc)
                                {
                                        ExtBV    RoomFlags;
                                        RoomIndexData *room = ch->in_room;

                                        if (NotAuthed(ch)
                                            && ch->PCData->Account)
                                        {
                                                if (!del_from_account(ch->PCData->Account, ch))
                                                {
                                                        bug("Failed to remove unauthed character from Account", 0);
                                                }
                                        }
                                        if (room)
                                        {
                                                RoomFlags = room->RoomFlags;
                                                /*
                                                 * char_from_room(ch); 
                                                 */
                                        }
                                        else
                                        {
                                                char_to_room(ch,
                                                             get_room_index
                                                             (RoomPluogusQuit));
                                        }
                                        if (!IsImmortal(ch))
                                                ch->position = PosResting;
                                        ch->hit = UMAX(1, ch->hit);
                                        save_char_obj(ch);
                                        if (room)
                                                SetBit(room->RoomFlags,
                                                         RoomHotel);
                                        do_quit(ch, "");
                                        if (room)
                                                room->RoomFlags = RoomFlags;
                                }
                                else if (ch == ch_save && IsSet(sysdata.save_flags, SvAuto) && ++save_count < 10) /* save max of 10 per tick */
                                        save_char_obj(ch);
                        }
                }

        }

        return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update(void)
{
        ObjData *obj;
        sh_int    AtTemp = 0;

        for (obj = last_object; obj; obj = gobj_prev)
        {
                CharData *rch;
                char     *message;

                if (obj == first_object && obj->prev)
                {
                        bug("obj_update: first_object->prev != NULL... fixed",
                            0);
                        obj->prev = NULL;
                }
                gobj_prev = obj->prev;
                if (gobj_prev && gobj_prev->next != obj)
                {
                        bug("obj_update: obj->prev->next != obj", 0);
                        return;
                }
                set_cur_obj(obj);
                if (obj->carried_by)
                        oprog_random_trigger(obj);
                else if (obj->in_room && obj->in_room->area->nplayer > 0)
                        oprog_random_trigger(obj);

                if (obj_extracted(obj))
                        continue;

                if (obj->item_type == ItemWeapon && obj->carried_by &&
                    (obj->wear_loc == WearWield
                     || obj->wear_loc == WearDualWield)
                    && obj->value[3] != WeaponBlaster && obj->value[4] > 0
                    && obj->value[3] != WeaponBowcaster
                    && obj->value[3] != WeaponForcePike)
                {
                        obj->value[4]--;
                        if (obj->value[4] <= 0)
                        {
                                if (obj->value[3] == WeaponLightsaber)
                                {
                                        act(AtPlain, "$p fizzles and dies.",
                                            obj->carried_by, obj, NULL,
                                            ToChar);
                                        act(AtPlain,
                                            "$n's lightsaber fizzles and dies.",
                                            obj->carried_by, NULL, NULL,
                                            ToRoom);
                                }
                                else if (obj->value[3] == WeaponVibroBlade)
                                {
                                        act(AtPlain, "$p stops vibrating.",
                                            obj->carried_by, obj, NULL,
                                            ToChar);
                                }
                        }
                }

                if (obj->item_type == ItemPipe)
                {
                        if (IsSet(obj->value[3], PipeLit))
                        {
                                if (--obj->value[1] <= 0)
                                {
                                        obj->value[1] = 0;
                                        RemoveBit(obj->value[3], PipeLit);
                                }
                                else if (IsSet(obj->value[3], PipeHot))
                                        RemoveBit(obj->value[3], PipeHot);
                                else
                                {
                                        if (IsSet
                                            (obj->value[3], PipeGoingout))
                                        {
                                                RemoveBit(obj->value[3],
                                                           PipeLit);
                                                RemoveBit(obj->value[3],
                                                           PipeGoingout);
                                        }
                                        else
                                                SetBit(obj->value[3],
                                                        PipeGoingout);
                                }
                                if (!IsSet(obj->value[3], PipeLit))
                                        SetBit(obj->value[3],
                                                PipeFullofash);
                        }
                        else
                                RemoveBit(obj->value[3], PipeHot);
                }   /* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

                if (obj->item_type == ItemCorpsePc
                    || obj->item_type == ItemCorpseNpc
                    || obj->item_type == ItemDroidCorpse)
                {
                        sh_int    timerfrac = UMAX(1, obj->timer - 1);

                        if (obj->item_type == ItemCorpsePc)
                                timerfrac = (int) (obj->timer / 8 + 1);

                        if (obj->timer > 0 && obj->value[2] > timerfrac)
                        {
                                char      buf[MaxStringLength];
                                char      name[MaxStringLength];
                                char     *bufptr;

                                bufptr = one_argument(obj->short_descr, name);
                                bufptr = one_argument(bufptr, name);
                                bufptr = one_argument(bufptr, name);

                                separate_obj(obj);
                                obj->value[2] = timerfrac;
                                if (obj->item_type == ItemDroidCorpse)
                                        snprintf(buf, MSL,
                                                 d_corpse_descs[UMIN
                                                                (timerfrac -
                                                                 1, 4)],
                                                 bufptr);
                                else
                                        snprintf(buf, MSL,
                                                 corpse_descs[UMIN
                                                              (timerfrac - 1,
                                                               4)],
                                                 capitalize(bufptr));

                                STRFREE(obj->description);
                                obj->description = STRALLOC(buf);
                        }
                }

                /*
                 * don't let inventory decay 
                 */
                if (IsObjStat(obj, ItemInventory))
                        continue;

                if (obj->timer > 0 && obj->timer < 5
                    && obj->item_type == ItemArmor)
                {
                        if (obj->carried_by)
                        {
                                act(AtTemp, "$p is almost dead.",
                                    obj->carried_by, obj, NULL, ToChar);
                        }
                }

                if ((obj->timer <= 0 || --obj->timer > 0))
                        continue;


                /*
                 * if we get this far, object's timer has expired. 
                 */

                AtTemp = AtPlain;
                switch (obj->item_type)
                {
                default:
                        message = "$p has depleted itself.";
                        AtTemp = AtPlain;
                        break;

                case ItemBeacon:
                        make_scraps(obj);
                        message = "$p runs out of power and self destructs.";
                        AtTemp = AtObject;
                        break;
                case ItemGrenade:
                        explode(obj);
                        return;
                        break;

                case ItemPortal:
                        message = "$p winks out of existence.";
                        remove_portal(obj);
                        obj->item_type = ItemTrash;    /* so extract_obj    */
                        AtTemp = AtMagic; /* doesn't remove_portal */
                        break;
                case ItemFountain:
                        message = "$p dries up.";
                        AtTemp = AtBlue;
                        break;
                case ItemCorpseNpc:
                        message = "$p decays into dust and blows away.";
                        AtTemp = AtObject;
                        break;
                case ItemDroidCorpse:
                        message = "$p rusts away into oblivion.";
                        AtTemp = AtObject;
                        break;
                case ItemCorpsePc:
                        message = "$p decays into dust and is blown away...";
                        AtTemp = AtMagic;
                        break;
                case ItemFood:
                        message = "$p is devoured by a swarm of maggots.";
                        AtTemp = AtHungry;
                        break;
                case ItemBlood:
                        message = "$p slowly seeps into the ground.";
                        AtTemp = AtBlood;
                        break;
                case ItemBloodstain:
                        message = "$p dries up into flakes and blows away.";
                        AtTemp = AtBlood;
                        break;
                case ItemScraps:
                        message = "$p crumbles and decays into nothing.";
                        AtTemp = AtObject;
                        break;
                case ItemFire:
                        if (obj->in_room)
                                --obj->in_room->light;
                        message = "$p burns out.";
                        AtTemp = AtFire;
                }

                if (obj->carried_by)
                {
                        act(AtTemp, message, obj->carried_by, obj, NULL,
                            ToChar);
                }
                else if (obj->in_room
                         && (rch = obj->in_room->first_person) != NULL
                         && !IsObjStat(obj, ItemBurried))
                {
                        act(AtTemp, message, rch, obj, NULL, ToRoom);
                        act(AtTemp, message, rch, obj, NULL, ToChar);
                }
                if (obj->item_type != ItemBeacon)
                        extract_obj(obj);
        }
        return;
}   /*
     * Function to check important stuff happening to a player
     * This function should take about 5% of mud cpu time
     */
void char_check(void)
{
        CharData *ch, *ch_next;
        ExitData *pexit;
        static int cnt = 0;
        int       door, retcode;

        cnt = (cnt + 1) % 2;

        for (ch = first_char; ch; ch = ch_next)
        {
                ch_next = ch->next;
                will_fall(ch, 0);

                if (char_died(ch))
                        continue;

                if (IsNpc(ch))
                {
                        if (cnt != 0)
                                continue;

                        /*
                         * running mobs -Thoric 
                         */
                        if (!IsSet(ch->act, ActSentinel)
                            && !ch->fighting && ch->hunting)
                        {
                                WaitState(ch, 2 * PulseViolence);
                                hunt_victim(ch);
                                continue;
                        }

                        if (ch->spec_fun)
                        {
                                if ((*ch->spec_fun) (ch))
                                        continue;
                                if (char_died(ch))
                                        continue;
                        }
                        if (ch->spec_2)
                        {
                                if ((*ch->spec_2) (ch))
                                        continue;
                                if (char_died(ch))
                                        continue;
                        }

                        if (!IsSet(ch->act, ActSentinel)
                            && !IsSet(ch->act, ActPrototype)
                            && !ch->held
                            && (door = number_bits(4)) <= 9
                            && (pexit = get_exit(ch->in_room, door)) != NULL
                            && pexit->to_room
                            && !IsSet(pexit->exit_info, ExClosed)
                            && !IsSet(pexit->to_room->RoomFlags,
                                        RoomNoMob)
                            && (!IsSet(ch->act, ActStayArea)
                                || pexit->to_room->area == ch->in_room->area))
                        {
                                retcode = move_char(ch, pexit, 0, FALSE);
                                if (char_died(ch))
                                        continue;
                                if (retcode != rNONE
                                    || IsSet(ch->act, ActSentinel)
                                    || ch->position < PosStanding)
                                        continue;
                        }
                        continue;
                }
                else
                {
                        if (ch->mount && ch->in_room != ch->mount->in_room)
                        {
                                RemoveBit(ch->mount->act, ActMounted);
                                ch->mount = NULL;
                                ch->position = PosStanding;
                                send_to_char
                                        ("No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r",
                                         ch);
                        }

                        if ((ch->in_room
                             && ch->in_room->sector_type == SectUnderwater)
                            || (ch->in_room
                                && ch->in_room->sector_type ==
                                SectOceanfloor))
                        {
                                if (!IsAffected(ch, AffAquaBreath))
                                {
                                        if (get_trust(ch) < LevelImmortal)
                                        {
                                                int       dam;


                                                dam = number_range(ch->
                                                                   max_hit /
                                                                   50,
                                                                   ch->
                                                                   max_hit /
                                                                   30);
                                                dam = UMAX(1, dam);
                                                if (ch->hit <= 0)
                                                        dam = UMIN(10, dam);
                                                if (number_bits(3) == 0)
                                                        send_to_char
                                                                ("You cough and choke as you try to breathe water!\n\r",
                                                                 ch);
                                                damage(ch, ch, dam,
                                                       TypeUndefined);
                                        }
                                }
                        }

                        if (char_died(ch))
                                continue;

                        if (ch->in_room
                            &&
                            ((ch->in_room->sector_type == SectWaterNoswim)
                             || (ch->in_room->sector_type ==
                                 SectWaterSwim)))
                        {
                                if (!IsAffected(ch, AffFlying)
                                    && !IsAffected(ch, AffFloating)
                                    && !IsAffected(ch, AffAquaBreath)
                                    && !ch->mount)
                                {
                                        if (get_trust(ch) < LevelImmortal)
                                        {
                                                int       dam;

                                                if (ch->endurance > 0)
                                                        ch->endurance--;
                                                else
                                                {
                                                        dam = number_range
                                                                (ch->max_hit /
                                                                 50,
                                                                 ch->max_hit /
                                                                 30);
                                                        dam = UMAX(1, dam);
                                                        if (ch->hit <= 0)
                                                                dam = UMIN(10,
                                                                           dam);
                                                        if (number_bits(3) ==
                                                            0)
                                                                send_to_char
                                                                        ("Struggling with exhaustion, you choke on a mouthful of water.\n\r",
                                                                         ch);
                                                        damage(ch, ch, dam,
                                                               TypeUndefined);
                                                }
                                        }
                                }
                        }

                }
        }
}   /*
     * Aggress.
     *
     * for each descriptor
     *     for each mob in room
     *         aggress on some random PC
     *
     * This function should take 5% to 10% of ALL mud cpu time.
     * Unfortunately, checking on each PC move is too tricky,
     *   because we don't the mob to just attack the first PC
     *   who leads the party into the room.
     *
     */
void aggr_update(void)
{
        DescriptorData *d, *dnext;
        CharData *wch;
        CharData *ch;
        CharData *ch_next;
        CharData *victim;
        struct act_prog_data *apdtmp;

#ifdef UNDEFD
        /*
         *  GRUNT!  To do
         *
         */
        if (IsNpc(wch) && wch->mpactnum > 0
            && wch->in_room->area->nplayer > 0)
        {
                MProgActList *tmp_act, *tmp2_act;

                for (tmp_act = wch->mpact; tmp_act; tmp_act = tmp_act->next)
                {
                        oprog_wordlist_check(tmp_act->buf, wch, tmp_act->ch,
                                             tmp_act->obj, tmp_act->vo,
                                             ActProg);
                        DISPOSE(tmp_act->buf);
                }
                for (tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act)
                {
                        tmp2_act = tmp_act->next;
                        DISPOSE(tmp_act);
                }
                wch->mpactnum = 0;
                wch->mpact = NULL;
        }
#endif

        /*
         * check mobprog act queue 
         */
        while ((apdtmp = mob_act_list) != NULL)
        {
                wch = (CharData *) mob_act_list->vo;
                if (!char_died(wch) && wch->mpactnum > 0)
                {
                        MProgActList *tmp_act;

                        while ((tmp_act = wch->mpact) != NULL)
                        {
                                if (tmp_act->obj
                                    && obj_extracted(tmp_act->obj))
                                        tmp_act->obj = NULL;
                                if (tmp_act->ch && !char_died(tmp_act->ch))
                                        mprog_wordlist_check(tmp_act->buf,
                                                             wch, tmp_act->ch,
                                                             tmp_act->obj,
                                                             tmp_act->vo,
                                                             ActProg);
                                wch->mpact = tmp_act->next;
                                DISPOSE(tmp_act->buf);
                                DISPOSE(tmp_act);
                        }
                        wch->mpactnum = 0;
                        wch->mpact = NULL;
                }
                mob_act_list = apdtmp->next;
                DISPOSE(apdtmp);
        }   /*
             * Just check descriptors here for victims to aggressive mobs
             * We can check for linkdead victims to mobile_update  -Thoric
             */
        for (d = first_descriptor; d; d = dnext)
        {
                dnext = d->next;
                if (d->connected != ConPlaying
                    || (wch = d->character) == NULL)
                        continue;

                if (char_died(wch)
                    || IsNpc(wch)
                    || wch->top_level >= LevelImmortal || !wch->in_room)
                        continue;

                for (ch = wch->in_room->first_person; ch; ch = ch_next)
                {
                        int       count = 0;

                        ch_next = ch->next_in_room;

                        if (!IsNpc(ch)
                            || ch->fighting
                            || IsAffected(ch, AffCharm)
                            || !IsAwake(ch)
                            || (IsSet(ch->act, ActWimpy))
                            || !can_see(ch, wch) || ch->position != 8)
                                continue;

                        if (is_hating(ch, wch) && ch->position == 8)
                        {
                                found_prey(ch, wch);
                                continue;
                        }

                        if (!IsSet(ch->act, ActAggressive)
                            || IsSet(ch->act, ActMounted)
                            || IsSet(ch->in_room->RoomFlags, RoomSafe))
                                continue;

                        victim = wch;

                        if (!victim)
                        {
                                bug("Aggr_update: null victim.", count);
                                continue;
                        }

                        if (get_timer(victim, TimerRecentfight) > 0)
                                continue;

                        if (IsNpc(ch) && IsSet(ch->attacks, AtckBackstab))
                        {
                                ObjData *obj;

                                if (!ch->mount
                                    && (obj =
                                        get_eq_char(ch, WearWield)) != NULL
                                    && obj->value[3] == 11
                                    && !victim->fighting
                                    && victim->hit >= victim->max_hit)
                                {
                                        WaitState(ch,
                                                   skill_table[gsn_backstab]->
                                                   beats);
                                        if (!IsAwake(victim)
                                            || number_percent() + 5 <
                                            ch->top_level)
                                        {
                                                global_retcode =
                                                        multi_hit(ch, victim,
                                                                  gsn_backstab);
                                                continue;
                                        }
                                        else
                                        {
                                                global_retcode =
                                                        damage(ch, victim, 0,
                                                               gsn_backstab);
                                                continue;
                                        }
                                }
                        }
                        global_retcode =
                                multi_hit(ch, victim, TypeUndefined);
                }
        }

        return;
}

/* From interp.c */
bool check_social args((CharData * ch, char *command, char *argument));

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
void drunk_randoms(CharData * ch)
{
        CharData *rvch = NULL;
        CharData *vch;
        sh_int    drunk;
        sh_int    position;

        if (IsNpc(ch) || ch->PCData->condition[CondDrunk] <= 0)
                return;

        if (number_percent() < 30)
                return;

        drunk = ch->PCData->condition[CondDrunk];
        position = ch->position;
        ch->position = PosStanding;

        if (number_percent() < (2 * drunk / 20))
                check_social(ch, "burp", "");
        else if (number_percent() < (2 * drunk / 20))
                check_social(ch, "hiccup", "");
        else if (number_percent() < (2 * drunk / 20))
                check_social(ch, "drool", "");
        else if (number_percent() < (2 * drunk / 20))
                check_social(ch, "fart", "");
        else if (drunk > (10 + (get_curr_con(ch) / 5))
                 && number_percent() < (2 * drunk / 18))
        {
                for (vch = ch->in_room->first_person; vch;
                     vch = vch->next_in_room)
                        if (number_percent() < 10)
                                rvch = vch;
                check_social(ch, "puke", (rvch ? rvch->name : (char *) ""));
        }

        ch->position = position;
        return;
}

void halucinations(CharData * ch)
{
        if (ch->mental_state >= 30
            && number_bits(5 - (ch->mental_state >= 50) -
                           (ch->mental_state >= 75)) == 0)
        {
                char     *t;

                switch (number_range(1, UMIN(20, (ch->mental_state + 5) / 5)))
                {
                default:
                case 1:
                        t = "You feel very restless... you can't sit still.\n\r";
                        break;
                case 2:
                        t = "You're tingling all over.\n\r";
                        break;
                case 3:
                        t = "Your skin is crawling.\n\r";
                        break;
                case 4:
                        t = "You suddenly feel that something is terribly wrong.\n\r";
                        break;
                case 5:
                        t = "Those damn little fairies keep laughing at you!\n\r";
                        break;
                case 6:
                        t = "You can hear your mother crying...\n\r";
                        break;
                case 7:
                        t = "Have you been here before, or not?  You're not sure...\n\r";
                        break;
                case 8:
                        t = "Painful childhood memories flash through your mind.\n\r";
                        break;
                case 9:
                        t = "You hear someone call your name in the distance...\n\r";
                        break;
                case 10:
                        t = "Your head is pulsating... you can't think straight.\n\r";
                        break;
                case 11:
                        t = "The ground... seems to be squirming...\n\r";
                        break;
                case 12:
                        t = "You're not quite sure what is real anymore.\n\r";
                        break;
                case 13:
                        t = "It's all a dream... or is it?\n\r";
                        break;
                case 14:
                        t = "They're coming to get you... coming to take you away...\n\r";
                        break;
                case 15:
                        t = "You begin to feel all powerful!\n\r";
                        break;
                case 16:
                        t = "You're light as air... the heavens are yours for the taking.\n\r";
                        break;
                case 17:
                        t = "Your whole life flashes by... and your future...\n\r";
                        break;
                case 18:
                        t = "You are everywhere and everything... you know all and are all!\n\r";
                        break;
                case 19:
                        t = "You feel immortal!\n\r";
                        break;
                case 20:
                        t = "Ahh... the power of a Supreme Entity... what to do...\n\r";
                        break;
                }
                send_to_char(t, ch);
        }
        return;
}

void auth_update(void)
{
        CharData *victim;
        DescriptorData *d;
        char      buf[MaxInputLength], log_buffer[MaxInputLength];
        bool      found_hit = FALSE;    /* was at least one found? */

        mudstrlcpy(log_buffer, "Pending authorizations:\n\r", MIL);
        for (d = first_descriptor; d; d = d->next)
        {
                if ((victim = d->character) && IsWaitingForAuth(victim))
                {
                        found_hit = TRUE;
                        snprintf(buf, MSL, " %s@%s new %s %s %s\n\r",
                                 victim->name, victim->desc->host,
                                 victim->sex ==
                                 SexMale ? "Male" : victim->sex ==
                                 SexFemale ? "Female" : "Neutra",
                                 victim->race->name(),
                                 ability_name[victim->main_ability]);
                        mudstrlcat(log_buffer, buf, MIL);
                }
        }
        if (found_hit)
        {
                log_string(log_buffer);
        }
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler(void)
{
        static int pulse_crashover;
        static int pulse_auth;
        struct timeval systime;
        struct timeval etime;

        if (timechar)
        {
                set_char_color(AtPlain, timechar);
                send_to_char("Starting update timer.\n\r", timechar);
                gettimeofday(&systime, NULL);
        }

        if (--sysdata.pulse_area <= 0)  /* > */
        {
                sysdata.pulse_area =
                        number_range(PulseArea / 2, 3 * PulseArea / 2);
                area_update();
                quest_update();
                bacta_update();
                battalion_update();
        }

        if (--sysdata.pulse_taxes <= 0)
        {
                sysdata.pulse_taxes = PulseTaxes;
                update_orbit();
                update_taxes();
                update_salaries();
                update_baccounts();
        }

        if (--sysdata.pulse_mobile <= 0)
        {
                sysdata.pulse_mobile = PulseMobile;
                mobile_update();
        }

        if (--sysdata.pulse_space <= 0)
        {
                sysdata.pulse_space = PulseSpace;
                update_shuttle();
                update_space();
                do_who(NULL, "");
        }

        if (--sysdata.pulse_recharge <= 0)
        {
                sysdata.pulse_recharge = PulseSpace / 3;
                recharge_ships();
        }

        if (--sysdata.pulse_ship <= 0)
        {
                sysdata.pulse_ship = PulseSpace / 10;
                move_ships();
        }

        if (--sysdata.pulse_violence <= 0)
        {
                sysdata.pulse_violence = PulseViolence;
                arena_update();
                violence_update();
        }

        if (--sysdata.pulse_point <= 0)
        {
                sysdata.pulse_point =
                        number_range((int) (PulseTick * 0.75),
                                     (int) (PulseTick * 1.25));
                weather_update();
                char_update();
                obj_update();
        }

        /*
         * This seems wrong to me, but i can't figure out why - Gavin 
         */
        if (--pulse_crashover <= 0)
        {
                pulse_crashover = PulseCrashOver;
        }
        else if (--pulse_crashover <= 0)
        {
                pulse_crashover = PulseCrashOver;
                init_crashover();
        }

        if (--pulse_auth <= 0)
        {
                pulse_auth = PulseAuth;
                auth_update();  /* Gorog */
        }

        if (--sysdata.pulse_second <= 0)
        {
                sysdata.pulse_second = PulsePerSecond;
                char_check();
#ifdef OlcHomes
                save_homes_check();
#endif
                check_pfiles(0);
                check_dns();

                /*
                 * reboot_check( "" ); Disabled to check if its lagging a lot - Scryn
                 */
                /*
                 * Much faster version enabled by Altrag..
                 * * although I dunno how it could lag too much, it was just a bunch
                 * * of comparisons.. 
                 */
                reboot_check(0);

                fireplanet_update();
        }

        if (auction->item && --auction->pulse <= 0)
        {
                auction->pulse = PulseAuction;
                auction_update();
        }

        mpsleep_update();   /* Check for sleeping mud progs -rkb */
        aggr_update();
        obj_act_update();
        room_act_update();
        clean_obj_queue();  /* dispose of extracted objects */
        clean_char_queue(); /* dispose of dead mobs/quitting chars */
        if (timechar)
        {
                gettimeofday(&etime, NULL);
                set_char_color(AtPlain, timechar);
                send_to_char("Update timing complete.\n\r", timechar);
                subtract_times(&etime, &systime);
                ch_printf(timechar, "Timing took %d.%06d seconds.\n\r",
                          etime.tv_sec, etime.tv_usec);
                timechar = NULL;
        }
        tail_chain();
        return;
}


void remove_portal(ObjData * portal)
{
        RoomIndexData *fromRoom, *toRoom;
        CharData *ch;
        ExitData *pexit;
        bool      found;

        if (!portal)
        {
                bug("remove_portal: portal is NULL", 0);
                return;
        }

        fromRoom = portal->in_room;
        found = FALSE;
        if (!fromRoom)
        {
                bug("remove_portal: portal->in_room is NULL", 0);
                return;
        }

        for (pexit = fromRoom->first_exit; pexit; pexit = pexit->next)
                if (IsSet(pexit->exit_info, ExPortal))
                {
                        found = TRUE;
                        break;
                }

        if (!found)
        {
                bug("remove_portal: portal not found in room %d!",
                    fromRoom->vnum);
                return;
        }

        if (pexit->vdir != DirPortal)
                bug("remove_portal: exit in dir %d != DirPortal",
                    pexit->vdir);

        if ((toRoom = pexit->to_room) == NULL)
                bug("remove_portal: toRoom is NULL", 0);

        extract_exit(fromRoom, pexit);
        /*
         * rendunancy 
         */
        /*
         * send a message to fromRoom 
         */
        /*
         * ch = fromRoom->first_person; 
         */
        /*
         * if(ch!=NULL) 
         */
        /*
         * act( AtPlain, "A magical portal below winks from existence.", ch, NULL, NULL, ToRoom ); 
         */

        /*
         * send a message to toRoom 
         */
        if (toRoom && (ch = toRoom->first_person) != NULL)
                act(AtPlain, "A magical portal above winks from existence.",
                    ch, NULL, NULL, ToRoom);

        /*
         * remove the portal obj: looks better to let update_obj do this 
         */
        /*
         * extract_obj(portal);  
         */

        return;
}

void reboot_check(time_t reset)
{
        static char *tmsg[] = { "SYSTEM: Reboot in 10 seconds.",
                "SYSTEM: Reboot in 30 seconds.",
                "SYSTEM: Reboot in 1 minute.",
                "SYSTEM: Reboot in 2 minutes.",
                "SYSTEM: Reboot in 3 minutes.",
                "SYSTEM: Reboot in 4 minutes.",
                "SYSTEM: Reboot in 5 minutes.",
                "SYSTEM: Reboot in 10 minutes.",
        };
        static const int times[] = { 10, 30, 60, 120, 180, 240, 300, 600 };
        static const int timesize = UMIN(sizeof(times) / sizeof(*times),
                                         sizeof(tmsg) / sizeof(*tmsg));
        char      buf[MaxStringLength];
        static int trun;
        static bool init;

        if (!init || reset >= current_time)
        {
                for (trun = timesize - 1; trun >= 0; trun--)
                        if (reset >= current_time + times[trun])
                                break;
                init = TRUE;
                return;
        }

        if ((current_time % 1800) == 0)
        {
                snprintf(buf, MSL, "%.24s: %d players", ctime(&current_time),
                         num_descriptors);
                append_to_file(UsageFile, buf);
        }

        if (new_boot_time_t - boot_time < 60 * 60 * 18 &&
            !set_boot_time->manual)
                return;

        if (new_boot_time_t <= current_time)
        {
                CharData *vch;
                extern bool mud_down;

                if (auction->item)
                {
                        snprintf(buf, MSL,
                                 "Sale of %s has been stopped by mud.",
                                 auction->item->short_descr);
                        talk_auction(buf);
                        obj_to_char(auction->item, auction->seller);
                        if (auction->item->item_type == ItemBeacon)
                                auction->item->value[0] = 0;
                        auction->item = NULL;
                        if (auction->buyer
                            && auction->buyer != auction->seller)
                        {
                                auction->buyer->gold += auction->bet;
                                send_to_char
                                        ("Your money has been returned.\n\r",
                                         auction->buyer);
                        }
                }
                echo_to_all(AtYellow,
                            "You are forced from these realms by a strong "
                            "presence\n\ras life here is reconstructed.",
                            EchoTarAll);

                for (vch = first_char; vch; vch = vch->next)
                        if (!IsNpc(vch))
                                save_char_obj(vch);
                mud_down = TRUE;
                return;
        }

        if (trun != -1 && new_boot_time_t - current_time <= times[trun])
        {
                echo_to_all(AtYellow, tmsg[trun], EchoTarAll);
                if (trun <= 5)
                        sysdata.DenyNewPlayers = TRUE;
                --trun;
                return;
        }
        return;
}


/* the auction update*/

void auction_update(void)
{
        int       tax, pay;
        char      buf[MaxStringLength];

        switch (++auction->going)   /* increase the going state */
        {
        case 1:    /* going once */
        case 2:    /* going twice */
                if (auction->bet > auction->starting)
                        snprintf(buf, MSL, "%s: going %s for %d.",
                                 auction->item->short_descr,
                                 ((auction->going == 1) ? "once" : "twice"),
                                 auction->bet);
                else
                        snprintf(buf, MSL,
                                 "%s: going %s (bid not received yet).",
                                 auction->item->short_descr,
                                 ((auction->going == 1) ? "once" : "twice"));

                talk_auction(buf);
                break;

        case 3:    /* SOLD! */
                if (!auction->buyer && auction->bet)
                {
                        bug("Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet);
                        auction->bet = 0;
                }
                if (auction->bet > 0 && auction->buyer != auction->seller)
                {
                        snprintf(buf, MSL, "%s sold to %s for %d.",
                                 auction->item->short_descr,
                                 IsNpc(auction->buyer) ? auction->buyer->
                                 short_descr : auction->buyer->name,
                                 auction->bet);
                        talk_auction(buf);

                        act(AtAction,
                            "The auctioneer materializes before you, and hands you $p.",
                            auction->buyer, auction->item, NULL, ToChar);
                        act(AtAction,
                            "The auctioneer materializes before $n, and hands $m $p.",
                            auction->buyer, auction->item, NULL, ToRoom);

                        if ((auction->buyer->carry_weight
                             + get_obj_weight(auction->item))
                            > can_carry_w(auction->buyer))
                        {
                                act(AtPlain,
                                    "$p is too heavy for you to carry with your current inventory.",
                                    auction->buyer, auction->item, NULL,
                                    ToChar);
                                act(AtPlain,
                                    "$n is carrying too much to also carry $p, and $e drops it.",
                                    auction->buyer, auction->item, NULL,
                                    ToRoom);
                                obj_to_room(auction->item,
                                            auction->buyer->in_room);
                        }
                        else
                                obj_to_char(auction->item, auction->buyer);
                        pay = (int) (auction->bet * 0.9);
                        tax = (int) (auction->bet * 0.1);
                        boost_economy(auction->seller->in_room->area, tax);
                        auction->seller->gold += pay;   /* give him the money, tax 10 % */
                        snprintf(buf, MSL,
                                 "The auctioneer pays you %d gold, charging an auction fee of %d.\n\r",
                                 pay, tax);
                        send_to_char(buf, auction->seller);
                        auction->item = NULL;   /* reset item */
                        if (IsSet(sysdata.save_flags, SvAuction))
                        {
                                save_char_obj(auction->buyer);
                                save_char_obj(auction->seller);
                        }
                }
                else    /* not sold */
                {
                        snprintf(buf, MSL,
                                 "No bids received for %s - object has been removed from auction\n\r.",
                                 auction->item->short_descr);
                        talk_auction(buf);
                        act(AtAction,
                            "The auctioneer appears before you to return $p to you.",
                            auction->seller, auction->item, NULL, ToChar);
                        act(AtAction,
                            "The auctioneer appears before $n to return $p to $m.",
                            auction->seller, auction->item, NULL, ToRoom);
                        if ((auction->seller->carry_weight +
                             get_obj_weight(auction->item)) >
                            can_carry_w(auction->seller))
                        {
                                act(AtPlain,
                                    "You drop $p as it is just too much to carry"
                                    " with everything else you're carrying.",
                                    auction->seller, auction->item, NULL,
                                    ToChar);
                                act(AtPlain,
                                    "$n drops $p as it is too much extra weight"
                                    " for $m with everything else.",
                                    auction->seller, auction->item, NULL,
                                    ToRoom);
                                obj_to_room(auction->item,
                                            auction->seller->in_room);
                        }
                        else
                                obj_to_char(auction->item, auction->seller);
                        tax = (int) (auction->item->cost * 0.05);
                        boost_economy(auction->seller->in_room->area, tax);
                        snprintf(buf, MSL,
                                 "The auctioneer charges you an auction fee of %d.\n\r",
                                 tax);
                        send_to_char(buf, auction->seller);
                        if ((auction->seller->gold - tax) < 0)
                                auction->seller->gold = 0;
                        else
                                auction->seller->gold -= tax;
                        if (IsSet(sysdata.save_flags, SvAuction))
                                save_char_obj(auction->seller);
                }   /* else */
                auction->item = NULL;   /* clear auction */
        }   /* switch */
}   /* func */

void subtract_times(struct timeval *etime, struct timeval *systime)
{
        etime->tv_sec -= systime->tv_sec;
        etime->tv_usec -= systime->tv_usec;
        while (etime->tv_usec < 0)
        {
                etime->tv_usec += 1000000;
                etime->tv_sec--;
        }
        return;
}

void bacta_update(void)
{
        CharData *ch;

        for (ch = first_char; ch != NULL; ch = ch->next)
        {
                if (IsNpc(ch))
                        continue;

                if (IsSet(ch->in_room->RoomFlags, RoomBacta))
                {
                        send_to_char
                                ("You feel the soothing bubbles of the bacta!\n\rYou are healed.\n\r",
                                 ch);

                        ch->hit += 100;
                        if (ch->hit > ch->max_hit)
                        {
                                ch->hit = ch->max_hit;
                        }
                }

                if (IsSet(ch->in_room->RoomFlags, RoomBactaCharge))
                {

                        if (ch->gold < 25)
                        {
                                send_to_char
                                        ("You do not have the money to pay for the bacta.\n\r",
                                         ch);
                                continue;
                        }
                        send_to_char
                                ("You pay the fee for the bacta tank.\n\rYou feel the soothing bubbles of the bacta!\n\rYou are healed.\n\r",
                                 ch);

                        ch->hit += 100;

                        ch->gold -= 25;
                        if (ch->hit > ch->max_hit)
                                ch->hit = ch->max_hit;
                }
        }
        return;
}

void battalion_update(void)
{
        PlanetData *planet;
        int       attchange, defchange, attmult, defmult;
        ClanData *clan;
        ClanData *aclan;
        ClanData *attacker;
        AreaData *area;
        CharData *ch;
        char      buf[MaxStringLength];
        InstallationData *installation;

        for (planet = first_planet; planet; planet = planet->next)
        {
                if (planet->attgovern && (planet->attbattalions > 0))
                {
                        clan = planet->governed_by;
                        attacker = planet->attgovern;
                        attchange = 0;
                        defchange = 0;
                        attmult = 1;
                        defmult = 1;

                        if (!clan)
                        {
                                planet->defbattalions = planet->attbattalions;
                                planet->attbattalions = 0;
                                planet->attgovern = NULL;
                                planet->governed_by = attacker;
                                snprintf(buf, MSL,
                                         "%s has been conquered by %s!",
                                         planet->name, attacker->name);
                                echo_to_all(AtRed, buf, 0);
                                continue;
                        }
                        for (area = planet->first_area; area;
                             area = area->next_on_planet)
                        {
                                for (ch = area->first_person; ch;
                                     ch = ch->next_in_area)
                                {
                                        aclan = NULL;
                                        if (ch->PCData->clan)
                                        {
                                                if (ch->PCData->clan->
                                                    mainclan)
                                                        aclan = ch->PCData->
                                                                clan->
                                                                mainclan;
                                                else
                                                        aclan = ch->PCData->
                                                                clan;
                                        }

                                        if (aclan == attacker)
                                        {
                                                defmult = defmult + 1;
                                                send_to_char
                                                        ("You help your clan attack in battle!",
                                                         ch);
                                        }
                                        if (aclan == clan)
                                        {
                                                attmult = attmult + 1;
                                                send_to_char
                                                        ("You help your clan defend in battle!",
                                                         ch);
                                        }
                                }
                        }
                        attmult += number_range(1, 3);
                        attmult -= number_range(1, 3);

                        defmult += number_range(1, 3);
                        defmult -= number_range(1, 3);

                        if (planet->pop_support < 0)
                                defmult = defmult * 2;
                        else
                                attmult = attmult * 2;

                        if (attmult < 0)
                                attmult = 0;
                        if (defmult < 0)
                                defmult = 0;
                        attchange = attmult + 1;
                        defchange = defmult;
                        planet->attbattalions -= attchange;
                        planet->defbattalions -= defchange;

                        if (0 >= planet->attbattalions)
                        {
                                if (planet->defbattalions < 0)
                                        planet->defbattalions = 0;
                                planet->attbattalions = 0;
                                planet->attgovern = NULL;
                                /*
                                 * If they are happy with the current gov, defending will increase pop support 
                                 */
                                if (planet->pop_support > 50)
                                        planet->pop_support += 5;
                                snprintf(buf, MSL,
                                         "%s has been defended against %s!",
                                         planet->name, attacker->name);
                                echo_to_all(AtRed, buf, 0);
                        }

                        if ((0 >= planet->defbattalions)
                            && (planet->attbattalions > 0))
                        {
                                planet->defbattalions = planet->attbattalions;
                                planet->attbattalions = 0;
                                planet->attgovern = NULL;
                                planet->governed_by = attacker;
                                snprintf(buf, MSL,
                                         "%s has been conquered by %s!",
                                         planet->name, attacker->name);
                                echo_to_all(AtRed, buf, 0);

                                planet->pop_support = 50;
                                for (installation = planet->first_install;
                                     installation;
                                     installation =
                                     installation->next_on_planet)
                                        installation->clan =
                                                planet->governed_by;
                        }
                }

        }
}

/* Can I remove this? */
void logsize_update(void)
{
        char      buf[MaxStringLength];

        snprintf(buf, MSL, "%s%s", SystemDir, BugFile);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", SystemDir, LogFile);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", SystemDir, IdeaFile);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", SystemDir, HelpFile);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", LogDir, "swr.log");
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
}
