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
 * Player tracking system for following movement trails and pathfinding.                *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"

#define BfsError	   -1
#define BfsAlreadyThere  -2
#define BfsNoPath	   -3
#define BfsMark         BV01

#define TrackThroughDoors

extern int top_room;

bool      mob_snipe(CharData * ch, CharData * victim);
ch_ret one_hit args((CharData * ch, CharData * victim, int dt));
RoomIndexData *generate_exit(RoomIndexData * in_room, ExitData ** pexit);

/* You can define or not define TrackThoughDoors, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

struct bfs_queue_struct
{
        RoomIndexData *room;
        char      dir;
        struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = NULL,
        *queue_tail = NULL, *room_queue = NULL;

/* Utility macros */
#define MARK(room)	(SetBit(	(room)->RoomFlags, BfsMark) )
#define UNMARK(room)	(RemoveBit(	(room)->RoomFlags, BfsMark) )
#define IsMarked(room)	(IsSet(	(room)->RoomFlags, BfsMark) )

RoomIndexData *toroom(RoomIndexData * room, sh_int door)
{
        return (get_exit(room, door)->to_room);
}

bool valid_edge(RoomIndexData * room, sh_int door)
{
        ExitData *pexit;
        RoomIndexData *to_room;

        pexit = get_exit(room, door);
        if (pexit && (to_room = pexit->to_room) != NULL
#ifndef TrackThroughDoors
            && !IsSet(pexit->exit_info, ExClosed)
#endif
            && !IsMarked(to_room))
                return TRUE;
        else
                return FALSE;
}

void bfs_enqueue(RoomIndexData * room, char dir)
{
        struct bfs_queue_struct *curr;

        CREATE(curr, struct bfs_queue_struct, 1);

        curr->room = room;
        curr->dir = dir;
        curr->next = NULL;

        if (queue_tail)
        {
                queue_tail->next = curr;
                queue_tail = curr;
        }
        else
                queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
        struct bfs_queue_struct *curr;

        curr = queue_head;

        if (!(queue_head = queue_head->next))
                queue_tail = NULL;
        DISPOSE(curr);
}


void bfs_clear_queue(void)
{
        while (queue_head)
                bfs_dequeue();
}

void room_enqueue(RoomIndexData * room)
{
        struct bfs_queue_struct *curr;

        CREATE(curr, struct bfs_queue_struct, 1);

        curr->room = room;
        curr->next = room_queue;

        room_queue = curr;
}

void clean_room_queue(void)
{
        struct bfs_queue_struct *curr, *curr_next;

        for (curr = room_queue; curr; curr = curr_next)
        {
                UNMARK(curr->room);
                curr_next = curr->next;
                DISPOSE(curr);
        }
        room_queue = NULL;
}


int find_first_step(RoomIndexData * src, RoomIndexData * target,
                    int maxdist)
{
        int       curr_dir, count;

        if (!src || !target)
        {
                bug("Illegal value passed to find_first_step (track.c)", 0);
                return BfsError;
        }

        if (src == target)
                return BfsAlreadyThere;

        if (src->area != target->area)
                return BfsNoPath;

        room_enqueue(src);
        MARK(src);

        /*
         * first, enqueue the first steps, saving which direction we're going. 
         */
        for (curr_dir = 0; curr_dir < 10; curr_dir++)
                if (valid_edge(src, curr_dir))
                {
                        MARK(toroom(src, curr_dir));
                        room_enqueue(toroom(src, curr_dir));
                        bfs_enqueue(toroom(src, curr_dir), curr_dir);
                }

        count = 0;
        while (queue_head)
        {
                if (++count > maxdist)
                {
                        bfs_clear_queue();
                        clean_room_queue();
                        return BfsNoPath;
                }
                if (queue_head->room == target)
                {
                        curr_dir = queue_head->dir;
                        bfs_clear_queue();
                        clean_room_queue();
                        return curr_dir;
                }
                else
                {
                        for (curr_dir = 0; curr_dir < 10; curr_dir++)
                                if (valid_edge(queue_head->room, curr_dir))
                                {
                                        MARK(toroom
                                             (queue_head->room, curr_dir));
                                        room_enqueue(toroom
                                                     (queue_head->room,
                                                      curr_dir));
                                        bfs_enqueue(toroom
                                                    (queue_head->room,
                                                     curr_dir),
                                                    queue_head->dir);
                                }
                        bfs_dequeue();
                }
        }
        clean_room_queue();

        return BfsNoPath;
}


CMDF do_track(CharData * ch, char *argument)
{
        CharData *vict;
        char      arg[MaxInputLength];
        char      buf[MaxStringLength];
        int       dir, maxdist;

        if (!IsNpc(ch) && !ch->pcdata->learned[gsn_track])
        {
                send_to_char("You do not know of this skill yet.\n\r", ch);
                return;
        }

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Whom are you trying to track?\n\r", ch);
                return;
        }

        WaitState(ch, skill_table[gsn_track]->beats);

        if (!(vict = get_char_world(ch, arg)))
        {
                send_to_char
                        ("You can't find a trail of anyone like that.\n\r",
                         ch);
                return;
        }

        maxdist = 100 + ch->top_level * 30;

        if (!IsNpc(ch))
                maxdist = (maxdist * ch->pcdata->learned[gsn_track]) / 100;

        dir = find_first_step(ch->in_room, vict->in_room, maxdist);
        switch (dir)
        {
        case BfsError:
                send_to_char("Hmm... something seems to be wrong.\n\r", ch);
                break;
        case BfsAlreadyThere:
                send_to_char("You're already in the same room!\n\r", ch);
                break;
        case BfsNoPath:
                snprintf(buf, MSL, "You can't sense a trail from here.\n\r");
                send_to_char(buf, ch);
                learn_from_failure(ch, gsn_track);
                break;
        default:
                ch_printf(ch, "You sense a trail %s from here...\n\r",
                          dir_name[dir]);
                learn_from_success(ch, gsn_track);
                break;
        }
}


void found_prey(CharData * ch, CharData * victim)
{
        char      buf[MaxStringLength];
        char      victname[MaxStringLength];



        if (victim == NULL)
        {
                bug("Found_prey: null victim", 0);
                return;
        }

        if (ch == NULL)
        {
                bug("Found_prey: null ch", 0);
                return;
        }

        if (victim->in_room == NULL)
        {
                bug("Found_prey: null victim->in_room", 0);
                return;
        }

        snprintf(victname, MSL, "%s",
                 IsNpc(victim) ? victim->short_descr : victim->name);

        if (!can_see(ch, victim))
        {
                if (number_percent() < 90)
                        return;
                switch (number_bits(2))
                {
                case 0:
                        {
                            /* Use stacked buffer approach to avoid truncation warnings */
                            char tmp[MSL*2]; /* Temporary buffer large enough for the format operation */
                            snprintf(tmp, sizeof(tmp), "Don't make me find you, %s!", victname);
                            tmp[sizeof(tmp)-1] = '\0'; /* Ensure null termination */
                            strncpy(buf, tmp, MSL-1);
                            buf[MSL-1] = '\0'; /* Ensure null termination */
                            do_say(ch, buf);
                        }
                        break;
                case 1:
                        act(AtAction, "$n sniffs around the room for $N.",
                            ch, NULL, victim, ToNotvict);
                        act(AtAction, "You sniff around the room for $N.",
                            ch, NULL, victim, ToChar);
                        act(AtAction, "$n sniffs around the room for you.",
                            ch, NULL, victim, ToVict);
                        snprintf(buf, MSL, "I can smell your blood!");
                        do_say(ch, buf);
                        break;
                case 2:
                        command_printf(ch, "yell I'm going to teat %s apart!",
                                       victname);
                        break;
                case 3:
                        do_say(ch, "Just wait until I find you...");
                        break;
                }
                return;
        }

        if (IsSet(ch->in_room->RoomFlags, RoomSafe))
        {
                if (number_percent() < 90)
                        return;
                switch (number_bits(2))
                {
                case 0:
                        do_say(ch, "C'mon out, you coward!");
                        command_printf(ch, "yell %s is a bloody coward!",
                                       victname);
                        break;
                case 1:
                        {
                            /* Use stacked buffer approach to avoid truncation warnings */
                            char tmp[MSL*2]; /* Temporary buffer large enough for the format operation */
                            snprintf(tmp, sizeof(tmp), "Let's take this outside, %s", victname);
                            tmp[sizeof(tmp)-1] = '\0'; /* Ensure null termination */
                            strncpy(buf, tmp, MSL-1);
                            buf[MSL-1] = '\0'; /* Ensure null termination */
                            do_say(ch, buf);
                        }
                        break;
                case 2:
                        command_printf(ch,
                                       "yell %s is a yellow bellied wimp!",
                                       victname);
                        break;
                case 3:
                        act(AtAction, "$n takes a few swipes at $N.", ch,
                            NULL, victim, ToNotvict);
                        act(AtAction, "You try to take a few swipes $N.", ch,
                            NULL, victim, ToChar);
                        act(AtAction, "$n takes a few swipes at you.", ch,
                            NULL, victim, ToVict);
                        break;
                }
                return;
        }

        switch (number_bits(2))
        {
        case 0:
                command_printf(ch, "yell You blood is mine %s!", victname);
                break;
        case 1:
                {
                    /* Use stacked buffer approach to avoid truncation warnings */
                    char tmp[MSL*2]; /* Temporary buffer large enough for the format operation */
                    snprintf(tmp, sizeof(tmp), "Alas, we meet again, %s!", victname);
                    tmp[sizeof(tmp)-1] = '\0'; /* Ensure null termination */
                    strncpy(buf, tmp, MSL-1);
                    buf[MSL-1] = '\0'; /* Ensure null termination */
                    do_say(ch, buf);
                }
                break;
        case 2:
                {
                    /* Use stacked buffer approach to avoid truncation warnings */
                    char tmp[MSL*2]; /* Temporary buffer large enough for the format operation */
                    snprintf(tmp, sizeof(tmp), "What do you want on your tombstone, %s?", victname);
                    tmp[sizeof(tmp)-1] = '\0'; /* Ensure null termination */
                    strncpy(buf, tmp, MSL-1);
                    buf[MSL-1] = '\0'; /* Ensure null termination */
                    do_say(ch, buf);
                }
                break;
        case 3:
                act(AtAction, "$n lunges at $N from out of nowhere!", ch,
                    NULL, victim, ToNotvict);
                act(AtAction, "You lunge at $N catching $M off guard!", ch,
                    NULL, victim, ToChar);
                act(AtAction, "$n lunges at you from out of nowhere!", ch,
                    NULL, victim, ToVict);
        }
        stop_hunting(ch);
        set_fighting(ch, victim);
        multi_hit(ch, victim, TypeUndefined);
        return;
}

void hunt_victim(CharData * ch)
{
        bool      found;
        CharData *tmp;
        sh_int    ret;

        if (!ch || !ch->hunting || !ch->hunting->who)
                return;

        /*
         * make sure the char still exists 
         */
        for (found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next)
                if (ch->hunting->who == tmp)
                        found = TRUE;

        if (!found)
        {
                do_say(ch, "Damn!  My prey is gone!!");
                stop_hunting(ch);
                return;
        }

        if ((ch->in_room == ch->hunting->who->in_room) && (ch->position == 8))
        {
                if (ch->fighting)
                        return;
                found_prey(ch, ch->hunting->who);
                return;
        }

/* hunting with snipe */
        {
                ObjData *wield;

                wield = get_eq_char(ch, WearWield);
                if (wield != NULL && wield->value[3] == WeaponBlaster)
                {
                        if (mob_snipe(ch, ch->hunting->who) == TRUE)
                                return;
                }
                else if (!IsSet(ch->act, ActDroid))
                        do_hide(ch, "");
        }

        ret = find_first_step(ch->in_room, ch->hunting->who->in_room, 5000);
        if (ret == BfsNoPath)
        {
                ExitData *pexit;
                int       attempt;

                for (attempt = 0; attempt < 25; attempt++)
                {
                        ret = number_door();
                        if ((pexit = get_exit(ch->in_room, ret)) == NULL
                            || !pexit->to_room
                            || IsSet(pexit->exit_info, ExClosed)
                            || IsSet(pexit->to_room->RoomFlags,
                                       RoomNoMob))
                                continue;
                }
        }
        if (ret < 0)
        {
                do_say(ch, "Damn!  Lost my prey!");
                stop_hunting(ch);
                return;
        }
        else
        {
                move_char(ch, get_exit(ch->in_room, ret), FALSE, FALSE);
                if (char_died(ch))
                        return;
                if (!ch->hunting)
                {
                        if (!ch->in_room)
                        {
                                char      buf[MaxStringLength];

                                snprintf(buf, MSL,
                                         "Hunt_victim: no ch->in_room!  Mob #%d, name: %s.  Placing mob in limbo.",
                                         ch->pIndexData->vnum, ch->name);
                                bug(buf, 0);
                                char_to_room(ch,
                                             get_room_index(RoomVnumLimbo));
                                return;
                        }
                        do_say(ch, "Damn!  Lost my prey!");
                        return;
                }
                if ((ch->in_room == ch->hunting->who->in_room)
                    && (ch->position == 8))
                        found_prey(ch, ch->hunting->who);
                return;
        }
}

bool mob_snipe(CharData * ch, CharData * victim)
{
        sh_int    dir, dist;
        sh_int    max_dist = 3;
        ExitData *pexit;
        RoomIndexData *was_in_room;
        RoomIndexData *to_room;
        char      buf[MaxStringLength];
        bool      pfound = FALSE;

        if (!ch->in_room || !victim->in_room)
                return FALSE;

        if (IsSet(ch->in_room->RoomFlags, RoomSafe))
                return FALSE;

        for (dir = 0; dir <= 10; dir++)
        {
                if ((pexit = get_exit(ch->in_room, dir)) == NULL)
                        continue;

                if (IsSet(pexit->exit_info, ExClosed))
                        continue;

                was_in_room = ch->in_room;

                for (dist = 0; dist <= max_dist; dist++)
                {
                        if (IsSet(pexit->exit_info, ExClosed))
                                break;

                        if (!pexit->to_room)
                                break;

                        to_room = NULL;
                        if (pexit->distance > 1)
                                to_room = generate_exit(ch->in_room, &pexit);

                        if (to_room == NULL)
                                to_room = pexit->to_room;

                        char_from_room(ch);
                        char_to_room(ch, to_room);


                        if (ch->in_room == victim->in_room)
                        {
                                pfound = TRUE;
                                break;
                        }

                        if ((pexit = get_exit(ch->in_room, dir)) == NULL)
                                break;

                }

                char_from_room(ch);
                char_to_room(ch, was_in_room);

                if (!pfound)
                {
                        char_from_room(ch);
                        char_to_room(ch, was_in_room);
                        continue;
                }

                if (IsSet(victim->in_room->RoomFlags, RoomSafe))
                        return FALSE;

                if (is_safe(ch, victim))
                        return FALSE;

                if (IsAffected(ch, AffCharm) && ch->master == victim)
                        return FALSE;

                if (ch->position == PosFighting)
                        return FALSE;

                switch (dir)
                {
                case 0:
                case 1:
                        dir += 2;
                        break;
                case 2:
                case 3:
                        dir -= 2;
                        break;
                case 4:
                case 7:
                        dir += 1;
                        break;
                case 5:
                case 8:
                        dir -= 1;
                        break;
                case 6:
                        dir += 3;
                        break;
                case 9:
                        dir -= 3;
                        break;
                }

                char_from_room(ch);
                char_to_room(ch, victim->in_room);

                snprintf(buf, MSL, "A blaster shot fires at you from the %s.",
                         dir_name[dir]);
                act(AtAction, buf, victim, NULL, ch, ToChar);
                act(AtAction, "You fire at $N.", ch, NULL, victim, ToChar);
                snprintf(buf, MSL, "A blaster shot fires at $N from the %s.",
                         dir_name[dir]);
                act(AtAction, buf, ch, NULL, victim, ToNotvict);

                one_hit(ch, victim, TypeUndefined);

                if (char_died(ch))
                        return TRUE;

                stop_fighting(ch, TRUE);

                if (victim && !char_died(victim) && victim->hit < 0)
                {
                        stop_hunting(ch);
                        stop_hating(ch);
                }

                char_from_room(ch);
                char_to_room(ch, was_in_room);

                return TRUE;
        }

        return FALSE;
}
