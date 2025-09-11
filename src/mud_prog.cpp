/*****************************************************************************************
 *                       DDDDD        A        RRRRRRR     K    K                        *
 *                       D    D      A A       R      R    K   K                         *
 *                       D     D    A   A      R      R    KK K                          *
 *                       D     D   A     A     RRRRRRR     K K                           *
 *                       D     D  AAAAAAAAA    R    R      K  K                          *
 *                       D    D  A         A   R     R     K   K                         *
 *                       DDDDD  A           A  R      R    K    K                        *
 *                                                                                       *
 *                                                                                       *
 *W      WW      W    A        RRRRRRR   RRRRRRR   IIIIIIII    OOOO   RRRRRRR     SSSSS  *
 * W    W  W    W    A A       R      R  R      R     II      O    O  R      R   S       *
 * W    W  W    W   A   A      R      R  R      R     II     O      O R      R   S       *
 * W    W  W    W  A     A     RRRRRRR   RRRRRRR      II     O      O RRRRRRR     SSSSS  *
 *  W  W    W  W  AAAAAAAAA    R    R    R    R       II     O      O R    R           S *
 *  W W     W W  A         A   R     R   R     R      II      O    O  R     R          S *
 *   W       W  A           A  R      R  R      R  IIIIIIII    OOOO   R      R    SSSSS  *
 *                                                                                       *
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
 *                $Id: mud_prog.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mud.hpp"
#include "races.hpp"
#include "bounty.hpp"

/* Defines by Narn for new mudprog parsing, used as 
   return values from mprog_do_command. */
#define COMMANDOK    1
#define IFTRUE       2
#define IFFALSE      3
#define ORTRUE       4
#define ORFALSE      5
#define FOUNDELSE    6
#define FOUNDENDIF   7
#define IFIGNORED    8
#define ORIGNORED    9

/* Ifstate defines, used to create and access ifstate array
   in mprog_driver. */

int       mprog_do_command(char *cmnd, CharData * mob, CharData * actor,
                           ObjData * obj, void *vo, CharData * rndm,
                           bool ignore, bool ignore_ors);

/*
 *  Mudprogram additions
 */
CharData *supermob;
struct act_prog_data *room_act_list;
struct act_prog_data *obj_act_list;
struct act_prog_data *mob_act_list;

MpsleepData *first_mpsleep = NULL;
MpsleepData *last_mpsleep = NULL;
MpsleepData *current_mpsleep = NULL;


char     *strip_tilde(char *str);



/*
 * Local function prototypes
 */

char     *mprog_next_command args((char *clist));
bool mprog_seval args((char *lhs, char *opr, char *rhs, CharData * mob));
bool mprog_veval args((int lhs, char *opr, int rhs, CharData * mob));
int mprog_do_ifcheck args((char *ifcheck, CharData * mob,
                           CharData * actor, ObjData * obj,
                           void *vo, CharData * rndm));
void mprog_translate args((char ch, char *t, CharData * mob,
                           CharData * actor, ObjData * obj,
                           void *vo, CharData * rndm));
void mprog_driver args((char *com_list, CharData * mob,
                        CharData * actor, ObjData * obj,
                        void *vo, bool single_step));

bool mprog_keyword_check args((const char *argu, const char *argl));


void      oprog_wordlist_check(char *arg, CharData * mob, CharData * actor,
                               ObjData * obj, void *vo, int type,
                               ObjData * iobj);
void      set_supermob(ObjData * obj);
bool      oprog_percent_check(CharData * mob, CharData * actor,
                              ObjData * obj, void *vo, int type);
void      rprog_percent_check(CharData * mob, CharData * actor,
                              ObjData * obj, void *vo, int type);
void      rprog_wordlist_check(char *arg, CharData * mob, CharData * actor,
                               ObjData * obj, void *vo, int type,
                               RoomIndexData * room);

/***************************************************************************
 * Local function code and brief comments.
 */

/* if you dont have these functions, you damn well should... */

#ifdef DunnoStrstr
char     *strstr(s1, s2)
     const char *s1;
     const char *s2;
{
        char     *cp;
        int       i, j = strlen(s1) - strlen(s2), k = strlen(s2);

        if (j < 0)
                return NULL;
        for (i = 0; i <= j && strncmp(s1++, s2, k) != 0; i++);
        return (i > j) ? NULL : (s1 - 1);
}
#endif

#define RID RoomIndexData

void init_supermob(void)
{
        RoomIndexData *office;

        supermob = create_mobile(get_mob_index(3));
        office = get_room_index(3);
        char_to_room(supermob, office);

#ifdef NOTDEFD
        CREATE(supermob, CharData, 1);
        clear_char(supermob);

        SetBit(supermob->act, ActIsNpc);
        supermob->name = STRALLOC("supermob");
        supermob->short_descr = STRALLOC(".");
        supermob->long_descr = STRALLOC(".");

        CREATE(supermob_index, MobIndexData, 1)
#endif
}


#undef RID


/* Used to get sequential lines of a multi line string (separated by "\n\r")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char     *mprog_next_command(char *clist)
{
        bool      Append = FALSE;
        char     *pointer = clist;

        while (*pointer != '\0')
        {
                if (Append == FALSE && (*pointer == '\n' || *pointer == '\r'))
                        break;
                if (Append == TRUE && *pointer != '\n' && *pointer != '\r')
                        Append = FALSE;
                else if (Append == TRUE
                         && (*pointer == '\n' || *pointer == '\r'))
                {
/* 
       A tilde is one symbol we can be absolutely certain never 
       to have as part of a prog, so can replace the returns with 
       tildes, and then later we will strip all tildes from the
       output before its parse the final time. 
*/
                        *pointer = '~';
                }
                if (*pointer == '+')
                {
                        Append = TRUE;
                        *pointer = ' ';
                }
                pointer++;
        }
        if (*pointer == '\n')
                *pointer++ = '\0';
        if (*pointer == '\r')
                *pointer++ = '\0';

        return (pointer);
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval(char *lhs, char *opr, char *rhs, CharData * mob)
{

        if (!str_cmp(opr, "=="))
                return (bool) (!str_cmp(lhs, rhs));
        if (!str_cmp(opr, "!="))
                return (bool) (str_cmp(lhs, rhs));
        if (!str_cmp(opr, "/"))
                return (bool) (!str_infix(rhs, lhs));
        if (!str_cmp(opr, "!/"))
                return (bool) (str_infix(rhs, lhs));

        snprintf(log_buf, MSL, "Improper MOBprog operator '%s'", opr);
        progbug(log_buf, mob);
        return 0;

}

bool mprog_veval(int lhs, char *opr, int rhs, CharData * mob)
{
        if (!str_cmp(opr, "=="))
                return (lhs == rhs);
        if (!str_cmp(opr, "!="))
                return (lhs != rhs);
        if (!str_cmp(opr, ">"))
                return (lhs > rhs);
        if (!str_cmp(opr, "<"))
                return (lhs < rhs);
        if (!str_cmp(opr, "<="))
                return (lhs <= rhs);
        if (!str_cmp(opr, ">="))
                return (lhs >= rhs);
        if (!str_cmp(opr, "&"))
                return (lhs & rhs);
        if (!str_cmp(opr, "|"))
                return (lhs | rhs);

        snprintf(log_buf, MSL, "Improper MOBprog operator '%s'", opr);
        progbug(log_buf, mob);

        return 0;

}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifcheck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return BERR otherwise return boolean 1,0
 * Redone by Altrag.. kill all that big copy-code that performs the
 * same action on each variable..
 */
int mprog_do_ifcheck(char *ifcheck, CharData * mob, CharData * actor,
                     ObjData * obj, void *vo, CharData * rndm)
{
        char      cvar[MaxInputLength];
        char      chck[MaxInputLength];
        char      opr[MaxInputLength];
        char      rval[MaxInputLength];
        char     *point = ifcheck;
        char     *pchck = chck;
        CharData *chkchar = NULL;
        ObjData *chkobj = NULL;
        int       lhsvl, rhsvl;

        if (!*point)
        {
                progbug("Null ifcheck", mob);
                return BERR;
        }
        while (*point == ' ')
                point++;
        while (*point != '(')
                if (*point == '\0')
                {
                        progbug("Ifcheck syntax error", mob);
                        return BERR;
                }
                else if (*point == ' ')
                        point++;
                else
                        *pchck++ = *point++;
        *pchck = '\0';
        point++;
        pchck = cvar;
        while (*point != ')')
                if (*point == '\0')
                {
                        progbug("Ifcheck syntax error", mob);
                        return BERR;
                }
                else if (*point == ' ')
                        point++;
                else
                        *pchck++ = *point++;
        point++;
/*	{
	char buf[MSL];
	snprintf(buf, MSL, "The value of point is: %s, pchck is : %s, ifcheck: %s, chck: %s, lhsvl: %d. rhsvl: %d", point, pchck, ifcheck, chck, lhsvl, rhsvl);
	do_say(mob, buf);
	}*/
        *pchck = '\0';
        while (*point == ' ')
                point++;
        if (!*point)
        {
                opr[0] = '\0';
                rval[0] = '\0';
        }
        else
        {
                pchck = opr;
                while (*point != ' ' && !isalnum(*point))
                        if (*point == '\0')
                        {
                                progbug("Ifcheck operator without value",
                                        mob);
                                return BERR;
                        }
                        else
                                *pchck++ = *point++;
                *pchck = '\0';

                while (*point == ' ')
                        point++;
                pchck = rval;
                while (*point != '\0' && *point != '\0')
                        *pchck++ = *point++;
                *pchck = '\0';
        }

        /*
         * chck contains check, cvar is the variable in the (), opr is the
         * * operator if there is one, and rval is the value if there was an
         * * operator.
         */
        if (cvar[0] == '$')
        {
                switch (cvar[1])
                {
                case 'i':
                        chkchar = mob;
                        break;
                case 'n':
                        chkchar = actor;
                        break;
                case 't':
                        chkchar = (CharData *) vo;
                        break;
                case 'r':
                        chkchar = rndm;
                        break;
                case 'o':
                        chkobj = obj;
                        break;
                case 'p':
                        chkobj = (ObjData *) vo;
                        break;
                default:
                        snprintf(rval, MSL, "Bad argument '%c' to '%s'",
                                 cvar[0], chck);
                        progbug(rval, mob);
                        return BERR;
                }
                if (!chkchar && !chkobj)
                        return BERR;
        }
        if (!str_cmp(chck, "rand"))
        {
                return (number_percent() <= atoi(cvar));
        }
        if (!str_cmp(chck, "economy"))
        {
                int       idx = atoi(cvar);
                RoomIndexData *room;

                if (!idx)
                {
                        if (!mob->in_room)
                        {
                                progbug("'economy' ifcheck: mob in NULL room with no room vnum " "argument", mob);
                                return BERR;
                        }
                        room = mob->in_room;
                }
                else
                        room = get_room_index(idx);
                if (!room)
                {
                        progbug("Bad room vnum passed to 'economy'", mob);
                        return BERR;
                }
                return mprog_veval(((room->area->high_economy >
                                     0) ? 1000000000 : 0) +
                                   room->area->low_economy, opr, atoi(rval),
                                   mob);
        }
        if (!str_cmp(chck, "mobinroom"))
        {
                int       vnum = atoi(cvar);

                /*
                 * int lhsvl; - Shadow Declaration - Gavin 
                 */
                CharData *oMob;

                if (vnum < 1 || vnum > MaxVnums)
                {
                        progbug("Bad vnum to 'mobinroom'", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (oMob = mob->in_room->first_person; oMob;
                     oMob = oMob->next_in_room)
                        if (IsNpc(oMob) && oMob->pIndexData->vnum == vnum)
                                lhsvl++;
                rhsvl = atoi(rval);
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "timeskilled"))
        {
                MobIndexData *pMob;

                if (chkchar)
                        pMob = chkchar->pIndexData;
                else if (!(pMob = get_mob_index(atoi(cvar))))
                {
                        progbug("TimesKilled ifcheck: bad vnum", mob);
                        return BERR;
                }
                return mprog_veval(pMob->killed, opr, atoi(rval), mob);
        }
        if (!str_cmp(chck, "ovnumhere"))
        {
                ObjData *pObj;
                int       vnum = atoi(cvar);

                if (vnum < 1 || vnum > MaxVnums)
                {
                        progbug("OvnumHere: bad vnum", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj)
                            && pObj->pIndexData->vnum == vnum)
                                lhsvl++;
                for (pObj = mob->in_room->first_content; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj)
                            && pObj->pIndexData->vnum == vnum)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "otypehere"))
        {
                ObjData *pObj;
                int       type;

                if (is_number(cvar))
                        type = atoi(cvar);
                else
                        type = get_otype(cvar);
                if (type < 0 || type > MaxItemType)
                {
                        progbug("OtypeHere: bad type", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj) && pObj->item_type == type)
                                lhsvl++;
                for (pObj = mob->in_room->first_content; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj) && pObj->item_type == type)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "ovnumroom"))
        {
                ObjData *pObj;
                int       vnum = atoi(cvar);

                if (vnum < 1 || vnum > MaxVnums)
                {
                        progbug("OvnumRoom: bad vnum", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->in_room->first_content; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj)
                            && pObj->pIndexData->vnum == vnum)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "otyperoom"))
        {
                ObjData *pObj;
                int       type;

                if (is_number(cvar))
                        type = atoi(cvar);
                else
                        type = get_otype(cvar);
                if (type < 0 || type > MaxItemType)
                {
                        progbug("OtypeRoom: bad type", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->in_room->first_content; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj) && pObj->item_type == type)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "ovnumcarry"))
        {
                ObjData *pObj;
                int       vnum = atoi(cvar);

                if (vnum < 1 || vnum > MaxVnums)
                {
                        progbug("OvnumCarry: bad vnum", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj)
                            && pObj->pIndexData->vnum == vnum)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "otypecarry"))
        {
                ObjData *pObj;
                int       type;

                if (is_number(cvar))
                        type = atoi(cvar);
                else
                        type = get_otype(cvar);
                if (type < 0 || type > MaxItemType)
                {
                        progbug("OtypeCarry: bad type", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (can_see_obj(mob, pObj) && pObj->item_type == type)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "ovnumwear"))
        {
                ObjData *pObj;
                int       vnum = atoi(cvar);

                if (vnum < 1 || vnum > MaxVnums)
                {
                        progbug("OvnumWear: bad vnum", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (pObj->wear_loc != WearNone
                            && can_see_obj(mob, pObj)
                            && pObj->pIndexData->vnum == vnum)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "otypewear"))
        {
                ObjData *pObj;
                int       type;

                if (is_number(cvar))
                        type = atoi(cvar);
                else
                        type = get_otype(cvar);
                if (type < 0 || type > MaxItemType)
                {
                        progbug("OtypeWear: bad type", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (pObj->wear_loc != WearNone
                            && can_see_obj(mob, pObj)
                            && pObj->item_type == type)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "ovnuminv"))
        {
                ObjData *pObj;
                int       vnum = atoi(cvar);

                if (vnum < 1 || vnum > MaxVnums)
                {
                        progbug("OvnumInv: bad vnum", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (pObj->wear_loc == WearNone
                            && can_see_obj(mob, pObj)
                            && pObj->pIndexData->vnum == vnum)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (!str_cmp(chck, "otypeinv"))
        {
                ObjData *pObj;
                int       type;

                if (is_number(cvar))
                        type = atoi(cvar);
                else
                        type = get_otype(cvar);
                if (type < 0 || type > MaxItemType)
                {
                        progbug("OtypeInv: bad type", mob);
                        return BERR;
                }
                lhsvl = 0;
                for (pObj = mob->first_carrying; pObj;
                     pObj = pObj->next_content)
                        if (pObj->wear_loc == WearNone
                            && can_see_obj(mob, pObj)
                            && pObj->item_type == type)
                                lhsvl++;
                rhsvl = is_number(rval) ? atoi(rval) : -1;
                if (rhsvl < 1)
                        rhsvl = 1;
                if (!*opr)
                        mudstrlcpy(opr, "==", MIL);
                return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
        if (chkchar)
        {
                if (!str_cmp(chck, "ismobinvis"))
                {
                        return (IsNpc(chkchar)
                                && IsSet(chkchar->act, ActMobinvis));
                }
                if (!str_cmp(chck, "mobinvislevel"))
                {
                        return (IsNpc(chkchar) ?
                                mprog_veval(chkchar->mobinvis, opr,
                                            atoi(rval), mob) : FALSE);
                }
                if (!str_cmp(chck, "ispc"))
                {
                        return IsNpc(chkchar) ? FALSE : TRUE;
                }
                if (!str_cmp(chck, "isnpc"))
                {
                        return IsNpc(chkchar) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "ismounted"))
                {
                        return (chkchar->position == PosMounted);
                }
                if (!str_cmp(chck, "isgood"))
                {
                        return IsGood(chkchar) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "isneutral"))
                {
                        return IsNeutral(chkchar) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "isevil"))
                {
                        return IsEvil(chkchar) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "isfight"))
                {
                        return who_fighting(chkchar) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "isimmort"))
                {
                        return (get_trust(chkchar) >= LevelImmortal);
                }
                if (!str_cmp(chck, "ischarmed"))
                {
                        return IsAffected(chkchar, AffCharm) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "isfollow"))
                {
                        return (chkchar->master != NULL &&
                                chkchar->master->in_room == chkchar->in_room);
                }
                if (!str_cmp(chck, "isaffected"))
                {
                        int       value = get_aflag(rval);

                        if (value < 0 || value > 31)
                        {
                                progbug("Unknown affect being checked", mob);
                                return BERR;
                        }
                        return IsAffected(chkchar,
                                           1 << value) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "iswanted"))
                {
                        if (IsNpc(chkchar) || !mob->in_room
                            || !mob->in_room->area
                            || !mob->in_room->area->planet)
                                return FALSE;
                        return is_wanted(chkchar,
                                         mob->in_room->area->
                                         planet) ? TRUE : FALSE;
                }
                if (!str_cmp(chck, "hitprcnt"))
                {
                        return mprog_veval(chkchar->hit / chkchar->max_hit,
                                           opr, atoi(rval), mob);
                }
                if (!str_cmp(chck, "inroom"))
                {
                        return mprog_veval(chkchar->in_room->vnum, opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "wasinroom"))
                {
                        return mprog_veval(chkchar->was_in_room->vnum, opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "sex"))
                {
                        return mprog_veval(chkchar->sex, opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "position"))
                {
                        return mprog_veval(chkchar->position, opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "doingquest"))
                {
                        return IsNpc(actor) ? FALSE :
                                mprog_veval(chkchar->PCData->quest_number,
                                            opr, atoi(rval), mob);
                }
                if (!str_cmp(chck, "ishelled"))
                {
                        return IsNpc(actor) ? FALSE :
                                mprog_veval(chkchar->PCData->release_date,
                                            opr, atoi(rval), mob);
                }

                if (!str_cmp(chck, "level"))
                {
                        return mprog_veval(get_trust(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "combatlevel"))
                {
                        return mprog_veval(chkchar->skill_level[0], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "pilotlevel"))
                {
                        return mprog_veval(chkchar->skill_level[1], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "engineeringlevel"))
                {
                        return mprog_veval(chkchar->skill_level[2], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "huntinglevel"))
                {
                        return mprog_veval(chkchar->skill_level[3], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "smugglinglevel"))
                {
                        return mprog_veval(chkchar->skill_level[4], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "diplomacylevel"))
                {
                        return mprog_veval(chkchar->skill_level[5], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "leadershiplevel"))
                {
                        return mprog_veval(chkchar->skill_level[6], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "forcelevel"))
                {
                        return mprog_veval(chkchar->skill_level[7], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "occupationlevel"))
                {
                        return mprog_veval(chkchar->skill_level[8], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "piracylevel"))
                {
                        return mprog_veval(chkchar->skill_level[9], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "mediclevel"))
                {
                        return mprog_veval(chkchar->skill_level[10], opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "goldamt"))
                {
                        return mprog_veval(chkchar->gold, opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "race"))
                {
                        if (IsNpc(chkchar))
                                return mprog_seval(chkchar->race->name(), opr,
                                                   rval, mob);
                        return mprog_seval((char *) chkchar->race->name(),
                                           opr, rval, mob);
                }
                /*
                 * Check added to see if the person isleader of == clan Shaddai 
                 */
                if (!str_cmp(chck, "isleader"))
                {
                        ClanData *temp;

                        if (IsNpc(chkchar))
                                return FALSE;
                        if ((temp = get_clan(rval)) == NULL)
                                return FALSE;
                        if (mprog_seval(chkchar->name, opr, temp->leader, mob)
                            || mprog_seval(chkchar->name, opr, temp->number1,
                                           mob)
                            || mprog_seval(chkchar->name, opr, temp->number2,
                                           mob))
                                return TRUE;
                        else
                                return FALSE;
                }
                if (!str_cmp(chck, "isplanetclanmember"))
                {
                        ClanData *temp;

                        if (IsNpc(chkchar))
                                return FALSE;
                        if (!chkchar->PCData || !chkchar->PCData->clan || !chkchar->in_room
							|| !chkchar->in_room->area || !chkchar->in_room->area->planet 
							|| !chkchar->in_room->area->planet->governed_by)
                                return FALSE;
						if ( chkchar->in_room->area->planet->governed_by == chkchar->PCData->clan )
							return TRUE;
						else
							return FALSE;
                }
                /*
                 * Is char wearing some eq on a specific wear loc?  -- Gorog 
                 */
                if (!str_cmp(chck, "wearing"))
                {
                        ObjData *temp_obj;

                        for (temp_obj = chkchar->first_carrying; temp_obj;
                             temp_obj = temp_obj->next_content)
                        {
                                if (chkchar == temp_obj->carried_by
                                    && temp_obj->wear_loc > -1
                                    &&
                                    !str_infix(wear_locs[temp_obj->wear_loc],
                                               rval))
                                        return TRUE;
                        }
                        return FALSE;
                }

                /*
                 * Is char wearing some specific vnum?  -- Gorog 
                 */
                if (!str_cmp(chck, "wearingvnum"))
                {
                        ObjData *temp_obj;

                        if (!is_number(rval))
                                return FALSE;
                        for (temp_obj = chkchar->first_carrying; temp_obj;
                             temp_obj = temp_obj->next_content)
                        {
                                if (chkchar == temp_obj->carried_by
                                    && temp_obj->wear_loc > -1
                                    && temp_obj->pIndexData->vnum ==
                                    atoi(rval))
                                        return TRUE;
                        }
                        return FALSE;
                }

                if (!str_cmp(chck, "clan"))
                {
                        if ((IsNpc(chkchar) && !chkchar->mob_clan)
                            || (!IsNpc(chkchar) && !chkchar->PCData->clan))
                                return FALSE;
                        if (IsNpc(chkchar))
                                return mprog_seval(chkchar->mob_clan, opr,
                                                   rval, mob);
                        else
                                return mprog_seval(chkchar->PCData->clan->
                                                   name, opr, rval, mob);
                }

                if (!str_cmp(chck, "clantype"))
                {
                        if (IsNpc(chkchar) || !chkchar->PCData->clan)
                                return FALSE;
                        return mprog_veval(chkchar->PCData->clan->ClanType,
                                           opr, atoi(rval), mob);
                }
                if (!str_cmp(chck, "str"))
                {
                        return mprog_veval(get_curr_str(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "wis"))
                {
                        return mprog_veval(get_curr_wis(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "int"))
                {
                        return mprog_veval(get_curr_int(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "dex"))
                {
                        return mprog_veval(get_curr_dex(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "con"))
                {
                        return mprog_veval(get_curr_con(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "cha"))
                {
                        return mprog_veval(get_curr_cha(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "lck"))
                {
                        return mprog_veval(get_curr_lck(chkchar), opr,
                                           atoi(rval), mob);
                }
                if (!str_cmp(chck, "frc"))
                {
                        return mprog_veval(get_curr_frc(chkchar), opr,
                                           atoi(rval), mob);
                }
        }
        if (chkobj)
        {
                if (!str_cmp(chck, "objtype"))
                {
                        return mprog_veval(chkobj->item_type, opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "objval0"))
                {
                        return mprog_veval(chkobj->value[0], opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "objval1"))
                {
                        return mprog_veval(chkobj->value[1], opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "objval2"))
                {
                        return mprog_veval(chkobj->value[2], opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "objval3"))
                {
                        return mprog_veval(chkobj->value[3], opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "objval4"))
                {
                        return mprog_veval(chkobj->value[4], opr, atoi(rval),
                                           mob);
                }
                if (!str_cmp(chck, "objval5"))
                {
                        return mprog_veval(chkobj->value[5], opr, atoi(rval),
                                           mob);
                }
        }
        /*
         * The following checks depend on the fact that cval[1] can only contain
         * one character, and that NULL checks were made previously. 
         */
        if (!str_cmp(chck, "number"))
        {
                if (chkchar)
                {
                        if (!IsNpc(chkchar))
                                return FALSE;
                        lhsvl = (chkchar ==
                                 mob) ? chkchar->gold : chkchar->pIndexData->
                                vnum;
                        return mprog_veval(lhsvl, opr, atoi(rval), mob);
                }
                return mprog_veval(chkobj->pIndexData->vnum, opr, atoi(rval),
                                   mob);
        }
        if (!str_cmp(chck, "name"))
        {
                if (chkchar)
                        return mprog_seval(chkchar->name, opr, rval, mob);
                return mprog_seval(chkobj->name, opr, rval, mob);
        }

        /*
         * Ok... all the ifchecks are done, so if we didnt find ours then something
         * * odd happened.  So report the bug and abort the MUDprogram (return error)
         */
        progbug("Unknown ifcheck", mob);
        return BERR;
}


/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object 
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */

/*
 *  There's no reason to make the mud crash when a variable's
 *  fubared.  I added some ifs.  I'm willing to trade some 
 *  performance for stability. -Haus
 *
 *  Narn's fubar ***ANNIHILATES*** you!  Hmm, could we add that
 *  as a weapon type? -Narn
 *
 *  Added char_died and obj_extracted checks	-Thoric
 */
void mprog_translate(char ch, char *t, CharData * mob, CharData * actor,
                     ObjData * obj, void *vo, CharData * rndm)
{
        static char *he_she[] = { "it", "he", "she" };
        static char *him_her[] = { "it", "him", "her" };
        static char *his_her[] = { "its", "his", "her" };
        CharData *vict = (CharData *) vo;
        ObjData *v_obj = (ObjData *) vo;

        *t = '\0';
        switch (ch)
        {
        case 'i':
                if (mob && !char_died(mob))
                {
                        if (mob->name)
                                one_argument(mob->name, t);
                }
                else
                        mudstrlcpy(t, "someone", MIL);
                break;

        case 'I':
                if (mob && !char_died(mob))
                {
                        if (mob->short_descr)
                        {
                                mudstrlcpy(t, mob->short_descr, MIL);
                        }
                        else
                        {
                                mudstrlcpy(t, "someone", MIL);
                        }
                }
                else
                        mudstrlcpy(t, "someone", MIL);
                break;

        case 'n':
                if (actor && !char_died(actor))
                {
                        one_argument(actor->name, t);
                        if (!IsNpc(actor))
                                *t = UPPER(*t);
                }
                else
                        mudstrlcpy(t, "someone", MIL);
                break;

        case 'N':
                if (actor && !char_died(actor))
                {
                        if (can_see(mob, actor))
                                if (IsNpc(actor))
                                        mudstrlcpy(t, actor->short_descr,
                                                   MIL);
                                else
                                {
                                        mudstrlcpy(t, actor->name, MIL);
                                        mudstrlcat(t, actor->PCData->title,
                                                   MIL);
                                }
                        else
                                mudstrlcpy(t, "someone", MIL);
                }
                else
                        mudstrlcpy(t, "someone", MIL);
                break;

        case 't':
                if (vict && !char_died(vict))
                {
                        one_argument(vict->name, t);
                        if (!IsNpc(vict))
                                *t = UPPER(*t);
                }
                else
                        mudstrlcpy(t, "someone", MIL);

                break;

        case 'T':
                if (vict && !char_died(vict))
                {
                        if (can_see(mob, vict))
                                if (IsNpc(vict))
                                        mudstrlcpy(t, vict->short_descr, MIL);
                                else
                                {
                                        mudstrlcpy(t, vict->name, MIL);
                                        mudstrlcat(t, " ", MIL);
                                        mudstrlcat(t, vict->PCData->title,
                                                   MIL);
                                }
                        else
                                mudstrlcpy(t, "someone", MIL);
                }
                else
                        mudstrlcpy(t, "someone", MIL);
                break;

        case 'r':
                if (rndm && !char_died(rndm))
                {
                        one_argument(rndm->name, t);
                        if (!IsNpc(rndm))
                        {
                                *t = UPPER(*t);
                        }
                }
                else
                        mudstrlcpy(t, "someone", MIL);
                break;

        case 'R':
                if (rndm && !char_died(rndm))
                {
                        if (can_see(mob, rndm))
                                if (IsNpc(rndm))
                                        mudstrlcpy(t, rndm->short_descr, MIL);
                                else
                                {
                                        mudstrlcpy(t, rndm->name, MIL);
                                        mudstrlcat(t, " ", MIL);
                                        mudstrlcat(t, rndm->PCData->title,
                                                   MIL);
                                }
                        else
                                mudstrlcpy(t, "someone", MIL);
                }
                else
                        mudstrlcpy(t, "someone", MIL);
                break;

        case 'e':
                if (actor && !char_died(actor))
                {
                        can_see(mob, actor) ? mudstrlcpy(t,
                                                         he_she[actor->sex],
                                                         MIL) : mudstrlcpy(t,
                                                                           "someone",
                                                                           MIL);
                }
                else
                        mudstrlcpy(t, "it", MIL);
                break;

        case 'm':
                if (actor && !char_died(actor))
                {
                        can_see(mob, actor) ? mudstrlcpy(t,
                                                         him_her[actor->sex],
                                                         MIL) : mudstrlcpy(t,
                                                                           "someone",
                                                                           MIL);
                }
                else
                        mudstrlcpy(t, "it", MIL);
                break;

        case 's':
                if (actor && !char_died(actor))
                {
                        can_see(mob, actor) ? mudstrlcpy(t,
                                                         his_her[actor->sex],
                                                         MIL) : mudstrlcpy(t,
                                                                           "someone's",
                                                                           MIL);
                }
                else
                        mudstrlcpy(t, "its'", MIL);
                break;

        case 'E':
                if (vict && !char_died(vict))
                {
                        can_see(mob, vict) ? mudstrlcpy(t, he_she[vict->sex],
                                                        MIL) : mudstrlcpy(t,
                                                                          "someone",
                                                                          MIL);
                }
                else
                        mudstrlcpy(t, "it", MIL);
                break;

        case 'M':
                if (vict && !char_died(vict))
                {
                        can_see(mob, vict) ? mudstrlcpy(t, him_her[vict->sex],
                                                        MIL) : mudstrlcpy(t,
                                                                          "someone",
                                                                          MIL);
                }
                else
                        mudstrlcpy(t, "it", MIL);
                break;

        case 'S':
                if (vict && !char_died(vict))
                {
                        can_see(mob, vict) ? mudstrlcpy(t, his_her[vict->sex],
                                                        MIL) : mudstrlcpy(t,
                                                                          "someone's",
                                                                          MIL);
                }
                else
                        mudstrlcpy(t, "its'", MIL);
                break;

        case 'j':
                if (mob && !char_died(mob))
                {
                        mudstrlcpy(t, he_she[mob->sex], MIL);
                }
                else
                {
                        mudstrlcpy(t, "it", MIL);
                }
                break;

        case 'k':
                if (mob && !char_died(mob))
                {
                        mudstrlcpy(t, him_her[mob->sex], MIL);
                }
                else
                {
                        mudstrlcpy(t, "it", MIL);
                }
                break;

        case 'l':
                if (mob && !char_died(mob))
                {
                        mudstrlcpy(t, his_her[mob->sex], MIL);
                }
                else
                {
                        mudstrlcpy(t, "it", MIL);
                }
                break;

        case 'J':
                if (rndm && !char_died(rndm))
                {
                        can_see(mob, rndm) ? mudstrlcpy(t, he_she[rndm->sex],
                                                        MIL) : mudstrlcpy(t,
                                                                          "someone",
                                                                          MIL);
                }
                else
                        mudstrlcpy(t, "it", MIL);
                break;

        case 'K':
                if (rndm && !char_died(rndm))
                {
                        can_see(mob, rndm) ? mudstrlcpy(t, him_her[rndm->sex],
                                                        MIL) : mudstrlcpy(t,
                                                                          "someone's",
                                                                          MIL);
                }
                else
                        mudstrlcpy(t, "its'", MIL);
                break;

        case 'L':
                if (rndm && !char_died(rndm))
                {
                        can_see(mob, rndm) ? mudstrlcpy(t, his_her[rndm->sex],
                                                        MIL) : mudstrlcpy(t,
                                                                          "someone",
                                                                          MIL);
                }
                else
                        mudstrlcpy(t, "its", MIL);
                break;

        case 'o':
                if (obj && !obj_extracted(obj))
                {
                        can_see_obj(mob,
                                    obj) ? (size_t) one_argument(obj->name,
                                                                 t) :
                                mudstrlcpy(t, "something", MIL);
                }
                else
                        mudstrlcpy(t, "something", MIL);
                break;

        case 'O':
                if (obj && !obj_extracted(obj))
                {
                        can_see_obj(mob, obj) ? mudstrlcpy(t,
                                                           obj->short_descr,
                                                           MIL) :
                                mudstrlcpy(t, "something", MIL);
                }
                else
                        mudstrlcpy(t, "something", MIL);
                break;

        case 'p':
                if (v_obj && !obj_extracted(v_obj))
                {
                        /*
                         * Stupid warnings about different types 
                         */
                        can_see_obj(mob,
                                    v_obj) ? (size_t) one_argument(v_obj->
                                                                   name,
                                                                   t) :
                                mudstrlcpy(t, "something", MIL);
                }
                else
                        mudstrlcpy(t, "something", MIL);
                break;

        case 'P':
                if (v_obj && !obj_extracted(v_obj))
                {
                        can_see_obj(mob, v_obj) ? mudstrlcpy(t,
                                                             v_obj->
                                                             short_descr,
                                                             MIL) :
                                mudstrlcpy(t, "something", MIL);
                }
                else
                        mudstrlcpy(t, "something", MIL);
                break;

        case 'a':
                if (obj && !obj_extracted(obj))
                {
                        mudstrlcpy(t, aoran(obj->name), MIL);
/*
          switch ( *( obj->name ) )
	  {
	    case 'a': case 'e': case 'i':
            case 'o': case 'u': mudstrlcpy( t, "an" , MIL);
	      break;
            default: mudstrlcpy( t, "a" , MIL);
          }
*/
                }
                else
                        mudstrlcpy(t, "a", MIL);
                break;

        case 'A':
                if (v_obj && !obj_extracted(v_obj))
                {
                        mudstrlcpy(t, aoran(v_obj->name), MIL);
                }
                else
                        mudstrlcpy(t, "a", MIL);
                break;

        case '$':
                mudstrlcpy(t, "$", MIL);
                break;

        default:
                progbug("Bad $var", mob);
                break;
        }

        return;

}

/*  The main focus of the MOBprograms.  This routine is called 
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 *
 *  This function rewritten by Narn for Realms of Despair, Dec/95.
 *
 */
void mprog_driver(char *com_list, CharData * mob, CharData * actor,
                  ObjData * obj, void *vo, bool single_step)
{
        char      tmpcmndlst[MaxStringLength];
        char     *command_list;
        char     *cmnd;
        CharData *rndm = NULL;
        CharData *vch = NULL;
        int       count = 0;
        int       ignorelevel = 0;
        int       iflevel, result;
        bool      ifstate[MaxIfs][DoElse + 1];
        static int prog_nest;
        MpsleepData *mpsleep = NULL;
        char      arg[MaxInputLength];
        int       count2 = 0;

        if IsAffected
                (mob, AffCharm) return;

        /*
         * Next couple of checks stop program looping. -- Altrag 
         */
        if (mob == actor)
        {
                progbug("triggering oneself.", mob);
                return;
        }

        if (++prog_nest > MaxProgNest)
        {
                progbug("max_prog_nest exceeded.", mob);
                --prog_nest;
                return;
        }

        /*
         * Make sure all ifstate bools are set to FALSE 
         */
        for (iflevel = 0; iflevel < MaxIfs; iflevel++)
        {
                for (count = 0; count <= DoElse; count++)
                {
                        ifstate[iflevel][count] = FALSE;
                }
        }

        iflevel = 0;

        /*
         * get a random visible player who is in the room with the mob.
         *
         *  If there isn't a random player in the room, rndm stays NULL.
         *  If you do a $r, $R, $j, or $k with rndm = NULL, you'll crash
         *  in mprog_translate.
         *
         *  Adding appropriate error checking in mprog_translate.
         *    -Haus
         *
         * This used to ignore players MaxLevel - 3 and higher (standard
         * Merc has 4 immlevels).  Thought about changing it to ignore all
         * imms, but decided to just take it out.  If the mob can see you, 
         * you may be chosen as the random player. -Narn
         *
         */

        count = 0;
        for (vch = mob->in_room->first_person; vch; vch = vch->next_in_room)
                if (!IsNpc(vch))
                {
                        if (number_range(0, count) == 0)
                                rndm = vch;
                        count++;
                }

        mudstrlcpy(tmpcmndlst, com_list, MSL);
        command_list = tmpcmndlst;

        /*
         * mpsleep - Restore the environment -rkb 
         */
        if (current_mpsleep)
        {
                ignorelevel = current_mpsleep->ignorelevel;
                iflevel = current_mpsleep->iflevel;
                if (single_step)
                        mob->mpscriptpos = 0;
                for (count = 0; count < MaxIfs; count++)
                {
                        for (count2 = 0; count2 <= DoElse; count2++)
                                ifstate[count][count2] =
                                        current_mpsleep->
                                        ifstate[count][count2];
                }
                current_mpsleep = NULL;
        }


        if (single_step)
        {
                if (mob->mpscriptpos > (int) strlen(tmpcmndlst))
                        mob->mpscriptpos = 0;
                else
                        command_list += mob->mpscriptpos;
                if (*command_list == '\0')
                {
                        command_list = tmpcmndlst;
                        mob->mpscriptpos = 0;
                }
        }

        /*
         * From here on down, the function is all mine.  The original code
         * did not support nested ifs, so it had to be redone.  The max 
         * logiclevel (MaxIfs) is defined at the beginning of this file, 
         * use it to increase/decrease max allowed nesting.  -Narn 
         */

        while (TRUE)
        {
                /*
                 * With these two lines, cmnd becomes the current line from the prog,
                 * and command_list becomes everything after that line. 
                 */
                cmnd = command_list;
                command_list = mprog_next_command(command_list);

                /*
                 * Are we at the end? 
                 */
                if (cmnd[0] == '\0')
                {
                        if (ifstate[iflevel][InIf]
                            || ifstate[iflevel][InElse])
                        {
                                progbug("Missing endif", mob);
                        }
                        --prog_nest;
                        return;
                }
                cmnd = strip_tilde(cmnd);
                /*
                 * mpsleep - Check if we should sleep -rkb 
                 */
                one_argument(cmnd, arg);
                if (!str_cmp(arg, "mpsleep"))
                {
/*                    if ( (ifstate[iflevel][InIf] == TRUE && ifstate[iflevel][DoIf] == FALSE) ||     // if we are in an if/else and we
                       (ifstate[iflevel][InElse] == TRUE && ifstate[iflevel][DoElse] == FALSE) )  // dont want to execute, dont..
                    {
                    }
                    else
                    {*/
                        CREATE(mpsleep, MpsleepData, 1);

                        /*
                         * State variables 
                         */
                        mpsleep->ignorelevel = ignorelevel;
                        mpsleep->iflevel = iflevel;
                        for (count = 0; count < MaxIfs; count++)
                        {
                                for (count2 = 0; count2 <= DoElse; count2++)
                                {
                                        mpsleep->ifstate[count][count2] =
                                                ifstate[count][count2];
                                }
                        }

                        /*
                         * Driver arguments 
                         */
                        mpsleep->com_list = STRALLOC(command_list);
                        mpsleep->mob = mob;
                        mpsleep->actor = actor;
                        mpsleep->obj = obj;
                        mpsleep->vo = vo;
                        mpsleep->single_step = single_step;

                        /*
                         * Time to sleep 
                         */
/*                        cmnd = one_argument(cmnd, arg);
                        cmnd = one_argument(cmnd, arg);
                        if (arg[0] == '\0')
                                mpsleep->timer = 4;
                        else
                                mpsleep->timer = atoi(arg);*/
                        /*
                         * Time to sleep 
                         */
                        cmnd = one_argument(cmnd, arg);
                        if (cmnd[0] == '\0')
                                mpsleep->timer = 4;
                        else
                                mpsleep->timer = atoi(cmnd);

                        if (mpsleep->timer < 1)
                        {
                                progbug("mpsleep - bad arg, using default",
                                        mob);
                                mpsleep->timer = 4;
                        }

                        /*
                         * Save type of prog, room, object or mob 
                         */
                        if (mpsleep->mob->pIndexData->vnum == 3)
                        {
                                if (!str_prefix
                                    ("Room", mpsleep->mob->description))
                                {
                                        mpsleep->type = MpRoom;
                                        mpsleep->room = mpsleep->mob->in_room;
                                }
                                else if (!str_prefix
                                         ("Object",
                                          mpsleep->mob->description))
                                        mpsleep->type = MpObj;
                        }
                        else
                                mpsleep->type = MpMob;

                        LINK(mpsleep, first_mpsleep, last_mpsleep, next,
                             prev);

                        --prog_nest;
                        return;
                        /*
                         * }
                         */
                }


                /*
                 * Evaluate/execute the command, check what happened. 
                 */
                result = mprog_do_command(cmnd, mob, actor, obj, vo, rndm,
                                          (ifstate[iflevel][InIf]
                                           && !ifstate[iflevel][DoIf])
                                          || (ifstate[iflevel][InElse]
                                              && !ifstate[iflevel][DoElse]),
                                          (ignorelevel > 0));

                /*
                 * Script prog support  -Thoric 
                 */
                if (single_step)
                {
                        mob->mpscriptpos = command_list - tmpcmndlst;
                        --prog_nest;
                        return;
                }

                /*
                 * This is the complicated part.  Act on the returned value from
                 * mprog_do_command according to the current logic state. 
                 */
                switch (result)
                {
                case COMMANDOK:
/*
#ifdef DEBUG
log_string( "COMMANDOK" );
#endif
*/
                        /*
                         * Ok, this one's a no-brainer. 
                         */
                        continue;
                        break;

                case IFTRUE:
/*
#ifdef DEBUG
log_string( "IFTRUE" );
#endif
*/
                        /*
                         * An if was evaluated and found true.  Note that we are in an
                         * if section and that we want to execute it. 
                         */
                        iflevel++;
                        if (iflevel == MaxIfs)
                        {
                                progbug("Maximum nested ifs exceeded", mob);
                                --prog_nest;
                                return;
                        }

                        ifstate[iflevel][InIf] = TRUE;
                        ifstate[iflevel][DoIf] = TRUE;
                        break;

                case IFFALSE:
/*
#ifdef DEBUG
log_string( "IFFALSE" );
#endif
*/
                        /*
                         * An if was evaluated and found false.  Note that we are in an
                         * if section and that we don't want to execute it unless we find
                         * an or that evaluates to true. 
                         */
                        iflevel++;
                        if (iflevel == MaxIfs)
                        {
                                progbug("Maximum nested ifs exceeded", mob);
                                --prog_nest;
                                return;
                        }
                        ifstate[iflevel][InIf] = TRUE;
                        ifstate[iflevel][DoIf] = FALSE;
                        break;

                case ORTRUE:
/*
#ifdef DEBUG
log_string( "ORTRUE" );
#endif
*/
                        /*
                         * An or was evaluated and found true.  We should already be in an
                         * if section, so note that we want to execute it. 
                         */
                        if (!ifstate[iflevel][InIf])
                        {
                                progbug("Unmatched or", mob);
                                --prog_nest;
                                return;
                        }
                        ifstate[iflevel][DoIf] = TRUE;
                        break;

                case ORFALSE:
/*
#ifdef DEBUG
log_string( "ORFALSE" );
#endif*/
                        /*
                         * An or was evaluated and found false.  We should already be in an
                         * if section, and we don't need to do much.  If the if was true or
                         * there were/will be other ors that evaluate(d) to true, they'll set
                         * do_if to true. 
                         */
                        if (!ifstate[iflevel][InIf])
                        {
                                progbug("Unmatched or", mob);
                                --prog_nest;
                                return;
                        }
                        continue;
                        break;

                case FOUNDELSE:
/*
#ifdef DEBUG
log_string( "FOUNDELSE" );
#endif*/
                        /*
                         * Found an else.  Make sure we're in an if section, bug out if not.
                         * If this else is not one that we wish to ignore, note that we're now 
                         * in an else section, and look at whether or not we executed the if 
                         * section to decide whether to execute the else section.  Ca marche 
                         * bien. 
                         */
                        if (ignorelevel > 0)
                                continue;

                        if (ifstate[iflevel][InElse])
                        {
                                progbug("Found else in an else section", mob);
                                --prog_nest;
                                return;
                        }
                        if (!ifstate[iflevel][InIf])
                        {
                                progbug("Unmatched else", mob);
                                --prog_nest;
                                return;
                        }

                        ifstate[iflevel][InElse] = TRUE;
                        ifstate[iflevel][DoElse] = !ifstate[iflevel][DoIf];
                        ifstate[iflevel][InIf] = FALSE;
                        ifstate[iflevel][DoIf] = FALSE;

                        break;

                case FOUNDENDIF:
/*
#ifdef DEBUG
log_string( "FOUNDENDIF" );
#endif*/
                        /*
                         * Hmm, let's see... FOUNDENDIF must mean that we found an endif.
                         * So let's make sure we were expecting one, return if not.  If this
                         * endif matches the if or else that we're executing, note that we are 
                         * now no longer executing an if.  If not, keep track of what we're 
                         * ignoring. 
                         */
                        if (!
                            (ifstate[iflevel][InIf]
                             || ifstate[iflevel][InElse]))
                        {
                                progbug("Unmatched endif", mob);
                                --prog_nest;
                                return;
                        }

                        if (ignorelevel > 0)
                        {
                                ignorelevel--;
                                continue;
                        }

                        ifstate[iflevel][InIf] = FALSE;
                        ifstate[iflevel][DoIf] = FALSE;
                        ifstate[iflevel][InElse] = FALSE;
                        ifstate[iflevel][DoElse] = FALSE;

                        iflevel--;
                        break;

                case IFIGNORED:
/*
#ifdef DEBUG
log_string( "IFIGNORED" );
#endif*/
                        if (!
                            (ifstate[iflevel][InIf]
                             || ifstate[iflevel][InElse]))
                        {
                                progbug("Parse error, ignoring if while not in if or else", mob);
                                --prog_nest;
                                return;
                        }
                        ignorelevel++;
                        break;

                case ORIGNORED:
/*
#ifdef DEBUG
log_string( "ORIGNORED" );
#endif*/
                        if (!
                            (ifstate[iflevel][InIf]
                             || ifstate[iflevel][InElse]))
                        {
                                progbug("Unmatched or", mob);
                                --prog_nest;
                                return;
                        }
                        if (ignorelevel == 0)
                        {
                                progbug("Parse error, mistakenly ignoring or",
                                        mob);
                                --prog_nest;
                                return;
                        }

                        break;

                case BERR:
/*
#ifdef DEBUG
log_string( "BERR" );
#endif*/
                        --prog_nest;
                        return;
                        break;
                }
        }
        --prog_nest;
        return;
}

/* This function replaces mprog_process_cmnd.  It is called from 
 * mprog_driver, once for each line in a mud prog.  This function
 * checks what the line is, executes if/or checks and calls interpret
 * to perform the the commands.  Written by Narn, Dec 95.
 */
int mprog_do_command(char *cmnd, CharData * mob, CharData * actor,
                     ObjData * obj, void *vo, CharData * rndm,
                     bool ignore, bool ignore_ors)
{
        char      firstword[MaxInputLength];
        char     *ifcheck;
        char      buf[MaxInputLength];
        char      tmp[MaxInputLength];
        char     *point, *str, *i;
        int       validif, vnum;

        /*
         * Isolate the first word of the line, it gives us a clue what
         * we want to do. 
         */
        ifcheck = one_argument(cmnd, firstword);

        if (!str_cmp(firstword, "if"))
        {
                /*
                 * Ok, we found an if.  According to the boolean 'ignore', either
                 * ignore the ifcheck and report that back to mprog_driver or do
                 * the ifcheck and report whether it was successful. 
                 */
                if (ignore)
                        return IFIGNORED;
                else
                        validif =
                                mprog_do_ifcheck(ifcheck, mob, actor, obj, vo,
                                                 rndm);

                if (validif == 1)
                        return IFTRUE;

                if (validif == 0)
                        return IFFALSE;
                return BERR;
        }

        if (!str_cmp(firstword, "or"))
        {
                /*
                 * Same behavior as with ifs, but use the boolean 'ignore_ors' to
                 * decide which way to go. 
                 */
                if (ignore_ors)
                        return ORIGNORED;
                else
                        validif =
                                mprog_do_ifcheck(ifcheck, mob, actor, obj, vo,
                                                 rndm);

                if (validif == 1)
                        return ORTRUE;

                if (validif == 0)
                        return ORFALSE;

                return BERR;
        }

        /*
         * For else and endif, just report back what we found.  Mprog_driver
         * keeps track of logiclevels. 
         */
        if (!str_cmp(firstword, "else"))
        {
                return FOUNDELSE;
        }

        if (!str_cmp(firstword, "endif"))
        {
                return FOUNDENDIF;
        }

        /*
         * Ok, didn't find an if, an or, an else or an endif.  
         * If the command is in an if or else section that is not to be 
         * performed, the boolean 'ignore' is set to true and we just 
         * return.  If not, we try to execute the command. 
         */

        if (ignore)
                return COMMANDOK;

        /*
         * If the command is 'break', that's all folks. 
         */
        if (!str_cmp(firstword, "break"))
                return BERR;

        vnum = mob->pIndexData->vnum;
        point = buf;
        str = cmnd;

        /*
         * This chunk of code taken from mprog_process_cmnd. 
         */
        while (*str != '\0')
        {
                if (*str != '$')
                {
                        *point++ = *str++;
                        continue;
                }
                str++;
                mprog_translate(*str, tmp, mob, actor, obj, vo, rndm);
                i = tmp;
                ++str;
                while ((*point = *i) != '\0')
                        ++point, ++i;
        }
        *point = '\0';

        interpret(mob, buf);

        /*
         * If the mob is mentally unstable and does things like fireball
         * itself, let's make sure it's still alive. 
         */
        if (char_died(mob))
        {
                return BERR;
        }

        return COMMANDOK;
}

/***************************************************************************
 * Global function code and brief comments.
 */


/* See if there's any mud programs waiting to be continued -rkb */
void mpsleep_update()
{
        MpsleepData *mpsleep;
        MpsleepData *tmpMpsleep;
        bool      delete_it;

        mpsleep = first_mpsleep;
        while (mpsleep)
        {
                delete_it = FALSE;

                if (mpsleep->mob)
                        delete_it = char_died(mpsleep->mob);

                if (mpsleep->actor && !delete_it)
                        delete_it = char_died(mpsleep->actor);

                if (mpsleep->obj && !delete_it)
                        delete_it = obj_extracted(mpsleep->obj);

                if (delete_it)
                {
                        log_string("mpsleep_update - Deleting expired prog.");

                        tmpMpsleep = mpsleep;
                        mpsleep = mpsleep->next;
                        STRFREE(tmpMpsleep->com_list);
                        UNLINK(tmpMpsleep, first_mpsleep, last_mpsleep, next,
                               prev);
                        DISPOSE(tmpMpsleep);

                        continue;
                }

                mpsleep = mpsleep->next;
        }

        mpsleep = first_mpsleep;
        while (mpsleep) /* Find progs to continue */
        {
                if (--mpsleep->timer <= 0)
                {
                        current_mpsleep = mpsleep;

                        if (mpsleep->type == MpRoom)
                                rset_supermob(mpsleep->room);
                        else if (mpsleep->type == MpObj)
                                set_supermob(mpsleep->obj);

                        mprog_driver(mpsleep->com_list, mpsleep->mob,
                                     mpsleep->actor, mpsleep->obj,
                                     mpsleep->vo, mpsleep->single_step);

                        release_supermob();

                        tmpMpsleep = mpsleep;
                        mpsleep = mpsleep->next;
                        STRFREE(tmpMpsleep->com_list);
                        UNLINK(tmpMpsleep, first_mpsleep, last_mpsleep, next,
                               prev);
                        DISPOSE(tmpMpsleep);

                        continue;
                }

                mpsleep = mpsleep->next;
        }
}


bool mprog_keyword_check(const char *argu, const char *argl)
{
        char      word[MaxInputLength];
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        unsigned int i;
        char     *arg, *arglist;
        char     *start, *end;

        mudstrlcpy(arg1, strlower(argu), MIL);
        arg = arg1;
        mudstrlcpy(arg2, strlower(argl), MIL);
        arglist = arg2;

        for (i = 0; i < strlen(arglist); i++)
                arglist[i] = LOWER(arglist[i]);
        for (i = 0; i < strlen(arg); i++)
                arg[i] = LOWER(arg[i]);
        if ((arglist[0] == 'p') && (arglist[1] == ' '))
        {
                arglist += 2;
                while ((start = strstr(arg, arglist)))
                        if ((start == arg || *(start - 1) == ' ')
                            && (*(end = start + strlen(arglist)) == ' '
                                || *end == '\n'
                                || *end == '\r' || *end == '\0'))
                                return TRUE;
                        else
                                arg = start + 1;
        }
        else
        {
                arglist = one_argument(arglist, word);
                for (; word[0] != '\0'; arglist = one_argument(arglist, word))
                        while ((start = strstr(arg, word)))
                                if ((start == arg || *(start - 1) == ' ')
                                    && (*(end = start + strlen(word)) == ' '
                                        || *end == '\n'
                                        || *end == '\r' || *end == '\0'))
                                        return TRUE;
                                else
                                        arg = start + 1;
        }
/*    bug( "don't match" ); */
        return FALSE;
}


/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check(char *arg, CharData * mob, CharData * actor,
                          ObjData * obj, void *vo, int type)
{

        char      temp1[MaxStringLength];
        char      temp2[MaxInputLength];
        char      word[MaxInputLength];
        MProgData *mprg;
        char     *list;
        char     *start;
        char     *dupl;
        char     *end;
        unsigned int i;

        for (mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next)
                if (mprg->type & type)
                {
                        mudstrlcpy(temp1, mprg->arglist, MSL);
                        list = temp1;
                        for (i = 0; i < strlen(list); i++)
                                list[i] = LOWER(list[i]);
                        mudstrlcpy(temp2, arg, MIL);
                        dupl = temp2;
                        for (i = 0; i < strlen(dupl); i++)
                                dupl[i] = LOWER(dupl[i]);
                        if ((list[0] == 'p') && (list[1] == ' '))
                        {
                                list += 2;
                                while ((start = strstr(dupl, list)))
                                        if ((start == dupl
                                             || *(start - 1) == ' ')
                                            && (*(end = start + strlen(list))
                                                == ' ' || *end == '\n'
                                                || *end == '\r'
                                                || *end == '\0'))
                                        {
                                                mprog_driver(mprg->comlist,
                                                             mob, actor, obj,
                                                             vo, FALSE);
                                                break;
                                        }
                                        else
                                                dupl = start + 1;
                        }
                        else
                        {
                                list = one_argument(list, word);
                                for (; word[0] != '\0';
                                     list = one_argument(list, word))
                                        while ((start = strstr(dupl, word)))
                                                if ((start == dupl
                                                     || *(start - 1) == ' ')
                                                    &&
                                                    (*
                                                     (end =
                                                      start + strlen(word)) ==
                                                     ' ' || *end == '\n'
                                                     || *end == '\r'
                                                     || *end == '\0'))
                                                {
                                                        mprog_driver(mprg->
                                                                     comlist,
                                                                     mob,
                                                                     actor,
                                                                     obj, vo,
                                                                     FALSE);
                                                        break;
                                                }
                                                else
                                                        dupl = start + 1;
                        }
                }

        return;

}

void mprog_percent_check(CharData * mob, CharData * actor, ObjData * obj,
                         void *vo, int type)
{
        MProgData *mprg;

        for (mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next)
                if ((mprg->type & type)
                    && (number_percent() <= atoi(mprg->arglist)))
                {
                        mprog_driver(mprg->comlist, mob, actor, obj, vo,
                                     FALSE);
                        if (type != GreetProg && type != AllGreetProg)
                                break;
                }

        return;

}

void mprog_time_check(CharData * mob, CharData * actor, ObjData * obj,
                      void *vo, int type)
{
        MProgData *mprg;
        bool      trigger_time;

        for (mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next)
        {
                trigger_time = (time_info.hour == atoi(mprg->arglist));

                if (!trigger_time)
                {
                        if (mprg->triggered)
                                mprg->triggered = FALSE;
                        continue;
                }

                if ((mprg->type & type)
                    && ((!mprg->triggered) || (mprg->type && HourProg)))
                {
                        mprg->triggered = TRUE;
                        mprog_driver(mprg->comlist, mob, actor, obj, vo,
                                     FALSE);
                }
        }
        return;
}


void mob_act_add(CharData * mob)
{
        struct act_prog_data *runner;

        for (runner = mob_act_list; runner; runner = runner->next)
                if (runner->vo == mob)
                        return;
        CREATE(runner, struct act_prog_data, 1);
        runner->vo = mob;
        runner->next = mob_act_list;
        mob_act_list = runner;
}


/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger(char *buf, CharData * mob, CharData * ch,
                       ObjData * obj, void *vo)
{
        MProgActList *tmp_act;
        MProgData *mprg;
        bool      found = FALSE;

        if (IsNpc(mob) && IsSet(mob->pIndexData->progtypes, ActProg))
        {
                /*
                 * Don't let a mob trigger itself, nor one instance of a mob
                 * trigger another instance. 
                 */
                if (IsNpc(ch) && ch->pIndexData == mob->pIndexData)
                        return;

                /*
                 * make sure this is a matching trigger 
                 */
                for (mprg = mob->pIndexData->mudprogs; mprg;
                     mprg = mprg->next)
                        if (mprg->type & ActProg
                            && mprog_keyword_check(buf, mprg->arglist))
                        {
                                found = TRUE;
                                break;
                        }
                if (!found)
                        return;

                CREATE(tmp_act, MProgActList, 1);
                if (mob->mpactnum > 0)
                        tmp_act->next = mob->mpact;
                else
                        tmp_act->next = NULL;

                mob->mpact = tmp_act;
                mob->mpact->buf = str_dup(buf);
                mob->mpact->ch = ch;
                mob->mpact->obj = obj;
                mob->mpact->vo = vo;
                mob->mpactnum++;
                mob_act_add(mob);
        }
        return;
}

void mprog_bribe_trigger(CharData * mob, CharData * ch, int amount)
{

        char      buf[MaxStringLength];
        MProgData *mprg;
        ObjData *obj;

        if (IsNpc(mob) && (mob->pIndexData->progtypes & BribeProg))
        {
                /*
                 * Don't let a mob trigger itself, nor one instance of a mob
                 * trigger another instance. 
                 */
                if (IsNpc(ch) && ch->pIndexData == mob->pIndexData)
                        return;

                obj = create_object(get_obj_index(ObjVnumMoneySome), 0);
                snprintf(buf, MSL, obj->short_descr, amount);
                STRFREE(obj->short_descr);
                obj->short_descr = STRALLOC(buf);
                obj->value[0] = amount;
                obj = obj_to_char(obj, mob);
                mob->gold -= amount;

                for (mprg = mob->pIndexData->mudprogs; mprg;
                     mprg = mprg->next)
                        if ((mprg->type & BribeProg)
                            && (amount >= atoi(mprg->arglist)))
                        {
                                mprog_driver(mprg->comlist, mob, ch, obj,
                                             NULL, FALSE);
                                break;
                        }
        }

        return;

}

void mprog_death_trigger(CharData * killer, CharData * mob)
{
        if (IsNpc(mob) && killer != mob
            && (mob->pIndexData->progtypes & DeathProg))
        {
                mprog_percent_check(mob, killer, NULL, NULL, DeathProg);
        }
        death_cry(mob);
        return;
}

void mprog_entry_trigger(CharData * mob)
{

        if (IsNpc(mob) && (mob->pIndexData->progtypes & EntryProg))
                mprog_percent_check(mob, NULL, NULL, NULL, EntryProg);
        return;

}

void mprog_fight_trigger(CharData * mob, CharData * ch)
{

        if (IsNpc(mob) && (mob->pIndexData->progtypes & FightProg))
                mprog_percent_check(mob, ch, NULL, NULL, FightProg);

        return;

}

void mprog_give_trigger(CharData * mob, CharData * ch, ObjData * obj)
{
        char      vnum[16];
        char      buf[MaxStringLength];
        MProgData *mprg;

        snprintf(vnum, MSL, "%d", obj->pIndexData->vnum);
        if (IsNpc(mob) && (mob->pIndexData->progtypes & GiveProg))
        {
                /*
                 * Don't let a mob trigger itself, nor one instance of a mob
                 * * trigger another instance. 
                 */
                if (IsNpc(ch) && ch->pIndexData == mob->pIndexData)
                        return;

                for (mprg = mob->pIndexData->mudprogs; mprg;
                     mprg = mprg->next)
                {
                        if (!(mprg->type & GiveProg))
                                continue;

                        one_argument(mprg->arglist, buf);

                        if (!str_cmp("all", buf)
                            || nifty_is_name(vnum, mprg->arglist)
                            || !str_cmp(obj->name, mprg->arglist)
                            || nifty_is_name(mprg->arglist, obj->name))
                        {
                                mprog_driver(mprg->comlist, mob, ch, obj,
                                             NULL, FALSE);
                                break;
                        }
                }
        }
        return;
}

void mprog_greet_trigger(CharData * ch)
{
        CharData *vmob, *vmob_next;

/*
#ifdef DEBUG
 char buf[MaxStringLength];
 snprintf( buf, MSL, "mprog_greet_trigger -> %s", ch->name );
 log_string( buf );
#endif
*/
        for (vmob = ch->in_room->first_person; vmob; vmob = vmob_next)
        {
                vmob_next = vmob->next_in_room;
                if (!IsNpc(vmob) || vmob->fighting || !IsAwake(vmob))
                        continue;

                /*
                 * Don't let a mob trigger itself, nor one instance of a mob
                 * trigger another instance. 
                 */
                if (IsNpc(ch) && ch->pIndexData == vmob->pIndexData)
                        continue;

                if (vmob->pIndexData->progtypes & GreetProg)
                        mprog_percent_check(vmob, ch, NULL, NULL, GreetProg);
                else if (vmob->pIndexData->progtypes & AllGreetProg)
                        mprog_percent_check(vmob, ch, NULL, NULL,
                                            AllGreetProg);
        }
        return;

}

void mprog_hitprcnt_trigger(CharData * mob, CharData * ch)
{

        MProgData *mprg;

        if (IsNpc(mob) && (mob->pIndexData->progtypes & HitprcntProg))
                for (mprg = mob->pIndexData->mudprogs; mprg;
                     mprg = mprg->next)
                        if ((mprg->type & HitprcntProg)
                            && ((100 * mob->hit / mob->max_hit) <
                                atoi(mprg->arglist)))
                        {
                                mprog_driver(mprg->comlist, mob, ch, NULL,
                                             NULL, FALSE);
                                break;
                        }

        return;

}

void mprog_random_trigger(CharData * mob)
{
        if (mob->pIndexData->progtypes & RandProg)
                mprog_percent_check(mob, NULL, NULL, NULL, RandProg);

        return;
}

void mprog_time_trigger(CharData * mob)
{
        if (mob->pIndexData->progtypes & TimeProg)
                mprog_time_check(mob, NULL, NULL, NULL, TimeProg);
        return;
}

void mprog_hour_trigger(CharData * mob)
{
        if (mob->pIndexData->progtypes & HourProg)
                mprog_time_check(mob, NULL, NULL, NULL, HourProg);
        return;
}

void mprog_speech_trigger(char *txt, CharData * actor)
{

        CharData *vmob;

        for (vmob = actor->in_room->first_person; vmob;
             vmob = vmob->next_in_room)
        {
                if (IsNpc(vmob)
                    && (vmob->pIndexData->progtypes & SpeechProg))
                {
                        if (IsNpc(actor)
                            && actor->pIndexData == vmob->pIndexData)
                                continue;
                        mprog_wordlist_check(txt, vmob, actor, NULL, NULL,
                                             SpeechProg);
                }
        }
        return;

}

void mprog_script_trigger(CharData * mob)
{
        MProgData *mprg;

        if (mob->pIndexData->progtypes & ScriptProg)
                for (mprg = mob->pIndexData->mudprogs; mprg;
                     mprg = mprg->next)
                        if ((mprg->type & ScriptProg))
                        {
                                if (mprg->arglist[0] == '\0'
                                    || mob->mpscriptpos != 0
                                    || atoi(mprg->arglist) == time_info.hour)
                                        mprog_driver(mprg->comlist, mob, NULL,
                                                     NULL, NULL, TRUE);
                        }
        return;
}

void oprog_script_trigger(ObjData * obj)
{
        MProgData *mprg;

        if (obj->pIndexData->progtypes & ScriptProg)
                for (mprg = obj->pIndexData->mudprogs; mprg;
                     mprg = mprg->next)
                        if ((mprg->type & ScriptProg))
                        {
                                if (mprg->arglist[0] == '\0'
                                    || obj->mpscriptpos != 0
                                    || atoi(mprg->arglist) == time_info.hour)
                                {
                                        set_supermob(obj);
                                        mprog_driver(mprg->comlist, supermob,
                                                     NULL, NULL, NULL, TRUE);
                                        obj->mpscriptpos =
                                                supermob->mpscriptpos;
                                        release_supermob();
                                }
                        }
        return;
}

void rprog_script_trigger(RoomIndexData * room)
{
        MProgData *mprg;

        if (room->progtypes & ScriptProg)
                for (mprg = room->mudprogs; mprg; mprg = mprg->next)
                        if ((mprg->type & ScriptProg))
                        {
                                if (mprg->arglist[0] == '\0'
                                    || room->mpscriptpos != 0
                                    || atoi(mprg->arglist) == time_info.hour)
                                {
                                        rset_supermob(room);
                                        mprog_driver(mprg->comlist, supermob,
                                                     NULL, NULL, NULL, TRUE);
                                        room->mpscriptpos =
                                                supermob->mpscriptpos;
                                        release_supermob();
                                }
                        }
        return;
}


/*
 *  Mudprogram additions begin here
 */
void set_supermob(ObjData * obj)
{
        RoomIndexData *room;
        ObjData *in_obj;
        CharData *mob;
        char      buf[200];

        if (!supermob)
                supermob = create_mobile(get_mob_index(3));

        mob = supermob; /* debugging */

        if (!obj)
                return;

        for (in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj)
                ;

        if (in_obj->carried_by)
        {
                room = in_obj->carried_by->in_room;
        }
        else
        {
                room = obj->in_room;
        }

        if (!room)
                return;

        if (supermob->short_descr)
                STRFREE(supermob->short_descr);

        supermob->short_descr = QUICKLINK(obj->short_descr);
        supermob->mpscriptpos = obj->mpscriptpos;

        /*
         * Added by Jenny to allow bug messages to show the vnum
         * of the object, and not just supermob's vnum 
         */
        snprintf(buf, MSL, "Object #%d", obj->pIndexData->vnum);
        STRFREE(supermob->description);
        supermob->description = STRALLOC(buf);

        if (room != NULL)
        {
                char_from_room(supermob);
                char_to_room(supermob, room);
        }
}

void release_supermob()
{
        char_from_room(supermob);
        char_to_room(supermob, get_room_index(3));
}


bool oprog_percent_check(CharData * mob, CharData * actor, ObjData * obj,
                         void *vo, int type)
{
        MProgData *mprg;
        bool      executed = FALSE;

        for (mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next)
                if ((mprg->type & type)
                    && (number_percent() <= atoi(mprg->arglist)))
                {
                        executed = TRUE;
                        mprog_driver(mprg->comlist, mob, actor, obj, vo,
                                     FALSE);
                        if (type != GreetProg)
                                break;
                }

        return executed;

}

/*
 * Triggers follow
 */


/*
 *  Hold on this
 *
void oprog_act_trigger( CharData *ch, ObjData *obj )
{
   set_supermob( obj );
   if ( obj->pIndexData->progtypes & ActProg ) 
     oprog_percent_check( supermob, ch, obj, NULL, ActProg );

 release_supermob();
 return;
}
 *
 *
 */

void oprog_greet_trigger(CharData * ch)
{
        ObjData *vobj;

        for (vobj = ch->in_room->first_content; vobj;
             vobj = vobj->next_content)
                if (vobj->pIndexData->progtypes & GreetProg)
                {
                        set_supermob(vobj); /* not very efficient to do here */
                        oprog_percent_check(supermob, ch, vobj, NULL,
                                            GreetProg);
                        release_supermob();
                }

        return;
}

void oprog_speech_trigger(char *txt, CharData * ch)
{
        ObjData *vobj;

        /*
         * supermob is set and released in oprog_wordlist_check 
         */
        for (vobj = ch->in_room->first_content; vobj;
             vobj = vobj->next_content)
                if (vobj->pIndexData->progtypes & SpeechProg)
                {
                        oprog_wordlist_check(txt, supermob, ch, vobj, NULL,
                                             SpeechProg, vobj);
                }

        return;
}

/*
 * Called at top of obj_update
 * make sure to put an if(!obj) continue
 * after it
 */
void oprog_random_trigger(ObjData * obj)
{
        if (!obj || !obj->pIndexData)
                return;

        if (obj->pIndexData->progtypes & RandProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, NULL, obj, NULL, RandProg);
                release_supermob();
        }
        return;
}

/*
 * in wear_obj, between each successful equip_char 
 * the subsequent return
 */
void oprog_wear_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & WearProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, WearProg);
                release_supermob();
        }
        return;
}

bool oprog_use_trigger(CharData * ch, ObjData * obj, CharData * vict,
                       ObjData * targ, void *vo)
{
        bool      executed = FALSE;

        vo = NULL;

        if (obj->pIndexData->progtypes & UseProg)
        {
                set_supermob(obj);
                if (obj->item_type == ItemStaff)
                {
                        if (vict)
                                executed =
                                        oprog_percent_check(supermob, ch, obj,
                                                            vict, UseProg);
                        else
                                executed =
                                        oprog_percent_check(supermob, ch, obj,
                                                            targ, UseProg);
                }
                else
                {
                        executed =
                                oprog_percent_check(supermob, ch, obj, NULL,
                                                    UseProg);
                }
                release_supermob();
        }
        return executed;
}

/*
 * call in remove_obj, right after unequip_char   
 * do a if(!ch) return right after, and return TRUE (?)
 * if !ch
 */
void oprog_remove_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & RemoveProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, RemoveProg);
                release_supermob();
        }
        return;
}


/*
 * call in do_sac, right before extract_obj
 */
void oprog_sac_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & SacProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, SacProg);
                release_supermob();
        }
        return;
}

/*
 * call in do_get, right before check_for_trap
 * do a if(!ch) return right after
 */
void oprog_get_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & GetProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, GetProg);
                release_supermob();
        }
        return;
}

/*
 * called in damage_obj in act_obj.c
 */
void oprog_damage_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & DamageProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, DamageProg);
                release_supermob();
        }
        return;
}

/*
 * called in do_repair in shops.c
 */
void oprog_repair_trigger(CharData * ch, ObjData * obj)
{

        if (obj->pIndexData->progtypes & RepairProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, RepairProg);
                release_supermob();
        }
        return;
}

/*
 * call twice in do_drop, right after the act( AtAction,...)
 * do a if(!ch) return right after
 */
void oprog_drop_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & DropProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, DropProg);
                release_supermob();
        }
        return;
}

/*
 * call towards end of do_examine, right before check_for_trap
 */
void oprog_examine_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & ExaProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, ExaProg);
                release_supermob();
        }
        return;
}


/*
 * call in fight.c, group_gain, after (?) the obj_to_room
 */
void oprog_zap_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & ZapProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, ZapProg);
                release_supermob();
        }
        return;
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_pull_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & PullProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, PullProg);
                release_supermob();
        }
        return;
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_push_trigger(CharData * ch, ObjData * obj)
{
        if (obj->pIndexData->progtypes & PushProg)
        {
                set_supermob(obj);
                oprog_percent_check(supermob, ch, obj, NULL, PushProg);
                release_supermob();
        }
        return;
}

void      obj_act_add(ObjData * obj);
void oprog_act_trigger(char *buf, ObjData * mobj, CharData * ch,
                       ObjData * obj, void *vo)
{
        if (mobj->pIndexData->progtypes & ActProg)
        {
                MProgActList *tmp_act;

                CREATE(tmp_act, MProgActList, 1);
                if (mobj->mpactnum > 0)
                        tmp_act->next = mobj->mpact;
                else
                        tmp_act->next = NULL;

                mobj->mpact = tmp_act;
                mobj->mpact->buf = str_dup(buf);
                mobj->mpact->ch = ch;
                mobj->mpact->obj = obj;
                mobj->mpact->vo = vo;
                mobj->mpactnum++;
                obj_act_add(mobj);
        }
        return;
}

void oprog_wordlist_check(char *arg, CharData * mob, CharData * actor,
                          ObjData * obj, void *vo, int type, ObjData * iobj)
{

        char      temp1[MaxStringLength];
        char      temp2[MaxInputLength];
        char      word[MaxInputLength];
        MProgData *mprg;
        char     *list;
        char     *start;
        char     *dupl;
        char     *end;
        unsigned int i;

        for (mprg = iobj->pIndexData->mudprogs; mprg; mprg = mprg->next)
                if (mprg->type & type)
                {
                        mudstrlcpy(temp1, mprg->arglist, MSL);
                        list = temp1;
                        for (i = 0; i < strlen(list); i++)
                                list[i] = LOWER(list[i]);
                        mudstrlcpy(temp2, arg, MIL);
                        dupl = temp2;
                        for (i = 0; i < strlen(dupl); i++)
                                dupl[i] = LOWER(dupl[i]);
                        if ((list[0] == 'p') && (list[1] == ' '))
                        {
                                list += 2;
                                while ((start = strstr(dupl, list)))
                                        if ((start == dupl
                                             || *(start - 1) == ' ')
                                            && (*(end = start + strlen(list))
                                                == ' ' || *end == '\n'
                                                || *end == '\r'
                                                || *end == '\0'))
                                        {
                                                set_supermob(iobj);
                                                mprog_driver(mprg->comlist,
                                                             mob, actor, obj,
                                                             vo, FALSE);
                                                release_supermob();
                                                break;
                                        }
                                        else
                                                dupl = start + 1;
                        }
                        else
                        {
                                list = one_argument(list, word);
                                for (; word[0] != '\0';
                                     list = one_argument(list, word))
                                        while ((start = strstr(dupl, word)))
                                                if ((start == dupl
                                                     || *(start - 1) == ' ')
                                                    &&
                                                    (*
                                                     (end =
                                                      start + strlen(word)) ==
                                                     ' ' || *end == '\n'
                                                     || *end == '\r'
                                                     || *end == '\0'))
                                                {
                                                        set_supermob(iobj);
                                                        mprog_driver(mprg->
                                                                     comlist,
                                                                     mob,
                                                                     actor,
                                                                     obj, vo,
                                                                     FALSE);
                                                        release_supermob();
                                                        break;
                                                }
                                                else
                                                        dupl = start + 1;
                        }
                }

        return;
}



/*
 *  room_prog support starts here
 *
 *
 */

void rset_supermob(RoomIndexData * room)
{
        char      buf[200];

        if (room)
        {
                STRFREE(supermob->short_descr);
                supermob->short_descr = QUICKLINK(room->name);
                STRFREE(supermob->name);
                supermob->name = QUICKLINK(room->name);

                supermob->mpscriptpos = room->mpscriptpos;

                /*
                 * Added by Jenny to allow bug messages to show the vnum
                 * of the room, and not just supermob's vnum 
                 */
                snprintf(buf, MSL, "Room #%d", room->vnum);
                STRFREE(supermob->description);
                supermob->description = STRALLOC(buf);

                char_from_room(supermob);
                char_to_room(supermob, room);

        }
}


void rprog_percent_check(CharData * mob, CharData * actor, ObjData * obj,
                         void *vo, int type)
{
        MProgData *mprg;

        if (!mob->in_room)
                return;

        for (mprg = mob->in_room->mudprogs; mprg; mprg = mprg->next)
                if ((mprg->type & type)
                    && (number_percent() <= atoi(mprg->arglist)))
                {
                        mprog_driver(mprg->comlist, mob, actor, obj, vo,
                                     FALSE);
                        if (type != EnterProg)
                                break;
                }

        return;
}

/*
 * Triggers follow
 */


/*
 *  Hold on this
 * Unhold. -- Alty
 */
void      room_act_add(RoomIndexData * room);
void rprog_act_trigger(char *buf, RoomIndexData * room, CharData * ch,
                       ObjData * obj, void *vo)
{
        if (room->progtypes & ActProg)
        {
                MProgActList *tmp_act;

                CREATE(tmp_act, MProgActList, 1);
                if (room->mpactnum > 0)
                        tmp_act->next = room->mpact;
                else
                        tmp_act->next = NULL;

                room->mpact = tmp_act;
                room->mpact->buf = str_dup(buf);
                room->mpact->ch = ch;
                room->mpact->obj = obj;
                room->mpact->vo = vo;
                room->mpactnum++;
                room_act_add(room);
        }
        return;
}

/*
 *
 */


void rprog_leave_trigger(CharData * ch)
{
        if (ch->in_room->progtypes & LeaveProg)
        {
                rset_supermob(ch->in_room);
                rprog_percent_check(supermob, ch, NULL, NULL, LeaveProg);
                release_supermob();
        }
        return;
}

void rprog_enter_trigger(CharData * ch)
{
        if (ch->in_room->progtypes & EnterProg)
        {
                rset_supermob(ch->in_room);
                rprog_percent_check(supermob, ch, NULL, NULL, EnterProg);
                release_supermob();
        }
        return;
}

void rprog_sleep_trigger(CharData * ch)
{
        if (ch->in_room->progtypes & SleepProg)
        {
                rset_supermob(ch->in_room);
                rprog_percent_check(supermob, ch, NULL, NULL, SleepProg);
                release_supermob();
        }
        return;
}

void rprog_rest_trigger(CharData * ch)
{
        if (ch->in_room->progtypes & RestProg)
        {
                rset_supermob(ch->in_room);
                rprog_percent_check(supermob, ch, NULL, NULL, RestProg);
                release_supermob();
        }
        return;
}

void rprog_rfight_trigger(CharData * ch)
{
        if (ch->in_room->progtypes & RfightProg)
        {
                rset_supermob(ch->in_room);
                rprog_percent_check(supermob, ch, NULL, NULL, RfightProg);
                release_supermob();
        }
        return;
}

void rprog_death_trigger(CharData * killer, CharData * ch)
{
        killer = NULL;
        if (ch->in_room->progtypes & RdeathProg)
        {
                rset_supermob(ch->in_room);
                rprog_percent_check(supermob, ch, NULL, NULL, RdeathProg);
                release_supermob();
        }
        return;
}

void rprog_speech_trigger(char *txt, CharData * ch)
{
        if (ch->in_room->progtypes & SpeechProg)
        {
                /*
                 * supermob is set and released in rprog_wordlist_check 
                 */
                rprog_wordlist_check(txt, supermob, ch, NULL, NULL,
                                     SpeechProg, ch->in_room);
        }
        return;
}

void rprog_random_trigger(CharData * ch)
{

        if (ch->in_room->progtypes & RandProg)
        {
                rset_supermob(ch->in_room);
                rprog_percent_check(supermob, ch, NULL, NULL, RandProg);
                release_supermob();
        }
        return;
}

void rprog_wordlist_check(char *arg, CharData * mob, CharData * actor,
                          ObjData * obj, void *vo, int type,
                          RoomIndexData * room)
{

        char      temp1[MaxStringLength];
        char      temp2[MaxInputLength];
        char      word[MaxInputLength];
        MProgData *mprg;
        char     *list;
        char     *start;
        char     *dupl;
        char     *end;
        int       i;

        if (actor && !char_died(actor) && actor->in_room)
                room = actor->in_room;

        for (mprg = room->mudprogs; mprg; mprg = mprg->next)
                if (mprg->type & type)
                {
                        mudstrlcpy(temp1, mprg->arglist, MSL);
                        list = temp1;
                        for (i = 0; i < (int) strlen(list); i++)
                                list[i] = LOWER(list[i]);
                        mudstrlcpy(temp2, arg, MIL);
                        dupl = temp2;
                        for (i = 0; i < (int) strlen(dupl); i++)
                                dupl[i] = LOWER(dupl[i]);
                        if ((list[0] == 'p') && (list[1] == ' '))
                        {
                                list += 2;
                                while ((start = strstr(dupl, list)))
                                        if ((start == dupl
                                             || *(start - 1) == ' ')
                                            && (*(end = start + strlen(list))
                                                == ' ' || *end == '\n'
                                                || *end == '\r'
                                                || *end == '\0'))
                                        {
                                                rset_supermob(room);
                                                mprog_driver(mprg->comlist,
                                                             mob, actor, obj,
                                                             vo, FALSE);
                                                release_supermob();
                                                break;
                                        }
                                        else
                                                dupl = start + 1;
                        }
                        else
                        {
                                list = one_argument(list, word);
                                for (; word[0] != '\0';
                                     list = one_argument(list, word))
                                        while ((start = strstr(dupl, word)))
                                                if ((start == dupl
                                                     || *(start - 1) == ' ')
                                                    &&
                                                    (*
                                                     (end =
                                                      start + strlen(word)) ==
                                                     ' ' || *end == '\n'
                                                     || *end == '\r'
                                                     || *end == '\0'))
                                                {
                                                        rset_supermob(room);
                                                        mprog_driver(mprg->
                                                                     comlist,
                                                                     mob,
                                                                     actor,
                                                                     obj, vo,
                                                                     FALSE);
                                                        release_supermob();
                                                        break;
                                                }
                                                else
                                                        dupl = start + 1;
                        }
                }
        return;
}

void rprog_time_check(CharData * mob, CharData * actor, ObjData * obj,
                      void *vo, int type)
{
        RoomIndexData *room = (RoomIndexData *) vo;
        MProgData *mprg;
        bool      trigger_time;

        for (mprg = room->mudprogs; mprg; mprg = mprg->next)
        {
                trigger_time = (time_info.hour == atoi(mprg->arglist));

                if (!trigger_time)
                {
                        if (mprg->triggered)
                                mprg->triggered = FALSE;
                        continue;
                }

                if ((mprg->type & type)
                    && ((!mprg->triggered) || (mprg->type & HourProg)))
                {
                        mprg->triggered = TRUE;
                        mprog_driver(mprg->comlist, mob, actor, obj, vo,
                                     FALSE);
                }
        }
        return;
}

void rprog_time_trigger(CharData * ch)
{
        if (ch->in_room->progtypes & TimeProg)
        {
                rset_supermob(ch->in_room);
                rprog_time_check(supermob, NULL, NULL, ch->in_room,
                                 TimeProg);
                release_supermob();
        }
        return;
}

void rprog_hour_trigger(CharData * ch)
{
        if (ch->in_room->progtypes & HourProg)
        {
                rset_supermob(ch->in_room);
                rprog_time_check(supermob, NULL, NULL, ch->in_room,
                                 HourProg);
                release_supermob();
        }
        return;
}

/* Written by Jenny, Nov 29/95 */
void progbug(char *str, CharData * mob)
{
        char      buf[MaxStringLength];

        if (!mob || !mob->pIndexData)
        {
                return;
        }
        /*
         * Check if we're dealing with supermob, which means the bug occurred
         * in a room or obj prog. 
         */
        if (mob->pIndexData->vnum == 3)
        {
                /*
                 * It's supermob.  In set_supermob and rset_supermob, the description
                 * was set to indicate the object or room, so we just need to show
                 * the description in the bug message. 
                 */
                snprintf(buf, MSL, "%s, %s.", str,
                         mob->description ==
                         NULL ? "(unknown)" : mob->description);
        }
        else
        {
                snprintf(buf, MSL, "%s, Mob #%d.", str,
                         mob->pIndexData->vnum);
        }

        bug(buf, 0);
        return;
}


/* Room act prog updates.  Use a separate list cuz we dont really wanna go
   thru 5-10000 rooms every pulse.. can we say lag? -- Alty */

void room_act_add(RoomIndexData * room)
{
        struct act_prog_data *runner;

        for (runner = room_act_list; runner; runner = runner->next)
                if (runner->vo == room)
                        return;
        CREATE(runner, struct act_prog_data, 1);
        runner->vo = room;
        runner->next = room_act_list;
        room_act_list = runner;
}


void room_act_update(void)
{
        struct act_prog_data *runner;
        MProgActList *mpact;

        while ((runner = room_act_list) != NULL)
        {
                RoomIndexData *room = (RoomIndexData *) runner->vo;

                while ((mpact = room->mpact) != NULL)
                {
                        if (mpact->ch->in_room == room)
                                rprog_wordlist_check(mpact->buf, supermob,
                                                     mpact->ch, mpact->obj,
                                                     mpact->vo, ActProg,
                                                     room);
                        room->mpact = mpact->next;
                        DISPOSE(mpact->buf);
                        DISPOSE(mpact);
                }
                room->mpact = NULL;
                room->mpactnum = 0;
                room_act_list = runner->next;
                DISPOSE(runner);
        }
        return;
}

void obj_act_add(ObjData * obj)
{
        struct act_prog_data *runner;

        for (runner = obj_act_list; runner; runner = runner->next)
                if (runner->vo == obj)
                        return;
        CREATE(runner, struct act_prog_data, 1);
        runner->vo = obj;
        runner->next = obj_act_list;
        obj_act_list = runner;
}
void obj_act_update(void)
{
        struct act_prog_data *runner;
        MProgActList *mpact;

        while ((runner = obj_act_list) != NULL)
        {
                ObjData *obj = (ObjData *) runner->vo;

                while ((mpact = obj->mpact) != NULL)
                {
                        oprog_wordlist_check(mpact->buf, supermob, mpact->ch,
                                             mpact->obj, mpact->vo, ActProg,
                                             obj);
                        obj->mpact = mpact->next;
                        DISPOSE(mpact->buf);
                        DISPOSE(mpact);
                }
                obj->mpact = NULL;
                obj->mpactnum = 0;
                obj_act_list = runner->next;
                DISPOSE(runner);
        }
        return;
}

/*
  
     See note above for why we use this.

*/
char     *strip_tilde(char *str)
{
        static char newstr[MaxStringLength];
        int       i, j;

        for (i = j = 0; str[i] != '\0'; i++)
                if (str[i] != '~')
                {
                        newstr[j++] = str[i];
                }
        newstr[j] = '\0';
        return newstr;
}
