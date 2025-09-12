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
 *                            Movement and Actions Module                                *
 ****************************************************************************************/
// ============================================================================
// System Headers
// ============================================================================
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// ============================================================================
// Local Headers
// ============================================================================
#include "mud.hpp"
#include "msp.hpp"

// ============================================================================
// Constants and Configuration
// ============================================================================
namespace {
    // Virtual room constants
    constexpr int VroomHashSize = 64;
    
    // Text formatting constants
    constexpr int MaxWordLength = 255;
    constexpr int WordwrapWidth = 78;
    
    // Movement and endurance thresholds
    constexpr int MinBashEndurance = 15;
    constexpr int MinStruggleEndurance = 100;
    constexpr int MinSubdueEndurance = 80;
    constexpr int FallDamageThreshold = 80;
    constexpr double EncumbranceThreshold = 0.95;
    
    // Wait state timers
    constexpr int BindWaitState = 2;
    constexpr int ReleaseWaitState = 4;
    constexpr int StruggleWaitMin = 2;
    constexpr int StruggleWaitMax = 7;
    
    // Auto-description limits
    constexpr int MaxRoomDescriptions = 8;
}

// ============================================================================
// Function Prototypes
// ============================================================================
char     *grab_word(char *argument, char *arg_first);
void      decorate_room(RoomIndexData * room);
RoomIndexData *generate_exit(RoomIndexData * in_room, ExitData ** pexit);
void      toggle_bexit_flag(ExitData * pexit, int flag);
void      remove_bexit_flag(ExitData * pexit, int flag);

// ============================================================================
// Movement and Direction Data
// ============================================================================
const sh_int movement_loss[SectMax] = {
        1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 5, 7, 4
};

const char *const dir_name[] = {
        "north", "east", "south", "west", "up", "down",
        "northeast", "northwest", "southeast", "southwest", "somewhere"
};

const int trap_door[] = {
        TrapN, TrapE, TrapS, TrapW, TrapU, TrapD,
        TrapNe, TrapNw, TrapSe, TrapSw
};

const sh_int rev_dir[] = {
        2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};

RoomIndexData *vroom_hash[VroomHashSize];

/*
 * Local functions.
 */
bool has_key(CharData * ch, int key);


// ============================================================================
// Sector and Room Description Data
// ============================================================================
const char *const sect_names[SectMax][2] = {
        {"In a room", "inside"}, {"A City Street", "cities"},
        {"In a field", "fields"}, {"In a forest", "forests"},
        {"hill", "hills"}, {"On a mountain", "mountains"},
        {"In the water", "waters"}, {"In rough water", "waters"},
        {"Underwater", "underwaters"}, {"In the air", "air"},
        {"In a desert", "deserts"}, {"Somewhere", "unknown"},
        {"ocean floor", "ocean floor"}, {"underground", "underground"},
        {"On a Starship", "starship"}
};

const int sent_total[SectMax] = {
        4, 24, 4, 4, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1
};

const char *const room_sents[SectMax][25] = {
        {
         "The room walls are made of durasteel and duraplast.",
         "You can smell the fumes of ships and vehicles wafting in from outside.",
         "You notice signs of a recent battle from the bloodstains on the floor.",
         "The ceiling is set from a mold of duraplast.",
         "A few people can be heard near by.",
         "The edge of the room is a little cooler than the other side."},
        {
         "You notice the occasional stray looking for food.",
         "Tall buildings loom on either side of you stretching to the sky.",
         "Some street people are putting on an interesting display of talent trying to earn some credits.",
         "Two people nearby shout heated words of argument at one another.",
         "You think you can make out several shady figures talking down a dark alleyway.",
         "A slight breeze blows through the tall buildings.",
         "A small crowd of people have gathered at one side of the street.",
         "Clouds far above you obscure the tops of the highest skyscrapers.",
         "A speeder moves slowly through the street avoiding pedestrians.",
         "A cloudcar flys by overhead.",
         "The air is thick and hard to breath.",
         "The many smells of the city assault your senses.",
         "You hear a scream far of in the distance.",
         "The buildings around you seem endless in number.",
         "The city stretches seemingly endless in all directions.",
         "The street is wide and long.",
         "A swoop rider passes quickly by weaving in and out of pedestrians and other vehicles.",
         "The surface of the road is worn from many travellers.",
         "You feel it would be very easy to get lost in such an enormous city.",
         "You can see other streets above and bellow this one running in many directions.",
         "There are entrances to several buildings at this level.",
         "Along the edge of the street railings prevent pedestrians from falling to their death.",
         "In between the many towers you can see down into depths of the lower city.",
         "A grate in the street prevents rainwater from building up.",
         "You can see you reflection in several of the transparisteel windows as you pass by."
         "You hear a scream far of in the distance."},

        {
         "You notice sparce patches of brush and shrubs.",
         "There is a small cluster of trees far off in the distance.",
         "Around you are grassy fields as far as the eye can see.",
         "Throughout the plains a wide variety of weeds and wildflowers are scattered."},

        {
         "Tall, dark evergreens prevent you from seeing very far.",
         "Many huge oak trees that look several hundred years old are here.",
         "You notice a solitary lonely weeping willow.",
         "To your left is a patch of bright white birch trees, slender and tall."},

        {
         "The rolling hills are lightly speckled with violet wildflowers.",
         "Slowly rolling grassy ridges march off into the horizon.",
         "Sparse thickets of small trees dot the landscape ocasionally.",
         "Once in awhile you hear the soft noise of the wind moving over the hills."},

        {
         "The rocky mountain pass offers many hiding places.",
         "Grey-green rocks jut up towards the sky, where sparse cloud-cover caps the towering peaks.",
         "A large canyon stretches away, where at some time in the past a deep, fast river flowed."},

        {
         "The water is smooth as glass."},

        {
         "Rough waves splash about angrily."},

        {
         "A small school of fish swims by."},

        {
         "The land is far far below.",
         "A misty haze of clouds drifts by."},

        {
         "Around you is sand as far as the eye can see.",
         "You think you see an oasis far in the distance."},

        {
         "You notice nothing unusual."},

        {
         "There are many rocks and coral which litter the ocean floor."},

        {
         "You stand in a lengthy tunnel of rock."}
        ,
        {
         "The starship interior is done in basic durasteel."}

};

// ============================================================================
// Utility Functions
// ============================================================================

int wherehome(CharData * ch)
{

        if (ch->PCData && ch->PCData->recall != 0)
                return ch->PCData->recall;

        if (ch->plr_home)
                return ch->plr_home->vnum;

        if (get_trust(ch) >= LevelImmortal)
                return RoomStartImmortal;
        return RoomVnumTemple;
}

char     *grab_word(char *argument, char *arg_first)
{
        char      cEnd;
        sh_int    count;

        count = 0;

        while (isspace(*argument))
                argument++;

        cEnd = ' ';
        if (*argument == '\'' || *argument == '"')
                cEnd = *argument++;

        while (*argument != '\0' && ++count < MaxWordLength)
        {
                if (*argument == cEnd)
                {
                        argument++;
                        break;
                }
                *arg_first++ = *argument++;
        }
        *arg_first = '\0';

        while (isspace(*argument))
                argument++;

        return argument;
}

char     *smash_newline(const char *str)
{
        static char ret[MaxStringLength];
        char     *retptr;

        retptr = ret;
        for (; *str != '\0'; str++)
        {
                if (*str == '\n' || *str == '\r')
                {
                        *retptr = ' ';
                        retptr++;
                }
                else
                {
                        *retptr = *str;
                        retptr++;
                }
        }
        *retptr = '\0';
        return ret;
}

char     *wordwrap(char *txt, sh_int wrap)
{
        static char buf[MaxStringLength];
        char     *bufp;

        buf[0] = '\0';
        bufp = buf;
        if (txt != NULL)
        {
                char      line[MaxStringLength];
                char      temp[MaxStringLength];
                char     *ptr, *p;
                int       ln, x;

                ++bufp;
                line[0] = '\0';
                ptr = smash_newline(txt);
                while (*ptr)
                {
                        ptr = grab_word(ptr, temp);
                        ln = static_cast<int>(strlen(line));
                        x = static_cast<int>(strlen(temp));
                        if ((ln + x + 1) < wrap)
                        {
                                if (line[ln - 1] == '.')
                                        mudstrlcat(line, "  ", MSL);
                                else
                                        mudstrlcat(line, " ", MSL);
                                mudstrlcat(line, temp, MSL);
                                p = strchr(line, '\n');
                                if (!p)
                                        p = strchr(line, '\r');
                                if (p)
                                {
                                        mudstrlcat(buf, line, MSL);
                                        line[0] = '\0';
                                }
                        }
                        else
                        {
                                mudstrlcat(line, "\r\n", MSL);
                                mudstrlcat(buf, line, MSL);
                                mudstrlcpy(line, temp, MSL);
                        }
                }
                if (line[0] != '\0')
                        mudstrlcat(buf, line, MSL);
        }
        return bufp;
}


void decorate_room(RoomIndexData * room)
{
        char      buf[MaxStringLength];
        char      buf2[MaxStringLength];
        int       nRand;
        int       iRand, len;
        int       previous[8];
        int       sector = room->sector_type;

        if (room->name)
                STRFREE(room->name);
        if (room->description)
                STRFREE(room->description);

        room->name = STRALLOC(const_cast<char*>(sect_names[sector][0]));
        buf[0] = '\0';
        nRand = number_range(1, UMin(MaxRoomDescriptions, sent_total[sector]));

        for (iRand = 0; iRand < nRand; iRand++)
                previous[iRand] = -1;

        for (iRand = 0; iRand < nRand; iRand++)
        {
                while (previous[iRand] == -1)
                {
                        int       x, z;

                        x = number_range(0, sent_total[sector] - 1);

                        for (z = 0; z < iRand; z++)
                                if (previous[z] == x)
                                        break;

                        if (z < iRand)
                                continue;

                        previous[iRand] = x;

                        len = static_cast<int>(strlen(buf));
                        snprintf(buf2, MSL, "%s", room_sents[sector][x]);
                        if (len > 5 && buf[len - 1] == '.')
                        {
                                mudstrlcat(buf, "  ", MSL);
                                buf2[0] = Upper(buf2[0]);
                        }
                        else if (len == 0)
                                buf2[0] = Upper(buf2[0]);
                        mudstrlcat(buf, buf2, MSL);
                }
        }
        snprintf(buf2, MSL, "%s\n\r", wordwrap(buf, WordwrapWidth));
        room->description = STRALLOC(buf2);
}

// ============================================================================
// Room Description and Auto-Generation Functions
// ============================================================================

CMDF do_autodescription(CharData * ch, [[maybe_unused]] const char *argument)
{
        char      buf[MaxStringLength];
        char      buf2[MaxStringLength];
        char      arg[MaxStringLength];
        int       nRand;
        int       iRand, len;
        int       previous[MaxRoomDescriptions];
        int       sector;
        RoomIndexData *room;

        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
                room = ch->in_room;
        else if ((room = get_room_index(atoi(arg))) == NULL)
        {
                send_to_char("That is not a Valid room", ch);
                return;
        }

        sector = room->sector_type;

        if (room->description)
                STRFREE(room->description);


        buf[0] = '\0';
        nRand = number_range(1, UMin(MaxRoomDescriptions, sent_total[sector]));

        for (iRand = 0; iRand < nRand; iRand++)
                previous[iRand] = -1;

        for (iRand = 0; iRand < nRand; iRand++)
        {
                while (previous[iRand] == -1)
                {
                        int       x, z;

                        x = number_range(0, sent_total[sector] - 1);

                        for (z = 0; z < iRand; z++)
                                if (previous[z] == x)
                                        break;

                        if (z < iRand)
                                continue;

                        previous[iRand] = x;

                        len = static_cast<int>(strlen(buf));
                        snprintf(buf2, MSL, "%s", room_sents[sector][x]);
                        if (len > 5 && buf[len - 1] == '.')
                        {
                                mudstrlcat(buf, "  ", MSL);
                                buf2[0] = Upper(buf2[0]);
                        }
                        else if (len == 0)
                                buf2[0] = Upper(buf2[0]);
                        mudstrlcat(buf, buf2, MSL);
                }
        }
        snprintf(buf2, MSL, "%s\n\r", wordwrap(buf, WordwrapWidth));
        room->description = STRALLOC(buf2);
}

// ============================================================================
// Virtual Room Management Functions
// ============================================================================

/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms()
{
        int       hash;
        RoomIndexData *room, *room_next, *prev;

        for (hash = static_cast<sh_int>(0); hash < VroomHashSize; hash++)
        {
                while (vroom_hash[hash]
                       && !vroom_hash[hash]->first_person
                       && !vroom_hash[hash]->first_content)
                {
                        room = vroom_hash[hash];
                        vroom_hash[hash] = room->next;
                        clean_room(room);
                        DISPOSE(room);
                        --top_vroom;
                }
                prev = NULL;
                for (room = vroom_hash[hash]; room; room = room_next)
                {
                        room_next = room->next;
                        if (!room->first_person && !room->first_content)
                        {
                                if (prev)
                                        prev->next = room_next;
                                clean_room(room);
                                DISPOSE(room);
                                --top_vroom;
                        }
                        if (room)
                                prev = room;
                }
        }
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
ExitData *get_exit(RoomIndexData * room, sh_int dir)
{
        ExitData *xit;

        if (!room)
        {
                bug("Get_exit: NULL room", 0);
                return NULL;
        }

        for (xit = room->first_exit; xit; xit = xit->next)
                if (xit->vdir == dir)
                        return xit;
        return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
ExitData *get_exit_to(RoomIndexData * room, sh_int dir, int vnum)
{
        ExitData *xit;

        if (!room)
        {
                bug("Get_exit: NULL room", 0);
                return NULL;
        }

        for (xit = room->first_exit; xit; xit = xit->next)
                if (xit->vdir == dir && xit->vnum == vnum)
                        return xit;
        return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
ExitData *get_exit_num(RoomIndexData * room, sh_int count)
{
        ExitData *xit;
        int       cnt;

        if (!room)
        {
                bug("Get_exit: NULL room", 0);
                return NULL;
        }

        for (cnt = 0, xit = room->first_exit; xit; xit = xit->next)
                if (++cnt == count)
                        return xit;
        return NULL;
}

// ============================================================================
// Exit and Movement Helper Functions
// ============================================================================

/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance(CharData * ch, sh_int endurance)
{
        int       cur, max;

        max = can_carry_w(ch);
        cur = ch->carry_weight;
        if (cur >= max)
                return static_cast<sh_int>(endurance * 7);
        else if (cur >= max * EncumbranceThreshold)
                return static_cast<sh_int>(endurance * 6);
        else if (cur >= max * 0.90)
                return static_cast<sh_int>(endurance * 5);
        else if (cur >= max * 0.85)
                return static_cast<sh_int>(endurance * 4);
        else if (cur >= max * 0.80)
                return static_cast<sh_int>(endurance * 3);
        else if (cur >= max * 0.75)
                return static_cast<sh_int>(endurance * 2);
        else
                return endurance;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall(CharData * ch, int fall)
{
        if (!ch)
                return FALSE;

        if (IsSet(ch->in_room->RoomFlags, RoomNofloor)
            && CanGo(ch, DirDown)
            && (!IsAffected(ch, AffFlying)
                || (ch->mount && !IsAffected(ch->mount, AffFlying))))
        {
                if (fall > FallDamageThreshold)
                {
                        bug("Falling (in a loop?) more than %d rooms: vnum %d", 
                            FallDamageThreshold, ch->in_room->vnum);
                        char_from_room(ch);
                        char_to_room(ch, get_room_index(wherehome(ch)));
                        fall = 0;
                        return TRUE;
                }
                set_char_color(AtFalling, ch);
                send_to_char("You're falling down...\n\r", ch);
                move_char(ch, get_exit(ch->in_room, DirDown), ++fall, FALSE);
                return TRUE;
        }
        return FALSE;
}


/*
 * create a 'virtual' room					-Thoric
 */
RoomIndexData *generate_exit(RoomIndexData * in_room, ExitData ** pexit)
{
        ExitData *xit, *bxit;
        ExitData *orig_exit = static_cast<ExitData *>(*pexit);
        RoomIndexData *room, *backroom;
        int       brvnum;
        int       serial;
        int       distance = -1;
        int       vdir = orig_exit->vdir;
        sh_int    hash;
        bool      found = FALSE;

        if (in_room->vnum > MaxVnums)  /* room is virtual */
        {
                serial = in_room->vnum;
                if ((serial & MaxVnums) == orig_exit->vnum)
                {
                        brvnum = serial >> 16;
                }
                else
                {
                        brvnum = serial & MaxVnums;
                        distance = orig_exit->distance - 1;
                }
                backroom = get_room_index(brvnum);
        }
        else
        {
                int       r1 = in_room->vnum;
                int       r2 = orig_exit->vnum;

                brvnum = r1;
                backroom = in_room;
                serial = (UMax(r1, r2) << 16) | UMin(r1, r2);
                distance = orig_exit->distance - 1;
        }
        hash = static_cast<sh_int>(serial % 64);

        for (room = vroom_hash[hash]; room; room = room->next)
                if (!found)
                {
                        CREATE(room, RoomIndexData, 1);
                        room->area = in_room->area;
                        room->vnum = serial;
                        room->sector_type = in_room->sector_type;
                        room->RoomFlags = in_room->RoomFlags;
                        decorate_room(room);
                        room->next = vroom_hash[hash];
                        vroom_hash[hash] = room;
                        ++top_vroom;
                }
        if (!found || (xit = get_exit(room, static_cast<sh_int>(vdir))) == NULL)
        {
                xit = make_exit(room, orig_exit->to_room, static_cast<sh_int>(vdir));
                xit->keyword = STRALLOC(const_cast<char*>(""));
                xit->description = STRALLOC(const_cast<char*>(""));
                xit->key = -1;
                xit->distance = static_cast<sh_int>(distance);
        }
        if (!found)
        {
                bxit = make_exit(room, backroom, rev_dir[vdir]);
                bxit->keyword = STRALLOC(const_cast<char*>(""));
                bxit->description = STRALLOC(const_cast<char*>(""));
                bxit->key = -1;
                {
                        ExitData *tmp;
                        int       fulldist = 0;

                        if ((tmp = get_exit(backroom, static_cast<sh_int>(vdir))) != NULL)
                        {
                                fulldist = tmp->distance;
                        }

                        bxit->distance = static_cast<sh_int>(fulldist - distance);
                }
        }
        /*
         * (ExitData *) pexit = xit; - FIXED - Gavin - This isn't actually right, should be deferfencing not typecasting 
         */
        *pexit = xit;
        return room;
}

// ============================================================================
// Core Movement Functions
// ============================================================================

ch_ret move_char(CharData * ch, ExitData * pexit, int fall, bool running)
{
        RoomIndexData *in_room;
        RoomIndexData *to_room;
        RoomIndexData *from_room;
        char      buf[MaxStringLength];
        char     *txt;
        char     *dtxt;
        ch_ret    retcode;
        sh_int    door, distance;
        bool      drunk = FALSE;
        bool      brief = FALSE;

        if (!IsNpc(ch))
                if (IsDrunk(ch, 2) && (ch->position != PosShove)
                    && (ch->position != PosDrag))
                        drunk = TRUE;

        if (drunk && !fall)
        {
                door = static_cast<sh_int>(number_door());
                pexit = get_exit(ch->in_room, static_cast<sh_int>(door));
        }

#ifdef DEBUG
        if (pexit)
        {
                snprintf(buf, MSL, "move_char: %s to door %d", ch->name,
                         pexit->vdir);
                log_string(buf);
        }
#endif

        retcode = rSTOP;
        txt = NULL;

        if (IsNpc(ch) && IsSet(ch->act, ActMounted))
                return retcode;

        in_room = ch->in_room;
        from_room = in_room;
        if (!pexit || (to_room = pexit->to_room) == NULL)
        {
                if (drunk)
                        send_to_char
                                ("You hit a wall in your drunken state.\n\r",
                                 ch);
                else
                        send_to_char("Alas, you cannot go that way.\n\r", ch);
                return rSTOP;
        }

        door = pexit->vdir;
        distance = pexit->distance;

        /*
         * Exit is only a "window", there is no way to travel in that direction
         * unless it's a door with a window in it       -Thoric
         */
        if (IsSet(pexit->exit_info, ExWindow)
            && !IsSet(pexit->exit_info, ExIsdoor))
        {
                send_to_char("Alas, you cannot go that way.\n\r", ch);
                return rSTOP;
        }

        if (IsSet(pexit->exit_info, ExPortal) && IsNpc(ch))
        {
                act(AtPlain, "Mobs can't use portals.", ch, NULL, NULL,
                    ToChar);
                return rSTOP;
        }

        if (IsSet(pexit->exit_info, ExNomob) && IsNpc(ch))
        {
                act(AtPlain, "Mobs can't enter there.", ch, NULL, NULL,
                    ToChar);
                return rSTOP;
        }
        if (IsNpc(ch))
                if (!check_pos(ch, 8) && !check_pos(ch, 9)
                    && !check_pos(ch, 10) && !check_pos(ch, 11))
                        return rSTOP;

        if (IsSet(pexit->exit_info, ExClosed)
            && (!IsAffected(ch, AffPassDoor)
                || IsSet(pexit->exit_info, ExNopassdoor)))
        {
                if (!IsSet(pexit->exit_info, ExSecret)
                    && !IsSet(pexit->exit_info, ExDig))
                {
                        if (drunk)
                        {
                                act(AtPlain,
                                    "$n runs into the $d in $s drunken state.",
                                    ch, NULL, pexit->keyword, ToRoom);
                                act(AtPlain,
                                    "You run into the $d in your drunken state.",
                                    ch, NULL, pexit->keyword, ToChar);
                        }
                        else
                                act(AtPlain, "The $d is closed.", ch, NULL,
                                    pexit->keyword, ToChar);
                }
                else
                {
                        if (drunk)
                                send_to_char
                                        ("You hit a wall in your drunken state.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("Alas, you cannot go that way.\n\r",
                                         ch);
                }

                return rSTOP;
        }

        /*
         * Crazy virtual room idea, created upon demand.        -Thoric
         */
        if (distance > 1)
                if ((to_room = generate_exit(in_room, &pexit)) == NULL)
                        send_to_char("Alas, you cannot go that way.\n\r", ch);

        if (!fall
            && IsAffected(ch, AffCharm)
            && ch->master && in_room == ch->master->in_room)
        {
                send_to_char("What?  And leave your beloved master?\n\r", ch);
                return rSTOP;
        }

        if (room_is_private(ch, to_room))
        {
                send_to_char("That room is private right now.\n\r", ch);
                return rSTOP;
        }

/*    if ( !IsImmortal(ch)
    &&  !IsNpc(ch)
    &&  ch->in_room->area != to_room->area )
    {
	if ( ch->top_level < to_room->area->low_hard_range )
	{
	    set_char_color( AtTell, ch );
	    switch( to_room->area->low_hard_range - ch->top_level )
	    {
		case 1:
		  send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'", ch );
		  break;
		case 2:
		  send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'", ch );
		  break;
		case 3:
		  send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch);
		  break;
		default:
		  send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch);
	    }
	    return rSTOP;
	}
	else
	if ( ch->top_level > to_room->area->hi_hard_range )
	{
	    set_char_color( AtTell, ch );
	    send_to_char( "A voice in your mind says, 'There is nothing more for you down that path.'", ch );
	    return rSTOP;
	}          
    } I don't like these - Gavin */

        if (!fall && !IsNpc(ch))
        {
                int       endurance;

                if (in_room->sector_type == SectAir
                    || to_room->sector_type == SectAir
                    || IsSet(pexit->exit_info, ExFly))
                {
                        if (ch->mount && !IsAffected(ch->mount, AffFlying))
                        {
                                send_to_char("Your mount can't fly.\n\r", ch);
                                return rSTOP;
                        }
                        if (!ch->mount && !IsAffected(ch, AffFlying))
                        {
                                send_to_char
                                        ("You'd need to fly to go there.\n\r",
                                         ch);
                                return rSTOP;
                        }
                }

                if (in_room->sector_type == SectWaterNoswim
                    || to_room->sector_type == SectWaterNoswim)
                {
                        ObjData *obj;
                        bool      found;

                        found = FALSE;
                        if (ch->mount)
                        {
                                if (IsAffected(ch->mount, AffFlying)
                                    || IsAffected(ch->mount, AffFloating))
                                        found = TRUE;
                        }
                        else if (IsAffected(ch, AffFlying)
                                 || IsAffected(ch, AffFloating))
                                found = TRUE;

                        /*
                         * Look for a boat.
                         */
                        if (!found)
                                for (obj = ch->first_carrying; obj;
                                     obj = obj->next_content)
                                {
                                        if (obj->item_type == ItemBoat)
                                        {
                                                found = TRUE;
                                                if (drunk)
                                                        txt = const_cast<char*>("paddles unevenly");
                                                else
                                                        txt = const_cast<char*>("paddles");
                                                break;
                                        }
                                }

                        if (!found)
                        {
                                send_to_char
                                        ("You'd need a boat to go there.\n\r",
                                         ch);
                                return rSTOP;
                        }
                }

                if (IsSet(pexit->exit_info, ExClimb))
                {
                        bool      found;

                        found = FALSE;
                        if (ch->mount && IsAffected(ch->mount, AffFlying))
                                found = TRUE;
                        else if (IsAffected(ch, AffFlying))
                                found = TRUE;

                        if (!found && !ch->mount)
                        {
                                if ((!IsNpc(ch)
                                     && number_percent() >
                                     ch->PCData->learned[gsn_climb]) || drunk
                                    || ch->mental_state < -90)
                                {
                                        send_to_char
                                                ("You start to climb... but lose your grip and fall!\n\r",
                                                 ch);
                                        learn_from_failure(ch, gsn_climb);
                                        if (pexit->vdir == DirDown)
                                        {
                                                retcode =
                                                        move_char(ch, pexit,
                                                                  1, running);
                                                return retcode;
                                        }
                                        if (number_percent() > 50
                                            && !IsImmortal(ch))
                                        {
                                                if (!IsNpc(ch)
                                                    /*
                                                     * !IsSet( ch->PCData->cyber, CyberLegs ) 
                                                     */
                                                        )
                                                {
                                                        if (!IsSet
                                                            (ch->bodyparts,
                                                             BodyLLeg))
                                                                SetBit(ch->
                                                                        bodyparts,
                                                                        BodyLLeg);
                                                        else if (!IsSet
                                                                 (ch->
                                                                  bodyparts,
                                                                  BodyRLeg))
                                                                SetBit(ch->
                                                                        bodyparts,
                                                                        BodyRLeg);
                                                }
                                        }

                                        set_char_color(AtHurt, ch);
                                        send_to_char
                                                ("OUCH! You hit the ground!\n\r",
                                                 ch);
                                        WaitState(ch, 20);
                                        retcode =
                                                damage(ch, ch,
                                                       (pexit->vdir ==
                                                        DirUp ? 10 : 5),
                                                       TypeUndefined);
                                        return retcode;
                                }
                                found = TRUE;
                                learn_from_success(ch, gsn_climb);
                                WaitState(ch, skill_table[gsn_climb]->beats);
                                txt = const_cast<char*>("climbs");
                        }

                        if (!found)
                        {
                                send_to_char("You can't climb.\n\r", ch);
                                return rSTOP;
                        }
                }

                if (ch->mount)
                {
                        switch (ch->mount->position)
                        {
                        case PosDead:
                                send_to_char("Your mount is dead!\n\r", ch);
                                return rSTOP;
                                break;

                        case PosMortal:
                        case PosIncap:
                                send_to_char
                                        ("Your mount is hurt far too badly to move.\n\r",
                                         ch);
                                return rSTOP;
                                break;

                        case PosStunned:
                                send_to_char
                                        ("Your mount is too stunned to do that.\n\r",
                                         ch);
                                return rSTOP;
                                break;

                        case PosSleeping:
                                send_to_char("Your mount is sleeping.\n\r",
                                             ch);
                                return rSTOP;
                                break;

                        case PosResting:
                                send_to_char("Your mount is resting.\n\r",
                                             ch);
                                return rSTOP;
                                break;

                        case PosSitting:
                                send_to_char
                                        ("Your mount is sitting down.\n\r",
                                         ch);
                                return rSTOP;
                                break;

                        default:
                                break;
                        }

                        if (!IsAffected(ch->mount, AffFlying)
                            && !IsAffected(ch->mount, AffFloating))
                                endurance =
                                        movement_loss[UMin
                                                      (SectMax - 1,
                                                       in_room->
                                                       sector_type)] * 3;
                        else
                                endurance = 3;
                        if (ch->mount->endurance < endurance)
                        {
                                send_to_char
                                        ("Your mount is too exhausted.\n\r",
                                         ch);
                                return rSTOP;
                        }
                }
                else
                {
                        if (!IsAffected(ch, AffFlying)
                            && !IsAffected(ch, AffFloating))
                                endurance =
                                        encumbrance(ch,
                                                    movement_loss[UMin
                                                                  (SectMax -
                                                                   1,
                                                                   in_room->
                                                                   sector_type)]
                                                    * 3);
                        else
                                endurance = 3;
                        if (ch->endurance < endurance)
                        {
                                send_to_char("You are too exhausted.\n\r",
                                             ch);
                                return rSTOP;
                        }
                }

                WaitState(ch, 1);
                if (ch->mount)
                        ch->mount->endurance -= static_cast<sh_int>(endurance);
                else
                        ch->endurance -= static_cast<sh_int>(endurance);
        }

        /*
         * Check if player can fit in the room
         */
        if (to_room->tunnel > 0)
        {
                CharData *ctmp;
                int       count = ch->mount ? 1 : 0;

                for (ctmp = to_room->first_person; ctmp;
                     ctmp = ctmp->next_in_room)
                        if (++count >= to_room->tunnel)
                        {
                                if (ch->mount && count == to_room->tunnel)
                                        send_to_char
                                                ("There is no room for both you and your mount in there.\n\r",
                                                 ch);
                                else
                                        send_to_char
                                                ("There is no room for you in there.\n\r",
                                                 ch);
                                return rSTOP;
                        }
        }

        /*
         * check for traps on exit - later 
         */

        if (!IsAffected(ch, AffSneak)
            && (IsNpc(ch) || !IsSet(ch->act, PlrWizinvis)))
        {
                if (fall)
                        txt = const_cast<char*>("falls");
                else if (!txt)
                {
                        if (ch->mount)
                        {
                                if (IsAffected(ch->mount, AffFloating))
                                        txt = const_cast<char*>("floats");
                                else if (IsAffected(ch->mount, AffFlying))
                                        txt = const_cast<char*>("flys");
                                else
                                        txt = const_cast<char*>("rides");
                        }
                        else
                        {
                                if (IsAffected(ch, AffFloating))
                                {
                                        if (drunk)
                                                txt = const_cast<char*>("floats unsteadily");
                                        else
                                                txt = const_cast<char*>("floats");
                                }
                                else if (IsAffected(ch, AffFlying))
                                {
                                        if (drunk)
                                                txt = const_cast<char*>("flys shakily");
                                        else
                                                txt = const_cast<char*>("flys");
                                }
                                else if (ch->position == PosShove)
                                        txt = const_cast<char*>("is shoved");
                                else if (ch->position == PosDrag)
                                        txt = const_cast<char*>("is dragged");
                                else
                                {
                                        if (drunk)
                                                txt = const_cast<char*>("stumbles drunkenly");
                                        else if (running)
                                                txt = const_cast<char*>("runs");
                                        else
                                                txt = const_cast<char*>("leaves");
                                }
                        }
                }
                if (ch->mount)
                {
                        snprintf(buf, MSL, "$n %s %s upon $N.", txt,
                                 dir_name[door]);
                        act(AtAction, buf, ch, NULL, ch->mount, ToNotvict);
                }
                else
                {
                        snprintf(buf, MSL, "$n %s $T.", txt);
                        act(AtAction, buf, ch, NULL, const_cast<void*>(static_cast<const void*>(dir_name[door])),
                            ToRoom);
                }
        }

        rprog_leave_trigger(ch);
        if (char_died(ch))
                return global_retcode;


        char_from_room(ch);
        char_to_room(ch, to_room);
        retcode = rNONE;
        if (ch->mount)
        {
                rprog_leave_trigger(ch->mount);

                /*
                 * Mount bug fix test. -Orion
                 */
                if (char_died(ch->mount))
                        return global_retcode;

                if (ch->mount)
                {
                        char_from_room(ch->mount);
                        char_to_room(ch->mount, to_room);
                }
        }

        if (!IsAffected(ch, AffSneak)
            && (IsNpc(ch) || !IsSet(ch->act, PlrWizinvis)))
        {
                if (fall)
                        txt = const_cast<char*>("falls");
                else if (ch->mount)
                {
                        if (IsAffected(ch->mount, AffFloating))
                                txt = const_cast<char*>("floats in");
                        else if (IsAffected(ch->mount, AffFlying))
                                txt = const_cast<char*>("flys in");
                        else
                                txt = const_cast<char*>("rides in");
                }
                else
                {
                        if (IsAffected(ch, AffFloating))
                        {
                                if (drunk)
                                        txt = const_cast<char*>("floats in unsteadily");
                                else
                                        txt = const_cast<char*>("floats in");
                        }
                        else if (IsAffected(ch, AffFlying))
                        {
                                if (drunk)
                                        txt = const_cast<char*>("flys in shakily");
                                else
                                        txt = const_cast<char*>("flys in");
                        }
                        else if (ch->position == PosShove)
                                txt = const_cast<char*>("is shoved in");
                        else if (ch->position == PosDrag)
                                txt = const_cast<char*>("is dragged in");
                        else
                        {
                                if (drunk)
                                        txt = const_cast<char*>("stumbles drunkenly in");
                                else if (running)
                                        txt = const_cast<char*>("runs in");
                                else
                                        txt = const_cast<char*>("arrives");
                        }
                }
                switch (door)
                {
                default:
                        dtxt = const_cast<char*>("somewhere");
                        break;
                case 0:
                        dtxt = const_cast<char*>("the south");
                        break;
                case 1:
                        dtxt = const_cast<char*>("the west");
                        break;
                case 2:
                        dtxt = const_cast<char*>("the north");
                        break;
                case 3:
                        dtxt = const_cast<char*>("the east");
                        break;
                case 4:
                        dtxt = const_cast<char*>("below");
                        break;
                case 5:
                        dtxt = const_cast<char*>("above");
                        break;
                case 6:
                        dtxt = const_cast<char*>("the south-west");
                        break;
                case 7:
                        dtxt = const_cast<char*>("the south-east");
                        break;
                case 8:
                        dtxt = const_cast<char*>("the north-west");
                        break;
                case 9:
                        dtxt = const_cast<char*>("the north-east");
                        break;
                }
                if (ch->mount)
                {
                        snprintf(buf, MSL, "$n %s from %s upon $N.", txt,
                                 dtxt);
                        act(AtAction, buf, ch, NULL, ch->mount, ToRoom);
                }
                else
                {
                        snprintf(buf, MSL, "$n %s from %s.", txt, dtxt);
                        act(AtAction, buf, ch, NULL, NULL, ToRoom);
                }
        }

        if (!running)
                do_look(ch, "auto");

        if (brief)
                SetBit(ch->act, PlrBrief);



        /*
         * BIG ugly looping problem here when the character is mptransed back
         * to the starting room.  To avoid this, check how many chars are in 
         * the room at the start and stop processing followers after doing
         * the right number of them.  -- Narn
         */
        if (!fall)
        {
                CharData *fch;
                CharData *nextinroom;
                int       chars = 0, count = 0;

                for (fch = from_room->first_person; fch;
                     fch = fch->next_in_room)
                        chars++;

                for (fch = from_room->first_person; fch && (count < chars);
                     fch = nextinroom)
                {
                        nextinroom = fch->next_in_room;
                        count++;
                        if (fch != ch   /* loop room bug fix here by Thoric */
                            && fch->master == ch
                            && fch->position == PosStanding)
                        {
                                act(AtAction, "You follow $N.", fch, NULL,
                                    ch, ToChar);
                                move_char(fch, pexit, 0, running);
                        }
                }
        }

        if (ch->in_room->first_content)
        {
                retcode = check_room_for_traps(ch, TrapEnterRoom);
        }

        if (char_died(ch))
                return retcode;

        mprog_entry_trigger(ch);
        if (char_died(ch))
                return retcode;

        rprog_enter_trigger(ch);
        if (char_died(ch))
                return retcode;

        mprog_greet_trigger(ch);
        if (char_died(ch))
                return retcode;

        oprog_greet_trigger(ch);
        if (char_died(ch))
                return retcode;

        if (!will_fall(ch, fall) && fall > 0)
        {
                if (!IsAffected(ch, AffFloating)
                    || (ch->mount && !IsAffected(ch->mount, AffFloating)))
                {
                        set_char_color(AtHurt, ch);
                        send_to_char("OUCH! You hit the ground!\n\r", ch);
                        WaitState(ch, 20);
                        retcode = damage(ch, ch, 50 * fall, TypeUndefined);
                }
                else
                {
                        set_char_color(AtMagic, ch);
                        send_to_char
                                ("You lightly float down to the ground.\n\r",
                                 ch);
                }
        }
        return retcode;
}

// ============================================================================
// Directional Movement Commands
// ============================================================================

CMDF do_north(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirNorth), 0, FALSE);
        return;
}


CMDF do_east(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirEast), 0, FALSE);
        return;
}


CMDF do_south(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirSouth), 0, FALSE);
        return;
}


CMDF do_west(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirWest), 0, FALSE);
        return;
}


CMDF do_up(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirUp), 0, FALSE);
        return;
}


CMDF do_down(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirDown), 0, FALSE);
        return;
}

CMDF do_northeast(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirNortheast), 0, FALSE);
        return;
}

CMDF do_northwest(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirNorthwest), 0, FALSE);
        return;
}

CMDF do_southeast(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirSoutheast), 0, FALSE);
        return;
}

CMDF do_southwest(CharData * ch, [[maybe_unused]] const char *argument)
{
        move_char(ch, get_exit(ch->in_room, DirSouthwest), 0, FALSE);
        return;
}



ExitData *find_door(CharData * ch, const char *arg, bool quiet)
{
        ExitData *pexit;
        int       door;

        if (arg == NULL || !str_cmp(arg, ""))
                return NULL;

        pexit = NULL;
        if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
                door = 0;
        else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
                door = 1;
        else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
                door = 2;
        else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
                door = 3;
        else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
                door = 4;
        else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
                door = 5;
        else if (!str_cmp(arg, "ne") || !str_cmp(arg, "northeast"))
                door = 6;
        else if (!str_cmp(arg, "nw") || !str_cmp(arg, "northwest"))
                door = 7;
        else if (!str_cmp(arg, "se") || !str_cmp(arg, "southeast"))
                door = 8;
        else if (!str_cmp(arg, "sw") || !str_cmp(arg, "southwest"))
                door = 9;
        else
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                {
                        if ((quiet || IsSet(pexit->exit_info, ExIsdoor))
                            && pexit->keyword
                            && nifty_is_name(const_cast<char*>(arg), pexit->keyword))
                                return pexit;
                }
                if (!quiet)
                        act(AtPlain, "You see no $T here.", ch, NULL, const_cast<void*>(static_cast<const void*>(arg)),
                            ToChar);
                return NULL;
        }

        if ((pexit = get_exit(ch->in_room, static_cast<sh_int>(door))) == NULL)
        {
                if (!quiet)
                        act(AtPlain, "You see no $T here.", ch, NULL, const_cast<void*>(static_cast<const void*>(arg)),
                            ToChar);
                return NULL;
        }

        if (quiet)
                return pexit;

        if (IsSet(pexit->exit_info, ExSecret))
        {
                act(AtPlain, "You see no $T here.", ch, NULL, const_cast<void*>(static_cast<const void*>(arg)), ToChar);
                return NULL;
        }

        if (!IsSet(pexit->exit_info, ExIsdoor))
        {
                send_to_char("You can't do that.\n\r", ch);
                return NULL;
        }

        return pexit;
}

// ============================================================================
// Door and Exit Manipulation Functions  
// ============================================================================

void toggle_bexit_flag(ExitData * pexit, int flag)
{
        ExitData *pexit_rev;

        ToggleBit(pexit->exit_info, flag);
        if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
                ToggleBit(pexit_rev->exit_info, flag);
}

void set_bexit_flag(ExitData * pexit, int flag)
{
        ExitData *pexit_rev;

        SetBit(pexit->exit_info, flag);
        if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
                SetBit(pexit_rev->exit_info, flag);
}

void remove_bexit_flag(ExitData * pexit, int flag)
{
        ExitData *pexit_rev;

        RemoveBit(pexit->exit_info, flag);
        if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
                RemoveBit(pexit_rev->exit_info, flag);
}

CMDF do_open(CharData * ch, [[maybe_unused]] const char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ExitData *pexit;
        int       door;

        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
        {
                do_openhatch(ch, "");
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'open door' 
                 */
                ExitData *pexit_rev;

                if (!IsSet(pexit->exit_info, ExIsdoor))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (!IsSet(pexit->exit_info, ExClosed))
                {
                        send_to_char("It's already open.\n\r", ch);
                        return;
                }
                if (IsSet(pexit->exit_info, ExLocked))
                {
                        send_to_char("It's locked.\n\r", ch);
                        return;
                }

                if (!IsSet(pexit->exit_info, ExSecret)
                    || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
                {
                        act(AtAction, "$n opens the $d.", ch, NULL,
                            pexit->keyword, ToRoom);
                        act(AtAction, "You open the $d.", ch, NULL,
                            pexit->keyword, ToChar);
                        if ((pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                        {
                                CharData *rch;

                                for (rch = pexit->to_room->first_person; rch;
                                     rch = rch->next_in_room)
                                        act(AtAction, "The $d opens.", rch,
                                            NULL, pexit_rev->keyword,
                                            ToChar);
                                sound_to_room(pexit->to_room, const_cast<char*>("door"));
                        }
                        remove_bexit_flag(pexit, ExClosed);
                        if ((door = pexit->vdir) >= 0 && door < 10)
                                check_room_for_traps(ch, trap_door[door]);

                        sound_to_room(ch->in_room, const_cast<char*>("door"));
                        return;
                }
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'open object' 
                 */
                if (obj->item_type != ItemContainer)
                {
                        ch_printf(ch, "%s isn't a container.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (!IsSet(obj->value[1], ContClosed))
                {
                        ch_printf(ch, "%s is already open.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (!IsSet(obj->value[1], ContCloseable))
                {
                        ch_printf(ch, "%s cannot be opened or closed.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (IsSet(obj->value[1], ContLocked))
                {
                        ch_printf(ch, "%s is locked.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }

                RemoveBit(obj->value[1], ContClosed);
                act(AtAction, "You open $p.", ch, obj, NULL, ToChar);
                act(AtAction, "$n opens $p.", ch, obj, NULL, ToRoom);
                check_for_trap(ch, obj, TrapOpen);
                return;
        }

        /*
         * Gavin - Fixed had a ; at the end of the iff statement 
         */
        if (!str_cmp(arg, "hatch"))
        {
                do_openhatch(ch, argument);
                return;
        }

        do_openhatch(ch, arg);
        return;
}



CMDF do_close(CharData * ch, [[maybe_unused]] const char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ExitData *pexit;
        int       door;

        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
        {
                do_closehatch(ch, "");
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'close door' 
                 */
                ExitData *pexit_rev;

                if (!IsSet(pexit->exit_info, ExIsdoor))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (IsSet(pexit->exit_info, ExClosed))
                {
                        send_to_char("It's already closed.\n\r", ch);
                        return;
                }

                act(AtAction, "$n closes the $d.", ch, NULL, pexit->keyword,
                    ToRoom);
                act(AtAction, "You close the $d.", ch, NULL, pexit->keyword,
                    ToChar);

                /*
                 * close the other side 
                 */
                if ((pexit_rev = pexit->rexit) != NULL
                    && pexit_rev->to_room == ch->in_room)
                {
                        CharData *rch;

                        SetBit(pexit_rev->exit_info, ExClosed);
                        for (rch = pexit->to_room->first_person; rch;
                             rch = rch->next_in_room)
                                act(AtAction, "The $d closes.", rch, NULL,
                                    pexit_rev->keyword, ToChar);
                }
                set_bexit_flag(pexit, ExClosed);
                if ((door = pexit->vdir) >= 0 && door < 10)
                        check_room_for_traps(ch, trap_door[door]);
                return;
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'close object' 
                 */
                if (obj->item_type != ItemContainer)
                {
                        ch_printf(ch, "%s isn't a container.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (IsSet(obj->value[1], ContClosed))
                {
                        ch_printf(ch, "%s is already closed.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (!IsSet(obj->value[1], ContCloseable))
                {
                        ch_printf(ch, "%s cannot be opened or closed.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }

                SetBit(obj->value[1], ContClosed);
                act(AtAction, "You close $p.", ch, obj, NULL, ToChar);
                act(AtAction, "$n closes $p.", ch, obj, NULL, ToRoom);
                check_for_trap(ch, obj, TrapClose);
                return;
        }

        if (!str_cmp(arg, "hatch"))
        {
                do_closehatch(ch, argument);
                return;
        }

        do_closehatch(ch, arg);
        return;
}


bool has_key(CharData * ch, int key)
{
        ObjData *obj;

        for (obj = ch->first_carrying; obj; obj = obj->next_content)
                if (obj->pIndexData->vnum == key || obj->value[0] == key)
                        return TRUE;

        return FALSE;
}


CMDF do_lock(CharData * ch, [[maybe_unused]] const char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ExitData *pexit;

        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
        {
                send_to_char("Lock what?\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'lock door' 
                 */

                if (!IsSet(pexit->exit_info, ExIsdoor))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (!IsSet(pexit->exit_info, ExClosed))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (pexit->key < 0)
                {
                        send_to_char("It can't be locked.\n\r", ch);
                        return;
                }
                if (!has_key(ch, pexit->key))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (IsSet(pexit->exit_info, ExLocked))
                {
                        send_to_char("It's already locked.\n\r", ch);
                        return;
                }

                if (!IsSet(pexit->exit_info, ExSecret)
                    || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
                {
                        send_to_char("*Click*\n\r", ch);
                        act(AtAction, "$n locks the $d.", ch, NULL,
                            pexit->keyword, ToRoom);
                        set_bexit_flag(pexit, ExLocked);
                        return;
                }
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'lock object' 
                 */
                if (obj->item_type != ItemContainer)
                {
                        send_to_char("That's not a container.\n\r", ch);
                        return;
                }
                if (!IsSet(obj->value[1], ContClosed))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (obj->value[2] < 0)
                {
                        send_to_char("It can't be locked.\n\r", ch);
                        return;
                }
                if (!has_key(ch, obj->value[2]))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (IsSet(obj->value[1], ContLocked))
                {
                        send_to_char("It's already locked.\n\r", ch);
                        return;
                }

                SetBit(obj->value[1], ContLocked);
                send_to_char("*Click*\n\r", ch);
                act(AtAction, "$n locks $p.", ch, obj, NULL, ToRoom);
                return;
        }

        ch_printf(ch, "You see no %s here.\n\r", arg);
        return;
}



CMDF do_unlock(CharData * ch, [[maybe_unused]] const char *argument)
{
        char      arg[MaxInputLength];
        ObjData *obj;
        ExitData *pexit;

        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
        {
                send_to_char("Unlock what?\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'unlock door' 
                 */

                if (!IsSet(pexit->exit_info, ExIsdoor))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (!IsSet(pexit->exit_info, ExClosed))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (pexit->key < 0)
                {
                        send_to_char("It can't be unlocked.\n\r", ch);
                        return;
                }
                if (!has_key(ch, pexit->key))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (!IsSet(pexit->exit_info, ExLocked))
                {
                        send_to_char("It's already unlocked.\n\r", ch);
                        return;
                }

                if (!IsSet(pexit->exit_info, ExSecret)
                    || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
                {
                        send_to_char("*Click*\n\r", ch);
                        act(AtAction, "$n unlocks the $d.", ch, NULL,
                            pexit->keyword, ToRoom);
                        remove_bexit_flag(pexit, ExLocked);
                        return;
                }
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'unlock object' 
                 */
                if (obj->item_type != ItemContainer)
                {
                        send_to_char("That's not a container.\n\r", ch);
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
                if (!has_key(ch, obj->value[2]))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (!IsSet(obj->value[1], ContLocked))
                {
                        send_to_char("It's already unlocked.\n\r", ch);
                        return;
                }

                RemoveBit(obj->value[1], ContLocked);
                send_to_char("*Click*\n\r", ch);
                act(AtAction, "$n unlocks $p.", ch, obj, NULL, ToRoom);
                return;
        }

        ch_printf(ch, "You see no %s here.\n\r", arg);
        return;
}

CMDF do_bashdoor(CharData * ch, [[maybe_unused]] const char *argument)
{
        ExitData *pexit;
        char      arg[MaxInputLength];

        if (!IsNpc(ch) && ch->PCData->learned[gsn_bashdoor] <= 0)
        {
                send_to_char
                        ("You're not enough of a warrior to bash doors!\n\r",
                         ch);
                return;
        }

        one_argument(const_cast<char*>(argument), arg);

        if (arg[0] == '\0')
        {
                send_to_char("Bash what?\n\r", ch);
                return;
        }

        if (ch->fighting)
        {
                send_to_char("You can't break off your fight.\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, arg, FALSE)) != NULL)
        {
                RoomIndexData *to_room;
                ExitData *pexit_rev;
                int       percent_chance;
                char     *keyword;

                if (!IsSet(pexit->exit_info, ExClosed))
                {
                        send_to_char("Calm down.  It is already open.\n\r",
                                     ch);
                        return;
                }

                WaitState(ch, skill_table[gsn_bashdoor]->beats);

                if (IsSet(pexit->exit_info, ExSecret))
                        keyword = const_cast<char*>("wall");
                else
                        keyword = pexit->keyword;
                if (!IsNpc(ch))
                        percent_chance =
                                ch->PCData->learned[gsn_bashdoor] / 2;
                else
                        percent_chance = 90;

                if (!IsSet(pexit->exit_info, ExBashproof)
                    && ch->endurance >= MinBashEndurance
                    && number_percent() <
                    (percent_chance + 4 * (get_curr_str(ch) - 19)))
                {
                        RemoveBit(pexit->exit_info, ExClosed);
                        if (IsSet(pexit->exit_info, ExLocked))
                                RemoveBit(pexit->exit_info, ExLocked);
                        SetBit(pexit->exit_info, ExBashed);

                        act(AtSkill, "Crash!  You bashed open the $d!", ch,
                            NULL, keyword, ToChar);
                        act(AtSkill, "$n bashes open the $d!", ch, NULL,
                            keyword, ToRoom);
                        learn_from_success(ch, gsn_bashdoor);

                        if ((to_room = pexit->to_room) != NULL
                            && (pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                        {
                                CharData *rch;

                                RemoveBit(pexit_rev->exit_info, ExClosed);
                                if (IsSet(pexit_rev->exit_info, ExLocked))
                                        RemoveBit(pexit_rev->exit_info,
                                                   ExLocked);
                                SetBit(pexit_rev->exit_info, ExBashed);

                                for (rch = to_room->first_person; rch;
                                     rch = rch->next_in_room)
                                {
                                        act(AtSkill, "The $d crashes open!",
                                            rch, NULL, pexit_rev->keyword,
                                            ToChar);
                                }
                        }
                        damage(ch, ch, (ch->max_hit / 20), gsn_bashdoor);

                }
                else
                {
                        act(AtSkill,
                            "WHAAAAM!!!  You bash against the $d, but it doesn't budge.",
                            ch, NULL, keyword, ToChar);
                        act(AtSkill,
                            "WHAAAAM!!!  $n bashes against the $d, but it holds strong.",
                            ch, NULL, keyword, ToRoom);
                        damage(ch, ch, (ch->max_hit / 20) + 10, gsn_bashdoor);
                        learn_from_failure(ch, gsn_bashdoor);
                }
        }
        else
        {
                act(AtSkill,
                    "WHAAAAM!!!  You bash against the wall, but it doesn't budge.",
                    ch, NULL, NULL, ToChar);
                act(AtSkill,
                    "WHAAAAM!!!  $n bashes against the wall, but it holds strong.",
                    ch, NULL, NULL, ToRoom);
                damage(ch, ch, (ch->max_hit / 20) + 10, gsn_bashdoor);
                learn_from_failure(ch, gsn_bashdoor);
        }

        return;
}

// ============================================================================
// Character Position and Stance Commands
// ============================================================================

CMDF do_stand(CharData * ch, [[maybe_unused]] const char *argument)
{
        argument = NULL;
        switch (ch->position)
        {
        case PosSleeping:
                if (IsAffected(ch, AffSleep))
                {
                        send_to_char("You can't seem to wake up!\n\r", ch);
                        return;
                }

                send_to_char("You wake and climb quickly to your feet.\n\r",
                             ch);
                act(AtAction, "$n arises from $s slumber.", ch, NULL, NULL,
                    ToRoom);
                ch->position = PosStanding;
                break;

        case PosResting:
                send_to_char("You gather yourself and stand up.\n\r", ch);
                act(AtAction, "$n rises from $s rest.", ch, NULL, NULL,
                    ToRoom);
                ch->position = PosStanding;
                break;

        case PosSitting:
                send_to_char("You move quickly to your feet.\n\r", ch);
                act(AtAction, "$n rises up.", ch, NULL, NULL, ToRoom);
                ch->position = PosStanding;
                break;

        case PosStanding:
                send_to_char("You are already standing.\n\r", ch);
                break;

        case PosFighting:
                send_to_char("You are already fighting!\n\r", ch);
                break;
        default:
                return;
        }

        return;
}


CMDF do_sit(CharData * ch, [[maybe_unused]] const char *argument)
{
        ObjData *obj = NULL;


        if (ch->position == PosFighting)
        {
                send_to_char("Maybe you should finish this fight first?\n\r",
                             ch);
                return;
        }

        /*
         * okay, now that we know we can sit, find an object to sit on 
         */
        if (argument[0] != '\0')
        {
                obj = get_obj_list(ch, const_cast<char*>(argument), ch->in_room->first_content);
                if (obj == NULL)
                {
                        send_to_char("You don't see that here.\n\r", ch);
                        return;
                }
        }

        if (obj != NULL)
        {
                if (obj->item_type != ItemFurniture)
                {
                        send_to_char("You can't sit on that.\n\r", ch);
                        return;
                }

                if (obj != NULL && ch->on != obj
                    && count_users(obj) >= obj->value[0])
                {
                        act(AtAction, "There's no more room on $p.", ch, obj,
                            NULL, ToChar);
                        return;
                }

                ch->on = obj;
        }
        switch (ch->position)
        {
        case PosSleeping:
                if (IsAffected(ch, AffSleep))
                {
                        send_to_char("You can't wake up!\n\r", ch);
                        return;
                }

                if (obj == NULL)
                {
                        send_to_char("You wake and sit up.\n\r", ch);
                        act(AtAction, "$n wakes and sits up.", ch, NULL,
                            NULL, ToRoom);
                }
                else if (obj->value[2] == SitAt)
                {
                        act(AtAction, "You wake and sit at $p.", ch, obj,
                            NULL, ToChar);
                        act(AtAction, "$n wakes and sits at $p.", ch, obj,
                            NULL, ToRoom);

                }
                else if (obj->value[2] == SitOn)
                {
                        act(AtAction, "You wake and sit on $p.", ch, obj,
                            NULL, ToChar);
                        act(AtAction, "$n wakes and sits at $p.", ch, obj,
                            NULL, ToRoom);
                }
                else
                {
                        act(AtAction, "You wake and sit in $p.", ch, obj,
                            NULL, ToChar);
                        act(AtAction, "$n wakes and sits in $p.", ch, obj,
                            NULL, ToRoom);
                }

                ch->position = PosSitting;
                break;
        case PosResting:
                if (obj == NULL)
                        send_to_char("You stop resting.\n\r", ch);
                else if (obj->value[2] == SitAt)
                {
                        act(AtAction, "You sit at $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n sits at $p.", ch, obj, NULL,
                            ToRoom);
                }

                else if (obj->value[2] == SitOn)
                {
                        act(AtAction, "You sit on $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n sits on $p.", ch, obj, NULL,
                            ToRoom);
                }
                ch->position = PosSitting;
                break;
        case PosSitting:
                send_to_char("You are already sitting down.\n\r", ch);
                break;

        case PosStanding:
                if (obj == NULL)
                {
                        send_to_char("You sit down.\n\r", ch);
                        act(AtAction, "$n sits down on the ground.", ch,
                            NULL, NULL, ToRoom);
                }
                else if (obj->value[2] == SitAt)
                {
                        act(AtAction, "You sit down at $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n sits down at $p.", ch, obj, NULL,
                            ToRoom);
                }
                else if (obj->value[2] == SitOn)
                {
                        act(AtAction, "You sit on $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n sits on $p.", ch, obj, NULL,
                            ToRoom);
                }
                else
                {
                        act(AtAction, "You sit down in $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n sits down in $p.", ch, obj, NULL,
                            ToRoom);
                }
                ch->position = PosSitting;
                break;
        default:
                {
                }
        }
        return;
}

CMDF do_rest(CharData * ch, [[maybe_unused]] const char *argument)
{
        ObjData *obj = NULL;

        /*
         * okay, now that we know we can rest, find an object to rest on 
         */
        if (argument[0] != '\0')
        {
                obj = get_obj_list(ch, const_cast<char*>(argument), ch->in_room->first_content);
                if (obj == NULL)
                {
                        send_to_char("You don't see that here.\n\r", ch);
                        return;
                }
        }
        else
                obj = ch->on;

        if (obj != NULL)
        {
                if (obj->item_type != ItemFurniture)
                {
                        send_to_char("You can't rest on that.\n\r", ch);
                        return;
                }

                if (obj != NULL && ch->on != obj
                    && count_users(obj) >= obj->value[0])
                {
                        act(AtAction, "There's no more room on $p.", ch, obj,
                            NULL, ToChar);
                        return;
                }

                ch->on = obj;
        }

        switch (ch->position)
        {
        case PosSleeping:
                if (IsAffected(ch, AffSleep))
                {
                        send_to_char("You can't wake up!\n\r", ch);
                        return;
                }

                if (obj == NULL)
                {
                        send_to_char("You wake up and start resting.\n\r",
                                     ch);
                        act(AtAction, "$n wakes up and starts resting.", ch,
                            NULL, NULL, ToRoom);
                }
                else if (obj->value[2] == RestAt)
                {
                        act(AtAction, "You wake up and rest at $p.", ch, obj,
                            NULL, ToChar);
                        act(AtAction, "$n wakes up and rests at $p.", ch,
                            obj, NULL, ToRoom);
                }
                else if (obj->value[2] == RestOn)
                {
                        act(AtAction, "You wake up and rest on $p.", ch, obj,
                            NULL, ToChar);
                        act(AtAction, "$n wakes up and rests on $p.", ch,
                            obj, NULL, ToRoom);
                }
                else
                {
                        act(AtAction, "You wake up and rest in $p.", ch, obj,
                            NULL, ToChar);
                        act(AtAction, "$n wakes up and rests in $p.", ch,
                            obj, NULL, ToRoom);
                }
                ch->position = PosResting;
                break;

        case PosResting:
                send_to_char("You are already resting.\n\r", ch);
                break;


        case PosStanding:
                if (obj == NULL)
                {
                        send_to_char("You rest.\n\r", ch);
                        act(AtAction, "$n sits down and rests.", ch, NULL,
                            NULL, ToRoom);
                }
                else if (obj->value[2] == RestAt)
                {
                        act(AtAction, "You sit down at $p and rest.", ch,
                            obj, NULL, ToChar);
                        act(AtAction, "$n sits down at $p and rests.", ch,
                            obj, NULL, ToRoom);
                }
                else if (obj->value[2] == RestOn)
                {
                        act(AtAction, "You sit on $p and rest.", ch, obj,
                            NULL, ToChar);
                        act(AtAction, "$n sits on $p and rests.", ch, obj,
                            NULL, ToRoom);
                }
                else
                {
                        act(AtAction, "You rest in $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n rests in $p.", ch, obj, NULL,
                            ToRoom);
                }
                ch->position = PosResting;
                break;

        case PosSitting:
                if (obj == NULL)
                {
                        send_to_char("You rest.\n\r", ch);
                        act(AtAction, "$n rests.", ch, NULL, NULL, ToRoom);
                }
                else if (obj->value[2] == RestAt)
                {
                        act(AtAction, "You rest at $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n rests at $p.", ch, obj, NULL,
                            ToRoom);
                }
                else if (obj->value[2] == RestOn)
                {
                        act(AtAction, "You rest on $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n rests on $p.", ch, obj, NULL,
                            ToRoom);
                }
                else
                {
                        act(AtAction, "You rest in $p.", ch, obj, NULL,
                            ToChar);
                        act(AtAction, "$n rests in $p.", ch, obj, NULL,
                            ToRoom);
                }
                ch->position = PosResting;
                break;

        case PosMounted:
                send_to_char("You'd better dismount first.\n\r", ch);
                return;
        default:
                {
                }
        }

        rprog_rest_trigger(ch);
        return;
}

CMDF do_sleep(CharData * ch, [[maybe_unused]] const char *argument)
{
        ObjData *obj = NULL;


        switch (ch->position)
        {
        case PosSleeping:
                send_to_char("You are already sleeping.\n\r", ch);
                break;

        case PosResting:
        case PosSitting:
        case PosStanding:
                if (argument[0] == '\0' && ch->on == NULL)
                {
                        send_to_char("You go to sleep.\n\r", ch);
                        act(AtAction, "$n goes to sleep.", ch, NULL, NULL,
                            ToRoom);
                        ch->position = PosSleeping;
                }
                else    /* find an object and sleep on it */
                {
                        if (argument[0] == '\0')
                                obj = ch->on;
                        else
                                obj = get_obj_list(ch, const_cast<char*>(argument),
                                                   ch->in_room->
                                                   first_content);

                        if (obj == NULL)
                        {
                                send_to_char("You don't see that here.\n\r",
                                             ch);
                                return;
                        }
                        if (obj->item_type != ItemFurniture)
                        {
                                send_to_char("You can't sleep on that!\n\r",
                                             ch);
                                return;
                        }

                        if (ch->on != obj
                            && count_users(obj) >= obj->value[0])
                        {
                                act(AtAction,
                                    "There is no room on $p for you.", ch,
                                    obj, NULL, ToChar);
                                return;
                        }

                        ch->on = obj;
                        if (obj->value[2] == SleepAt)
                        {
                                act(AtAction, "You go to sleep at $p.", ch,
                                    obj, NULL, ToChar);
                                act(AtAction, "$n goes to sleep at $p.", ch,
                                    obj, NULL, ToRoom);
                        }
                        else if (obj->value[2] == SleepOn)
                        {
                                act(AtAction, "You go to sleep on $p.", ch,
                                    obj, NULL, ToChar);
                                act(AtAction, "$n goes to sleep on $p.", ch,
                                    obj, NULL, ToRoom);
                        }
                        else
                        {
                                act(AtAction, "You go to sleep in $p.", ch,
                                    obj, NULL, ToChar);
                                act(AtAction, "$n goes to sleep in $p.", ch,
                                    obj, NULL, ToRoom);
                        }
                        ch->position = PosSleeping;
                }
                break;

        case PosFighting:
                send_to_char("You are busy fighting!\n\r", ch);
                break;
        default:
                {
                }
        }

        rprog_sleep_trigger(ch);
        return;
}




CMDF do_wake(CharData * ch, [[maybe_unused]] const char *argument)
{
        char      arg[MaxInputLength];
        CharData *victim;

        one_argument(const_cast<char*>(argument), arg);
        if (arg[0] == '\0')
        {
                do_stand(ch, argument);
                return;
        }

        if (!IsAwake(ch))
        {
                send_to_char("You are asleep yourself!\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IsAwake(victim))
        {
                act(AtPlain, "$N is already awake.", ch, NULL, victim,
                    ToChar);
                return;
        }

        if (IsAffected(victim, AffSleep) || victim->position < PosSleeping)
        {
                act(AtPlain, "You can't seem to wake $M!", ch, NULL, victim,
                    ToChar);
                return;
        }

        act(AtAction, "You wake $M.", ch, NULL, victim, ToChar);
        victim->position = PosStanding;
        act(AtAction, "$n wakes you.", ch, NULL, victim, ToVict);
        return;
}


/*
 * "Climb" in a certain direction.				-Thoric
 */
CMDF do_climb(CharData * ch, [[maybe_unused]] const char *argument)
{
        ExitData *pexit;

        if (argument[0] == '\0')
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                        if (IsSet(pexit->exit_info, EX_xCLIMB))
                        {
                                move_char(ch, pexit, 0, FALSE);
                                return;
                        }
                send_to_char("You cannot climb here.\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, argument, TRUE)) != NULL
            && IsSet(pexit->exit_info, EX_xCLIMB))
        {
                move_char(ch, pexit, 0, FALSE);
                return;
        }
        send_to_char("You cannot climb there.\n\r", ch);
        return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
CMDF do_enter(CharData * ch, [[maybe_unused]] const char *argument)
{
        ExitData *pexit;

        if (argument[0] == '\0')
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                        if (IsSet(pexit->exit_info, EX_xENTER))
                        {
                                move_char(ch, pexit, 0, FALSE);
                                return;
                        }
                send_to_char("You cannot find an entrance here.\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, argument, TRUE)) != NULL
            && IsSet(pexit->exit_info, EX_xENTER))
        {
                move_char(ch, pexit, 0, FALSE);
                return;
        }
        do_board(ch, argument);
        return;
}

/*
 * Leave through an exit.					-Thoric
 */
CMDF do_leave(CharData * ch, [[maybe_unused]] const char *argument)
{
        ExitData *pexit;

        if (argument[0] == '\0')
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                        if (IsSet(pexit->exit_info, EX_xLEAVE))
                        {
                                move_char(ch, pexit, 0, FALSE);
                                return;
                        }
                do_leaveship(ch, "");
                return;
        }

        if ((pexit = find_door(ch, argument, TRUE)) != NULL
            && IsSet(pexit->exit_info, EX_xLEAVE))
        {
                move_char(ch, pexit, 0, FALSE);
                return;
        }
        do_leaveship(ch, "");
        return;
}

/* Run command taken from DOTD codebase - Samson 2-25-99 */
/* Added argument to let players specify how far to run.
 * Fixed an infinite loop bug where somehow a closed door would cause problems.
 * Added Overland support to the command. Samson 4-4-01
 */
/* Supressed display of rooms/terrain until you stop to prevent buffer overflows - Samson 4-16-01 */
CMDF do_run(CharData * ch, [[maybe_unused]] const char *argument)
{
        RoomIndexData *from_room;
        ExitData *pexit;
        int       diff = 0;

        if (argument[0] == '\0')
        {
                send_to_char("Run where?\n\r", ch);
                return;
        }

        if (ch->position != PosStanding && ch->position != PosMounted)
        {
                send_to_char
                        ("You are not in the correct position for that.\n\r",
                         ch);
                return;
        }

        from_room = ch->in_room;

        while ((pexit = find_door(ch, argument, TRUE)) != NULL)
        {
                diff = ch->endurance;
                if (ch->endurance < 1)
                {
                        send_to_char
                                ("You are too exhausted to run anymore.\n\r",
                                 ch);
                        ch->endurance = static_cast<sh_int>(0);
                        break;
                }
                if (move_char(ch, pexit, 0, TRUE) == rSTOP)
                        break;
                diff -= ch->endurance;
                /*
                 * Double movement item for running 
                 */
                ch->endurance -= static_cast<sh_int>(diff);
        }

        if (ch->in_room == from_room)
        {
                send_to_char("You try to run but don't get anywhere.\n\r",
                             ch);
                act(AtAction, "$n tries to run but doesn't get anywhere.",
                    ch, NULL, NULL, ToRoom);
                return;
        }

        send_to_char("You slow down after your run.\n\r", ch);
        act(AtAction, "$n slows down after $s run.", ch, NULL, NULL,
            ToRoom);

        do_look(ch, "auto");
        return;
}

// ============================================================================
// Character Binding and Holding System
// ============================================================================

CMDF do_struggle_binding(CharData * ch)
{
        ObjData *obj = NULL;
        int       chance = 0;

        if (!ch->held)
        {
                bug("%s struggling on a binding without held TRUE!",
                    ch->name);
                return;
        }
        if ((obj = get_eq_char(ch, WearBinding)) == NULL)
        {
                bug("%s is do_struggle_binding with no binding!", ch->name);
                return;
        }
        if (ch->endurance < MinStruggleEndurance)
        {
                send_to_char("You're too tired to struggle more.", ch);
                return;
        }
        else
        {
                chance = (get_curr_str(ch) +
                          (get_curr_dex(ch) / 2)) /
                        number_range(get_curr_str(ch) / 5,
                                     get_curr_str(ch) / 2);
                if (obj->value[0] == 100)
                        chance = 0;
                chance = URange(1, chance - obj->value[1], 100);
                obj->value[2] = URange(1, obj->value[2] - chance, 99);
                ch->endurance = static_cast<sh_int>(
                        URange(0, ch->endurance - number_range(150, 500),
                               ch->max_endurance));
                if (ch->endurance < 0)
                        ch->endurance = static_cast<sh_int>(0);  /* Added in to fix strange bug */
                if (obj->value[2] == 0)
                {
                        act(AtAction,
                            "You struggle, and break free from $p!", ch, obj,
                            NULL, ToChar);
                        act(AtAction,
                            "$n struggles, and breaks free from $p!", ch, obj,
                            NULL, ToRoom);
                        unequip_char(ch, obj);
                        make_scraps(obj);
                        WaitState(ch, 3);
                        return;
                }
                else
                {
                        act(AtAction, "You struggle, but $p holds strong.",
                            ch, obj, NULL, ToChar);
                        act(AtAction,
                            "$n struggles, to no avail, against $p.", ch, obj,
                            NULL, ToRoom);
                        return;
                }
        }
}
CMDF do_hold_person(CharData * ch, const char *argument)
{
        char      buf[MaxStringLength];
        CharData *victim = NULL;

        if (argument[0] == '\0')
        {
                send_to_char("Whom do you wish to grab ahold of?\n\r", ch);
                return;
        }
        if (ch->holding)
        {
                send_to_char("But you are already holding someone!\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, const_cast<char*>(argument))) == NULL)
        {
                sprintf(buf, "You can't find %s.\n\r", argument);
                send_to_char(buf, ch);
                return;
        }
		if (ch == victim)
		{
			send_to_char("You love yourself that much, huh?\n\r", ch);
			return;
		}
        if (IsImmortal(victim) && !IsImmortal(ch))
        {
                send_to_char("You can't do that.", ch);
                return;
        }
        if (victim->holding != NULL)
        {
                act(AtAction, "But $E is holding someone. Try MURDERING $M.",
                    ch, NULL, victim, ToChar);
                return;
        }

        if (IsNpc(victim) && victim->pIndexData->pShop != NULL)
        {
                send_to_char
                        ("The shopkeeper struggles too much for you to get a good hold!\n\r",
                         ch);
                return;
        }

        act(AtAction, "You grab ahold of $N!", ch, NULL, victim, ToChar);
        act(AtAction, "$n grabs ahold of $N!", ch, NULL, victim, ToNotvict);
        act(AtAction, "$n grabs ahold of you!", ch, NULL, victim, ToVict);
        ch->holding = victim;
        victim->heldby = ch;
        victim->held = TRUE;
        victim->position = PosStanding;
        victim->master = ch;
        victim->leader = ch;
        WaitState(ch, static_cast<sh_int>(number_range(StruggleWaitMin, StruggleWaitMax)));
        return;
}

CMDF do_struggle(CharData * ch)
{
        CharData *holder = NULL;
        ObjData *obj = NULL;
        int       chance = 0, diff_str = 0;

        if (!ch->held)
        {
                send_to_char("You have nothing to struggle against.\n\r", ch);
                return;
        }
        if ((obj = get_eq_char(ch, WearBinding)) != NULL)
        {
                do_struggle_binding(ch);
                return;
        }
        if (!ch->heldby)
        {
                ch->held = FALSE;
                ch->heldby = NULL;
                ch->master = NULL;
                ch->leader = NULL;
                do_struggle(ch);
                return;
        }
        else
                holder = ch->heldby;

        if (ch->endurance < MinStruggleEndurance)
        {
                send_to_char("You are too tired to struggle more.\n\r", ch);
                return;
        }
        if ((diff_str = (get_curr_str(ch) - get_curr_str(holder))) > 3)
        {
                act(AtAction,
                    "With little effort, you break free of $N's grip on you.",
                    ch, NULL, holder, ToChar);
                act(AtAction,
                    "With little effort, $n breaks free of your grip!", ch,
                    NULL, holder, ToVict);
                act(AtAction,
                    "With little effort, $n breaks free of $N's grip.", ch,
                    NULL, holder, ToNotvict);
                ch->leader = NULL;
                ch->heldby = NULL;
                ch->held = FALSE;
                holder->holding = NULL;
                ch->master = NULL;
                WaitState(holder, static_cast<sh_int>(number_range(2, 7)));
                return;
        }
        else
        {
                chance = URange(0,
                                ((get_curr_str(ch) + get_curr_dex(ch)) +
                                 number_range(10, 40)), 100);
                if (number_percent() > chance)
                {
                        act(AtAction,
                            "You struggle against $N's grip on you, to no avail.",
                            ch, NULL, holder, ToChar);
                        act(AtAction,
                            "$n struggles against your grip on $m, but fails to escape.",
                            ch, NULL, holder, ToVict);
                        act(AtAction,
                            "$n struggles against $N's grip on $m, but $N holds strong.",
                            ch, NULL, holder, ToNotvict);
                        ch->endurance = static_cast<sh_int>(
                                URange(0,
                                       ch->endurance - number_range(120, 700),
                                       ch->max_endurance));
                        WaitState(ch, static_cast<sh_int>(number_range(5, 12)));
                        return;
                }
                else
                {
                        act(AtAction,
                            "You struggle against $N's grip, breaking free!",
                            ch, NULL, holder, ToChar);
                        act(AtAction,
                            "$n struggles against $N's grip, breaking free!",
                            ch, NULL, holder, ToNotvict);
                        act(AtAction,
                            "$n struggles against your grip, breaking free!",
                            ch, NULL, holder, ToVict);
                        ch->heldby = NULL;
                        ch->held = FALSE;
                        holder->holding = NULL;
                        ch->master = NULL;
                        ch->leader = NULL;
                        WaitState(holder, static_cast<sh_int>(number_range(2, 7)));
                        return;
                }
        }
}
CMDF do_unbind(CharData * ch, [[maybe_unused]] const char *argument)
{
        CharData *victim = NULL;
        ObjData *obj = NULL;
        char      arg[MaxInputLength];
        int       keycode = 0;
        bool      keybind = FALSE;


        if (argument[0] == '\0')
        {
                send_to_char("Usage: unbind <target> <binding code>\n\r", ch);
                return;
        }
        char *mutable_argument = const_cast<char*>(argument);
        mutable_argument = one_argument(mutable_argument, arg);
        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("You don't see anyone like that here.\n\r", ch);
                return;
        }
        if ((obj = get_eq_char(victim, WearBinding)) == NULL)
        {
                act(AtAction, "$n, $E doesn't seem to be bound.", ch, NULL,
                    victim, ToChar);
                return;
        }
        if (obj->value[4] != 0)
        {
                keybind = TRUE;
                keycode = atoi(argument);
        }
        if (obj->value[3] != keycode)
        {
                act(AtAction, "Thats not the right code to unlock $N's $p.",
                    ch, obj, victim, ToChar);
                act(AtAction,
                    "$n presses a few buttons on $p, but nothing happens.",
                    ch, obj, victim, ToVict);
                act(AtAction,
                    "$n presses a few buttons on $N's $p, but nothing happens.",
                    ch, obj, victim, ToNotvict);
                return;
        }
        else
        {
                unequip_char(ch, obj);
                separate_obj(obj);
                obj_from_char(obj);
                obj_to_char(obj, ch);
                if (victim->heldby == NULL)
                        victim->held = FALSE;
                if (keybind)
                {
                        act(AtAction,
                            "You enter the code to unlock $p on $N.", ch, obj,
                            victim, ToChar);
                        act(AtAction,
                            "$n presses a few buttons on $p, and it unlocks!",
                            ch, obj, victim, ToVict);
                        act(AtAction,
                            "$n presses a few buttons on $p, and it unlocks.",
                            ch, obj, victim, ToNotvict);
                        return;
                }
                else
                {
                        act(AtAction, "You unbind $p from $N.", ch, obj,
                            victim, ToChar);
                        act(AtAction, "$n unbinds $p from $N.", ch, obj,
                            victim, ToNotvict);
                        act(AtAction, "$n unbinds you, removing $p!", ch,
                            obj, victim, ToVict);
                        return;
                }
        }
}



CMDF do_subdue(CharData * ch, [[maybe_unused]] const char *argument)
{
        char      buf[MaxStringLength];
        CharData *victim = NULL;
        int       strain_amount = 0;

        if (argument[0] == '\0')
        {
                send_to_char("Who do you want to subdue?\n\r", ch);
                return;
        }
        if ((victim = get_char_room(ch, const_cast<char*>(argument))) == NULL)
        {
                sprintf(buf, "You don't see any %s nearby.\n\r", argument);
                send_to_char(buf, ch);
                return;
        }
        if (!victim->held)
        {
                act(AtAction, "But $E isn't held or bound.", ch, NULL,
                    victim, ToChar);
                return;
        }
        if (victim->endurance < MinSubdueEndurance)
        {
                act(AtAction, "$N looks pretty subdued already.", ch, NULL,
                    victim, ToChar);
                return;
        }
        if ((strain_amount =
             (get_curr_str(ch) + get_curr_dex(ch) + (ch->endurance / 10))) ==
            0)
        {
                bug("%s returning 0 value for strain_amount.", ch->name);
                return;
        }
        else
        {
                victim->endurance = static_cast<sh_int>(
                        URange(0,
                               victim->endurance - (strain_amount +
                                                    number_range(strain_amount
                                                                 / 5,
                                                                 strain_amount
                                                                 / 2)),
                               victim->max_endurance));
                act(AtAction, "You rough $M up, and $E looks weaker.", ch,
                    NULL, victim, ToChar);
                act(AtAction, "$n roughs you up, and you feel drained!", ch,
                    NULL, victim, ToVict);
                act(AtAction, "$n roughs $N up, and $E looks weaker.", ch,
                    NULL, victim, ToNotvict);
                WaitState(ch, static_cast<sh_int>(number_range(4, 8)));
                return;
        }
}
CMDF do_bind(CharData * ch, [[maybe_unused]] const char *argument)
{
        ObjData *obj = NULL;
        CharData *victim = NULL;
        char      arg[MaxInputLength];
        int       keycode = 0;
        bool      keylock = FALSE;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: bind <victim>\n\r", ch);
                send_to_char
                        ("Note: You must be holding something to bind them with.\n\r",
                         ch);
                return;
        }
        char *mutable_argument = const_cast<char*>(argument);
        mutable_argument = one_argument(mutable_argument, arg);
        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("You don't see anyone like that nearby.\n\r",
                             ch);
                return;
        }
        if (!victim->held && IsAwake(victim) && victim->endurance > MinStruggleEndurance)
        {
                act(AtAction,
                    "Well, $N seems a little too lively to be bound.", ch,
                    NULL, victim, ToChar);
                return;
        }
        if (victim->held == TRUE && victim->endurance > MinStruggleEndurance)
        {
                act(AtAction, "Try subduing $M first.", ch, NULL, victim,
                    ToChar);
                return;
        }
        if ((obj = get_eq_char(ch, WearHold)) == NULL
            || obj->item_type != ItemBinding)
        {
                act(AtAction,
                    "You need to be holding something to bind $M with.", ch,
                    NULL, victim, ToChar);
                return;
        }

        else
        {
                if (obj->value[4] > 0)
                {
                        keylock = TRUE;
                        if (argument[0] == '\0')
                                keycode = 0;
                        else
                                keycode = atoi(argument);
                }
                if (keylock)
                        obj->value[3] = keycode;
                unequip_char(ch, obj);
                SetBit(obj->wear_loc, WearHold);
                SetBit(obj->wear_loc, static_cast<sh_int>(ItemWearBinding));
                SetBit(obj->extra_flags, ItemNoremove);
                separate_obj(obj);
                obj_from_char(obj);
                obj = obj_to_char(obj, victim);
                equip_char(victim, obj, WearBinding);
                victim->held = TRUE;
                if (victim->heldby)
                {
                        victim->heldby->holding = NULL;
                        victim->heldby = NULL;
                }
                act(AtAction, "You bind $M up with $p.", ch, obj, victim,
                    ToChar);
                act(AtAction, "$n binds you up with $p.", ch, obj, victim,
                    ToVict);
                act(AtAction, "$n binds $N up with $p.", ch, obj, victim,
                    ToNotvict);
                WaitState(ch, static_cast<sh_int>(number_range(2, 6)));
                return;
        }
}
CMDF do_release(CharData * ch, [[maybe_unused]] const char *argument)
{
        CharData *victim = NULL;
        ObjData *obj = NULL;

        argument = NULL;

        if (ch->holding == NULL)
        {
                send_to_char
                        ("Release what? You're not holding or carrying anyone.\n\r",
                         ch);
                return;
        }

        victim = ch->holding;
        if (ch->in_room != victim->in_room)
        {
                send_to_char("They arn't here", ch);
                return;
        }
        else
        {
                act(AtAction, "You release $N.", ch, NULL, victim, ToChar);
                act(AtAction, "$n releases $N.", ch, NULL, victim,
                    ToNotvict);
                act(AtAction, "$n releases you.", ch, NULL, victim, ToVict);
                if ((obj = get_eq_char(victim, WearBinding)) == NULL)
                        victim->held = FALSE;
                ch->holding = NULL;
                victim->master = NULL;
                victim->heldby = NULL;
                victim->leader = NULL;
                WaitState(ch, BindWaitState);
                WaitState(victim, ReleaseWaitState);
        }
        return;
}
