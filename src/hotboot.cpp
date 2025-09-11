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
 *                           ^     +----- |  / ^     ^ |     | +-\                       *
 *                          / \    |      | /  |\   /| |     | |  \                      *
 *                         /   \   +---   |<   | \ / | |     | |  |                      *
 *                        /-----\  |      | \  |  v  | |     | |  /                      *
 *                       /       \ |      |  \ |     | +-----+ +-/                       *
 *****************************************************************************************
 *                                                                                       *
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson), Levi Beckerson (Whir),           *
 * Michael Ward (Tarl), Erik Wolfe (Dwip), Cameron Carroll (Cam), Cyberfox, Karangi,     *
 * Rathian, Raine, and Adjani. All Rights Reserved.                                      *
 *                                                                                       *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                          SWR Hotboot module                                           *
 ****************************************************************************************/

// System includes
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

// Project includes
#include "mud.hpp"
#include "changes.hpp"
#include "homes.hpp"
#include "olc-shuttle.hpp"
#include "Account.hpp"
#include "channels.hpp"
#include "space2.hpp"

// Constants
#define MaxNest          100
#define FilenameSize     256
#define DirnameSize      100
#define BufferSize       100

// Global variables
static ObjData *rgObjNest[MaxNest];

// External references
extern int port;    /* Port number to be used       */
extern int control, control2, conclient, conjava;
extern RoomIndexData *room_index_hash[MaxKeyHash];

// Function prototypes
bool flush_buffer args((DescriptorData * d, bool fPrompt));
void save_sysdata args((SystemData sys));
void hotboot(bool debug, bool save);
bool is_area_inprogress args((void));
void init_descriptor args((DescriptorData * dnew, int desc));
bool write_to_descriptor(int desc, char *txt, int length);

#ifdef MCCP
bool write_to_descriptor_old(int desc, char *txt, int length);
#endif

// =============================================================================
// Ship Save/Load Functions
// =============================================================================

/*
 * Save the world's ship files
 */
void write_ship(FILE * fp, ShipData * ship)
{
#ifndef HotbootShips
        return;
#endif
        if (!fp)
                return;
        /*
         * What other conditions? 
         */
        /*
         * How about only ones that landed 
         */
        if (ship->shipstate == ShipDocked)
                return;
        if (ship->ship_class > ShipPlatform)
                return;

        fprintf(fp, "%s", "#SHIP\n");
        fprintf(fp, "Shipfname	%s~\n", ship->filename);
        if (ship->currspeed != 0)
                fprintf(fp, "Currspeed %d\n", ship->currspeed);
        if (ship->chaff != ship->maxchaff)
                fprintf(fp, "Chaff %d\n", ship->chaff);
        if (ship->shield != ship->maxshield)
                fprintf(fp, "Shield %d\n", ship->shield);
        if (ship->missiles != ship->maxmissiles)
                fprintf(fp, "Missiles %d\n", ship->missiles);
        if (ship->torpedos != ship->maxtorpedos)
                fprintf(fp, "Torpedos %d\n", ship->torpedos);
        if (ship->rockets != ship->maxrockets)
                fprintf(fp, "Rockets %d\n", ship->rockets);
        fprintf(fp, "Autorecharge %d\n", ship->autorecharge);
        fprintf(fp, "Autotrack %d\n", ship->autotrack);
        fprintf(fp, "Autospeed %d\n", ship->autospeed);
        fprintf(fp, "Autopilot %d\n", ship->autopilot);
        fprintf(fp, "Hatchopen %d\n", (int) ship->hatchopen);

        fprintf(fp, "VX %d\n", (int) ship->vx);
        fprintf(fp, "VY %d\n", (int) ship->vy);
        fprintf(fp, "VZ %d\n", (int) ship->vz);

        fprintf(fp, "HX %d\n", (int) ship->hx);
        fprintf(fp, "HY %d\n", (int) ship->hy);
        fprintf(fp, "HZ %d\n", (int) ship->hz);

        fprintf(fp, "JX %d\n", (int) ship->jx);
        fprintf(fp, "JY %d\n", (int) ship->jy);
        fprintf(fp, "JZ %d\n", (int) ship->jz);

        if (ship->target0 && ship->target0->name)
                fprintf(fp, "Target0 %s~\n", ship->target0->name);
        if (ship->target1 && ship->target1->name)
                fprintf(fp, "Target1 %s~\n", ship->target1->name);
        if (ship->target2 && ship->target2->name)
                fprintf(fp, "Target2 %s~\n", ship->target2->name);
        if (ship->dockedto && ship->dockedto->name)
                fprintf(fp, "Dockedto %s~\n", ship->dockedto->name);
        fprintf(fp, "Flags %d\n", ship->flags);

        fprintf(fp, "Shipstate %d\n", ship->shipstate);
        if (ship->dest != NULL && ship->dest[0] != '\0')
        {
                fprintf(fp, "LandDest %s~\n", ship->dest);
        }

        if (ship->shipstate == ShipHyperspace)
        {
                fprintf(fp, "Hyperdistance %d\n", ship->hyperdistance);
                fprintf(fp, "Currjump %s~\n", ship->currjump->name);
        }
        if (ship->energy != ship->maxenergy)
                fprintf(fp, "Energy %d\n", ship->energy);
        if (ship->hull != ship->maxhull)
                fprintf(fp, "Hull %d\n", ship->hull);

        if (ship->starsystem)
                fprintf(fp, "Starsystem %s~\n", ship->starsystem->name);
        fprintf(fp, "%s", "EndShip\n\n");
        return;
}

// =============================================================================
// Channel History Save/Load Functions
// =============================================================================

void load_oochistory(void)
{
        FILE     *fp;
        int       i, ccount = 0, x;
        ChannelData *channel;


        if (!(fp = fopen(OochistoryFile, "r")))
        {
                bug("Could not open OOChistory File for reading.", 0);
                return;
        }
        ccount = fread_number(fp);
        for (x = 0; x < ccount; x++)
        {
                channel = get_channel(fread_string_noalloc(fp));
                channel->logpos = fread_number(fp);
                CREATE(channel->log, LogData, sysdata.channellog);

                for (i = 0; i < channel->logpos + 1; i++)
                {
                        channel->log[i].name = fread_string(fp);
                        channel->log[i].message = fread_string_nohash(fp);
                        channel->log[i].time = fread_number(fp);
                        channel->log[i].language =
                                get_language(fread_string_noalloc(fp));
                }
                continue;
        }
        FCLOSE(fp);
        unlink(OochistoryFile);
        return;
}

void save_oochistory(void)
{
        FILE     *fp;
        int       i, ccount = 0;
        ChannelData *channel;

        if (!(fp = fopen(OochistoryFile, "w")))
        {
                perror(OochistoryFile);
                return;
        }

        for (channel = first_channel; channel; channel = channel->next)
                if (channel->history && channel->log)
                        ccount++;
        fprintf(fp, "%d\n", ccount);
        for (channel = first_channel; channel; channel = channel->next)
        {
                if (!channel->history || !channel->log)
                        continue;

                fprintf(fp, "%s~\n", channel->name);
                fprintf(fp, "%d\n", channel->logpos);
                for (i = 0; i <= channel->logpos; i++)
                {
                        if (!channel->log[i].name ||
                            channel->log[i].name[0] == '\0' ||
                            !channel->log[i].message ||
                            channel->log[i].message[0] == '\0')
                                continue;
                        fprintf(fp, "%s~\n", channel->log[i].name);
                        fprintf(fp, "%s~\n", channel->log[i].message);
                        fprintf(fp, "%ld\n", channel->log[i].time);
                        fprintf(fp, "%s~\n", channel->log[i].language->name);
                }

        }
        fprintf(fp, "\n");
        FCLOSE(fp);
        return;
}

ShipData *load_ship(FILE * fp)
{
        ShipData *ship = NULL;
        const char *word;
        bool      fMatch;

#ifndef HotbootShips
        return NULL;
#endif
        word = feof(fp) ? "EndShip" : fread_word(fp);
        if (!str_cmp(word, "EndShip"))
                return NULL;

        if (!str_cmp(word, "SHIPFNAME"))
        {
                char     *name = fread_string_nohash(fp);   /* is this right? - Gavin */
                ShipData *temp_ship = NULL;

                for (temp_ship = first_ship; temp_ship;
                     temp_ship = temp_ship->next)
                {
                        if (!str_cmp(name, temp_ship->filename))
                        {
                                ship = temp_ship;
                                break;
                        }
                }

                if (ship == NULL)
                {
                        bug("load_ship: No ship data for filename %s", name);
                        DISPOSE(name);
                        
                        // Skip to end of ship data
                        for (;;)
                        {
                                word = feof(fp) ? "EndShip" : fread_word(fp);
                                if (!str_cmp(word, "EndShip"))
                                        break;
                        }
                        return NULL;
                }

                DISPOSE(name);
        }
        else
        {
                for (;;)
                {
                        word = feof(fp) ? "EndShip" : fread_word(fp);
                        if (!str_cmp(word, "EndShip"))
                                break;
                }
                bug("load_ship: shipfname not found");
                return NULL;
        }
        for (;;)
        {
                word = feof(fp) ? "EndShip" : fread_word(fp);
                fMatch = FALSE;
                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;
                        /*
                         * case '#':
                         * if ( !str_cmp( word, "#TORPEDO" ) )
                         * {
                         * mob->tempnum = -9999; 
                         * fread_obj( mob, fp, OsCarry );
                         * }
                         */
                case 'A':
                        KEY("Autorecharge", ship->autorecharge,
                            (bool) fread_number(fp));
                        KEY("Autotrack", ship->autotrack,
                            (bool) fread_number(fp));
                        KEY("Autospeed", ship->autospeed,
                            (bool) fread_number(fp));
                        KEY("Autopilot", ship->autopilot,
                            (bool) fread_number(fp));
                        break;
                case 'C':
                        KEY("Currspeed", ship->currspeed, fread_number(fp));
                        if (!str_cmp(word, "Currjump"))
                        {
                                char     *temp = fread_string_nohash(fp);

                                extract_ship(ship);
                                ship->location = 0;
                                ship->currjump = starsystem_from_name(temp);
                                DISPOSE(temp);
                                fMatch = TRUE;
                                break;
                        }
                        KEY("Chaff", ship->chaff, fread_number(fp));
                        break;

                case 'D':
                        KEY("Dockedto", ship->dockedto,
                            get_ship(fread_string_noalloc(fp)));
                        break;

                case 'E':
                        KEY("Energy", ship->energy, fread_number(fp));
                        if (!str_cmp(word, "EndShip"))
                        {
                                return ship;
                        }
                        if (!str_cmp(word, "End"))  /* End of object, need to ignore this. sometimes they creep in there somehow -- Scion */
                                fMatch = TRUE;  /* Trick the system into thinking it matched something */
                        break;

                case 'F':
                        KEY("Flags", ship->flags, fread_number(fp));
                        break;

                case 'H':
                        KEY("Hull", ship->hull, fread_number(fp));
                        KEY("HX", ship->hx, (float) fread_number(fp));
                        KEY("HY", ship->hy, (float) fread_number(fp));
                        KEY("HZ", ship->hz, (float) fread_number(fp));
                        KEY("Hyperdistance", ship->hyperdistance,
                            fread_number(fp));
                        KEY("Hatchopen", ship->hatchopen,
                            (bool) fread_number(fp));
                        break;

                case 'J':
                        KEY("JX", ship->jx, (float) fread_number(fp));
                        KEY("JY", ship->jy, (float) fread_number(fp));
                        KEY("JZ", ship->jz, (float) fread_number(fp));
                        break;
                case 'L':
                        if (!str_cmp(word, "Landdest"))
                        {
                                if (ship->dest != NULL)
                                        STRFREE(ship->dest);
                                ship->dest = fread_string(fp);
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'M':
                        KEY("Missiles", ship->missiles, fread_number(fp));
                        break;
                case 'R':
                        KEY("Rockets", ship->rockets, fread_number(fp));
                        break;
                case 'S':
                        KEY("Shield", ship->shield, fread_number(fp));
                        KEY("Shipstate", ship->shipstate, fread_number(fp));

                        if (!str_cmp(word, "Starsystem"))
                        {
                                char     *star_name = fread_string_nohash(fp);
                                SpaceData *starsystem =
                                        starsystem_from_name(star_name);
                                DISPOSE(star_name);
                                fMatch = TRUE;

                                extract_ship(ship);
                                ship->location = 0;
                                if (ship->starsystem)
                                        ship_from_starsystem(ship,
                                                             starsystem);
                                ship_to_starsystem(ship, starsystem);
                                break;
                        }
                        break;
                case 'T':
                        KEY("Torpedos", ship->torpedos, fread_number(fp));
                        KEY("Target0", ship->target0,
                            get_ship(fread_string_noalloc(fp)));
                        KEY("Target1", ship->target1,
                            get_ship(fread_string_noalloc(fp)));
                        KEY("Target2", ship->target2,
                            get_ship(fread_string_noalloc(fp)));
                        break;

                case 'V':
                        KEY("VX", ship->vx, (float) fread_number(fp));
                        KEY("VY", ship->vy, (float) fread_number(fp));
                        KEY("VZ", ship->vz, (float) fread_number(fp));
                        break;

                }
                if (!fMatch && str_cmp(word, "End"))
                        bug("load_ship: no match: %s", word);
        }
        return NULL;
}

// =============================================================================
// Mobile Save/Load Functions
// =============================================================================

void save_mobile(FILE * fp, CharData * mob)
{
        AffectData *paf;
        SkillType *skill = NULL;

        if (!IsNpc(mob) || !fp)
                return;
        fprintf(fp, "%s", "#MOBILE\n");
        fprintf(fp, "Vnum	%d\n", mob->pIndexData->vnum);
        fprintf(fp, "Level   %d\n", mob->top_level);
        fprintf(fp, "Gold	%ld\n", mob->gold);
        if (mob->in_room)
        {
                if (IsSet(mob->act, ActSentinel))
                {
                        /*
                         * Sentinel mobs get stamped with a "home room" when they are created
                         * by create_mobile(), so we need to save them in their home room regardless
                         * of where they are right now, so they will go to their home room when they
                         * enter the game from a reboot or copyover -- Scion 
                         */
                        fprintf(fp, "Room	%d\n", mob->home_vnum);
                }
                else
                        fprintf(fp, "Room	%d\n", mob->in_room->vnum);
        }
        else
                fprintf(fp, "Room	%d\n", RoomVnumLimbo);
#ifdef OVERLANDCODE
        fprintf(fp, "Coordinates  %d %d %d\n", mob->x, mob->y, mob->map);
#endif
        if (QUICKMATCH(mob->name, mob->pIndexData->PlayerName) == 0)
                fprintf(fp, "Name     %s~\n", mob->name);
        if (QUICKMATCH(mob->short_descr, mob->pIndexData->short_descr) == 0)
                fprintf(fp, "Short	%s~\n", mob->short_descr);
        if (QUICKMATCH(mob->long_descr, mob->pIndexData->long_descr) == 0)
                fprintf(fp, "Long	%s~\n", mob->long_descr);
        if (QUICKMATCH(mob->description, mob->pIndexData->description) == 0)
                fprintf(fp, "Description %s~\n", mob->description);
        if (QUICKMATCH(mob->mob_clan, mob->pIndexData->mob_clan) == 0)
                fprintf(fp, "Mobclan %s~\n", mob->mob_clan);
        fprintf(fp, "HpEndurance   %d %d %d %d\n",
                mob->hit, mob->max_hit, mob->endurance, mob->max_endurance);
        fprintf(fp, "Position %d\n", mob->position);
        if (mob->master && mob->master->name)
                fprintf(fp, "Master %s~\n", mob->master->name);
        if (mob->leader && mob->leader->name)
                fprintf(fp, "Leader %s~\n", mob->leader->name);
        fprintf(fp, "Flags %d\n", mob->act);
        if (mob->affected_by)
                fprintf(fp, "AffectedBy   %d\n", mob->affected_by);

        for (paf = mob->first_affect; paf; paf = paf->next)
        {
                if (paf->type >= 0
                    && (skill = get_skilltype(paf->type)) == NULL)
                        continue;

                if (paf->type >= 0 && paf->type < TypePersonal)
                        fprintf(fp, "AffectData   '%s' %3d %3d %3d %d\n",
                                skill->name, paf->duration, paf->modifier,
                                paf->location, paf->bitvector);
                else
                        fprintf(fp, "Affect       %3d %3d %3d %3d %d\n",
                                paf->type, paf->duration, paf->modifier,
                                paf->location, paf->bitvector);
        }

        de_equip_char(mob);

        if (mob->first_carrying)
                fwrite_obj(mob, mob->last_carrying, fp, 0, OsCarry, TRUE);

        re_equip_char(mob);

        fprintf(fp, "%s", "EndMobile\n\n");
        return;
}

// =============================================================================
// World State Save/Load Functions
// =============================================================================

void save_world(CharData * ch)
{
        FILE     *mobfp;
        FILE     *shipfp;
        FILE     *objfp;
        int       mobfile = 0;
        int       shipfile = 0;
        char      filename[FilenameSize];
        CharData *rch;
        RoomIndexData *pRoomIndex;
        int       iHash;

        ch = NULL;
        log_string("Preserving world state....");

        snprintf(filename, FilenameSize, "%s%s", SystemDir, MobFile);
        if ((mobfp = fopen(filename, "w")) == NULL)
        {
                bug("%s", "save_world: fopen mob file");
                perror(filename);
        }
        else
                mobfile++;

        snprintf(filename, FilenameSize, "%s%s", SystemDir, ShipFile);
        if ((shipfp = fopen(filename, "w")) == NULL)
        {
                bug("%s", "save_world: fopen ship file");
                perror(filename);
        }
        else
                shipfile++;

        for (iHash = 0; iHash < MaxKeyHash; iHash++)
        {
                for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
                     pRoomIndex = pRoomIndex->next)
                {
                        if (pRoomIndex)
                        {
                                if (!pRoomIndex->first_content  /* Skip room if nothing in it */
#ifdef OlcHomes
                                    || pRoomIndex->home
#endif
                                    || xIS_SET(pRoomIndex->RoomFlags, RoomClanstoreroom)  /* These rooms save on their own */
                                    || xIS_SET(pRoomIndex->RoomFlags, RoomPlrHome))
                                        continue;

                                snprintf(filename, FilenameSize, "%s%d", HotbootDir,
                                         pRoomIndex->vnum);
                                if ((objfp = fopen(filename, "w")) == NULL)
                                {
                                        bug("save_world: fopen %d",
                                            pRoomIndex->vnum);
                                        perror(filename);
                                        continue;
                                }
                                fwrite_obj(NULL, pRoomIndex->last_content,
                                           objfp, 0, OsCarry, TRUE);
                                fprintf(objfp, "%s", "#END\n");
                                FCLOSE(objfp);
                        }
                }
        }

        if (mobfile)
        {
                for (rch = first_char; rch; rch = rch->next)
                {
                        if (!IsNpc(rch) || rch == supermob
                            || IsSet(rch->act, ActPrototype)
                            || IsSet(rch->act, ActPet)
                            || rch->owner != NULL)
                                continue;
                        else
                                save_mobile(mobfp, rch);
                }
                fprintf(mobfp, "%s", "#END\n");
                FCLOSE(mobfp);
        }

        /*
         * FIXME - Loop Through ships here.
         * * Problem would be to make sure they are uniquely identified, so you don't set 2 ships that are set exactly the same way here.
         * * If its in space. Store the system its in, and its coords.. Current energy too?
         */
#ifdef HotbootShips
        if (shipfile)
        {
                ShipData *ship = NULL;

                for (ship = first_ship; ship; ship = ship->next)
                {
                        write_ship(shipfp, ship);
                }
                fprintf(shipfp, "%s", "#END\n");
                FCLOSE(shipfp); /* This should be here instead of below the brace */
        }

#endif

        /*
         * Save sysdata, so that we have the proper number when we read in oochistory log 
         */
        save_sysdata(sysdata);
        save_oochistory();

        return;
}

CharData *load_mobile(FILE * fp)
{
        CharData *mob = NULL;
        const char *word;
        bool      fMatch;
        int       inroom = 0;
        RoomIndexData *pRoomIndex = NULL;

        word = feof(fp) ? "EndMobile" : fread_word(fp);
        if (!str_cmp(word, "Vnum"))
        {
                int       vnum;

                vnum = fread_number(fp);
                if (get_mob_index(vnum) == NULL)
                {
                        bug("load_mobile: No index data for vnum %d", vnum);
                        return NULL;
                }
                mob = create_mobile(get_mob_index(vnum));
                if (!mob)
                {
                        for (;;)
                        {
                                word = feof(fp) ? "EndMobile" :
                                        fread_word(fp);
                                /*
                                 * So we don't get so many bug messages when something messes up
                                 * * --Shaddai 
                                 */
                                if (!str_cmp(word, "EndMobile"))
                                        break;
                        }
                        bug("load_mobile: Unable to create mobile for vnum %d", vnum);
                        return NULL;
                }
        }
        else
        {
                for (;;)
                {
                        word = feof(fp) ? "EndMobile" : fread_word(fp);
                        /*
                         * So we don't get so many bug messages when something messes up
                         * * --Shaddai 
                         */
                        if (!str_cmp(word, "EndMobile"))
                                break;
                }
                extract_char(mob, TRUE);
                bug("%s", "load_mobile: Vnum not found");
                return NULL;
        }
        for (;;)
        {
                word = feof(fp) ? "EndMobile" : fread_word(fp);
                fMatch = FALSE;
                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;
                case '#':
                        if (!str_cmp(word, "#OBJECT"))
                        {
                                mob->tempnum = -9999;   /* Hackish, yes. Works though doesn't it? */
                                fread_obj(mob, fp, OsCarry);
                        }
                case 'A':
                        if (!str_cmp(word, "Affect")
                            || !str_cmp(word, "AffectData"))
                        {
                                AffectData *paf;

                                CREATE(paf, AffectData, 1);
                                if (!str_cmp(word, "Affect"))
                                {
                                        paf->type = fread_number(fp);
                                }
                                else
                                {
                                        int       sn;
                                        char     *sname = fread_word(fp);

                                        if ((sn = skill_lookup(sname)) < 0)
                                                bug("%s",
                                                    "load_mobile: unknown skill.");
                                        else
                                                paf->type = sn;
                                }

                                paf->duration = fread_number(fp);
                                paf->modifier = fread_number(fp);
                                paf->location = fread_number(fp);
                                if (paf->location == ApplyWeaponspell
                                    || paf->location == ApplyWearspell
                                    || paf->location == ApplyRemovespell
                                    || paf->location == ApplyStripsn)
                                        paf->modifier =
                                                slot_lookup(paf->modifier);
                                paf->bitvector = fread_number(fp);
                                LINK(paf, mob->first_affect, mob->last_affect,
                                     next, prev);
                                fMatch = TRUE;
                                break;
                        }
                        KEY("AffectedBy", mob->affected_by, fread_number(fp));
                        break;
#ifdef OVERLANDCODE
                case 'C':
                        if (!str_cmp(word, "Coordinates"))
                        {
                                mob->x = fread_number(fp);
                                mob->y = fread_number(fp);
                                mob->map = fread_number(fp);

                                fMatch = TRUE;
                                break;
                        }
                        break;
#endif
                case 'D':
                        KEY("Description", mob->description,
                            fread_string(fp));
                        break;
                case 'E':
                        if (!str_cmp(word, "EndMobile"))
                        {
                                if (inroom == 0)
                                        inroom = RoomVnumLimbo;
                                pRoomIndex = get_room_index(inroom);
                                if (!pRoomIndex)
                                        pRoomIndex =
                                                get_room_index
                                                (RoomVnumLimbo);
                                char_to_room(mob, pRoomIndex);
                                mob->tempnum = -9998;   /* Yet another hackish fix! */
                                return mob;
                        }
                        if (!str_cmp(word, "End"))  /* End of object, need to ignore this. sometimes they creep in there somehow -- Scion */
                                fMatch = TRUE;  /* Trick the system into thinking it matched something */
                        break;
                case 'F':
                        KEY("Flags", mob->act, fread_number(fp));
                case 'G':
                        KEY("Gold", mob->gold, fread_number(fp));
                        break;
                case 'H':
                        if (!str_cmp(word, "HpEndurance"))
                        {
                                mob->hit = fread_number(fp);
                                mob->max_hit = fread_number(fp);
                                mob->endurance = fread_number(fp);
                                mob->max_endurance = fread_number(fp);

                                if (mob->max_endurance <= 0)
                                        mob->max_endurance = 150;

                                fMatch = TRUE;
                                break;
                        }
                        break;
                case 'L':
                        KEY("Long", mob->long_descr, fread_string(fp));
                        KEY("Level", mob->top_level, fread_number(fp));
                        KEY("Leader", mob->groupleader, fread_string(fp));
                        break;
                case 'M':
                        KEY("Mobclan", mob->mob_clan, fread_string(fp));
                        KEY("Master", mob->following, fread_string(fp));
                        break;
                case 'N':
                        KEY("Name", mob->name, fread_string(fp));
                        break;
                case 'P':
                        KEY("Position", mob->position, fread_number(fp));
                        break;
                case 'R':
                        KEY("Room", inroom, fread_number(fp));
                        break;
                case 'S':
                        KEY("Short", mob->short_descr, fread_string(fp));
                        break;
                }
                if (!fMatch && str_cmp(word, "End"))
                        bug("load_mobile: no match: %s", word);
        }
        return NULL;
}

// =============================================================================
// Object File Handling
// =============================================================================

void read_obj_file(char *dirname, char *filename)
{
        RoomIndexData *room;
        FILE     *fp;
        char      fname[FilenameSize];
        int       vnum;

        vnum = atoi(filename);
        snprintf(fname, FilenameSize, "%s%s", dirname, filename);
        if ((room = get_room_index(vnum)) == NULL)
        {
                bug("read_obj_file: ARGH! Missing room index for %d!", vnum);
                unlink(fname);
                return;
        }


        if ((fp = fopen(fname, "r")) != NULL)
        {
                sh_int    iNest;
                bool      found;
                ObjData *tobj, *tobj_next;

                rset_supermob(room);
                for (iNest = 0; iNest < MaxNest; iNest++)
                        rgObjNest[iNest] = NULL;

                found = TRUE;
                for (;;)
                {
                        char      letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("%s", "read_obj_file: # not found.");
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "OBJECT"))   /* Objects  */
                                fread_obj(supermob, fp, OsCarry);
                        else if (!str_cmp(word, "END")) /* Done     */
                                break;
                        else
                        {
                                bug("read_obj_file: bad section: %s", word);
                                break;
                        }
                }
                FCLOSE(fp);
                unlink(fname);
                for (tobj = supermob->first_carrying; tobj; tobj = tobj_next)
                {
                        tobj_next = tobj->next_content;
#ifdef OVERLANDCODE
                        if (IsObjStat(tobj, ItemOnmap))
                        {
                                SetActFlag(supermob, ActOnmap);
                                supermob->map = tobj->map;
                                supermob->x = tobj->x;
                                supermob->y = tobj->y;
                        }
#endif
                        obj_from_char(tobj);
#ifndef OVERLANDCODE
                        obj_to_room(tobj, room);
#else
                        obj_to_room(tobj, room, supermob);
                        RemoveActFlag(supermob, ActOnmap);
                        supermob->map = -1;
                        supermob->x = -1;
                        supermob->y = -1;
#endif
                }
                release_supermob();
        }
        else
                log_string("Cannot open obj file");

        return;
}

void load_obj_files(void)
{
        DIR      *dp;
        struct dirent *dentry;
        char      directory_name[DirnameSize];

        boot_log("World state: loading objs");
        snprintf(directory_name, DirnameSize, "%s", HotbootDir);
        dp = opendir(directory_name);
        dentry = readdir(dp);
        while (dentry)
        {
                /*
                 * Added by Tarl 3 Dec 02 because we are now using CVS 
                 */
                if (!str_cmp(dentry->d_name, "CVS"))
                {
                        dentry = readdir(dp);
                        continue;
                }
                if (dentry->d_name[0] != '.')
                        read_obj_file(directory_name, dentry->d_name);
                dentry = readdir(dp);
        }
        closedir(dp);
        return;
}

void load_world(CharData * ch)
{
        FILE     *mobfp;
        FILE     *shipfp;
        char      file1[FilenameSize];
        char      file2[FilenameSize];
        char     *word;
        int       done = 0;
        bool      mobfile = FALSE;
        bool      shipfile = FALSE;

        ch = NULL;

        snprintf(file1, FilenameSize, "%s%s", SystemDir, MobFile);
        if ((mobfp = fopen(file1, "r")) == NULL)
        {
                bug("%s", "load_world: fopen mob file");
                perror(file1);
        }
        else
                mobfile = TRUE;

        snprintf(file2, FilenameSize, "%s%s", SystemDir, ShipFile);
        if ((shipfp = fopen(file2, "r")) == NULL)
        {
                bug("%s", "load_world: fopen ship file");
                perror(file1);
        }
        else
                shipfile = TRUE;

        if (mobfile)
        {
                boot_log("World state: loading mobs");
                while (done == 0)
                {
                        if (feof(mobfp))
                                done++;
                        else
                        {
                                word = fread_word(mobfp);
                                if (str_cmp(word, "#END"))
                                        load_mobile(mobfp);
                                else
                                        done++;
                        }
                }
                FCLOSE(mobfp);
        }

        load_obj_files();

#ifdef HotbootShips
        if (shipfile)
        {
                done = 0;
                boot_log("World state: loading ships");
                while (done == 0)
                {
                        if (feof(shipfp))
                                done++;
                        else
                        {
                                word = fread_word(shipfp);
                                if (str_cmp(word, "#END"))
                                        load_ship(shipfp);
                                else
                                        done++;
                        }
                }
                FCLOSE(shipfp);
                boot_log("World_state:  Done");
        }
#endif

        /*
         * Once loaded, the data needs to be purged in the event it causes a crash so that it won't try to reload 
         */
        unlink(file1);
        unlink(file2);

        boot_log("World_state: Loading Channel History");
        load_oochistory();
        boot_log("World_state:  Done");
        return;
}

// =============================================================================
// Hotboot Command Functions
// =============================================================================

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */
CMDF do_hotboot(CharData * ch, char *argument)
{
        CharData *victim = NULL;
        DescriptorData *d;
        int       count = 0;
        bool      found = FALSE;

        if (compilelock)
        {
                set_char_color(AtRed, ch);
                send_to_char
                        ("Sorry, the mud cannot be shutdown during a compiler operation.\n\rPlease wait for the compiler to finish.\n\r",
                         ch);
                return;
        }

        if (argument[0] != '\0' && str_cmp(argument, "now")
            && str_cmp(argument, "warn")
            && str_cmp(argument, "poscrash")
            && str_cmp(argument, "nosave") && str_cmp(argument, "debug"))
        {
                send_to_char
                        ("Syntax: hotboot (warn/now/nosave/poscrash/debug)\n\r",
                         ch);
                return;
        }

        if (is_area_inprogress())
        {
                send_to_char("Arena in progress", ch);
                return;
        }

        for (d = first_descriptor; d; d = d->next)
        {
                if ((d->connected == ConPlaying
                     || d->connected == ConEditing)
                    && (victim = d->character) != NULL && !IsNpc(victim)
                    && victim->in_room && victim->fighting
                    && victim->top_level >= 1
                    && victim->top_level <= MaxLevel)
                {
                        found = TRUE;
                        count++;
                }
        }

        if (found)
        {
                ch_printf(ch,
                          "Cannot hotboot at this time. There are %d combats in progress.\n\r",
                          count);
                return;
        }

        found = FALSE;
        for (d = first_descriptor; d; d = d->next)
        {
                if (d->connected == ConEditing && d->character)
                {
                        found = TRUE;
                        break;
                }
                if (d->character && NotAuthed(d->character))
                {
                        found = TRUE;
                        break;
                }
        }

        if (found)
        {
                send_to_char
                        ("Cannot hotboot at this time. Someone is using the line editor, or someone isn't authed.\n\r",
                         ch);
                return;
        }

        snprintf(log_buf, MSL, "Hotboot initiated by %s.", ch->name);
        log_string(log_buf);

        if (!str_cmp(argument, "warn"))
        {
                echo_to_all(AtWhite, "Hotboot Warning, commencing soon.",
                            EchoTarAll);
                return;
        }

        if (!str_cmp(argument, "poscrash"))
        {
                echo_to_all(AtWhite,
                            "Possible Crash. Please prepare accordingly.",
                            EchoTarAll);
                return;
        }

        hotboot(!str_cmp(argument, "debug"), !!str_cmp(argument, "nosave"));
        return;
}

// =============================================================================
// Hotboot Core Functions
// =============================================================================

void crash_hotboot(void)
{

        echo_to_all(AtRed,
                    "\n\rReality swirls and changes around you, and things are not quite as they were...\n\r",
                    0);
        snprintf(log_buf, MSL, "%s", "Hotboot initiated by crash.");
        log_string(log_buf);
        hotboot(FALSE, TRUE);
}

void hotboot(bool debug, bool save)
{
        FILE     *fp;
        DescriptorData *d, *de_next;
        char      buf[BufferSize], buf2[BufferSize], buf3[BufferSize];
        AreaData *tarea;
        ShipData *ship;
        PlanetData *planet;
        Timer    *timer, *timer_next;

#ifdef OlcShuttle
        ShuttleData *tshuttle;
#endif
        fp = fopen(HotbootFile, "w");

        if (!fp)
        {
                bug("Could not write to hotboot file: %s. Hotboot aborted.",
                    HotbootFile);
                perror("do_hotboot:fopen");
                return;
        }

        /*
         * And this one here will save the status of all objects and mobs in the game.
         * * This really should ONLY ever be used here. The less we do stuff like this the better.
         */
        save_world(supermob);


        if (save)
        {
                log_string("Saving player files and connection states....");
                /*
                 * load_changes(); - Gavin 
                 */
                save_changes();
                for (tarea = first_area; tarea; tarea = tarea->next)
                        fold_area(tarea, tarea->filename, FALSE, FALSE);
                for (tarea = first_bsort; tarea; tarea = tarea->next_sort)
                        if (IsSet(tarea->status, AreaLoaded))
                                fold_area(tarea, tarea->filename, FALSE,
                                          TRUE);
#ifdef OlcShuttle
                for (tshuttle = first_shuttle; tshuttle;
                     tshuttle = tshuttle->next)
                        save_shuttle(tshuttle);
#endif
#ifdef OlcHomes
                {
                        HomeData *home = NULL;

                        for (home = first_home; home; home = home->next)
                               home->save(); 
                }
#endif
                for (ship = first_ship; ship; ship = ship->next)
                        save_ship(ship);

                for (planet = first_planet; planet; planet = planet->next)
                        save_planet(planet, TRUE);

                save_changes();
                save_world(NULL);
        }

        CheckLinks(first_descriptor, last_descriptor, next, prev,
                    DescriptorData);
        /*
         * Write out all the pulses and times and such. To make sure the copyover is seamless
         * * pulse_area
         * * pulse_taxes
         * * pulse_mobile
         * * pulse_space
         * * pulse_recharge
         * * pulse_ship
         * * pulse_point
         * * pulse_second - Maybe
         * * 
         */
        fprintf(fp, "%d %d %d %d %d %d %d %d\n",
                sysdata.pulse_area,
                sysdata.pulse_taxes,
                sysdata.pulse_mobile,
                sysdata.pulse_space,
                sysdata.pulse_recharge,
                sysdata.pulse_space,
                sysdata.pulse_point, sysdata.pulse_second);

        /*
         * For each playing descriptor, save its state 
         */
        for (d = first_descriptor; d; d = de_next)
        {
                CharData *och = CH(d);

                de_next = d->next;  /* We delete from the list , so need to save this */
                if (!d->character || d->connected < ConPlaying)    /* drop those logging on */
                {
                        write_to_descriptor(d->descriptor,
                                            "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r",
                                            0);
                        close_socket(d, FALSE); /* throw'em out */
                }
                else
                {

                        if (IsImmortal(och) && save)
                                do_savearea(och, "");

                        for (timer = och->first_timer; timer;
                             timer = timer_next)
                        {
                                timer_next = timer->next;
                                if (timer->type == TimerDoFun)
                                {
                                        int       tempsub;

                                        tempsub = och->substate;
                                        och->substate = timer->value;
                                        (timer->do_fun) (och, "");
                                        if (char_died(och))
                                                break;
                                        och->substate = tempsub;
                                }
                                extract_timer(och, timer);
                        }

                        fprintf(fp, "%d %d %d %d %d %s %s %s\n",
                                d->descriptor, och->in_room->vnum,
#ifdef MCCP
                                d->Compressing, /* Do we want MCCP compression? */
#else
                                0,
#endif
                                (int) d->MspDetected,
                                (int) d->MxpDetected,
                                och->name, d->host, d->Client);
                        /*
                         * One of two places this gets changed 
                         */
                        och->pcdata->hotboot = TRUE;
                        save_char_obj(och);
                        save_account(och->pcdata->Account);
                        save_home(och);
#ifdef MCCP
                        compressEnd(d);
#endif
                }
        }
        fprintf(fp, "%s", "-1\n");
        FCLOSE(fp);
        log_string("Executing hotboot....");

        /*
         * added this in case there's a need to debug the contents of the various files 
         */
        if (debug)
        {
                log_string("Hotboot debug - Aborting before execl");
                return;
        }

#ifdef WEB
        if (sysdata.web)
        {

                printf(log_buf, "Shutting down webserver on port %d.",
                       port + 2);
                log_string(log_buf);
                shutdown_web();
        }
#endif
        /*
         * Close reserve and other always-open files and release other resources 
         */
        FCLOSE(fpReserve);
        FCLOSE(fpLOG);

        /*
         * exec - descriptors are inherited 
         */
#ifdef IMC
        imc_hotboot();
#endif
        snprintf(buf, BufferSize, "%d", port);
        snprintf(buf2, BufferSize, "%d", control);
#ifdef IMC
        if (this_imcmud)
                snprintf(buf3, BufferSize, "%d", this_imcmud->desc);
        else
                strncpy(buf3, "-1", BufferSize);
#else
        strncpy(buf3, "-1", BufferSize);
#endif

        /*
         * Uncomment this bfd_close line if you've installed the dlsym snippet, you'll need it. 
         */
        dlclose(sysdata.dlHandle);
        execl(ExeFile, "swr", buf, "hotboot", buf2, buf3, (char *) NULL);

        /*
         * Failed - sucessful exec will not return 
         */
        perror("do_hotboot: execl");

        /*
         * Here you might want to reopen fpReserve 
         */
        /*
         * Since I'm a neophyte type guy, I'll assume this is a good idea and cut and past from main()  
         */

        if ((fpReserve = fopen(NullFile, "r")) == NULL)
        {
                perror(NullFile);
                exit(1);
        }
        if ((fpLOG = fopen(NullFile, "r")) == NULL)
        {
                perror(NullFile);
                exit(1);
        }
        bug("%s", "Hotboot execution failed!!");
}

// =============================================================================
// Hotboot Recovery Functions
// =============================================================================

/* Recover from a hotboot - load players*/
void hotboot_recover()
{
        DescriptorData *d;
        FILE     *fp;
        char      name[100];
        char      Client[100];
        char      host[MaxStringLength];
        int       desc, room;
        bool      fOld;
        int       bCompress;
        int       mxp, msp;

        log_string("Hotboot recovery initiated");

        fp = fopen(HotbootFile, "r");

        if (!fp)    /* there are some descriptors open which will hang forever then ? */
        {
                perror("hotboot_recover: fopen");
                bug("%s", "Hotboot file not found. Exitting.");
                exit(1);
        }

        unlink(HotbootFile);   /* In case something crashes - doesn't prevent reading */
        fscanf(fp, "%d %d %d %d %d %d %d %d\n",
               &sysdata.pulse_area, &sysdata.pulse_taxes,
               &sysdata.pulse_mobile, &sysdata.pulse_space,
               &sysdata.pulse_recharge, &sysdata.pulse_space,
               &sysdata.pulse_point, &sysdata.pulse_second);
        for (;;)
        {
                fscanf(fp, "%d %d %d %d %d %s %s %s \n", &desc, &room,
                       &bCompress, &msp, &mxp, name, host, Client);
                if (desc == -1)
                        break;

                /*
                 * Write something, and check if it goes error-free 
                 */
#ifdef MCCP
                if (!write_to_descriptor_old(desc, " ", 0))
#else
                if (!write_to_descriptor(desc, " ", 0))
#endif
                {
                        close(desc);    /* nope */
                        continue;
                }

                /*
                 * I hate code duplication 
                 */
                if (++num_descriptors > sysdata.maxplayers)
                        sysdata.maxplayers = num_descriptors;
                CREATE(d, DescriptorData, 1);
                init_descriptor(d, desc);   /* set up various stuff */
                if (d->host)
                        STRFREE(d->host);
                d->host = STRALLOC(host);
                if (d->Client)
                        STRFREE(d->Client);
                d->Client = STRALLOC(Client);
                d->MxpDetected = (bool) mxp;
                d->MspDetected = (bool) msp;
                d->host = STRALLOC(host);
                d->IFd = -1;
                d->IPid = -1;

                LINK(d, first_descriptor, last_descriptor, next, prev);
                d->connected = ConCopyoverRecover;    /* negative so close_socket
                                                         * will cut them off */
                /*
                 * telet negotiation asking what thier Client is 
                 */
                write_to_buffer(d, (const char *) do_termtype_str, 0);
#ifdef MCCP
                if (bCompress)
                        compressStart(d, bCompress);
#endif

                /*
                 * Now, find the pfile 
                 */

                fOld = load_char_obj(d, name, FALSE, FALSE);

                if (!fOld)  /* Player file not found?! */
                {
                        write_to_descriptor(desc,
                                            "\n\rSomehow, your character was lost in the copyover sorry.\n\r",
                                            0);
                        close_socket(d, FALSE);
                }
                else    /* ok! */
                {
                        /*
                         * Just In Case,  Someone said this isn't necassary, but _why_
                         * do we want to dump someone in limbo? 
                         */
                        if (!d->character->in_room)
                                d->character->in_room =
                                        get_room_index(RoomVnumTemple);

                        /*
                         * Insert in the char_list 
                         */
                        LINK(d->character, first_char, last_char, next, prev);

                        char_to_room(d->character, d->character->in_room);
                        load_home(d->character);
                        d->connected = ConPlaying;
#ifdef ACCOUNT
                        d->Account = d->character->pcdata->Account;
#endif

                        if (d->character->pcdata->area)
                                do_loadarea(d->character, "");
                }
        }

        FCLOSE(fp);
        {
                CharData *ch;

                for (ch = first_char; ch; ch = ch->next)
                {
                        if (ch->following && ch->following[0] != '\0')
                        {
                                ch->master =
                                        get_char_world_nocheck(ch->following);
                                STRFREE(ch->following);
                        }

                        if (ch->groupleader && ch->groupleader[0] != '\0')
                        {
                                ch->leader =
                                        get_char_world_nocheck(ch->
                                                               groupleader);
                                STRFREE(ch->groupleader);
                        }
                }
        }

}
