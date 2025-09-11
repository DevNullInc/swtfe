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
 * Dark Warrior Code additions and changes from the Star Wars Reality code copyright (c) *
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
 * Magic system implementation for spells, skills, and supernatural abilities.          *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"
#include "races.hpp"

/*
 * Local functions.
 */
void say_spell args((CharData * ch, int sn));
CharData *make_poly_mob args((CharData * ch, int vnum));
ch_ret spell_affect args((int sn, int level, CharData * ch, void *vo));
ch_ret spell_affectchar args((int sn, int level, CharData * ch, void *vo));
bool arena_can_fight args((CharData * ch, CharData * victim));


/*
 * Is immune to a damage type
 */
bool is_immune(CharData * ch, sh_int damtype)
{
        switch (damtype)
        {
        case SdFire:
                if (IsSet(ch->immune, RisFire))
                        return TRUE;
        case SdCold:
                if (IsSet(ch->immune, RisCold))
                        return TRUE;
        case SdElectricity:
                if (IsSet(ch->immune, RisElectricity))
                        return TRUE;
        case SdEnergy:
                if (IsSet(ch->immune, RisEnergy))
                        return TRUE;
        case SdAcid:
                if (IsSet(ch->immune, RisAcid))
                        return TRUE;
        case SdPoison:
                if (IsSet(ch->immune, RisPoison))
                        return TRUE;
        case SdDrain:
                if (IsSet(ch->immune, RisDrain))
                        return TRUE;
        }
        return FALSE;
}

/*
 * Lookup a skill by name, only stopping at skills the player has.
 */
int ch_slookup(CharData * ch, const char *name)
{
        int       sn;

        if (IsNpc(ch))
                return skill_lookup(name);
        for (sn = 0; sn < top_sn; sn++)
        {
                if (!skill_table[sn]->name)
                        break;
                if (ch->pcdata->learned[sn] > 0
                    && LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
                    && !str_prefix(name, skill_table[sn]->name))
                        return sn;
        }

        return -1;
}

/*
 * Lookup a personal skill
 */
int personal_lookup(CharData * ch, const char *name)
{
        ch = NULL;
        name = NULL;
        return -1;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup(const char *name)
{
        int       sn;

        if ((sn =
             bsearch_skill(name, gsn_first_spell, gsn_first_skill - 1)) == -1)
                if ((sn =
                     bsearch_skill(name, gsn_first_skill,
                                   gsn_first_weapon - 1)) == -1)
                        if ((sn =
                             bsearch_skill(name, gsn_first_weapon,
                                           gsn_first_tongue - 1)) == -1)
                                if ((sn =
                                     bsearch_skill(name, gsn_first_tongue,
                                                   gsn_top_sn - 1)) == -1
                                    && gsn_top_sn < top_sn)
                                {
                                        for (sn = gsn_top_sn; sn < top_sn;
                                             sn++)
                                        {
                                                if (!skill_table[sn]
                                                    || !skill_table[sn]->name)
                                                        return -1;
                                                if (LOWER(name[0]) ==
                                                    LOWER(skill_table[sn]->
                                                          name[0])
                                                    && !str_prefix(name,
                                                                   skill_table
                                                                   [sn]->
                                                                   name))
                                                        return sn;
                                        }
                                        return -1;
                                }
        return sn;
}

/*
 * Return a skilltype pointer based on sn			-Thoric
 * Returns NULL if bad, unused or personal sn.
 */
SkillType *get_skilltype(int sn)
{
        if (sn >= TypePersonal)
                return NULL;
        if (sn >= TypeHit)
                return NULL;
        return IsValidSn(sn) ? skill_table[sn] : NULL;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 */
int bsearch_skill(const char *name, int first, int top)
{
        int       sn;

        for (;;)
        {
                sn = (first + top) >> 1;

                if (LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
                    && !str_prefix(name, skill_table[sn]->name))
                        return sn;
                if (first >= top)
                        return -1;
                if (strcmp(name, skill_table[sn]->name) < 1)
                        top = sn - 1;
                else
                        first = sn + 1;
        }
        return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 * Check for exact matches only
 */
int bsearch_skill_exact(const char *name, int first, int top)
{
        int       sn;

        for (;;)
        {
                sn = (first + top) >> 1;
                if (!str_prefix(name, skill_table[sn]->name))
                        return sn;
                if (first >= top)
                        return -1;
                if (strcmp(name, skill_table[sn]->name) < 1)
                        top = sn - 1;
                else
                        first = sn + 1;
        }
        return -1;
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
int ch_bsearch_skill(CharData * ch, const char *name, int first, int top)
{
        int       sn;

        for (;;)
        {
                sn = (first + top) >> 1;

                if (LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
                    && !str_prefix(name, skill_table[sn]->name)
                    && ch->pcdata->learned[sn] > 0)
                        return sn;
                if (first >= top)
                        return -1;
                if (strcmp(name, skill_table[sn]->name) < 1)
                        top = sn - 1;
                else
                        first = sn + 1;
        }
        return -1;
}


int find_spell(CharData * ch, const char *name, bool know)
{
        if (IsNpc(ch) || !know)
                return bsearch_skill(name, gsn_first_spell,
                                     gsn_first_skill - 1);
        else
                return ch_bsearch_skill(ch, name, gsn_first_spell,
                                        gsn_first_skill - 1);
}

int find_skill(CharData * ch, const char *name, bool know)
{
        if (IsNpc(ch) || !know)
                return bsearch_skill(name, gsn_first_skill,
                                     gsn_first_weapon - 1);
        else
                return ch_bsearch_skill(ch, name, gsn_first_skill,
                                        gsn_first_weapon - 1);
}

int find_weapon(CharData * ch, const char *name, bool know)
{
        if (IsNpc(ch) || !know)
                return bsearch_skill(name, gsn_first_weapon,
                                     gsn_first_tongue - 1);
        else
                return ch_bsearch_skill(ch, name, gsn_first_weapon,
                                        gsn_first_tongue - 1);
}

int find_tongue(CharData * ch, const char *name, bool know)
{
        if (IsNpc(ch) || !know)
                return bsearch_skill(name, gsn_first_tongue, gsn_top_sn - 1);
        else
                return ch_bsearch_skill(ch, name, gsn_first_tongue,
                                        gsn_top_sn - 1);
}


/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup(int slot)
{
        extern bool fBootDb;
        int       sn;

        if (slot <= 0)
                return -1;

        for (sn = 0; sn < top_sn; sn++)
                if (slot == skill_table[sn]->slot)
                        return sn;

        if (fBootDb)
        {
                bug("Slot_lookup: bad slot %d.", slot);
                abort();
        }

        return -1;
}

/*
 * Fancy message handling for a successful casting		-Thoric
 */
void successful_casting(SkillType * skill, CharData * ch,
                        CharData * victim, ObjData * obj)
{
        sh_int    chitroom =
                (skill->type == SkillSpell ? AtMagic : AtAction);
        sh_int    chit = (skill->type == SkillSpell ? AtMagic : AtHit);
        sh_int    chitme = (skill->type == SkillSpell ? AtMagic : AtHitme);

        if (skill->target != TarCharOffensive)
        {
                chit = chitroom;
                chitme = chitroom;
        }

        if (ch && ch != victim)
        {
                if (skill->hit_char && skill->hit_char[0] != '\0')
                        act(chit, skill->hit_char, ch, obj, victim, ToChar);
                else if (skill->type == SkillSpell)
                        act(chit, "Ok.", ch, NULL, NULL, ToChar);
        }
        if (ch && skill->hit_room && skill->hit_room[0] != '\0')
                act(chitroom, skill->hit_room, ch, obj, victim, ToNotvict);
        if (ch && victim && skill->hit_vict && skill->hit_vict[0] != '\0')
        {
                if (ch != victim)
                        act(chitme, skill->hit_vict, ch, obj, victim,
                            ToVict);
                else
                        act(chitme, skill->hit_vict, ch, obj, victim,
                            ToChar);
        }
        else if (ch && ch == victim && skill->type == SkillSpell)
                act(chitme, "Ok.", ch, NULL, NULL, ToChar);
}

/*
 * Fancy message handling for a failed casting			-Thoric
 */
void failed_casting(SkillType * skill, CharData * ch, CharData * victim,
                    ObjData * obj)
{
        sh_int    chitroom =
                (skill->type == SkillSpell ? AtMagic : AtAction);
        sh_int    chit = (skill->type == SkillSpell ? AtMagic : AtHit);
        sh_int    chitme = (skill->type == SkillSpell ? AtMagic : AtHitme);

        if (skill->target != TarCharOffensive)
        {
                chit = chitroom;
                chitme = chitroom;
        }

        if (ch && ch != victim)
        {
                if (skill->miss_char && skill->miss_char[0] != '\0')
                        act(chit, skill->miss_char, ch, obj, victim, ToChar);
                else if (skill->type == SkillSpell)
                        act(chit, "You failed.", ch, NULL, NULL, ToChar);
        }
        if (ch && skill->miss_room && skill->miss_room[0] != '\0')
                act(chitroom, skill->miss_room, ch, obj, victim, ToNotvict);
        if (ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0')
        {
                if (ch != victim)
                        act(chitme, skill->miss_vict, ch, obj, victim,
                            ToVict);
                else
                        act(chitme, skill->miss_vict, ch, obj, victim,
                            ToChar);
        }
        else if (ch && ch == victim)
        {
                if (skill->miss_char && skill->miss_char[0] != '\0')
                        act(chitme, skill->miss_char, ch, obj, victim,
                            ToChar);
                else if (skill->type == SkillSpell)
                        act(chitme, "You failed.", ch, NULL, NULL, ToChar);
        }
}

/*
 * Fancy message handling for being immune to something		-Thoric
 */
void immune_casting(SkillType * skill, CharData * ch,
                    CharData * victim, ObjData * obj)
{
        sh_int    chitroom =
                (skill->type == SkillSpell ? AtMagic : AtAction);
        sh_int    chit = (skill->type == SkillSpell ? AtMagic : AtHit);
        sh_int    chitme = (skill->type == SkillSpell ? AtMagic : AtHitme);

        if (skill->target != TarCharOffensive)
        {
                chit = chitroom;
                chitme = chitroom;
        }

        if (ch && ch != victim)
        {
                if (skill->imm_char && skill->imm_char[0] != '\0')
                        act(chit, skill->imm_char, ch, obj, victim, ToChar);
                else if (skill->miss_char && skill->miss_char[0] != '\0')
                        act(chit, skill->hit_char, ch, obj, victim, ToChar);
                else if (skill->type == SkillSpell
                         || skill->type == SkillSkill)
                        act(chit, "That appears to have no effect.", ch, NULL,
                            NULL, ToChar);
        }
        if (ch && skill->imm_room && skill->imm_room[0] != '\0')
                act(chitroom, skill->imm_room, ch, obj, victim, ToNotvict);
        else if (ch && skill->miss_room && skill->miss_room[0] != '\0')
                act(chitroom, skill->miss_room, ch, obj, victim, ToNotvict);
        if (ch && victim && skill->imm_vict && skill->imm_vict[0] != '\0')
        {
                if (ch != victim)
                        act(chitme, skill->imm_vict, ch, obj, victim,
                            ToVict);
                else
                        act(chitme, skill->imm_vict, ch, obj, victim,
                            ToChar);
        }
        else if (ch && victim && skill->miss_vict
                 && skill->miss_vict[0] != '\0')
        {
                if (ch != victim)
                        act(chitme, skill->miss_vict, ch, obj, victim,
                            ToVict);
                else
                        act(chitme, skill->miss_vict, ch, obj, victim,
                            ToChar);
        }
        else if (ch && ch == victim)
        {
                if (skill->imm_char && skill->imm_char[0] != '\0')
                        act(chit, skill->imm_char, ch, obj, victim, ToChar);
                else if (skill->miss_char && skill->miss_char[0] != '\0')
                        act(chit, skill->miss_char, ch, obj, victim, ToChar);
                else if (skill->type == SkillSpell
                         || skill->type == SkillSkill)
                        act(chit, "That appears to have no affect.", ch, NULL,
                            NULL, ToChar);
        }
}


/*
 * Utter mystical words for an sn.
 */
void say_spell(CharData * ch, int sn)
{
        CharData *rch;

        sn = 0;

        for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
        {
                if (rch != ch)
                        act(AtMagic,
                            "$n pauses and concentrates for a moment.", ch,
                            NULL, rch, ToVict);
        }

        return;
}


/*
 * Make adjustments to saving throw based in RIS		-Thoric
 */
int ris_save(CharData * ch, int percent_chance, int ris)
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
                return 1000;
        if (modifier == 10)
                return percent_chance;
        return (percent_chance * modifier) / 10;
}


/*								    -Thoric
 * Fancy dice expression parsing complete with order of operations,
 * simple exponent support, dice support as well as a few extra
 * New Force bits using endurance except mana/move
 * variables: L = level, H = hp, M = endurance, V = endurance, S = str, X = dex
 *            I = int, W = wis, C = con, A = cha, U = Luck, A = age
 *
 * Used for spell dice parsing, ie: 3d8+L-6
 *
 */
int rd_parse(CharData * ch, int level, char *expression)
{
        int       x, lop = 0, gop = 0, eop = 0;
        char      operation;
        char     *sexp[2];
        int       total = 0, len = 0;

        /*
         * take care of nulls coming in 
         */
        if (!expression || !strlen(expression))
                return 0;

        /*
         * get rid of brackets if they surround the entire expresion 
         */
        if ((*expression == '(') && expression[strlen(expression) - 1] == ')')
        {
                expression[strlen(expression) - 1] = '\0';
                expression++;
        }

        /*
         * check if the expresion is just a number 
         */
        len = strlen(expression);
        if (len == 1 && isalpha(expression[0]))
                switch (expression[0])
                {
                case 'L':
                case 'l':
                        return level;
                case 'H':
                case 'h':
                        return ch->hit;
                case 'M':
                case 'm':
                        return ch->endurance;
                case 'V':
                case 'v':
                        return ch->endurance;
                case 'S':
                case 's':
                        return get_curr_str(ch);
                case 'I':
                case 'i':
                        return get_curr_int(ch);
                case 'W':
                case 'w':
                        return get_curr_wis(ch);
                case 'X':
                case 'x':
                        return get_curr_dex(ch);
                case 'C':
                case 'c':
                        return get_curr_con(ch);
                case 'A':
                case 'a':
                        return get_curr_cha(ch);
                case 'U':
                case 'u':
                        return get_curr_lck(ch);
                case 'Y':
                case 'y':
                        return get_age(ch);
                }

        for (x = 0; x < len; ++x)
                if (!isdigit(expression[x]) && !isspace(expression[x]))
                        break;
        if (x == len)
                return (atoi(expression));

        /*
         * break it into 2 parts 
         */
        for (x = 0; x < (int) strlen(expression); ++x)
                switch (expression[x])
                {
                case '^':
                        if (!total)
                                eop = x;
                        break;
                case '-':
                case '+':
                        if (!total)
                                lop = x;
                        break;
                case '*':
                case '/':
                case '%':
                case 'd':
                case 'D':
                        if (!total)
                                gop = x;
                        break;
                case '(':
                        ++total;
                        break;
                case ')':
                        --total;
                        break;
                }
        if (lop)
                x = lop;
        else if (gop)
                x = gop;
        else
                x = eop;
        operation = expression[x];
        expression[x] = '\0';
        sexp[0] = expression;
        sexp[1] = (char *) (expression + x + 1);

        /*
         * work it out 
         */
        total = rd_parse(ch, level, sexp[0]);
        switch (operation)
        {
        case '-':
                total -= rd_parse(ch, level, sexp[1]);
                break;
        case '+':
                total += rd_parse(ch, level, sexp[1]);
                break;
        case '*':
                total *= rd_parse(ch, level, sexp[1]);
                break;
        case '/':
                total /= rd_parse(ch, level, sexp[1]);
                break;
        case '%':
                total %= rd_parse(ch, level, sexp[1]);
                break;
        case 'd':
        case 'D':
                total = dice(total, rd_parse(ch, level, sexp[1]));
                break;
        case '^':
                {
                        int       y = rd_parse(ch, level, sexp[1]), z = total;

                        for (x = 1; x < y; ++x, z *= total);
                        total = z;
                        break;
                }
        }
        return total;
}

/* wrapper function so as not to destroy exp */
int dice_parse(CharData * ch, int level, char *expression)
{
        char      buf[MaxInputLength];

        mudstrlcpy(buf, expression, MIL);
        return rd_parse(ch, level, buf);
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_poison_death(int level, CharData * victim)
{
        int       save;

        save = 50 + (victim->top_level - level -
                     victim->SavingPoisonDeath) * 2;
        save = URANGE(5, save, 95);
        return chance(victim, save);
}

bool saves_wands(int level, CharData * victim)
{
        int       save;

        if (IsSet(victim->immune, RisMagic))
                return TRUE;

        save = 50 + (victim->top_level - level - victim->SavingWand) * 2;
        save = URANGE(5, save, 95);
        return chance(victim, save);
}

bool saves_para_petri(int level, CharData * victim)
{
        int       save;

        save = 50 + (victim->top_level - level -
                     victim->SavingParaPetri) * 2;
        save = URANGE(5, save, 95);
        return chance(victim, save);
}

bool saves_breath(int level, CharData * victim)
{
        int       save;

        save = 50 + (victim->top_level - level - victim->SavingBreath) * 2;
        save = URANGE(5, save, 95);
        return chance(victim, save);
}

bool saves_spell_staff(int level, CharData * victim)
{
        int       save;

        if (IsSet(victim->immune, RisMagic))
                return TRUE;

        if (IsNpc(victim) && level > 10)
                level -= 5;
        save = 50 + (victim->top_level - level -
                     victim->SavingSpellStaff) * 2;
        save = URANGE(5, save, 95);
        return chance(victim, save);
}


/*
 * Process the spell's required components, if any		-Thoric
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */
bool process_spell_components(CharData * ch, int sn)
{
        SkillType *skill = get_skilltype(sn);
        char     *comp = skill->components;
        char     *check;
        char      arg[MaxInputLength];
        bool      consume, fail, found;
        int       val, value;
        ObjData *obj;

        /*
         * if no components necessary, then everything is cool 
         */
        if (!comp || comp[0] == '\0')
                return TRUE;

/* disable the whole damn shabang */

        return TRUE;

        while (comp[0] != '\0')
        {
                comp = one_argument(comp, arg);
                consume = TRUE;
                fail = found = FALSE;
                val = -1;
                switch (arg[1])
                {
                default:
                        check = arg + 1;
                        break;
                case '!':
                        check = arg + 2;
                        fail = TRUE;
                        break;
                case '+':
                        check = arg + 2;
                        consume = FALSE;
                        break;
                case '@':
                        check = arg + 2;
                        val = 0;
                        break;
                case '#':
                        check = arg + 2;
                        val = 1;
                        break;
                case '$':
                        check = arg + 2;
                        val = 2;
                        break;
                case '%':
                        check = arg + 2;
                        val = 3;
                        break;
                case '^':
                        check = arg + 2;
                        val = 4;
                        break;
                case '&':
                        check = arg + 2;
                        val = 5;
                        break;
                }
                value = atoi(check);
                obj = NULL;
                switch (UPPER(arg[0]))
                {
                case 'T':
                        for (obj = ch->first_carrying; obj;
                             obj = obj->next_content)
                                if (obj->item_type == value)
                                {
                                        if (fail)
                                        {
                                                send_to_char
                                                        ("Something disrupts the use of this power...\n\r",
                                                         ch);
                                                return FALSE;
                                        }
                                        found = TRUE;
                                        break;
                                }
                        break;
                case 'V':
                        for (obj = ch->first_carrying; obj;
                             obj = obj->next_content)
                                if (obj->pIndexData->vnum == value)
                                {
                                        if (fail)
                                        {
                                                send_to_char
                                                        ("Something disrupts the use of this power...\n\r",
                                                         ch);
                                                return FALSE;
                                        }
                                        found = TRUE;
                                        break;
                                }
                        break;
                case 'K':
                        for (obj = ch->first_carrying; obj;
                             obj = obj->next_content)
                                if (nifty_is_name(check, obj->name))
                                {
                                        if (fail)
                                        {
                                                send_to_char
                                                        ("Something disrupts the use of this power...\n\r",
                                                         ch);
                                                return FALSE;
                                        }
                                        found = TRUE;
                                        break;
                                }
                        break;
                case 'G':
                        if (ch->gold >= value)
                        {
                                if (fail)
                                {
                                        send_to_char
                                                ("Something disrupts the use of this power...\n\r",
                                                 ch);
                                        return FALSE;
                                }
                                else
                                {
                                        if (consume)
                                        {
                                                set_char_color(AtGold, ch);
                                                send_to_char
                                                        ("You feel a little lighter...\n\r",
                                                         ch);
                                                ch->gold -= value;
                                        }
                                        continue;
                                }
                        }
                        break;
                case 'H':
                        if (ch->hit >= value)
                        {
                                if (fail)
                                {
                                        send_to_char
                                                ("Something disrupts the use of this power...\n\r",
                                                 ch);
                                        return FALSE;
                                }
                                else
                                {
                                        if (consume)
                                        {
                                                set_char_color(AtBlood, ch);
                                                send_to_char
                                                        ("You feel a little weaker...\n\r",
                                                         ch);
                                                ch->hit -= value;
                                                update_pos(ch);
                                        }
                                        continue;
                                }
                        }
                        break;
                }
                /*
                 * having this component would make the spell fail... if we get
                 * here, then the caster didn't have that component 
                 */
                if (fail)
                        continue;
                if (!found)
                {
                        send_to_char("Something is missing...\n\r", ch);
                        return FALSE;
                }
                if (obj)
                {
                        if (val >= 0 && val < 6)
                        {
                                separate_obj(obj);
                                if (obj->value[val] <= 0)
                                        return FALSE;
                                else if (--obj->value[val] == 0)
                                {
                                        act(AtMagic,
                                            "$p glows briefly, then disappears in a puff of smoke!",
                                            ch, obj, NULL, ToChar);
                                        act(AtMagic,
                                            "$p glows briefly, then disappears in a puff of smoke!",
                                            ch, obj, NULL, ToRoom);
                                        extract_obj(obj);
                                }
                                else
                                        act(AtMagic,
                                            "$p glows briefly and a whisp of smoke rises from it.",
                                            ch, obj, NULL, ToChar);
                        }
                        else if (consume)
                        {
                                separate_obj(obj);
                                act(AtMagic,
                                    "$p glows brightly, then disappears in a puff of smoke!",
                                    ch, obj, NULL, ToChar);
                                act(AtMagic,
                                    "$p glows brightly, then disappears in a puff of smoke!",
                                    ch, obj, NULL, ToRoom);
                                extract_obj(obj);
                        }
                        else
                        {
                                int       count = obj->count;

                                obj->count = 1;
                                act(AtMagic, "$p glows briefly.", ch, obj,
                                    NULL, ToChar);
                                obj->count = count;
                        }
                }
        }
        return TRUE;
}

int       pAbort;

/*
 * Locate targets.
 */
void     *locate_targets(CharData * ch, char *arg, int sn,
                         CharData ** victim, ObjData ** obj)
{
        SkillType *skill = get_skilltype(sn);
        void     *vo = NULL;

        *victim = NULL;
        *obj = NULL;

        switch (skill->target)
        {
        default:
                bug("Do_cast: bad target for sn %d.", sn);
                return &pAbort;

        case TarIgnore:
                break;

        case TarCharOffensive:
                if (arg[0] == '\0')
                {
                        if ((*victim = who_fighting(ch)) == NULL)
                        {
                                send_to_char("Cast the spell on whom?\n\r",
                                             ch);
                                return &pAbort;
                        }
                }
                else
                {
                        if ((*victim = get_char_room(ch, arg)) == NULL)
                        {
                                send_to_char("They aren't here.\n\r", ch);
                                return &pAbort;
                        }
                }

                if (is_safe(ch, *victim))
                        return &pAbort;

                if (ch == *victim)
                {
                        send_to_char("Cast this on yourself?  Okay...\n\r",
                                     ch);
                        /*
                         * send_to_char( "You can't do that to yourself.\n\r", ch );
                         * return &pAbort;
                         */
                }

                if (!IsNpc(ch))
                {
                        if (!IsNpc(*victim))
                        {
                                /*
                                 * Sheesh! can't do anything
                                 * send_to_char( "You can't do that on a player.\n\r", ch );
                                 * return &pAbort;
                                 */

                                if (get_timer(ch, TimerPkilled) > 0)
                                {
                                        send_to_char
                                                ("You have been killed in the last 5 minutes.\n\r",
                                                 ch);
                                        return &pAbort;
                                }

                                if (get_timer(*victim, TimerPkilled) > 0)
                                {
                                        send_to_char
                                                ("This player has been killed in the last 5 minutes.\n\r",
                                                 ch);
                                        return &pAbort;
                                }

                        }

                        if (IsAffected(ch, AffCharm)
                            && ch->master == *victim)
                        {
                                send_to_char
                                        ("You can't do that on your own follower.\n\r",
                                         ch);
                                return &pAbort;
                        }
                }

                vo = (void *) *victim;
                break;

        case TarCharDefensive:
                if (arg[0] == '\0')
                        *victim = ch;
                else
                {
                        if ((*victim = get_char_room(ch, arg)) == NULL)
                        {
                                send_to_char("They aren't here.\n\r", ch);
                                return &pAbort;
                        }
                }
                vo = (void *) *victim;
                break;

        case TarCharSelf:
                if (arg[0] != '\0' && !nifty_is_name(arg, ch->name))
                {
                        send_to_char
                                ("You cannot cast this spell on another.\n\r",
                                 ch);
                        return &pAbort;
                }

                vo = (void *) ch;
                break;

        case TarObjInv:
                if (arg[0] == '\0')
                {
                        send_to_char
                                ("What should the spell be cast upon?\n\r",
                                 ch);
                        return &pAbort;
                }

                if ((*obj = get_obj_carry(ch, arg)) == NULL)
                {
                        send_to_char("You are not carrying that.\n\r", ch);
                        return &pAbort;
                }

                vo = (void *) *obj;
                break;
        }

        return vo;
}


/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char     *target_name;


/*
 * Cast a spell.  Multi-caster and component support by Thoric
 */
CMDF do_cast(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        static char staticbuf[MaxInputLength];
        CharData *victim;
        ObjData *obj;
        void     *vo = NULL;
        int       endurance;
        int       sn;
        ch_ret    retcode;
        bool      dont_wait = FALSE;
        SkillType *skill = NULL;
        struct timeval time_used;

        retcode = rNONE;

        if (!arena_can_fight(ch, victim))
        {
                send_to_char
                        ("You can't do that yet. Wait for betting to close.\n\r",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                /*
                 * no ordering charmed mobs to cast spells 
                 */
                if (IsNpc(ch) && IsAffected(ch, AffCharm))
                {
                        send_to_char
                                ("You can't seem to do that right now...\n\r",
                                 ch);
                        return;
                }

                if (xIS_SET(ch->in_room->RoomFlags, RoomNoMagic))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char("You failed.\n\r", ch);
                        return;
                }

                target_name = one_argument(argument, arg1);
                one_argument(target_name, arg2);

                if (arg1[0] == '\0')
                {
                        send_to_char("Cast which what where?\n\r", ch);
                        return;
                }

                if (get_trust(ch) < LevelGod)
                {
                        if ((sn = find_spell(ch, arg1, TRUE)) < 0
                            || (!IsNpc(ch) && ch->pcdata->learned[sn] <= 0))
                        {
                                send_to_char("You can't do that.\n\r", ch);
                                return;
                        }
                        if ((skill = get_skilltype(sn)) == NULL)
                        {
                                send_to_char
                                        ("You can't do that right now...\n\r",
                                         ch);
                                return;
                        }
                }
                else
                {
                        if ((sn = skill_lookup(arg1)) < 0)
                        {
                                send_to_char
                                        ("We didn't create that yet...\n\r",
                                         ch);
                                return;
                        }
                        if (sn >= MaxSkill)
                        {
                                send_to_char("Hmm... that might hurt.\n\r",
                                             ch);
                                return;
                        }
                        if ((skill = get_skilltype(sn)) == NULL)
                        {
                                send_to_char
                                        ("Somethis is severely wrong with that one...\n\r",
                                         ch);
                                return;
                        }
                        if (skill->type != SkillSpell)
                        {
                                send_to_char("That isn't a Force power.\n\r",
                                             ch);
                                return;
                        }
                        if (!skill->spell_fun)
                        {
                                send_to_char
                                        ("We didn't finish that one yet...\n\r",
                                         ch);
                                return;
                        }
                }

                /*
                 * Something else removed by Merc           -Thoric
                 */
                if (ch->position < skill->minimum_position)
                {
                        switch (ch->position)
                        {
                        default:
                                send_to_char
                                        ("You can't concentrate enough.\n\r",
                                         ch);
                                break;
                        case PosSitting:
                                send_to_char
                                        ("You can't summon enough energy sitting down.\n\r",
                                         ch);
                                break;
                        case PosResting:
                                send_to_char
                                        ("You're too relaxed to cast that spell.\n\r",
                                         ch);
                                break;
                        case PosFighting:
                                send_to_char
                                        ("You can't concentrate enough while fighting!\n\r",
                                         ch);
                                break;
                        case PosSleeping:
                                send_to_char
                                        ("You dream about great feats of magic.\n\r",
                                         ch);
                                break;
                        }
                        return;
                }

                if (skill->spell_fun == spell_null)
                {
                        send_to_char("That's not a spell!\n\r", ch);
                        return;
                }

                if (!skill->spell_fun)
                {
                        send_to_char("You cannot cast that... yet.\n\r", ch);
                        return;
                }
/* 
 * Redo for endurance use: get how far off of the target alignment they are. This will be at the most, 2000.
 * so to get a difference value, divide by 200, and multiply by skill->min_endurance to get the cost to that player
 */
                endurance =
                        IsNpc(ch) ? 0 : (skill_table[sn]->alignment > -300
                                          && skill_table[sn]->alignment <
                                          300) ? skill_table[sn]->
                        min_endurance
                        : (abs
                           (((skill_table[sn]->alignment -
                              ch->alignment) / 100)) +
                           1) * skill_table[sn]->min_endurance;

                /*
                 * Locate targets.
                 */
                vo = locate_targets(ch, arg2, sn, &victim, &obj);
                if (vo == &pAbort)
                        return;

                if (is_safe(ch, victim))
                {
                        set_char_color(AtMagic, ch);
                        send_to_char("You cannot do that to them.\n\r", ch);
                        return;
                }


                if (!IsNpc(ch) && ch->endurance < endurance)
                {
                        send_to_char
                                ("The Force is not strong enough within you.\n\r",
                                 ch);
                        return;
                }
                if (skill->participants <= 1)
                        break;
                /*
                 * multi-participant spells         -Thoric 
                 */
                add_timer(ch, TimerDoFun, UMIN(skill->beats / 10, 3),
                          do_cast, 1);
                act(AtMagic,
                    "You begin to feel the Force in yourself and those around you...",
                    ch, NULL, NULL, ToChar);
                act(AtMagic,
                    "$n reaches out with the Force to those around...", ch,
                    NULL, NULL, ToRoom);
                snprintf(staticbuf, MSL, "%s %s", arg2, target_name);
                ch->dest_buf = str_dup(staticbuf);
                ch->tempnum = sn;
                return;
        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                if (IsValidSn((sn = ch->tempnum)))
                {
                        if ((skill = get_skilltype(sn)) == NULL)
                        {
                                send_to_char("Something went wrong...\n\r",
                                             ch);
                                bug("do_cast: SubTimerDoAbort: bad sn %d",
                                    sn);
                                return;
                        }
                        endurance =
                                IsNpc(ch) ? 0 : (skill_table[sn]->alignment >
                                                  -300
                                                  && skill_table[sn]->
                                                  alignment <
                                                  300) ? skill_table[sn]->
                                min_endurance
                                : (abs
                                   (((skill_table[sn]->alignment -
                                      ch->alignment) / 100)) +
                                   1) * skill_table[sn]->min_endurance;

                        if (get_trust(ch) < LevelImmortal) /* so imms dont lose endurance */
                                ch->endurance -= endurance / 3;
                }
                set_char_color(AtMagic, ch);
                send_to_char("You stop your concentration\n\r", ch);
                /*
                 * should add percent_chance of backfire here 
                 */
                return;
        case 1:
                sn = ch->tempnum;
                if ((skill = get_skilltype(sn)) == NULL)
                {
                        send_to_char("Something went wrong...\n\r", ch);
                        bug("do_cast: substate 1: bad sn %d", sn);
                        return;
                }
                if (!ch->dest_buf || !IsValidSn(sn)
                    || skill->type != SkillSpell)
                {
                        send_to_char
                                ("Something negates the powers of the Force.\n\r",
                                 ch);
                        bug("do_cast: ch->dest_buf NULL or bad sn (%d)", sn);
                        return;
                }
                endurance =
                        IsNpc(ch) ? 0 : (skill_table[sn]->alignment > -300
                                          && skill_table[sn]->alignment <
                                          300) ? skill_table[sn]->
                        min_endurance
                        : (abs
                           (((skill_table[sn]->alignment -
                              ch->alignment) / 100)) +
                           1) * skill_table[sn]->min_endurance;
                mudstrlcpy(staticbuf, (const char *) ch->dest_buf, MIL);
                target_name = one_argument(staticbuf, arg2);
                DISPOSE(ch->dest_buf);
                ch->substate = SubNone;
                if (skill->participants > 1)
                {
                        int       cnt = 1;
                        CharData *tmp;
                        Timer    *t;

                        for (tmp = ch->in_room->first_person; tmp;
                             tmp = tmp->next_in_room)
                                if (tmp != ch
                                    && (t =
                                        get_timerptr(tmp,
                                                     TimerDoFun)) != NULL
                                    && t->count >= 1 && t->do_fun == do_cast
                                    && tmp->tempnum == sn && tmp->dest_buf
                                    && !str_cmp((const char *) tmp->dest_buf,
                                                staticbuf))
                                        ++cnt;
                        if (cnt >= skill->participants)
                        {
                                for (tmp = ch->in_room->first_person; tmp;
                                     tmp = tmp->next_in_room)
                                        if (tmp != ch
                                            && (t =
                                                get_timerptr(tmp,
                                                             TimerDoFun)) !=
                                            NULL && t->count >= 1
                                            && t->do_fun == do_cast
                                            && tmp->tempnum == sn
                                            && tmp->dest_buf
                                            && !str_cmp((const char *) tmp->
                                                        dest_buf, staticbuf))
                                        {
                                                extract_timer(tmp, t);
                                                act(AtMagic,
                                                    "Channeling your energy into $n, you help direct the Force",
                                                    ch, NULL, tmp, ToVict);
                                                act(AtMagic,
                                                    "$N channels $S energy into you!",
                                                    ch, NULL, tmp, ToChar);
                                                act(AtMagic,
                                                    "$N channels $S energy into $n!",
                                                    ch, NULL, tmp,
                                                    ToNotvict);
                                                learn_from_success(tmp, sn);

                                                tmp->endurance -= endurance;
                                                tmp->substate = SubNone;
                                                tmp->tempnum = -1;
                                                DISPOSE(tmp->dest_buf);
                                        }
                                dont_wait = TRUE;
                                send_to_char
                                        ("You concentrate all the energy into a burst of Force!\n\r",
                                         ch);
                                vo = locate_targets(ch, arg2, sn, &victim,
                                                    &obj);
                                if (vo == &pAbort)
                                        return;
                        }
                        else
                        {
                                set_char_color(AtMagic, ch);
                                send_to_char
                                        ("There was not enough power for that to succeed...\n\r",
                                         ch);

                                if (get_trust(ch) < LevelImmortal) /* so imms dont lose endurance */
                                        ch->endurance -= endurance / 2;
                                learn_from_failure(ch, sn);
                                return;
                        }
                }
        }

        if (str_cmp(skill->name, "ventriloquate"))
                say_spell(ch, sn);

        if (!dont_wait)
                WaitState(ch, skill->beats);

        /*
         * Getting ready to cast... check for spell components  -Thoric
         */
        if (!process_spell_components(ch, sn))
        {

                if (get_trust(ch) < LevelImmortal) /* so imms dont lose endurance */
                        ch->endurance -= endurance / 2;
                learn_from_failure(ch, sn);
                return;
        }
/* Redo for DW so that anyone can cast any spell, but it costs more the more your out of alignment.

    if ( !IsNpc(ch) && abs(ch->alignment - skill->alignment) > 1010 )
    { 
      if ( ch->alignment > skill->alignment  )
      {
          send_to_char( "You do not have enough anger in you.\n\r", ch );
	  if (get_trust(ch)  < LevelImmortal)   
	        ch->endurance -= endurance / 2;
	  return;	
      }
      if (  ch->alignment < skill->alignment )
      {
          send_to_char( "Your anger and hatred prevent you from focusing.\n\r", ch );
	  if (get_trust(ch)  < LevelImmortal)   
	        ch->endurance -= endurance / 2;
	  return;	
      }
    }    
*/

        if (!IsNpc(ch)
            && (number_percent() + skill->difficulty * 5) >
            ch->pcdata->learned[sn])
        {
                /*
                 * Some more interesting loss of concentration messages  -Thoric 
                 */
                switch (number_bits(2))
                {
                case 0:    /* too busy */
                        if (ch->fighting)
                                send_to_char
                                        ("This round of battle is too hectic to concentrate properly.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("You lost your concentration.\n\r",
                                         ch);
                        break;
                case 1:    /* irritation */
                        if (number_bits(2) == 0)
                        {
                                switch (number_bits(2))
                                {
                                case 0:
                                        send_to_char
                                                ("A tickle in your nose prevents you from keeping your concentration.\n\r",
                                                 ch);
                                        break;
                                case 1:
                                        send_to_char
                                                ("An itch on your leg keeps you from properly using the Force.\n\r",
                                                 ch);
                                        break;
                                case 2:
                                        send_to_char
                                                ("A nagging though prevents you from focusing on the Force.\n\r",
                                                 ch);
                                        break;
                                case 3:
                                        send_to_char
                                                ("A twitch in your eye disrupts your concentration for a moment.\n\r",
                                                 ch);
                                        break;
                                }
                        }
                        else
                                send_to_char
                                        ("Something distracts you, and you lose your concentration.\n\r",
                                         ch);
                        break;
                case 2:    /* not enough time */
                        if (ch->fighting)
                                send_to_char
                                        ("There wasn't enough time this round to complete your concentration.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("You lost your concentration.\n\r",
                                         ch);
                        break;
                case 3:
                        send_to_char
                                ("A disturbance in the Force muddles your concentration.\n\r",
                                 ch);
                        break;
                }

                if (get_trust(ch) < LevelImmortal) /* so imms dont lose endurance */
                        ch->endurance -= endurance;
                learn_from_failure(ch, sn);
                return;
        }
        else
        {

                ch->endurance -= endurance;

                /*
                 * check for immunity to magic if victim is known...
                 * and it is a TarCharDefensive/SELF spell
                 * otherwise spells will have to check themselves
                 */
                if ((skill->target == TarCharDefensive
                     || skill->target == TarCharSelf)
                    && victim && IsSet(victim->immune, RisMagic))
                {
                        immune_casting(skill, ch, victim, NULL);
                        retcode = rSPELL_FAILED;
                }
                else
                {
                        start_timer(&time_used);
                        retcode =
                                (*skill->spell_fun) (sn,
                                                     ch->
                                                     skill_level
                                                     [ForceAbility], ch, vo);
                        end_timer(&time_used);
                        update_userec(&time_used, &skill->userec);
                }
        }

        if (retcode == rCHAR_DIED || retcode == rERROR || char_died(ch))
                return;
        if (retcode != rSPELL_FAILED)
        {
                int       force_exp;

                force_exp = skill->min_level * skill->min_level * 10;
                force_exp =
                        URANGE(0, force_exp,
                               (exp_level(ch->skill_level[ForceAbility] + 1)
                                -
                                exp_level(ch->skill_level[ForceAbility])) /
                               35);
                if (!ch->fighting)
                        ch_printf(ch, "You gain %d Force experience.\n\r",
                                  force_exp);
                gain_exp(ch, force_exp, ForceAbility);
                learn_from_success(ch, sn);
        }
        else
                learn_from_failure(ch, sn);

        /*
         * Fixed up a weird mess here, and added double safeguards  -Thoric
         */
        if (skill->target == TarCharOffensive
            && victim && !char_died(victim) && victim != ch)
        {
                CharData *vch, *vch_next;

                for (vch = ch->in_room->first_person; vch; vch = vch_next)
                {
                        vch_next = vch->next_in_room;

                        if (vch == victim)
                        {
                                if (victim->master != ch && !victim->fighting)
                                        retcode =
                                                multi_hit(victim, ch,
                                                          TypeUndefined);
                                break;
                        }
                }
        }

        return;
}


/*
 * Cast spells at targets using a magical object.
 */
ch_ret obj_cast_spell(int sn, int level, CharData * ch, CharData * victim,
                      ObjData * obj)
{
        void     *vo;
        ch_ret    retcode = rNONE;
        int       levdiff = ch->top_level - level;
        SkillType *skill = get_skilltype(sn);
        struct timeval time_used;

        if (sn == -1)
                return retcode;
        if (!skill || !skill->spell_fun)
        {
                bug("Obj_cast_spell: bad sn %d.", sn);
                return rERROR;
        }

        if (xIS_SET(ch->in_room->RoomFlags, RoomNoMagic))
        {
                set_char_color(AtMagic, ch);
                send_to_char("Nothing seems to happen...\n\r", ch);
                return rNONE;
        }

        /*
         * Basically this was added to cut down on level 5 players using level
         * 40 scrolls in battle too often ;)        -Thoric
         */
        if ((skill->target == TarCharOffensive || number_bits(7) == 1)    /* 1/128 percent_chance if non-offensive */
            && !chance(ch, 95 + levdiff))
        {
                switch (number_bits(2))
                {
                case 0:
                        failed_casting(skill, ch, victim, NULL);
                        break;
                case 1:
                        act(AtMagic, "The $t backfires!", ch, skill->name,
                            victim, ToChar);
                        if (victim)
                                act(AtMagic, "$n's $t backfires!", ch,
                                    skill->name, victim, ToVict);
                        act(AtMagic, "$n's $t backfires!", ch, skill->name,
                            victim, ToNotvict);
                        return damage(ch, ch, number_range(1, level),
                                      TypeUndefined);
                case 2:
                        failed_casting(skill, ch, victim, NULL);
                        break;
                case 3:
                        act(AtMagic, "The $t backfires!", ch, skill->name,
                            victim, ToChar);
                        if (victim)
                                act(AtMagic, "$n's $t backfires!", ch,
                                    skill->name, victim, ToVict);
                        act(AtMagic, "$n's $t backfires!", ch, skill->name,
                            victim, ToNotvict);
                        return damage(ch, ch, number_range(1, level),
                                      TypeUndefined);
                }
                return rNONE;
        }

        target_name = "";
        switch (skill->target)
        {
        default:
                bug("Obj_cast_spell: bad target for sn %d.", sn);
                return rERROR;

        case TarIgnore:
                vo = NULL;
                if (victim)
                        target_name = victim->name;
                else if (obj)
                        target_name = obj->name;
                break;

        case TarCharOffensive:
                if (victim != ch)
                {
                        if (!victim)
                                victim = who_fighting(ch);
                        if (!victim || !IsNpc(victim))
                        {
                                send_to_char("You can't do that.\n\r", ch);
                                return rNONE;
                        }
                }
                if (ch != victim && is_safe(ch, victim))
                        return rNONE;
                vo = (void *) victim;
                break;

        case TarCharDefensive:
                if (victim == NULL)
                        victim = ch;
                vo = (void *) victim;
                if (IsSet(victim->immune, RisMagic))
                {
                        immune_casting(skill, ch, victim, NULL);
                        return rNONE;
                }
                break;

        case TarCharSelf:
                vo = (void *) ch;
                if (IsSet(ch->immune, RisMagic))
                {
                        immune_casting(skill, ch, victim, NULL);
                        return rNONE;
                }
                break;

        case TarObjInv:
                if (obj == NULL)
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return rNONE;
                }
                vo = (void *) obj;
                break;
        }

        start_timer(&time_used);
        retcode = (*skill->spell_fun) (sn, level, ch, vo);
        end_timer(&time_used);
        update_userec(&time_used, &skill->userec);

        if (retcode == rSPELL_FAILED)
                retcode = rNONE;

        if (retcode == rCHAR_DIED || retcode == rERROR)
                return retcode;

        if (char_died(ch))
                return rCHAR_DIED;

        if (skill->target == TarCharOffensive
            && victim != ch && !char_died(victim))
        {
                CharData *vch;
                CharData *vch_next;

                for (vch = ch->in_room->first_person; vch; vch = vch_next)
                {
                        vch_next = vch->next_in_room;
                        if (victim == vch && !victim->fighting
                            && victim->master != ch)
                        {
                                retcode =
                                        multi_hit(victim, ch, TypeUndefined);
                                break;
                        }
                }
        }

        return retcode;
}



/*
 * Spell functions.
 */
SPELLF spell_acid_blast(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);

        sith_penalty(ch);

        dam = dice(level, 6);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}




SPELLF spell_blindness(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        AffectData af;
        int       tmp;
        SkillType *skill = get_skilltype(sn);

        if (SpellFlag(skill, SfPksensitive)
            && !IsNpc(ch) && !IsNpc(victim))
                tmp = level;
        else
                tmp = level;

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }
        if (IsAffected(victim, AffBlind) || saves_spell_staff(tmp, victim))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        af.type = sn;
        af.location = ApplyHitroll;
        af.modifier = -4;
        af.duration = (int) ((1 + (level / 3)) * DurConv);
        af.bitvector = AffBlind;
        affect_to_char(victim, &af);
        set_char_color(AtMagic, victim);
        send_to_char("You are blinded!\n\r", victim);
        if (ch != victim)
                send_to_char("Ok.\n\r", ch);
        return rNONE;
}

SPELLF spell_cause_light(int sn, int level, CharData * ch, void *vo)
{
        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 50;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        return damage(ch, (CharData *) vo, dice(1, 8) + level / 3, sn);
}



SPELLF spell_cause_critical(int sn, int level, CharData * ch, void *vo)
{
        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 70;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        return damage(ch, (CharData *) vo, dice(3, 8) + level, sn);
}



SPELLF spell_cause_serious(int sn, int level, CharData * ch, void *vo)
{
        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 90;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        return damage(ch, (CharData *) vo, dice(level, 2), sn);
}


SPELLF spell_change_sex(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        AffectData af;
        SkillType *skill = get_skilltype(sn);

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }
        if (is_affected(victim, sn))
                return rSPELL_FAILED;
        af.type = sn;
        af.duration = (int) (10 * level * DurConv);
        af.location = ApplySex;
        do
        {
                af.modifier = number_range(0, 2) - victim->sex;
        }
        while (af.modifier == 0);
        af.bitvector = 0;
        affect_to_char(victim, &af);
        set_char_color(AtMagic, victim);
        send_to_char("You feel different.\n\r", victim);
        if (ch != victim)
                send_to_char("Ok.\n\r", ch);
        return rNONE;
}


SPELLF spell_charm_person(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        AffectData af;
        int       percent_chance;
        char      buf[MaxStringLength];
        SkillType *skill = get_skilltype(sn);

        if (victim == ch)
        {
                send_to_char("You like yourself even better!\n\r", ch);
                return rSPELL_FAILED;
        }

        if (IsSet(victim->immune, RisMagic)
            || IsSet(victim->immune, RisCharm))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (!IsNpc(victim) && !IsNpc(ch))
        {
                send_to_char("I don't think so...\n\r", ch);
                send_to_char
                        ("You feel as if someone tried to enter your mind but failed..\n\r",
                         victim);
                return rSPELL_FAILED;
        }

        percent_chance = ris_save(victim, level, RisCharm);

        if (IsAffected(victim, AffCharm)
            || percent_chance == 1000
            || IsAffected(ch, AffCharm)
            || level < victim->top_level
            || circle_follow(victim, ch)
            || saves_spell_staff(percent_chance, victim))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (victim->master)
                stop_follower(victim);
        add_follower(victim, ch);
        af.type = sn;
        af.duration = (int) ((number_fuzzy((level + 1) / 3) + 1) * DurConv);
        af.location = 0;
        af.modifier = 0;
        af.bitvector = AffCharm;
        affect_to_char(victim, &af);
        act(AtMagic, "Isn't $n just so nice?", ch, NULL, victim, ToVict);
        act(AtMagic, "$N's eyes glaze over...", ch, NULL, victim, ToRoom);
        if (ch != victim)
                send_to_char("Ok.\n\r", ch);

        snprintf(buf, MSL, "%s has charmed %s.", ch->name, victim->name);
        log_string_plus(buf, LogNormal, ch->top_level);
        return rNONE;
}

SPELLF spell_cure_blindness(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        SkillType *skill = get_skilltype(sn);

        level = 0;

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (!is_affected(victim, gsn_blindness))
                return rSPELL_FAILED;

        if (ch != victim)
        {
                send_to_char
                        ("The noble Jedi use their powers to help others!\n\r",
                         ch);
                ch->alignment = ch->alignment + 25;
                ch->alignment = URANGE(-1000, ch->alignment, 1000);
                jedi_bonus(ch);
        }

        affect_strip(victim, gsn_blindness);
        set_char_color(AtMagic, victim);
        send_to_char("Your vision returns!\n\r", victim);
        if (ch != victim)
                send_to_char("Ok.\n\r", ch);
        return rNONE;
}


SPELLF spell_cure_poison(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        SkillType *skill = get_skilltype(sn);

        level = 0;

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (is_affected(victim, gsn_poison))
        {
                if (ch != victim)
                {
                        send_to_char
                                ("The nobel Jedi use their powers to help others!\n\r",
                                 ch);
                        ch->alignment = ch->alignment + 25;
                        ch->alignment = URANGE(-1000, ch->alignment, 1000);
                        jedi_bonus(ch);
                }

                affect_strip(victim, gsn_poison);
                act(AtMagic, "$N looks better.", ch, NULL, victim,
                    ToNotvict);
                set_char_color(AtMagic, victim);
                send_to_char("A warm feeling runs through your body.\n\r",
                             victim);
                victim->mental_state =
                        URANGE(-100, victim->mental_state, -10);
                send_to_char("Ok.\n\r", ch);
                return rNONE;
        }
        else
                return rSPELL_FAILED;
}


SPELLF spell_curse(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        AffectData af;
        SkillType *skill = get_skilltype(sn);

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (IsAffected(victim, AffCurse)
            || saves_spell_staff(level, victim))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }
        af.type = sn;
        af.duration = (int) ((4 * level) * DurConv);
        af.location = ApplyHitroll;
        af.modifier = -1;
        af.bitvector = AffCurse;
        affect_to_char(victim, &af);

        af.location = ApplySavingSpell;
        af.modifier = 1;
        affect_to_char(victim, &af);

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 50;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        set_char_color(AtMagic, victim);
        send_to_char("You feel unclean.\n\r", victim);
        if (ch != victim)
                send_to_char("Ok.\n\r", ch);
        return rNONE;
}


SPELLF spell_detect_poison(int sn, int level, CharData * ch, void *vo)
{
        ObjData *obj = (ObjData *) vo;

        sn = level = 0;

        set_char_color(AtMagic, ch);
        if (obj->item_type == ItemDrinkCon || obj->item_type == ItemFood)
        {
                if (obj->value[3] != 0)
                        send_to_char("You smell poisonous fumes.\n\r", ch);
                else
                        send_to_char("It looks very delicious.\n\r", ch);
        }
        else
        {
                send_to_char("It doesn't look poisoned.\n\r", ch);
        }

        return rNONE;
}


SPELLF spell_dispel_evil(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;
        SkillType *skill = get_skilltype(sn);

        if (!IsNpc(ch) && IsEvil(ch))
                victim = ch;

        if (IsGood(victim))
        {
                act(AtMagic, "The light side protects $N.", ch, NULL, victim,
                    ToRoom);
                return rSPELL_FAILED;
        }

        if (IsNeutral(victim))
        {
                act(AtMagic, "$N does not seem to be affected.", ch, NULL,
                    victim, ToChar);
                return rSPELL_FAILED;
        }

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        dam = dice(level, 4);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}


SPELLF spell_dispel_magic(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       affected_by, cnt;
        SkillType *skill = get_skilltype(sn);

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (IsNpc(victim) && IsAffected(victim, AffPossess))
        {
                immune_casting(skill, ch, victim, NULL);
                return rVICT_IMMUNE;
        }

        if ((victim->affected_by || victim->first_affect) && ch == victim)
        {
                set_char_color(AtMagic, ch);
                send_to_char("You pass your hands around your body...\n\r",
                             ch);
                while (victim->first_affect)
                        affect_remove(victim, victim->first_affect);
                victim->affected_by = victim->race->affected();
                return rNONE;
        }
        else if (victim->affected_by == victim->race->affected()
                 || level < victim->top_level
                 || saves_spell_staff(level, victim))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (!IsNpc(victim))
        {
                send_to_char("You can't do that... yet.\n\r", ch);
                return rSPELL_FAILED;
        }

        cnt = 0;
        for (;;)
        {
                affected_by = 1 << number_bits(5);
                if (IsSet(victim->affected_by, affected_by))
                        break;
                if (cnt++ > 30)
                {
                        failed_casting(skill, ch, victim, NULL);
                        return rNONE;
                }
        }
        RemoveBit(victim->affected_by, affected_by);
        successful_casting(skill, ch, victim, NULL);

        return rNONE;
}



SPELLF spell_earthquake(int sn, int level, CharData * ch, void *vo)
{
        CharData *vch;
        CharData *vch_next;
        bool      ch_died;
        ch_ret    retcode;
        SkillType *skill = get_skilltype(sn);

        vo = NULL;

        ch_died = FALSE;
        retcode = rNONE;

        if (xIS_SET(ch->in_room->RoomFlags, RoomSafe))
        {
                failed_casting(skill, ch, NULL, NULL);
                return rSPELL_FAILED;
        }

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        act(AtMagic, "The earth trembles beneath your feet!", ch, NULL, NULL,
            ToChar);
        act(AtMagic, "$n makes the earth tremble and shiver.", ch, NULL,
            NULL, ToRoom);

        for (vch = first_char; vch; vch = vch_next)
        {
                vch_next = vch->next;
                if (!vch->in_room)
                        continue;
                if (vch->in_room == ch->in_room)
                {
                        if (!IsNpc(vch) && IsSet(vch->act, PlrWizinvis)
                            && vch->pcdata->wizinvis >= LevelImmortal)
                                continue;

                        if (IsAffected(vch, AffFloating)
                            || IsAffected(vch, AffFlying))
                                continue;

                        if (ch == vch)
                                continue;

                        retcode = damage(ch, vch, level + dice(2, 8), sn);
                        if (retcode == rCHAR_DIED || char_died(ch))
                        {
                                ch_died = TRUE;
                                continue;
                        }
                        if (char_died(vch))
                                continue;
                }

                if (!ch_died && vch->in_room->area == ch->in_room->area)
                {
                        set_char_color(AtMagic, vch);
                        send_to_char("The earth trembles and shivers.\n\r",
                                     vch);
                }
        }

        if (ch_died)
                return rCHAR_DIED;
        else
                return rNONE;
}


SPELLF spell_enchant_weapon(int sn, int level, CharData * ch, void *vo)
{
        ObjData *obj = (ObjData *) vo;
        AffectData *paf;

        sn = 0;

        if (obj->item_type != ItemWeapon
            || IsObjStat(obj, ItemMagic) || obj->first_affect)
                return rSPELL_FAILED;

        /*
         * Bug fix here. -- Alty 
         */
        separate_obj(obj);
        CREATE(paf, AffectData, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = ApplyHitroll;
        paf->modifier = level / 15;
        paf->bitvector = 0;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);

        CREATE(paf, AffectData, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = ApplyDamroll;
        paf->modifier = level / 15;
        paf->bitvector = 0;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);

        if (IsGood(ch))
        {
                SetBit(obj->extra_flags, ItemAntiEvil);
                act(AtBlue, "$p glows blue.", ch, obj, NULL, ToChar);
        }
        else if (IsEvil(ch))
        {
                SetBit(obj->extra_flags, ItemAntiGood);
                act(AtRed, "$p glows red.", ch, obj, NULL, ToChar);
        }
        else
        {
                SetBit(obj->extra_flags, ItemAntiEvil);
                SetBit(obj->extra_flags, ItemAntiGood);
                act(AtYellow, "$p glows yellow.", ch, obj, NULL, ToChar);
        }

        send_to_char("Ok.\n\r", ch);
        return rNONE;
}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
SPELLF spell_energy_drain(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;
        int       percent_chance;
        SkillType *skill = get_skilltype(sn);

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 200;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        percent_chance = ris_save(victim, victim->top_level, RisDrain);
        if (percent_chance == 1000
            || saves_spell_staff(percent_chance, victim))
        {
                failed_casting(skill, ch, victim, NULL);    /* SB */
                return rSPELL_FAILED;
        }

        if (victim->top_level <= 2)
                dam = ch->hit + 1;
        else
        {
                victim->endurance /= 2;
                dam = dice(1, level);
                ch->hit += dam;
        }

        if (ch->hit > ch->max_hit)
                ch->hit = ch->max_hit;
        return damage(ch, victim, dam, sn);
}



SPELLF spell_fireball(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;

        static const sh_int dam_each[] = {
                1,
                1, 4, 7, 10, 13, 16, 19, 22, 25, 28,
                31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
                44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
                51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
                58, 58, 59, 60, 60, 61, 62, 62, 63, 64,
                64, 65, 65, 66, 66, 67, 68, 68, 69, 69,
                70, 71, 71, 72, 72, 73, 73, 74, 75, 75
        };
        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        level = UMIN(level,
                     (int) sizeof(dam_each) / (int) sizeof(dam_each[0]) - 1);
        level = UMAX(0, level);
        dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}



SPELLF spell_flamestrike(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        dam = dice(6, 8);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}

SPELLF spell_harm(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;
        SkillType *skill = get_skilltype(sn);

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        dam = UMAX(20, victim->hit - dice(1, 4));
        if (saves_spell_staff(level, victim))
                dam = UMIN(50, dam / 4);
        dam = UMIN(100, dam);
        return damage(ch, victim, dam, sn);
}


SPELLF spell_identify(int sn, int level, CharData * ch, void *vo)
{
/* Modified by Scryn to work on mobs/players/objs */
        ObjData *obj;
        CharData *victim;
        AffectData *paf;
        SkillType *sktmp;
        SkillType *skill = get_skilltype(sn);

        level = 0;
        vo = NULL;

        if (target_name[0] == '\0')
        {
                send_to_char("What would you like identified?\n\r", ch);
                return rSPELL_FAILED;
        }

        if ((obj = get_obj_carry(ch, target_name)) != NULL)
        {
                set_char_color(AtLblue, ch);
                ch_printf(ch,
                          "Object '%s' is %s, special properties: %s %s.\n\rIts weight is %d, value is %d.\n\r",
                          obj->name,
                          aoran(item_type_name(obj)),
                          extra_bit_name(obj->extra_flags),
                          magic_bit_name(obj->magic_flags),
                          obj->weight, obj->cost);
                set_char_color(AtMagic, ch);

                switch (obj->item_type)
                {
                case ItemPill:
                case ItemScroll:
                case ItemPotion:
                        ch_printf(ch, "Level %d spells of:", obj->value[0]);

                        if (obj->value[1] >= 0
                            && (sktmp = get_skilltype(obj->value[1])) != NULL)
                        {
                                send_to_char(" '", ch);
                                send_to_char(sktmp->name, ch);
                                send_to_char("'", ch);
                        }

                        if (obj->value[2] >= 0
                            && (sktmp = get_skilltype(obj->value[2])) != NULL)
                        {
                                send_to_char(" '", ch);
                                send_to_char(sktmp->name, ch);
                                send_to_char("'", ch);
                        }

                        if (obj->value[3] >= 0
                            && (sktmp = get_skilltype(obj->value[3])) != NULL)
                        {
                                send_to_char(" '", ch);
                                send_to_char(sktmp->name, ch);
                                send_to_char("'", ch);
                        }

                        send_to_char(".\n\r", ch);
                        break;

                case ItemDevice:
                        ch_printf(ch, "Has %d(%d) charges of level %d",
                                  obj->value[1], obj->value[2],
                                  obj->value[0]);

                        if (obj->value[3] >= 0
                            && (sktmp = get_skilltype(obj->value[3])) != NULL)
                        {
                                send_to_char(" '", ch);
                                send_to_char(sktmp->name, ch);
                                send_to_char("'", ch);
                        }

                        send_to_char(".\n\r", ch);
                        break;

                case ItemWeapon:
                        ch_printf(ch, "Damage is %d to %d (average %d).\n\r",
                                  obj->value[1], obj->value[2],
                                  (obj->value[1] + obj->value[2]) / 2);
                        if (obj->value[3] == WeaponBlaster)
                        {
                                if (obj->blaster_setting == BlasterFull)
                                        ch_printf(ch,
                                                  "It is set on FULL power.\n\r");
                                else if (obj->blaster_setting == BlasterHigh)
                                        ch_printf(ch,
                                                  "It is set on HIGH power.\n\r");
                                else if (obj->blaster_setting ==
                                         BlasterNormal)
                                        ch_printf(ch,
                                                  "It is set on NORMAL power.\n\r");
                                else if (obj->blaster_setting == BlasterHalf)
                                        ch_printf(ch,
                                                  "It is set on HALF power.\n\r");
                                else if (obj->blaster_setting == BlasterLow)
                                        ch_printf(ch,
                                                  "It is set on LOW power.\n\r");
                                else if (obj->blaster_setting == BlasterStun)
                                        ch_printf(ch,
                                                  "It is set on STUN.\n\r");
                                ch_printf(ch,
                                          "It has %d out of %d charges.\n\r",
                                          obj->value[4], obj->value[5]);
                        }
                        else if (obj->value[3] == WeaponLightsaber ||
                                 obj->value[3] == WeaponVibroBlade ||
                                 obj->value[3] == WeaponForcePike)
                        {
                                ch_printf(ch,
                                          "It has %d out of %d units of charge remaining.\n\r",
                                          obj->value[4], obj->value[5]);
                        }
                        else if (obj->value[3] == WeaponBowcaster)
                        {
                                ch_printf(ch,
                                          "It has %d out of %d energy bolts remaining.\n\r",
                                          obj->value[4], obj->value[5]);
                        }
                        break;

                case ItemAmmo:
                        ch_printf(ch, "It has %d charges.\n\r",
                                  obj->value[0]);
                        break;

                case ItemBolt:
                        ch_printf(ch, "It has %d energy bolts.\n\r",
                                  obj->value[0]);
                        break;

                case ItemBattery:
                        ch_printf(ch, "It has %d units of charge.\n\r",
                                  obj->value[0]);
                        break;

                case ItemArmor:
                        ch_printf(ch,
                                  "Current Armor class is %d. ( based on current condition )\n\r",
                                  obj->value[0]);
                        ch_printf(ch,
                                  "Maximum Armor class is %d. ( based on top condition )\n\r",
                                  obj->value[1]);
                        ch_printf(ch,
                                  "Applied Armor class is %d. ( based condition and location worn )\n\r",
                                  apply_ac(obj, obj->wear_loc));
                        break;
                }

                for (paf = obj->pIndexData->first_affect; paf;
                     paf = paf->next)
                        showaffect(ch, paf);

                for (paf = obj->first_affect; paf; paf = paf->next)
                        showaffect(ch, paf);

                return rNONE;
        }

        else if ((victim = get_char_room(ch, target_name)) != NULL)
        {

                if (IsSet(victim->immune, RisMagic))
                {
                        immune_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }

                if (IsNpc(victim))
                {
                        ch_printf(ch,
                                  "%s appears to be between level %d and %d.\n\r",
                                  victim->name,
                                  victim->top_level - (victim->top_level % 5),
                                  victim->top_level -
                                  (victim->top_level % 5) + 5);
                }
                else
                {
                        ch_printf(ch, "%s appears to be level %d.\n\r",
                                  victim->name, victim->top_level);
                }

                ch_printf(ch, "%s looks like %s.\n\r",
                          victim->name, victim->race->name());

                if ((chance(ch, 50)
                     && ch->top_level >= victim->top_level + 10)
                    || IsImmortal(ch))
                {
                        ch_printf(ch, "%s appears to be affected by: ",
                                  victim->name);

                        if (!victim->first_affect)
                        {
                                send_to_char("nothing.\n\r", ch);
                                return rNONE;
                        }

                        for (paf = victim->first_affect; paf; paf = paf->next)
                        {
                                if (victim->first_affect !=
                                    victim->last_affect)
                                {
                                        if (paf != victim->last_affect
                                            && (sktmp =
                                                get_skilltype(paf->type)) !=
                                            NULL)
                                                ch_printf(ch, "%s, ",
                                                          sktmp->name);

                                        if (paf == victim->last_affect
                                            && (sktmp =
                                                get_skilltype(paf->type)) !=
                                            NULL)
                                        {
                                                ch_printf(ch, "and %s.\n\r",
                                                          sktmp->name);
                                                return rNONE;
                                        }
                                }
                                else
                                {
                                        if ((sktmp =
                                             get_skilltype(paf->type)) !=
                                            NULL)
                                                ch_printf(ch, "%s.\n\r",
                                                          sktmp->name);
                                        else
                                                send_to_char("\n\r", ch);
                                        return rNONE;
                                }
                        }
                }
        }

        else
        {
                ch_printf(ch, "You can't find %s!\n\r", target_name);
                return rSPELL_FAILED;
        }
        return rNONE;
}



SPELLF spell_invis(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim;
        SkillType *skill = get_skilltype(sn);

        vo = NULL;

/* Modifications on 1/2/96 to work on player/object - Scryn */

        if (target_name[0] == '\0')
                victim = ch;
        else
                victim = get_char_room(ch, target_name);

        if (victim)
        {
                AffectData af;

                if (IsSet(victim->immune, RisMagic))
                {
                        immune_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }

                if (IsAffected(victim, AffInvisible))
                {
                        failed_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }

                act(AtMagic, "$n fades out of existence.", victim, NULL,
                    NULL, ToRoom);
                af.type = sn;
                af.duration = (int) (((level / 4) + 12) * DurConv);
                af.location = ApplyNone;
                af.modifier = 0;
                af.bitvector = AffInvisible;
                affect_to_char(victim, &af);
                act(AtMagic, "You fade out of existence.", victim, NULL,
                    NULL, ToChar);
                return rNONE;
        }
        else
        {
                ObjData *obj;

                obj = get_obj_carry(ch, target_name);

                if (obj)
                {
                        if (IsObjStat(obj, ItemInvis)
                            || chance(ch, 40 + level / 10))
                        {
                                failed_casting(skill, ch, NULL, NULL);
                                return rSPELL_FAILED;
                        }

                        SetBit(obj->extra_flags, ItemInvis);
                        act(AtMagic, "$p fades out of existence.", ch, obj,
                            NULL, ToChar);
                        return rNONE;
                }
        }
        ch_printf(ch, "You can't find %s!\n\r", target_name);
        return rSPELL_FAILED;
}



SPELLF spell_know_alignment(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        char     *msg;
        int       ap;
        SkillType *skill = get_skilltype(sn);

        level = 0;

        if (!victim)
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        ap = victim->alignment;

        if (ap > 700)
                msg = "$N has an aura as white as the driven snow.";
        else if (ap > 350)
                msg = "$N is of excellent moral character.";
        else if (ap > 100)
                msg = "$N is often kind and thoughtful.";
        else if (ap > -100)
                msg = "$N doesn't have a firm moral commitment.";
        else if (ap > -350)
                msg = "$N lies to $S friends.";
        else if (ap > -700)
                msg = "$N's slash DISEMBOWELS you!";
        else
                msg = "I'd rather just not say anything at all about $N.";

        act(AtMagic, msg, ch, NULL, victim, ToChar);
        return rNONE;
}


SPELLF spell_lightning_bolt(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        static const sh_int dam_each[] = {
                1,
                2, 4, 6, 8, 10, 12, 14, 16, 18, 20,
                22, 24, 26, 28, 30, 35, 40, 45, 50, 55,
                60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
                92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
                112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
                132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
                152, 154, 156, 158, 160, 162, 164, 166, 168, 170
        };

        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        level = UMIN(level,
                     (int) sizeof(dam_each) / (int) sizeof(dam_each[0]) - 1);
        level = UMAX(0, level);
        dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}



SPELLF spell_locate_object(int sn, int level, CharData * ch, void *vo)
{
        char      buf[MaxInputLength];
        ObjData *obj;
        ObjData *in_obj;
        bool      found;
        int       cnt;

        sn = level = 0;
        vo = NULL;

        found = FALSE;
        for (obj = first_object; obj; obj = obj->next)
        {
                if (!can_see_obj(ch, obj)
                    || !nifty_is_name(target_name, obj->name))
                        continue;
                if (IsObjStat(obj, ItemPrototype) && !IsImmortal(ch))
                        continue;

                found = TRUE;

                for (cnt = 0, in_obj = obj;
                     in_obj->in_obj && cnt < 100;
                     in_obj = in_obj->in_obj, ++cnt)
                        ;
                if (cnt >= MaxNest)
                {
                        snprintf(buf, MSL,
                                 "spell_locate_obj: object [%d] %s is nested more than %d times!",
                                 obj->pIndexData->vnum, obj->short_descr,
                                 MaxNest);
                        bug(buf, 0);
                        continue;
                }

                if (in_obj->carried_by)
                {
                        if (IsImmortal(in_obj->carried_by)
                            && !IsNpc(in_obj->carried_by)
                            && (get_trust(ch) <
                                in_obj->carried_by->pcdata->wizinvis)
                            && IsSet(in_obj->carried_by->act, PlrWizinvis))
                                continue;

                        snprintf(buf, MSL, "%s carried by %s.\n\r",
                                 obj_short(obj), PERS(in_obj->carried_by,
                                                      ch));
                }
                else
                {
                        snprintf(buf, MSL, "%s in %s.\n\r",
                                 obj_short(obj), in_obj->in_room == NULL
                                 ? "somewhere" : in_obj->in_room->name);
                }

                buf[0] = UPPER(buf[0]);
                set_char_color(AtMagic, ch);
                send_to_char(buf, ch);
        }

        if (!found)
        {
                send_to_char("Nothing like that exists.\n\r", ch);
                return rSPELL_FAILED;
        }
        return rNONE;
}



SPELLF spell_magic_missile(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        static const sh_int dam_each[] = {
                1,
                3, 3, 4, 4, 5, 6, 6, 6, 6, 6,
                7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
                9, 9, 9, 9, 9, 10, 10, 10, 10, 10,
                11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
                13, 13, 13, 13, 13, 14, 14, 14, 14, 14,
                15, 15, 15, 15, 15, 16, 16, 16, 16, 16,
                17, 17, 17, 17, 17, 18, 18, 18, 18, 18
        };
        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        level = UMIN(level,
                     (int) sizeof(dam_each) / (int) sizeof(dam_each[0]) - 1);
        level = UMAX(0, level);
        dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
        /*
         * What's this?  You can't save vs. magic missile!      -Thoric
         * if ( saves_spell( level, victim ) )
         * dam /= 2;
         */
        return damage(ch, victim, dam, sn);
}




SPELLF spell_pass_door(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        AffectData af;
        SkillType *skill = get_skilltype(sn);

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (IsAffected(victim, AffPassDoor))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }
        af.type = sn;
        af.duration = (int) (number_fuzzy(level / 4) * DurConv);
        af.location = ApplyNone;
        af.modifier = 0;
        af.bitvector = AffPassDoor;
        affect_to_char(victim, &af);
        act(AtMagic, "$n turns translucent.", victim, NULL, NULL, ToRoom);
        act(AtMagic, "You turn translucent.", victim, NULL, NULL, ToChar);
        return rNONE;
}



SPELLF spell_poison(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        AffectData af;
        int       percent_chance;
        bool      first = TRUE;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        percent_chance = ris_save(victim, level, RisPoison);
        if (percent_chance == 1000
            || saves_poison_death(percent_chance, victim))
                return rSPELL_FAILED;
        if (IsAffected(victim, AffPoison))
                first = FALSE;
        af.type = sn;
        af.duration = (int) (level * DurConv);
        af.location = ApplyStr;
        af.modifier = -2;
        af.bitvector = AffPoison;
        affect_join(victim, &af);
        set_char_color(AtMagic, victim);
        send_to_char("You feel very sick.\n\r", victim);
        victim->mental_state = URANGE(20, victim->mental_state
                                      + (first ? 5 : 0), 100);
        if (ch != victim)
                send_to_char("Ok.\n\r", ch);
        return rNONE;
}

SPELLF spell_remove_trap(int sn, int level, CharData * ch, void *vo)
{
        ObjData *obj;
        ObjData *trap;
        bool      found;
        int       retcode;
        SkillType *skill = get_skilltype(sn);

        level = 0;
        vo = NULL;

        if (!target_name || target_name[0] == '\0')
        {
                send_to_char("Remove trap on what?\n\r", ch);
                return rSPELL_FAILED;
        }

        found = FALSE;

        if (!ch->in_room->first_content)
        {
                send_to_char("You can't find that here.\n\r", ch);
                return rNONE;
        }

        for (obj = ch->in_room->first_content; obj; obj = obj->next_content)
                if (can_see_obj(ch, obj)
                    && nifty_is_name(target_name, obj->name))
                {
                        found = TRUE;
                        break;
                }

        if (!found)
        {
                send_to_char("You can't find that here.\n\r", ch);
                return rSPELL_FAILED;
        }

        if ((trap = get_trap(obj)) == NULL)
        {
                failed_casting(skill, ch, NULL, NULL);
                return rSPELL_FAILED;
        }


        if (chance(ch, 70 + get_curr_wis(ch)))
        {
                send_to_char("Ooops!\n\r", ch);
                retcode = spring_trap(ch, trap);
                if (retcode == rNONE)
                        retcode = rSPELL_FAILED;
                return retcode;
        }

        extract_obj(trap);

        successful_casting(skill, ch, NULL, NULL);
        return rNONE;
}


SPELLF spell_shocking_grasp(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        static const int dam_each[] = {
                1,
                2, 4, 6, 8, 10, 15, 20, 25, 29, 33,
                36, 39, 39, 39, 40, 40, 41, 41, 42, 42,
                43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
                48, 48, 49, 49, 50, 50, 51, 51, 52, 52,
                53, 53, 54, 54, 55, 55, 56, 56, 57, 57,
                58, 58, 59, 59, 60, 60, 61, 61, 62, 62,
                63, 63, 64, 64, 65, 65, 66, 66, 67, 67
        };
        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        level = UMIN(level,
                     (int) sizeof(dam_each) / (int) sizeof(dam_each[0]) - 1);
        level = UMAX(0, level);
        dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}



SPELLF spell_sleep(int sn, int level, CharData * ch, void *vo)
{
        AffectData af;
        int       retcode;
        int       percent_chance;
        int       tmp;
        CharData *victim;
        SkillType *skill = get_skilltype(sn);

        vo = NULL;

        if ((victim = get_char_room(ch, target_name)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return rSPELL_FAILED;
        }

        if (!IsNpc(victim) && victim->fighting)
        {
                send_to_char("You cannot sleep a fighting player.\n\r", ch);
                return rSPELL_FAILED;
        }

        if (is_safe(ch, victim))
                return rSPELL_FAILED;

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (SpellFlag(skill, SfPksensitive)
            && !IsNpc(ch) && !IsNpc(victim))
                tmp = level;
        else
                tmp = level;

        if (IsAffected(victim, AffSleep)
            || (percent_chance = ris_save(victim, tmp, RisSleep)) == 1000
            || level < victim->top_level
            || (victim != ch
                && xIS_SET(victim->in_room->RoomFlags, RoomSafe))
            || saves_spell_staff(percent_chance, victim))
        {
                failed_casting(skill, ch, victim, NULL);
                if (ch == victim)
                        return rSPELL_FAILED;
                if (!victim->fighting)
                {
                        retcode = multi_hit(victim, ch, TypeUndefined);
                        if (retcode == rNONE)
                                retcode = rSPELL_FAILED;
                        return retcode;
                }
        }
        af.type = sn;
        af.duration = (int) ((4 + level) * DurConv);
        af.location = ApplyNone;
        af.modifier = 0;
        af.bitvector = AffSleep;
        affect_join(victim, &af);

        /*
         * Added by Narn at the Request of Dominus. 
         */
        if (!IsNpc(victim))
        {
                snprintf(log_buf, MSL, "%s has cast sleep on %s.", ch->name,
                         victim->name);
                log_string_plus(log_buf, LogNormal, ch->top_level);
        }

        if (IsAwake(victim))
        {
                act(AtMagic, "You feel very sleepy ..... zzzzzz.", victim,
                    NULL, NULL, ToChar);
                act(AtMagic, "$n goes to sleep.", victim, NULL, NULL,
                    ToRoom);
                victim->position = PosSleeping;
        }
        if (IsNpc(victim))
                start_hating(victim, ch);

        return rNONE;
}

SPELLF spell_ventriloquate(int sn, int level, CharData * ch, void *vo)
{
        char      buf1[MaxStringLength];
        char      buf2[MaxStringLength];
        char      speaker[MaxInputLength];
        CharData *vch;

        sn = 0;
        vo = NULL;

        target_name = one_argument(target_name, speaker);

        snprintf(buf1, MSL, "%s says '%s'.\n\r", speaker, target_name);
        snprintf(buf2, MSL, "Someone makes %s say '%s'.\n\r", speaker,
                 target_name);
        buf1[0] = UPPER(buf1[0]);

        for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
        {
                if (!is_name(speaker, vch->name))
                {
                        set_char_color(AtSay, vch);
                        send_to_char(saves_spell_staff(level, vch) ? buf2 :
                                     buf1, vch);
                }
        }

        return rNONE;
}



SPELLF spell_weaken(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        AffectData af;
        SkillType *skill = get_skilltype(sn);

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }
        if (is_affected(victim, sn) || saves_wands(level, victim))
                return rSPELL_FAILED;
        af.type = sn;
        af.duration = (int) (level / 2 * DurConv);
        af.location = ApplyStr;
        af.modifier = -2;
        af.bitvector = 0;
        affect_to_char(victim, &af);
        set_char_color(AtMagic, victim);
        send_to_char("You feel weaker.\n\r", victim);
        if (ch != victim)
                send_to_char("Ok.\n\r", ch);
        return rNONE;
}

/*
 * NPC spells.
 */
SPELLF spell_acid_breath(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        ObjData *obj_lose;
        ObjData *obj_next;
        int       dam;
        int       hpch;

        if (chance(ch, 2 * level) && !saves_breath(level, victim))
        {
                for (obj_lose = victim->first_carrying; obj_lose;
                     obj_lose = obj_next)
                {
                        int       iWear;

                        obj_next = obj_lose->next_content;

                        if (number_bits(2) != 0)
                                continue;

                        switch (obj_lose->item_type)
                        {
                        case ItemArmor:
                                if (obj_lose->value[0] > 0)
                                {
                                        separate_obj(obj_lose);
                                        act(AtDamage,
                                            "$p is pitted and etched!",
                                            victim, obj_lose, NULL, ToChar);
                                        if ((iWear =
                                             obj_lose->wear_loc) != WearNone)
                                                victim->Armor -=
                                                        apply_ac(obj_lose,
                                                                 iWear);
                                        obj_lose->value[0] -= 1;
                                        obj_lose->cost = 0;
                                        if (iWear != WearNone)
                                                victim->Armor +=
                                                        apply_ac(obj_lose,
                                                                 iWear);
                                }
                                break;

                        case ItemHolster:
                        case ItemContainer:
                                separate_obj(obj_lose);
                                act(AtDamage, "$p fumes and dissolves!",
                                    victim, obj_lose, NULL, ToChar);
                                act(AtObject,
                                    "The contents of $p spill out onto the ground.",
                                    victim, obj_lose, NULL, ToRoom);
                                act(AtObject,
                                    "The contents of $p spill out onto the ground.",
                                    victim, obj_lose, NULL, ToChar);
                                empty_obj(obj_lose, NULL, victim->in_room);
                                extract_obj(obj_lose);
                                break;
                        }
                }
        }

        hpch = UMAX(10, ch->hit);
        dam = number_range(hpch / 16 + 1, hpch / 8);
        if (saves_breath(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}



SPELLF spell_fire_breath(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        ObjData *obj_lose;
        ObjData *obj_next;
        int       dam;
        int       hpch;

        if (chance(ch, 2 * level) && !saves_breath(level, victim))
        {
                for (obj_lose = victim->first_carrying; obj_lose;
                     obj_lose = obj_next)
                {
                        char     *msg;

                        obj_next = obj_lose->next_content;
                        if (number_bits(2) != 0)
                                continue;

                        switch (obj_lose->item_type)
                        {
                        default:
                                continue;
                        case ItemHolster:
                        case ItemContainer:
                                msg = "$p ignites and burns!";
                                break;
                        case ItemPotion:
                                msg = "$p bubbles and boils!";
                                break;
                        case ItemScroll:
                                msg = "$p crackles and burns!";
                                break;
                        case ItemStaff:
                                msg = "$p smokes and chars!";
                                break;
                        case ItemWand:
                                msg = "$p sparks and sputters!";
                                break;
                        case ItemDevice:
                                msg = "$p sparks and sputters!";
                                break;
                        case ItemFood:
                                msg = "$p blackens and crisps!";
                                break;
                        case ItemPill:
                                msg = "$p melts and drips!";
                                break;
                        }

                        separate_obj(obj_lose);
                        act(AtDamage, msg, victim, obj_lose, NULL, ToChar);
                        if (obj_lose->item_type == ItemContainer ||
                            obj_lose->item_type == ItemHolster)
                        {
                                act(AtObject,
                                    "The contents of $p spill out onto the ground.",
                                    victim, obj_lose, NULL, ToRoom);
                                act(AtObject,
                                    "The contents of $p spill out onto the ground.",
                                    victim, obj_lose, NULL, ToChar);
                                empty_obj(obj_lose, NULL, victim->in_room);
                        }
                        extract_obj(obj_lose);
                }
        }

        hpch = UMAX(10, ch->hit);
        dam = number_range(hpch / 16 + 1, hpch / 8);
        if (saves_breath(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}



SPELLF spell_frost_breath(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        ObjData *obj_lose;
        ObjData *obj_next;
        int       dam;
        int       hpch;

        if (chance(ch, 2 * level) && !saves_breath(level, victim))
        {
                for (obj_lose = victim->first_carrying; obj_lose;
                     obj_lose = obj_next)
                {
                        char     *msg;

                        obj_next = obj_lose->next_content;
                        if (number_bits(2) != 0)
                                continue;

                        switch (obj_lose->item_type)
                        {
                        default:
                                continue;
                        case ItemHolster:
                        case ItemContainer:
                        case ItemDrinkCon:
                        case ItemPotion:
                                msg = "$p freezes and shatters!";
                                break;
                        }

                        separate_obj(obj_lose);
                        act(AtDamage, msg, victim, obj_lose, NULL, ToChar);
                        if (obj_lose->item_type == ItemContainer ||
                            obj_lose->item_type == ItemHolster)
                        {
                                act(AtObject,
                                    "The contents of $p spill out onto the ground.",
                                    victim, obj_lose, NULL, ToRoom);
                                act(AtObject,
                                    "The contents of $p spill out onto the ground.",
                                    victim, obj_lose, NULL, ToChar);
                                empty_obj(obj_lose, NULL, victim->in_room);
                        }
                        extract_obj(obj_lose);
                }
        }

        hpch = UMAX(10, ch->hit);
        dam = number_range(hpch / 16 + 1, hpch / 8);
        if (saves_breath(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}


SPELLF spell_lightning_breath(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;
        int       hpch;

        hpch = UMAX(10, ch->hit);
        dam = number_range(hpch / 16 + 1, hpch / 8);
        if (saves_breath(level, victim))
                dam /= 2;
        return damage(ch, victim, dam, sn);
}

SPELLF spell_null(int sn, int level, CharData * ch, void *vo)
{
        sn = level = 0;
        vo = NULL;
        send_to_char("That's not a spell!\n\r", ch);
        return rNONE;
}

/* don't remove, may look redundant, but is important */
SPELLF spell_notfound(int sn, int level, CharData * ch, void *vo)
{
        sn = level = 0;
        vo = NULL;
        send_to_char("That's not a spell!\n\r", ch);
        return rNONE;
}


SPELLF spell_farsight(int sn, int level, CharData * ch, void *vo)
{
        RoomIndexData *location;
        RoomIndexData *original;
        CharData *victim;
        SkillType *skill = get_skilltype(sn);

        sn = level = 0;
        vo = NULL;

        /*
         * The spell fails if the victim isn't playing, the victim is the caster,
         * the target room has private, solitary, noastral, death or proto flags,
         * the caster's room is norecall, the victim is too high in level, the 
         * victim is a proto mob, the victim makes the saving throw or the pkill 
         * flag on the caster is not the same as on the victim.  Got it?
         */
        if ((victim = get_char_world(ch, target_name)) == NULL
            || victim == ch
            || !victim->in_room
            || xIS_SET(victim->in_room->RoomFlags, RoomPrivate)
            || xIS_SET(victim->in_room->RoomFlags, RoomPrototype)
            || (IsNpc(victim) && IsSet(victim->act, ActPrototype)))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        location = victim->in_room;
        if (!location)
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }
        successful_casting(skill, ch, victim, NULL);
        original = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, location);
        do_look(ch, "auto");
        char_from_room(ch);
        char_to_room(ch, original);
        return rNONE;
}

SPELLF spell_recharge(int sn, int level, CharData * ch, void *vo)
{
        ObjData *obj = (ObjData *) vo;

        sn = level = 0;

        if (obj->item_type == ItemStaff || obj->item_type == ItemWand)
        {
                separate_obj(obj);
                if (obj->value[2] == obj->value[1]
                    || obj->value[1] > (obj->pIndexData->value[1] * 4))
                {
                        act(AtFire, "$p bursts into flames, injuring you!",
                            ch, obj, NULL, ToChar);
                        act(AtFire, "$p bursts into flames, charring $n!",
                            ch, obj, NULL, ToRoom);
                        extract_obj(obj);
                        if (damage(ch, ch, obj->level * 2, TypeUndefined) ==
                            rCHAR_DIED || char_died(ch))
                                return rCHAR_DIED;
                        else
                                return rSPELL_FAILED;
                }

                if (chance(ch, 2))
                {
                        act(AtYellow,
                            "$p glows with a blinding magical luminescence.",
                            ch, obj, NULL, ToChar);
                        obj->value[1] *= 2;
                        obj->value[2] = obj->value[1];
                        return rNONE;
                }
                else if (chance(ch, 5))
                {
                        act(AtYellow,
                            "$p glows brightly for a few seconds...", ch, obj,
                            NULL, ToChar);
                        obj->value[2] = obj->value[1];
                        return rNONE;
                }
                else if (chance(ch, 10))
                {
                        act(AtWhite, "$p disintegrates into a void.", ch,
                            obj, NULL, ToChar);
                        act(AtWhite,
                            "$n's attempt at recharging fails, and $p disintegrates.",
                            ch, obj, NULL, ToRoom);
                        extract_obj(obj);
                        return rSPELL_FAILED;
                }
                else if (chance(ch, 50 - (ch->skill_level[ForceAbility])))
                {
                        send_to_char("Nothing happens.\n\r", ch);
                        return rSPELL_FAILED;
                }
                else
                {
                        act(AtMagic, "$p feels warm to the touch.", ch, obj,
                            NULL, ToChar);
                        --obj->value[1];
                        obj->value[2] = obj->value[1];
                        return rNONE;
                }
        }
        else
        {
                send_to_char("You can't recharge that!\n\r", ch);
                return rSPELL_FAILED;
        }
}

/* Scryn 2/2/96 */
SPELLF spell_remove_invis(int sn, int level, CharData * ch, void *vo)
{
        ObjData *obj;
        SkillType *skill = get_skilltype(sn);

        vo = NULL;
        level = 0;

        if (target_name[0] == '\0')
        {
                send_to_char("What should the spell be cast upon?\n\r", ch);
                return rSPELL_FAILED;
        }

        obj = get_obj_carry(ch, target_name);

        if (obj)
        {
                if (!IsObjStat(obj, ItemInvis))
                        return rSPELL_FAILED;

                RemoveBit(obj->extra_flags, ItemInvis);
                act(AtMagic, "$p becomes visible again.", ch, obj, NULL,
                    ToChar);

                send_to_char("Ok.\n\r", ch);
                return rNONE;
        }
        else
        {
                CharData *victim;

                victim = get_char_room(ch, target_name);

                if (victim)
                {
                        if (!can_see(ch, victim))
                        {
                                ch_printf(ch, "You don't see %s!\n\r",
                                          target_name);
                                return rSPELL_FAILED;
                        }

                        if (!str_cmp(victim->race->name(), "defel"))
                                return rSPELL_FAILED;

                        if (!IsAffected(victim, AffInvisible))
                        {
                                send_to_char("They are not invisible!\n\r",
                                             ch);
                                return rSPELL_FAILED;
                        }

                        if (is_safe(ch, victim))
                        {
                                failed_casting(skill, ch, victim, NULL);
                                return rSPELL_FAILED;
                        }

                        if (IsSet(victim->immune, RisMagic))
                        {
                                immune_casting(skill, ch, victim, NULL);
                                return rSPELL_FAILED;
                        }
                        if (!IsNpc(victim))
                        {
                                if (chance(ch, 50)
                                    && ch->skill_level[ForceAbility] <
                                    victim->top_level)
                                {
                                        failed_casting(skill, ch, victim,
                                                       NULL);
                                        return rSPELL_FAILED;
                                }

                        }
                        else
                        {
                                if (chance(ch, 50)
                                    && ch->skill_level[ForceAbility] + 15 <
                                    victim->top_level)
                                {
                                        failed_casting(skill, ch, victim,
                                                       NULL);
                                        return rSPELL_FAILED;
                                }
                        }

                        affect_strip(victim, gsn_invis);
                        affect_strip(victim, gsn_mass_invis);
                        RemoveBit(victim->affected_by, AffInvisible);
                        send_to_char("Ok.\n\r", ch);
                        return rNONE;
                }

                ch_printf(ch, "You can't find %s!\n\r", target_name);
                return rSPELL_FAILED;
        }
}


/* Works now.. -- Altrag */
SPELLF spell_possess(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim;
        char      buf[MaxStringLength];
        AffectData af;
        SkillType *skill = get_skilltype(sn);

        vo = NULL;

        if (ch && ch->desc && ch->desc->original)
        {
                send_to_char("You are not in your original state.\n\r", ch);
                return rSPELL_FAILED;
        }

        if ((victim = get_char_room(ch, target_name)) == NULL)
        {
                send_to_char("They aren't here!\n\r", ch);
                return rSPELL_FAILED;
        }

        if (victim == ch)
        {
                send_to_char("You can't possess yourself!\n\r", ch);
                return rSPELL_FAILED;
        }

        if (!IsNpc(victim))
        {
                send_to_char("You can't possess another player!\n\r", ch);
                return rSPELL_FAILED;
        }

        if (victim->desc)
        {
                ch_printf(ch, "%s is already possessed.\n\r",
                          victim->short_descr);
                return rSPELL_FAILED;
        }

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (IsAffected(victim, AffPossess)
            || level < (victim->top_level + 30)
            || victim->desc || !chance(ch, 25))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        send_to_char
                ("You feel the hatred grow within you as you twist your victims mind!\n\r",
                 ch);
        ch->alignment = ch->alignment - 50;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        af.type = sn;
        af.duration =
                20 + (ch->skill_level[ForceAbility] - victim->top_level) / 2;
        af.location = 0;
        af.modifier = 0;
        af.bitvector = AffPossess;
        affect_to_char(victim, &af);

        snprintf(buf, MSL, "You have possessed %s!\n\r", victim->short_descr);

        ch->desc->character = victim;
        ch->desc->original = ch;
        victim->desc = ch->desc;
        ch->desc = NULL;
        ch->switched = victim;
        send_to_char(buf, victim);

        return rNONE;

}

/* Ignores pickproofs, but can't unlock containers. -- Altrag 17/2/96 */
SPELLF spell_knock(int sn, int level, CharData * ch, void *vo)
{
        ExitData *pexit;
        SkillType *skill = get_skilltype(sn);

        level = 0;
        vo = NULL;

        set_char_color(AtMagic, ch);
        /*
         * shouldn't know why it didn't work, and shouldn't work on pickproof
         * exits.  -Thoric
         */
        if (!(pexit = find_door(ch, target_name, FALSE))
            || !IsSet(pexit->exit_info, ExClosed)
            || !IsSet(pexit->exit_info, ExLocked)
            || IsSet(pexit->exit_info, ExPickproof))
        {
                failed_casting(skill, ch, NULL, NULL);
                return rSPELL_FAILED;
        }
        RemoveBit(pexit->exit_info, ExLocked);
        send_to_char("*Click*\n\r", ch);
        if (pexit->rexit && pexit->rexit->to_room == ch->in_room)
                RemoveBit(pexit->rexit->exit_info, ExLocked);
        check_room_for_traps(ch, TrapUnlock | trap_door[pexit->vdir]);
        return rNONE;
}

/* Tells to sleepers in are. -- Altrag 17/2/96 */
SPELLF spell_dream(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim;
        char      arg[MaxInputLength];

        sn = level = 0;
        vo = NULL;

        target_name = one_argument(target_name, arg);
        set_char_color(AtMagic, ch);
        if (!(victim = get_char_world(ch, arg)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return rSPELL_FAILED;
        }
        if (victim->position != PosSleeping)
        {
                send_to_char("They aren't asleep.\n\r", ch);
                return rSPELL_FAILED;
        }
        if (!target_name)
        {
                send_to_char("What do you want them to dream about?\n\r", ch);
                return rSPELL_FAILED;
        }

        set_char_color(AtTell, victim);
        ch_printf(victim, "You have dreams about %s telling you '%s'.\n\r",
                  PERS(ch, victim), target_name);
        send_to_char("Ok.\n\r", ch);
        return rNONE;
}

SPELLF spell_polymorph(int sn, int level, CharData * ch, void *vo)
{
        int       poly_vnum;
        CharData *poly_mob;

        sn = level = 0;
        vo = NULL;

        if (IsNpc(ch))
        {
                send_to_char("Mobs can't polymorph!\n\r", ch);
                return rSPELL_FAILED;
        }

        if (ch->desc->original)
        {
                send_to_char("You are not in your original state.\n\r", ch);
                return rSPELL_FAILED;
        }

        if (!str_cmp(target_name, "wolf"))
                poly_vnum = MobVnumPolyWolf;

        else
        {
                set_char_color(AtMagic, ch);
                send_to_char("You can't polymorph into that!\n\r", ch);
                return rSPELL_FAILED;
        }

        poly_mob = make_poly_mob(ch, poly_vnum);
        if (!poly_mob)
        {
                bug("Spell_polymorph: null polymob!", 0);
                return rSPELL_FAILED;
        }

        char_to_room(poly_mob, ch->in_room);
        char_from_room(ch);
        char_to_room(ch, get_room_index(RoomVnumPoly));
        ch->desc->character = poly_mob;
        ch->desc->original = ch;
        poly_mob->desc = ch->desc;
        ch->desc = NULL;
        ch->switched = poly_mob;

        return rNONE;
}

CharData *make_poly_mob(CharData * ch, int vnum)
{
        CharData *mob;
        MobIndexData *pMobIndex;

        if (!ch)
        {
                bug("Make_poly_mob: null ch!", 0);
                return NULL;
        }

        if (vnum < 10 || vnum > 16)
        {
                bug("Make_poly_mob: Vnum not in polymorphing mobs range", 0);
                return NULL;
        }

        if ((pMobIndex = get_mob_index(vnum)) == NULL)
        {
                bug("Make_poly_mob: Can't find mob %d", vnum);
                return NULL;
        }
        mob = create_mobile(pMobIndex);
        SetBit(mob->act, ActPolymorphed);
        return mob;
}


SPELLF spell_potential(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        char     *msg;
        int       toop;
        SkillType *skill = get_skilltype(sn);

        level = 0;

        if (!victim)
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        toop = victim->perm_frc;

        if (toop < 1)
                msg = "You cannot sense the Force in $N.";
        else if (toop < 6)
                msg = "You sense the Force faintly in $N.";
        else if (toop < 11)
                msg = "The Force is moderately strong in $N.";
        else if (toop < 16)
                msg = "The Force is very stong in $N.";
        else if (toop < 20)
                msg = "$N may become a very stong Jedi Knight.";
        else if (toop >= 20)
                msg = "$N may become a Jedi Master.";
        else
                msg = "I'd rather just not say anything at all about $N.";

        act(AtMagic, msg, ch, NULL, victim, ToChar);
        return rNONE;
}

SPELLF spell_sense_force(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        char     *msg;
        int       toop;
        SkillType *skill = get_skilltype(sn);

        level = 0;

        if (str_cmp(ch->race->name(), "duinuogwuin"))
        {
                send_to_char
                        ("Only Duinuogwuin can sense the Force in others.\n\r",
                         ch);
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (!victim)
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        if (IsSet(victim->immune, RisMagic))
        {
                immune_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        toop = victim->perm_frc;

        if (toop < 1)
                msg = "You cannot sense the Force in $N.";
        else if (toop < 6)
                msg = "You sense the Force faintly in $N.";
        else if (toop < 11)
                msg = "The Force is moderately strong in $N.";
        else if (toop < 16)
                msg = "The Force is very stong in $N.";
        else if (toop < 20)
                msg = "$N may become a very stong Jedi Knight.";
        else if (toop >= 20)
                msg = "$N may become a Jedi Master.";
        else
                msg = "I'd rather just not say anything at all about $N.";

        act(AtMagic, msg, ch, NULL, victim, ToChar);
        return rNONE;
}

CMDF do_revert(CharData * ch, char *argument)
{

        CharData *mob;

        argument = NULL;

        if (!IsNpc(ch) || !IsSet(ch->act, ActPolymorphed))
        {
                send_to_char("You are not polymorphed.\n\r", ch);
                return;
        }

        RemoveBit(ch->act, ActPolymorphed);

        char_from_room(ch->desc->original);

        if (ch->desc->character)
        {
                mob = ch->desc->character;
                char_to_room(ch->desc->original, ch->desc->character->in_room); /*WORKS!! */
                ch->desc->character = ch->desc->original;
                ch->desc->original = NULL;
                ch->desc->character->desc = ch->desc;
                ch->desc->character->switched = NULL;
                ch->desc = NULL;
                extract_char(mob, TRUE);
                return;
        }

/*  else
  {
    location = NULL;
    if(ch->desc->original->pcdata->clan)
      location = get_room_index(ch->desc->original->pcdata->clan->recall);
    if(!location)
      location = get_room_index(RoomVnumTemple);
    char_to_room(ch->desc->original, location);
  }
*/
        ch->desc->character = ch->desc->original;
        ch->desc->original = NULL;
        ch->desc->character->desc = ch->desc;
        ch->desc->character->switched = NULL;
        ch->desc = NULL;
        return;
}

/* Added spells spiral_blast, scorching surge,
    nostrum, and astral   by SB for Augurer class 
7/10/96 */
SPELLF spell_spiral_blast(int sn, int level, CharData * ch, void *vo)
{
        CharData *vch;
        CharData *vch_next;
        int       dam;
        int       hpch;
        bool      ch_died;

        vo = NULL;

        ch_died = FALSE;

        if (xIS_SET(ch->in_room->RoomFlags, RoomSafe))
        {
                set_char_color(AtMagic, ch);
                send_to_char("You fail to breathe.\n\r", ch);
                return rNONE;
        }


        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        for (vch = ch->in_room->first_person; vch; vch = vch_next)
        {
                vch_next = vch->next_in_room;
                if (!IsNpc(vch) && IsSet(vch->act, PlrWizinvis)
                    && vch->pcdata->wizinvis >= LevelImmortal)
                        continue;

                if (IsNpc(ch) ? !IsNpc(vch) : IsNpc(vch))
                {
                        act(AtMagic, "Swirling colours radiate from $n"
                            ", encompassing $N.", ch, ch, vch, ToRoom);
                        act(AtMagic, "Swirling colours radiate from you,"
                            " encompassing $N", ch, ch, vch, ToChar);

                        hpch = UMAX(10, ch->hit);
                        dam = number_range(hpch / 14 + 1, hpch / 7);
                        if (saves_breath(level, vch))
                                dam /= 2;
                        if (damage(ch, vch, dam, sn) == rCHAR_DIED ||
                            char_died(ch))
                                ch_died = TRUE;
                }
        }

        if (ch_died)
                return rCHAR_DIED;
        else
                return rNONE;
}

SPELLF spell_scorching_surge(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        static const sh_int dam_each[] = {
                1,
                1, 2, 3, 4, 5, 6, 8, 10, 12, 14,
                16, 18, 20, 25, 30, 35, 40, 45, 50, 55,
                60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
                92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
                112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
                132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
                152, 154, 156, 158, 160, 162, 164, 166, 168, 170
        };
        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        level = UMIN(level / 2,
                     (int) sizeof(dam_each) / (int) sizeof(dam_each[0]) - 1);
        level = UMAX(0, level);
        dam = number_range(dam_each[level], dam_each[level] * 10);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic, "A fiery current lashes through $n's body!",
            ch, NULL, NULL, ToRoom);
        act(AtMagic, "A fiery current lashes through your body!",
            ch, NULL, NULL, ToChar);
        return damage(ch, victim, (int) (dam * 1.4), sn);
}

    /*******************************************************
	 * Everything after this point is part of SMAUG SPELLS *
	 *******************************************************/

/*
 * saving throw check						-Thoric
 */
bool check_save(int sn, int level, CharData * ch, CharData * victim)
{
        SkillType *skill = get_skilltype(sn);
        bool      saved = FALSE;

        if (SpellFlag(skill, SfPksensitive)
            && !IsNpc(ch) && !IsNpc(victim))
                level /= 2;

        if (skill->saves)
                switch (skill->saves)
                {
                case SsPoisonDeath:
                        saved = saves_poison_death(level, victim);
                        break;
                case SsRodWands:
                        saved = saves_wands(level, victim);
                        break;
                case SsParaPetri:
                        saved = saves_para_petri(level, victim);
                        break;
                case SsBreath:
                        saved = saves_breath(level, victim);
                        break;
                case SsSpellStaff:
                        saved = saves_spell_staff(level, victim);
                        break;
                }
        return saved;
}

/*
 * Generic offensive spell damage attack			-Thoric
 */
SPELLF spell_attack(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        SkillType *skill = get_skilltype(sn);
        bool      saved = check_save(sn, level, ch, victim);
        int       dam;
        ch_ret    retcode;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        if (saved && !SpellFlag(skill, SfSaveHalfDamage))
        {
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }
        if (skill->dice)
                dam = UMAX(0, dice_parse(ch, level, skill->dice));
        else
                dam = dice(1, level);
        if (saved)
                dam /= 2;
        retcode = damage(ch, victim, dam, sn);
        if (retcode == rNONE && skill->affects
            && !char_died(ch) && !char_died(victim))
                retcode = spell_affectchar(sn, level, ch, victim);
        return retcode;
}

/*
 * Generic area attack						-Thoric
 */
SPELLF spell_area_attack(int sn, int level, CharData * ch, void *vo)
{
        CharData *vch, *vch_next;
        SkillType *skill = get_skilltype(sn);
        bool      saved;
        bool      affects;
        int       dam;
        bool      ch_died = FALSE;
        ch_ret    retcode = 0;

        vo = NULL;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        if (xIS_SET(ch->in_room->RoomFlags, RoomSafe))
        {
                failed_casting(skill, ch, NULL, NULL);
                return rSPELL_FAILED;
        }

        affects = (skill->affects ? TRUE : FALSE);
        if (skill->hit_char && skill->hit_char[0] != '\0')
                act(AtMagic, skill->hit_char, ch, NULL, NULL, ToChar);
        if (skill->hit_room && skill->hit_room[0] != '\0')
                act(AtMagic, skill->hit_room, ch, NULL, NULL, ToRoom);

        for (vch = ch->in_room->first_person; vch; vch = vch_next)
        {
                vch_next = vch->next_in_room;

                if (!IsNpc(vch) && IsSet(vch->act, PlrWizinvis)
                    && vch->pcdata->wizinvis >= LevelImmortal)
                        continue;

                if (vch != ch && (IsNpc(ch) ? !IsNpc(vch) : IsNpc(vch)))
                {
                        saved = check_save(sn, level, ch, vch);
                        if (saved && !SpellFlag(skill, SfSaveHalfDamage))
                        {
                                failed_casting(skill, ch, vch, NULL);
                                dam = 0;
                        }
                        else if (skill->dice)
                                dam = dice_parse(ch, level, skill->dice);
                        else
                                dam = dice(1, level);
                        if (saved && SpellFlag(skill, SfSaveHalfDamage))
                                dam /= 2;
                        retcode = damage(ch, vch, dam, sn);
                }
                if (retcode == rNONE && affects && !char_died(ch)
                    && !char_died(vch))
                        retcode = spell_affectchar(sn, level, ch, vch);
                if (retcode == rCHAR_DIED || char_died(ch))
                {
                        ch_died = TRUE;
                        break;
                }
        }
        return retcode;
}


ch_ret spell_affectchar(int sn, int level, CharData * ch, void *vo)
{
        AffectData af;
        SmaugAff *saf;
        SkillType *skill = get_skilltype(sn);
        CharData *victim = (CharData *) vo;
        int       percent_chance;
        ch_ret    retcode = rNONE;
		bool affected = FALSE, first = TRUE;

        if (SpellFlag(skill, SfRecastable))
                affect_strip(victim, sn);
        for (saf = skill->affects; saf; saf = saf->next)
        {
    if(saf->location >= ReverseApply)
    {
     if(!SpellFlag(skill, SfAccumulative))
     {
      if(first == TRUE)
      {
       if(SpellFlag(skill, SfRecastable))
        affect_strip(ch, sn);
       if(is_affected(ch, sn))
        affected = TRUE;
      }
      first = FALSE;
      if(affected == TRUE)
       continue;
     }
     victim = ch;
    }
    else
      victim = (CharData *) vo;;
                /*
                 * Check if char has this bitvector already 
                 */
                if ((af.bitvector = saf->bitvector) != 0
                    && IsAffected(victim, af.bitvector)
                    && !SpellFlag(skill, SfAccumulative))
                        continue;
                /*
                 * necessary for affect_strip to work properly...
                 */
                switch (af.bitvector)
                {
                default:
                        af.type = sn;
                        break;
                case AffPoison:
                        af.type = gsn_poison;

                        send_to_char
                                ("You feel the hatred grow within you!\n\r",
                                 ch);
                        ch->alignment = ch->alignment - 100;
                        ch->alignment = URANGE(-1000, ch->alignment, 1000);
                        sith_penalty(ch);

                        percent_chance = ris_save(victim, level, RisPoison);
                        if (percent_chance == 1000)
                        {
                                retcode = rVICT_IMMUNE;
                                if (SpellFlag(skill, SfStoponfail))
                                        return retcode;
                                continue;
                        }
                        if (saves_poison_death(percent_chance, victim))
                        {
                                if (SpellFlag(skill, SfStoponfail))
                                        return retcode;
                                continue;
                        }
                        victim->mental_state =
                                URANGE(30, victim->mental_state + 2, 100);
                        break;
                case AffBlind:
                        af.type = gsn_blindness;
                        break;
                case AffInvisible:
                        af.type = gsn_invis;
                        break;
                case AffSleep:
                        af.type = gsn_sleep;
                        percent_chance = ris_save(victim, level, RisSleep);
                        if (percent_chance == 1000)
                        {
                                retcode = rVICT_IMMUNE;
                                if (SpellFlag(skill, SfStoponfail))
                                        return retcode;
                                continue;
                        }
                        break;
                case AffCharm:
                        af.type = gsn_charm_person;
                        percent_chance = ris_save(victim, level, RisCharm);
                        if (percent_chance == 1000)
                        {
                                retcode = rVICT_IMMUNE;
                                if (SpellFlag(skill, SfStoponfail))
                                        return retcode;
                                continue;
                        }
                        break;
                case AffPossess:
                        af.type = gsn_possess;
                        break;
                }
                af.duration = dice_parse(ch, level, saf->duration);
                af.modifier = dice_parse(ch, level, saf->modifier);
                af.location = saf->location % ReverseApply;
                if (af.duration == 0)
                {

                        switch (af.location)
                        {
                        case ApplyHit:
                                if (ch != victim
                                    && victim->hit < victim->max_hit
                                    && af.modifier > 0)
                                {
                                        send_to_char
                                                ("The nobel Jedi use their powers to help others!\n\r",
                                                 ch);
                                        ch->alignment = ch->alignment + 20;
                                        ch->alignment =
                                                URANGE(-1000, ch->alignment,
                                                       1000);
                                        jedi_bonus(ch);
                                }
                                if (af.modifier > 0
                                    && victim->hit >= victim->max_hit)
                                {
                                        return rSPELL_FAILED;
                                }
                                victim->hit =
                                        URANGE(0, victim->hit + af.modifier,
                                               victim->max_hit);
                                update_pos(victim);
                                break;
                        case ApplyMana:
                                if (af.modifier > 0
                                    && victim->endurance >=
                                    victim->max_endurance)
                                {
                                        return rSPELL_FAILED;
                                }
                                if (ch != victim)
                                {
                                        send_to_char
                                                ("The nobel Jedi use their powers to help others!\n\r",
                                                 ch);
                                        ch->alignment = ch->alignment + 25;
                                        ch->alignment =
                                                URANGE(-1000, ch->alignment,
                                                       1000);
                                        jedi_bonus(ch);
                                }
                                victim->endurance =
                                        URANGE(0,
                                               victim->endurance +
                                               af.modifier,
                                               victim->max_endurance);
                                update_pos(victim);
                                break;
                        case ApplyMove:
                                if (af.modifier > 0
                                    && victim->endurance >=
                                    victim->max_endurance)
                                {
                                        return rSPELL_FAILED;
                                }
                                if (ch != victim)
                                {
                                        send_to_char
                                                ("The nobel Jedi use their powers to help others!\n\r",
                                                 ch);
                                        ch->alignment = ch->alignment + 25;
                                        ch->alignment =
                                                URANGE(-1000, ch->alignment,
                                                       1000);
                                        jedi_bonus(ch);
                                }
                                victim->endurance =
                                        URANGE(0,
                                               victim->endurance +
                                               af.modifier,
                                               victim->max_endurance);
                                update_pos(victim);
                                break;
                        default:
                                affect_modify(victim, &af, TRUE);
                                break;
                        }
                }
                else if (SpellFlag(skill, SfAccumulative))
                        affect_join(victim, &af);
                else
                        affect_to_char(victim, &af);
        }
        update_pos(victim);
        return retcode;
}


/*
 * Generic spell affect						-Thoric
 */
ch_ret spell_affect(int sn, int level, CharData * ch, void *vo)
{
        SmaugAff *saf;
        SkillType *skill = get_skilltype(sn);
        CharData *victim = (CharData *) vo;
        bool      groupsp;
        bool      areasp;
        bool      hitchar = FALSE, hitroom = FALSE, hitvict = FALSE;
        ch_ret    retcode;

        if (!skill->affects)
        {
                bug("spell_affect has no affects sn %d", sn);
                return rNONE;
        }
        if (SpellFlag(skill, SfGroupspell))
                groupsp = TRUE;
        else
                groupsp = FALSE;

        if (SpellFlag(skill, SfArea))
                areasp = TRUE;
        else
                areasp = FALSE;
        if (!groupsp && !areasp)
        {
                /*
                 * Can't find a victim 
                 */
                if (!victim)
                {
                        failed_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }

                if ((IsSet(victim->immune, RisMagic))
                    || is_immune(victim, SpellDamage(skill)))
                {
                        immune_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }

                /*
                 * Spell is already on this guy 
                 */
                if (is_affected(victim, sn)
                    && !SpellFlag(skill, SfAccumulative)
                    && !SpellFlag(skill, SfRecastable))
                {
                        failed_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }

                if ((saf = skill->affects) && !saf->next
                    && saf->location == ApplyStripsn
                    && !is_affected(victim,
                                    dice_parse(ch, level, saf->modifier)))
                {
                        failed_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }

                if (check_save(sn, level, ch, victim))
                {
                        failed_casting(skill, ch, victim, NULL);
                        return rSPELL_FAILED;
                }
        }
        else
        {
                if (skill->hit_char && skill->hit_char[0] != '\0')
                {
                        if (strstr(skill->hit_char, "$N"))
                                hitchar = TRUE;
                        else
                                act(AtMagic, skill->hit_char, ch, NULL, NULL,
                                    ToChar);
                }
                if (skill->hit_room && skill->hit_room[0] != '\0')
                {
                        if (strstr(skill->hit_room, "$N"))
                                hitroom = TRUE;
                        else
                                act(AtMagic, skill->hit_room, ch, NULL, NULL,
                                    ToRoom);
                }
                if (skill->hit_vict && skill->hit_vict[0] != '\0')
                        hitvict = TRUE;
                if (victim)
                        victim = victim->in_room->first_person;
                else
                        victim = ch->in_room->first_person;
        }
        if (!victim)
        {
                bug("spell_affect: could not find victim: sn %d", sn);
                failed_casting(skill, ch, victim, NULL);
                return rSPELL_FAILED;
        }

        for (; victim; victim = victim->next_in_room)
        {
                if (groupsp || areasp)
                {
                        if ((groupsp && !is_same_group(victim, ch))
                            || IsSet(victim->immune, RisMagic)
                            || is_immune(victim, SpellDamage(skill))
                            || check_save(sn, level, ch, victim)
                            || (!SpellFlag(skill, SfRecastable)
                                && is_affected(victim, sn)))
                                continue;

                        if (hitvict && ch != victim)
                        {
                                act(AtMagic, skill->hit_vict, ch, NULL,
                                    victim, ToVict);
                                if (hitroom)
                                {
                                        act(AtMagic, skill->hit_room, ch,
                                            NULL, victim, ToNotvict);
                                        act(AtMagic, skill->hit_room, ch,
                                            NULL, victim, ToChar);
                                }
                        }
                        else if (hitroom)
                                act(AtMagic, skill->hit_room, ch, NULL,
                                    victim, ToRoom);
                        if (ch == victim)
                        {
                                if (hitvict)
                                        act(AtMagic, skill->hit_vict, ch,
                                            NULL, ch, ToChar);
                                else if (hitchar)
                                        act(AtMagic, skill->hit_char, ch,
                                            NULL, ch, ToChar);
                        }
                        else if (hitchar)
                                act(AtMagic, skill->hit_char, ch, NULL,
                                    victim, ToChar);
                }
                retcode = spell_affectchar(sn, level, ch, victim);
                if (!groupsp && !areasp)
                {
                        if (retcode == rSPELL_FAILED)
                        {
                                failed_casting(skill, ch, victim, NULL);
                                return rSPELL_FAILED;
                        }
                        if (retcode == rVICT_IMMUNE)
                                immune_casting(skill, ch, victim, NULL);
                        else
                                successful_casting(skill, ch, victim, NULL);
                        break;
                }
        }
        return rNONE;
}

/*
 * Generic inventory object spell				-Thoric
 */
SPELLF spell_obj_inv(int sn, int level, CharData * ch, void *vo)
{
        ObjData *obj = (ObjData *) vo;
        SkillType *skill = get_skilltype(sn);

        if (!obj)
        {
                failed_casting(skill, ch, NULL, NULL);
                return rNONE;
        }

        switch (SpellAction(skill))
        {
        default:
        case SaNone:
                return rNONE;

        case SaCreate:
                if (SpellFlag(skill, SfWater))    /* create water */
                {
                        int       water;

                        if (obj->item_type != ItemDrinkCon)
                        {
                                send_to_char
                                        ("It is unable to hold water.\n\r",
                                         ch);
                                return rSPELL_FAILED;
                        }

                        if (obj->value[2] != LiqWater && obj->value[1] != 0)
                        {
                                send_to_char
                                        ("It contains some other liquid.\n\r",
                                         ch);
                                return rSPELL_FAILED;
                        }

                        water = UMIN((skill->
                                      dice ? dice_parse(ch, level,
                                                        skill->
                                                        dice) : level) *
                                     (weather_info.sky >=
                                      SkyRaining ? 2 : 1),
                                     obj->value[0] - obj->value[1]);

                        if (water > 0)
                        {
                                separate_obj(obj);
                                obj->value[2] = LiqWater;
                                obj->value[1] += water;
                                if (!is_name("water", obj->name))
                                {
                                        char      buf[MaxStringLength];

                                        snprintf(buf, MSL, "%s water",
                                                 obj->name);
                                        STRFREE(obj->name);
                                        obj->name = STRALLOC(buf);
                                }
                        }
                        successful_casting(skill, ch, NULL, obj);
                        return rNONE;
                }
                if (SpellDamage(skill) == SdFire) /* burn object */
                {
                        /*
                         * return rNONE; 
                         */
                }
                if (SpellDamage(skill) == SdPoison    /* poison object */
                    || SpellClass(skill) == ScDeath)
                {
                        switch (obj->item_type)
                        {
                        default:
                                failed_casting(skill, ch, NULL, obj);
                                break;
                        case ItemFood:
                        case ItemDrinkCon:
                                separate_obj(obj);
                                obj->value[3] = 1;
                                successful_casting(skill, ch, NULL, obj);
                                break;
                        }
                        return rNONE;
                }
                if (SpellClass(skill) == ScLife   /* purify food/water */
                    && (obj->item_type == ItemFood
                        || obj->item_type == ItemDrinkCon))
                {
                        switch (obj->item_type)
                        {
                        default:
                                failed_casting(skill, ch, NULL, obj);
                                break;
                        case ItemFood:
                        case ItemDrinkCon:
                                separate_obj(obj);
                                obj->value[3] = 0;
                                successful_casting(skill, ch, NULL, obj);
                                break;
                        }
                        return rNONE;
                }

                if (SpellClass(skill) != ScNone)
                {
                        failed_casting(skill, ch, NULL, obj);
                        return rNONE;
                }
                switch (SpellPower(skill)) /* clone object */
                {
                        ObjData *clone;

                default:
                case SpNone:
                        if (obj->cost >
                            ch->skill_level[ForceAbility] *
                            get_curr_int(ch) * get_curr_wis(ch))
                        {
                                failed_casting(skill, ch, NULL, obj);
                                return rNONE;
                        }
                        break;
                case SpMinor:
                        if (ch->skill_level[ForceAbility] - obj->level < 20
                            || obj->cost >
                            ch->skill_level[ForceAbility] *
                            get_curr_int(ch) / 5)
                        {
                                failed_casting(skill, ch, NULL, obj);
                                return rNONE;
                        }
                        break;
                case SpGreater:
                        if (ch->skill_level[ForceAbility] - obj->level < 5
                            || obj->cost >
                            ch->skill_level[ForceAbility] * 10 *
                            get_curr_int(ch) * get_curr_wis(ch))
                        {
                                failed_casting(skill, ch, NULL, obj);
                                return rNONE;
                        }
                        break;
                case SpMajor:
                        if (ch->skill_level[ForceAbility] - obj->level < 0
                            || obj->cost >
                            ch->skill_level[ForceAbility] * 50 *
                            get_curr_int(ch) * get_curr_wis(ch))
                        {
                                failed_casting(skill, ch, NULL, obj);
                                return rNONE;
                        }
                        clone = clone_object(obj);
                        clone->timer =
                                skill->dice ? dice_parse(ch, level,
                                                         skill->dice) : 0;
                        obj_to_char(clone, ch);
                        successful_casting(skill, ch, NULL, obj);
                        break;
                }
                return rNONE;

        case SaDestroy:
        case SaResist:
        case SaSuscept:
        case SaDivinate:
                if (SpellDamage(skill) == SdPoison)   /* detect poison */
                {
                        if (obj->item_type == ItemDrinkCon
                            || obj->item_type == ItemFood)
                        {
                                if (obj->value[3] != 0)
                                        send_to_char
                                                ("You smell poisonous fumes.\n\r",
                                                 ch);
                                else
                                        send_to_char
                                                ("It looks very delicious.\n\r",
                                                 ch);
                        }
                        else
                                send_to_char("It doesn't look poisoned.\n\r",
                                             ch);
                        return rNONE;
                }
                return rNONE;
        case SaObscure:   /* make obj invis */
                if (IsObjStat(obj, ItemInvis)
                    || chance(ch,
                              skill->dice ? dice_parse(ch, level,
                                                       skill->dice) : 20))
                {
                        failed_casting(skill, ch, NULL, NULL);
                        return rSPELL_FAILED;
                }
                successful_casting(skill, ch, NULL, obj);
                SetBit(obj->extra_flags, ItemInvis);
                return rNONE;

        case SaChange:
                return rNONE;
        }
        return rNONE;
}

/*
 * Generic object creating spell				-Thoric
 */
SPELLF spell_create_obj(int sn, int level, CharData * ch, void *vo)
{
        SkillType *skill = get_skilltype(sn);
        int       lvl;
        int       vnum = skill->value;
        ObjData *obj;
        ObjIndexData *oi;

        vo = NULL;

        switch (SpellPower(skill))
        {
        default:
        case SpNone:
                lvl = 10;
                break;
        case SpMinor:
                lvl = 0;
                break;
        case SpGreater:
                lvl = level / 2;
                break;
        case SpMajor:
                lvl = level;
                break;
        }

        /*
         * Add predetermined objects here
         */
        if (vnum == 0)
        {
                if (!str_cmp(target_name, "sword"))
                        vnum = ObjVnumSchoolSword;
                if (!str_cmp(target_name, "shield"))
                        vnum = ObjVnumSchoolShield;
        }

        if ((oi = get_obj_index(vnum)) == NULL
            || (obj = create_object(oi, lvl)) == NULL)
        {
                failed_casting(skill, ch, NULL, NULL);
                return rNONE;
        }
        obj->timer = skill->dice ? dice_parse(ch, level, skill->dice) : 0;
        successful_casting(skill, ch, NULL, obj);
        if (CanWear(obj, ItemTake))
                obj_to_char(obj, ch);
        else
                obj_to_room(obj, ch->in_room);
        return rNONE;
}

/*
 * Generic mob creating spell					-Thoric
 */
SPELLF spell_create_mob(int sn, int level, CharData * ch, void *vo)
{
        SkillType *skill = get_skilltype(sn);
        int       lvl;
        int       vnum = skill->value;
        CharData *mob;
        MobIndexData *mi;
        AffectData af;

        vo = NULL;

        /*
         * set maximum mob level 
         */
        switch (SpellPower(skill))
        {
        default:
        case SpNone:
                lvl = 20;
                break;
        case SpMinor:
                lvl = 5;
                break;
        case SpGreater:
                lvl = level / 2;
                break;
        case SpMajor:
                lvl = level;
                break;
        }

        /*
         * Add predetermined mobiles here
         */
        if (vnum == 0)
        {
                return rNONE;
        }

        if ((mi = get_mob_index(vnum)) == NULL
            || (mob = create_mobile(mi)) == NULL)
        {
                failed_casting(skill, ch, NULL, NULL);
                return rNONE;
        }
        mob->top_level =
                UMIN(lvl,
                     skill->dice ? dice_parse(ch, level,
                                              skill->dice) : mob->top_level);
        mob->Armor = interpolate(mob->top_level, 100, -100);

        mob->max_hit =
                mob->top_level * 8 +
                number_range(mob->top_level * mob->top_level / 4,
                             mob->top_level * mob->top_level);
        mob->hit = mob->max_hit;
        mob->gold = 0;
        successful_casting(skill, ch, mob, NULL);
        char_to_room(mob, ch->in_room);
        add_follower(mob, ch);
        af.type = sn;
        af.duration = (int) ((number_fuzzy((level + 1) / 3) + 1) * DurConv);
        af.location = 0;
        af.modifier = 0;
        af.bitvector = AffCharm;
        affect_to_char(mob, &af);
        return rNONE;
}

/*
 * Generic handler for new "SMAUG" spells			-Thoric
 */
SPELLF spell_smaug(int sn, int level, CharData * ch, void *vo)
{
        struct skill_type *skill = get_skilltype(sn);

        switch (skill->target)
        {
        case TarIgnore:

                /*
                 * offensive area spell 
                 */
                if (SpellFlag(skill, SfArea)
                    && ((SpellAction(skill) == SaDestroy
                         && SpellClass(skill) == ScLife)
                        || (SpellAction(skill) == SaCreate
                            && SpellClass(skill) == ScDeath)))
                        return spell_area_attack(sn, level, ch, vo);

                if (SpellAction(skill) == SaCreate)
                {
                        if (SpellFlag(skill, SfObject))   /* create object */
                                return spell_create_obj(sn, level, ch, vo);
                        if (SpellClass(skill) == ScLife)  /* create mob */
                                return spell_create_mob(sn, level, ch, vo);
                }

                /*
                 * affect a distant player 
                 */
                if (SpellFlag(skill, SfDistant)
                    && SpellFlag(skill, SfCharacter))
                        return spell_affect(sn, level, ch,
                                            get_char_world(ch, target_name));

                /*
                 * affect a player in this room (should have been TarCharXxx) 
                 */
                if (SpellFlag(skill, SfCharacter))
                        return spell_affect(sn, level, ch,
                                            get_char_room(ch, target_name));

                /*
                 * will fail, or be an area/group affect 
                 */
                return spell_affect(sn, level, ch, vo);

        case TarCharOffensive:
                /*
                 * a regular damage inflicting spell attack 
                 */
                if ((SpellAction(skill) == SaDestroy
                     && SpellClass(skill) == ScLife)
                    || (SpellAction(skill) == SaCreate
                        && SpellClass(skill) == ScDeath))
                        return spell_attack(sn, level, ch, vo);

                /*
                 * a nasty spell affect 
                 */
                return spell_affect(sn, level, ch, vo);

        case TarCharDefensive:

        case TarCharSelf:
                if (vo && SpellAction(skill) == SaDestroy)
                {
                        CharData *victim = (CharData *) vo;

                        /*
                         * cure poison 
                         */
                        if (SpellDamage(skill) == SdPoison)
                        {
                                if (is_affected(victim, gsn_poison))
                                {
                                        affect_strip(victim, gsn_poison);
                                        victim->mental_state =
                                                URANGE(-100,
                                                       victim->mental_state,
                                                       -10);
                                        successful_casting(skill, ch, victim,
                                                           NULL);
                                        return rNONE;
                                }
                                failed_casting(skill, ch, victim, NULL);
                                return rSPELL_FAILED;
                        }
                        /*
                         * cure blindness 
                         */
                        if (SpellClass(skill) == ScIllusion)
                        {
                                if (is_affected(victim, gsn_blindness))
                                {
                                        affect_strip(victim, gsn_blindness);
                                        successful_casting(skill, ch, victim,
                                                           NULL);
                                        return rNONE;
                                }
                                failed_casting(skill, ch, victim, NULL);
                                return rSPELL_FAILED;
                        }
                }
                return spell_affect(sn, level, ch, vo);

        case TarObjInv:
                return spell_obj_inv(sn, level, ch, vo);
        }
        return rNONE;
}



/* Haus' new, new mage spells follow */

/*
 *  4 Energy Spells
 */
SPELLF spell_ethereal_fist(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        level = UMIN(35, level);
        dam = level * number_range(1, 6) - 31;
        dam = UMAX(0, dam);

        if (saves_spell_staff(level, victim))
                dam = 0;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        act(AtMagic,
            "A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}


SPELLF spell_spectral_furor(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        level = UMAX(0, level);
        level = UMIN(16, level);
        dam = level * number_range(1, 7) + 7;
        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic, "The fabric of the cosmos strains in fury about $N!",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_hand_of_chaos(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        dam = level * number_range(1, 7) + 9;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_spell_staff(level, victim))
                dam = 0;
        act(AtMagic,
            "$N is grasped by an incomprehensible hand of darkness!", ch,
            NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}


SPELLF spell_disruption(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        level = UMIN(14, level);
        dam = level * number_range(1, 6) + 8;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        if (saves_spell_staff(level, victim))
                dam = 0;
        act(AtMagic,
            "A weird energy encompasses $N, causing you to question $S continued existence.",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_sonic_resonance(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        level = UMIN(23, level);
        dam = level * number_range(1, 8);

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_spell_staff(level, victim))
                dam = dam * 3 / 4;
        act(AtMagic,
            "A cylinder of kinetic energy enshrouds $N causing $S to resonate.",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

/*
 * 3 Mentalstate spells
 */
SPELLF spell_mind_wrack(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        /*
         * decrement mentalstate by up to 50 
         */

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        level = UMAX(0, level);
        dam = number_range(0, 0);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic,
            "$n stares intently at $N, causing $N to seem very lethargic.",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_mind_wrench(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        /*
         * increment mentalstate by up to 50 
         */

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        level = UMAX(0, level);
        dam = number_range(0, 0);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic,
            "$n stares intently at $N, causing $N to seem very hyperactive.",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}


/* Non-offensive spell! */
SPELLF spell_revive(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        /*
         * set mentalstate to mentalstate/2 
         */
        level = UMAX(0, level);
        dam = number_range(0, 0);
        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic,
            "$n concentrates intently, and begins looking more centered.", ch,
            NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

/*
 * n Acid Spells
 */
SPELLF spell_sulfurous_spray(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        level = UMIN(19, level);
        dam = 2 * level * number_range(1, 7) + 11;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_spell_staff(level, victim))
                dam /= 4;
        act(AtMagic,
            "A stinking spray of sulfurous liquid rains down on $N.", ch,
            NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_caustic_fount(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        level = UMIN(42, level);
        dam = 2 * level * number_range(1, 6) - 31;
        dam = UMAX(0, dam);

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_spell_staff(level, victim))
                dam = dam * 3 / 4;
        act(AtMagic,
            "A fountain of caustic liquid forms below $N.  The smell of $S degenerating tissues is revolting! ",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_acetum_primus(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        dam = 2 * level * number_range(1, 4) + 7;

        if (saves_spell_staff(level, victim))
                dam = 3 * dam / 4;
        act(AtMagic,
            "A cloak of primal acid enshrouds $N, sparks form as it consumes all it touches. ",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

/*
 *  Electrical
 */

SPELLF spell_galvanic_whip(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        level = UMIN(10, level);
        dam = level * number_range(1, 6) + 5;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic,
            "$n conjures a whip of ionized particles, which lashes ferociously at $N.",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_magnetic_thrust(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        dam = (level * number_range(1, 6)) + 16;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic,
            "An unseen energy moves nearby, causing your hair to stand on end!",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_quantum_spike(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam, l;

        level = UMAX(0, level);
        l = UMAX(1, level - 90);
        dam = l * number_range(1, 40) + 145;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_spell_staff(level, victim))
                dam /= 2;
        act(AtMagic,
            "$N seems to dissolve into tiny unconnected particles, then is painfully reassembled.",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

/*
 * Black-magicish guys
 */

/* L2 Mage Spell */
SPELLF spell_black_hand(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        level = UMIN(5, level);
        dam = level * number_range(1, 6) + 3;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_poison_death(level, victim))
                dam /= 4;
        act(AtMagic,
            "$n conjures a mystical hand, which swoops menacingly at $N.", ch,
            NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_black_fist(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        level = UMAX(0, level);
        dam = level * number_range(1, 9) + 4;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);


        if (saves_poison_death(level, victim))
                dam /= 4;
        act(AtMagic,
            "$n forms a fist with the Force, which swoops menacingly at $N.",
            ch, NULL, victim, ToNotvict);
        return damage(ch, victim, dam, sn);
}

SPELLF spell_black_lightning(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim = (CharData *) vo;
        int       dam;

        dam = 100;

        send_to_char("You feel the hatred grow within you!\n\r", ch);
        ch->alignment = ch->alignment - 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);
        sith_penalty(ch);

        act(AtBlue,
            "Bolts of electricity shoot from the fingers of $n, sending $N into a fit of painful spasms.",
            ch, NULL, victim, ToNotvict);
        act(AtBlue,
            "Bolts of electricity shoot from your fingertips, sending $N into a fit of painful spasms.",
            ch, NULL, victim, ToChar);
        act(AtBlue,
            "Intense pain spreads through your body as bolts of electricity from $N assault you.",
            victim, NULL, ch, ToChar);

        if (saves_poison_death(level, victim))
                return damage(ch, victim, dam, sn);
        else
        {
                damage(ch, victim, dam, sn);
                if (char_died(victim))
                        return rCHAR_DIED;
                if (spell_black_lightning(sn, level, ch, vo) == rCHAR_DIED)
                        return rCHAR_DIED;
                return rNONE;
        }
}



SPELLF spell_force_healing(int sn, int level, CharData * ch, void *vo)
{
        CharData *victim;

        vo = NULL;
        sn = 0;
        level = 0;

        if (target_name[0] == '\0')
                victim = ch;
        else if ((victim = get_char_room(ch, target_name)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return rSPELL_FAILED;
        }

        act(AtGreen, "Glowing green light emanates from $n.", ch, NULL, NULL,
            ToRoom);
        if (victim == ch)
                send_to_char("Using the Force, you mend your wounds.\n\r",
                             ch);
        if (victim != ch)
        {
                ch_printf(ch, "Using the Force, you mend %s's wounds.\n\r",
                          victim->name);
                act(AtPlain, "$n uses the Force to mend your wounds.", ch,
                    NULL, victim, ToVict);
        }
        if (victim != ch)
        {
                ch->alignment = ch->alignment + 50;
                ch->alignment = URANGE(-1000, ch->alignment, 1000);
                jedi_bonus(ch);
        }

        victim->hit +=
                (number_percent() +
                 (victim ==
                  ch ? (int) ch->pcdata->
                  learned[gsn_force_healing] : (int) ch->pcdata->
                  learned[gsn_force_healing] / 2));
        if (victim->hit > victim->max_hit)
                victim->hit = victim->max_hit;

        return rNONE;
}

CMDF do_meditate(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       chance = 0, boost = 0, xp = 0;

        mudstrlcpy(arg, argument, MIL);

        if (IsNpc(ch)) /* Droids can't meditate! */
        {
                send_to_char
                        ("&RMeditate probably wouldn't do you much good.\n\r",
                         ch);
                return;
        }
        if (!str_cmp(ch->race->name(), "droid"))
        {
                send_to_char("&zOB-1 Kenobi? Droids can't meditate.\n\r", ch);
                return;
        }

        switch (ch->substate)
        {
        default:

                send_to_char
                        ("&bYou meditate peacefully, tuning yourself to the Force.\n\r",
                         ch);
                act(AtPlain,
                    "$n begins to meditate on the ways of the Force.", ch,
                    NULL, argument, ToRoom);
                add_timer(ch, TimerDoFun, 2, do_meditate, 1);
                ch->dest_buf = str_dup(arg);
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
                        ("&bYou rise from your trance, pushing the thoughts aside.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;

        chance = IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                     learned[gsn_meditate]);

        if (ch->perm_frc <= 0)
                chance = -1;

        if (number_percent() > chance)
        {
                send_to_char
                        ("&bYou spend much time in a deep mediation, but fail to atune yourself to the Force.\n\r",
                         ch);
                learn_from_failure(ch, gsn_meditate);
                return;
        }

        send_to_char
                ("&bThe Force swirls and you can feel the life surrounding you.&w\n\r",
                 ch);

        /*
         * Mana Increase 
         */
        boost = (ch->perm_frc * chance) / 10;
        ch->endurance = URANGE(0, ch->endurance + boost, ch->max_endurance);

        /*
         * Force Experience --- Only up to level 10 
         */
        if (ch->skill_level[ForceAbility] <= 10)
        {
                xp = chance;
                xp = URANGE(0, xp,
                            (exp_level(ch->skill_level[ForceAbility] + 1) -
                             exp_level(ch->skill_level[ForceAbility])) / 35);
                ch_printf(ch, "You gain %d Force experience.\n\r", xp);
                gain_exp(ch, xp, ForceAbility);
        }

        learn_from_success(ch, gsn_meditate);
}
