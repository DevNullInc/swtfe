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
 *                                   Skills Module                                       *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"
#include "races.hpp"

char     *const spell_flag[] =
        { "water", "earth", "air", "astral", "area", "distant", "reverse",
        "save_half_dam", "save_negates", "accumulative", "recastable",
        "noscribe",
        "nobrew", "group", "object", "character", "secretskill", "pksensitive"
};

char     *const spell_saves[] =
        { "none", "poison_death", "wands", "para_petri", "breath",
        "spell_staff"
};

char     *const spell_damage[] =
        { "none", "fire", "cold", "electricity", "energy", "acid", "poison",
        "drain"
};

char     *const spell_action[] =
        { "none", "create", "destroy", "resist", "suscept", "divinate",
        "obscure",
        "change"
};

char     *const spell_power[] = { "none", "minor", "greater", "major" };

char     *const spell_class[] =
        { "none", "lunar", "solar", "travel", "summon", "life", "death",
        "illusion"
};

char     *const target_type[] =
        { "ignore", "offensive", "defensive", "self", "objinv" };


void      show_char_to_char(CharData * list, CharData * ch);

int       ris_save(CharData * ch, int percent_chance, int ris);
bool      check_illegal_psteal(CharData * ch, CharData * victim);
int get_partflag args((char *flag));

/* from arena.c */
bool    in_arena        args((CharData * ch));

/* from magic.c */
void      failed_casting(struct skill_type *skill, CharData * ch,
                         CharData * victim, ObjData * obj);

int xp_compute args((CharData * gch, CharData * victim));

RoomIndexData *generate_exit(RoomIndexData * in_room, ExitData ** pexit);

/*
 * Dummy function
 */
void skill_notfound(CharData * ch, char *argument)
{
        argument = NULL;
        send_to_char("Huh?\n\r", ch);
        return;
}


int get_ssave(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(spell_saves) / sizeof(spell_saves[0]); x++)
                if (!str_cmp(name, spell_saves[x]))
                        return x;
        return -1;
}

int get_starget(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(target_type) / sizeof(target_type[0]); x++)
                if (!str_cmp(name, target_type[x]))
                        return x;
        return -1;
}

int get_sflag(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(spell_flag) / sizeof(spell_flag[0]); x++)
                if (!str_cmp(name, spell_flag[x]))
                        return x;
        return -1;
}

int get_sdamage(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(spell_damage) / sizeof(spell_damage[0]); x++)
                if (!str_cmp(name, spell_damage[x]))
                        return x;
        return -1;
}

int get_saction(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(spell_action) / sizeof(spell_action[0]); x++)
                if (!str_cmp(name, spell_action[x]))
                        return x;
        return -1;
}

int get_spower(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(spell_power) / sizeof(spell_power[0]); x++)
                if (!str_cmp(name, spell_power[x]))
                        return x;
        return -1;
}

int get_sclass(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(spell_class) / sizeof(spell_class[0]); x++)
                if (!str_cmp(name, spell_class[x]))
                        return x;
        return -1;
}

bool is_legal_kill(CharData * ch, CharData * vch)
{
        if (IsNpc(ch) || IsNpc(vch))
                return TRUE;
        if (is_safe(ch, vch))
                return FALSE;
        return TRUE;
}


extern char *target_name;   /* from magic.c */

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
bool check_skill(CharData * ch, char *command, char *argument)
{
        int       sn;
        int       first = gsn_first_skill;
        int       top = gsn_first_weapon - 1;
        struct timeval time_used;
        int       endurance;

        /*
         * bsearch for the skill 
         */
        for (;;)
        {
                sn = (first + top) >> 1;

                if (LOWER(command[0]) == LOWER(skill_table[sn]->name[0])
                    && !str_prefix(command, skill_table[sn]->name)
                    && (skill_table[sn]->skill_fun
                        || skill_table[sn]->spell_fun != spell_null)
                    && (IsNpc(ch) || (ch->pcdata->learned[sn] > 0)))
                        break;
                if (first >= top)
                        return FALSE;
                if (strcmp(command, skill_table[sn]->name) < 1)
                        top = sn - 1;
                else
                        first = sn + 1;
        }

        if (!check_pos(ch, skill_table[sn]->minimum_position))
                return TRUE;

        if (IsNpc(ch)
            && (IsAffected(ch, AffCharm) || IsAffected(ch, AffPossess)))
        {
                send_to_char
                        ("For some reason, you seem unable to perform that...\n\r",
                         ch);
                act(AtGrey, "$n looks around.", ch, NULL, NULL, ToRoom);
                return TRUE;
        }

        if (!IsImmortal(ch) && ch->race && skill_table[sn]->races
            && !is_name(ch->race->name(), skill_table[sn]->races))
        {
                send_to_char("Not quite able to perform that...\n\r", ch);
                act(AtGrey, "$n looks around.", ch, NULL, NULL, ToRoom);
                return TRUE;
        }
        /*
         * check if endurance is required 
         */
        if (skill_table[sn]->min_endurance)
        {
                endurance =
                        IsNpc(ch) ? 0 : (skill_table[sn]->alignment > -300
                                          && skill_table[sn]->alignment <
                                          300) ? skill_table[sn]->
                        min_endurance
                        : (abs
                           (((skill_table[sn]->alignment -
                              ch->alignment) / 100)) +
                           1) * skill_table[sn]->min_endurance;

                if (!IsNpc(ch) && ch->endurance < endurance)
                {
                        send_to_char
                                ("You need to rest before using the Force any more.\n\r",
                                 ch);
                        return TRUE;
                }
        }
        else
        {
                endurance = 0;
        }

        /*
         * Is this a real do-fun, or a really a spell?
         */
        if (!skill_table[sn]->skill_fun)
        {
                ch_ret    retcode = rNONE;
                void     *vo = NULL;
                CharData *victim = NULL;
                ObjData *obj = NULL;

                target_name = "";

                switch (skill_table[sn]->target)
                {
                default:
                        bug("Check_skill: bad target for sn %d.", sn);
                        send_to_char("Something went wrong...\n\r", ch);
                        return TRUE;

                case TarIgnore:
                        vo = NULL;
                        if (argument[0] == '\0')
                        {
                                if ((victim = who_fighting(ch)) != NULL)
                                        target_name = victim->name;
                        }
                        else
                                target_name = argument;
                        break;

                case TarCharOffensive:
                        if (argument[0] == '\0'
                            && (victim = who_fighting(ch)) == NULL)
                        {
                                ch_printf(ch, "%s who?\n\r",
                                          capitalize(skill_table[sn]->name));
                                return TRUE;
                        }
                        else if (argument[0] != '\0'
                                 && (victim =
                                     get_char_room(ch, argument)) == NULL)
                        {
                                send_to_char("They aren't here.\n\r", ch);
                                return TRUE;
                        }
                        if (is_safe(ch, victim))
                                return TRUE;
                        vo = (void *) victim;
                        break;

                case TarCharDefensive:
                        if (argument[0] != '\0'
                            && (victim = get_char_room(ch, argument)) == NULL)
                        {
                                send_to_char("They aren't here.\n\r", ch);
                                return TRUE;
                        }
                        if (!victim)
                                victim = ch;
                        vo = (void *) victim;
                        break;

                case TarCharSelf:
                        vo = (void *) ch;
                        break;

                case TarObjInv:
                        if ((obj = get_obj_carry(ch, argument)) == NULL)
                        {
                                send_to_char("You can't find that.\n\r", ch);
                                return TRUE;
                        }
                        vo = (void *) obj;
                        break;
                }

                /*
                 * waitstate 
                 */
                WaitState(ch, skill_table[sn]->beats);
                /*
                 * check for failure 
                 */
                {
                        int       percentage;

                        if ((percentage =
                             (number_percent() +
                              skill_table[sn]->difficulty * 5)) >
                            (IsNpc(ch) ? 75 : ch->pcdata->learned[sn]))
                        {
                                failed_casting(skill_table[sn], ch,
                                               (CharData *) vo, obj);
                                learn_from_failure(ch, sn);
                                if (endurance)
                                {
                                        ch->endurance -= endurance / 2;
                                }
                                return TRUE;
                        }
                }
                if (endurance)
                {
                        ch->endurance -= endurance;
                }
                start_timer(&time_used);
                retcode =
                        (*skill_table[sn]->spell_fun) (sn, ch->top_level, ch,
                                                       vo);
                end_timer(&time_used);
                update_userec(&time_used, &skill_table[sn]->userec);

                if (retcode == rCHAR_DIED || retcode == rERROR)
                        return TRUE;

                if (char_died(ch))
                        return TRUE;

                if (retcode == rSPELL_FAILED)
                {
                        learn_from_failure(ch, sn);
                        retcode = rNONE;
                }
                else
                        learn_from_success(ch, sn);

                if (skill_table[sn]->target == TarCharOffensive
                    && victim != ch && !char_died(victim))
                {
                        CharData *vch;
                        CharData *vch_next;

                        for (vch = ch->in_room->first_person; vch;
                             vch = vch_next)
                        {
                                vch_next = vch->next_in_room;
                                if (victim == vch && !victim->fighting
                                    && victim->master != ch)
                                {
                                        retcode =
                                                multi_hit(victim, ch,
                                                          TypeUndefined);
                                        break;
                                }
                        }
                }
                return TRUE;
        }

        if (endurance)
        {
                ch->endurance -= endurance;
        }
        ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
        ch->last_cmd = skill_table[sn]->skill_fun;
        start_timer(&time_used);
        (*skill_table[sn]->skill_fun) (ch, argument);
        end_timer(&time_used);
        update_userec(&time_used, &skill_table[sn]->userec);

        tail_chain();
        return TRUE;
}

/*
 * Lookup a skills information
 * High god command
 */
CMDF do_slookup(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        char      arg[MaxInputLength], arg2[MIL];
        int       sn;
        SkillType *skill = NULL;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);
        if (arg[0] == '\0')
        {
                send_to_char("Slookup what?\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "all"))
        {
                if (!arg2 || arg2[0] == '\0')
                        for (sn = 0;
                             sn < top_sn && skill_table[sn]
                             && skill_table[sn]->name; sn++)
                                pager_printf(ch,
                                             "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r",
                                             sn, skill_table[sn]->slot,
                                             skill_table[sn]->name,
                                             spell_damage[SpellDamage
                                                          (skill_table[sn])]);
                else if (!str_cmp(arg2, "spells"))
                        for (sn = 0;
                             sn < top_sn && skill_table[sn]
                             && skill_table[sn]->name; sn++)
                                if (skill_table[sn]->type == SkillSpell)
                                        pager_printf(ch,
                                                     "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r",
                                                     sn,
                                                     skill_table[sn]->slot,
                                                     skill_table[sn]->name,
                                                     spell_damage[SpellDamage
                                                                  (skill_table
                                                                   [sn])]);
                                else if (!str_cmp(arg2, "skills"))
                                        for (sn = 0;
                                             sn < top_sn && skill_table[sn]
                                             && skill_table[sn]->name; sn++)
                                                if (skill_table[sn]->type ==
                                                    SkillSkill)
                                                        pager_printf(ch,
                                                                     "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r",
                                                                     sn,
                                                                     skill_table
                                                                     [sn]->
                                                                     slot,
                                                                     skill_table
                                                                     [sn]->
                                                                     name,
                                                                     spell_damage
                                                                     [SpellDamage
                                                                      (skill_table
                                                                       [sn])]);
                                                else
                                                        for (sn = 0;
                                                             sn < top_sn
                                                             &&
                                                             skill_table[sn]
                                                             &&
                                                             skill_table[sn]->
                                                             name; sn++)
                                                                pager_printf
                                                                        (ch,
                                                                         "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r",
                                                                         sn,
                                                                         skill_table
                                                                         [sn]->
                                                                         slot,
                                                                         skill_table
                                                                         [sn]->
                                                                         name,
                                                                         spell_damage
                                                                         [SpellDamage
                                                                          (skill_table
                                                                           [sn])]);

        }
        else
        {
                SmaugAff *aff;
                int       cnt = 0;

                if (is_number(arg))
                {
                        sn = atoi(arg);
                        if ((skill = get_skilltype(sn)) == NULL)
                        {
                                send_to_char("Invalid sn.\n\r", ch);
                                return;
                        }
                        sn %= 1000;
                }
                else if ((sn = skill_lookup(arg)) >= 0)
                        skill = skill_table[sn];
                else
                {
                        send_to_char
                                ("No such skill, spell, proficiency or tongue.\n\r",
                                 ch);
                        return;
                }
                if (!skill)
                {
                        send_to_char("Not created yet.\n\r", ch);
                        return;
                }

                ch_printf(ch, "Sn: %4d Slot: %4d %s: '%-20s'\n\r",
                          sn, skill->slot, skill_tname[skill->type],
                          skill->name);
                if (skill->flags)
                {
                        int       x;

                        ch_printf(ch,
                                  "Damtype: %s  Acttype: %s   Classtype: %s   Powertype: %s\n\r",
                                  spell_damage[SpellDamage(skill)],
                                  spell_action[SpellAction(skill)],
                                  spell_class[SpellClass(skill)],
                                  spell_power[SpellPower(skill)]);
                        mudstrlcpy(buf, "Flags:", MSL);
                        for (x = 11; x < 32; x++)
                                if (SpellFlag(skill, 1 << x))
                                {
                                        mudstrlcat(buf, " ", MSL);
                                        mudstrlcat(buf, spell_flag[x - 11],
                                                   MSL);
                                }
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                }
                ch_printf(ch, "Saves: %s\n\r",
                          spell_saves[(int) skill->saves]);

                if (skill->difficulty != '\0')
                        ch_printf(ch, "Difficulty: %d\n\r",
                                  (int) skill->difficulty);

                ch_printf(ch,
                          "Type: %s  Target: %s  Minpos: %d  Endurance: %d  Beats: %d\n\r",
                          skill_tname[skill->type],
                          target_type[URANGE
                                      ((int) TarIgnore, skill->target,
                                       (int) TarObjInv)],
                          skill->minimum_position, skill->min_endurance,
                          skill->beats);
                ch_printf(ch, "Flags: %d  Guild: %s  Code: %s\n\r",
                          skill->flags, ability_name[skill->guild],
                          skill->skill_fun ? skill->skill_fun_name : skill->
                          spell_fun_name);
                ch_printf(ch, "Dammsg: %s\n\rWearoff: %s\n",
                          skill->noun_damage,
                          skill->msg_off ? skill->msg_off : "(none set)");
                if (skill->dice && skill->dice[0] != '\0')
                        ch_printf(ch, "Dice: %s\n\r", skill->dice);
                if (skill->teachers && skill->teachers[0] != '\0')
                        ch_printf(ch, "Teachers: %s\n\r", skill->teachers);
                if (skill->races && skill->races[0] != '\0')
                        ch_printf(ch, "Races: %s\n\r", skill->races);
                if (!xIS_EMPTY(skill->body_parts))
                        ch_printf(ch, "Parts: %s\n\r",
                                  ext_flag_string(&skill->body_parts,
                                                  part_flags));
                if (skill->components && skill->components[0] != '\0')
                        ch_printf(ch, "Components: %s\n\r",
                                  skill->components);
                if (skill->participants)
                        ch_printf(ch, "Participants: %d\n\r",
                                  (int) skill->participants);
                if (skill->userec.num_uses)
                        send_timer(&skill->userec, ch);
                for (aff = skill->affects; aff; aff = aff->next)
                {
                        if (aff == skill->affects)
                                send_to_char("\n\r", ch);
                        snprintf(buf, MSL, "Affect %d", ++cnt);
                        if (aff->location)
                        {
                                mudstrlcat(buf, " modifies ", MSL);
                                mudstrlcat(buf,
                                           a_types[aff->location %
                                                   ReverseApply], MSL);
                                mudstrlcat(buf, " by '", MSL);
                                mudstrlcat(buf, aff->modifier, MSL);
                                if (aff->bitvector)
                                        mudstrlcat(buf, "' and", MSL);
                                else
                                        mudstrlcat(buf, "'", MSL);
                        }
                        if (aff->bitvector)
                        {
                                int       x;

                                mudstrlcat(buf, " applies", MSL);
                                for (x = 0; x < 32; x++)
                                        if (IsSet(aff->bitvector, 1 << x))
                                        {
                                                mudstrlcat(buf, " ", MSL);
                                                mudstrlcat(buf, a_flags[x],
                                                           MSL);
                                        }
                        }
                        if (aff->duration[0] != '\0'
                            && aff->duration[0] != '0')
                        {
                                mudstrlcat(buf, " for '", MSL);
                                mudstrlcat(buf, aff->duration, MSL);
                                mudstrlcat(buf, "' rounds", MSL);
                        }
                        if (aff->location >= ReverseApply)
                                mudstrlcat(buf, " (affects caster only)",
                                           MSL);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        if (!aff->next)
                                send_to_char("\n\r", ch);
                }
                if (skill->hit_char && skill->hit_char[0] != '\0')
                        ch_printf(ch, "Hitchar   : %s\n\r", skill->hit_char);
                if (skill->hit_vict && skill->hit_vict[0] != '\0')
                        ch_printf(ch, "Hitvict   : %s\n\r", skill->hit_vict);
                if (skill->hit_room && skill->hit_room[0] != '\0')
                        ch_printf(ch, "Hitroom   : %s\n\r", skill->hit_room);
                if (skill->miss_char && skill->miss_char[0] != '\0')
                        ch_printf(ch, "Misschar  : %s\n\r", skill->miss_char);
                if (skill->miss_vict && skill->miss_vict[0] != '\0')
                        ch_printf(ch, "Missvict  : %s\n\r", skill->miss_vict);
                if (skill->miss_room && skill->miss_room[0] != '\0')
                        ch_printf(ch, "Missroom  : %s\n\r", skill->miss_room);
                if (skill->die_char && skill->die_char[0] != '\0')
                        ch_printf(ch, "Diechar   : %s\n\r", skill->die_char);
                if (skill->die_vict && skill->die_vict[0] != '\0')
                        ch_printf(ch, "Dievict   : %s\n\r", skill->die_vict);
                if (skill->die_room && skill->die_room[0] != '\0')
                        ch_printf(ch, "Dieroom   : %s\n\r", skill->die_room);
                if (skill->imm_char && skill->imm_char[0] != '\0')
                        ch_printf(ch, "Immchar   : %s\n\r", skill->imm_char);
                if (skill->imm_vict && skill->imm_vict[0] != '\0')
                        ch_printf(ch, "Immvict   : %s\n\r", skill->imm_vict);
                if (skill->imm_room && skill->imm_room[0] != '\0')
                        ch_printf(ch, "Immroom   : %s\n\r", skill->imm_room);
                if (skill->guild >= 0 && skill->guild < MaxAbility)
                {
                        snprintf(buf, MSL,
                                 "guild: %s   Align: %4d   lvl: %3d held: %d\n\r",
                                 ability_name[skill->guild], skill->alignment,
                                 skill->min_level, skill->held);
                        send_to_char(buf, ch);
                }
                send_to_char("\n\r", ch);
        }

        return;
}

/*
 * Set a skill's attributes or what skills a player has.
 * High god command, with support for creating skills/spells/etc
 */
CMDF do_sset(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        CharData *victim;
        int       value;
        int       sn;
        bool      fAll;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Syntax: sset <victim> <skill> <value>\n\r", ch);
                send_to_char("or:     sset <victim> all     <value>\n\r", ch);
                if (get_trust(ch) > LevelSubImplem)
                {
                        send_to_char("or:     sset save skill table\n\r", ch);
                        send_to_char
                                ("or:     sset create skill 'new skill'\n\r",
                                 ch);
                        send_to_char("or:     sset <sn> delete <sn> \n\r",
                                     ch);
                }
                if (get_trust(ch) > LevelGreater)
                {
                        send_to_char
                                ("or:     sset <sn>     <field> <value>\n\r",
                                 ch);
                        send_to_char("\n\rField being one of:\n\r", ch);
                        send_to_char
                                ("  name code target minpos slot mana beats dammsg wearoff guild minlevel\n\r",
                                 ch);
                        send_to_char
                                ("  type damtype acttype classtype powertype flag dice value difficulty affect\n\r",
                                 ch);
                        send_to_char
                                ("  rmaffect level adept hit miss die imm (char/vict/room)\n\r",
                                 ch);
                        send_to_char("  components teachers races\n\r", ch);
                        send_to_char
                                ("Affect having the fields: <location> <modfifier> [duration] [bitvector]\n\r",
                                 ch);
                        send_to_char
                                ("(See AFFECTTYPES for location, and AffectedBy for bitvector)\n\r",
                                 ch);
                }
                send_to_char("Skill being any skill or spell.\n\r", ch);
                return;
        }

        if (get_trust(ch) > LevelSubImplem
            && !str_cmp(arg1, "save") && !str_cmp(argument, "table"))
        {
                if (!str_cmp(arg2, "skill"))
                {
                        send_to_char("Saving skill table...\n\r", ch);
                        save_skill_table();
                        return;
                }
        }
        if (get_trust(ch) > LevelSubImplem
            && !str_cmp(arg1, "create") && (!str_cmp(arg2, "skill")))
        {
                struct skill_type *skill;
                sh_int    type = SkillUnknown;

                if (top_sn >= MaxSkill)
                {
                        ch_printf(ch,
                                  "The current top sn is %d, which is the maximum.  "
                                  "To add more skills,\n\rMAX_SKILL will have to be "
                                  "raised in mud.h, and the mud recompiled.\n\r",
                                  top_sn);
                        return;
                }
                CREATE(skill, struct skill_type, 1);
                skill_table[top_sn++] = skill;
                skill->name = str_dup(argument);
                skill->noun_damage = str_dup("");
                skill->msg_off = str_dup("");
                skill->spell_fun = spell_smaug;
                skill->type = type;
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (arg1[0] == 'h')
                sn = atoi(arg1 + 1);
        else
                sn = atoi(arg1);
        if (get_trust(ch) > LevelGreater
            &&
            ((arg1[0] == 'h' && is_number(arg1 + 1)
              && (sn = atoi(arg1 + 1)) >= 0) || (is_number(arg1)
                                                 && (sn = atoi(arg1)) >= 0)))
        {
                struct skill_type *skill;

                if ((skill = get_skilltype(sn)) == NULL)
                {
                        send_to_char("Skill number out of range.\n\r", ch);
                        return;
                }
                sn %= 1000;

                if (!str_cmp(arg2, "difficulty"))
                {
                        skill->difficulty = atoi(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "participants"))
                {
                        skill->participants = atoi(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "alignment"))
                {
                        skill->alignment = atoi(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "damtype"))
                {
                        int       x = get_sdamage(argument);

                        if (x == -1)
                                send_to_char("Not a spell damage type.\n\r",
                                             ch);
                        else
                        {
                                SetSdam(skill, x);
                                send_to_char("Ok.\n\r", ch);
                        }
                        return;
                }
                if (!str_cmp(arg2, "parts"))
                {
                        char      arg3[MSL];
                        int       value;

                        while (argument[0] != '\0')
                        {
                                argument = one_argument(argument, arg3);
                                value = get_partflag(arg3);
                                if (value < 0 || value > MaxBits)
                                        ch_printf(ch, "Unknown flag: %s\n\r",
                                                  arg3);
                                else
                                        xTOGGLE_BIT(skill->body_parts, value);
                        }
                        return;
                }
                if (!str_cmp(arg2, "acttype"))
                {
                        int       x = get_saction(argument);

                        if (x == -1)
                                send_to_char("Not a spell action type.\n\r",
                                             ch);
                        else
                        {
                                SetSact(skill, x);
                                send_to_char("Ok.\n\r", ch);
                        }
                        return;
                }
                if (!str_cmp(arg2, "classtype"))
                {
                        int       x = get_sclass(argument);

                        if (x == -1)
                                send_to_char("Not a spell class type.\n\r",
                                             ch);
                        else
                        {
                                SetScla(skill, x);
                                send_to_char("Ok.\n\r", ch);
                        }
                        return;
                }
                if (!str_cmp(arg2, "powertype"))
                {
                        int       x = get_spower(argument);

                        if (x == -1)
                                send_to_char("Not a spell power type.\n\r",
                                             ch);
                        else
                        {
                                SetSpow(skill, x);
                                send_to_char("Ok.\n\r", ch);
                        }
                        return;
                }
                if (!str_cmp(arg2, "flag"))
                {
                        int       x = get_sflag(argument);

                        if (x == -1)
                                send_to_char("Not a spell flag.\n\r", ch);
                        else
                        {
                                ToggleBit(skill->flags, 1 << (x + 11));
                                send_to_char("Ok.\n\r", ch);
                        }
                        return;
                }
                if (!str_cmp(arg2, "saves"))
                {
                        int       x = get_ssave(argument);

                        if (x == -1)
                                send_to_char("Not a saving type.\n\r", ch);
                        else
                        {
                                skill->saves = x;
                                send_to_char("Ok.\n\r", ch);
                        }
                        return;
                }

                if (!str_cmp(arg2, "code"))
                {
                        SpellFun *spellfun;
                        DoFun   *dofun;

                        if (!str_prefix("do_", argument)
                            && (dofun =
                                skill_function(argument)) != skill_notfound)
                        {
                                skill->skill_fun = dofun;
                                skill->spell_fun = NULL;
                                if (skill->skill_fun_name)
                                        DISPOSE(skill->skill_fun_name);
                                skill->skill_fun_name = str_dup(argument);
                        }
                        else if ((spellfun =
                                  spell_function(argument)) != spell_notfound)
                        {
                                skill->spell_fun = spellfun;
                                skill->skill_fun = NULL;
                                if (skill->skill_fun_name)
                                        DISPOSE(skill->skill_fun_name);
                                skill->spell_fun_name = str_dup(argument);
                        }
                        else if (validate_spec_fun(argument))
                        {
                                send_to_char
                                        ("Cannot use a spec_fun for skills or spells.\n\r",
                                         ch);
                                return;
                        }
                        else
                        {
                                send_to_char("Not a spell or skill.\n\r", ch);
                                return;
                        }
                        send_to_char("Ok.\n\r", ch);
                        return;
                }

                if (!str_cmp(arg2, "target"))
                {
                        int       x = get_starget(argument);

                        if (x == -1)
                                send_to_char("Not a Valid target type.\n\r",
                                             ch);
                        else
                        {
                                skill->target = x;
                                send_to_char("Ok.\n\r", ch);
                        }
                        return;
                }
                if (!str_cmp(arg2, "minpos"))
                {
                        skill->minimum_position =
                                URANGE(PosDead, atoi(argument), PosDrag);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "minlevel"))
                {
                        skill->min_level =
                                URANGE(1, atoi(argument), MaxLevel);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "slot"))
                {
                        skill->slot = URANGE(0, atoi(argument), 30000);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "endurance"))
                {
                        skill->min_endurance =
                                URANGE(0, atoi(argument), 2000);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "beats"))
                {
                        skill->beats = URANGE(0, atoi(argument), 120);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "guild"))
                {
                        skill->guild = atoi(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "value"))
                {
                        skill->value = atoi(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "type"))
                {
                        skill->type = get_skill(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "rmaffect"))
                {
                        SmaugAff *aff = skill->affects;
                        SmaugAff *aff_next;
                        int       num = atoi(argument);
                        int       cnt = 1;

                        if (!aff)
                        {
                                send_to_char
                                        ("This spell has no special affects to remove.\n\r",
                                         ch);
                                return;
                        }
                        if (num == 1)
                        {
                                skill->affects = aff->next;
                                DISPOSE(aff->duration);
                                DISPOSE(aff->modifier);
                                DISPOSE(aff);
                                send_to_char("Removed.\n\r", ch);
                                return;
                        }
                        for (; aff; aff = aff->next)
                        {
                                if (++cnt == num
                                    && (aff_next = aff->next) != NULL)
                                {
                                        aff->next = aff_next->next;
                                        DISPOSE(aff_next->duration);
                                        DISPOSE(aff_next->modifier);
                                        DISPOSE(aff_next);
                                        send_to_char("Removed.\n\r", ch);
                                        return;
                                }
                        }
                        send_to_char("Not found.\n\r", ch);
                        return;
                }
                /*
                 * affect <location> <modifier> <duration> <bitvector>
                 */
                if (!str_cmp(arg2, "affect"))
                {
                        char      location[MaxInputLength];
                        char      modifier[MaxInputLength];
                        char      duration[MaxInputLength];
                        char      bitvector[MaxInputLength];
                        int       loc, bit, tmpbit;
                        SmaugAff *aff;

                        argument = one_argument(argument, location);
                        argument = one_argument(argument, modifier);
                        argument = one_argument(argument, duration);

                        if (location[0] == '!')
                                loc = get_atype(location + 1) + ReverseApply;
                        else
                                loc = get_atype(location);
                        if ((loc % ReverseApply) < 0
                            || (loc % ReverseApply) >= MaxApplyType)
                        {
                                send_to_char
                                        ("Unknown affect location.  See AFFECTTYPES.\n\r",
                                         ch);
                                return;
                        }
                        bit = 0;
                        while (argument[0] != 0)
                        {
                                argument = one_argument(argument, bitvector);
                                if ((tmpbit = get_aflag(bitvector)) == -1)
                                        ch_printf(ch,
                                                  "Unknown bitvector: %s.  See AffectedBy\n\r",
                                                  bitvector);
                                else
                                        bit |= (1 << tmpbit);
                        }
                        CREATE(aff, SmaugAff, 1);
                        if (!str_cmp(duration, "0"))
                                duration[0] = '\0';
                        if (!str_cmp(modifier, "0"))
                                modifier[0] = '\0';
                        aff->duration = str_dup(duration);
                        aff->location = loc;
                        if (loc == ApplyAffect || loc == ApplyResistant
                            || loc == ApplyImmune
                            || loc == ApplySusceptible)
                        {
                                int       modval = get_aflag(modifier);

                                /*
                                 * Sanitize the flag input for the modifier if needed -- Samson 
                                 */
                                if (modval < 0)
                                        modval = 0;
                                snprintf(modifier, MaxInputLength, "%d",
                                         modval);
                        }
                        aff->modifier = str_dup(modifier);
                        aff->bitvector = bit;
                        aff->next = skill->affects;
                        skill->affects = aff;
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "level"))
                {
                        skill->min_level =
                                URANGE(1, atoi(argument), MaxLevel);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "held"))
                {
                        skill->held = URANGE(0, atoi(argument), 1);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "adept"))
                {
                        return;
                }
                if (!str_cmp(arg2, "name"))
                {
                        DISPOSE(skill->name);
                        skill->name = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "dammsg"))
                {
                        DISPOSE(skill->noun_damage);
                        if (!str_cmp(argument, "clear"))
                                skill->noun_damage = str_dup("");
                        else
                                skill->noun_damage = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "wearoff"))
                {
                        DISPOSE(skill->msg_off);
                        if (str_cmp(argument, "clear"))
                                skill->msg_off = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "hitchar"))
                {
                        if (skill->hit_char)
                                DISPOSE(skill->hit_char);
                        if (str_cmp(argument, "clear"))
                                skill->hit_char = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "hitvict"))
                {
                        if (skill->hit_vict)
                                DISPOSE(skill->hit_vict);
                        if (str_cmp(argument, "clear"))
                                skill->hit_vict = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "hitroom"))
                {
                        if (skill->hit_room)
                                DISPOSE(skill->hit_room);
                        if (str_cmp(argument, "clear"))
                                skill->hit_room = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "misschar"))
                {
                        if (skill->miss_char)
                                DISPOSE(skill->miss_char);
                        if (str_cmp(argument, "clear"))
                                skill->miss_char = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "missvict"))
                {
                        if (skill->miss_vict)
                                DISPOSE(skill->miss_vict);
                        if (str_cmp(argument, "clear"))
                                skill->miss_vict = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "missroom"))
                {
                        if (skill->miss_room)
                                DISPOSE(skill->miss_room);
                        if (str_cmp(argument, "clear"))
                                skill->miss_room = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "diechar"))
                {
                        if (skill->die_char)
                                DISPOSE(skill->die_char);
                        if (str_cmp(argument, "clear"))
                                skill->die_char = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "dievict"))
                {
                        if (skill->die_vict)
                                DISPOSE(skill->die_vict);
                        if (str_cmp(argument, "clear"))
                                skill->die_vict = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "dieroom"))
                {
                        if (skill->die_room)
                                DISPOSE(skill->die_room);
                        if (str_cmp(argument, "clear"))
                                skill->die_room = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "immchar"))
                {
                        if (skill->imm_char)
                                DISPOSE(skill->imm_char);
                        if (str_cmp(argument, "clear"))
                                skill->imm_char = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "immvict"))
                {
                        if (skill->imm_vict)
                                DISPOSE(skill->imm_vict);
                        if (str_cmp(argument, "clear"))
                                skill->imm_vict = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "immroom"))
                {
                        if (skill->imm_room)
                                DISPOSE(skill->imm_room);
                        if (str_cmp(argument, "clear"))
                                skill->imm_room = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "dice"))
                {
                        if (skill->dice)
                                DISPOSE(skill->dice);
                        if (str_cmp(argument, "clear"))
                                skill->dice = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "components"))
                {
                        if (skill->components)
                                DISPOSE(skill->components);
                        if (str_cmp(argument, "clear"))
                                skill->components = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "teachers"))
                {
                        if (skill->teachers)
                                DISPOSE(skill->teachers);
                        if (str_cmp(argument, "clear"))
                                skill->teachers = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "races"))
                {
                        if (skill->races)
                                DISPOSE(skill->races);
                        if (str_cmp(argument, "clear"))
                                skill->races = str_dup(argument);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg2, "delete"))
                {
                        int       x;

                        for (x = skill_lookup(skill->name); x < top_sn; x++)
                        {
                                skill_table[x] = skill_table[x + 1];
                                for (victim = first_char; victim;
                                     victim = victim->next)
                                {
                                        if (IsNpc(victim))
                                                continue;
                                        victim->pcdata->learned[x] =
                                                victim->pcdata->learned[x +
                                                                        1];
                                }
                        }
                        top_sn--;
                        skill->spell_fun = NULL;
                        skill->skill_fun = NULL;
                        if (skill->name)
                                DISPOSE(skill->name);
                        if (skill->noun_damage)
                                DISPOSE(skill->noun_damage);
                        if (skill->msg_off)
                                DISPOSE(skill->msg_off);
                        if (skill->hit_char)
                                DISPOSE(skill->hit_char);
                        if (skill->hit_vict)
                                DISPOSE(skill->hit_vict);
                        if (skill->hit_room)
                                DISPOSE(skill->hit_room);
                        if (skill->miss_char)
                                DISPOSE(skill->miss_char);
                        if (skill->miss_vict)
                                DISPOSE(skill->miss_vict);
                        if (skill->miss_room)
                                DISPOSE(skill->miss_room);
                        if (skill->die_char)
                                DISPOSE(skill->die_char);
                        if (skill->die_vict)
                                DISPOSE(skill->die_vict);
                        if (skill->die_room)
                                DISPOSE(skill->die_room);
                        if (skill->imm_char)
                                DISPOSE(skill->imm_char);
                        if (skill->imm_vict)
                                DISPOSE(skill->imm_vict);
                        if (skill->imm_room)
                                DISPOSE(skill->imm_room);
                        if (skill->dice)
                                DISPOSE(skill->dice);
                        if (skill->teachers)
                                DISPOSE(skill->teachers);
                        if (skill->races)
                                DISPOSE(skill->races);
                        if (skill->components)
                                DISPOSE(skill->components);
                        DISPOSE(skill);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                do_sset(ch, "");
                return;
        }

        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                if ((sn = skill_lookup(arg1)) >= 0)
                {
                        snprintf(arg1, MSL, "%d %s %s", sn, arg2, argument);
                        do_sset(ch, arg1);
                }
                else
                        send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IsNpc(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        fAll = !str_cmp(arg2, "all");
        sn = 0;
        if (!fAll && (sn = skill_lookup(arg2)) < 0)
        {
                send_to_char("No such skill or spell.\n\r", ch);
                return;
        }

        /*
         * Snarf the value.
         */
        if (!is_number(argument))
        {
                send_to_char("Value must be numeric.\n\r", ch);
                return;
        }

        value = atoi(argument);
        if (value < 0 || value > 100)
        {
                send_to_char("Value range is 0 to 100.\n\r", ch);
                return;
        }

        if (fAll)
        {
                for (sn = 0; sn < top_sn; sn++)
                {
                        /*
                         * Fix by Narn to prevent ssetting skills the player shouldn't have. 
                         */
                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MaxAbility)
                                continue;
                        if (skill_table[sn]->name
                            && (victim->skill_level[skill_table[sn]->guild] >=
                                skill_table[sn]->min_level || value == 0))
                                victim->pcdata->learned[sn] = value;
                }
        }
        else
                victim->pcdata->learned[sn] = value;

        return;
}


void learn_from_success(CharData * ch, int sn)
{
        int       adept, gain, sklvl, learn, percent, percent_chance;

        if (IsNpc(ch) || ch->pcdata->learned[sn] == 0)
                return;

        if (sn < 0)
        {
                bug("SN: %d is negative", sn);
                return;
        }
        if (sn == skill_lookup("meditate")
            && ch->skill_level[ForceAbility] < 2)
                gain_exp(ch, 25, ForceAbility);

        sklvl = skill_table[sn]->min_level;

        if (skill_table[sn]->guild < 0
            || skill_table[sn]->guild >= MaxAbility)
                return;

        adept = (ch->skill_level[skill_table[sn]->guild] -
                 skill_table[sn]->min_level) * 5 + 50;
        adept = UMIN(adept, 100);

        if (ch->pcdata->learned[sn] >= adept)
                return;

        if (sklvl == 0 || sklvl > ch->skill_level[skill_table[sn]->guild])
                sklvl = ch->skill_level[skill_table[sn]->guild];
        if (ch->pcdata->learned[sn] < 100)
        {
                percent_chance =
                        ch->pcdata->learned[sn] +
                        (5 * skill_table[sn]->difficulty);
                percent = number_percent();
                if (percent >= percent_chance)
                        learn = 2;
                else if (percent_chance - percent > 25)
                        return;
                else
                        learn = 1;
                ch->pcdata->learned[sn] =
                        UMIN(adept, ch->pcdata->learned[sn] + learn);
                if (ch->pcdata->learned[sn] == 100) /* fully learned! */
                {
                        gain = 50 * sklvl;
                        set_char_color(AtWhite, ch);
                        ch_printf(ch,
                                  "You are now an adept of %s!  You gain %d Bonus experience!\n\r",
                                  skill_table[sn]->name, gain);
                }
                else
                {
                        gain = 10 * sklvl;
                        if (!ch->fighting && sn != gsn_hide
                            && sn != gsn_sneak)
                        {
                                set_char_color(AtWhite, ch);
                                ch_printf(ch,
                                          "You gain %d experience points from your success!\n\r",
                                          gain);
                        }
                }
                gain_exp(ch, gain, skill_table[sn]->guild);
        }
}


void learn_from_failure(CharData * ch, int sn)
{
        ch = NULL;
        sn = 0;
}

CMDF do_gouge(CharData * ch, char *argument)
{
        CharData *victim;
        AffectData af;
        sh_int    dam;
        int       percent;

        argument = NULL;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (!IsNpc(ch) && !ch->pcdata->learned[gsn_gouge])
        {
                send_to_char("You do not yet know of this skill.\n\r", ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't get close enough while mounted.\n\r",
                             ch);
                return;
        }

        if ((victim = who_fighting(ch)) == NULL)
        {
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        percent = number_percent() - (get_curr_lck(ch) - 13);

        if (IsNpc(ch) || percent < ch->pcdata->learned[gsn_gouge])
        {
                dam = number_range(1, ch->skill_level[CombatAbility]);
                global_retcode = damage(ch, victim, dam, gsn_gouge);
                if (global_retcode == rNONE)
                {
                        if (!IsAffected(victim, AffBlind))
                        {
                                af.type = gsn_blindness;
                                af.location = ApplyHitroll;
                                af.modifier = -6;
                                af.duration =
                                        3 +
                                        (ch->skill_level[CombatAbility] /
                                         20);
                                af.bitvector = AffBlind;
                                affect_to_char(victim, &af);
                                act(AtSkill, "You can't see a thing!",
                                    victim, NULL, NULL, ToChar);
                        }
                        WaitState(ch, PulseViolence);
                        WaitState(victim, PulseViolence);
                        /*
                         * Taken out by Request - put back in by Thoric
                         * * This is how it was designed.  You'd be a tad stunned
                         * * if someone gouged you in the eye.
                         */
                }
                else if (global_retcode == rVICT_DIED)
                {
                        act(AtBlood,
                            "Your fingers plunge into your victim's brain, causing immediate death!",
                            ch, NULL, NULL, ToChar);
                }
                if (global_retcode != rCHAR_DIED
                    && global_retcode != rBOTH_DIED)
                        learn_from_success(ch, gsn_gouge);
        }
        else
        {
                WaitState(ch, skill_table[gsn_gouge]->beats);
                global_retcode = damage(ch, victim, 0, gsn_gouge);
                learn_from_failure(ch, gsn_gouge);
        }

        return;
}

CMDF do_detrap(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ObjData *trap;
        int       percent;
        bool      found = FALSE;

        switch (ch->substate)
        {
        default:
                if (IsNpc(ch) && IsAffected(ch, AffCharm))
                {
                        send_to_char
                                ("You can't concentrate enough for that.\n\r",
                                 ch);
                        return;
                }
                argument = one_argument(argument, arg);
                if (!IsNpc(ch) && !ch->pcdata->learned[gsn_detrap])
                {
                        send_to_char("You do not yet know of this skill.\n\r",
                                     ch);
                        return;
                }
                if (arg[0] == '\0')
                {
                        send_to_char("Detrap what?\n\r", ch);
                        return;
                }
                if (ms_find_obj(ch))
                        return;
                found = FALSE;
                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                if (!ch->in_room->first_content)
                {
                        send_to_char("You can't find that here.\n\r", ch);
                        return;
                }
                for (obj = ch->in_room->first_content; obj;
                     obj = obj->next_content)
                {
                        if (can_see_obj(ch, obj)
                            && nifty_is_name(arg, obj->name))
                        {
                                found = TRUE;
                                break;
                        }
                }
                if (!found)
                {
                        send_to_char("You can't find that here.\n\r", ch);
                        return;
                }
                act(AtAction,
                    "You carefully begin your attempt to remove a trap from $p...",
                    ch, obj, NULL, ToChar);
                act(AtAction,
                    "$n carefully attempts to remove a trap from $p...", ch,
                    obj, NULL, ToRoom);
                ch->dest_buf = str_dup(obj->name);
                add_timer(ch, TimerDoFun, 3, do_detrap, 1);
/*	    WaitState( ch, skill_table[gsn_detrap]->beats ); */
                return;
        case 1:
                if (!ch->dest_buf)
                {
                        send_to_char("Your detrapping was interrupted!\n\r",
                                     ch);
                        bug("do_detrap: ch->dest_buf NULL!", 0);
                        return;
                }
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                ch->dest_buf = NULL;
                ch->substate = SubNone;
                break;
        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char("You carefully stop what you were doing.\n\r",
                             ch);
                return;
        }

        if (!ch->in_room->first_content)
        {
                send_to_char("You can't find that here.\n\r", ch);
                return;
        }
        for (obj = ch->in_room->first_content; obj; obj = obj->next_content)
        {
                if (can_see_obj(ch, obj) && nifty_is_name(arg, obj->name))
                {
                        found = TRUE;
                        break;
                }
        }
        if (!found)
        {
                send_to_char("You can't find that here.\n\r", ch);
                return;
        }
        if ((trap = get_trap(obj)) == NULL)
        {
                send_to_char("You find no trap on that.\n\r", ch);
                return;
        }

        percent = number_percent() - (ch->skill_level[SmugglingAbility] / 20)
                - (get_curr_lck(ch) - 16);

        separate_obj(obj);
        if (!IsNpc(ch) || percent > ch->pcdata->learned[gsn_detrap])
        {
                send_to_char("Ooops!\n\r", ch);
                spring_trap(ch, trap);
                learn_from_failure(ch, gsn_detrap);
                return;
        }

        extract_obj(trap);

        send_to_char("You successfully remove a trap.\n\r", ch);
        learn_from_success(ch, gsn_detrap);
        return;
}

CMDF do_dig(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ObjData *startobj;
        bool      found, shovel;
        ExitData *pexit;

        switch (ch->substate)
        {
        default:
                if (IsNpc(ch) && IsAffected(ch, AffCharm))
                {
                        send_to_char
                                ("You can't concentrate enough for that.\n\r",
                                 ch);
                        return;
                }
                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                one_argument(argument, arg);
                if (arg[0] != '\0')
                {
                        if ((pexit = find_door(ch, arg, TRUE)) == NULL
                            && get_dir(arg) == -1)
                        {
                                send_to_char("What direction is that?\n\r",
                                             ch);
                                return;
                        }
                        if (pexit)
                        {
                                if (!IsSet(pexit->exit_info, ExDig)
                                    && !IsSet(pexit->exit_info, ExClosed))
                                {
                                        send_to_char
                                                ("There is no need to dig out that exit.\n\r",
                                                 ch);
                                        return;
                                }
                        }
                }
                else
                {
                        switch (ch->in_room->sector_type)
                        {
                        case SectCity:
                        case SectInside:
                                send_to_char
                                        ("The floor is too hard to dig through.\n\r",
                                         ch);
                                return;
                        case SectWaterSwim:
                        case SectWaterNoswim:
                        case SectUnderwater:
                                send_to_char("You cannot dig here.\n\r", ch);
                                return;
                        case SectAir:
                                send_to_char("What?  In the air?!\n\r", ch);
                                return;
                        }
                }
                add_timer(ch, TimerDoFun,
                          UMIN(skill_table[gsn_dig]->beats / 10, 3), do_dig,
                          1);
                ch->dest_buf = str_dup(arg);
                send_to_char("You begin digging...\n\r", ch);
                act(AtPlain, "$n begins digging...", ch, NULL, NULL,
                    ToRoom);
                return;

        case 1:
                if (!ch->dest_buf)
                {
                        send_to_char("Your digging was interrupted!\n\r", ch);
                        act(AtPlain, "$n's digging was interrupted!", ch,
                            NULL, NULL, ToRoom);
                        bug("do_dig: dest_buf NULL", 0);
                        return;
                }
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char("You stop digging...\n\r", ch);
                act(AtPlain, "$n stops digging...", ch, NULL, NULL, ToRoom);
                return;
        }

        ch->substate = SubNone;

        /*
         * not having a shovel makes it harder to succeed 
         */
        shovel = FALSE;
        for (obj = ch->first_carrying; obj; obj = obj->next_content)
                if (obj->item_type == ItemShovel)
                {
                        shovel = TRUE;
                        break;
                }

        /*
         * dig out an ExDig exit... 
         */
        if (arg[0] != '\0')
        {
                if ((pexit = find_door(ch, arg, TRUE)) != NULL
                    && IsSet(pexit->exit_info, ExDig)
                    && IsSet(pexit->exit_info, ExClosed))
                {
                        /*
                         * 4 times harder to dig open a passage without a shovel 
                         */
                        if ((number_percent() * (shovel ? 1 : 4)) <
                            (IsNpc(ch) ? 80 : ch->pcdata->learned[gsn_dig]))
                        {
                                RemoveBit(pexit->exit_info, ExClosed);
                                send_to_char("You dig open a passageway!\n\r",
                                             ch);
                                act(AtPlain, "$n digs open a passageway!",
                                    ch, NULL, NULL, ToRoom);
                                learn_from_success(ch, gsn_dig);
                                return;
                        }
                }
                learn_from_failure(ch, gsn_dig);
                send_to_char("Your dig did not discover any exit...\n\r", ch);
                act(AtPlain, "$n's dig did not discover any exit...", ch,
                    NULL, NULL, ToRoom);
                return;
        }

        startobj = ch->in_room->first_content;
        found = FALSE;

        for (obj = startobj; obj; obj = obj->next_content)
        {
                /*
                 * twice as hard to find something without a shovel 
                 */
                if (IsObjStat(obj, ItemBurried)
                    && (number_percent() * (shovel ? 1 : 2)) <
                    (IsNpc(ch) ? 80 : ch->pcdata->learned[gsn_dig]))
                {
                        found = TRUE;
                        break;
                }
        }

        if (!found)
        {
                send_to_char("Your dig uncovered nothing.\n\r", ch);
                act(AtPlain, "$n's dig uncovered nothing.", ch, NULL, NULL,
                    ToRoom);
                learn_from_failure(ch, gsn_dig);
                return;
        }

        separate_obj(obj);
        RemoveBit(obj->extra_flags, ItemBurried);
        act(AtSkill, "Your dig uncovered $p!", ch, obj, NULL, ToChar);
        act(AtSkill, "$n's dig uncovered $p!", ch, obj, NULL, ToRoom);
        learn_from_success(ch, gsn_dig);

        return;
}


CMDF do_search(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj = NULL;
        ObjData *container;
        ObjData *startobj;
        int       percent, door;
        bool      found, room;

        door = -1;
        switch (ch->substate)
        {
        default:
                if (IsNpc(ch) && IsAffected(ch, AffCharm))
                {
                        send_to_char
                                ("You can't concentrate enough for that.\n\r",
                                 ch);
                        return;
                }
                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }
                argument = one_argument(argument, arg);
                if (arg[0] != '\0' && (door = get_door(arg)) == -1)
                {
                        container = get_obj_here(ch, arg);
                        if (!container)
                        {
                                send_to_char("You can't find that here.\n\r",
                                             ch);
                                return;
                        }
                        if (container->item_type != ItemContainer)
                        {
                                send_to_char("You can't search in that!\n\r",
                                             ch);
                                return;
                        }
                        if (IsSet(container->value[1], ContClosed))
                        {
                                send_to_char("It is closed.\n\r", ch);
                                return;
                        }
                }
                add_timer(ch, TimerDoFun,
                          UMIN(skill_table[gsn_search]->beats / 10, 3),
                          do_search, 1);
                send_to_char("You begin your search...\n\r", ch);
                ch->dest_buf = str_dup(arg);
                return;

        case 1:
                if (!ch->dest_buf)
                {
                        send_to_char("Your search was interrupted!\n\r", ch);
                        bug("do_search: dest_buf NULL", 0);
                        return;
                }
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;
        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                send_to_char("You stop your search...\n\r", ch);
                return;
        }
        ch->substate = SubNone;
        if (arg[0] == '\0')
        {
                room = TRUE;
                startobj = ch->in_room->first_content;
        }
        else
        {
                if ((door = get_door(arg)) != -1)
                        startobj = NULL;
                else
                {
                        container = get_obj_here(ch, arg);
                        if (!container)
                        {
                                send_to_char("You can't find that here.\n\r",
                                             ch);
                                return;
                        }
                        startobj = container->first_content;
                }
        }

        found = FALSE;

        if ((!startobj && door == -1) || IsNpc(ch))
        {
                send_to_char("You find nothing.\n\r", ch);
                learn_from_failure(ch, gsn_search);
                return;
        }

        percent = number_percent();

        if (door != -1)
        {
                ExitData *pexit;

                if ((pexit = get_exit(ch->in_room, door)) != NULL
                    && IsSet(pexit->exit_info, ExSecret)
                    && IsSet(pexit->exit_info, EX_xSEARCHABLE)
                    && percent <
                    (IsNpc(ch) ? 80 : ch->pcdata->learned[gsn_search]))
                {
                        act(AtSkill, "Your search reveals the $d!", ch, NULL,
                            pexit->keyword, ToChar);
                        act(AtSkill, "$n finds the $d!", ch, NULL,
                            pexit->keyword, ToRoom);
                        RemoveBit(pexit->exit_info, ExSecret);
                        learn_from_success(ch, gsn_search);
                        return;
                }
        }
        else
                for (obj = startobj; obj; obj = obj->next_content)
                {
                        if (IsObjStat(obj, ItemHidden)
                            && percent < ch->pcdata->learned[gsn_search])
                        {
                                found = TRUE;
                                break;
                        }
                }

        if (!found)
        {
                send_to_char("You find nothing.\n\r", ch);
                learn_from_failure(ch, gsn_search);
                return;
        }

        separate_obj(obj);
        RemoveBit(obj->extra_flags, ItemHidden);
        act(AtSkill, "Your search reveals $p!", ch, obj, NULL, ToChar);
        act(AtSkill, "$n finds $p!", ch, obj, NULL, ToRoom);
        learn_from_success(ch, gsn_search);
        return;
}


CMDF do_steal(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        CharData *victim, *mst;
        ObjData *obj, *obj_next;
        int       percent, xp;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Steal what from whom?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if ((victim = get_char_room(ch, arg2)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomSafe))
        {
                set_char_color(AtMagic, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (check_illegal_psteal(ch, victim))
        {
                send_to_char("You can't steal from that player.\n\r", ch);
                return;
        }

        if (victim->owner != NULL)
        {
                send_to_char("You cannot steal from them!\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_steal]->beats);
        percent = number_percent() + (IsAwake(victim) ? 10 : -50)
                - (get_curr_lck(ch) - 15) + (get_curr_lck(victim) - 13);

        if (victim->position == PosFighting
            || percent > (IsNpc(ch) ? 90 : ch->pcdata->learned[gsn_steal]))
        {
                /*
                 * Failure.
                 */
                send_to_char("Oops...\n\r", ch);
                act(AtAction, "$n tried to steal from you!\n\r", ch, NULL,
                    victim, ToVict);
                act(AtAction, "$n tried to steal from $N.\n\r", ch, NULL,
                    victim, ToNotvict);
                command_printf(victim, "yell %s is a bloody thief!",
                               ch->name);

                learn_from_failure(ch, gsn_steal);
                if (!IsNpc(ch))
                {
                        if (legal_loot(ch, victim))
                        {
                                global_retcode =
                                        multi_hit(victim, ch, TypeUndefined);
                        }
                        else
                        {
                                /*
                                 * log_string( buf ); 
                                 */
                                if (IsNpc(ch))
                                {
                                        if ((mst = ch->master) == NULL)
                                                return;
                                }
                                else
                                        mst = ch;
                                if (IsNpc(mst))
                                        return;

                        }
                }

                return;
        }

        if (IsNpc(victim))
                add_kill(ch, victim);   /* makes it harder to steal from same char */

        if (!str_cmp(arg1, "credits")
            || !str_cmp(arg1, "credit") || !str_cmp(arg1, "money"))
        {
                int       amount;

                amount = (int) (victim->gold * number_range(1, 10) / 100);
                if (amount <= 0)
                {
                        send_to_char("You couldn't get any credits.\n\r", ch);
                        learn_from_failure(ch, gsn_steal);
                        return;
                }

                ch->gold += amount;
                victim->gold -= amount;
                ch_printf(ch, "Aha!  You got %d credits.\n\r", amount);
                learn_from_success(ch, gsn_steal);
                if (IsNpc(victim))
                {
                        xp = UMIN(amount * 100,
                                  (exp_level
                                   (ch->skill_level[PiracyAbility] + 1) -
                                   exp_level(ch->
                                             skill_level[PiracyAbility])) /
                                  25);
                        xp = UMIN(xp, xp_compute(ch, victim));
                        gain_exp(ch, xp, PiracyAbility);
                        ch_printf(ch, "&WYou gain %ld piracy experience!\n\r",
                                  xp);
                }
                return;
        }

        if ((obj = get_obj_carry(victim, arg1)) == NULL)
        {
                if (victim->position <= PosSleeping)
                {
                        if ((obj = get_obj_wear(victim, arg1)) != NULL)
                        {
                                if ((obj_next =
                                     get_eq_char(victim,
                                                 obj->wear_loc)) != obj)
                                {
                                        ch_printf(ch,
                                                  "They are wearing %s on top of %s.\n\r",
                                                  obj_next->short_descr,
                                                  obj->short_descr);
                                        send_to_char
                                                ("You'll have to steal that first.\n\r",
                                                 ch);
                                        learn_from_failure(ch, gsn_steal);
                                        return;
                                }
                                else
                                        unequip_char(victim, obj);
                        }
                }

                send_to_char("You can't seem to find it.\n\r", ch);
                learn_from_failure(ch, gsn_steal);
                return;
        }

        if (!can_drop_obj(ch, obj)
            || IsObjStat(obj, ItemInventory)
            || IsObjStat(obj, ItemPrototype))
        {
                send_to_char("You can't manage to pry it away.\n\r", ch);
                learn_from_failure(ch, gsn_steal);
                return;
        }

        if (ch->carry_number + (get_obj_number(obj) / obj->count) >
            can_carry_n(ch))
        {
                send_to_char("You have your hands full.\n\r", ch);
                learn_from_failure(ch, gsn_steal);
                return;
        }

        if (ch->carry_weight + (get_obj_weight(obj) / obj->count) >
            can_carry_w(ch))
        {
                send_to_char("You can't carry that much weight.\n\r", ch);
                learn_from_failure(ch, gsn_steal);
                return;
        }

        send_to_char("Ok.\n\r", ch);
        learn_from_success(ch, gsn_steal);
        if (IsNpc(victim))
        {
                xp = UMIN(obj->cost * 10 + 1000,
                          (exp_level(ch->skill_level[PiracyAbility] + 1) -
                           exp_level(ch->skill_level[PiracyAbility])) / 10);
                xp = UMIN(xp, xp_compute(ch, victim));
                gain_exp(ch, xp, PiracyAbility);
                ch_printf(ch, "&WYou gain %ld piracy experience!\n\r", xp);
        }
        separate_obj(obj);
        obj_from_char(obj);
        obj_to_char(obj, ch);

        return;
}


CMDF do_backstab(CharData * ch, char *argument)
{
	char      arg[MaxInputLength];
	CharData *victim;
	ObjData *obj;
	int       percent;

	if (IsNpc(ch) && IsAffected(ch, AffCharm))
	{
		send_to_char("You can't do that right now.\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (ch->mount)
	{
		send_to_char("You can't get close enough while mounted.\n\r",
				ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Backstab whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("How can you sneak up on yourself?\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	/*
	 * Added stabbing weapon. -Narn 
	 */
	obj = get_eq_char(ch, WearWield);
	if (!obj || obj->item_type != ItemWeapon
			|| obj->value[3] != WeaponKnife)
	{
		obj = get_eq_char(ch, WearDualWield);
                if ((!obj || (obj->item_type != ItemWeapon &&
                                                obj->value[3] !=WeaponKnife)) &&
                                !IsNpc(ch) && IsSet(ch->pcdata->flags, PcflagAutodraw))
                {
                        ObjData *holster1 = get_eq_char(ch, WearHolsterL),
                                 *holster2 = get_eq_char(ch, WearHolsterR);
                        if (!obj && holster1 && holster1->first_content) {
                                if (holster1->first_content->item_type == ItemWeapon &&
                                                holster1->first_content->value[3] == WeaponKnife)
                                        obj = holster1->first_content; 
                        }
                        if (!obj && holster2 && holster2->first_content) {
                                if (holster2->first_content->item_type == ItemWeapon &&
                                                holster2->first_content->value[3] == WeaponKnife)
                                        obj = holster2->first_content; 
                        }
                }
		if (!obj || obj->item_type != ItemWeapon ||
				obj->value[3] != WeaponKnife)
		{
			send_to_char
				("You need to wield a stabbing weapon.\n\r",
				 ch);
			return;
		}
	}

	if (victim->fighting)
	{
		send_to_char
			("You can't backstab someone who is in combat.\n\r",
			 ch);
		return;
	}

	/*
	 * Can backstab a char even if it's hurt as long as it's sleeping. -Narn 
	 */
	if (victim->hit < victim->max_hit && IsAwake(victim))
	{
		act(AtPlain,
				"$N is hurt and suspicious ... you can't sneak up.", ch,
				NULL, victim, ToChar);
		return;
	}

	percent = number_percent() - (get_curr_lck(ch) - 14)
		+ (get_curr_lck(victim) - 13);

	WaitState(ch, skill_table[gsn_backstab]->beats);
	if (!IsAwake(victim)
			|| IsNpc(ch) || percent < ch->pcdata->learned[gsn_backstab])
	{
		learn_from_success(ch, gsn_backstab);
		global_retcode = multi_hit(ch, victim, gsn_backstab);

	}
	else
	{
		learn_from_failure(ch, gsn_backstab);
		global_retcode = damage(ch, victim, 0, gsn_backstab);
	}
	return;
}


CMDF do_rescue(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        CharData *fch;
        int       percent;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Rescue whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char
                        ("You try and rescue yourself, but fail miserably.\n\r",
                         ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (!IsNpc(ch) && IsNpc(victim))
        {
                send_to_char("Doesn't need your help!\n\r", ch);
                return;
        }

        if ((fch = who_fighting(victim)) == NULL)
        {
                send_to_char("They are not fighting right now.\n\r", ch);
                return;
        }

        ch->alignment = ch->alignment + 5;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);

        percent = number_percent() - (get_curr_lck(ch) - 14)
                - (get_curr_lck(victim) - 16);

        WaitState(ch, skill_table[gsn_rescue]->beats);
        if (!IsNpc(ch) && percent > ch->pcdata->learned[gsn_rescue])
        {
                send_to_char("You fail the rescue.\n\r", ch);
                act(AtSkill, "$n tries to rescue you!", ch, NULL, victim,
                    ToVict);
                act(AtSkill, "$n tries to rescue $N!", ch, NULL, victim,
                    ToNotvict);
                learn_from_failure(ch, gsn_rescue);
                return;
        }

        act(AtSkill, "You rescue $N!", ch, NULL, victim, ToChar);
        act(AtSkill, "$n rescues you!", ch, NULL, victim, ToVict);
        act(AtSkill, "$n moves in front of $N!", ch, NULL, victim,
            ToNotvict);

        ch->alignment = ch->alignment + 50;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);

        learn_from_success(ch, gsn_rescue);
        stop_fighting(fch, FALSE);
        stop_fighting(victim, FALSE);
        if (ch->fighting)
                stop_fighting(ch, FALSE);

        /*
         * check_killer( ch, fch ); 
         */
        set_fighting(ch, fch);
        set_fighting(fch, ch);
        return;
}



CMDF do_kick(CharData * ch, char *argument)
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

        WaitState(ch, skill_table[gsn_kick]->beats);
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_kick])
        {
                learn_from_success(ch, gsn_kick);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_kick);
        }
        else
        {
                learn_from_failure(ch, gsn_kick);
                global_retcode = damage(ch, victim, 0, gsn_kick);
        }
        return;
}

CMDF do_punch(CharData * ch, char *argument)
{
        CharData *victim;

        argument = NULL;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (!IsNpc(ch) && ch->pcdata->learned[gsn_punch] <= 0)
        {
                send_to_char
                        ("Your mind races as you realize you have no idea how to do that.\n\r",
                         ch);
                return;
        }

        if ((victim = who_fighting(ch)) == NULL)
        {
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_punch]->beats);
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_punch])
        {
                learn_from_success(ch, gsn_punch);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_punch);
        }
        else
        {
                learn_from_failure(ch, gsn_punch);
                global_retcode = damage(ch, victim, 0, gsn_punch);
        }
        return;
}


CMDF do_bite(CharData * ch, char *argument)
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
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_bite])
        {
                learn_from_success(ch, gsn_bite);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_bite);
        }
        else
        {
                learn_from_failure(ch, gsn_bite);
                global_retcode = damage(ch, victim, 0, gsn_bite);
        }
        return;
}


CMDF do_claw(CharData * ch, char *argument)
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
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_claw])
        {
                learn_from_success(ch, gsn_claw);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_claw);
        }
        else
        {
                learn_from_failure(ch, gsn_claw);
                global_retcode = damage(ch, victim, 0, gsn_claw);
        }
        return;
}


CMDF do_sting(CharData * ch, char *argument)
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
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_sting])
        {
                learn_from_success(ch, gsn_sting);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_sting);
        }
        else
        {
                learn_from_failure(ch, gsn_sting);
                global_retcode = damage(ch, victim, 0, gsn_sting);
        }
        return;
}


CMDF do_tail(CharData * ch, char *argument)
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
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_tail])
        {
                learn_from_success(ch, gsn_tail);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_tail);
        }
        else
        {
                learn_from_failure(ch, gsn_tail);
                global_retcode = damage(ch, victim, 0, gsn_tail);
        }
        return;
}


CMDF do_bash(CharData * ch, char *argument)
{
        CharData *victim;
        int       percent_chance;

        argument = NULL;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (!IsNpc(ch) && ch->pcdata->learned[gsn_bash] <= 0)
        {
                send_to_char
                        ("Your mind races as you realize you have no idea how to do that.\n\r",
                         ch);
                return;
        }

        if ((victim = who_fighting(ch)) == NULL)
        {
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        percent_chance = (((get_curr_dex(victim) + get_curr_str(victim))
                           - (get_curr_dex(ch) + get_curr_str(ch))) * 10) +
                10;
        if (!IsNpc(ch) && !IsNpc(victim))
                percent_chance += 25;
        if (victim->fighting && victim->fighting->who != ch)
                percent_chance += 19;
        WaitState(ch, skill_table[gsn_bash]->beats);
        if (IsNpc(ch)
            || (number_percent() + percent_chance) <
            ch->pcdata->learned[gsn_bash])
        {
                learn_from_success(ch, gsn_bash);
                /*
                 * do not change anything here!  -Thoric 
                 */
                WaitState(ch, 2 * PulseViolence);
                WaitState(victim, 2 * PulseViolence);
                victim->position = PosSitting;
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[CombatAbility]),
                               gsn_bash);
        }
        else
        {
                WaitState(ch, 2 * PulseViolence);
                learn_from_failure(ch, gsn_bash);
                global_retcode = damage(ch, victim, 0, gsn_bash);
        }
        return;
}


CMDF do_stun(CharData * ch, char *argument)
{
        CharData *victim;
        AffectData af;
        int       percent_chance;
        bool      fail;

        argument = NULL;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (!IsNpc(ch) && ch->pcdata->learned[gsn_stun] <= 0)
        {
                send_to_char
                        ("Your mind races as you realize you have no idea how to do that.\n\r",
                         ch);
                return;
        }

        if ((victim = who_fighting(ch)) == NULL)
        {
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        if (ch->endurance < 16)
        {
                set_char_color(AtSkill, ch);
                send_to_char("You are far too tired to do that.\n\r", ch);
                return; /* missing return fixed March 11/96 */
        }

        WaitState(ch, skill_table[gsn_stun]->beats);
        fail = FALSE;
        percent_chance =
                ris_save(victim, ch->skill_level[CombatAbility],
                         RisParalysis);
        if (percent_chance == 1000)
                fail = TRUE;
        else
                fail = saves_para_petri(percent_chance, victim);

        percent_chance = (((get_curr_dex(victim) + get_curr_str(victim))
                           - (get_curr_dex(ch) + get_curr_str(ch))) * 10) +
                10;
        /*
         * harder for player to stun another player 
         */
        if (!IsNpc(ch) && !IsNpc(victim))
                percent_chance += sysdata.stun_plr_vs_plr;
        else
                percent_chance += sysdata.stun_regular;
        if (!fail
            && (IsNpc(ch)
                || (number_percent() + percent_chance) <
                ch->pcdata->learned[gsn_stun]))
        {
                learn_from_success(ch, gsn_stun);
                /*
                 * DO *NOT* CHANGE!    -Thoric    
                 */
                ch->endurance -= 15;
                WaitState(ch, 2 * PulseViolence);
                WaitState(victim, PulseViolence);
                act(AtSkill, "$N smashes into you, leaving you stunned!",
                    victim, NULL, ch, ToChar);
                act(AtSkill, "You smash into $N, leaving $M stunned!", ch,
                    NULL, victim, ToChar);
                act(AtSkill, "$n smashes into $N, leaving $M stunned!", ch,
                    NULL, victim, ToNotvict);
                if (!IsAffected(victim, AffParalysis))
                {
                        af.type = gsn_stun;
                        af.location = ApplyAc;
                        af.modifier = 20;
                        af.duration = 3;
                        af.bitvector = AffParalysis;
                        affect_to_char(victim, &af);
                        update_pos(victim);
                }
        }
        else
        {
                WaitState(ch, 2 * PulseViolence);
                ch->endurance -= 5;
                learn_from_failure(ch, gsn_stun);
                act(AtSkill,
                    "$N charges at you screaming, but you dodge out of the way.",
                    victim, NULL, ch, ToChar);
                act(AtSkill,
                    "Your attempt to stun $N leaves you racing past $E as $e laughs.",
                    ch, NULL, victim, ToChar);
                act(AtSkill,
                    "$n charges screaming at $N, but keeps going right on past.",
                    ch, NULL, victim, ToNotvict);
        }
        return;
}




/*
 * Disarm a creature.
 * Caller must check for successful attack.
 * Check for loyalty flag (weapon disarms to inventory) for pkillers -Blodkai
 */
void disarm(CharData * ch, CharData * victim)
{
        ObjData *obj, *tmpobj;

        if ((obj = get_eq_char(victim, WearWield)) == NULL)
                return;

        if ((tmpobj = get_eq_char(victim, WearDualWield)) != NULL
            && number_bits(1) == 0)
                obj = tmpobj;

        if (get_eq_char(ch, WearWield) == NULL && number_bits(1) == 0)
        {
                learn_from_failure(ch, gsn_disarm);
                return;
        }

        if (IsNpc(ch) && !can_see_obj(ch, obj) && number_bits(1) == 0)
        {
                learn_from_failure(ch, gsn_disarm);
                return;
        }

        if (check_grip(ch, victim))
        {
                learn_from_failure(ch, gsn_disarm);
                return;
        }

        act(AtSkill, "$n DISARMS you!", ch, NULL, victim, ToVict);
        act(AtSkill, "You disarm $N!", ch, NULL, victim, ToChar);
        act(AtSkill, "$n disarms $N!", ch, NULL, victim, ToNotvict);
        learn_from_success(ch, gsn_disarm);

        if (obj == get_eq_char(victim, WearWield)
            && (tmpobj = get_eq_char(victim, WearDualWield)) != NULL)
                tmpobj->wear_loc = WearWield;

        obj_from_char(obj);
        if (in_arena(ch)) 
                obj_to_char(obj, victim);
        else
                obj_to_room(obj, victim->in_room);

        return;
}


CMDF do_disarm(CharData * ch, char *argument)
{
        CharData *victim;
        ObjData *obj;
        int       percent;

        argument = NULL;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (!IsNpc(ch) && ch->pcdata->learned[gsn_disarm] <= 0)
        {
                send_to_char("You don't know how to disarm opponents.\n\r",
                             ch);
                return;
        }

        if (get_eq_char(ch, WearWield) == NULL)
        {
                send_to_char("You must wield a weapon to disarm.\n\r", ch);
                return;
        }

        if ((victim = who_fighting(ch)) == NULL)
        {
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        if ((obj = get_eq_char(victim, WearWield)) == NULL)
        {
                send_to_char("Your opponent is not wielding a weapon.\n\r",
                             ch);
                return;
        }

        WaitState(ch, skill_table[gsn_disarm]->beats);
        percent =
                number_percent() + victim->skill_level[CombatAbility] -
                ch->skill_level[CombatAbility] - (get_curr_lck(ch) - 15) +
                (get_curr_lck(victim) - 15);
        if (!can_see_obj(ch, obj))
                percent += 10;
        if (IsNpc(ch) || percent < ch->pcdata->learned[gsn_disarm] * 2 / 3)
                disarm(ch, victim);
        else
        {
                send_to_char("You failed.\n\r", ch);
                learn_from_failure(ch, gsn_disarm);
        }
        return;
}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip(CharData * ch, CharData * victim)
{
        if (IsAffected(victim, AffFlying)
            || IsAffected(victim, AffFloating))
                return;
        if (victim->mount)
        {
                if (IsAffected(victim->mount, AffFlying)
                    || IsAffected(victim->mount, AffFloating))
                        return;
                act(AtSkill, "$n trips your mount and you fall off!", ch,
                    NULL, victim, ToVict);
                act(AtSkill, "You trip $N's mount and $N falls off!", ch,
                    NULL, victim, ToChar);
                act(AtSkill, "$n trips $N's mount and $N falls off!", ch,
                    NULL, victim, ToNotvict);
                RemoveBit(victim->mount->act, ActMounted);
                victim->mount = NULL;
                WaitState(ch, 2 * PulseViolence);
                WaitState(victim, 2 * PulseViolence);
                victim->position = PosResting;
                return;
        }
        if (victim->wait == 0)
        {
                act(AtSkill, "$n trips you and you go down!", ch, NULL,
                    victim, ToVict);
                act(AtSkill, "You trip $N and $N goes down!", ch, NULL,
                    victim, ToChar);
                act(AtSkill, "$n trips $N and $N goes down!", ch, NULL,
                    victim, ToNotvict);

                WaitState(ch, 2 * PulseViolence);
                WaitState(victim, 2 * PulseViolence);
                victim->position = PosResting;
        }

        return;
}


CMDF do_pick(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *gch;
        ObjData *obj;
        ExitData *pexit;
        ShipData *ship;

        if (IsNpc(ch))
        {
                return;
        }

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Pick what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_pick_lock]->beats);

        /*
         * look for guards 
         */
        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                if (IsNpc(gch) && IsAwake(gch)
                    && ch->skill_level[PiracyAbility] < gch->top_level)
                {
                        act(AtPlain, "$N is standing too close to the lock.",
                            ch, NULL, gch, ToChar);
                        return;
                }
        }


        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'pick door' 
                 */
                /*
                 * RoomIndexData *to_room; 
                 *//*
                 * Unused 
                 */
                ExitData *pexit_rev;

                if (!IsSet(pexit->exit_info, ExClosed))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (pexit->key < 0)
                {
                        send_to_char("It can't be picked.\n\r", ch);
                        return;
                }
                if (!IsSet(pexit->exit_info, ExLocked))
                {
                        send_to_char("It's already unlocked.\n\r", ch);
                        return;
                }
                if (IsSet(pexit->exit_info, ExPickproof))
                {
                        send_to_char("You failed.\n\r", ch);
                        learn_from_failure(ch, gsn_pick_lock);
                        check_room_for_traps(ch,
                                             TrapPick | trap_door[pexit->
                                                                   vdir]);
                        return;
                }

                if (!IsNpc(ch)
                    && number_percent() > ch->pcdata->learned[gsn_pick_lock])
                {
                        send_to_char("You failed.\n\r", ch);
                        learn_from_failure(ch, gsn_pick_lock);
                        return;
                }

                RemoveBit(pexit->exit_info, ExLocked);
                send_to_char("*Click*\n\r", ch);
                act(AtAction, "$n picks the $d.", ch, NULL, pexit->keyword,
                    ToRoom);
                learn_from_success(ch, gsn_pick_lock);
                {
                        long      xpgain;

                        xpgain = URANGE(500,
                                        (number_percent() *
                                         (ch->skill_level[PiracyAbility] *
                                          4)), 15000);
                        gain_exp(ch, xpgain, PiracyAbility);
                        ch_printf(ch, "You gain %d piracy experience.",
                                  xpgain);
                }
                /*
                 * pick the other side 
                 */
                if ((pexit_rev = pexit->rexit) != NULL
                    && pexit_rev->to_room == ch->in_room)
                {
                        RemoveBit(pexit_rev->exit_info, ExLocked);
                }
                check_room_for_traps(ch, TrapPick | trap_door[pexit->vdir]);
                return;
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                if (obj->item_type != ItemContainer)
                {
                        send_to_char("You can't pick that.\n\r", ch);
                        return;
                }
                if (!IsSet(obj->value[1], ContClosed))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (obj->value[2] < 0)
                {
                        send_to_char("It can't be unlocked.\n\r", ch);
                        return;
                }
                if (!IsSet(obj->value[1], ContLocked))
                {
                        send_to_char("It's already unlocked.\n\r", ch);
                        return;
                }
                if (IsSet(obj->value[1], ContPickproof))
                {
                        send_to_char("You failed.\n\r", ch);
                        learn_from_failure(ch, gsn_pick_lock);
                        check_for_trap(ch, obj, TrapPick);
                        return;
                }

                if (!IsNpc(ch)
                    && number_percent() > ch->pcdata->learned[gsn_pick_lock])
                {
                        send_to_char("You failed.\n\r", ch);
                        learn_from_failure(ch, gsn_pick_lock);
                        return;
                }

                separate_obj(obj);
                RemoveBit(obj->value[1], ContLocked);
                send_to_char("*Click*\n\r", ch);
                act(AtAction, "$n picks $p.", ch, obj, NULL, ToRoom);
                learn_from_success(ch, gsn_pick_lock);
                {
                        long      xpgain;

                        xpgain = UMIN(20,
                                      (exp_level
                                       (ch->skill_level[PiracyAbility] + 1) -
                                       exp_level(ch->
                                                 skill_level
                                                 [PiracyAbility])));
                        gain_exp(ch, xpgain, PiracyAbility);
                        ch_printf(ch, "You gain %d piracy experience.",
                                  xpgain);
                }

                check_for_trap(ch, obj, TrapPick);
                return;
        }

        if ((ship = ship_in_room(ch->in_room, arg)) != NULL)
        {
                char      buf[MaxStringLength];

                if (IsNpc(ch) || ch->pcdata->learned[gsn_pickshiplock] == 0)
                {
                        send_to_char("Huh?\n\r", ch);
                        return;
                }
                if (check_pilot(ch, ship))
                {
                        send_to_char("&RWhat would be the point of that!\n\r",
                                     ch);
                        return;
                }

                if (ship->shipstate != ShipDocked
                    && ship->shipstate != ShipDisabled)
                {
                        send_to_char
                                ("&RThat ship has already started to launch",
                                 ch);
                        return;
                }

                WaitState(ch, 10);

                if (IsNpc(ch) || !ch->pcdata
                    || (number_percent() >
                        ch->pcdata->learned[gsn_pickshiplock]
                        && (number_percent() / 2) >
                        ch->pcdata->learned[gsn_jimmyshiplock]))
                {
                        send_to_char("You failed.\n\r", ch);
                        snprintf(buf, MSL,
                                 "[ALARM] %s attempting to pick %s.",
                                 ch->name, ship->name);
                        echo_to_room(AtRed, ch->in_room, buf);
                        learn_from_failure(ch, gsn_pickshiplock);
                        learn_from_failure(ch, gsn_jimmyshiplock);
                        return;
                }

                if (!ship->hatchopen)
                {
                        ship->hatchopen = TRUE;
                        act(AtPlain,
                            "You pick the lock and open the hatch on $T.", ch,
                            NULL, ship->name, ToChar);
                        act(AtPlain, "$n picks open the hatch on $T.", ch,
                            NULL, ship->name, ToRoom);
                        echo_to_room(AtYellow,
                                     get_room_index(ship->entrance),
                                     "The hatch opens from the outside.");
                        learn_from_success(ch, gsn_pickshiplock);
                        learn_from_success(ch, gsn_jimmyshiplock);
                        {
                                long      xpgain;

                                xpgain = UMIN(20,
                                              (exp_level
                                               (ch->
                                                skill_level[PiracyAbility] +
                                                1) -
                                               exp_level(ch->
                                                         skill_level
                                                         [PiracyAbility])));
                                gain_exp(ch, xpgain, PiracyAbility);
                                ch_printf(ch,
                                          "You gain %d piracy experience.",
                                          xpgain);
                        }

                }
                return;
        }

        ch_printf(ch, "You see no %s here.\n\r", arg);
        return;
}



CMDF do_sneak(CharData * ch, char *argument)
{
        AffectData af;

        argument = NULL;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        send_to_char("You attempt to move silently.\n\r", ch);
        affect_strip(ch, gsn_sneak);

        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_sneak])
        {
                af.type = gsn_sneak;
                af.duration =
                        (int) (ch->skill_level[SmugglingAbility] * DurConv);
                af.location = ApplyNone;
                af.modifier = 0;
                af.bitvector = AffSneak;
                affect_to_char(ch, &af);
                learn_from_success(ch, gsn_sneak);
        }
        else
                learn_from_failure(ch, gsn_sneak);

        return;
}



CMDF do_hide(CharData * ch, char *argument)
{
        argument = NULL;
        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        send_to_char("You attempt to hide.\n\r", ch);

        if (IsAffected(ch, AffHide))
                RemoveBit(ch->affected_by, AffHide);

        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_hide])
        {
                SetBit(ch->affected_by, AffHide);
                learn_from_success(ch, gsn_hide);
        }
        else
                learn_from_failure(ch, gsn_hide);
        return;
}

CMDF do_slight(CharData * ch, char *argument)
{
        argument = NULL;
        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        send_to_char("You attempt to be secretive.\n\r", ch);

        if (IsSet(ch->act, PlrSecretive))
                RemoveBit(ch->act, PlrSecretive);

        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_slight])
        {
                SetBit(ch->act, PlrSecretive);
                learn_from_success(ch, gsn_slight);
        }
        else
                learn_from_failure(ch, gsn_slight);
        return;
}

/*
 * Contributed by Alander.
 */
CMDF do_visible(CharData * ch, char *argument)
{
        argument = NULL;
        affect_strip(ch, gsn_invis);
        affect_strip(ch, gsn_mass_invis);
        affect_strip(ch, gsn_sneak);
        affect_strip(ch, gsn_hide);
        affect_strip(ch, gsn_slight);
        RemoveBit(ch->act, PlrSecretive);
        RemoveBit(ch->affected_by, AffHide);
        if (str_cmp(ch->race->name(), "defel")) /* Defel has perm invis */
                RemoveBit(ch->affected_by, AffInvisible);
        if (str_cmp(ch->race->name(), "noghri"))    /* Noghri has perm sneak */
                RemoveBit(ch->affected_by, AffSneak);
        send_to_char("Ok.\n\r", ch);
        return;
}


CMDF do_recall(CharData * ch, char *argument)
{
        RoomIndexData *location;
        CharData *opponent;

        argument = NULL;
        location = NULL;

        location = get_room_index(wherehome(ch));

        if (get_trust(ch) < LevelImmortal)
        {
                AreaData *pArea;

                if (!ch->pcdata || !(pArea = ch->pcdata->area))
                {
                        send_to_char("Only builders can recall.\n\r", ch);
                        return;
                }

                if (ch->in_room->vnum < pArea->low_r_vnum
                    || ch->in_room->vnum > pArea->hi_r_vnum)
                {
                        send_to_char
                                ("You can only recall from your assigned area.\n\r",
                                 ch);
                        return;
                }
        }

        if (!location)
        {
                send_to_char("You are completely lost.\n\r", ch);
                return;
        }

        if (ch->in_room == location)
                return;


        if ((opponent = who_fighting(ch)) != NULL)
        {

                if (number_bits(1) == 0
                    || (!IsNpc(opponent) && number_bits(3) > 1))
                {
                        WaitState(ch, 4);
                        ch_printf(ch, "You failed!\n\r");
                        return;
                }

                ch_printf(ch, "You recall from combat!\n\r");
                stop_fighting(ch, TRUE);
        }

        act(AtAction, "$n disappears in a swirl of the Force.", ch, NULL,
            NULL, ToRoom);
        char_from_room(ch);
        char_to_room(ch, location);
        if (ch->mount)
        {
                char_from_room(ch->mount);
                char_to_room(ch->mount, location);
        }
        act(AtAction, "$n appears in a swirl of the Force.", ch, NULL, NULL,
            ToRoom);
        do_look(ch, "auto");

        return;
}


CMDF do_aid(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        int       percent;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Aid whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("Aid yourself?\n\r", ch);
                return;
        }

        if (victim->position > PosStunned)
        {
                act(AtPlain, "$N doesn't need your help.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if (victim->hit <= -400)
        {
                act(AtPlain, "$N's condition is beyond your aiding ability.",
                    ch, NULL, victim, ToChar);
                return;
        }

        ch->alignment = ch->alignment + 20;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);

        percent = number_percent() - (get_curr_lck(ch) - 13);
        WaitState(ch, skill_table[gsn_aid]->beats);
        if (!IsNpc(ch) && percent > ch->pcdata->learned[gsn_aid])
        {
                send_to_char("You fail.\n\r", ch);
                learn_from_failure(ch, gsn_aid);
                return;
        }

        ch->alignment = ch->alignment + 20;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);

        act(AtSkill, "You aid $N!", ch, NULL, victim, ToChar);
        act(AtSkill, "$n aids $N!", ch, NULL, victim, ToNotvict);
        learn_from_success(ch, gsn_aid);
        if (victim->hit < 1)
                victim->hit = 1;

        update_pos(victim);
        act(AtSkill, "$n aids you!", ch, NULL, victim, ToVict);
        return;
}


CMDF do_mount(CharData * ch, char *argument)
{
        CharData *victim;

        if (!IsNpc(ch) && ch->pcdata->learned[gsn_mount] <= 0)
        {
                send_to_char("I don't think that would be a good idea...\n\r",
                             ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You're already mounted!\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                send_to_char("You can't find that here.\n\r", ch);
                return;
        }

        if (!IsNpc(victim) || !IsSet(victim->act, ActMountable))
        {
                send_to_char("You can't mount that!\n\r", ch);
                return;
        }

        if (IsSet(victim->act, ActMounted))
        {
                send_to_char("That mount already has a rider.\n\r", ch);
                return;
        }

        if (victim->position < PosStanding)
        {
                send_to_char("Your mount must be standing.\n\r", ch);
                return;
        }

        if (victim->position == PosFighting || victim->fighting)
        {
                send_to_char("Your mount is moving around too much.\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_mount]->beats);
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_mount])
        {
                SetBit(victim->act, ActMounted);
                ch->mount = victim;
                act(AtSkill, "You mount $N.", ch, NULL, victim, ToChar);
                act(AtSkill, "$n skillfully mounts $N.", ch, NULL, victim,
                    ToNotvict);
                act(AtSkill, "$n mounts you.", ch, NULL, victim, ToVict);
                learn_from_success(ch, gsn_mount);
                ch->position = PosMounted;
        }
        else
        {
                act(AtSkill, "You unsuccessfully try to mount $N.", ch, NULL,
                    victim, ToChar);
                act(AtSkill, "$n unsuccessfully attempts to mount $N.", ch,
                    NULL, victim, ToNotvict);
                act(AtSkill, "$n tries to mount you.", ch, NULL, victim,
                    ToVict);
                learn_from_failure(ch, gsn_mount);
        }
        return;
}


CMDF do_dismount(CharData * ch, char *argument)
{
        CharData *victim;

        argument = NULL;

        if ((victim = ch->mount) == NULL)
        {
                send_to_char("You're not mounted.\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_mount]->beats);
        if (IsNpc(ch) || number_percent() < ch->pcdata->learned[gsn_mount])
        {
                act(AtSkill, "You dismount $N.", ch, NULL, victim, ToChar);
                act(AtSkill, "$n skillfully dismounts $N.", ch, NULL, victim,
                    ToNotvict);
                act(AtSkill, "$n dismounts you.  Whew!", ch, NULL, victim,
                    ToVict);
                RemoveBit(victim->act, ActMounted);
                ch->mount = NULL;
                ch->position = PosStanding;
                learn_from_success(ch, gsn_mount);
        }
        else
        {
                act(AtSkill, "You fall off while dismounting $N.  Ouch!", ch,
                    NULL, victim, ToChar);
                act(AtSkill, "$n falls off of $N while dismounting.", ch,
                    NULL, victim, ToNotvict);
                act(AtSkill, "$n falls off your back.", ch, NULL, victim,
                    ToVict);
                learn_from_failure(ch, gsn_mount);
                RemoveBit(victim->act, ActMounted);
                ch->mount = NULL;
                ch->position = PosSitting;
                global_retcode = damage(ch, ch, 1, TypeUndefined);
        }
        return;
}


/**************************************************************************/


/*
 * Check for parry.
 */
bool check_parry(CharData * ch, CharData * victim)
{
        int       chances;
        ObjData *wield;

        if (!IsAwake(victim))
                return FALSE;

        if (IsNpc(victim) && !IsSet(victim->defenses, DfndParry))
                return FALSE;

        if (IsNpc(victim))
        {
                /*
                 * Tuan was here.  :)   *** so was Durga :p *** 
                 */
                chances = UMIN(60, victim->skill_level[CombatAbility]);
        }
        else
        {
                if ((wield = get_eq_char(victim, WearWield)) == NULL ||
                    (wield->value[3] != WeaponLightsaber))
                {
                        if ((wield =
                             get_eq_char(victim, WearDualWield)) == NULL
                            || (wield->value[3] != WeaponLightsaber))
                                return FALSE;
                }
                chances = (int) (victim->pcdata->learned[gsn_parry]);
        }

        chances = URANGE(10, chances, 90);

        if (number_range(1, 100) > chances)
        {
                learn_from_failure(victim, gsn_parry);
                return FALSE;
        }
        if (!IsNpc(victim) && !IsSet(victim->pcdata->flags, PcflagGag))
                 /*SB*/ act(AtPlain,
                            "&p&PYou parry &O&Y$n's &W&Pattack.&R&W", ch,
                            NULL, victim, ToVict);

        if (!IsNpc(ch) && !IsSet(ch->pcdata->flags, PcflagGag))  /* SB */
                act(AtPlain, "&Y$N &W&Pparries your attack.&R&W", ch, NULL,
                    victim, ToChar);

        learn_from_success(victim, gsn_parry);
        return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge(CharData * ch, CharData * victim)
{
        int       chances;

        if (!IsAwake(victim))
                return FALSE;

        if (IsNpc(victim) && !IsSet(victim->defenses, DfndDodge))
                return FALSE;

        if (IsNpc(victim))
                chances = UMIN(60, victim->top_level);
        else
                chances = (int) (victim->pcdata->learned[gsn_dodge] / 2);

        if (number_range(1, 100) > chances)
        {
                learn_from_failure(victim, gsn_dodge);
                return FALSE;
        }

        if (!IsNpc(victim) && !IsSet(victim->pcdata->flags, PcflagGag))
                act(AtPlain, "&cYou dodge &Y$n's&c attack.&R&W", ch, NULL,
                    victim, ToVict);

        if (!IsNpc(ch) && !IsSet(ch->pcdata->flags, PcflagGag))
                act(AtPlain, "&Y$N&c dodges your attack.&R&W", ch, NULL,
                    victim, ToChar);

        learn_from_success(victim, gsn_dodge);
        return TRUE;
}

CMDF do_poison_weapon(CharData * ch, char *argument)
{
        ObjData *obj;
        ObjData *pobj;
        ObjData *wobj;
        char      arg[MaxInputLength];
        int       percent;

        if (!IsNpc(ch) && ch->pcdata->learned[gsn_poison_weapon] <= 0)
        {
                send_to_char("What do you think you are, a thief?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("What are you trying to poison?\n\r", ch);
                return;
        }
        if (ch->fighting)
        {
                send_to_char("While you're fighting?  Nice try.\n\r", ch);
                return;
        }
        if (ms_find_obj(ch))
                return;

        if (!(obj = get_obj_carry(ch, arg)))
        {
                send_to_char("You do not have that weapon.\n\r", ch);
                return;
        }
        if (obj->item_type != ItemWeapon)
        {
                send_to_char("That item is not a weapon.\n\r", ch);
                return;
        }
        if (IsObjStat(obj, ItemPoisoned))
        {
                send_to_char("That weapon is already poisoned.\n\r", ch);
                return;
        }
        /*
         * Now we have a Valid weapon...check to see if we have the powder. 
         */
        for (pobj = ch->first_carrying; pobj; pobj = pobj->next_content)
        {
                if (pobj->pIndexData->vnum == ObjVnumBlackPowder)
                        break;
        }
        if (!pobj)
        {
                send_to_char("You do not have the black poison powder.\n\r",
                             ch);
                return;
        }
        /*
         * Okay, we have the powder...do we have water? 
         */
        for (wobj = ch->first_carrying; wobj; wobj = wobj->next_content)
        {
                if (wobj->item_type == ItemDrinkCon
                    && wobj->value[1] > 0 && wobj->value[2] == 0)
                        break;
        }
        if (!wobj)
        {
                send_to_char("You have no water to mix with the powder.\n\r",
                             ch);
                return;
        }
        /*
         * And does the thief have steady enough hands? 
         */
        if (!IsNpc(ch) && (ch->pcdata->condition[CondDrunk] > 0))
        {
                send_to_char
                        ("Your hands aren't steady enough to properly mix the poison.\n\r",
                         ch);
                return;
        }
        WaitState(ch, skill_table[gsn_poison_weapon]->beats);

        percent = (number_percent() - get_curr_lck(ch) - 14);

        /*
         * Check the skill percentage 
         */
        separate_obj(pobj);
        separate_obj(wobj);
        if (!IsNpc(ch) && percent > ch->pcdata->learned[gsn_poison_weapon])
        {
                set_char_color(AtRed, ch);
                send_to_char
                        ("You failed and spill some on yourself.  Ouch!\n\r",
                         ch);
                set_char_color(AtGrey, ch);
                damage(ch, ch, ch->skill_level[HuntingAbility],
                       gsn_poison_weapon);
                act(AtRed, "$n spills the poison all over!", ch, NULL, NULL,
                    ToRoom);
                extract_obj(pobj);
                extract_obj(wobj);
                learn_from_failure(ch, gsn_poison_weapon);
                return;
        }
        separate_obj(obj);
        /*
         * Well, I'm tired of waiting.  Are you? 
         */
        act(AtRed, "You mix $p in $P, creating a deadly poison!", ch, pobj,
            wobj, ToChar);
        act(AtRed, "$n mixes $p in $P, creating a deadly poison!", ch, pobj,
            wobj, ToRoom);
        act(AtGreen, "You pour the poison over $p, which glistens wickedly!",
            ch, obj, NULL, ToChar);
        act(AtGreen, "$n pours the poison over $p, which glistens wickedly!",
            ch, obj, NULL, ToRoom);
        SetBit(obj->extra_flags, ItemPoisoned);
        obj->cost *= ch->skill_level[HuntingAbility] / 2;
        /*
         * Set an object timer.  Don't want proliferation of poisoned weapons 
         */
        obj->timer = 10 + ch->skill_level[HuntingAbility];

        if (IsObjStat(obj, ItemBless))
                obj->timer *= 2;

        if (IsObjStat(obj, ItemMagic))
                obj->timer *= 2;

        /*
         * WHAT?  All of that, just for that one bit?  How lame. ;) 
         */
        act(AtBlue, "The remainder of the poison eats through $p.", ch, wobj,
            NULL, ToChar);
        act(AtBlue, "The remainder of the poison eats through $p.", ch, wobj,
            NULL, ToRoom);
        extract_obj(pobj);
        extract_obj(wobj);
        learn_from_success(ch, gsn_poison_weapon);
        return;
}

bool check_grip(CharData * ch, CharData * victim)
{
        int       percent_chance;

        if (!IsAwake(victim))
                return FALSE;

        if (IsNpc(victim) && !IsSet(victim->defenses, DfndGrip))
                return FALSE;

        if (IsNpc(victim))
                percent_chance = UMIN(60, 2 * victim->top_level);
        else
                percent_chance =
                        (int) (victim->pcdata->learned[gsn_grip] / 2);

        /*
         * Consider Luck as a factor 
         */
        percent_chance += (2 * (get_curr_lck(victim) - 13));

        if (number_percent() >=
            percent_chance + victim->top_level - ch->top_level)
        {
                learn_from_failure(victim, gsn_grip);
                return FALSE;
        }
        act(AtSkill, "You evade $n's attempt to disarm you.", ch, NULL,
            victim, ToVict);
        act(AtSkill, "$N holds $S weapon strongly, and is not disarmed.", ch,
            NULL, victim, ToChar);
        learn_from_success(victim, gsn_grip);
        return TRUE;
}

CMDF do_circle(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;
        ObjData *obj;
        int       percent;

        if (IsNpc(ch) && IsAffected(ch, AffCharm))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        one_argument(argument, arg);

        if (ch->mount)
        {
                send_to_char("You can't circle while mounted.\n\r", ch);
                return;
        }

        if (arg[0] == '\0')
        {
                send_to_char("Circle around whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("How can you sneak up on yourself?\n\r", ch);
                return;
        }

        if (is_safe(ch, victim))
                return;

        obj = get_eq_char(ch, WearWield);
        if (!obj || obj->item_type != ItemWeapon
            || obj->value[3] != WeaponKnife)
        {
                obj = get_eq_char(ch, WearDualWield);
                if (!obj || obj->item_type != ItemWeapon
                    || obj->value[3] != WeaponKnife)
                {
                        send_to_char
                                ("You need to wield a stabbing weapon.\n\r",
                                 ch);
                        return;
                }
        }

        if (!ch->fighting)
        {
                send_to_char("You can't circle when you aren't fighting.\n\r",
                             ch);
                return;
        }

        if (!victim->fighting)
        {
                send_to_char
                        ("You can't circle around a person who is not fighting.\n\r",
                         ch);
                return;
        }

        if (victim->num_fighting < 2)
        {
                act(AtPlain,
                    "You can't circle around them without a distraction.", ch,
                    NULL, victim, ToChar);
                return;
        }

        percent = number_percent() - (get_curr_lck(ch) - 16)
                + (get_curr_lck(victim) - 13);

        WaitState(ch, skill_table[gsn_circle]->beats);
        if (percent <
            (IsNpc(ch) ? (ch->skill_level[HuntingAbility] * 1.5) : ch->
             pcdata->learned[gsn_circle]))
        {
                learn_from_success(ch, gsn_circle);
                global_retcode = multi_hit(ch, victim, gsn_circle);

        }
        else
        {
                learn_from_failure(ch, gsn_circle);
                global_retcode = damage(ch, victim, 0, gsn_circle);
        }
        return;
}

/* Berserk and HitAll. -- Altrag */
CMDF do_berserk(CharData * ch, char *argument)
{
        sh_int    percent;
        AffectData af;

        argument = NULL;

        if (!ch->fighting)
        {
                send_to_char("But you aren't fighting!\n\r", ch);
                return;
        }

        if (IsAffected(ch, AffBerserk))
        {
                send_to_char("Your rage is already at its peak!\n\r", ch);
                return;
        }

        percent = IsNpc(ch) ? 80 : ch->pcdata->learned[gsn_berserk];
        WaitState(ch, skill_table[gsn_berserk]->beats);
        if (!chance(ch, percent))
        {
                send_to_char("You couldn't build up enough rage.\n\r", ch);
                learn_from_failure(ch, gsn_berserk);
                return;
        }
        af.type = gsn_berserk;
        /*
         * Hmmm.. 10-20 combat rounds at level 50.. good enough for most mobs,
         * and if not they can always go berserk again.. shrug.. maybe even
         * too high. -- Altrag 
         */
        af.duration = number_range(ch->top_level / 5, ch->top_level * 2 / 5);
        /*
         * Hmm.. you get stronger when yer really enraged.. mind over matter
         * type thing.. 
         */
        af.location = ApplyStr;
        af.modifier = 1;
        af.bitvector = AffBerserk;
        affect_to_char(ch, &af);
        send_to_char("You start to lose control..\n\r", ch);
        learn_from_success(ch, gsn_berserk);
        return;
}

/* External from fight.c */
ch_ret one_hit args((CharData * ch, CharData * victim, int dt));
CMDF do_hitall(CharData * ch, char *argument)
{
        CharData *vch;
        CharData *vch_next;
        sh_int    nvict = 0;
        sh_int    nhit = 0;
        sh_int    percent;

        argument = NULL;

        if (xIS_SET(ch->in_room->RoomFlags, RoomSafe))
        {
                send_to_char("You cannot do that here.\n\r", ch);
                return;
        }

        if (!ch->in_room->first_person)
        {
                send_to_char("There's no one here!\n\r", ch);
                return;
        }
        percent = IsNpc(ch) ? 80 : ch->pcdata->learned[gsn_hitall];
        for (vch = ch->in_room->first_person; vch; vch = vch_next)
        {
                vch_next = vch->next_in_room;
                if (is_same_group(ch, vch) || !is_legal_kill(ch, vch) ||
                    !can_see(ch, vch) || is_safe(ch, vch))
                        continue;
                if (++nvict > ch->skill_level[CombatAbility] / 5)
                        break;
                if (chance(ch, percent))
                {
                        nhit++;
                        global_retcode = one_hit(ch, vch, TypeUndefined);
                }
                else
                        global_retcode = damage(ch, vch, 0, TypeUndefined);
                /*
                 * Fireshield, etc. could kill ch too.. :>.. -- Altrag 
                 */
                if (global_retcode == rCHAR_DIED
                    || global_retcode == rBOTH_DIED || char_died(ch))
                        return;
        }
        if (!nvict)
        {
                send_to_char("There's no one here!\n\r", ch);
                return;
        }
        ch->endurance = UMAX(0, ch->endurance - nvict * 3 + nhit);
        if (nhit)
                learn_from_success(ch, gsn_hitall);
        else
                learn_from_failure(ch, gsn_hitall);
        return;
}



bool check_illegal_psteal(CharData * ch, CharData * victim)
{
        ch = victim = NULL;
        return FALSE;
}

CMDF do_scan(CharData * ch, char *argument)
{
        RoomIndexData *was_in_room;
        RoomIndexData *to_room;
        ExitData *pexit;
        sh_int    dir = -1;
        sh_int    dist;
        sh_int    max_dist = 5;

        if (argument[0] == '\0')
        {
                send_to_char("Scan in a direction...\n\r", ch);
                return;
        }

        if ((dir = get_door(argument)) == -1)
        {
                send_to_char("Scan in WHAT direction?\n\r", ch);
                return;
        }

        was_in_room = ch->in_room;
        act(AtGrey, "Scanning $t...", ch, dir_name[dir], NULL, ToChar);
        act(AtGrey, "$n scans $t.", ch, dir_name[dir], NULL, ToRoom);

        if (IsNpc(ch) || (number_percent() > ch->pcdata->learned[gsn_scan]))
        {
                act(AtGrey, "You stop scanning $t as your vision blurs.", ch,
                    dir_name[dir], NULL, ToChar);
                learn_from_failure(ch, gsn_scan);
                return;
        }


        if ((pexit = get_exit(ch->in_room, dir)) == NULL)
        {
                act(AtGrey, "You can't see $t.", ch, dir_name[dir], NULL,
                    ToChar);
                return;
        }

        if (ch->top_level < 50)
                max_dist--;
        if (ch->top_level < 20)
                max_dist--;

        for (dist = 1; dist <= max_dist;)
        {
                if (IsSet(pexit->exit_info, ExClosed))
                {
                        if (IsSet(pexit->exit_info, ExSecret))
                                act(AtGrey,
                                    "Your view $t is blocked by a wall.", ch,
                                    dir_name[dir], NULL, ToChar);
                        else
                                act(AtGrey,
                                    "Your view $t is blocked by a door.", ch,
                                    dir_name[dir], NULL, ToChar);
                        break;
                }

                to_room = NULL;
                if (pexit->distance > 1)
                        to_room = generate_exit(ch->in_room, &pexit);

                if (to_room == NULL)
                        to_room = pexit->to_room;

                if (room_is_private(ch, to_room)
                    && get_trust(ch) < LevelGreater)
                {
                        act(AtGrey,
                            "Your view $t is blocked by a private room.", ch,
                            dir_name[dir], NULL, ToChar);
                        break;
                }
                char_from_room(ch);
                char_to_room(ch, to_room);
                set_char_color(AtRmname, ch);
                send_to_char(ch->in_room->name, ch);
                send_to_char("\n\r", ch);
                show_list_to_char(ch->in_room->first_content, ch, FALSE,
                                  FALSE);
                show_char_to_char(ch->in_room->first_person, ch);

                switch (ch->in_room->sector_type)
                {
                default:
                        dist++;
                        break;
                case SectAir:
                        if (number_percent() < 80)
                                dist++;
                        break;
                case SectInside:
                case SectField:
                case SectUnderground:
                        dist++;
                        break;
                case SectForest:
                case SectCity:
                case SectDesert:
                case SectHills:
                        dist += 2;
                        break;
                case SectWaterSwim:
                case SectWaterNoswim:
                        dist += 3;
                        break;
                case SectMountain:
                case SectUnderwater:
                case SectOceanfloor:
                        dist += 4;
                        break;
                }

                if (dist >= max_dist)
                {
                        act(AtGrey,
                            "Your vision blurs with distance and you see no "
                            "farther $t.", ch, dir_name[dir], NULL, ToChar);
                        break;
                }
                if ((pexit = get_exit(ch->in_room, dir)) == NULL)
                {
                        act(AtGrey, "Your view $t is blocked by a wall.", ch,
                            dir_name[dir], NULL, ToChar);
                        break;
                }
        }

        char_from_room(ch);
        char_to_room(ch, was_in_room);
        learn_from_success(ch, gsn_scan);

        return;
}

CMDF do_add_teacher(CharData * ch, char *argument)
{
        char      arg1[MIL];
        char      buf[MSL];
        int       sn, vnum;
        SkillType *skill;

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("syntax: addteacher <skill> <mob>\n\r", ch);
                return;
        }

        if (is_number(arg1))
                sn = atoi(arg1);
        else
                sn = skill_lookup(arg1);

        if (is_number(argument))
                vnum = atoi(argument);
        else
        {
                CharData *vch = NULL;

                if ((vch = get_char_world(ch, argument)) == NULL
                    || !IsNpc(vch))
                {
                        send_to_char("Invalid teacher.\n\r", ch);
                        return;
                }
                SetBit(vch->act, ActPractice);
                SetBit(vch->pIndexData->act, ActPractice);
                vnum = vch->pIndexData->vnum;
        }
        if ((skill = get_skilltype(sn)) == NULL)
        {
                send_to_char("Skill number out of range.\n\r", ch);
                return;
        }

        if (skill->teachers)
        {
                snprintf(buf, MSL, "%s %d", skill->teachers, vnum);
                DISPOSE(skill->teachers);
        }
        else
                snprintf(buf, MSL, "%d", vnum);
        skill->teachers = str_dup(buf);
        ch_printf(ch, "Teacher added, skills teachers are: %s\n\r", buf);
}

/*
CMDF do_convertskillraces(CharData *ch, char *argument)
{
	char buf[MSL];
	char tmpbuf[MSL];
	for ( sh_int sn = 0; sn < top_sn; sn++ )
		{
	    	if ( !skill_table[sn]->name )
				break;
			if ( !skill_table[sn]->races || !skill_table[sn]->races[0] != '\0' )
				continue;

			ch_printf(ch, "Converting for skill: %s\n\r", skill_table[sn]->name);
			argument = skill_table[sn]->races;
			tmpbuf[0] = '\0';
    	    while ( argument[0] != '\0' )
        	{
            	argument = one_argument( argument, buf );
				ch_printf(ch, "\tConverting %s into %s\n\r", buf, race_table[atoi(buf)].race_name);
				mudstrlcat(tmpbuf, race_table[atoi(buf)].race_name, MSL);
				mudstrlcat(tmpbuf, " ", MSL);
    	    }
			stralloc_printf(&skill_table[sn]->races, "%s", tmpbuf);	
		}
	send_to_char( "Done.\n\r", ch );
}*/


CMDF do_skilllist(CharData * ch, char *argument)
{
        char      buf[MSL];
        int       sn, count = 0;
        CharData *vch = NULL;

        if ((vch = get_char_room(ch, argument)) == NULL || !IsNpc(vch)
            || !IsSet(vch->act, ActPractice))
        {
                send_to_char("Invalid teacher.\n\r", ch);
                return;
        }

        ch_printf(ch, "&BMob &w\"&z%s&w\" can teach:\n\r\t",
                  vch->short_descr);

        snprintf(buf, MSL, "%d", vch->pIndexData->vnum);

        for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name;
             sn++)
                if (skill_table[sn]->teachers
                    && is_name(buf, skill_table[sn]->teachers))
                {
                        if (count != 0 && (count % 3) != 0)
                                send_to_char(", ", ch);
                        ch_printf(ch, "%20s", skill_table[sn]->name);
                        count++;
                        if ((count % 3) == 0)
                                send_to_char(",\n\r\t", ch);
                }
}
