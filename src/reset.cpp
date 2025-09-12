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
 *                             Area Reset Module                                         *
 ****************************************************************************************/
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"
#include "installations.hpp"

/* Externals */
extern int top_reset;
char     *sprint_reset args((CharData * ch, ResetData * pReset,
                             sh_int num, bool rlist));
ResetData *parse_reset args((AreaData * tarea, char *argument,
                              CharData * ch));
int get_wearloc args((char *type));
int get_trapflag args((char *flag));
int get_exflag args((char *flag));
int get_rflag args((char *flag));

bool is_room_reset args((ResetData * pReset, RoomIndexData * aRoom,
                         AreaData * pArea));
void delete_reset args((AreaData * pArea, ResetData * pReset));

#define RID RoomIndexData
RID      *find_room args((CharData * ch, char *argument,
                          RoomIndexData * pRoom));
#undef RID
void edit_reset args((CharData * ch, char *argument, AreaData * pArea,
                      RoomIndexData * aRoom));
#define RD ResetData
RD       *find_reset args((AreaData * pArea, RoomIndexData * pRoom,
                           int num));
#undef RD
void list_resets args((CharData * ch, AreaData * pArea,
                       RoomIndexData * pRoom, int start, int end));



ResetData *find_reset(AreaData * pArea, RoomIndexData * pRoom, int numb)
{
        ResetData *pReset;
        int       num = 0;

        for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
                if (is_room_reset(pReset, pRoom, pArea) && ++num >= numb)
                        return pReset;
        return NULL;
}

/* This is one loopy function.  Ugh. -- Altrag */
bool is_room_reset(ResetData * pReset, RoomIndexData * aRoom,
                   AreaData * pArea)
{
        RoomIndexData *pRoom;
        ResetData *reset;
        int       pr;

        if (!aRoom)
                return TRUE;
        switch (pReset->command)
        {
        case 'M':
        case 'O':
                pRoom = get_room_index(pReset->arg3);
                if (!pRoom || pRoom != aRoom)
                        return FALSE;
                return TRUE;
        case 'P':
        case 'T':
        case 'H':
                if (pReset->command == 'H')
                        pr = pReset->arg1;
                else
                        pr = pReset->arg3;
                for (reset = pReset->prev; reset; reset = reset->prev)
                        if ((reset->command == 'O' || reset->command == 'P' ||
                             reset->command == 'G' || reset->command == 'E')
                            && (!pr || pr == reset->arg1)
                            && get_obj_index(reset->arg1))
                                break;
                if (reset && is_room_reset(reset, aRoom, pArea))
                        return TRUE;
                return FALSE;
        case 'B':
                switch (pReset->arg2 & BitResetTypeMask)
                {
                case BitResetDoor:
                case BitResetRoom:
                        return (aRoom->vnum == pReset->arg1);
                case BitResetMobile:
                        for (reset = pReset->prev; reset; reset = reset->prev)
                                if (reset->command == 'M'
                                    && get_mob_index(reset->arg1))
                                        break;
                        if (reset && is_room_reset(reset, aRoom, pArea))
                                return TRUE;
                        return FALSE;
                case BitResetObject:
                        for (reset = pReset->prev; reset; reset = reset->prev)
                                if ((reset->command == 'O'
                                     || reset->command == 'P'
                                     || reset->command == 'G'
                                     || reset->command == 'E')
                                    && (!pReset->arg1
                                        || pReset->arg1 == reset->arg1)
                                    && get_obj_index(reset->arg1))
                                        break;
                        if (reset && is_room_reset(reset, aRoom, pArea))
                                return TRUE;
                        return FALSE;
                }
                return FALSE;
        case 'G':
        case 'E':
                for (reset = pReset->prev; reset; reset = reset->prev)
                        if (reset->command == 'M'
                            && get_mob_index(reset->arg1))
                                break;
                if (reset && is_room_reset(reset, aRoom, pArea))
                        return TRUE;
                return FALSE;
        case 'D':
        case 'R':
                pRoom = get_room_index(pReset->arg1);
                if (!pRoom || pRoom->area != pArea
                    || (aRoom && pRoom != aRoom))
                        return FALSE;
                return TRUE;
        default:
                return FALSE;
        }
        return FALSE;
}

RoomIndexData *find_room(CharData * ch, char *argument,
                           RoomIndexData * pRoom)
{
        char      arg[MaxInputLength];

        if (pRoom)
                return pRoom;
        one_argument(argument, arg);
        if (!is_number(arg) && arg[0] != '\0')
        {
                send_to_char("Reset to which room?\n\r", ch);
                return NULL;
        }
        if (arg[0] == '\0')
                pRoom = ch->in_room;
        else
                pRoom = get_room_index(atoi(arg));
        if (!pRoom)
        {
                send_to_char("Room does not exist.\n\r", ch);
                return NULL;
        }
        return pRoom;
}

/* Separate function for recursive purposes */
#define DelReset(area, reset, rprev) \
do { \
  rprev = reset->prev; \
  delete_reset(area, reset); \
  reset = rprev; \
  continue; \
} while(0)
void delete_reset(AreaData * pArea, ResetData * pReset)
{
        ResetData *reset;
        ResetData *reset_prev;

        if (pReset->command == 'M')
        {
                for (reset = pReset->next; reset; reset = reset->next)
                {
                        /*
                         * Break when a new mob found 
                         */
                        if (reset->command == 'M')
                                break;
                        /*
                         * Delete anything mob is holding 
                         */
                        if (reset->command == 'G' || reset->command == 'E')
                                DelReset(pArea, reset, reset_prev);
                        if (reset->command == 'B' &&
                            (reset->arg2 & BitResetTypeMask) ==
                            BitResetMobile && (!reset->arg1
                                                 || reset->arg1 ==
                                                 pReset->arg1))
                                DelReset(pArea, reset, reset_prev);
                }
        }
        else if (pReset->command == 'O' || pReset->command == 'P' ||
                 pReset->command == 'G' || pReset->command == 'E')
        {
                for (reset = pReset->next; reset; reset = reset->next)
                {
                        if (reset->command == 'T' &&
                            (!reset->arg3 || reset->arg3 == pReset->arg1))
                                DelReset(pArea, reset, reset_prev);
                        if (reset->command == 'H' &&
                            (!reset->arg1 || reset->arg1 == pReset->arg1))
                                DelReset(pArea, reset, reset_prev);
                        /*
                         * Delete nested objects, even if they are the same object. 
                         */
                        if (reset->command == 'P' && (reset->arg3 > 0 ||
                                                      pReset->command != 'P'
                                                      || reset->extra - 1 ==
                                                      pReset->extra)
                            && (!reset->arg3 || reset->arg3 == pReset->arg1))
                                DelReset(pArea, reset, reset_prev);
                        if (reset->command == 'B' &&
                            (reset->arg2 & BitResetTypeMask) ==
                            BitResetObject && (!reset->arg1
                                                 || reset->arg1 ==
                                                 pReset->arg1))
                                DelReset(pArea, reset, reset_prev);

                        /*
                         * Break when a new object of same type is found 
                         */
                        if ((reset->command == 'O' || reset->command == 'P' ||
                             reset->command == 'G' || reset->command == 'E')
                            && reset->arg1 == pReset->arg1)
                                break;
                }
        }
        if (pReset == pArea->last_mob_reset)
                pArea->last_mob_reset = NULL;
        if (pReset == pArea->last_obj_reset)
                pArea->last_obj_reset = NULL;
        UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
        DISPOSE(pReset);
        return;
}

#undef DelReset

ResetData *find_oreset(CharData * ch, AreaData * pArea,
                        RoomIndexData * pRoom, char *name)
{
        ResetData *reset;

        if (!*name)
        {
                for (reset = pArea->last_reset; reset; reset = reset->prev)
                {
                        if (!is_room_reset(reset, pRoom, pArea))
                                continue;
                        switch (reset->command)
                        {
                        default:
                                continue;
                        case 'O':
                        case 'E':
                        case 'G':
                        case 'P':
                                break;
                        }
                        break;
                }
                if (!reset)
                        send_to_char("No object resets in list.\n\r", ch);
                return reset;
        }
        else
        {
                char      arg[MaxInputLength];
                int       cnt = 0, num = number_argument(name, arg);
                ObjIndexData *pObjTo = NULL;

                for (reset = pArea->first_reset; reset; reset = reset->next)
                {
                        if (!is_room_reset(reset, pRoom, pArea))
                                continue;
                        switch (reset->command)
                        {
                        default:
                                continue;
                        case 'O':
                        case 'E':
                        case 'G':
                        case 'P':
                                break;
                        }
                        if ((pObjTo = get_obj_index(reset->arg1)) &&
                            is_name(arg, pObjTo->name) && ++cnt == num)
                                break;
                }
                if (!pObjTo || !reset)
                {
                        send_to_char("To object not in reset list.\n\r", ch);
                        return NULL;
                }
        }
        return reset;
}

ResetData *find_mreset(CharData * ch, AreaData * pArea,
                        RoomIndexData * pRoom, char *name)
{
        ResetData *reset;

        if (!*name)
        {
                for (reset = pArea->last_reset; reset; reset = reset->prev)
                {
                        if (!is_room_reset(reset, pRoom, pArea))
                                continue;
                        switch (reset->command)
                        {
                        default:
                                continue;
                        case 'M':
                                break;
                        }
                        break;
                }
                if (!reset)
                        send_to_char("No mobile resets in list.\n\r", ch);
                return reset;
        }
        else
        {
                char      arg[MaxInputLength];
                int       cnt = 0, num = number_argument(name, arg);
                MobIndexData *pMob = NULL;

                for (reset = pArea->first_reset; reset; reset = reset->next)
                {
                        if (!is_room_reset(reset, pRoom, pArea))
                                continue;
                        switch (reset->command)
                        {
                        default:
                                continue;
                        case 'M':
                                break;
                        }
                        if ((pMob = get_mob_index(reset->arg1)) &&
                            is_name(arg, pMob->PlayerName) && ++cnt == num)
                                break;
                }
                if (!pMob || !reset)
                {
                        send_to_char("Mobile not in reset list.\n\r", ch);
                        return NULL;
                }
        }
        return reset;
}

void edit_reset(CharData * ch, char *argument, AreaData * pArea,
                RoomIndexData * aRoom)
{
        char      arg[MaxInputLength];
        ResetData *pReset = NULL;
        ResetData *reset = NULL;
        MobIndexData *pMob = NULL;
        RoomIndexData *pRoom;
        ObjIndexData *pObj;
        int       num = 0;
        int       vnum;
        char     *origarg = argument;

        argument = one_argument(argument, arg);
        if (!*arg || !str_cmp(arg, "?"))
        {
                char     *nm =
                        (ch->substate ==
                         SubRepeatCmd ? (char *) "" : (aRoom ? (char *)
                                                        "rreset " : (char *)
                                                        "reset "));
                char     *rn = (aRoom ? (char *) "" : (char *) " [room#]");

                ch_printf(ch,
                          "Syntax: %s<list|edit|delete|add|insert|place%s>\n\r",
                          nm, (aRoom ? "" : "|area"));
                ch_printf(ch, "Syntax: %sremove <#>\n\r", nm);
                ch_printf(ch, "Syntax: %smobile <mob#> [limit]%s\n\r", nm,
                          rn);
                ch_printf(ch, "Syntax: %sobject <obj#> [limit [room%s]]\n\r",
                          nm, rn);
                ch_printf(ch,
                          "Syntax: %sobject <obj#> give <mob name> [limit]\n\r",
                          nm);
                ch_printf(ch,
                          "Syntax: %sobject <obj#> equip <mob name> <location> "
                          "[limit]\n\r", nm);
                ch_printf(ch,
                          "Syntax: %sobject <obj#> put <to_obj name> [limit]\n\r",
                          nm);
                ch_printf(ch, "Syntax: %shide <obj name>\n\r", nm);
                ch_printf(ch,
                          "Syntax: %strap <obj name> <type> <charges> <flags>\n\r",
                          nm);
                ch_printf(ch,
                          "Syntax: %strap room <type> <charges> <flags>\n\r",
                          nm);
                ch_printf(ch,
                          "Syntax: %sbit <set|toggle|remove> door%s <dir> "
                          "<exit flags>\n\r", nm, rn);
                ch_printf(ch,
                          "Syntax: %sbit <set|toggle|remove> object <obj name> "
                          "<extra flags>\n\r", nm);
                ch_printf(ch,
                          "Syntax: %sbit <set|toggle|remove> mobile <mob name> "
                          "<affect flags>\n\r", nm);
                ch_printf(ch,
                          "Syntax: %sbit <set|toggle|remove> room%s <room flags>"
                          "\n\r", nm, rn);
                ch_printf(ch, "Syntax: %srandom <last dir>%s\n\r", nm, rn);
                if (!aRoom)
                {
                        send_to_char
                                ("\n\r[room#] will default to the room you are in, "
                                 "if unspecified.\n\r", ch);
                }
                return;
        }
        if (!str_cmp(arg, "on"))
        {
                ch->substate = SubRepeatCmd;
                ch->dest_buf = (aRoom ? (void *) aRoom : (void *) pArea);
                send_to_char("Reset mode on.\n\r", ch);
                return;
        }
        if (!aRoom && !str_cmp(arg, "area"))
        {
                if (!pArea->first_reset)
                {
                        send_to_char("You don't have any resets defined.\n\r",
                                     ch);
                        return;
                }
                num = pArea->nplayer;
                pArea->nplayer = 0;
                reset_area(pArea);
                pArea->nplayer = num;
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "list"))
        {
                int       start, end;

                argument = one_argument(argument, arg);
                start = is_number(arg) ? atoi(arg) : -1;
                argument = one_argument(argument, arg);
                end = is_number(arg) ? atoi(arg) : -1;
                list_resets(ch, pArea, aRoom, start, end);
                return;
        }

        if (!str_cmp(arg, "edit"))
        {
                argument = one_argument(argument, arg);
                num = atoi(arg);

                if (!*arg || !is_number(arg))
                {
                        send_to_char
                                ("Usage: reset edit <number> <command>\n\r",
                                 ch);
                        return;
                }
                if (!(pReset = find_reset(pArea, aRoom, num)))
                {
                        send_to_char("Reset not found.\n\r", ch);
                        return;
                }
                if (!(reset = parse_reset(pArea, argument, ch)))
                {
                        send_to_char
                                ("Error in reset.  Reset not changed.\n\r",
                                 ch);
                        return;
                }
                reset->prev = pReset->prev;
                reset->next = pReset->next;
                if (!pReset->prev)
                        pArea->first_reset = reset;
                else
                        pReset->prev->next = reset;
                if (!pReset->next)
                        pArea->last_reset = reset;
                else
                        pReset->next->prev = reset;
                DISPOSE(pReset);
                send_to_char("Done.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "add"))
        {
                if ((pReset = parse_reset(pArea, argument, ch)) == NULL)
                {
                        send_to_char("Error in reset.  Reset not added.\n\r",
                                     ch);
                        return;
                }
                add_reset(pArea, pReset->command, pReset->extra, pReset->arg1,
                          pReset->arg2, pReset->arg3);
                DISPOSE(pReset);
                send_to_char("Done.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "place"))
        {
                if ((pReset = parse_reset(pArea, argument, ch)) == NULL)
                {
                        send_to_char("Error in reset.  Reset not added.\n\r",
                                     ch);
                        return;
                }
                place_reset(pArea, pReset->command, pReset->extra,
                            pReset->arg1, pReset->arg2, pReset->arg3);
                DISPOSE(pReset);
                send_to_char("Done.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "insert"))
        {
                argument = one_argument(argument, arg);
                if (!*arg || !is_number(arg))
                {
                        send_to_char
                                ("Usage: reset insert <number> <command>\n\r",
                                 ch);
                        return;
                }
                num = atoi(arg);
                if ((reset = find_reset(pArea, aRoom, num)) == NULL)
                {
                        send_to_char("Reset not found.\n\r", ch);
                        return;
                }
                if ((pReset = parse_reset(pArea, argument, ch)) == NULL)
                {
                        send_to_char
                                ("Error in reset.  Reset not inserted.\n\r",
                                 ch);
                        return;
                }
                INSERT(pReset, reset, pArea->first_reset, next, prev);
                send_to_char("Done.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "delete"))
        {
                int       start, end;
                bool      found;

                if (!*argument)
                {
                        send_to_char("Usage: reset delete <start> [end]\n\r",
                                     ch);
                        return;
                }
                argument = one_argument(argument, arg);
                start = is_number(arg) ? atoi(arg) : -1;
                end = is_number(arg) ? atoi(arg) : -1;
                num = 0;
                found = FALSE;
                for (pReset = pArea->first_reset; pReset; pReset = reset)
                {
                        reset = pReset->next;
                        if (!is_room_reset(pReset, aRoom, pArea))
                                continue;
                        if (start > ++num)
                                continue;
                        if ((end != -1 && num > end) || (end == -1 && found))
                                return;
                        UNLINK(pReset, pArea->first_reset, pArea->last_reset,
                               next, prev);
                        if (pReset == pArea->last_mob_reset)
                                pArea->last_mob_reset = NULL;
                        DISPOSE(pReset);
                        top_reset--;
                        found = TRUE;
                }
                if (!found)
                        send_to_char("Reset not found.\n\r", ch);
                else
                        send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "remove"))
        {
                int       iarg;

                argument = one_argument(argument, arg);
                if (arg[0] == '\0' || !is_number(arg))
                {
                        send_to_char("Delete which reset?\n\r", ch);
                        return;
                }
                iarg = atoi(arg);
                for (pReset = pArea->first_reset; pReset;
                     pReset = pReset->next)
                {
                        if (is_room_reset(pReset, aRoom, pArea)
                            && ++num == iarg)
                                break;
                }
                if (!pReset)
                {
                        send_to_char("Reset does not exist.\n\r", ch);
                        return;
                }
                delete_reset(pArea, pReset);
                send_to_char("Reset deleted.\n\r", ch);
                return;
        }
        if (!str_prefix(arg, "mobile"))
        {
                argument = one_argument(argument, arg);
                if (arg[0] == '\0' || !is_number(arg))
                {
                        send_to_char("Reset which mobile vnum?\n\r", ch);
                        return;
                }
                if (!(pMob = get_mob_index(atoi(arg))))
                {
                        send_to_char("Mobile does not exist.\n\r", ch);
                        return;
                }
                argument = one_argument(argument, arg);
                if (arg[0] == '\0')
                        num = 1;
                else if (!is_number(arg))
                {
                        send_to_char("Reset how many mobiles?\n\r", ch);
                        return;
                }
                else
                        num = atoi(arg);
                if (!(pRoom = find_room(ch, argument, aRoom)))
                        return;
                pReset = make_reset('M', 0, pMob->vnum, num, pRoom->vnum);
                LINK(pReset, pArea->first_reset, pArea->last_reset, next,
                     prev);
                send_to_char("Mobile reset added.\n\r", ch);
                return;
        }
        if (!str_prefix(arg, "object"))
        {
                argument = one_argument(argument, arg);
                if (arg[0] == '\0' || !is_number(arg))
                {
                        send_to_char("Reset which object vnum?\n\r", ch);
                        return;
                }
                if (!(pObj = get_obj_index(atoi(arg))))
                {
                        send_to_char("Object does not exist.\n\r", ch);
                        return;
                }
                argument = one_argument(argument, arg);
                if (arg[0] == '\0')
                        mudstrlcpy(arg, "room", MIL);
                if (!str_prefix(arg, "put"))
                {
                        argument = one_argument(argument, arg);
                        if (!(reset = find_oreset(ch, pArea, aRoom, arg)))
                                return;
                        /*
                         * Put in_objects after hide and trap resets 
                         */
                        while (reset->next && (reset->next->command == 'H' ||
                                               reset->next->command == 'T'
                                               || reset->next->command ==
                                               'B'))
                                reset = reset->next;
/*      pReset = make_reset('P', 1, pObj->vnum, num, reset->arg1);*/
                        argument = one_argument(argument, arg);
                        if ((vnum = atoi(arg)) < 1)
                                vnum = 1;
                        pReset = make_reset('P', reset->extra + 1, pObj->vnum,
                                            vnum, 0);
                        /*
                         * Grumble.. insert puts pReset before reset, and we need it after,
                         * so we make a hackup and reverse all the list params.. :P.. 
                         */
                        INSERT(pReset, reset, pArea->last_reset, prev, next);
                        send_to_char("Object reset in object created.\n\r",
                                     ch);
                        return;
                }
                if (!str_prefix(arg, "give"))
                {
                        argument = one_argument(argument, arg);
                        if (!(reset = find_mreset(ch, pArea, aRoom, arg)))
                                return;
                        while (reset->next && reset->next->command == 'B')
                                reset = reset->next;
                        argument = one_argument(argument, arg);
                        if ((vnum = atoi(arg)) < 1)
                                vnum = 1;
                        pReset = make_reset('G', 1, pObj->vnum, vnum, 0);
                        INSERT(pReset, reset, pArea->last_reset, prev, next);
                        send_to_char("Object reset to mobile created.\n\r",
                                     ch);
                        return;
                }
                if (!str_prefix(arg, "equip"))
                {
                        argument = one_argument(argument, arg);
                        if (!(reset = find_mreset(ch, pArea, aRoom, arg)))
                                return;
                        while (reset->next && reset->next->command == 'B')
                                reset = reset->next;
                        num = get_wearloc(argument);
                        if (num < 0)
                        {
                                send_to_char
                                        ("Reset object to which location?\n\r",
                                         ch);
                                return;
                        }
                        for (pReset = reset->next; pReset;
                             pReset = pReset->next)
                        {
                                if (pReset->command == 'M')
                                        break;
                                if (pReset->command == 'E'
                                    && pReset->arg3 == num)
                                {
                                        send_to_char
                                                ("Mobile already has an item equipped there.\n\r",
                                                 ch);
                                        return;
                                }
                        }
                        argument = one_argument(argument, arg);
                        if ((vnum = atoi(arg)) < 1)
                                vnum = 1;
                        pReset = make_reset('E', 1, pObj->vnum, vnum, num);
                        INSERT(pReset, reset, pArea->last_reset, prev, next);
                        send_to_char
                                ("Object reset equipped by mobile created.\n\r",
                                 ch);
                        return;
                }
                if (arg[0] == '\0' || !(num = (int) str_cmp(arg, "room")) ||
                    is_number(arg))
                {
                        if (!(bool) num)
                                argument = one_argument(argument, arg);
                        if (!(pRoom = find_room(ch, argument, aRoom)))
                                return;
                        if (pRoom->area != pArea)
                        {
                                send_to_char
                                        ("Cannot reset objects to other areas.\n\r",
                                         ch);
                                return;
                        }
                        if ((vnum = atoi(arg)) < 1)
                                vnum = 1;
                        pReset = make_reset('O', 0, pObj->vnum, vnum,
                                            pRoom->vnum);
                        LINK(pReset, pArea->first_reset, pArea->last_reset,
                             next, prev);
                        send_to_char("Object reset added.\n\r", ch);
                        return;
                }
                send_to_char("Reset object to where?\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "random"))
        {
                argument = one_argument(argument, arg);
                vnum = get_dir(arg);
                if (vnum < 0 || vnum > 9)
                {
                        send_to_char("Reset which random doors?\n\r", ch);
                        return;
                }
                if (vnum == 0)
                {
                        send_to_char
                                ("There is no point in randomizing one door.\n\r",
                                 ch);
                        return;
                }
                pRoom = find_room(ch, argument, aRoom);
                if (pRoom->area != pArea)
                {
                        send_to_char
                                ("Cannot randomize doors in other areas.\n\r",
                                 ch);
                        return;
                }
                pReset = make_reset('R', 0, pRoom->vnum, vnum, 0);
                LINK(pReset, pArea->first_reset, pArea->last_reset, next,
                     prev);
                send_to_char("Reset random doors created.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "trap"))
        {
                char      oname[MaxInputLength];
                int       chrg, value, extra = 0;
                bool      isobj;

                argument = one_argument(argument, oname);
                argument = one_argument(argument, arg);
                num = is_number(arg) ? atoi(arg) : -1;
                argument = one_argument(argument, arg);
                chrg = is_number(arg) ? atoi(arg) : -1;
                isobj = is_name(argument, "obj");
                if (isobj == is_name(argument, "room"))
                {
                        send_to_char
                                ("Reset: TRAP: Must specify ROOM or OBJECT\n\r",
                                 ch);
                        return;
                }
                if (!str_cmp(oname, "room") && !isobj)
                {
                        vnum = (aRoom ? aRoom->vnum : ch->in_room->vnum);
                        extra = TrapRoom;
                }
                else
                {
                        if (is_number(oname) && !isobj)
                        {
                                vnum = atoi(oname);
                                if (!get_room_index(vnum))
                                {
                                        send_to_char
                                                ("Reset: TRAP: no such room\n\r",
                                                 ch);
                                        return;
                                }
                                reset = NULL;
                                extra = TrapRoom;
                        }
                        else
                        {
                                if (!
                                    (reset =
                                     find_oreset(ch, pArea, aRoom, oname)))
                                        return;
/*        vnum = reset->arg1;*/
                                vnum = 0;
                                extra = TrapObj;
                        }
                }
                if (num < 1 || num > MaxTraptype)
                {
                        send_to_char("Reset: TRAP: invalid trap type\n\r",
                                     ch);
                        return;
                }
                if (chrg < 0 || chrg > 10000)
                {
                        send_to_char("Reset: TRAP: invalid trap charges\n\r",
                                     ch);
                        return;
                }
                while (*argument)
                {
                        argument = one_argument(argument, arg);
                        value = get_trapflag(arg);
                        if (value < 0 || value > 31)
                        {
                                send_to_char("Reset: TRAP: bad flag\n\r", ch);
                                return;
                        }
                        SetBit(extra, 1 << value);
                }
                pReset = make_reset('T', extra, num, chrg, vnum);
                if (reset)
                        INSERT(pReset, reset, pArea->last_reset, prev, next);
                else
                        LINK(pReset, pArea->first_reset, pArea->last_reset,
                             next, prev);
                send_to_char("Trap created.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "bit"))
        {
                int       (*flfunc) (char *type);
                int       flags = 0;
                char      option[MaxInputLength];
                char     *parg;

                argument = one_argument(argument, option);
                if (!*option)
                {
                        send_to_char
                                ("You must specify SET, REMOVE, or TOGGLE.\n\r",
                                 ch);
                        return;
                }
                num = 0;
                if (!str_prefix(option, "set"))
                        SetBit(num, BitResetSet);
                else if (!str_prefix(option, "toggle"))
                        SetBit(num, BitResetToggle);
                else if (str_prefix(option, "remove"))
                {
                        send_to_char
                                ("You must specify SET, REMOVE, or TOGGLE.\n\r",
                                 ch);
                        return;
                }
                argument = one_argument(argument, option);
                parg = argument;
                argument = one_argument(argument, arg);
                if (!*option)
                {
                        send_to_char
                                ("Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r",
                                 ch);
                        return;
                }
                if (!str_prefix(option, "door"))
                {
                        SetBit(num, BitResetDoor);
                        if (aRoom)
                        {
                                pRoom = aRoom;
                                argument = parg;
                        }
                        else if (!is_number(arg))
                        {
                                pRoom = ch->in_room;
                                argument = parg;
                        }
                        else if (!(pRoom = find_room(ch, arg, aRoom)))
                                return;
                        argument = one_argument(argument, arg);
                        if (!*arg)
                        {
                                send_to_char("Must specify direction.\n\r",
                                             ch);
                                return;
                        }
                        vnum = get_dir(arg);
                        SetBit(num, vnum << BitResetDoorThreshold);
                        vnum = pRoom->vnum;
                        flfunc = &get_exflag;
                        reset = NULL;
                }
                else if (!str_prefix(option, "object"))
                {
                        SetBit(num, BitResetObject);
                        vnum = 0;
                        flfunc = &get_oflag;
                        if (!(reset = find_oreset(ch, pArea, aRoom, arg)))
                                return;
                }
                else if (!str_prefix(option, "mobile"))
                {
                        SetBit(num, BitResetMobile);
                        vnum = 0;
                        flfunc = &get_aflag;
                        if (!(reset = find_mreset(ch, pArea, aRoom, arg)))
                                return;
                }
                else if (!str_prefix(option, "room"))
                {
                        SetBit(num, BitResetRoom);
                        if (aRoom)
                        {
                                pRoom = aRoom;
                                argument = parg;
                        }
                        else if (!is_number(arg))
                        {
                                pRoom = ch->in_room;
                                argument = parg;
                        }
                        else if (!(pRoom = find_room(ch, arg, aRoom)))
                                return;
                        vnum = pRoom->vnum;
                        flfunc = &get_rflag;
                        reset = NULL;
                }
                else
                {
                        send_to_char
                                ("Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r",
                                 ch);
                        return;
                }
                while (*argument)
                {
                        int       value;

                        argument = one_argument(argument, arg);
                        value = (*flfunc) (arg);
                        if (value < 0 || value > 31)
                        {
                                send_to_char("Reset: BIT: bad flag\n\r", ch);
                                return;
                        }
                        SetBit(flags, 1 << value);
                }
                if (!flags)
                {
                        send_to_char("Set which flags?\n\r", ch);
                        return;
                }
                pReset = make_reset('B', 1, vnum, num, flags);
                if (reset)
                        INSERT(pReset, reset, pArea->last_reset, prev, next);
                else
                        LINK(pReset, pArea->first_reset, pArea->last_reset,
                             next, prev);
                send_to_char("Bitvector reset created.\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "hide"))
        {
                argument = one_argument(argument, arg);
                if (!(reset = find_oreset(ch, pArea, aRoom, arg)))
                        return;
/*    pReset = make_reset('H', 1, reset->arg1, 0, 0);*/
                pReset = make_reset('H', 1, 0, 0, 0);
                INSERT(pReset, reset, pArea->last_reset, prev, next);
                send_to_char("Object hide reset created.\n\r", ch);
                return;
        }
        if (ch->substate == SubRepeatCmd)
        {
                ch->substate = SubNone;
                interpret(ch, origarg);
                ch->substate = SubRepeatCmd;
                ch->last_cmd = (aRoom ? do_rreset : do_reset);
        }
        else
                edit_reset(ch, "", pArea, aRoom);
        return;
}

CMDF do_reset(CharData * ch, char *argument)
{
        AreaData *pArea = NULL;
        char      arg[MaxInputLength];
        char     *parg;

        parg = one_argument(argument, arg);
        if (ch->substate == SubRepeatCmd)
        {
                pArea = (AreaData *) ch->dest_buf;
                if (pArea && pArea != ch->PCData->area
                    && pArea != ch->in_room->area)
                {
                        AreaData *tmp;

                        for (tmp = first_build; tmp; tmp = tmp->next)
                                if (tmp == pArea)
                                        break;
                        if (!tmp)
                                for (tmp = first_area; tmp; tmp = tmp->next)
                                        if (tmp == pArea)
                                                break;
                        if (!tmp)
                        {
                                send_to_char
                                        ("Your area pointer got lost.  Reset mode off.\n\r",
                                         ch);
                                bug("do_reset: %s's dest_buf points to invalid area", ch->name);
                                ch->substate = SubNone;
                                ch->dest_buf = NULL;
                                return;
                        }
                }
                if (!*arg)
                {
                        ch_printf(ch, "Editing resets for area: %s\n\r",
                                  pArea->name);
                        return;
                }
                if (!str_cmp(arg, "done") || !str_cmp(arg, "off"))
                {
                        send_to_char("Reset mode off.\n\r", ch);
                        ch->substate = SubNone;
                        ch->dest_buf = NULL;
                        return;
                }
        }
        if (!pArea && get_trust(ch) > LevelGod)
        {
                char      fname[80];

                snprintf(fname, MSL, "%s.are", capitalize(arg));
                for (pArea = first_build; pArea; pArea = pArea->next)
                        if (!str_cmp(fname, pArea->filename))
                        {
                                argument = parg;
                                break;
                        }
                if (!pArea)
                        pArea = !IsNpc(ch) ? ch->PCData->
                                area : ((ch->desc && ch->desc->original) ?
                                        ch->desc->original->PCData->
                                        area : NULL);
                ;
                if (!pArea)
                        pArea = ch->in_room->area;
        }
        else
                pArea = ch->PCData->area;
        if (!pArea)
        {
                send_to_char("You do not have an assigned area.\n\r", ch);
                return;
        }
        edit_reset(ch, argument, pArea, NULL);
        return;
}

CMDF do_rreset(CharData * ch, char *argument)
{
        RoomIndexData *pRoom;

        if (ch->substate == SubRepeatCmd)
        {
                pRoom = (RoomIndexData *) ch->dest_buf;
                if (!pRoom)
                {
                        send_to_char
                                ("Your room pointer got lost.  Reset mode off.\n\r",
                                 ch);
                        bug("do_rreset: %s's dest_buf points to invalid room",
                            ch->name);
                }
                ch->substate = SubNone;
                ch->dest_buf = NULL;
                return;
        }
        else
                pRoom = ch->in_room;
        if (!can_rmodify(ch, pRoom))
                return;
        edit_reset(ch, argument, pRoom->area, pRoom);
        return;
}

void add_obj_reset(AreaData * pArea, char cm, ObjData * obj, int v2, int v3)
{
        ObjData *inobj;
        static int iNest;

        if ((cm == 'O' || cm == 'P')
            && obj->pIndexData->vnum == ObjVnumTrap)
        {
                if (cm == 'O')
                        add_reset(pArea, 'T', obj->value[3], obj->value[1],
                                  obj->value[0], v3);
                return;
        }
        add_reset(pArea, cm, (cm == 'P' ? iNest : 1), obj->pIndexData->vnum,
                  v2, v3);
        /*
         * Only add hide for in-room objects that are hidden and cant be moved, as
         * hide is an update reset, not a load-only reset. 
         */
        if (cm == 'O' && IsObjStat(obj, ItemHidden) &&
            !IsSet(obj->wear_flags, ItemTake))
                add_reset(pArea, 'H', 1, 0, 0, 0);
        for (inobj = obj->first_content; inobj; inobj = inobj->next_content)
                if (inobj->pIndexData->vnum == ObjVnumTrap)
                        add_obj_reset(pArea, 'O', inobj, 0, 0);
        if (cm == 'P')
                iNest++;
        for (inobj = obj->first_content; inobj; inobj = inobj->next_content)
                add_obj_reset(pArea, 'P', inobj, 1, 0);
        if (cm == 'P')
                iNest--;
        return;
}

void instaroom(AreaData * pArea, RoomIndexData * pRoom, bool dodoors)
{
        CharData *rch;
        ObjData *obj;

        for (rch = pRoom->first_person; rch; rch = rch->next_in_room)
        {
                if (!IsNpc(rch))
                        continue;
                add_reset(pArea, 'M', 1, rch->pIndexData->vnum,
                          rch->pIndexData->count, pRoom->vnum);
                for (obj = rch->first_carrying; obj; obj = obj->next_content)
                {
                        if (obj->wear_loc == WearNone)
                                add_obj_reset(pArea, 'G', obj, 1, 0);
                        else
                                add_obj_reset(pArea, 'E', obj, 1,
                                              obj->wear_loc);
                }
        }
        for (obj = pRoom->first_content; obj; obj = obj->next_content)
        {
                if (obj->item_type == ItemSpacecraft)
                        continue;
                add_obj_reset(pArea, 'O', obj, 1, pRoom->vnum);
        }
        if (dodoors)
        {
                ExitData *pexit;

                for (pexit = pRoom->first_exit; pexit; pexit = pexit->next)
                {
                        int       state = 0;

                        if (!IsSet(pexit->exit_info, ExIsdoor))
                                continue;
                        if (IsSet(pexit->exit_info, ExClosed))
                        {
                                if (IsSet(pexit->exit_info, ExLocked))
                                        state = 2;
                                else
                                        state = 1;
                        }
                        add_reset(pArea, 'D', 0, pRoom->vnum, pexit->vdir,
                                  state);
                }
        }
        return;
}

void wipe_resets(AreaData * pArea, RoomIndexData * pRoom)
{
        ResetData *pReset;

        for (pReset = pArea->first_reset; pReset;)
        {
				if (pReset->command != 'R'
                    && is_room_reset(pReset, pRoom, pArea))
                {
                        /*
                         * Resets always go forward, so we can safely use the previous reset,
                         * providing it exists, or first_reset if it doesnt.  -- Altrag 
                         */
                        ResetData *prev = pReset->prev;

                        delete_reset(pArea, pReset);
                        pReset = (prev ? prev->next : pArea->first_reset);
                }
                else
                        pReset = pReset->next;
        }
        return;
}

CMDF do_instaroom(CharData * ch, char *argument)
{
        AreaData *pArea;
        RoomIndexData *pRoom;
        bool      dodoors;
        char      arg[MaxInputLength];

        if (IsNpc(ch) || get_trust(ch) < LevelSavior || !ch->PCData ||
            !ch->PCData->area)
        {
                send_to_char
                        ("You don't have an assigned area to create resets for.\n\r",
                         ch);
                return;
        }
        argument = one_argument(argument, arg);
        if (!str_cmp(argument, "nodoors"))
                dodoors = FALSE;
        else
                dodoors = TRUE;
        pArea = ch->PCData->area;
        if (!(pRoom = find_room(ch, arg, NULL)))
        {
                send_to_char("Room doesn't exist.\n\r", ch);
                return;
        }
        if (!can_rmodify(ch, pRoom))
                return;
        if (pRoom->area != pArea && get_trust(ch) < LevelGreater)
        {
                send_to_char("You cannot reset that room.\n\r", ch);
                return;
        }
        if (pArea->first_reset)
                wipe_resets(pArea, pRoom);
        instaroom(pArea, pRoom, dodoors);
        send_to_char("Room resets installed.\n\r", ch);
}

CMDF do_instazone(CharData * ch, char *argument)
{
        AreaData *pArea;
        int       vnum;
        RoomIndexData *pRoom;
        bool      dodoors;

        if (IsNpc(ch) || get_trust(ch) < LevelSavior || !ch->PCData ||
            !ch->PCData->area)
        {
                send_to_char
                        ("You don't have an assigned area to create resets for.\n\r",
                         ch);
                return;
        }
        if (!str_cmp(argument, "nodoors"))
                dodoors = FALSE;
        else
                dodoors = TRUE;
        pArea = ch->PCData->area;
        if (pArea->first_reset)
                wipe_resets(pArea, NULL);
        for (vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++)
        {
                if (!(pRoom = get_room_index(vnum)) || pRoom->area != pArea)
                        continue;
                instaroom(pArea, pRoom, dodoors);
        }
        send_to_char("Area resets installed.\n\r", ch);
        return;
}

int generate_itemlevel(AreaData * pArea, ObjIndexData * pObjIndex)
{
        int       olevel;
        int       min = UMax(pArea->low_soft_range, 1);
        int       max = UMin(pArea->hi_soft_range, min + 15);

        if (pObjIndex->level > 0)
                olevel = UMin(pObjIndex->level, MaxLevel);
        else
                switch (pObjIndex->item_type)
                {
                default:
                        olevel = 0;
                        break;
                case ItemPill:
                        olevel = number_range(min, max);
                        break;
                case ItemPotion:
                        olevel = number_range(min, max);
                        break;
                case ItemDevice:
                        olevel = number_range(min, max);
                        break;
                case ItemArmor:
                        olevel = number_range(min + 4, max + 1);
                        break;
                case ItemWeapon:
                        olevel = number_range(min + 4, max + 1);
                        break;
                }
        return olevel;
}

/*
 * Reset one area.
 */
void reset_area(AreaData * pArea)
{
        ResetData *pReset;
        CharData *mob;
        ObjData *obj;
        ObjData *lastobj;
        RoomIndexData *pRoomIndex;
        MobIndexData *pMobIndex;
        ObjIndexData *pObjIndex;
        ObjIndexData *pObjToIndex;
        ExitData *pexit;
        ObjData *to_obj;
        int       level = 0;
        int      *plc = NULL;
        ExtBV   *xplc;
        InstallationData *installation;

        if (!pArea)
        {
                bug("reset_area: NULL pArea", 0);
                return;
        }

        mob = NULL;
        obj = NULL;
        lastobj = NULL;
        if (!pArea->first_reset)
        {
                bug("%s: reset_area: no resets", pArea->filename);
                return;
        }
        level = 0;
        for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
        {
                switch (pReset->command)
                {
                default:
                        bug("Reset_area: bad command %c.", pReset->command);
                        break;

                case 'M':
                        if (!(pMobIndex = get_mob_index(pReset->arg1)))
                        {
                                bug("Reset_area: 'M': bad mob vnum %d.",
                                    pReset->arg1);
                                continue;
                        }
                        if (!(pRoomIndex = get_room_index(pReset->arg3)))
                        {
                                bug("Reset_area: 'M': bad room vnum %d.",
                                    pReset->arg3);
                                continue;
                        }
                        if (pMobIndex->count >= pReset->arg2)
                        {
                                mob = NULL;
                                break;
                        }
                        mob = create_mobile(pMobIndex);
                        {
                                RoomIndexData *pRoomPrev =
                                        get_room_index(pReset->arg3 - 1);

                                if (pRoomPrev
                                    && IsSet(pRoomPrev->RoomFlags,
                                               RoomPetShop))
                                        SetBit(mob->act, ActPet);
                        }
                        if (room_is_dark(pRoomIndex))
                                SetBit(mob->affected_by, AffInfrared);
                        char_to_room(mob, pRoomIndex);
                        economize_mobgold(mob);
                        level = URange(0, mob->top_level - 2, LevelAvatar);
                        if (IsSet(mob->act, ActCitizen) && pArea->planet)
                                pArea->planet->population++;
                        if ((installation =
                             installation_from_room(mob->in_room->vnum)) !=
                            NULL)
                                mob->mob_clan = installation->clan->name;
                        break;

                case 'G':
                case 'E':
                        if (!(pObjIndex = get_obj_index(pReset->arg1)))
                        {
                                bug("Reset_area: 'E' or 'G': bad obj vnum %d.", pReset->arg1);
                                continue;
                        }
                        if (!mob)
                        {
                                lastobj = NULL;
                                break;
                        }
                        if (mob->pIndexData->pShop)
                        {
                                int       olevel =
                                        generate_itemlevel(pArea, pObjIndex);
                                obj = create_object(pObjIndex, olevel);
                                SetBit(obj->extra_flags, ItemInventory);
                        }
                        else
                                obj = create_object(pObjIndex,
                                                    number_fuzzy(level));
                        obj->level = URange(0, obj->level, LevelAvatar);
                        obj = obj_to_char(obj, mob);
                        if (pReset->command == 'E')
                        {
                                if (obj->carried_by != mob)
                                {
                                        bug("'E' reset: can't give object %d to mob %d.", obj->pIndexData->vnum, mob->pIndexData->vnum);
                                        break;
                                }
                                equip_char(mob, obj, pReset->arg3);
                        }
                        lastobj = obj;
                        break;

                case 'O':
                        if (!(pObjIndex = get_obj_index(pReset->arg1)))
                        {
                                bug("Reset_area: 'O': bad obj vnum %d.",
                                    pReset->arg1);
                                continue;
                        }
                        if (!(pRoomIndex = get_room_index(pReset->arg3)))
                        {
                                bug("Reset_area: 'O': bad room vnum %d.",
                                    pReset->arg3);
                                continue;
                        }
                        if (count_obj_list
                            (pObjIndex, pRoomIndex->first_content) > 0)
                        {
                                obj = NULL;
                                lastobj = NULL;
                                break;
                        }
                        obj = create_object(pObjIndex,
                                            number_fuzzy(generate_itemlevel
                                                         (pArea, pObjIndex)));
                        obj->level = UMin(obj->level, LevelAvatar);
                        obj->cost = 0;
                        obj_to_room(obj, pRoomIndex);
                        lastobj = obj;
                        break;

                case 'P':
                        if (!(pObjIndex = get_obj_index(pReset->arg1)))
                        {
                                bug("Reset_area: 'P': bad obj vnum %d.",
                                    pReset->arg1);
                                continue;
                        }
                        if (pReset->arg3 > 0)
                        {
                                if (!
                                    (pObjToIndex =
                                     get_obj_index(pReset->arg3)))
                                {
                                        bug("Reset_area: 'P': bad objto vnum %d.", pReset->arg3);
                                        continue;
                                }
                                if (pArea->nplayer > 0 ||
                                    !(to_obj = get_obj_type(pObjToIndex)) ||
                                    !to_obj->in_room ||
                                    count_obj_list(pObjIndex,
                                                   to_obj->first_content) > 0)
                                {
                                        obj = NULL;
                                        break;
                                }
                                lastobj = to_obj;
                        }
                        else
                        {
                                int       iNest;

                                if (!lastobj)
                                        break;
                                to_obj = lastobj;
                                for (iNest = 0; iNest < pReset->extra;
                                     iNest++)
                                        if (!(to_obj = to_obj->last_content))
                                        {
                                                bug("Reset_area: 'P': Invalid nesting obj %d.", pReset->arg1);
                                                iNest = -1;
                                                break;
                                        }
                                if (iNest < 0)
                                        continue;
                        }
                        obj = create_object(pObjIndex,
                                            number_fuzzy(UMax
                                                         (generate_itemlevel
                                                          (pArea, pObjIndex),
                                                          to_obj->level)));
                        obj->level = UMin(obj->level, LevelAvatar);
                        obj_to_obj(obj, to_obj);
                        break;

                case 'T':
                        if (IsSet(pReset->extra, TrapObj))
                        {
                                /*
                                 * We need to preserve obj for future 'T' and 'H' checks 
                                 */
                                ObjData *pobj;

                                if (pReset->arg3 > 0)
                                {
                                        if (!
                                            (pObjToIndex =
                                             get_obj_index(pReset->arg3)))
                                        {
                                                bug("Reset_area: 'T': bad objto vnum %d.", pReset->arg3);
                                                continue;
                                        }
                                        if (pArea->nplayer > 0 ||
                                            !(to_obj =
                                              get_obj_type(pObjToIndex))
                                            || (to_obj->carried_by
                                                && !IsNpc(to_obj->
                                                           carried_by))
                                            || is_trapped(to_obj))
                                                break;
                                }
                                else
                                {
                                        if (!lastobj || !obj)
                                                break;
                                        to_obj = obj;
                                }
                                pobj = make_trap(pReset->arg2, pReset->arg1,
                                                 number_fuzzy(to_obj->level),
                                                 pReset->extra);
                                obj_to_obj(pobj, to_obj);
                        }
                        else
                        {
                                if (!
                                    (pRoomIndex =
                                     get_room_index(pReset->arg3)))
                                {
                                        bug("Reset_area: 'T': bad room %d.",
                                            pReset->arg3);
                                        continue;
                                }
                                if (pArea->nplayer > 0 ||
                                    count_obj_list(get_obj_index
                                                   (ObjVnumTrap),
                                                   pRoomIndex->
                                                   first_content) > 0)
                                        break;
                                to_obj = make_trap(pReset->arg1, pReset->arg1,
                                                   10, pReset->extra);
                                obj_to_room(to_obj, pRoomIndex);
                        }
                        break;

                case 'H':
                        if (pReset->arg1 > 0)
                        {
                                if (!
                                    (pObjToIndex =
                                     get_obj_index(pReset->arg1)))
                                {
                                        bug("Reset_area: 'H': bad objto vnum %d.", pReset->arg1);
                                        continue;
                                }
                                if (pArea->nplayer > 0 ||
                                    !(to_obj = get_obj_type(pObjToIndex)) ||
                                    !to_obj->in_room ||
                                    to_obj->in_room->area != pArea ||
                                    IsObjStat(to_obj, ItemHidden))
                                        break;
                        }
                        else
                        {
                                if (!lastobj || !obj)
                                        break;
                                to_obj = obj;
                        }
                        SetBit(to_obj->extra_flags, ItemHidden);
                        break;

                case 'B':
                        switch (pReset->arg2 & BitResetTypeMask)
                        {
                        case BitResetDoor:
                                {
                                        int       doornum;

                                        if (!
                                            (pRoomIndex =
                                             get_room_index(pReset->arg1)))
                                        {
                                                bug("Reset_area: 'B': door: bad room vnum %d.", pReset->arg1);
                                                continue;
                                        }
                                        doornum =
                                                (pReset->
                                                 arg2 & BitResetDoorMask)
                                                >> BitResetDoorThreshold;
                                        if (!
                                            (pexit =
                                             get_exit(pRoomIndex, doornum)))
                                                break;
                                        plc = &pexit->exit_info;
                                }
                                break;
                        case BitResetRoom:
                                if (!
                                    (pRoomIndex =
                                     get_room_index(pReset->arg1)))
                                {
                                        bug("Reset_area: 'B': room: bad room vnum %d.", pReset->arg1);
                                        continue;
                                }
                                xplc = &pRoomIndex->RoomFlags;
                                break;
                        case BitResetObject:
                                if (pReset->arg1 > 0)
                                {
                                        if (!
                                            (pObjToIndex =
                                             get_obj_index(pReset->arg1)))
                                        {
                                                bug("Reset_area: 'B': object: bad objto vnum %d.", pReset->arg1);
                                                continue;
                                        }
                                        if (!
                                            (to_obj =
                                             get_obj_type(pObjToIndex))
                                            || !to_obj->in_room
                                            || to_obj->in_room->area != pArea)
                                                continue;
                                }
                                else
                                {
                                        if (!lastobj || !obj)
                                                continue;
                                        to_obj = obj;
                                }
                                plc = &to_obj->extra_flags;
                                break;
                        case BitResetMobile:
                                if (!mob)
                                        continue;
                                plc = &mob->affected_by;
                                break;
                        default:
                                bug("Reset_area: 'B': bad options %d.",
                                    pReset->arg2);
                                continue;
                        }
                        if (IsSet(pReset->arg2, BitResetSet))
                                SetBit(*plc, pReset->arg3);
                        else if (IsSet(pReset->arg2, BitResetToggle))
                                ToggleBit(*plc, pReset->arg3);
                        else
                                RemoveBit(*plc, pReset->arg3);
                        break;

                case 'D':
                        if (!(pRoomIndex = get_room_index(pReset->arg1)))
                        {
                                bug("Reset_area: 'D': bad room vnum %d.",
                                    pReset->arg1);
                                continue;
                        }
                        if (!(pexit = get_exit(pRoomIndex, pReset->arg2)))
                                break;
                        switch (pReset->arg3)
                        {
                        case 0:
                                RemoveBit(pexit->exit_info, ExClosed);
                                RemoveBit(pexit->exit_info, ExLocked);
                                break;
                        case 1:
                                SetBit(pexit->exit_info, ExClosed);
                                RemoveBit(pexit->exit_info, ExLocked);
                                if (IsSet(pexit->exit_info, EX_xSEARCHABLE))
                                        SetBit(pexit->exit_info, ExSecret);
                                break;
                        case 2:
                                SetBit(pexit->exit_info, ExClosed);
                                SetBit(pexit->exit_info, ExLocked);
                                if (IsSet(pexit->exit_info, EX_xSEARCHABLE))
                                        SetBit(pexit->exit_info, ExSecret);
                                break;
                        }
                        break;

                case 'R':
                        if (!(pRoomIndex = get_room_index(pReset->arg1)))
                        {
                                bug("Reset_area: 'R': bad room vnum %d.",
                                    pReset->arg1);
                                continue;
                        }
                        randomize_exits(pRoomIndex, pReset->arg2 - 1);
                        break;
                }
        }
        return;
}

void list_resets(CharData * ch, AreaData * pArea, RoomIndexData * pRoom,
                 int start, int end)
{
        ResetData *pReset;
        RoomIndexData *room;
        MobIndexData *mob;
        ObjIndexData *obj, *obj2;
        ObjIndexData *lastobj;
        ResetData *lo_reset;
        bool      found;
        int       num = 0;
        const char *rname, *mname = NULL, *oname = NULL;
        char      buf[256];
        char     *pbuf;

        if (!ch || !pArea)
                return;
        room = NULL;
        mob = NULL;
        obj = NULL;
        lastobj = NULL;
        lo_reset = NULL;
        found = FALSE;

        for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
        {
                if (!is_room_reset(pReset, pRoom, pArea))
                        continue;
                ++num;
                snprintf(buf, MSL, "%2d) ", num);
                pbuf = buf + strlen(buf);
                switch (pReset->command)
                {
                default:
                        snprintf(pbuf, MSL,
                                 "*** BAD RESET: %c %d %d %d %d ***\n\r",
                                 pReset->command, pReset->extra, pReset->arg1,
                                 pReset->arg2, pReset->arg3);
                        break;
                case 'M':
                        if (!(mob = get_mob_index(pReset->arg1)))
                                mname = "Mobile: *BAD VNUM*";
                        else
                                mname = mob->PlayerName;
                        if (!(room = get_room_index(pReset->arg3)))
                                rname = "Room: *BAD VNUM*";
                        else
                                rname = room->name;
                        snprintf(pbuf, MSL, "%s (%d) -> %s (%d) [%d]", mname,
                                 pReset->arg1, rname, pReset->arg3,
                                 pReset->arg2);
                        if (!room)
                                mob = NULL;
                        if ((room = get_room_index(pReset->arg3 - 1)) &&
                            IsSet(room->RoomFlags, RoomPetShop))
                                mudstrlcat(buf, " (pet)\n\r", 256);
                        else
                                mudstrlcat(buf, "\n\r", 256);
                        break;
                case 'G':
                case 'E':
                        if (!mob)
                                mname = "* ERROR: NO MOBILE! *";
                        if (!(obj = get_obj_index(pReset->arg1)))
                                oname = "Object: *BAD VNUM*";
                        else
                                oname = obj->name;
                        snprintf(pbuf, MSL, "%s (%d) -> %s (%s) [%d]", oname,
                                 pReset->arg1, mname,
                                 (pReset->command ==
                                  'G' ? "carry" : wear_locs[URange(0,pReset->arg3,MaxWear)]),
                                 pReset->arg2);
                        if (mob && mob->pShop)
                                mudstrlcat(buf, " (shop)\n\r", 256);
                        else
                                mudstrlcat(buf, "\n\r", 256);
                        lastobj = obj;
                        lo_reset = pReset;
                        break;
                case 'O':
                        if (!(obj = get_obj_index(pReset->arg1)))
                                oname = "Object: *BAD VNUM*";
                        else
                                oname = obj->name;
                        if (!(room = get_room_index(pReset->arg3)))
                                rname = "Room: *BAD VNUM*";
                        else
                                rname = room->name;
                        snprintf(pbuf, MSL,
                                 "(object) %s (%d) -> %s (%d) [%d]\n\r",
                                 oname, pReset->arg1, rname, pReset->arg3,
                                 pReset->arg2);
                        if (!room)
                                obj = NULL;
                        lastobj = obj;
                        lo_reset = pReset;
                        break;
                case 'P':
                        if (!(obj = get_obj_index(pReset->arg1)))
                                oname = "Object1: *BAD VNUM*";
                        else
                                oname = obj->name;
                        obj2 = NULL;
                        if (pReset->arg3 > 0)
                        {
                                obj2 = get_obj_index(pReset->arg3);
                                rname = (obj2 ? obj2->
                                         name : "Object2: *BAD VNUM*");
                                lastobj = obj2;
                        }
                        else if (!lastobj)
                                rname = "Object2: *NULL obj*";
                        else if (pReset->extra == 0)
                        {
                                rname = lastobj->name;
                                obj2 = lastobj;
                        }
                        else
                        {
                                int       iNest;
                                ResetData *reset;

                                reset = lo_reset->next;
                                for (iNest = 0; iNest < pReset->extra;
                                     iNest++)
                                {
                                        for (; reset; reset = reset->next)
                                                if (reset->command == 'O'
                                                    || reset->command == 'G'
                                                    || reset->command == 'E'
                                                    || (reset->command == 'P'
                                                        && !reset->arg3
                                                        && reset->extra ==
                                                        iNest))
                                                        break;
                                        if (!reset || reset->command != 'P')
                                                break;
                                }
                                if (!reset)
                                        rname = "Object2: *BAD NESTING*";
                                else if (!(obj2 = get_obj_index(reset->arg1)))
                                        rname = "Object2: *NESTED BAD VNUM*";
                                else
                                        rname = obj2->name;
                        }
                        snprintf(pbuf, MSL,
                                 "(Put) %s (%d) -> %s (%d) [%d] {nest %d}\n\r",
                                 oname, pReset->arg1, rname,
                                 (obj2 ? obj2->vnum : pReset->arg3),
                                 pReset->arg2, pReset->extra);
                        break;
                case 'T':
                        snprintf(pbuf, MSL, "TRAP: %d %d %d %d (%s)\n\r",
                                 pReset->extra, pReset->arg1, pReset->arg2,
                                 pReset->arg3, flag_string(pReset->extra,
                                                           trap_flags));
                        break;
                case 'H':
                        if (pReset->arg1 > 0)
                                if (!(obj2 = get_obj_index(pReset->arg1)))
                                        rname = "Object: *BAD VNUM*";
                                else
                                        rname = obj2->name;
                        else if (!obj)
                                rname = "Object: *NULL obj*";
                        else
                                rname = oname;
                        snprintf(pbuf, MSL, "Hide %s (%d)\n\r", rname,
                                 (pReset->arg1 >
                                  0 ? pReset->arg1 : obj ? obj->vnum : 0));
                        break;
                case 'B':
                        {
                                char     *const *flagarray;

                                mudstrlcpy(pbuf, "BIT: ", MSL);
                                pbuf += 5;
                                if (IsSet(pReset->arg2, BitResetSet))
                                {
                                        mudstrlcpy(pbuf, "Set: ", 251);
                                        pbuf += 5;
                                }
                                else if (IsSet
                                         (pReset->arg2, BitResetToggle))
                                {
                                        mudstrlcpy(pbuf, "Toggle: ", 251);
                                        pbuf += 8;
                                }
                                else
                                {
                                        mudstrlcpy(pbuf, "Remove: ", 251);
                                        pbuf += 8;
                                }
                                switch (pReset->arg2 & BitResetTypeMask)
                                {
                                case BitResetDoor:
                                        {
                                                int       door;

                                                if (!
                                                    (room =
                                                     get_room_index(pReset->
                                                                    arg1)))
                                                        rname = "Room: *BAD VNUM*";
                                                else
                                                        rname = room->name;
                                                door = (pReset->
                                                        arg2 &
                                                        BitResetDoorMask)
                                                        >>
                                                        BitResetDoorThreshold;
                                                door = URange(0, door,
                                                              MaxDir + 1);
                                                snprintf(pbuf, MSL,
                                                         "Exit %s%s (%d), Room %s (%d)",
                                                         dir_name[door], (room
                                                                          &&
                                                                          get_exit
                                                                          (room,
                                                                           door)
                                                                          ? ""
                                                                          :
                                                                          " (NO EXIT!)"),
                                                         door, rname,
                                                         pReset->arg1);
                                        }
                                        flagarray = ex_flags;
                                        break;
                                case BitResetRoom:
                                        if (!
                                            (room =
                                             get_room_index(pReset->arg1)))
                                                rname = "Room: *BAD VNUM*";
                                        else
                                                rname = room->name;
                                        snprintf(pbuf, MSL, "Room %s (%d)",
                                                 rname, pReset->arg1);
                                        flagarray = r_flags;
                                        break;
                                case BitResetObject:
                                        if (pReset->arg1 > 0)
                                                if (!
                                                    (obj2 =
                                                     get_obj_index(pReset->
                                                                   arg1)))
                                                        rname = "Object: *BAD VNUM*";
                                                else
                                                        rname = obj2->name;
                                        else if (!obj)
                                                rname = "Object: *NULL obj*";
                                        else
                                                rname = oname;
                                        snprintf(pbuf, MSL, "Object %s (%d)",
                                                 rname,
                                                 (pReset->arg1 >
                                                  0 ? pReset->
                                                  arg1 : obj ? obj->
                                                  vnum : 0));
                                        flagarray = o_flags;
                                        break;
                                case BitResetMobile:
                                        if (pReset->arg1 > 0)
                                        {
                                                MobIndexData *mob2;

                                                if (!
                                                    (mob2 =
                                                     get_mob_index(pReset->
                                                                   arg1)))
                                                        rname = "Mobile: *BAD VNUM*";
                                                else
                                                        rname = mob2->
                                                                PlayerName;
                                        }
                                        else if (!mob)
                                                rname = "Mobile: *NULL mob*";
                                        else
                                                rname = mname;
                                        snprintf(pbuf, MSL, "Mobile %s (%d)",
                                                 rname,
                                                 (pReset->arg1 >
                                                  0 ? pReset->
                                                  arg1 : mob ? mob->
                                                  vnum : 0));
                                        flagarray = a_flags;
                                        break;
                                default:
                                        snprintf(pbuf, MSL, "bad type %d",
                                                 pReset->
                                                 arg2 & BitResetTypeMask);
                                        flagarray = NULL;
                                        break;
                                }
                                pbuf += strlen(pbuf);
                                if (flagarray)
                                        snprintf(pbuf, MSL,
                                                 "; flags: %s [%d]\n\r",
                                                 flag_string(pReset->arg3,
                                                             flagarray),
                                                 pReset->arg3);
                                else
                                        snprintf(pbuf, MSL, "; flags %d\n\r",
                                                 pReset->arg3);
                        }
                        break;
                case 'D':
                        {
                                char     *ef_name;

                                pReset->arg2 =
                                        URange(0, pReset->arg2, MaxDir + 1);
                                if (!(room = get_room_index(pReset->arg1)))
                                        rname = "Room: *BAD VNUM*";
                                else
                                        rname = room->name;
                                switch (pReset->arg3)
                                {
                                default:
                                        ef_name = "(* ERROR *)";
                                        break;
                                case 0:
                                        ef_name = "Open";
                                        break;
                                case 1:
                                        ef_name = "Close";
                                        break;
                                case 2:
                                        ef_name = "Close and lock";
                                        break;
                                }
                                snprintf(pbuf, MSL,
                                         "%s [%d] the %s%s [%d] door %s (%d)\n\r",
                                         ef_name, pReset->arg3,
                                         dir_name[pReset->arg2], (room
                                                                  &&
                                                                  get_exit
                                                                  (room,
                                                                   pReset->
                                                                   arg2) ? ""
                                                                  :
                                                                  " (NO EXIT!)"),
                                         pReset->arg2, rname, pReset->arg1);
                        }
                        break;
                case 'R':
                        if (!(room = get_room_index(pReset->arg1)))
                                rname = "Room: *BAD VNUM*";
                        else
                                rname = room->name;
                        snprintf(pbuf, MSL,
                                 "Randomize exits 0 to %d -> %s (%d)\n\r",
                                 pReset->arg2, rname, pReset->arg1);
                        break;
                }
                if (start == -1 || num >= start)
                        send_to_char(buf, ch);
                if (end != -1 && num >= end)
                        break;
        }
        if (num == 0)
                send_to_char("You don't have any resets defined.\n\r", ch);
        return;
}

/* Setup put nesting levels, regardless of whether or not the resets will
   actually reset, or if they're bugged. */
void renumber_put_resets(AreaData * pArea)
{
        ResetData *pReset, *lastobj = NULL;

        for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
        {
                switch (pReset->command)
                {
                default:
                        break;
                case 'G':
                case 'E':
                case 'O':
                        lastobj = pReset;
                        break;
                case 'P':
                        if (pReset->arg3 == 0)
                        {
                                if (!lastobj)
                                        pReset->extra = 1000000;
                                else if (lastobj->command != 'P'
                                         || lastobj->arg3 > 0)
                                        pReset->extra = 0;
                                else
                                        pReset->extra = lastobj->extra + 1;
                                lastobj = pReset;
                        }
                }
        }
        return;
}

/*
 * Create a new reset (for online building)			-Thoric
 */
ResetData *make_reset(char letter, int extra, int arg1, int arg2, int arg3)
{
        ResetData *pReset;

        CREATE(pReset, ResetData, 1);
        pReset->command = letter;
        pReset->extra = extra;
        pReset->arg1 = arg1;
        pReset->arg2 = arg2;
        pReset->arg3 = arg3;
        top_reset++;
        return pReset;
}

/*
 * Add a reset to an area				-Thoric
 */
ResetData *add_reset(AreaData * tarea, char letter, int extra, int arg1,
                      int arg2, int arg3)
{
        ResetData *pReset;

        if (!tarea)
        {
                bug("add_reset: NULL area!", 0);
                return NULL;
        }

        letter = Upper(letter);
        pReset = make_reset(letter, extra, arg1, arg2, arg3);
        switch (letter)
        {
        case 'M':
                tarea->last_mob_reset = pReset;
                break;
        case 'H':
                if (arg1 > 0)
                        break;
        case 'E':
        case 'G':
        case 'P':
        case 'O':
                tarea->last_obj_reset = pReset;
                break;
        case 'T':
                if (IsSet(extra, TrapObj) && arg1 == 0)
                        tarea->last_obj_reset = pReset;
                break;
        }

        LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
        return pReset;
}

/*
 * Place a reset into an area, insert sorting it		-Thoric
 */
ResetData *place_reset(AreaData * tarea, char letter, int extra, int arg1,
                        int arg2, int arg3)
{
        ResetData *pReset, *tmp, *tmp2;

        if (!tarea)
        {
                bug("place_reset: NULL area!", 0);
                return NULL;
        }

        letter = Upper(letter);
        pReset = make_reset(letter, extra, arg1, arg2, arg3);
        if (letter == 'M')
                tarea->last_mob_reset = pReset;

        if (tarea->first_reset)
        {
                switch (letter)
                {
                default:
                        bug("place_reset: Bad reset type %c", letter);
                        return NULL;
                case 'D':
                case 'R':
                        for (tmp = tarea->last_reset; tmp; tmp = tmp->prev)
                                if (tmp->command == letter)
                                        break;
                        if (tmp)    /* organize by location */
                                for (;
                                     tmp && tmp->command == letter
                                     && tmp->arg1 > arg1; tmp = tmp->prev);
                        if (tmp)    /* organize by direction */
                                for (;
                                     tmp && tmp->command == letter
                                     && tmp->arg1 == tmp->arg1
                                     && tmp->arg2 > arg2; tmp = tmp->prev);
                        if (tmp)
                                INSERT(pReset, tmp, tarea->first_reset, next,
                                       prev);
                        else
                                LINK(pReset, tarea->first_reset,
                                     tarea->last_reset, next, prev);
                        return pReset;
                case 'M':
                case 'O':
                        /*
                         * find last reset of same type 
                         */
                        for (tmp = tarea->last_reset; tmp; tmp = tmp->prev)
                                if (tmp->command == letter)
                                        break;
                        tmp2 = tmp ? tmp->next : NULL;
                        /*
                         * organize by location 
                         */
                        for (; tmp; tmp = tmp->prev)
                                if (tmp->command == letter
                                    && tmp->arg3 <= arg3)
                                {
                                        tmp2 = tmp->next;
                                        /*
                                         * organize by vnum 
                                         */
                                        if (tmp->arg3 == arg3)
                                                for (; tmp; tmp = tmp->prev)
                                                        if (tmp->command ==
                                                            letter
                                                            && tmp->arg3 ==
                                                            tmp->arg3
                                                            && tmp->arg1 <=
                                                            arg1)
                                                        {
                                                                tmp2 = tmp->
                                                                        next;
                                                                break;
                                                        }
                                        break;
                                }
                        /*
                         * skip over E or G for that mob 
                         */
                        if (tmp2 && letter == 'M')
                        {
                                for (; tmp2; tmp2 = tmp2->next)
                                        if (tmp2->command != 'E'
                                            && tmp2->command != 'G')
                                                break;
                        }
                        else
                                /*
                                 * skip over P, T or H for that obj 
                                 */
                        if (tmp2 && letter == 'O')
                        {
                                for (; tmp2; tmp2 = tmp2->next)
                                        if (tmp2->command != 'P'
                                            && tmp2->command != 'T'
                                            && tmp2->command != 'H')
                                                break;
                        }
                        if (tmp2)
                                INSERT(pReset, tmp2, tarea->first_reset, next,
                                       prev);
                        else
                                LINK(pReset, tarea->first_reset,
                                     tarea->last_reset, next, prev);
                        return pReset;
                case 'G':
                case 'E':
                        /*
                         * find the last mob 
                         */
                        if ((tmp = tarea->last_mob_reset) != NULL)
                        {
                                /*
                                 * See if there are any resets for this mob yet,
                                 * put E before G and organize by vnum
                                 */
                                if (tmp->next)
                                {
                                        tmp = tmp->next;
                                        if (tmp && tmp->command == 'E')
                                        {
                                                if (letter == 'E')
                                                        for (;
                                                             tmp
                                                             && tmp->
                                                             command == 'E'
                                                             && tmp->arg1 <
                                                             arg1;
                                                             tmp = tmp->next);
                                                else
                                                        for (;
                                                             tmp
                                                             && tmp->
                                                             command == 'E';
                                                             tmp = tmp->next);
                                        }
                                        else if (tmp && tmp->command == 'G'
                                                 && letter == 'G')
                                                for (;
                                                     tmp
                                                     && tmp->command == 'G'
                                                     && tmp->arg1 < arg1;
                                                     tmp = tmp->next);
                                        if (tmp)
                                                INSERT(pReset, tmp,
                                                       tarea->first_reset,
                                                       next, prev);
                                        else
                                                LINK(pReset,
                                                     tarea->first_reset,
                                                     tarea->last_reset, next,
                                                     prev);
                                }
                                else
                                        LINK(pReset, tarea->first_reset,
                                             tarea->last_reset, next, prev);
                                return pReset;
                        }
                        break;
                case 'P':
                case 'T':
                case 'H':
                        /*
                         * find the object in question 
                         */
                        if (((letter == 'P' && arg3 == 0)
                             || (letter == 'T' && IsSet(extra, TrapObj)
                                 && arg1 == 0) || (letter == 'H'
                                                   && arg1 == 0))
                            && (tmp = tarea->last_obj_reset) != NULL)
                        {
                                if ((tmp = tmp->next) != NULL)
                                        INSERT(pReset, tmp,
                                               tarea->first_reset, next,
                                               prev);
                                else
                                        LINK(pReset, tarea->first_reset,
                                             tarea->last_reset, next, prev);
                                return pReset;
                        }

                        for (tmp = tarea->last_reset; tmp; tmp = tmp->prev)
                                if ((tmp->command == 'O'
                                     || tmp->command == 'G'
                                     || tmp->command == 'E'
                                     || tmp->command == 'P')
                                    && tmp->arg1 == arg3)
                                {
                                        /*
                                         * See if there are any resets for this object yet,
                                         * put P before H before T and organize by vnum
                                         */
                                        if (tmp->next)
                                        {
                                                tmp = tmp->next;
                                                if (tmp
                                                    && tmp->command == 'P')
                                                {
                                                        if (letter == 'P'
                                                            && tmp->arg3 ==
                                                            arg3)
                                                                for (;
                                                                     tmp
                                                                     && tmp->
                                                                     command
                                                                     == 'P'
                                                                     && tmp->
                                                                     arg3 ==
                                                                     arg3
                                                                     && tmp->
                                                                     arg1 <
                                                                     arg1;
                                                                     tmp =
                                                                     tmp->
                                                                     next);
                                                        else if (letter !=
                                                                 'T')
                                                                for (;
                                                                     tmp
                                                                     && tmp->
                                                                     command
                                                                     == 'P'
                                                                     && tmp->
                                                                     arg3 ==
                                                                     arg3;
                                                                     tmp =
                                                                     tmp->
                                                                     next);
                                                }
                                                else if (tmp
                                                         && tmp->command ==
                                                         'H')
                                                {
                                                        if (letter == 'H'
                                                            && tmp->arg3 ==
                                                            arg3)
                                                                for (;
                                                                     tmp
                                                                     && tmp->
                                                                     command
                                                                     == 'H'
                                                                     && tmp->
                                                                     arg3 ==
                                                                     arg3
                                                                     && tmp->
                                                                     arg1 <
                                                                     arg1;
                                                                     tmp =
                                                                     tmp->
                                                                     next);
                                                        else if (letter !=
                                                                 'H')
                                                                for (;
                                                                     tmp
                                                                     && tmp->
                                                                     command
                                                                     == 'H'
                                                                     && tmp->
                                                                     arg3 ==
                                                                     arg3;
                                                                     tmp =
                                                                     tmp->
                                                                     next);
                                                }
                                                else if (tmp
                                                         && tmp->command ==
                                                         'T' && letter == 'T')
                                                        for (;
                                                             tmp
                                                             && tmp->
                                                             command == 'T'
                                                             && tmp->arg3 ==
                                                             arg3
                                                             && tmp->arg1 <
                                                             arg1;
                                                             tmp = tmp->next);
                                                if (tmp)
                                                        INSERT(pReset, tmp,
                                                               tarea->
                                                               first_reset,
                                                               next, prev);
                                                else
                                                        LINK(pReset,
                                                             tarea->
                                                             first_reset,
                                                             tarea->
                                                             last_reset, next,
                                                             prev);
                                        }
                                        else
                                                LINK(pReset,
                                                     tarea->first_reset,
                                                     tarea->last_reset, next,
                                                     prev);
                                        return pReset;
                                }
                        break;
                }
                /*
                 * likely a bad reset if we get here... add it anyways 
                 */
        }
        LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
        return pReset;
}
