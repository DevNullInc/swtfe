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
 *                               Space Module                                            *
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
 * Space travel and ship management system for exploring the galaxy and handling space combat. *
 ****************************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.hpp"
#ifdef MXP
#  include "mxp.hpp"
#endif
#include "msp.hpp"
#include "olc-shuttle.hpp"
#include "astral.hpp"
#include "space2.hpp"

ShipData *first_ship;
ShipData *last_ship;

MissileData *first_missile;
MissileData *last_missile;

SpaceData *first_starsystem;
SpaceData *last_starsystem;

char     *const ShipFlags[] = {
        "simulator", "cloak", "stealth", "interdictor", "republic", "empire",
        "hunter", "smuggler", "pirate", "police", "clan1",
        "clan2", "clan3", "clan4", "clan5", "troop", "taxi", "r17", "r18",
        "r19", "r20",
        "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
        "r30", "r31"
};

int get_shipflag(char *flag)
{
        int       x;

        for (x = 0; x < 32; x++)
                if (!str_cmp(flag, ShipFlags[x]))
                        return x;
        return -1;
}

/* local routines */
void fread_ship args((ShipData * ship, FILE * fp));
bool load_ship_file args((char *shipfile));
void write_ship_list args((void));
void fread_starsystem args((SpaceData * starsystem, FILE * fp));
bool load_starsystem args((char *starsystemfile));
void write_starsystem_list args((void));
void resetship args((ShipData * ship));
void free_ship args((ShipData * ship));
void landship args((ShipData * ship, char *arg));
void launchship args((ShipData * ship));
void      echo_to_room_dnr
args((int ecolor, RoomIndexData * room, char *argument));
bool      autofly(ShipData * ship);
bool      is_facing(ShipData * ship, ShipData * target);
void      clear_targets(ShipData * ship);

/* from comm.c */
bool write_to_descriptor args((int desc, char *txt, int length));

RoomIndexData *generate_exit(RoomIndexData * in_room, ExitData ** pexit);

void echo_to_room_dnr(int ecolor, RoomIndexData * room, char *argument)
{
        CharData *vic;

        if (room == NULL)
                return;

        for (vic = room->first_person; vic; vic = vic->next_in_room)
        {
                set_char_color(ecolor, vic);
                send_to_char(argument, vic);
        }
}

/* Dam i uglified this code..
 * might want to go back to the old version
 * - Gavin
 */
void show_ships_to_char(ShipData * ship, CharData * ch)
{
        int       count = 0, len = 0, clen = 0;

        while (ship)
        {
                count++;
                clen = strlen_color(ship->name);
                len = strlen(ship->name);
                set_char_color(AtShip, ch);
/*                if (IsMxp(ch))
                        ch_printf(ch, MXPTAG("ship '%s'"), ship->name);*/
                ch_printf(ch, "%-35s%*c", ship->name, (len - clen), ' ');
/*                if (IsMxp(ch))
                        send_to_char(MXPTAG("/ship"), ch);*/
                if ((count % 2) == 0)
                        send_to_char("\n\r", ch);
                ship = ship->next_in_room;
        }
        if (count > 0 && (count % 2) != 0)
                send_to_char("\n\r", ch);
        return;
}



void move_ships()
{
        ShipData *ship;
        MissileData *missile;
        MissileData *m_next;
        ShipData *target;
        float     dx, dy, dz, change;
        char      buf[MaxStringLength];
        CharData *ch;
        bool      ch_found = FALSE;
        BodyData *body = NULL;

        for (missile = first_missile; missile; missile = m_next)
        {
                m_next = missile->next;

                ship = missile->fired_from;
                target = missile->target;

                if (target->starsystem
                    && target->starsystem == missile->starsystem)
                {
                        if (missile->mx < target->vx)
                                missile->mx +=
                                        UMin(missile->Speed / 5,
                                             (int) (target->vx -
                                                    missile->mx));
                        else if (missile->mx > target->vx)
                                missile->mx -=
                                        UMin(missile->Speed / 5,
                                             (int) (missile->mx -
                                                    target->vx));
                        if (missile->my < target->vy)
                                missile->my +=
                                        UMin(missile->Speed / 5,
                                             (int) (target->vy -
                                                    missile->my));
                        else if (missile->my > target->vy)
                                missile->my -=
                                        UMin(missile->Speed / 5,
                                             (int) (missile->my -
                                                    target->vy));
                        if (missile->mz < target->vz)
                                missile->mz +=
                                        UMin(missile->Speed / 5,
                                             (int) (target->vz -
                                                    missile->mz));
                        else if (missile->mz > target->vz)
                                missile->mz -=
                                        UMin(missile->Speed / 5,
                                             (int) (missile->mz -
                                                    target->vz));

                        if (abs((int) missile->mx) - abs((int) target->vx) <=
                            20
                            && abs((int) missile->mx) -
                            abs((int) target->vx) >= -20
                            && abs((int) missile->my) -
                            abs((int) target->vy) <= 20
                            && abs((int) missile->my) -
                            abs((int) target->vy) >= -20
                            && abs((int) missile->mz) -
                            abs((int) target->vz) <= 20
                            && abs((int) missile->mz) -
                            abs((int) target->vz) >= -20)
                        {
                                if (target->chaff_released <= 0)
                                {
                                        echo_to_room(AtYellow,
                                                     get_room_index(ship->
                                                                    gunseat),
                                                     "Your missile hits its target dead on!");
                                        echo_to_cockpit(AtBlood, target,
                                                        "The ship is hit by a missile.");
                                        echo_to_ship(AtRed, target,
                                                     "A loud explosion shakes thee ship violently!");
                                        snprintf(buf, MSL,
                                                 "You see a small explosion as %s is hit by a missile",
                                                 target->name);
                                        echo_to_system(AtOrange, target, buf,
                                                       ship);
                                        for (ch = first_char; ch;
                                             ch = ch->next)
                                                if (!IsNpc(ch)
                                                    && nifty_is_name(missile->
                                                                     fired_by,
                                                                     ch->
                                                                     name))
                                                {
                                                        ch_found = TRUE;
                                                        damage_ship_ch(target,
                                                                       20 +
                                                                       missile->
                                                                       missiletype
                                                                       *
                                                                       missile->
                                                                       missiletype
                                                                       * 20,
                                                                       30 +
                                                                       missile->
                                                                       missiletype
                                                                       *
                                                                       missile->
                                                                       missiletype
                                                                       *
                                                                       missile->
                                                                       missiletype
                                                                       * 30,
                                                                       ch);
                                                }
                                        if (!ch_found)
                                                damage_ship(target,
                                                            20 +
                                                            missile->
                                                            missiletype *
                                                            missile->
                                                            missiletype * 20,
                                                            30 +
                                                            missile->
                                                            missiletype *
                                                            missile->
                                                            missiletype *
                                                            ship->
                                                            missiletype * 30);
                                        extract_missile(missile);
                                }
                                else
                                {
                                        echo_to_room(AtYellow,
                                                     get_room_index(ship->
                                                                    gunseat),
                                                     "Your missile explodes harmlessly in a cloud of chaff!");
                                        echo_to_cockpit(AtYellow, target,
                                                        "A missile explodes in your chaff.");
                                        extract_missile(missile);
                                }
                                continue;
                        }
                        else
                        {
                                missile->age++;
                                if (missile->age >= 50)
                                {
                                        extract_missile(missile);
                                        continue;
                                }
                        }
                }
                else
                {
                        extract_missile(missile);
                        continue;
                }
        }

        for (ship = first_ship; ship; ship = ship->next)
        {
                if (!ship->starsystem)
                        continue;

                if (ship->currspeed > 0)
                {

                        change = sqrt(ship->hx * ship->hx +
                                      ship->hy * ship->hy +
                                      ship->hz * ship->hz);

                        if (change > 0)
                        {
                                dx = ship->hx / change;
                                dy = ship->hy / change;
                                dz = ship->hz / change;
                                ship->vx += (dx * ship->currspeed / 5);
                                ship->vy += (dy * ship->currspeed / 5);
                                ship->vz += (dz * ship->currspeed / 5);
                        }

                }

                if (autofly(ship))
                        continue;

                ForEachList(BodyList, ship->starsystem->bodies, body)
                {
                        int       distance = 0;

                        if (ship->currspeed <= 0)
                                continue;
                        distance = body->distance(ship);

                        if ((distance < body->gravity() / 10)
                            && body->type() == StarBody && distance > 0)
                        {
                                echo_to_cockpit(AtBlood + AtBlink, ship,
                                                "You fly directly into the sun.");
                                snprintf(buf, MSL,
                                         "%s flys directly into %s!",
                                         ship->name, body->name());
                                echo_to_system(AtOrange, ship, buf, NULL);
                                destroy_ship(ship, NULL);
                                /*
                                 * continue; We do not want to continure here, we want to
                                 * break the for loop here, the ships is not longer in a 
                                 * system, so we don't need to do any of these checks, lets
                                 * just go onto the next ship 
                                 */
                                break;
                        }

                        if (distance < body->gravity()
                            && (body->type() == PlanetBody
                                || body->type() == MoonBody))
                        {
                                snprintf(buf, MSL, "You begin orbitting %s.",
                                         body->name());
                                echo_to_cockpit(AtYellow, ship, buf);
                                snprintf(buf, MSL, "%s begins orbiting %s.",
                                         ship->name, body->name());
                                echo_to_system(AtOrange, ship, buf, NULL);
                                ship->currspeed = 0;
                                continue;
                        }
                }
        }
}

void recharge_ships()
{
        ShipData *ship;
        char      buf[MaxStringLength];

        for (ship = first_ship; ship; ship = ship->next)
        {

                if (ship->statet0 > 0)
                {
                        ship->energy -= ship->statet0;
                        ship->statet0 = 0;
                }
                if (ship->statet1 > 0)
                {
                        ship->energy -= ship->statet1;
                        ship->statet1 = 0;
                }
                if (ship->statet2 > 0)
                {
                        ship->energy -= ship->statet2;
                        ship->statet2 = 0;
                }
                if (ship->statet0i > 0)
                {
                        ship->energy -= ship->statet0i;
                        ship->statet0i = 0;
                }

                if (ship->missilestate == MissileReload2)
                {
                        ship->missilestate = MissileReady;
                        if (ship->missiles > 0)
                                echo_to_room(AtYellow,
                                             get_room_index(ship->gunseat),
                                             "Missile launcher reloaded.");
                }

                if (ship->missilestate == MissileReload)
                {
                        ship->missilestate = MissileReload2;
                }

                if (ship->missilestate == MissileFired)
                        ship->missilestate = MissileReload;

                if (autofly(ship))
                {
                        if (ship->starsystem)
                        {
                                if (ship->target0
                                    && ship->statet0 != LaserDamaged)
                                {
                                        int       percent_chance = 50;
                                        ShipData *target = ship->target0;
                                        int       shots;

                                        if (ship->shipstate != ShipHyperspace
                                            && ship->energy > 25
                                            && ship->target0->starsystem ==
                                            ship->starsystem
                                            && abs((int) target->vx -
                                                   (int) ship->vx) <= 1000
                                            && abs((int) target->vy -
                                                   (int) ship->vy) <= 1000
                                            && abs((int) target->vz -
                                                   (int) ship->vz) <= 1000
                                            && ship->statet0 < ship->lasers)
                                        {

                                                if (ship->ship_class > 1
                                                    || is_facing(ship,
                                                                 target))
                                                {
                                                        percent_chance +=
                                                                target->
                                                                ship_class *
                                                                25;
                                                        percent_chance -=
                                                                target->
                                                                manuever / 10;
                                                        percent_chance -=
                                                                target->
                                                                currspeed /
                                                                20;
                                                        percent_chance -=
                                                                (abs
                                                                 ((int)
                                                                  target->vx -
                                                                  (int) ship->
                                                                  vx) / 70);
                                                        percent_chance -=
                                                                (abs
                                                                 ((int)
                                                                  target->vy -
                                                                  (int) ship->
                                                                  vy) / 70);
                                                        percent_chance -=
                                                                (abs
                                                                 ((int)
                                                                  target->vz -
                                                                  (int) ship->
                                                                  vz) / 70);
                                                        percent_chance =
                                                                URange(10,
                                                                       percent_chance,
                                                                       90);
                                                        if (number_percent() >
                                                            percent_chance)
                                                        {
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "%s fires at you but misses.",
                                                                         ship->
                                                                         name);
                                                                echo_to_cockpit
                                                                        (AtOrange,
                                                                         target,
                                                                         buf);
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "Laserfire from %s barely misses %s.",
                                                                         ship->
                                                                         name,
                                                                         target->
                                                                         name);
                                                                echo_to_system
                                                                        (AtOrange,
                                                                         target,
                                                                         buf,
                                                                         NULL);
                                                        }
                                                        else
                                                        {
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "Laserfire from %s hits %s.",
                                                                         ship->
                                                                         name,
                                                                         target->
                                                                         name);
                                                                echo_to_system
                                                                        (AtOrange,
                                                                         target,
                                                                         buf,
                                                                         NULL);
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "You are hit by lasers from %s!",
                                                                         ship->
                                                                         name);
                                                                echo_to_cockpit
                                                                        (AtBlood,
                                                                         target,
                                                                         buf);
                                                                echo_to_ship
                                                                        (AtRed,
                                                                         target,
                                                                         "A small explosion vibrates through the ship.");
                                                                for (shots =
                                                                     0;
                                                                     shots <=
                                                                     ship->
                                                                     lasers;
                                                                     shots++)
                                                                {
                                                                        damage_ship
                                                                                (target,
                                                                                 5,
                                                                                 10);
                                                                }
                                                                ship->statet0++;
                                                        }
                                                }
                                        }
                                }
                        }
                }

        }
}
void target_output(ShipData * ship, ShipData * target)
{
        int       distance = -1;
        char      hdir, ddir, vdir;
        char      buf[MaxStringLength];

        if (!ship || !target)
                return;
        if (!target->name)
                return;

        distance = distance_ship_ship(ship, target);
        if (target->vx > ship->vx)
                hdir = 'E';
        else if (target->vx < ship->vx)
                hdir = 'W';
        else
                hdir = ' ';

        if (target->vy > ship->vy)
                ddir = 'N';
        else if (target->vy < ship->vy)
                ddir = 'S';
        else
                ddir = ' ';
        if (target->vz > ship->vz)
                vdir = 'U';
        else if (target->vz < ship->vz)
                vdir = 'D';
        else
                vdir = ' ';
        snprintf(buf, MSL,
                 "&BTarget:&C %s    %.0f %.0f %.0f    &BDist:&C %d  &BDir:&C %c %c %c",
                 target->name, target->vx, target->vy, target->vz, distance,
                 hdir, vdir, ddir);
        echo_to_room(AtLblue, get_room_index(ship->gunseat), buf);
}

void update_space()
{
        ShipData *ship, *sh_next;
        ShipData *target;
        char      buf[MaxStringLength];
        int       too_close, target_too_close;
        int       recharge;
        BodyData *body = NULL;

        for (ship = first_ship; ship; ship = sh_next)
        {
                sh_next = ship->next;
                if (ship->starsystem)
                {
                        if (ship->energy > 0
                            && ship->shipstate == ShipDisabled
                            && ship->ship_class != ShipPlatform)
                                ship->energy -= 100;
                        else if (ship->energy > 0)
                                ship->energy += (5 + ship->ship_class * 5);
                        else
                        {
                                bug("%s has %d/%d energy", ship->name,
                                    ship->energy, ship->maxenergy);
                                destroy_ship(ship, NULL);
                                continue;
                        }
                }

                if (ship->chaff_released > 0)
                        ship->chaff_released = FALSE;

                if (ship->lastdoc == ship->location
                    || ship->shipstate == ShipDocked)
                        ship->autopilot = FALSE;

                if (ship->shipstate == ShipHyperspace)
                {
                        ship->hyperdistance -= ship->hyperspeed * 2;
                        if (ship->hyperdistance <= 0)
                        {
                                ship_to_starsystem(ship, ship->currjump);

                                if (ship->starsystem == NULL)
                                {
                                        echo_to_cockpit(AtRed, ship,
                                                        "Ship lost in Hyperspace. Make new calculations.");
                                }
                                else
                                {
                                        echo_to_room(AtYellow,
                                                     get_room_index(ship->
                                                                    pilotseat),
                                                     "Hyperjump complete.");
                                        echo_to_ship(AtYellow, ship,
                                                     "The ship lurches slightly as it comes out of hyperspace.");
                                        if (!IsSet(ship->flags, ShipCloak)
                                            && !IsSet(ship->flags,
                                                       ShipStealth))
                                        {
                                                snprintf(buf, MSL,
                                                         "%s enters the starsystem at %.0f %.0f %.0f",
                                                         ship->name, ship->vx,
                                                         ship->vy, ship->vz);
                                                echo_to_system(AtYellow,
                                                               ship, buf,
                                                               NULL);
                                        }
                                        else if (IsSet
                                                 (ship->flags, ShipStealth)
                                                 && !IsSet(ship->flags,
                                                            ShipCloak))
                                        {
                                                snprintf(buf, MSL,
                                                         "You notice a flash as a ship enters the starsystem,");
                                                echo_to_system(AtYellow,
                                                               ship, buf,
                                                               NULL);
                                        }
                                        ship->shipstate = ShipReady;
                                        STRFREE(ship->home);
                                        ship->home =
                                                STRALLOC(ship->starsystem->
                                                         name);
                                        if (str_cmp("Public", ship->owner))
                                                save_ship(ship);

                                }
                        }
                        else
                        {
                                snprintf(buf, MSL, "%d", ship->hyperdistance);
                                echo_to_room_dnr(AtYellow,
                                                 get_room_index(ship->
                                                                pilotseat),
                                                 "Remaining jump distance: ");
                                echo_to_room(AtWhite,
                                             get_room_index(ship->pilotseat),
                                             buf);

                        }
                }

                /*
                 * following was originaly to fix ships that lost their pilot 
                 * * in the middle of a manuever and are stuck in a busy state 
                 * * but now used for timed manouevers such as turning 
                 */

                if (ship->shipstate == ShipBusy3)
                {
                        echo_to_room(AtYellow,
                                     get_room_index(ship->pilotseat),
                                     "Manuever complete.");
                        ship->shipstate = ShipReady;
                }
                if (ship->shipstate == ShipBusy2)
                        ship->shipstate = ShipBusy3;
                if (ship->shipstate == ShipBusy)
                        ship->shipstate = ShipBusy2;

                if (ship->shipstate == ShipLand2)
                        landship(ship, ship->dest);
                if (ship->shipstate == ShipLand)
                        ship->shipstate = ShipLand2;

                if (ship->shipstate == ShipLaunch2)
                        launchship(ship);
                if (ship->shipstate == ShipLaunch)
                        ship->shipstate = ShipLaunch2;


                ship->shield = UMax(0, ship->shield - 1 - ship->ship_class);

                if (ship->autorecharge && ship->maxshield > ship->shield
                    && ship->energy > 100)
                {
                        recharge =
                                UMin(ship->maxshield - ship->shield,
                                     10 + ship->ship_class * 10);
                        recharge = UMin(recharge, ship->energy / 2 - 100);
                        recharge = UMax(1, recharge);
                        ship->shield += recharge;
                        ship->energy -= recharge;
                }

                if (ship->shield > 0 && ship->energy < 200)
                {
                        ship->shield = 0;
                        echo_to_cockpit(AtRed, ship,
                                        "The ships shields fizzle and die.");
                        ship->autorecharge = FALSE;
                }

                if (ship->starsystem && ship->currspeed > 0)
                {
                        CharData *vic;
                        RoomIndexData *room =
                                get_room_index(ship->pilotseat);
                        if (room)
                        {
                                snprintf(buf, MSL,
                                         "&BSpeed:&C %d  &BCoords: &C%.0f %.0f %.0f ",
                                         ship->currspeed, ship->vx, ship->vy,
                                         ship->vz);
                                for (vic = room->first_person; vic;
                                     vic = vic->next_in_room)
                                {
                                        set_char_color(AtLblue, vic);
                                        if (!IsNpc(vic)
                                            && !IsSet(vic->act, PlrBrief))
                                        {
                                                send_to_char(buf, vic);
                                                send_to_char("\n\r", vic);
                                        }
                                }
                                if (ship->pilotseat != ship->coseat
                                    && (room = get_room_index(ship->coseat)))
                                {
                                        for (vic = room->first_person; vic;
                                             vic = vic->next_in_room)
                                        {
                                                set_char_color(AtLblue, vic);
                                                if (!IsNpc(vic)
                                                    && !IsSet(vic->act,
                                                               PlrBrief))
                                                {
                                                        send_to_char(buf,
                                                                     vic);
                                                        send_to_char("\n\r",
                                                                     vic);
                                                }
                                        }
                                }
                        }
                }

                if (ship->starsystem)
                {
                        too_close = ship->currspeed + 50;
                        for (target = ship->starsystem->first_ship; target;
                             target = target->next_in_starsystem)
                        {
                                if (target == ship->dockedto)
                                        continue;
                                target_too_close =
                                        too_close + target->currspeed;
                                if (target != ship
                                    && abs((int) ship->vx -
                                           (int) target->vx) <
                                    target_too_close
                                    && abs((int) ship->vy -
                                           (int) target->vy) <
                                    target_too_close
                                    && abs((int) ship->vz -
                                           (int) target->vz) <
                                    target_too_close)
                                {
                                        snprintf(buf, MSL,
                                                 "Proximity alert: %s  %.0f %.0f %.0f",
                                                 target->name, target->vx,
                                                 target->vy, target->vz);
                                        echo_to_room(AtRed,
                                                     get_room_index(ship->
                                                                    pilotseat),
                                                     buf);
                                }
                        }

                        ForEachList(BodyList, ship->starsystem->bodies,
                                      body)
                        {
                                too_close = ship->currspeed + body->gravity();
                                if ((body->distance(ship) < too_close) &&
                                    (body->type() == StarBody
                                     || body->type() == PlanetBody))
                                {
                                        snprintf(buf, MSL,
                                                 "Proximity alert: %s  %d %d %d",
                                                 body->name(), body->xpos(),
                                                 body->ypos(), body->zpos());
                                        echo_to_room(AtRed,
                                                     get_room_index(ship->
                                                                    pilotseat),
                                                     buf);
                                }
                        }
                }
                target_output(ship, ship->target0);
                if (ship->target0
                    && (!ship->target0->starsystem
                        || ship->starsystem != ship->target0->starsystem))
                        ship->target0 = NULL;
                target_output(ship, ship->target1);
                if (ship->target1
                    && (!ship->target1->starsystem
                        || ship->starsystem != ship->target1->starsystem))
                        ship->target1 = NULL;
                target_output(ship, ship->target2);
                if (ship->target2
                    && (!ship->target2->starsystem
                        || ship->starsystem != ship->target2->starsystem))
                        ship->target2 = NULL;

                if (ship->energy < 100 && ship->starsystem)
                {
                        echo_to_cockpit(AtRed, ship,
                                        "Warning: Ship fuel low.");
                }

                ship->energy = URange(0, ship->energy, ship->maxenergy);
        }

        /*
         * Make more efficiant? 
         */
        for (ship = first_ship; ship; ship = ship->next)
        {
                if (ship->autotrack && ship->target0 && ship->ship_class < 3)
                {
                        target = ship->target0;
                        too_close = ship->currspeed + 10;
                        target_too_close = too_close + target->currspeed;
                        if (target != ship && ship->shipstate == ShipReady &&
                            abs((int) ship->vx - (int) target->vx) <
                            target_too_close
                            && abs((int) ship->vy - (int) target->vy) <
                            target_too_close
                            && abs((int) ship->vz - (int) target->vz) <
                            target_too_close)
                        {
                                ship->hx = 0 - (ship->target0->vx - ship->vx);
                                ship->hy = 0 - (ship->target0->vy - ship->vy);
                                ship->hz = 0 - (ship->target0->vz - ship->vz);
                                ship->energy -= ship->currspeed / 10;
                                echo_to_room(AtRed,
                                             get_room_index(ship->pilotseat),
                                             "Autotrack: Evading to avoid collision!\n\r");
                                if (ship->ship_class == FighterShip
                                    || (ship->ship_class == MidsizeShip
                                        && ship->manuever > 50))
                                        ship->shipstate = ShipBusy3;
                                else if (ship->ship_class == MidsizeShip
                                         || (ship->ship_class == CapitalShip
                                             && ship->manuever > 50))
                                        ship->shipstate = ShipBusy2;
                                else
                                        ship->shipstate = ShipBusy;
                        }
                        else if (!is_facing(ship, ship->target0))
                        {
                                ship->hx = ship->target0->vx - ship->vx;
                                ship->hy = ship->target0->vy - ship->vy;
                                ship->hz = ship->target0->vz - ship->vz;
                                ship->energy -= ship->currspeed / 10;
                                echo_to_room(AtBlue,
                                             get_room_index(ship->pilotseat),
                                             "Autotracking target ... setting new course.\n\r");
                                if (ship->ship_class == FighterShip
                                    || (ship->ship_class == MidsizeShip
                                        && ship->manuever > 50))
                                        ship->shipstate = ShipBusy3;
                                else if (ship->ship_class == MidsizeShip
                                         || (ship->ship_class == CapitalShip
                                             && ship->manuever > 50))
                                        ship->shipstate = ShipBusy2;
                                else
                                        ship->shipstate = ShipBusy;
                        }
                }

                if (autofly(ship))
                {
                        if (ship->starsystem)
                        {
                                if (ship->target0)
                                {
                                        int       percent_chance = 50;

                                        /*
                                         * auto assist ships 
                                         */

                                        for (target =
                                             ship->starsystem->first_ship;
                                             target;
                                             target =
                                             target->next_in_starsystem)
                                        {
                                                if (autofly(target))
                                                        if (!str_cmp
                                                            (target->owner,
                                                             ship->owner)
                                                            && target != ship)
                                                                if (target->
                                                                    target0 ==
                                                                    NULL
                                                                    && ship->
                                                                    target0 !=
                                                                    target)
                                                                {
                                                                        target->target0 = ship->target0;
                                                                        snprintf(buf, MSL, "You are being targetted by %s.", target->name);
                                                                        echo_to_cockpit
                                                                                (AtBlood,
                                                                                 target->
                                                                                 target0,
                                                                                 buf);
                                                                        break;
                                                                }
                                        }

                                        target = ship->target0;
                                        ship->autotrack = TRUE;
                                        if (ship->ship_class != ShipPlatform)
                                                ship->currspeed =
                                                        ship->realspeed;
                                        if (ship->energy > 200)
                                                ship->autorecharge = TRUE;


                                        if (ship->shipstate != ShipHyperspace
                                            && ship->energy > 25
                                            && ship->missilestate ==
                                            MissileReady
                                            && ship->target0->starsystem ==
                                            ship->starsystem
                                            && abs((int) target->vx -
                                                   (int) ship->vx) <= 1200
                                            && abs((int) target->vy -
                                                   (int) ship->vy) <= 1200
                                            && abs((int) target->vz -
                                                   (int) ship->vz) <= 1200
                                            && ship->missiles > 0)
                                        {
                                                if (ship->ship_class > 1
                                                    || is_facing(ship,
                                                                 target))
                                                {
                                                        percent_chance -=
                                                                target->
                                                                manuever / 5;
                                                        percent_chance -=
                                                                target->
                                                                currspeed /
                                                                20;
                                                        percent_chance +=
                                                                target->
                                                                ship_class *
                                                                target->
                                                                ship_class *
                                                                25;
                                                        percent_chance -=
                                                                (abs
                                                                 ((int)
                                                                  target->vx -
                                                                  (int) ship->
                                                                  vx) / 100);
                                                        percent_chance -=
                                                                (abs
                                                                 ((int)
                                                                  target->vy -
                                                                  (int) ship->
                                                                  vy) / 100);
                                                        percent_chance -=
                                                                (abs
                                                                 ((int)
                                                                  target->vz -
                                                                  (int) ship->
                                                                  vz) / 100);
                                                        percent_chance +=
                                                                (30);
                                                        percent_chance =
                                                                URange(10,
                                                                       percent_chance,
                                                                       90);

                                                        if (number_percent() >
                                                            percent_chance)
                                                        {
                                                        }
                                                        else
                                                        {
                                                                new_missile
                                                                        (ship,
                                                                         target,
                                                                         NULL,
                                                                         ConcussionMissile);
                                                                ship->missiles--;
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "Incoming missile from %s.",
                                                                         ship->
                                                                         name);
                                                                echo_to_cockpit
                                                                        (AtBlood,
                                                                         target,
                                                                         buf);
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "%s fires a missile towards %s.",
                                                                         ship->
                                                                         name,
                                                                         target->
                                                                         name);
                                                                echo_to_system
                                                                        (AtOrange,
                                                                         target,
                                                                         buf,
                                                                         NULL);

                                                                if (ship->
                                                                    ship_class
                                                                    ==
                                                                    CapitalShip
                                                                    || ship->
                                                                    ship_class
                                                                    ==
                                                                    ShipPlatform)
                                                                        ship->missilestate = MissileReload2;
                                                                else
                                                                        ship->missilestate = MissileFired;
                                                        }
                                                }
                                        }

                                        if (ship->missilestate ==
                                            MissileDamaged)
                                                ship->missilestate =
                                                        MissileReady;
                                        if (ship->statet0 == LaserDamaged)
                                                ship->statet0 = LaserReady;
                                        if (ship->shipstate == ShipDisabled)
                                                ship->shipstate = ShipReady;

                                }
                                else
                                {
                                        ship->currspeed = 0;
                                        for (target = first_ship; target;
                                             target = target->next)
                                                if (ship->starsystem ==
                                                    target->starsystem)
                                                        if (ship->clan
                                                            && target->clan
                                                            && ship->clan->
                                                            enemy)
                                                                if (target->
                                                                    clan ==
                                                                    ship->
                                                                    clan->
                                                                    enemy)
                                                                {
                                                                        ship->target0 = target;
                                                                        snprintf(buf, MSL, "You are being targetted by %s.", ship->name);
                                                                        echo_to_cockpit
                                                                                (AtBlood,
                                                                                 target,
                                                                                 buf);
                                                                        break;
                                                                }
                                }
                        }
                        else
                        {
                                if (number_range(1, 25) == 25)
                                {
                                        ship_to_starsystem(ship,
                                                           starsystem_from_name
                                                           (ship->home));
                                        ship->vx = number_range(-5000, 5000);
                                        ship->vy = number_range(-5000, 5000);
                                        ship->vz = number_range(-5000, 5000);
                                        ship->hx = 1;
                                        ship->hy = 1;
                                        ship->hz = 1;
                                }
                        }
                }

                if ((ship->ship_class == CapitalShip
                     || ship->ship_class == ShipPlatform)
                    && ship->target0 == NULL)
                {
                        if (ship->missiles < ship->maxmissiles)
                                ship->missiles++;
                        if (ship->torpedos < ship->maxtorpedos)
                                ship->torpedos++;
                        if (ship->rockets < ship->maxrockets)
                                ship->rockets++;
                }
        }
}

void write_starsystem_list()
{
        SpaceData *tstarsystem;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, 256, "%s%s", SpaceDir, SpaceList);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open starsystem.lst for writing!\n\r", 0);
                return;
        }
        for (tstarsystem = first_starsystem; tstarsystem;
             tstarsystem = tstarsystem->next)
                fprintf(fpout, "%s\n", tstarsystem->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}


/*
 * Get pointer to space structure from starsystem name.
 */
SpaceData *starsystem_from_name(char *name)
{
        SpaceData *starsystem;

        for (starsystem = first_starsystem; starsystem;
             starsystem = starsystem->next)
                if (!str_cmp(name, starsystem->name))
                        return starsystem;

        for (starsystem = first_starsystem; starsystem;
             starsystem = starsystem->next)
                if (!str_prefix(name, starsystem->name))
                        return starsystem;

        return NULL;
}

/*
 * Get pointer to space structure from the dock vnun.
 */
SpaceData *starsystem_from_vnum(int vnum)
{
        ShipData *ship;
        DockData *dock;
        RoomIndexData *pRoomIndex;

        for (dock = first_dock; dock; dock = dock->next)
                if (vnum == dock->vnum)
                        return dock->body->starsystem();

        for (ship = first_ship; ship; ship = ship->next)
        {
                if (vnum == ship->hanger)
                        return ship->starsystem;
                else if (vnum >= ship->firstroom && vnum <= ship->lastroom)
                        return NULL;
        }

        pRoomIndex = get_room_index(vnum);
        if (pRoomIndex != NULL && pRoomIndex->area->body)
                return pRoomIndex->area->body->starsystem();

        return NULL;
}


/*
 * Save a starsystem's data to its data file
 */
void save_starsystem(SpaceData * starsystem)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MaxStringLength];

        if (!starsystem)
        {
                bug("save_starsystem: null starsystem pointer!", 0);
                return;
        }

        if (!starsystem->filename || starsystem->filename[0] == '\0')
        {
                snprintf(buf, MSL, "save_starsystem: %s has no filename",
                         starsystem->name);
                bug(buf, 0);
                return;
        }

        snprintf(filename, MSL, "%s%s", SpaceDir, starsystem->filename);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_starsystem: fopen", 0);
                perror(filename);
        }
        else
        {
                fprintf(fp, "#SPACE\n");
                fprintf(fp, "Name         %s~\n", starsystem->name);
                fprintf(fp, "Filename     %s~\n", starsystem->filename);
                fprintf(fp, "Xpos         %d\n", starsystem->xpos);
                fprintf(fp, "Ypos         %d\n", starsystem->ypos);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NullFile, "r");
        return;
}


/*
 * Read in actual starsystem data.
 */

void fread_starsystem(SpaceData * starsystem, FILE * fp)
{
        char      buf[MaxStringLength];
        const char *word;
        bool      fMatch;


        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (Upper(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;


                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!starsystem->name)
                                        starsystem->name = STRALLOC("");
                                return;
                        }
                        break;

                case 'F':
                        KEY("Filename", starsystem->filename,
                            fread_string_nohash(fp));
                        break;

                case 'N':
                        KEY("Name", starsystem->name, fread_string(fp));
                        break;


                case 'X':
                        KEY("Xpos", starsystem->xpos, fread_number(fp));

                case 'Y':
                        KEY("Ypos", starsystem->ypos, fread_number(fp));

                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_starsystem: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }
}

/*
 * Load a starsystem file
 */

bool load_starsystem(char *starsystemfile)
{
        char      filename[256];
        SpaceData *starsystem;
        FILE     *fp;
        bool      found;

        NEW(starsystem, SpaceData);
        starsystem->first_ship = NULL;
        starsystem->last_ship = NULL;
        starsystem->first_planet = NULL;
        starsystem->last_planet = NULL;

        found = FALSE;
        snprintf(filename, 256, "%s%s", SpaceDir, starsystemfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

                found = TRUE;
                LINK(starsystem, first_starsystem, last_starsystem, next,
                     prev);
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
                                bug("Load_starsystem_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "SPACE"))
                                fread_starsystem(starsystem, fp);
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MaxStringLength];

                                snprintf(buf, MSL,
                                         "Load_starsystem_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (!(found))
                DELETE(starsystem);

        return found;
}

/*
 * Load in all the starsystem files.
 */
void load_space()
{
        FILE     *fpList;
        const char *filename;
        char      starsystemlist[256];
        char      buf[MaxStringLength];


        first_starsystem = NULL;
        last_starsystem = NULL;

        snprintf(starsystemlist, 256, "%s%s", SpaceDir, SpaceList);
        FCLOSE(fpReserve);
        if ((fpList = fopen(starsystemlist, "r")) == NULL)
        {
                perror(starsystemlist);
                exit(1);
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;


                if (!load_starsystem((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load starsystem file: %s",
                                 filename);
                        bug(buf, 0);
                }
        }
        FCLOSE(fpList);
        boot_log(" Done starsystems ");
        fpReserve = fopen(NullFile, "r");
        return;
}

CMDF do_setstarsystem(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        SpaceData *starsystem;

        if (IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg2[0] == '\0' || arg1[0] == '\0')
        {
                send_to_char
                        ("Usage: setstarsystem <starsystem> <field> <values>\n\r",
                         ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char("name filename xpos ypos,\n\r", ch);
                send_to_char("", ch);
                return;
        }

        starsystem = starsystem_from_name(arg1);
        if (!starsystem)
        {
                send_to_char("No such starsystem.\n\r", ch);
                return;
        }


        if (!str_cmp(arg2, "xpos"))
        {
                starsystem->xpos = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_starsystem(starsystem);
                return;
        }

        if (!str_cmp(arg2, "ypos"))
        {
                starsystem->ypos = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_starsystem(starsystem);
                return;
        }


        if (!str_cmp(arg2, "name"))
        {
                STRFREE(starsystem->name);
                starsystem->name = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_starsystem(starsystem);
                return;
        }

        do_setstarsystem(ch, "");
        return;
}

CMDF do_showstarsystem(CharData * ch, char *argument)
{
        SpaceData *starsystem;
        BodyData *body = NULL;
        DockData *dock = NULL;

        starsystem = starsystem_from_name(argument);

        if (starsystem == NULL)
        {
                send_to_char("&RNo such starsystem.\n\r", ch);
                return;
        }

        ch_printf(ch,
                  "Starsystem:%s     Filename: %s    Xpos: %d   Ypos: %d\n\r",
                  starsystem->name, starsystem->filename, starsystem->xpos,
                  starsystem->ypos);
        ForEachList(BodyList, starsystem->bodies, body)
        {
                ch_printf(ch,
                          "%s: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
                          body->type_name(), body->name(),
                          body->gravity(), body->xpos(),
                          body->ypos(), body->zpos());
                ForEachList(DockList, body->docks(), dock)
                        ch_printf(ch, "     Doc: %5d (%s)\n\r",
                                  dock->vnum, dock->name);
        }
        return;
}

CMDF do_makestarsystem(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        char      filename[256];
        SpaceData *starsystem;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Usage: makestarsystem <starsystem name>\n\r",
                             ch);
                return;
        }


        NEW(starsystem, SpaceData);
        starsystem->first_ship = NULL;
        starsystem->last_ship = NULL;
        starsystem->first_planet = NULL;
        starsystem->last_planet = NULL;
        LINK(starsystem, first_starsystem, last_starsystem, next, prev);

        starsystem->name = STRALLOC(argument);

        argument = one_argument(argument, arg);
        snprintf(filename, 256, "%s.system", strlower(arg));
        starsystem->filename = str_dup(filename);
        save_starsystem(starsystem);
        write_starsystem_list();
}

CMDF do_starsystems(CharData * ch, char *argument)
{
        SpaceData *starsystem;
        int       count = 0;

        argument = NULL;

        for (starsystem = first_starsystem; starsystem;
             starsystem = starsystem->next)
        {
                ch_printf(ch, "&G%s\n\r", starsystem->name);
                count++;
        }

        if (!count)
        {
                send_to_char("There are no starsystems currently formed.\n\r",
                             ch);
                return;
        }
        else
                ch_printf(ch,
                          "There are no currently %d starsystems formed.\n\r",
                          count);
}

void echo_to_ship(int color, ShipData * ship, char *argument)
{
        int       room;

        for (room = ship->firstroom; room <= ship->lastroom; room++)
        {
                echo_to_room(color, get_room_index(room), argument);
        }

}

ShipData *ship_from_room(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum >= ship->firstroom && vnum <= ship->lastroom)
                        return ship;
        return NULL;
}

void echo_to_cockpit(int color, ShipData * ship, char *argument)
{
        int       room;

        for (room = ship->firstroom; room <= ship->lastroom; room++)
        {
                if (room == ship->cockpit || room == ship->navseat
                    || room == ship->pilotseat || room == ship->coseat
                    || room == ship->gunseat || room == ship->engineroom
                    || room == ship->turret1 || room == ship->turret2)
                        echo_to_room(color, get_room_index(room), argument);
        }

}

void echo_to_system(int color, ShipData * ship, char *argument,
                    ShipData * ignore)
{
        ShipData *target;
        BodyData *body = NULL;
        AreaData *area = NULL;
        RoomIndexData *pRoom;
        int       rnum = 0;

        if (!ship->starsystem)
                return;

        for (target = ship->starsystem->first_ship; target;
             target = target->next_in_starsystem)
        {
                if (target != ship && target != ignore)
                        echo_to_cockpit(color, target, argument);
        }

        ForEachList(BodyList, ship->starsystem->bodies, body)
        {
                ForEachList(AreaList, body->areas(), area)
                {
                        for (rnum = area->low_r_vnum; rnum <= area->hi_r_vnum;
                             rnum++)
                        {
                                if ((pRoom = get_room_index(rnum)) == NULL)
                                        continue;
                                if (IsSet(pRoom->RoomFlags, RoomMonitor))
                                        echo_to_room(color, pRoom, argument);
                        }
                }
        }
}

bool is_facing(ShipData * ship, ShipData * target)
{
        float     dy, dx, dz, hx, hy, hz;
        float     cosofa;

        hx = ship->hx;
        hy = ship->hy;
        hz = ship->hz;

        dx = target->vx - ship->vx;
        dy = target->vy - ship->vy;
        dz = target->vz - ship->vz;

        cosofa = (hx * dx + hy * dy + hz * dz)
                / (sqrt(hx * hx + hy * hy + hz * hz) +
                   sqrt(dx * dx + dy * dy + dz * dz));

        if (cosofa > 0.75)
                return TRUE;

        return FALSE;
}


long int get_ship_value(ShipData * ship)
{
        long int  price;

        if (ship->ship_class == FighterShip)
                price = 5000;
        else if (ship->ship_class == MidsizeShip)
                price = 50000;
        else if (ship->ship_class == CapitalShip)
                price = 500000;
        else
                price = 2000;

        if (ship->ship_class <= CapitalShip)
                price += (ship->manuever * 100 * (1 + ship->ship_class));

        price += (ship->tractorbeam * 100);
        price += (ship->realspeed * 10);
        price += (ship->astro_array * 5);
        price += (5 * ship->maxhull);
        price += (2 * ship->maxenergy);
        price += (100 * ship->maxchaff);


        if (!str_cmp(ship->selfdestruct, "Installed"))
                price += 1000;

        if (ship->maxenergy > 5000)
                price += ((ship->maxenergy - 5000) * 20);

        if (ship->maxenergy > 10000)
                price += ((ship->maxenergy - 10000) * 50);

        if (ship->maxhull > 1000)
                price += ((ship->maxhull - 1000) * 10);

        if (ship->maxhull > 10000)
                price += ((ship->maxhull - 10000) * 20);

        if (ship->maxshield > 200)
                price += ((ship->maxshield - 200) * 50);

        if (ship->maxshield > 1000)
                price += ((ship->maxshield - 1000) * 100);

        if (ship->realspeed > 100)
                price += ((ship->realspeed - 100) * 500);

        if (ship->lasers > 5)
                price += ((ship->lasers - 5) * 500);

        if (ship->ions > 2)
                price += ((ship->ions - 2) * 1500);

        if (ship->ions)
                price += ((ship->ions) * 500);

        if (ship->maxshield)
                price += (1000 + 10 * ship->maxshield);

        if (ship->lasers)
                price += (500 + 500 * ship->lasers);

        if (ship->maxmissiles)
                price += (1000 + 100 * ship->maxmissiles);
        if (ship->maxrockets)
                price += (2000 + 200 * ship->maxmissiles);
        if (ship->maxtorpedos)
                price += (1500 + 150 * ship->maxmissiles);
        if (ship->missiles)
                price += (250 * ship->missiles);
        else if (ship->torpedos)
                price += (500 * ship->torpedos);
        else if (ship->rockets)
                price += (1000 * ship->rockets);

        if (ship->stealth != 0)
                price += 25000;
        if (ship->cloak != 0)
                price += 50000;
        if (ship->interdictor != 0)
                price += 40000;
        if (ship->maxbattalions)
                price += (2000 + 150 * ship->maxbattalions);
        if (ship->maxcargo)
                price += (100 * ship->maxcargo);


        if (ship->turret1)
                price += 5000;

        if (ship->turret2)
                price += 5000;

        if (ship->hyperspeed)
                price += (1000 + ship->hyperspeed * 10);

        if (ship->hanger)
                price += (ship->ship_class == MidsizeShip ? 50000 : 100000);

        price = (price * 3) / 2;

        return price;

}

void write_ship_list()
{
        ShipData *tship;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", ShipDir, ShipList);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open ship.lst for writing!\n\r", 0);
                return;
        }
        for (tship = first_ship; tship; tship = tship->next)
                fprintf(fpout, "%s\n", tship->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

ShipData *ship_in_room(RoomIndexData * room, char *name)
{
        ShipData *ship;

        if (!room)
                return NULL;

        for (ship = room->first_ship; ship; ship = ship->next_in_room)
                if (!str_cmp(name, smash_color(ship->name)))
                        return ship;

        for (ship = room->first_ship; ship; ship = ship->next_in_room)
                if (nifty_is_name_prefix(name, smash_color(ship->name)))
                        return ship;

        return NULL;
}

/*
 * Get pointer to ship structure from ship name.
 */
ShipData *get_ship(char *name)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (!str_cmp(name, smash_color(ship->name)))
                        return ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (nifty_is_name_prefix(name, smash_color(ship->name)))
                        return ship;

        return NULL;
}

/*
 * Checks if ships in a starsystem and returns poiner if it is.
 */
ShipData *get_ship_here(char *name, SpaceData * starsystem)
{
        ShipData *ship;

        if (starsystem == NULL)
                return NULL;

        for (ship = starsystem->first_ship; ship;
             ship = ship->next_in_starsystem)
                if (!str_cmp(name, smash_color(ship->name)))
                        return ship;

        for (ship = starsystem->first_ship; ship;
             ship = ship->next_in_starsystem)
                if (nifty_is_name_prefix(name, smash_color(ship->name)))
                        return ship;

        return NULL;
}


/*
 * Get pointer to ship structure from cockpit, turret, or entrance ramp vnum.
 */

ShipData *ship_from_cockpit(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->cockpit || vnum == ship->turret1
                    || vnum == ship->turret2 || vnum == ship->pilotseat
                    || vnum == ship->coseat || vnum == ship->navseat
                    || vnum == ship->gunseat || vnum == ship->engineroom)
                        return ship;
        return NULL;
}

ShipData *ship_from_pilotseat(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->pilotseat)
                        return ship;
        return NULL;
}

ShipData *ship_from_coseat(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->coseat)
                        return ship;
        return NULL;
}

ShipData *ship_from_navseat(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->navseat)
                        return ship;
        return NULL;
}

ShipData *ship_from_gunseat(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->gunseat)
                        return ship;
        return NULL;
}

ShipData *ship_from_engine(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
        {
                if (ship->engineroom)
                {
                        if (vnum == ship->engineroom)
                                return ship;
                }
                else
                {
                        if (vnum == ship->cockpit)
                                return ship;
                }
        }

        return NULL;
}



ShipData *ship_from_turret(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->gunseat || vnum == ship->turret1
                    || vnum == ship->turret2)
                        return ship;
        return NULL;
}

ShipData *ship_from_entrance(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->entrance)
                        return ship;
        return NULL;
}

ShipData *ship_from_hanger(int vnum)
{
        ShipData *ship;

        for (ship = first_ship; ship; ship = ship->next)
                if (vnum == ship->hanger)
                        return ship;
        return NULL;
}


void save_ship(ShipData * ship)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MaxStringLength];

        if (!ship)
        {
                bug("save_ship: null ship pointer!", 0);
                return;
        }

        if (!ship->filename || ship->filename[0] == '\0')
        {
                snprintf(buf, MSL, "save_ship: %s has no filename",
                         ship->name);
                bug(buf, 0);
                return;
        }

        snprintf(filename, 256, "%s%s", ShipDir, ship->filename);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_ship: fopen", 0);
                perror(filename);
        }
        else
        {
                fprintf(fp, "#SHIP\n");
                fprintf(fp, "Name         %s~\n", ship->name);
                fprintf(fp, "Filename     %s~\n", ship->filename);
                fprintf(fp, "Description  %s~\n", ship->description);
                fprintf(fp, "Owner        %s~\n", ship->owner);
                fprintf(fp, "Pilot        %s~\n", ship->pilot);
                fprintf(fp, "Copilot      %s~\n", ship->copilot);
                fprintf(fp, "Class        %d\n", ship->ship_class);
                fprintf(fp, "Tractorbeam  %d\n", ship->tractorbeam);
                fprintf(fp, "Shipyard     %d\n", ship->shipyard);
                fprintf(fp, "Hanger       %d\n", ship->hanger);
                fprintf(fp, "Turret1      %d\n", ship->turret1);
                fprintf(fp, "Turret2      %d\n", ship->turret2);
                fprintf(fp, "Statet0      %d\n", ship->statet0);
                fprintf(fp, "Statet1      %d\n", ship->statet1);
                fprintf(fp, "Statet2      %d\n", ship->statet2);
                fprintf(fp, "Lasers       %d\n", ship->lasers);
                fprintf(fp, "Missiles     %d\n", ship->missiles);
                fprintf(fp, "Maxmissiles  %d\n", ship->maxmissiles);
                fprintf(fp, "Rockets     %d\n", ship->rockets);
                fprintf(fp, "Maxrockets  %d\n", ship->maxrockets);
                fprintf(fp, "Torpedos     %d\n", ship->torpedos);
                fprintf(fp, "Maxtorpedos  %d\n", ship->maxtorpedos);
                fprintf(fp, "Lastdoc      %d\n", ship->lastdoc);
                fprintf(fp, "Firstroom    %d\n", ship->firstroom);
                fprintf(fp, "Lastroom     %d\n", ship->lastroom);
                fprintf(fp, "Shield       %d\n", ship->shield);
                fprintf(fp, "Maxshield    %d\n", ship->maxshield);
                fprintf(fp, "Hull         %d\n", ship->hull);
                fprintf(fp, "Maxhull      %d\n", ship->maxhull);
                fprintf(fp, "Maxenergy    %d\n", ship->maxenergy);
                fprintf(fp, "Hyperspeed   %d\n", ship->hyperspeed);
                fprintf(fp, "Comm         %d\n", ship->comm);
                fprintf(fp, "Chaff        %d\n", ship->chaff);
                fprintf(fp, "Maxchaff     %d\n", ship->maxchaff);
                fprintf(fp, "Sensor       %d\n", ship->sensor);
                fprintf(fp, "Astro_array  %d\n", ship->astro_array);
                fprintf(fp, "Realspeed    %d\n", ship->realspeed);
                fprintf(fp, "Type         %d\n", ship->type);
                fprintf(fp, "Cockpit      %d\n", ship->cockpit);
                fprintf(fp, "Coseat       %d\n", ship->coseat);
                fprintf(fp, "Pilotseat    %d\n", ship->pilotseat);
                fprintf(fp, "Gunseat      %d\n", ship->gunseat);
                fprintf(fp, "Navseat      %d\n", ship->navseat);
                fprintf(fp, "Engineroom   %d\n", ship->engineroom);
                fprintf(fp, "Entrance     %d\n", ship->entrance);
                fprintf(fp, "Shipstate    %d\n", ship->shipstate);
                fprintf(fp, "Missilestate %d\n", ship->missilestate);
                fprintf(fp, "Energy       %d\n", ship->energy);
                fprintf(fp, "Manuever     %d\n", ship->manuever);
                fprintf(fp, "SelfDpass    %d\n", ship->selfdpass);
                fprintf(fp, "Selfdestruct %s~\n", ship->selfdestruct);
                fprintf(fp, "Home         %s~\n", ship->home);
                fprintf(fp, "MaxCargo     %d\n", ship->maxcargo);
                fprintf(fp, "Bombs        %d\n", ship->bombs);
                fprintf(fp, "MaxBombs        %d\n", ship->maxbombs);

                if (ship->cargo > 0)
                {
                        fprintf(fp, "Cargo     %d\n", ship->cargo);
                        fprintf(fp, "CargoType %d\n", ship->cargotype);
                }
                fprintf(fp, "Flags        %d\n", ship->flags);
                fprintf(fp, "Sim_Vnum        %d\n", ship->sim_vnum);
                fprintf(fp, "Stealth        %d\n", ship->stealth);
                fprintf(fp, "Cloak        %d\n", ship->cloak);
                fprintf(fp, "Interdictor       %d\n", ship->interdictor);
                fprintf(fp, "battalions       %d\n", ship->battalions);
                fprintf(fp, "Maxbattalions       %d\n", ship->maxbattalions);
                fprintf(fp, "Ions      %d\n", ship->ions);
                fprintf(fp, "Lastbuilt   %d\n", ship->lastbuilt);
                if (ship->prototype)
                        fprintf(fp, "Prototype   %s~\n",
                                ship->prototype->name);

                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NullFile, "r");
        return;
}


/*
 * Read in actual ship data.
 */

void fread_ship(ShipData * ship, FILE * fp)
{
        char      buf[MaxStringLength];
        const char *word;
        bool      fMatch;
        int       dummy_number;


        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (Upper(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Astro_array", ship->astro_array,
                            fread_number(fp));
                        break;

                case 'B':
                        KEY("Battalions", ship->battalions, fread_number(fp));
                        KEY("Bombs", ship->bombs, fread_number(fp));
                        break;

                case 'C':

                        KEY("Cargo", ship->cargo, fread_number(fp));
                        KEY("CargoType", ship->cargotype, fread_number(fp));
                        KEY("Cockpit", ship->cockpit, fread_number(fp));
                        KEY("Coseat", ship->coseat, fread_number(fp));
                        KEY("Class", ship->ship_class, fread_number(fp));
                        KEY("Copilot", ship->copilot, fread_string(fp));
                        KEY("Comm", ship->comm, fread_number(fp));
                        KEY("Chaff", ship->chaff, fread_number(fp));
                        KEY("Cloak", ship->cloak, fread_number(fp));
                        break;


                case 'D':
                        KEY("Description", ship->description,
                            fread_string(fp));
                        break;

                case 'E':
                        KEY("Engineroom", ship->engineroom, fread_number(fp));
                        KEY("Entrance", ship->entrance, fread_number(fp));
                        KEY("Energy", ship->energy, fread_number(fp));
                        if (!str_cmp(word, "End"))
                        {
                                if (!ship->prototype)
                                        ship->prototype =
                                                get_protoship("generic");
                                if (!ship->home)
                                        ship->home = STRALLOC("");
                                if (!ship->name)
                                        ship->name = STRALLOC("");
                                if (!ship->owner)
                                        ship->owner = STRALLOC("");
                                if (!ship->description)
                                        ship->description = STRALLOC("");
                                if (!ship->copilot)
                                        ship->copilot = STRALLOC("");
                                if (!ship->pilot)
                                        ship->pilot = STRALLOC("");
                                ship->sensor = UMax(50, ship->sensor);
                                if (ship->shipstate != ShipDisabled)
                                        ship->shipstate = ShipDocked;
                                if (ship->statet0 != LaserDamaged)
                                        ship->statet0 = LaserReady;
                                if (ship->statet1 != LaserDamaged)
                                        ship->statet1 = LaserReady;
                                if (ship->statet2 != LaserDamaged)
                                        ship->statet2 = LaserReady;
                                if (ship->missilestate != MissileDamaged)
                                        ship->missilestate = MissileReady;
                                if (ship->shipyard <= 0)
                                        ship->shipyard = RoomLimboShipyard;
                                if (ship->lastdoc <= 0)
                                        ship->lastdoc = ship->shipyard;
                                ship->bayopen = TRUE;
                                ship->autopilot = FALSE;
                                ship->hatchopen = FALSE;
                                if (ship->navseat <= 0)
                                        ship->navseat = ship->cockpit;
                                if (ship->gunseat <= 0)
                                        ship->gunseat = ship->cockpit;
                                if (ship->coseat <= 0)
                                        ship->coseat = ship->cockpit;
                                if (ship->pilotseat <= 0)
                                        ship->pilotseat = ship->cockpit;
                                if (ship->missiletype == 1)
                                {
                                        ship->torpedos = ship->missiles;    /* for back compatability */
                                        ship->missiles = 0;
                                }
                                ship->starsystem = NULL;
                                ship->energy = ship->maxenergy;
                                ship->hull = ship->maxhull;
                                ship->in_room = NULL;
                                ship->next_in_room = NULL;
                                ship->prev_in_room = NULL;
                                ship->dockedto = NULL;
                                return;
                        }
                        break;

                case 'F':
                        KEY("Filename", ship->filename, fread_string(fp));
                        KEY("Firstroom", ship->firstroom, fread_number(fp));
                        KEY("Flags", ship->flags, fread_number(fp));
                        break;

                case 'G':
                        KEY("Gunseat", ship->gunseat, fread_number(fp));
                        break;

                case 'H':
                        KEY("Home", ship->home, fread_string(fp));
                        KEY("Hyperspeed", ship->hyperspeed, fread_number(fp));
                        KEY("Hull", ship->hull, fread_number(fp));
                        KEY("Hanger", ship->hanger, fread_number(fp));
                        break;

                case 'I':
                        KEY("Interdictor", ship->interdictor,
                            fread_number(fp));
                        KEY("Ions", ship->ions, fread_number(fp));
                        break;

                case 'L':
                        KEY("Laserstr", ship->lasers,
                            (sh_int) (fread_number(fp) / 10));
                        KEY("Lasers", ship->lasers, fread_number(fp));
                        KEY("Lastdoc", ship->lastdoc, fread_number(fp));
                        KEY("Lastroom", ship->lastroom, fread_number(fp));
                        KEY("Lastbuilt", ship->lastbuilt, fread_number(fp));
                        break;

                case 'M':
                        KEY("Manuever", ship->manuever, fread_number(fp));
                        KEY("Maxbattalions", ship->maxbattalions,
                            fread_number(fp));
                        KEY("MaxBombs", ship->maxbombs, fread_number(fp));
                        KEY("MaxCargo", ship->maxcargo, fread_number(fp));
                        KEY("Maxmissiles", ship->maxmissiles,
                            fread_number(fp));
                        KEY("Maxtorpedos", ship->maxtorpedos,
                            fread_number(fp));
                        KEY("Maxrockets", ship->maxrockets, fread_number(fp));
                        KEY("Missiles", ship->missiles, fread_number(fp));
                        KEY("Missiletype", ship->missiletype,
                            fread_number(fp));
                        KEY("Maxshield", ship->maxshield, fread_number(fp));
                        KEY("Maxenergy", ship->maxenergy, fread_number(fp));
                        KEY("Missilestate", ship->missilestate,
                            fread_number(fp));
                        KEY("Maxhull", ship->maxhull, fread_number(fp));
                        KEY("Maxchaff", ship->maxchaff, fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", ship->name, fread_string(fp));
                        KEY("Navseat", ship->navseat, fread_number(fp));
                        break;

                case 'O':
                        KEY("Owner", ship->owner, fread_string(fp));
                        KEY("Objectnum", dummy_number, fread_number(fp));
                        break;

                case 'P':
                        KEY("Pilot", ship->pilot, fread_string(fp));
                        KEY("Pilotseat", ship->pilotseat, fread_number(fp));
                        if (!str_cmp(word, "Prototype"))
                        {
                                char     *tmp;

                                tmp = fread_string_nohash(fp);
                                ship->prototype = get_protoship(tmp);
                                if (ship->prototype == NULL)
                                        ship->prototype =
                                                get_protoship("generic");
                                DISPOSE(tmp);
                                fMatch = TRUE;
                        }
/*            KEY( "Prototype",     ship->prototype,          get_protoship( fread_string( fp ) ));*/
                        break;

                case 'R':
                        KEY("Realspeed", ship->realspeed, fread_number(fp));
                        KEY("Rockets", ship->rockets, fread_number(fp));
                        break;

                case 'S':

                        KEY("Selfdestruct", ship->selfdestruct,
                            fread_string(fp));
                        KEY("SelfDpass", ship->selfdpass, fread_number(fp));
                        KEY("Shipyard", ship->shipyard, fread_number(fp));
                        KEY("Sim_vnum", ship->sim_vnum, fread_number(fp));
                        KEY("Sensor", ship->sensor, fread_number(fp));
                        KEY("Shield", ship->shield, fread_number(fp));
                        KEY("Shipstate", ship->shipstate, fread_number(fp));
                        KEY("Statet0", ship->statet0, fread_number(fp));
                        KEY("Statet1", ship->statet1, fread_number(fp));
                        KEY("Statet2", ship->statet2, fread_number(fp));
                        KEY("Stealth", ship->stealth, fread_number(fp));
                        break;

                case 'T':
                        KEY("Type", ship->type, fread_number(fp));
                        KEY("Tractorbeam", ship->tractorbeam,
                            fread_number(fp));
                        KEY("Turret1", ship->turret1, fread_number(fp));
                        KEY("Turret2", ship->turret2, fread_number(fp));
                        KEY("Torpedos", ship->torpedos, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_ship: no match: %s", word);
                        bug(buf, 0);
                }
        }
}

/*
 * Load a ship file
 */

bool load_ship_file(char *shipfile)
{
        char      filename[256];
        ShipData *ship;
        FILE     *fp;
        bool      found;
        RoomIndexData *pRoomIndex;
        ClanData *clan;

        CREATE(ship, ShipData, 1);

        found = FALSE;
        snprintf(filename, 256, "%s%s", ShipDir, shipfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

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
                                bug("Load_ship_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "SHIP"))
                        {
                                fread_ship(ship, fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MaxStringLength];

                                snprintf(buf, MSL,
                                         "Load_ship_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        if (!(found))
                DISPOSE(ship);
        else
        {
                LINK(ship, first_ship, last_ship, next, prev);
                if (!str_cmp("Public", ship->owner) || ship->type == MobShip
                    || ship->type == ClanMobShip)
                {

                        if (ship->ship_class != ShipPlatform
                            && ship->type != MobShip
                            && ship->type != ClanMobShip)
                        {
                                extract_ship(ship);
                                ship_to_room(ship, ship->shipyard);

                                ship->location = ship->shipyard;
                                ship->lastdoc = ship->shipyard;
                                ship->shipstate = ShipDocked;
                        }

                        ship->currspeed = 0;
                        ship->energy = ship->maxenergy;
                        ship->chaff = ship->maxchaff;
                        ship->hull = ship->maxhull;
                        ship->shield = 0;

                        ship->statet1 = LaserReady;
                        ship->statet2 = LaserReady;
                        ship->statet0 = LaserReady;
                        ship->missilestate = LaserReady;

                        ship->currjump = NULL;
                        ship->target0 = NULL;
                        ship->target1 = NULL;
                        ship->target2 = NULL;

                        ship->hatchopen = FALSE;
                        ship->bayopen = TRUE;

                        ship->missiles = ship->maxmissiles;
                        ship->torpedos = ship->maxtorpedos;
                        ship->rockets = ship->maxrockets;
                        ship->autorecharge = FALSE;
                        ship->autotrack = FALSE;
                        ship->autospeed = FALSE;
                        ship->clan = get_clan(ship->owner);

                }

                else if ((pRoomIndex = get_room_index(ship->lastdoc)) != NULL
                         && ship->ship_class != ShipPlatform)
                {
                        LINK(ship, pRoomIndex->first_ship,
                             pRoomIndex->last_ship, next_in_room,
                             prev_in_room);
                        ship->in_room = pRoomIndex;
                        ship->location = ship->lastdoc;
                }


                if (ship->ship_class == ShipPlatform
                    || ship->type == MobShip || ship->type == ClanMobShip
                    || (ship->ship_class == CapitalShip && !ship->in_room))
                {
                        ship_to_starsystem(ship,
                                           starsystem_from_name(ship->home));
                        ship->vx = number_range(-5000, 5000);
                        ship->vy = number_range(-5000, 5000);
                        ship->vz = number_range(-5000, 5000);
                        ship->hx = 1;
                        ship->hy = 1;
                        ship->hz = 1;
                        ship->shipstate = ShipReady;
                        ship->autopilot = TRUE;
                        ship->autorecharge = TRUE;
                        ship->shield = ship->maxshield;
                        ship->clan = get_clan(ship->owner);


                }

                if (ship->type != MobShip
                    && (clan = get_clan(ship->owner)) != NULL)
                {
                        clan->spacecraft++;
                        ship->clan = clan;
                }
        }

        return found;
}

/*
 * Load in all the ship files.
 */
void load_ships()
{
        FILE     *fpList;
        const char *filename;
        char      shiplist[256];
        char      buf[MaxStringLength];


        first_ship = NULL;
        last_ship = NULL;
        first_missile = NULL;
        last_missile = NULL;

        snprintf(shiplist, 256, "%s%s", ShipDir, ShipList);
        FCLOSE(fpReserve);
        if ((fpList = fopen(shiplist, "r")) == NULL)
        {
                perror(shiplist);
                exit(1);
        }

        for (;;)
        {

                filename = feof(fpList) ? "$" : fread_word(fpList);

                if (filename[0] == '$')
                        break;

                if (!load_ship_file((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load ship file: %s",
                                 filename);
                        bug(buf, 0);
                }

        }
        FCLOSE(fpList);
        boot_log(" Done ships ");
        fpReserve = fopen(NullFile, "r");
        return;
}

void resetship(ShipData * ship)
{
        ship->shipstate = ShipReady;

        if (ship->ship_class != ShipPlatform && ship->type != MobShip)
        {
                extract_ship(ship);
                ship_to_room(ship, ship->shipyard);

                ship->location = ship->shipyard;
                ship->lastdoc = ship->shipyard;
                ship->shipstate = ShipDocked;
        }

        if (ship->starsystem)
                ship_from_starsystem(ship, ship->starsystem);

        ship->selfdpass = number_range(10001, 99999);

        ship->currspeed = 0;
        ship->energy = ship->maxenergy;
        ship->chaff = ship->maxchaff;
        ship->hull = ship->maxhull;
        ship->shield = 0;

        ship->statet1 = LaserReady;
        ship->statet2 = LaserReady;
        ship->statet0 = LaserReady;
        ship->missilestate = LaserReady;

        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;

        ship->hatchopen = FALSE;
        ship->bayopen = TRUE;

        ship->missiles = ship->maxmissiles;
        ship->torpedos = ship->maxtorpedos;
        ship->rockets = ship->maxrockets;
        ship->autorecharge = FALSE;
        ship->autotrack = FALSE;
        ship->autospeed = FALSE;
        if (ship->clan)
                ship->clan = NULL;

        if (str_cmp("Public", ship->owner) && ship->type != MobShip)
        {
                ClanData *clan;

                if (ship->type != MobShip
                    && (clan = get_clan(ship->owner)) != NULL)
                {
                        clan->spacecraft--;
                }
                if (ship->owner && ship->owner[0] != '\0')
                        STRFREE(ship->owner);
                ship->owner = STRALLOC("");
                if (ship->pilot && ship->pilot[0] != '\0')
                        STRFREE(ship->pilot);
                ship->pilot = STRALLOC("");
                if (ship->copilot && ship->copilot[0] != '\0')
                        STRFREE(ship->copilot);
                ship->copilot = STRALLOC("");
        }

        if (ship->type == Republic
            || (ship->type == MobShip
                && !str_cmp(ship->owner, "The Galactic Republic")))
        {
                STRFREE(ship->home);
                ship->home = STRALLOC("coruscant");
        }
        else if (ship->type == Imperial
                 || (ship->type == MobShip
                     && !str_cmp(ship->owner, "The Crimson Empire")))
        {
                STRFREE(ship->home);
                ship->home = STRALLOC("byss");
        }
        else if (ship->type == Civilian)
        {
                STRFREE(ship->home);
                ship->home = STRALLOC("tatooine");
        }

        save_ship(ship);
}

CMDF do_resetship(CharData * ch, char *argument)
{
        ShipData *ship;

        ship = get_ship(argument);
        if (ship == NULL)
        {
                send_to_char("&RNo such ship!", ch);
                return;
        }

        resetship(ship);

        if ((ship->ship_class == ShipPlatform || ship->type == MobShip
             || ship->ship_class == CapitalShip) && ship->home)
        {
                ship_to_starsystem(ship, starsystem_from_name(ship->home));
                ship->vx = number_range(-5000, 5000);
                ship->vy = number_range(-5000, 5000);
                ship->vz = number_range(-5000, 5000);
                ship->shipstate = ShipReady;
                ship->autopilot = TRUE;
                ship->autorecharge = TRUE;
                ship->shield = ship->maxshield;
        }

}

CMDF do_setship(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        ShipData *ship;
        int       tempnum;
        RoomIndexData *roomindex;

        if (IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0')
        {
                send_to_char("Usage: setship <ship> <field> <values>\n\r",
                             ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char
                        ("filename name owner copilot pilot description home\n\r",
                         ch);
                send_to_char
                        ("cockpit entrance turret1 turret2 hanger selfdpass selfd\n\r",
                         ch);
                send_to_char("engineroom firstroom lastroom shipyard\n\r",
                             ch);
                send_to_char("manuever Speed hyperspeed tractorbeam\n\r", ch);
                send_to_char("lasers missiles shield hull energy chaff\n\r",
                             ch);
                send_to_char("comm sensor astroarray class torpedos\n\r", ch);
                send_to_char
                        ("pilotseat coseat gunseat navseat rockets maxcargo\n\r",
                         ch);
                send_to_char
                        ("interdictor cloak stealth battalions maxbattalions ions\n\r",
                         ch);
                send_to_char("coordx coordy coordz direction\n\r", ch);
                send_to_char("bombs\n\r", ch);
                return;
        }

        ship = get_ship(arg1);
        if (!ship)
        {
                send_to_char("No such ship.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "owner"))
        {
                ClanData *clan;

                if (ship->type != MobShip
                    && (clan = get_clan(ship->owner)) != NULL)
                {
                        clan->spacecraft--;
                }
                STRFREE(ship->owner);
                ship->owner = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                if (ship->type != MobShip
                    && (clan = get_clan(ship->owner)) != NULL)
                {
                        clan->spacecraft++;
                        ship->clan = clan;
                }
                return;
        }

        if (!str_cmp(arg2, "home"))
        {
                STRFREE(ship->home);
                ship->home = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "pilot"))
        {
                STRFREE(ship->pilot);
                ship->pilot = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "bombs"))
        {
                ship->maxbombs = atoi(argument);
                ship->bombs = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }
        if (!str_cmp(arg2, "copilot"))
        {
                STRFREE(ship->copilot);
                ship->copilot = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "firstroom"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room does not exist.\n\r", ch);
                        return;
                }
                ship->firstroom = tempnum;
                ship->lastroom = tempnum;
                ship->cockpit = tempnum;
                ship->coseat = tempnum;
                ship->pilotseat = tempnum;
                ship->gunseat = tempnum;
                ship->navseat = tempnum;
                ship->entrance = tempnum;
                ship->turret1 = 0;
                ship->turret2 = 0;
                ship->hanger = 0;
                send_to_char
                        ("You will now need to set the other rooms in the ship.\n\r",
                         ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "lastroom"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom)
                {
                        send_to_char
                                ("The last room on a ship must be greater than or equal to the first room.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class == FighterShip
                    && (tempnum - ship->firstroom) > 5)
                {
                        send_to_char
                                ("Starfighters may have up to 5 rooms only.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class == MidsizeShip
                    && (tempnum - ship->firstroom) > 25)
                {
                        send_to_char
                                ("Midships may have up to 25 rooms only.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class == CapitalShip
                    && (tempnum - ship->firstroom) > 100)
                {
                        send_to_char
                                ("Capital Ships may have up to 100 rooms only.\n\r",
                                 ch);
                        return;
                }
                ship->lastroom = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "cockpit"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->turret1 || tempnum == ship->turret2
                    || tempnum == ship->hanger)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->cockpit = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "pilotseat"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->turret1 || tempnum == ship->turret2
                    || tempnum == ship->hanger)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->pilotseat = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }
        if (!str_cmp(arg2, "coseat"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->turret1 || tempnum == ship->turret2
                    || tempnum == ship->hanger)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->coseat = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }
        if (!str_cmp(arg2, "navseat"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->turret1 || tempnum == ship->turret2
                    || tempnum == ship->hanger)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->navseat = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }
        if (!str_cmp(arg2, "gunseat"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->turret1 || tempnum == ship->turret2
                    || tempnum == ship->hanger)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->gunseat = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "entrance"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                ship->entrance = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "turret1"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class == FighterShip)
                {
                        send_to_char
                                ("Starfighters can't have extra laser turrets.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->cockpit || tempnum == ship->entrance ||
                    tempnum == ship->turret2 || tempnum == ship->hanger
                    || tempnum == ship->engineroom)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->turret1 = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "turret2"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class == FighterShip)
                {
                        send_to_char
                                ("Starfighters can't have extra laser turrets.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->cockpit || tempnum == ship->entrance ||
                    tempnum == ship->turret1 || tempnum == ship->hanger
                    || tempnum == ship->engineroom)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->turret2 = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "hanger"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->cockpit || tempnum == ship->entrance ||
                    tempnum == ship->turret1 || tempnum == ship->turret2
                    || tempnum == ship->engineroom)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class == FighterShip)
                {
                        send_to_char
                                ("Starfighters are to small to have hangers for other ships!\n\r",
                                 ch);
                        return;
                }
                ship->hanger = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "engineroom"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.\n\r", ch);
                        return;
                }
                if (tempnum < ship->firstroom || tempnum > ship->lastroom)
                {
                        send_to_char
                                ("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",
                                 ch);
                        return;
                }
                if (tempnum == ship->cockpit || tempnum == ship->entrance ||
                    tempnum == ship->turret1 || tempnum == ship->turret2
                    || tempnum == ship->hanger)
                {
                        send_to_char
                                ("That room is already being used by another part of the ship\n\r",
                                 ch);
                        return;
                }
                ship->engineroom = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "shipyard"))
        {
                tempnum = atoi(argument);
                roomindex = get_room_index(tempnum);
                if (roomindex == NULL)
                {
                        send_to_char("That room doesn't exist.", ch);
                        return;
                }
                ship->shipyard = tempnum;
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "type"))
        {
                if (!str_cmp(argument, "republic"))
                        ship->type = Republic;
                else if (!str_cmp(argument, "imperial"))
                        ship->type = Imperial;
                else if (!str_cmp(argument, "civilian"))
                        ship->type = Civilian;
                else if (!str_cmp(argument, "mob"))
                        ship->type = MobShip;
                else if (!str_cmp(argument, "player"))
                        ship->type = PlayerShip;
                else if (!str_cmp(argument, "clan_mob"))
                        ship->type = ClanMobShip;
                else
                {
                        send_to_char
                                ("Ship type must be either: republic, imperial, civilian, player or mob.\n\r",
                                 ch);
                        return;
                }
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "name"))
        {
                STRFREE(ship->name);
                ship->name = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "filename"))
        {
                STRFREE(ship->filename);
                ship->filename = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                write_ship_list();
                return;
        }

        if (!str_cmp(arg2, "desc"))
        {
                STRFREE(ship->description);
                ship->description = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "manuever"))
        {
                ship->manuever = URange(0, atoi(argument), 120);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "lasers"))
        {
                ship->lasers = URange(0, atoi(argument), 10);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }


        if (!str_cmp(arg2, "selfd"))
        {
                if (str_cmp(argument, "Yes") && str_cmp(argument, "No"))
                {
                        send_to_char("&CChoices&R:&W Yes and No.\n\r", ch);
                        return;
                }
                if (!str_cmp(argument, "Yes") || !str_cmp(argument, "yes"))
                {
                        ship->selfdestruct = STRALLOC("Installed");
                }
                if (!str_cmp(argument, "No") || !str_cmp(argument, "no"))
                {
                        ship->selfdestruct = STRALLOC("Not Installed");
                }
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "selfdpass"))
        {
                if ((atoi(argument) < 10001) || (atoi(argument) > 99999))
                {
                        send_to_char
                                ("Selfdestruct Password must be between 10001 and 99999.\n\r",
                                 ch);
                        return;
                }
                ship->selfdpass = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }


        if (!str_cmp(arg2, "class"))
        {
                ship->ship_class = URange(0, atoi(argument), 9);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "missiles"))
        {
                ship->maxmissiles = URange(0, atoi(argument), 255);
                ship->missiles = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "torpedos"))
        {
                ship->maxtorpedos = URange(0, atoi(argument), 255);
                ship->torpedos = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "rockets"))
        {
                ship->maxrockets = URange(0, atoi(argument), 255);
                ship->rockets = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "Speed"))
        {
                ship->realspeed = URange(0, atoi(argument), 150);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "tractorbeam"))
        {
                ship->tractorbeam = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "hyperspeed"))
        {
                ship->hyperspeed = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "shield"))
        {
                ship->maxshield = URange(0, atoi(argument), 1000);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "hull"))
        {
                ship->hull = URange(1, atoi(argument), 20000);
                ship->maxhull = URange(1, atoi(argument), 20000);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "energy"))
        {
                ship->energy = URange(1, atoi(argument), 30000);
                ship->maxenergy = URange(1, atoi(argument), 30000);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "sensor"))
        {
                ship->sensor = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "astroarray"))
        {
                ship->astro_array = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "comm"))
        {
                ship->comm = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "cloak"))
        {
                ship->cloak = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "stealth"))
        {
                ship->stealth = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "interdictor"))
        {
                ship->interdictor = URange(0, atoi(argument), 255);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "chaff"))
        {
                ship->chaff = URange(0, atoi(argument), 25);
                ship->maxchaff = URange(0, atoi(argument), 25);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "ions"))
        {
                ship->ions = URange(0, atoi(argument), 6);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "battalions"))
        {
                ship->battalions = URange(0, atoi(argument), 50);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxbattalions"))
        {
                ship->maxbattalions = URange(0, atoi(argument), 50);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "flags"))
        {
                if (!argument || argument[0] == '\0')
                {
                        send_to_char
                                ("Usage: setship <ship> flags <flag> [flag]...\n\r",
                                 ch);
                        send_to_char
                                ("Valid flags are: simulator, cloak, stealth, interdictor, taxi\n\r",
                                 ch);
                        send_to_char
                                ("Republic, Empire, Hunter, Smuggler, Pirate, Police, Troop\n\r",
                                 ch);
                        return;
                }

                while (argument[0] != '\0')
                {
                        argument = one_argument(argument, arg3);
                        tempnum = get_shipflag(arg3);

                        if (tempnum < 0 || tempnum > 31)
                        {
                                ch_printf(ch, "Unknown flag: %s\n\r", arg3);
                                return;
                        }
                        ToggleBit(ship->flags, 1 << tempnum);
                }
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxcargo"))
        {
                ship->maxcargo = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "coordx"))
        {
                ship->vx = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "coordy"))
        {
                ship->vy = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "coordz"))
        {
                ship->vz = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "coords"))
        {
                sscanf(argument, "%f %f %f", &(ship->vx), &(ship->vy),
                       &(ship->vz));
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(arg2, "dockedto"))
        {
                ship->dockedto = get_ship(argument);
                if (ship->dockedto == NULL)
                        send_to_char("No such target ship.\n\r", ch);
                else
                        send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }


        if (!str_cmp(arg2, "evasive"))
        {
                ship->evasive = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }


        if (!str_cmp(arg2, "prototype"))
        {
                ship->prototype = get_protoship(argument);
                if (!ship->prototype)
                {
                        send_to_char("Invalid Ship prototype.\n\r", ch);
                        ship->prototype = get_protoship("generic");
                        return;
                }
                send_to_char("Done.\n\r", ch);
                save_ship(ship);
                return;
        }


        do_setship(ch, "");
        return;
}

CMDF do_showship(CharData * ch, char *argument)
{
        ShipData *ship;

        if (IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Usage: showship <ship>\n\r", ch);
                return;
        }

        ship = get_ship(argument);
        if (!ship)
        {
                send_to_char("No such ship.\n\r", ch);
                return;
        }
        set_char_color(AtYellow, ch);
        ch_printf(ch, "%s %s : %s\n\rFilename: %s\n\r",
                  ship->type == Republic ? "New Republic" :
                  ship->type == Imperial ? "Imperial" :
                  ship->type == PlayerShip ? "Player" :
                  ship->type == ClanMobShip ? "Clan Mob" :
                  ship->type == Civilian ? "Civilian" : "Mob",
                  ship->ship_class == FighterShip ? "Starfighter" :
                  (ship->ship_class == MidsizeShip ? "Midship" :
                   (ship->ship_class == CapitalShip ? "Capital Ship" :
                    (ship->ship_class ==
                     ShipPlatform ? "Platform" : "Unknown"))), ship->name,
                  ship->filename);
        if (ship->clan && ship->clan != NULL && ship->clan->name)
                ch_printf(ch,
                          "Home: %s   Description: %s\n\rOwner: %s   Clan: %s    Pilot: %s   Copilot: %s\n\r",
                          ship->home, ship->description, ship->owner,
                          ship->clan->name, ship->pilot, ship->copilot);

        else
                ch_printf(ch,
                          "Home: %s   Description: %s\n\rOwner: %s   Pilot: %s   Copilot: %s\n\r",
                          ship->home, ship->description, ship->owner,
                          ship->pilot, ship->copilot);
        ch_printf(ch, "Firstroom: %d   Lastroom: %d", ship->firstroom,
                  ship->lastroom);
        ch_printf(ch,
                  "Cockpit: %d   Entrance: %d   Hanger: %d  Engineroom: %d\n\r",
                  ship->cockpit, ship->entrance, ship->hanger,
                  ship->engineroom);
        ch_printf(ch,
                  "Pilotseat: %d   Coseat: %d   Navseat: %d  Gunseat: %d\n\r",
                  ship->pilotseat, ship->coseat, ship->navseat,
                  ship->gunseat);
        ch_printf(ch, "Location: %d   Lastdoc: %d   Shipyard: %d\n\r",
                  ship->location, ship->lastdoc, ship->shipyard);
        ch_printf(ch,
                  "Tractor Beam: %d   Comm: %d   Sensor: %d   Astro Array: %d\n\r",
                  ship->tractorbeam, ship->comm, ship->sensor,
                  ship->astro_array);
        ch_printf(ch, "Hanger Door: %s \n\r",
                  ship->bayopen == TRUE ? "OPEN" : ship->bayopen ==
                  FALSE ? "CLOSED" : "Unknown");
        ch_printf(ch, "Lasers: %d  Laser Condition: %s\n\r", ship->lasers,
                  ship->statet0 == LaserDamaged ? "Damaged" : "Good");
        ch_printf(ch, "Turret One: %d  Condition: %s\n\r", ship->turret1,
                  ship->statet1 == LaserDamaged ? "Damaged" : "Good");
        ch_printf(ch, "Turret Two: %d  Condition: %s\n\r", ship->turret2,
                  ship->statet2 == LaserDamaged ? "Damaged" : "Good");
        ch_printf(ch, "Selfdestruct: %s   Selfdestructpass: %d\n\r",
                  ship->selfdestruct, ship->selfdpass);
        ch_printf(ch,
                  "Missiles: %d/%d  Torpedos: %d/%d  Rockets: %d/%d  Condition: %s\n\r",
                  ship->missiles, ship->maxmissiles, ship->torpedos,
                  ship->maxtorpedos, ship->rockets, ship->maxrockets,
                  ship->missilestate == MissileDamaged ? "Damaged" : "Good");
        ch_printf(ch, "Hull: %d/%d  Ship Condition: %s\n\r", ship->hull,
                  ship->maxhull,
                  ship->shipstate == ShipDisabled ? "Disabled" : "Running");

        ch_printf(ch,
                  "Shields: %d/%d   Energy(fuel): %d/%d   Chaff: %d/%d\n\r",
                  ship->shield, ship->maxshield, ship->energy,
                  ship->maxenergy, ship->chaff, ship->maxchaff);
        ch_printf(ch, "Current Coordinates: %.0f %.0f %.0f\n\r", ship->vx,
                  ship->vy, ship->vz);
        ch_printf(ch, "Current Heading: %.0f %.0f %.0f\n\r", ship->hx,
                  ship->hy, ship->hz);
        ch_printf(ch,
                  "Speed: %d/%d   Hyperspeed: %d\n\r  Manuever: %d  Evasive:%d\n\r",
                  ship->currspeed, ship->realspeed, ship->hyperspeed,
                  ship->manuever, ship->evasive);
        ch_printf(ch, "Cargo: %d/%d, Cargo Type: %s \n\r", ship->cargo,
                  ship->maxcargo, cargo_names[ship->cargotype]);
        ch_printf(ch, "Ship flags: %s\n\r",
                  flag_string(ship->flags, ShipFlags));
        ch_printf(ch, "Cloak: %d   Stealth: %d   Interdictor: %d\n\r",
                  ship->cloak, ship->stealth, ship->interdictor);
        ch_printf(ch, "battalions: %d/%d   Ion cannons: %d\n\r",
                  ship->battalions, ship->maxbattalions, ship->ions);
        if (ship->dockedto)
                ch_printf(ch, "Docked to: %s\n\r", ship->dockedto->name);
        if (ship->prototype && ship->prototype->name)
                ch_printf(ch, "Prototype: %s\n\r", ship->prototype->name);
        if (ship->type == PlayerShip)
                ch_printf(ch, "Last Built: %d\n\r", ship->lastbuilt);
        return;
}

CMDF do_makeship(CharData * ch, char *argument)
{
        ShipData *ship;
        char      arg[MaxInputLength];

        argument = one_argument(argument, arg);

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Usage: makeship <filename> <ship name>\n\r",
                             ch);
                return;
        }

        CREATE(ship, ShipData, 1);
        LINK(ship, first_ship, last_ship, next, prev);

        ship->name = STRALLOC(argument);
        ship->description = STRALLOC("");
        ship->owner = STRALLOC("");
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->selfdestruct = STRALLOC("Not Installed");
        ship->selfdpass = number_range(10001, 99999);
        ship->type = Civilian;
        ship->starsystem = NULL;
        ship->energy = ship->maxenergy;
        ship->hull = ship->maxhull;
        ship->in_room = NULL;
        ship->next_in_room = NULL;
        ship->prev_in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->maxcargo = 0;
        ship->cargo = 0;
        ship->cargotype = 0;


        ship->filename = STRALLOC(arg);
        save_ship(ship);
        write_ship_list();

}

CMDF do_copyship(CharData * ch, char *argument)
{
        ShipData *ship;
        ShipData *old;
        char      arg[MaxInputLength];
        char      arg2[MaxInputLength];

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);

        if (!argument || argument[0] == '\0')
        {
                send_to_char
                        ("Usage: copyship <oldshipname> <filename> <newshipname>\n\r",
                         ch);
                return;
        }

        old = get_ship(arg);

        if (!old)
        {
                send_to_char("Thats not a ship!\n\r", ch);
                return;
        }

        CREATE(ship, ShipData, 1);
        LINK(ship, first_ship, last_ship, next, prev);

        ship->name = STRALLOC(argument);
        ship->description = STRALLOC("");
        ship->owner = STRALLOC("");
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->selfdestruct = old->selfdestruct;
        ship->selfdpass = number_range(10001, 99999);
        ship->type = old->type;
        ship->ship_class = old->ship_class;
        ship->lasers = old->lasers;
        ship->maxmissiles = old->maxmissiles;
        ship->maxrockets = old->maxrockets;
        ship->maxtorpedos = old->maxtorpedos;
        ship->maxshield = old->maxshield;
        ship->maxhull = old->maxhull;
        ship->maxenergy = old->maxenergy;
        ship->hyperspeed = old->hyperspeed;
        ship->maxchaff = old->maxchaff;
        ship->realspeed = old->realspeed;
        ship->manuever = old->manuever;
        ship->in_room = NULL;
        ship->next_in_room = NULL;
        ship->prev_in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;

        ship->filename = STRALLOC(arg2);
        save_ship(ship);
        write_ship_list();
}

CMDF do_ships(CharData * ch, char *argument)
{
        ShipData *ship;
        int       count;

        argument = NULL;

        if (!IsNpc(ch))
        {
                count = 0;
                send_to_char
                        ("&YThe following ships are owned by you or by your organization:\n\r",
                         ch);
                send_to_char
                        ("\n\r&WShip                               Owner\n\r",
                         ch);
                for (ship = first_ship; ship; ship = ship->next)
                {
                        if (str_cmp(ship->owner, ch->name))
                        {
                                if (!ch->pcdata || !ch->pcdata->clan
                                    || str_cmp(ship->owner,
                                               ch->pcdata->clan->name)
                                    || ship->ship_class > ShipPlatform)
                                        continue;
                        }

                        if (ship->type == MobShip)
                                continue;
                        else if (ship->type == Republic)
                                set_char_color(AtBlood, ch);
                        else if (ship->type == Imperial)
                                set_char_color(AtDgreen, ch);
                        else
                                set_char_color(AtBlue, ch);

                        if (ship->in_room)
                        {
                                if (ship->in_room->area->body)
                                        ch_printf(ch, "%s (%s) [%s]\n\r",
                                                  ship->name,
                                                  ship->in_room->name,
                                                  ship->in_room->area->body->
                                                  name());
                                else
                                        ch_printf(ch, "%s (%s)\n\r",
                                                  ship->name,
                                                  ship->in_room->name);
                        }
                        else
                                ch_printf(ch, "%s\n\r", ship->name);

                        count++;
                }

                if (!count)
                {
                        send_to_char("There are no ships owned by you.\n\r",
                                     ch);
                }

        }


        count = 0;
        send_to_char("&Y\n\rThe following ships are docked here:\n\r", ch);

        send_to_char
                ("\n\r&WShip                               Owner          Cost/Rent\n\r",
                 ch);
        for (ship = first_ship; ship; ship = ship->next)
        {
                if (ship->location != ch->in_room->vnum
                    || ship->ship_class > ShipPlatform)
                        continue;

                if (ship->type == MobShip)
                        continue;
                else if (ship->type == Republic)
                        set_char_color(AtBlood, ch);
                else if (ship->type == Imperial)
                        set_char_color(AtDgreen, ch);
                else
                        set_char_color(AtBlue, ch);

                ch_printf(ch, "%-35s %-15s", ship->name,
                          check_pilot(ch, ship) ? ship->owner : "---------");
                if (ship->type == MobShip
                    || ship->ship_class == ShipPlatform)
                {
                        ch_printf(ch, "\n\r");
                        continue;
                }
                if (!str_cmp(ship->owner, "Public"))
                {
                        ch_printf(ch, "%ld to rent.\n\r",
                                  get_ship_value(ship) / 100);
                }
                else if (str_cmp(ship->owner, ""))
                        ch_printf(ch, "%s", "\n\r");
                else
                        ch_printf(ch, "%ld to buy.\n\r",
                                  get_ship_value(ship));

                count++;
        }

        if (!count)
        {
                send_to_char("There are no ships docked here.\n\r", ch);
        }
}

void show_all_ships(CharData * ch, int type)
{
        char      buf[MSL];
        ShipData *ship;
        int       count = 0;

        switch (type)
        {
        default:
                send_to_pager
                        ("&Y\n\rThe following ships are currently formed:\n\r",
                         ch);
        case MidsizeShip:
                send_to_pager
                        ("&Y\n\rThe following midships are currently formed:\n\r",
                         ch);
                break;
        case FighterShip:
                send_to_pager
                        ("&Y\n\rThe following midships are currently formed:\n\r",
                         ch);
                break;
        case CapitalShip:
                send_to_pager
                        ("&Y\n\rThe following capital are currently formed:\n\r",
                         ch);
                break;
        }
        send_to_pager("\n\r&WShip                               Owner\n\r",
                      ch);

        for (ship = first_ship; ship; ship = ship->next)
        {
                /*
                 * Not the Ship type we want. -1 = ALL 
                 */
                if (type > -1 && ship->ship_class == type)
                        continue;

                if (ship->ship_class > ShipPlatform)
                        continue;
                else if (ship->type == MobShip && !IsImmortal(ch))
                        continue;
                else if (ship->type == Republic)
                        set_pager_color(AtBlood, ch);
                else if (ship->type == Imperial)
                        set_pager_color(AtDgreen, ch);
                else
                        set_pager_color(AtBlue, ch);

                snprintf(buf, MSL, "%-35s %-15s", ship->name, ship->owner);
                send_to_pager(buf, ch);

                if (ship->type == MobShip
                    || ship->ship_class == ShipPlatform)
                {
                        send_to_pager("\n\r", ch);
                        continue;
                }

                if (!str_cmp(ship->owner, "Public"))
                {
                        snprintf(buf, MSL, "%ld to rent.\n\r",
                                 get_ship_value(ship) / 100);
                        send_to_pager(buf, ch);
                }
                else if (str_cmp(ship->owner, ""))
                {
                        /*
                         * ch_printf( ch, "%s", "\n\r" ); - Dude, wtf is this? 
                         */
                        send_to_pager("\n\r", ch);
                }
                else
                {
                        snprintf(buf, MSL, "%ld to buy.\n\r",
                                 get_ship_value(ship));
                        send_to_pager(buf, ch);
                }
                count++;
        }

        if (!count)
        {
                send_to_pager("There are no ships currently formed.\n\r", ch);
                return;
        }
}

CMDF do_allships(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0' || str_cmp(arg1, "all") == 0)
        {
                show_all_ships(ch, -1);
        }
        else if (!str_cmp(arg1, "midships"))
        {
                show_all_ships(ch, MidsizeShip);
        }
        else if (!str_cmp(arg1, "fighters"))
        {
                show_all_ships(ch, FighterShip);
        }
        else if (!str_cmp(arg1, "capital"))
        {
                show_all_ships(ch, CapitalShip);
        }
        else
        {
                send_to_char("Options are:", ch);
                send_to_char("\tall\t\t Shows all ships", ch);
                send_to_char("\tfighters\t Shows all fighters", ch);
                send_to_char("\tmidships\t Shows all midships", ch);
                send_to_char("\tcapital\t Shows all capital ships", ch);
        }

        return;
}

CMDF do_freeships(CharData * ch, char *argument)
{
        ShipData *ship;
        int       count = 0;
        long      price;

        argument = NULL;

        count = 0;
        send_to_char
                ("&Y\n\rThe following ships are currently available for purchase:\n\r",
                 ch);
        send_to_char
                ("\n\r&R-------------------------------------------------------------",
                 ch);
        send_to_char
                ("\n\r&R|&WShip                            &R   |  &W      Cost         &R  |\n\r",
                 ch);
        send_to_char
                ("&R-------------------------------------------------------------\n\r",
                 ch);



        for (ship = first_ship; ship; ship = ship->next)
        {
                if (!str_cmp(ship->owner, ""))
                {
                        price = get_ship_value(ship);
                        ch_printf(ch,
                                  "&R|&W%-35s&R|&W        %-7d        &R|\n\r",
                                  ship->name, price);

                }
                count++;
        }
        send_to_char
                ("&R-------------------------------------------------------------",
                 ch);

        if (!count)
        {
                send_to_char("There are no ships currently formed.\n\r", ch);
                return;
        }
}

void ship_to_starsystem(ShipData * ship, SpaceData * starsystem)
{
        if (starsystem == NULL)
                return;

        if (ship == NULL)
                return;

        LINK(ship, starsystem->first_ship, starsystem->last_ship,
             next_in_starsystem, prev_in_starsystem);

        ship->starsystem = starsystem;

}

void new_missile(ShipData * ship, ShipData * target, CharData * ch,
                 int missiletype)
{
        SpaceData *starsystem;
        MissileData *missile;

        if (ship == NULL)
                return;

        if (target == NULL)
                return;

        if ((starsystem = ship->starsystem) == NULL)
                return;

        CREATE(missile, MissileData, 1);
        LINK(missile, first_missile, last_missile, next, prev);

        missile->target = target;
        missile->fired_from = ship;
        if (ch)
                missile->fired_by = STRALLOC(ch->name);
        else
                missile->fired_by = STRALLOC("");
        missile->missiletype = missiletype;
        missile->age = 0;
        if (missile->missiletype == HeavyBomb)
                missile->Speed = 20;
        else if (missile->missiletype == ProtonTorpedo)
                missile->Speed = 200;
        else if (missile->missiletype == ConcussionMissile)
                missile->Speed = 300;
        else
                missile->Speed = 50;

        missile->mx = (int) ship->vx;
        missile->my = (int) ship->vy;
        missile->mz = (int) ship->vz;

        if (starsystem->first_missile == NULL)
                starsystem->first_missile = missile;

        if (starsystem->last_missile)
        {
                starsystem->last_missile->next_in_starsystem = missile;
                missile->prev_in_starsystem = starsystem->last_missile;
        }

        starsystem->last_missile = missile;

        missile->starsystem = starsystem;

}

void ship_from_starsystem(ShipData * ship, SpaceData * starsystem)
{

        if (starsystem == NULL)
                return;

        if (ship == NULL)
                return;


        UNLINK(ship, starsystem->first_ship, starsystem->last_ship,
               next_in_starsystem, prev_in_starsystem);
        ship->starsystem = NULL;
}

void extract_missile(MissileData * missile)
{
        SpaceData *starsystem;

        if (missile == NULL)
                return;

        if ((starsystem = missile->starsystem) != NULL)
        {

                if (starsystem->last_missile == missile)
                        starsystem->last_missile =
                                missile->prev_in_starsystem;

                if (starsystem->first_missile == missile)
                        starsystem->first_missile =
                                missile->next_in_starsystem;

                if (missile->prev_in_starsystem)
                        missile->prev_in_starsystem->next_in_starsystem =
                                missile->next_in_starsystem;

                if (missile->next_in_starsystem)
                        missile->next_in_starsystem->prev_in_starsystem =
                                missile->prev_in_starsystem;

                missile->starsystem = NULL;
                missile->next_in_starsystem = NULL;
                missile->prev_in_starsystem = NULL;

        }

        UNLINK(missile, first_missile, last_missile, next, prev);

        missile->target = NULL;
        missile->fired_from = NULL;
        if (missile->fired_by)
                STRFREE(missile->fired_by);

        DISPOSE(missile);

}

bool is_rental(CharData * ch, ShipData * ship)
{
        ch = NULL;
        if (!str_cmp("Public", ship->owner))
                return TRUE;

        return FALSE;
}

bool check_pilot(CharData * ch, ShipData * ship)
{
        if (!str_cmp(ch->name, ship->owner) || !str_cmp(ch->name, ship->pilot)
            || !str_cmp(ch->name, ship->copilot)
            || !str_cmp("Public", ship->owner))
                return TRUE;

        if (!IsNpc(ch) && ch->pcdata && ch->pcdata->clan)
        {
                if (!str_cmp(ch->pcdata->clan->name, ship->owner))
                {
                        if (HasClanPerm(ch, ch->pcdata->clan, "pilot"))
                        {
                                return TRUE;
                        }
                }
        }

        return FALSE;
}

bool extract_ship(ShipData * ship)
{
        RoomIndexData *room;

        if ((room = ship->in_room) != NULL)
        {
                UNLINK(ship, room->first_ship, room->last_ship, next_in_room,
                       prev_in_room);
                ship->in_room = NULL;
        }
        return TRUE;
}

void damage_ship_ch(ShipData * ship, int min, int max, CharData * ch)
{
        int       damage_amount, shield_dmg;
        long      xp;

        damage_amount = number_range(min, max);

        xp = (exp_level(ch->skill_level[PilotingAbility] + 1) -
              exp_level(ch->skill_level[PilotingAbility])) / 25;
        xp = UMin(get_ship_value(ship) / 100, xp);
        gain_exp(ch, xp, PilotingAbility);

        if (ship->shield > 0)
        {
                shield_dmg = UMin(ship->shield, damage_amount);
                damage_amount -= shield_dmg;
                ship->shield -= shield_dmg;
                if (ship->shield == 0)
                        echo_to_cockpit(AtBlood, ship, "Shields down...");
        }

        if (damage_amount > 0)
        {
                if (number_range(1, 100) <= 5
                    && ship->shipstate != ShipDisabled)
                {
                        echo_to_cockpit(AtBlood + AtBlink, ship,
                                        "Ships Drive DAMAGED!");
                        ship->shipstate = ShipDisabled;
                }

                if (number_range(1, 100) <= 5
                    && ship->missilestate != MissileDamaged
                    && ship->maxmissiles > 0)
                {
                        echo_to_room(AtBlood + AtBlink,
                                     get_room_index(ship->gunseat),
                                     "Ships Missile Launcher DAMAGED!");
                        ship->missilestate = MissileDamaged;
                }

                if (number_range(1, 100) <= 2
                    && ship->statet0 != LaserDamaged)
                {
                        echo_to_room(AtBlood + AtBlink,
                                     get_room_index(ship->gunseat),
                                     "Lasers DAMAGED!");
                        ship->statet1 = LaserDamaged;
                }

                if (number_range(1, 100) <= 5
                    && ship->statet1 != LaserDamaged && ship->turret1)
                {
                        echo_to_room(AtBlood + AtBlink,
                                     get_room_index(ship->turret1),
                                     "Turret DAMAGED!");
                        ship->statet1 = LaserDamaged;
                }

                if (number_range(1, 100) <= 5
                    && ship->statet2 != LaserDamaged && ship->turret2)
                {
                        echo_to_room(AtBlood + AtBlink,
                                     get_room_index(ship->turret2),
                                     "Turret DAMAGED!");
                        ship->statet2 = LaserDamaged;
                }

        }

        ship->hull -= damage_amount * 5;

        if (ship->hull <= 0)
        {
                destroy_ship(ship, ch);

                xp = (exp_level(ch->skill_level[PilotingAbility] + 1) -
                      exp_level(ch->skill_level[PilotingAbility]));
                xp = UMin(get_ship_value(ship), xp);
                gain_exp(ch, xp, PilotingAbility);
                ch_printf(ch, "&WYou gain %ld piloting experience!\n\r", xp);
                return;
        }

        if (ship->hull <= ship->maxhull / 20)
                echo_to_cockpit(AtBlood + AtBlink, ship,
                                "WARNING! Ship hull severely damaged!");

}

void damage_ship(ShipData * ship, int min, int max)
{
        int       damage_amount, shield_dmg;

        damage_amount = number_range(min, max);

        if (ship->shield > 0)
        {
                shield_dmg = UMin(ship->shield, damage_amount);
                damage_amount -= shield_dmg;
                ship->shield -= shield_dmg;
                if (ship->shield == 0)
                        echo_to_cockpit(AtBlood, ship, "Shields down...");
        }

        if (damage_amount > 0)
        {

                if (number_range(1, 100) <= 5
                    && ship->shipstate != ShipDisabled)
                {
                        echo_to_cockpit(AtBlood + AtBlink, ship,
                                        "Ships Drive DAMAGED!");
                        ship->shipstate = ShipDisabled;
                }

                if (number_range(1, 100) <= 5
                    && ship->missilestate != MissileDamaged
                    && ship->maxmissiles > 0)
                {
                        echo_to_room(AtBlood + AtBlink,
                                     get_room_index(ship->gunseat),
                                     "Ships Missile Launcher DAMAGED!");
                        ship->missilestate = MissileDamaged;
                }

                if (number_range(1, 100) <= 2
                    && ship->statet1 != LaserDamaged && ship->turret1)
                {
                        echo_to_room(AtBlood + AtBlink,
                                     get_room_index(ship->turret1),
                                     "Turret DAMAGED!");
                        ship->statet1 = LaserDamaged;
                }

                if (number_range(1, 100) <= 2
                    && ship->statet2 != LaserDamaged && ship->turret2)
                {
                        echo_to_room(AtBlood + AtBlink,
                                     get_room_index(ship->turret2),
                                     "Turret DAMAGED!");
                        ship->statet2 = LaserDamaged;
                }

        }

        ship->hull -= damage_amount * 5;

        if (ship->hull <= 0)
        {
                destroy_ship(ship, NULL);
                return;
        }

        if (ship->hull <= ship->maxhull / 20)
                echo_to_cockpit(AtBlood + AtBlink, ship,
                                "WARNING! Ship hull severely damaged!");

}

bool escape_pod(CharData * ch, ShipData * ship)
{
        BodyData *body = NULL;
        RoomIndexData *pRoom = NULL;
        ObjData *scraps;
        AreaData *area = NULL;
        int       rnum = 0;

        if (IsNpc(ch) || IsImmortal(ch))
                return FALSE;   /* NPCs Die */
        if (!ship->starsystem)
                return FALSE;   /* ... is this possible? */
        /*
         * Should this be um, a um, sysdata thing? 
         */
        /*
         * (Flanx) OOC: escape pod idea:success %=lck*4
         * (Gavin) OOC: if (rand% <= lck*4)?
         */
        if (number_percent() >= (get_curr_lck(ch) * 4))
                return FALSE;

        /*
         * Loop through rooms in bodies instead of starsystems.
         * *
         * * number_percentage should deal with Luck thing too
         * * then to check if we use this room, do another check?
         */
        ForEachList(BodyList, ship->starsystem->bodies, body)
        {
                if (body->type() == StarBody
                    || body->type() == AsteroidBody
                    || body->type() == CometBody
                    || body->type() == BlackholeBody
                    || body->type() == NebulaBody)
                        continue;

                ForEachList(AreaList, body->areas(), area)
                {
                        for (rnum = area->low_r_vnum; rnum <= area->hi_r_vnum;
                             rnum++)
                        {
                                if ((pRoom = get_room_index(rnum)) == NULL)
                                        continue;
                                if (IsOutsideRoom(pRoom))
                                {
                                        char_from_room(ch);
                                        char_to_room(ch, pRoom);
                                        ch->position = PosResting;
                                        if (!IsImmortal(ch))
                                                ch->hit = -1;
                                        update_pos(ch);
                                        echo_to_room(AtWhite, ch->in_room,
                                                     "There is loud explosion as an escape pod hits the earth.");
                                        scraps = create_object(get_obj_index
                                                               (ObjVnumScraps),
                                                               0);
                                        scraps->timer = 15;
                                        STRFREE(scraps->short_descr);
                                        scraps->short_descr =
                                                STRALLOC
                                                ("a battered escape pod");
                                        STRFREE(scraps->description);
                                        scraps->description =
                                                STRALLOC
                                                ("The smoking shell of an escape pod litters the earth.\n\r");
                                        obj_to_room(scraps, pRoom);
                                        return TRUE;
                                }
                        }
                }
        }
        /*
         * we didn't get to the pod 
         */
        return FALSE;
}

void destroy_ship(ShipData * ship, CharData * ch)
{
        char      buf[MaxStringLength];
        int       roomnum;
        RoomIndexData *room;
        ObjData *robj;
        CharData *rch;


        snprintf(buf, MSL, "%s explodes in a blinding flash of light!",
                 ship->name);
        echo_to_system(AtWhite + AtBlink, ship, buf, NULL);

        snprintf(buf, MSL, "%s destroyed by %s", ship->name,
                 ch ? ch->name : "(none)");
        log_string(buf);

        if (ship->ship_class != FighterShip)
        {
                echo_to_ship(AtWhite, ship,
                             "The ship is shaken by a FATAL explosion. You realize its escape or perish.");
                echo_to_ship(AtWhite, ship,
                             "The last thing you remember is reaching for the escape pod release lever.");
        }
        echo_to_ship(AtWhite + AtBlink, ship, "A blinding flash of light.");
        echo_to_ship(AtWhite, ship, "And then darkness....");


        for (roomnum = ship->firstroom; roomnum <= ship->lastroom; roomnum++)
        {
                room = get_room_index(roomnum);

                if (room != NULL)
                {
                        for (rch = room->first_person; rch;
                             rch = room->first_person)
                        {
                                if (IsSet(ship->flags, ShipSimulator))
                                {
                                        resetship(ship);
                                        ship->shipyard = ship->sim_vnum;
                                        ship->shipstate = ShipReady;
                                        extract_ship(ship);
                                        ship_to_room(ship, ship->shipyard);
                                        ship->location = ship->shipyard;
                                        ship->lastdoc = ship->shipyard;
                                        ship->shipstate = ShipDocked;
                                        if (ship->starsystem)
                                                ship_from_starsystem(ship,
                                                                     ship->
                                                                     starsystem);
                                        save_ship(ship);
                                        send_to_char
                                                ("The lights dim and the hatch opens.\n\r",
                                                 rch);

                                        return;
                                }
                                else if (IsImmortal(rch))
                                {
                                        char_from_room(rch);
                                        char_to_room(rch,
                                                     get_room_index(wherehome
                                                                    (rch)));
                                }
                                else if (ship->ship_class != FighterShip
                                         && escape_pod(rch, ship))
                                {
                                        continue;
                                }
                                else
                                {
                                        if (ch)
                                                raw_kill(ch, rch);
                                        else
                                                raw_kill(rch, rch);
                                }
                        }

                        for (robj = room->first_content; robj;
                             robj = robj->next_content)
                        {
                                separate_obj(robj);
                                extract_obj(robj);
                        }
                }
        }

        resetship(ship);

        if (ship->type == PlayerShip)
        {
                transship(ship, 45);
                really_destroy_ship(ship);
        }
        else if (ship->type == ClanMobShip)
        {
                transship(ship, 45);
                really_destroy_mob_ship(ship);
        }
}

CMDF do_really_destroy_ship(CharData * ch, char *argument)
{
        ShipData *ship;

        ship = get_ship(argument);
        if (!ship)
        {
                send_to_char("No such ship.\n\r", ch);
                return;
        }
        if (ship->type == ClanMobShip)
                really_destroy_mob_ship(ship);
        else
                really_destroy_ship(ship);
        send_to_char("Ship Utterly destroyed", ch);
        return;
}

void really_destroy_ship(ShipData * ship)
{

        RoomIndexData *room;
        AreaData *area;
        int       roomnum;
        char      file[MaxStringLength];

        if (!ship)
                return;
		/* Safe to remove this check from here because destory_ship checks for playership first
		 * and we want it to work for dismantle ship
		 * - Gavin 
		if (ship->type != PlayerShip)
				return; */
        if ((room = get_room_index(ship->firstroom)) == NULL)
                return;

        if ((area = room->area) == NULL)
                return;

        /*
         * Ugh i hate these - Gavin 
         */
        for (roomnum = ship->firstroom; roomnum <= ship->lastroom; roomnum++)
        {
                if ((room = get_room_index(roomnum)) == NULL)
                        continue;
                clean_room(room);
                delete_room(room);
                room = NULL;
        }

        fold_area(area, area->filename, FALSE, TRUE);

        extract_ship(ship);
        snprintf(file, MSL, "%s%s", ShipDir, ship->filename);
        remove(file);
        free_ship(ship);
        clear_targets(ship);
        UNLINK(ship, first_ship, last_ship, next, prev);
        DISPOSE(ship);
        ship = NULL;
        write_ship_list();
        return;
}

void really_destroy_mob_ship(ShipData * ship)
{
        char      file[MaxStringLength];

        if (!ship || (ship->type != ClanMobShip))
                return;

        extract_ship(ship);
        snprintf(file, MSL, "%s%s", ShipDir, ship->filename);
        free_ship(ship);
        remove(file);

        clear_targets(ship);
        UNLINK(ship, first_ship, last_ship, next, prev);
        DISPOSE(ship);
        ship = NULL;
        write_ship_list();

        return;
}

void clear_targets(ShipData * ship)
{
        ShipData *target;

        for (target = first_ship; target; target = target->next)
        {
                if (target->target0 == ship)
                        target->target0 = NULL;
                if (target->target1 == ship)
                        target->target1 = NULL;
                if (target->target2 == ship)
                        target->target2 = NULL;
        }
}

void free_ship(ShipData * ship)
{
        if (!ship)
        {
                bug("Free_ship: null ship!");
                return;
        }
        if (ship->filename)
                STRFREE(ship->filename);
        if (ship->name)
                STRFREE(ship->name);
        if (ship->owner)
                STRFREE(ship->owner);
        if (ship->pilot)
                STRFREE(ship->pilot);
        if (ship->copilot)
                STRFREE(ship->copilot);
        if (ship->dest)
                STRFREE(ship->dest);
        if (ship->home)
                STRFREE(ship->home);
        if (ship->description)
                STRFREE(ship->description);
        if (ship->dockedto)
        {
                ship->dockedto->dockedto = NULL;
                ship->dockedto = NULL;
        }
        if (ship->selfdestruct)
                STRFREE(ship->selfdestruct);
        if (ship->tractorby)
        {
                /*
                 * do something to the ship that is is tractoring? like reset status 
                 */
                ship->tractorby = NULL;
        }
        ship->type = 0;
        ship->ship_class = 0;
        ship->comm = 0;
        ship->sensor = 0;
        ship->astro_array = 0;
        ship->hyperspeed = 0;
        ship->hyperdistance = 0;
        ship->realspeed = 0;
        ship->currspeed = 0;
        ship->shipstate = 0;
        ship->statet0 = 0;
        ship->statet1 = 0;
        ship->statet2 = 0;
        ship->statet0i = 0;
        ship->missiletype = 0;
        ship->missilestate = 0;
        ship->missiles = 0;
        ship->maxmissiles = 0;
        ship->torpedos = 0;
        ship->maxtorpedos = 0;
        ship->rockets = 0;
        ship->maxrockets = 0;
        ship->lasers = 0;
        ship->torpedos = 0;
        ship->tractorbeam = 0;
        ship->torpedos = 0;
        ship->ions = 0;
        ship->torpedos = 0;
        ship->manuever = 0;
        ship->torpedos = 0;
        ship->selfdpass = 0;
        ship->torpedos = 0;
        ship->maxcargo = 0;
        ship->cargo = 0;
        ship->cargotype = 0;
        ship->bayopen = 0;
        ship->autorecharge = 0;
        ship->hatchopen = 0;
        ship->autotrack = 0;
        ship->autospeed = 0;
        ship->stealth = 0;
        ship->cloak = 0;
        ship->interdictor = 0;
        ship->vx = 0;
        ship->vy = 0;
        ship->vz = 0;
        ship->hx = 0;
        ship->hy = 0;
        ship->hz = 0;
        ship->jx = 0;
        ship->jy = 0;
        ship->jz = 0;
        ship->maxenergy = 0;
        ship->energy = 0;
        ship->shield = 0;
        ship->maxshield = 0;
        ship->maxhull = 0;
        ship->hull = 0;
        ship->cockpit = 0;
        ship->turret1 = 0;
        ship->turret2 = 0;
        ship->location = 0;
        ship->lastdoc = 0;
        ship->shipyard = 0;
        ship->entrance = 0;
        ship->hanger = 0;
        ship->engineroom = 0;
        ship->firstroom = 0;
        ship->lastroom = 0;
        ship->lastbuilt = 0;
        ship->navseat = 0;
        ship->pilotseat = 0;
        ship->coseat = 0;
        ship->evasive = 0;
        ship->gunseat = 0;
        ship->chaff = 0;
        ship->maxchaff = 0;
        ship->chaff_released = 0;
        ship->autopilot = 0;
        ship->flags = 0;
        ship->sim_vnum = 0;
        ship->battalions = 0;
        ship->maxbattalions = 0;
}

bool ship_to_room(ShipData * ship, int vnum)
{
        RoomIndexData *shipto;

        if ((shipto = get_room_index(vnum)) == NULL)
                return FALSE;
        LINK(ship, shipto->first_ship, shipto->last_ship, next_in_room,
             prev_in_room);
        ship->in_room = shipto;
        return TRUE;
}


CMDF do_board(CharData * ch, char *argument)
{
        RoomIndexData *fromroom;
        RoomIndexData *toroom;
        ShipData *ship;
        char     *name = NULL;
        CharData *rch = NULL;
        CharData *next_in_room = NULL;

#ifdef OlcShuttle
        ShuttleData *shuttle;
#endif

        if (IsSet(ch->affected_by, AffRestrained))
        {
                send_to_char
                        ("How do you expect to do that while restrained?\n\r",
                         ch);
                return;
        }

        if (IsSet(ch->act, ActMounted) && IsNpc(ch))
        {
                act(AtPlain, "You can't go in there riding THAT.", ch, NULL,
                    argument, ToChar);
                return;
        }

        fromroom = ch->in_room;
        if ((ship = ship_from_entrance(ch->in_room->vnum)) != NULL
            && ship->dockedto)
        {
                if (!ship->hatchopen || !ship->dockedto->hatchopen)
                {
                        send_to_char("&RThe hatch is closed!\n\r", ch);
                        return;
                }
                name = ship->dockedto->name;
                toroom = get_room_index(ship->dockedto->entrance);
        }
        else if (argument[0] == '\0')
        {
                send_to_char("Board what?\n\r", ch);
                return;
        }
        else if ((ship = ship_in_room(ch->in_room, argument)) != NULL)
        {

                if ((toroom = get_room_index(ship->entrance)) == NULL)
                {
                        send_to_char("That ship has no entrance!\n\r", ch);
                        return;
                }

                if (!ship->hatchopen)
                {
                        send_to_char("&RThe hatch is closed!\n\r", ch);
                        return;
                }

                if (ship->shipstate == ShipLaunch
                    || ship->shipstate == ShipLaunch2)
                {
                        send_to_char
                                ("&rThat ship has already started launching!\n\r",
                                 ch);
                        return;
                }

                name = ship->name;

        }
#ifdef OlcShuttle
        else if ((shuttle = shuttle_in_room(ch->in_room, argument)) != NULL)
        {
                name = shuttle->name;
                if ((toroom = get_room_index(shuttle->entrance)) == NULL)
                {
                        send_to_char("That ship has no entrance!\n\r", ch);
                        return;
                }
        }
#endif
        else
        {
                act(AtPlain, "I see no $T here.", ch, NULL, argument,
                    ToChar);
                return;
        }
        if (toroom == NULL)
        {
                send_to_char("Error", ch);
                return;
        }

        if (toroom->tunnel > 0)
        {
                CharData *ctmp;
                int       count = 0;

                for (ctmp = toroom->first_person; ctmp;
                     ctmp = ctmp->next_in_room)
                {
                        if (++count >= toroom->tunnel)
                        {
                                send_to_char
                                        ("There is no room for you in there.\n\r",
                                         ch);
                                return;
                        }
                }
        }

        act(AtPlain, "$n enters $T.", ch, NULL, name, ToRoom);
        act(AtPlain, "You enter $T.", ch, NULL, name, ToChar);
        char_from_room(ch);
        char_to_room(ch, toroom);
        act(AtPlain, "$n enters the ship.", ch, NULL, NULL, ToRoom);
        do_look(ch, "auto");
        for (rch = fromroom->first_person; rch; rch = next_in_room)
        {
                next_in_room = rch->next_in_room;
                if (rch != ch   /* loop room bug fix here by Thoric */
                    && rch->master == ch && rch->position == PosStanding)
                {
                        act(AtPlain, "$n follows $N.", rch, name, ch,
                            ToRoom);
                        act(AtPlain, "You follow $N.", rch, name, ch,
                            ToChar);
                        do_board(rch, argument);
                }
        }
}

bool rent_ship(CharData * ch, ShipData * ship)
{

        long      price;

        if (IsNpc(ch))
                return FALSE;

        price = get_ship_value(ship) / 100;

        if (ch->gold < price)
        {
                ch_printf(ch,
                          "&RRenting this ship costs %ld. You don't have enough credits!\n\r",
                          price);
                return FALSE;
        }

        ch->gold -= price;
        ch_printf(ch, "&GYou pay %ld credits to rent the ship.\n\r", price);
        return TRUE;

}

CMDF do_leaveship(CharData * ch, char *argument)
{
        RoomIndexData *fromroom = NULL;
        RoomIndexData *toroom = NULL;
        ShipData *ship = NULL;
        CharData *rch = NULL;
        CharData *next_in_room = NULL;

#ifdef OlcShuttle
        ShuttleData *shuttle = NULL;
#endif
        argument = NULL;
        fromroom = ch->in_room;

        if ((ship = ship_from_entrance(fromroom->vnum)) != NULL)
        {
                if (ship->ship_class == ShipPlatform)
                {
                        send_to_char("You can't do that here.\n\r", ch);
                        return;
                }

                if (ship->lastdoc != ship->location)
                {
                        send_to_char
                                ("&rMaybe you should wait until the ship lands.\n\r",
                                 ch);
                        return;
                }

                if (ship->shipstate != ShipDocked
                    && ship->shipstate != ShipDisabled)
                {
                        send_to_char
                                ("&rPlease wait till the ship is properly docked.\n\r",
                                 ch);
                        return;
                }

                if (!ship->hatchopen)
                {
                        send_to_char("&RYou need to open the hatch first",
                                     ch);
                        return;
                }

                if ((toroom = get_room_index(ship->location)) == NULL)
                {
                        send_to_char
                                ("The exit doesn't seem to be working properly.\n\r",
                                 ch);
                        return;
                }
        }
#ifdef OlcShuttle
        else if ((shuttle = shuttle_from_entrance(fromroom->vnum)) != NULL)
        {
/*		if ( shuttle->state != ShuttleStateLanding ) */
                if (!shuttle->in_room
                    || (toroom =
                        get_room_index(shuttle->in_room->vnum)) == NULL)
                {
                        send_to_char
                                ("The ship hasn't landed yet. Do you want to kill yourself?\n\r",
                                 ch);
                        return;
                }
        }
#endif
        else
        {
                send_to_char("I see no exit here.\n\r", ch);
                return;
        }

        act(AtPlain, "$n exits the ship.", ch, NULL, NULL, ToRoom);
        act(AtPlain, "You exit the ship.", ch, NULL, NULL, ToChar);
        char_from_room(ch);
        char_to_room(ch, toroom);
        act(AtPlain, "$n steps out of a ship.", ch, NULL, NULL, ToRoom);
        do_look(ch, "auto");
        for (rch = fromroom->first_person; rch; rch = next_in_room)
        {
                next_in_room = rch->next_in_room;
                if (rch != ch   /* loop room bug fix here by Thoric */
                    && rch->master == ch && rch->position == PosStanding)
                {
                        act(AtPlain, "$n follows $N.", rch, NULL, ch,
                            ToRoom);
                        act(AtPlain, "You follow $N.", rch, NULL, ch,
                            ToChar);
                        do_leaveship(rch, argument);
                }
        }
}

CMDF do_launch(CharData * ch, char *argument)
{
        int       percent_chance = 0;
        long      price = 0;
        ShipData *ship;
        char      buf[MaxStringLength];

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if ((ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYou don't seem to be in the pilot seat!\n\r",
                             ch);
                return;
        }

        if (autofly(ship) || ship->autopilot == TRUE)
        {
                send_to_char
                        ("&RThe ship is set on autopilot, you'll have to turn it off first.\n\r",
                         ch);
                return;
        }

        if (ship->ship_class == ShipPlatform)
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!check_pilot(ch, ship))
        {
                send_to_char
                        ("&RHey, thats not your ship! Try renting a public one.\n\r",
                         ch);
                return;
        }

        if (ship->lastdoc != ship->location)
        {
                send_to_char("&rYou don't seem to be docked right now.\n\r",
                             ch);
                return;
        }

        if (ship->shipstate != ShipDocked
            && ship->shipstate != ShipDisabled)
        {
                send_to_char("The ship is not docked right now.\n\r", ch);
                return;
        }

        if (ship->type == PlayerShip && ship->lastbuilt != ship->lastroom)
        {
                send_to_char
                        ("This ship is not fully constructed, you can't launch!",
                         ch);
                return;
        }
        if (ship->ship_class == FighterShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_starfighters]);
        if (ship->ship_class == MidsizeShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_midships]);
        if (ship->ship_class == CapitalShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_capitalships]);
        if (number_percent() < percent_chance)
        {
                if (is_rental(ch, ship))
                        if (!rent_ship(ch, ship))
                                return;
                if (!is_rental(ch, ship))
                {
                        if (ship->ship_class == FighterShip)
                                price = 20;
                        if (ship->ship_class == MidsizeShip)
                                price = 50;
                        if (ship->ship_class == CapitalShip)
                                price = 500;

                        price += (ship->maxhull - ship->hull);
                        if (ship->missiles)
                                price += (50 *
                                          (ship->maxmissiles -
                                           ship->missiles));
                        else if (ship->torpedos)
                                price += (75 *
                                          (ship->maxtorpedos -
                                           ship->torpedos));
                        else if (ship->rockets)
                                price += (150 *
                                          (ship->maxrockets - ship->rockets));

                        if (ship->shipstate == ShipDisabled)
                                price += 200;
                        if (ship->missilestate == MissileDamaged)
                                price += 100;
                        if (ship->statet0 == LaserDamaged)
                                price += 50;
                        if (ship->statet1 == LaserDamaged)
                                price += 50;
                        if (ship->statet2 == LaserDamaged)
                                price += 50;
                }

                if (ch->pcdata && ch->pcdata->clan
                    && !str_cmp(ch->pcdata->clan->name, ship->owner))
                {
                        if (ch->pcdata->clan->funds < price)
                        {
                                ch_printf(ch,
                                          "&R%s doesn't have enough funds to prepare this ship for launch.\n\r",
                                          ch->pcdata->clan->name);
                                return;
                        }

                        ch->pcdata->clan->funds -= price;
                        ch_printf(ch,
                                  "&GIt costs %s %ld credits to ready this ship for launch.\n\r",
                                  ch->pcdata->clan->name, price);
                }
                else if (str_cmp(ship->owner, "Public"))
                {
                        if (ch->gold < price)
                        {
                                ch_printf(ch,
                                          "&RYou don't have enough funds to prepare this ship for launch.\n\r");
                                return;
                        }

                        ch->gold -= price;
                        ch_printf(ch,
                                  "&GYou pay %ld credits to ready the ship for launch.\n\r",
                                  price);

                }

                ship->energy = ship->maxenergy;
                ship->chaff = ship->maxchaff;
                ship->missiles = ship->maxmissiles;
                ship->torpedos = ship->maxtorpedos;
                ship->rockets = ship->maxrockets;
                ship->shield = 0;
                ship->autorecharge = FALSE;
                ship->autotrack = FALSE;
                ship->autospeed = FALSE;
                ship->hull = ship->maxhull;

                ship->missilestate = MissileReady;
                ship->statet0 = LaserReady;
                ship->statet1 = LaserReady;
                ship->statet2 = LaserReady;
                ship->shipstate = ShipDocked;

                if (ch->pcdata->clan && str_cmp(ship->owner, "Public"))
                        ship->clan = ch->pcdata->clan;
                else
                        ship->clan = NULL;

                if (ship->hatchopen)
                {
                        ship->hatchopen = FALSE;
                        snprintf(buf, MSL, "The hatch on %s closes.",
                                 ship->name);
                        echo_to_room(AtYellow,
                                     get_room_index(ship->location), buf);
                        echo_to_room(AtYellow,
                                     get_room_index(ship->entrance),
                                     "The hatch slides shut.");
                        sound_to_room(get_room_index(ship->entrance), "door");
                        sound_to_room(get_room_index(ship->location), "door");
                }
                set_char_color(AtGreen, ch);
                send_to_char("Launch sequence initiated.\n\r", ch);
                act(AtPlain,
                    "$n starts up the ship and begins the launch sequence.",
                    ch, NULL, argument, ToRoom);
                echo_to_ship(AtYellow, ship,
                             "The ship hums as it lifts off the ground.");
                snprintf(buf, MSL, "%s begins to launch.", ship->name);
                echo_to_room(AtYellow, get_room_index(ship->location), buf);
                ship->shipstate = ShipLaunch;
                ship->currspeed = ship->realspeed;
                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                sound_to_ship(ship, "xwing");
                return;
        }
        set_char_color(AtRed, ch);
        send_to_char("You fail to work the controls properly!\n\r", ch);
        if (ship->ship_class == FighterShip)
                learn_from_failure(ch, gsn_starfighters);
        if (ship->ship_class == MidsizeShip)
                learn_from_failure(ch, gsn_midships);
        if (ship->ship_class == CapitalShip)
                learn_from_failure(ch, gsn_capitalships);
        return;

}

void launchship(ShipData * ship)
{
        char      buf[MaxStringLength];
        ShipData *target;
        int       plusminus;
        bool      found = FALSE;
        SpaceData *simul;
        DockData *dock;

        for (simul = first_starsystem; simul; simul = simul->next)
        {
                if (!strcmp(simul->name, "Simulator\0"))
                        break;
        }

        if (IsSet(ship->flags, ShipSimulator))
        {
                ship_to_starsystem(ship, simul);
        }
        else
        {
                ship_to_starsystem(ship,
                                   starsystem_from_vnum(ship->location));
        }

        if ((ship->starsystem == NULL
             && !IsSet(ship->flags, ShipSimulator)))
        {
                echo_to_room(AtYellow, get_room_index(ship->pilotseat),
                             "Launch path blocked .. Launch aborted.");
                echo_to_ship(AtYellow, ship,
                             "The ship slowly sets back back down on the landing pad.");
                snprintf(buf, MSL, "%s slowly sets back down.", ship->name);
                echo_to_room(AtYellow, get_room_index(ship->location), buf);
                ship->shipstate = ShipDocked;
                return;
        }

        if (ship->ship_class == MidsizeShip)
        {
                sound_to_room(get_room_index(ship->location), "falcon");
                sound_to_ship(ship, "falcon");
        }
        else if (ship->owner == "The Empire")
        {
                sound_to_ship(ship, "tie");
                sound_to_room(get_room_index(ship->location), "tie");
        }
        else
        {
                sound_to_ship(ship, "xwing");
                sound_to_room(get_room_index(ship->location), "xwing");
        }

        extract_ship(ship);

        ship->location = 0;

        if (ship->shipstate != ShipDisabled)
                ship->shipstate = ShipReady;

        plusminus = number_range(-1, 2);
        if (plusminus > 0)
                ship->hx = 1;
        else
                ship->hx = -1;

        plusminus = number_range(-1, 2);
        if (plusminus > 0)
                ship->hy = 1;
        else
                ship->hy = -1;

        plusminus = number_range(-1, 2);
        if (plusminus > 0)
                ship->hz = 1;
        else
                ship->hz = -1;

        if (IsSet(ship->flags, ShipSimulator))
        {
                ship->vx = 1500;
                ship->vx = 1500;
                ship->vz = 1500;
        }

        /*
         * FIXME - to cleaner loop 
         */
        for (dock = first_dock; dock; dock = dock->next)
        {
                if (ship->lastdoc == dock->vnum)
                {
                        ship->vx = dock->body->xpos();
                        ship->vy = dock->body->ypos();
                        ship->vz = dock->body->zpos();
                        found = TRUE;
                }
        }

        if (!found)
        {
                for (target = ship->starsystem->first_ship; target;
                     target = target->next_in_starsystem)
                {
                        if (ship->lastdoc == target->hanger)
                        {
                                ship->vx = target->vx;
                                ship->vy = target->vy;
                                ship->vz = target->vz;
                        }
                }
        }

        ship->energy -= (100 + 100 * ship->ship_class);

        ship->sim_vnum = ship->lastdoc;

        ship->vx += (ship->hx * ship->currspeed * 2);
        ship->vy += (ship->hy * ship->currspeed * 2);
        ship->vz += (ship->hz * ship->currspeed * 2);

        echo_to_room(AtGreen, get_room_index(ship->location),
                     "Launch complete.\n\r");
        echo_to_ship(AtYellow, ship,
                     "The ship leaves the platform far behind as it flies into space.");
        if (!IsSet(ship->flags, ShipCloak)
            && !IsSet(ship->flags, ShipStealth))
        {
                snprintf(buf, MSL,
                         "%s lifts off from the planet at %.0f %.0f %.0f",
                         ship->name, ship->vx, ship->vy, ship->vz);
                echo_to_system(AtYellow, ship, buf, NULL);
        }
        else if (!IsSet(ship->flags, ShipCloak)
                 && IsSet(ship->flags, ShipStealth))
        {
                snprintf(buf, MSL,
                         "You notice a ship lifting off from a planet");
                echo_to_system(AtYellow, ship, buf, NULL);
        }
        snprintf(buf, MSL, "%s lifts off into space.", ship->name);
        echo_to_room(AtYellow, get_room_index(ship->lastdoc), buf);

}

CMDF do_land(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent_chance = 0;
        ShipData *ship;
        ShipData *target;
        DockData *dock = NULL;

        mudstrlcpy(arg, argument, MIL);

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class >= ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if (ship->dockedto)
        {
                send_to_char("You are currently docked\n\r", ch);
                return;
        }

        if (ship->pilotseat != ch->in_room->vnum)
        {
                send_to_char("&RYou need to be in the pilot seat!\n\r", ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first.\n\r",
                         ch);
                return;
        }

        if (IsSet(ship->flags, ShipCloak))
        {
                send_to_char("&RYou can't land a cloaked ship!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipDisabled)
        {
                send_to_char
                        ("&RThe ships drive is disabled. Unable to land.\n\r",
                         ch);
                return;
        }

        if (ship->shipstate == ShipDocked)
        {
                send_to_char("&RThe ship is already docked!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }

        if (ship->shipstate != ShipReady)
        {
                send_to_char
                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                         ch);
                return;
        }

        if (ship->starsystem == NULL)
        {
                send_to_char("&RThere's nowhere to land around here!", ch);
                return;
        }

        if (IsSet(ship->flags, ShipInterdictor))
        {
                send_to_char
                        ("&RYou can not land while generating a gravity cone!\n\r",
                         ch);
                return;
        }

        if (ship->energy < (25 + 25 * ship->ship_class))
        {
                send_to_char("&RTheres not enough fuel!\n\r", ch);
                return;
        }


        if (argument[0] == '\0')
        {
                BodyData *body = NULL;

                set_char_color(AtCyan, ch);
                send_to_char("Land where?\n\r\n\rChoices:\n\r", ch);

                send_to_char("Planets:\n\r", ch);
                ForEachList(BodyList, ship->starsystem->bodies, body)
                {
                        if (body->type() != PlanetBody)
                                continue;
                        ForEachList(DockList, body->docks(), dock)
                        {
                                if (dock->hidden)
                                        continue;
                                ch_printf(ch,
                                          "         %s (%s) %d %d %d Distance: %d\n\r",
                                          dock->name, dock->body->name(),
                                          dock->body->xpos(),
                                          dock->body->ypos(),
                                          dock->body->zpos(),
                                          body->distance(ship));
                        }
                }

                send_to_char("Moon:\n\r", ch);
                ForEachList(BodyList, ship->starsystem->bodies, body)
                {
                        if (body->type() != MoonBody)
                                continue;
                        ForEachList(DockList, body->docks(), dock)
                        {
                                if (dock->hidden)
                                        continue;
                                ch_printf(ch,
                                          "         %s (%s) %d %d %d Distance: %d\n\r",
                                          dock->name, dock->body->name(),
                                          dock->body->xpos(),
                                          dock->body->ypos(),
                                          dock->body->zpos(),
                                          body->distance(ship));
                        }
                }

                send_to_char("Ships:\n\r", ch);
                for (target = ship->starsystem->first_ship; target;
                     target = target->next_in_starsystem)
                {
                        if (target->hanger > 0 && target != ship)
                                ch_printf(ch,
                                          "         %s    %.0f %.0f %.0f Distance: %d\n\r",
                                          target->name, target->vx,
                                          target->vy, target->vz,
                                          distance_ship_ship(target, ship));
                }
                ch_printf(ch, "\n\rYour Coordinates: %.0f %.0f %.0f\n\r",
                          ship->vx, ship->vy, ship->vz);
                return;
        }


        if ((dock = get_dock_isname(ship, argument)) == NULL)
        {
                target = get_ship_here(argument, ship->starsystem);
                if (target == NULL)
                {
                        send_to_char
                                ("&RI don't see that here. Type land by itself for a list\n\r",
                                 ch);
                        return;
                }
                if (target == ship)
                {
                        send_to_char
                                ("&RYou can't land your ship inside itself!\n\r",
                                 ch);
                        return;
                }
                if (!target->hanger)
                {
                        send_to_char
                                ("&RThat ship has no hanger for you to land in!\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class == MidsizeShip
                    && target->ship_class == MidsizeShip)
                {
                        send_to_char
                                ("&RThat ship is not big enough for your ship to land in!\n\r",
                                 ch);
                        return;
                }

                if (ship->ship_class == CapitalShip
                    && target->ship_class != ShipPlatform)
                {
                        send_to_char
                                ("&RCapital ships are to big to land. You'll have to take a shuttle.\n\r",
                                 ch);
                        return;
                }
                if (!target->bayopen)
                {
                        send_to_char
                                ("&RTheir hanger is closed. You'll have to ask them to open it for you\n\r",
                                 ch);
                        return;
                }
                if ((target->vx > ship->vx + 200)
                    || (target->vx < ship->vx - 200)
                    || (target->vy > ship->vy + 200)
                    || (target->vy < ship->vy - 200)
                    || (target->vz > ship->vz + 200)
                    || (target->vz < ship->vz - 200))
                {
                        send_to_char
                                ("&R That ship is too far away! You'll have to fly a little closer.\n\r",
                                 ch);
                        return;
                }
        }
        else
        {
                if (ship->ship_class == CapitalShip)
                {
                        send_to_char
                                ("&RCapital ships are to big to land. You'll have to take a shuttle.\n\r",
                                 ch);
                        return;
                }

                if (dock->body->distance(ship) > 200)
                {
                        send_to_char
                                ("&RThat docking pad is way to far away! You'll have to fly a little closer.\n\r",
                                 ch);
                        return;
                }
        }

        if (ship->ship_class == FighterShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_starfighters]);
        if (ship->ship_class == MidsizeShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_midships]);
        if (ship->ship_class == CapitalShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_capitalships]);
        if (number_percent() < percent_chance)
        {
                set_char_color(AtGreen, ch);
                send_to_char("Landing sequence initiated.\n\r", ch);
                act(AtPlain, "$n begins the landing sequence.", ch,
                    NULL, argument, ToRoom);
                echo_to_ship(AtYellow, ship,
                             "The ship slowly begins its landing aproach.");
                ship->dest = STRALLOC(arg);
                ship->shipstate = ShipLand;
                ship->currspeed = 0;
                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (starsystem_from_vnum(ship->lastdoc) != ship->starsystem)
                {
                        int       xp =
                                (exp_level
                                 (ch->skill_level[PilotingAbility] + 1) -
                                 exp_level(ch->
                                           skill_level[PilotingAbility]));
                        xp = UMin(get_ship_value(ship), xp);
                        gain_exp(ch, xp, PilotingAbility);
                        ch_printf(ch,
                                  "&WYou gain %ld points of flight experience!\n\r",
                                  UMin(get_ship_value(ship), xp));
                }
                return;
        }
        send_to_char("You fail to work the controls properly.\n\r", ch);
        if (ship->ship_class == FighterShip)
                learn_from_failure(ch, gsn_starfighters);
        else
                learn_from_failure(ch, gsn_midships);
        return;
}

void landship(ShipData * ship, char *arg)
{
        ShipData *target;
        char      buf[MaxStringLength];
        int       destination = -1;
        DockData *dock;

        if (ship->dockedto)
        {
                echo_to_room(AtYellow, get_room_index(ship->pilotseat),
                             "Could not complete aproach. Attached to ship. Landing aborted.");
                echo_to_ship(AtYellow, ship,
                             "The ship pulls back up out of its landing sequence.");
                if (ship->shipstate != ShipDisabled)
                        ship->shipstate = ShipReady;
                return;
        }
        if ((dock = get_dock_isname(ship, arg)) != NULL)
                destination = dock->vnum;
        else
        {
                if ((target = get_ship_here(arg, ship->starsystem)) != NULL)
                {
                        if (target != ship && target->bayopen)
                                destination = target->hanger;
                        else if (target == ship)
                        {
                                bug("%s is trying to land on itself",
                                    ship->name);
                        }
                        else if (!target->bayopen)
                        {
                                echo_to_room(AtYellow,
                                             get_room_index(ship->pilotseat),
                                             "Hanger doors appear to be closed.");
                                /*
                                 * ADD DAMAGE HERE - FIXME 
                                 */
                        }
                }
        }

        if (destination == -1 || !ship_to_room(ship, destination))
        {
                echo_to_room(AtYellow, get_room_index(ship->pilotseat),
                             "Could not complete aproach. Landing aborted.");
                echo_to_ship(AtYellow, ship,
                             "The ship pulls back up out of its landing sequence.");
                if (ship->shipstate != ShipDisabled)
                        ship->shipstate = ShipReady;
                return;
        }
        echo_to_room(AtYellow, get_room_index(ship->pilotseat),
                     "Landing sequence complete.");
        echo_to_ship(AtYellow, ship,
                     "You feel a slight thud as the ship sets down on the ground.");
        snprintf(buf, MSL, "%s disapears from your scanner.", ship->name);
        echo_to_system(AtYellow, ship, buf, NULL);

        ship->location = destination;
        ship->lastdoc = ship->location;
        if (ship->shipstate != ShipDisabled)
                ship->shipstate = ShipDocked;
        ship_from_starsystem(ship, ship->starsystem);

        snprintf(buf, MSL, "%s lands on the platform.", ship->name);
        echo_to_room(AtYellow, get_room_index(ship->location), buf);

        ship->energy = ship->energy - 25 - 25 * ship->ship_class;

        if (!str_cmp("Public", ship->owner))
        {
                ship->energy = ship->maxenergy;
                ship->chaff = ship->maxchaff;
                ship->missiles = ship->maxmissiles;
                ship->torpedos = ship->maxtorpedos;
                ship->rockets = ship->maxrockets;
                ship->shield = 0;
                ship->autorecharge = FALSE;
                ship->autotrack = FALSE;
                ship->autospeed = FALSE;
                ship->hull = ship->maxhull;

                ship->missilestate = MissileReady;
                ship->statet0 = LaserReady;
                ship->statet1 = LaserReady;
                ship->statet2 = LaserReady;
                ship->shipstate = ShipDocked;
                echo_to_cockpit(AtYellow, ship,
                                "Repairing and refueling ship...");
        }
        STRFREE(ship->dest);
        save_ship(ship);
}


CMDF do_accelerate(CharData * ch, char *argument)
{
        int       percent_chance = 0;
        int       change;
        ShipData *ship;
        char      buf[MaxStringLength];

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if (ship->dockedto)
        {
                send_to_char("You are currently docked.", ch);
                return;
        }

        /*
         * Silly loops, now its faster 
         */
        if (ship->pilotseat != ch->in_room->vnum)
        {
                send_to_char
                        ("&RThe controls must be at the pilots chair...\n\r",
                         ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first.\n\r",
                         ch);
                return;
        }

        if (ship->ship_class == ShipPlatform)
        {
                send_to_char("&RPlatforms can't move!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }
        if (ship->shipstate == ShipDisabled)
        {
                send_to_char
                        ("&RThe ships drive is disabled. Unable to accelerate.\n\r",
                         ch);
                return;
        }
        if (ship->shipstate == ShipDocked)
        {
                send_to_char
                        ("&RYou can't do that until after you've launched!\n\r",
                         ch);
                return;
        }
        if (ship->energy < abs((atoi(argument) - abs(ship->currspeed)) / 10))
        {
                send_to_char("&RTheres not enough fuel!\n\r", ch);
                return;
        }

        if (ship->ship_class == FighterShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_starfighters]);
        if (ship->ship_class == MidsizeShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_midships]);
        if (ship->ship_class == CapitalShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_capitalships]);
        if (number_percent() >= percent_chance)
        {
                send_to_char("&RYou fail to work the controls properly.\n\r",
                             ch);
                if (ship->ship_class == FighterShip)
                        learn_from_failure(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_failure(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_failure(ch, gsn_capitalships);
                return;
        }

        change = atoi(argument);

        act(AtPlain, "$n manipulates the ships controls.", ch,
            NULL, argument, ToRoom);

        if (change > ship->currspeed)
        {
                send_to_char("&GAccelerating\n\r", ch);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to accelerate.");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL, "%s begins to Speed up.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
        }

        if (change < ship->currspeed)
        {
                send_to_char("&GDecelerating\n\r", ch);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to slow down.");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL, "%s begins to slow down.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
        }

        ship->energy -= abs((change - abs(ship->currspeed)) / 10);

        ship->currspeed = URange(0, change, ship->realspeed);

        if (ship->ship_class == FighterShip)
                learn_from_success(ch, gsn_starfighters);
        if (ship->ship_class == MidsizeShip)
                learn_from_success(ch, gsn_midships);
        if (ship->ship_class == CapitalShip)
                learn_from_success(ch, gsn_capitalships);

}

CMDF do_trajectory(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        int       percent_chance = 0;
        float     vx, vy, vz;
        ShipData *ship;


        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if ((ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYour not in the pilots seat.\n\r", ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first.\n\r",
                         ch);
                return;
        }

        if (ship->shipstate == ShipDisabled)
        {
                send_to_char
                        ("&RThe ships drive is disabled. Unable to manuever.\n\r",
                         ch);
                return;
        }
        if (ship->ship_class == ShipPlatform)
        {
                send_to_char("&RPlatforms can't turn!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }
        if (ship->shipstate == ShipDocked)
        {
                send_to_char
                        ("&RYou can't do that until after you've launched!\n\r",
                         ch);
                return;
        }
        if (ship->shipstate != ShipReady)
        {
                send_to_char
                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                         ch);
                return;
        }
        if (ship->energy < (ship->currspeed / 10))
        {
                send_to_char("&RTheres not enough fuel!\n\r", ch);
                return;
        }

        if (ship->ship_class == FighterShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_starfighters]);
        if (ship->ship_class == MidsizeShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_midships]);
        if (ship->ship_class == CapitalShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_capitalships]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYou fail to work the controls properly.\n\r",
                             ch);
                if (ship->ship_class == FighterShip)
                        learn_from_failure(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_failure(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_failure(ch, gsn_capitalships);
                return;
        }

        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

/* Autocourses */
        if (!str_prefix(arg2, "options"))
        {
                ch_printf(ch, "&GOptions for course command.\n\r");
                ch_printf(ch,
                          "&GCoordinates: <coord x> <coord y> <coord z>.\n\r");
                ch_printf(ch,
                          "&G  Setting your coordinates will turn you towards those you choose.\n\r");
                ch_printf(ch, "&GTarget - turn towards your target.\n\r");
                ch_printf(ch, "&GFlee - turn away from your target.\n\r");
                ch_printf(ch, "&GUp - turn straight up.\n\r");
                ch_printf(ch, "&GDown - turn straight down.\n\r");
                ch_printf(ch, "&GEast - turn straight east.\n\r");
                ch_printf(ch, "&GWest - turn straight west.\n\r");
                ch_printf(ch, "&GNorth - turn straight north.\n\r");
                ch_printf(ch, "&GSouth - turn straight south.\n\r");
                return;
        }


        if (!str_prefix(arg2, "target"))
        {
                ship->hx = ship->target0->vx - ship->vx;
                ship->hy = ship->target0->vy - ship->vy;
                ship->hz = ship->target0->vz - ship->vz;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch,
                          "&GNew course set towards target %.0f %.0f %.0f.\n\r",
                          ship->target0->vx, ship->target0->vy,
                          ship->target0->vz);
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

        if (!str_prefix(arg2, "flee"))
        {
                ship->hx = ship->target0->vx + ship->vx;
                ship->hy = ship->target0->vy + ship->vy;
                ship->hz = ship->target0->vz + ship->vz;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch,
                          "&GNew course set away from target %.0f %.0f %.0f.\n\r",
                          ship->target0->vx, ship->target0->vy,
                          ship->target0->vz);
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

        if (!str_prefix(arg2, "up"))
        {
                ship->hx = 0;
                ship->hy = 0;
                ship->hz = 1;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch, "&GNew course set straight up.\n\r");
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

        if (!str_prefix(arg2, "down"))
        {
                ship->hx = 0;
                ship->hy = 0;
                ship->hz = -1;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch, "&GNew course set straight down.\n\r");
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

        if (!str_prefix(arg2, "north"))
        {
                ship->hx = 0;
                ship->hy = 1;
                ship->hz = 0;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch, "&GNew course set straight north.\n\r");
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

        if (!str_prefix(arg2, "south"))
        {
                ship->hx = 0;
                ship->hy = -1;
                ship->hz = 0;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch, "&GNew course set straight south.\n\r");
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

        if (!str_prefix(arg2, "east"))
        {
                ship->hx = 1;
                ship->hy = 0;
                ship->hz = 0;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch, "&GNew course set straight east.\n\r");
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

        if (!str_prefix(arg2, "west"))
        {
                ship->hx = -1;
                ship->hy = 0;
                ship->hz = 0;
                ship->energy -= (ship->currspeed / 10);
                ship->evasive = 0;
                ch_printf(ch, "&GNew course set straight west.\n\r");
                act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
                    argument, ToRoom);
                echo_to_cockpit(AtYellow, ship,
                                "The ship begins to turn.\n\r");
                if (!IsSet(ship->flags, ShipCloak))
                {
                        snprintf(buf, MSL,
                                 "%s turns altering its present course.",
                                 ship->name);
                        echo_to_system(AtOrange, ship, buf, NULL);
                }
                if (ship->ship_class == FighterShip
                    || (ship->ship_class == MidsizeShip
                        && ship->manuever > 50))
                        ship->shipstate = ShipBusy3;
                else if (ship->ship_class == MidsizeShip
                         || (ship->ship_class == CapitalShip
                             && ship->manuever > 50))
                        ship->shipstate = ShipBusy2;
                else
                        ship->shipstate = ShipBusy;

                if (ship->ship_class == FighterShip)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_success(ch, gsn_capitalships);
                return;
        }

/*End Autocourses*/
        vx = atof(arg2);
        vy = atof(arg3);
        vz = atof(argument);

        if (vx == ship->vx && vy == ship->vy && vz == ship->vz)
        {
                ch_printf(ch, "The ship is already at %.0f %.0f %.0f !", vx,
                          vy, vz);
        }

        ship->hx = vx - ship->vx;
        ship->hy = vy - ship->vy;
        ship->hz = vz - ship->vz;

        ship->energy -= (ship->currspeed / 10);
        ship->evasive = 0;

        ch_printf(ch, "&GNew course set, aproaching %.0f %.0f %.0f.\n\r", vx,
                  vy, vz);
        act(AtPlain, "$n manipulates the ships controls.", ch, NULL,
            argument, ToRoom);

        echo_to_cockpit(AtYellow, ship, "The ship begins to turn.\n\r");
        if (!IsSet(ship->flags, ShipCloak))
        {
                snprintf(buf, MSL, "%s turns altering its present course.",
                         ship->name);
                echo_to_system(AtOrange, ship, buf, NULL);
        }

        if (ship->ship_class == FighterShip
            || (ship->ship_class == MidsizeShip && ship->manuever > 50))
                ship->shipstate = ShipBusy3;
        else if (ship->ship_class == MidsizeShip
                 || (ship->ship_class == CapitalShip && ship->manuever > 50))
                ship->shipstate = ShipBusy2;
        else
                ship->shipstate = ShipBusy;

        if (ship->ship_class == FighterShip)
                learn_from_success(ch, gsn_starfighters);
        if (ship->ship_class == MidsizeShip)
                learn_from_success(ch, gsn_midships);
        if (ship->ship_class == CapitalShip)
                learn_from_success(ch, gsn_capitalships);

}


CMDF do_buyship(CharData * ch, char *argument)
{
        long      price;
        ShipData *ship;
        ObjData *obj;



        if (IsNpc(ch) || !ch->pcdata)
        {
                send_to_char("&ROnly players can do that!\n\r", ch);
                return;
        }

        ship = ship_in_room(ch->in_room, argument);
        if (!ship)
        {
                ship = ship_from_cockpit(ch->in_room->vnum);

                if (!ship)
                {
                        act(AtPlain, "I see no $T here.", ch, NULL, argument,
                            ToChar);
                        return;
                }
        }

        if (str_cmp(ship->owner, "") || ship->type == MobShip)
        {
                send_to_char("&RThat ship isn't for sale!", ch);
                return;
        }



        price = get_ship_value(ship);
        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->pIndexData->vnum == 10326
                    && ship->ship_class == FighterShip)
                {
                        ch_printf(ch,
                                  "&GYou pay trade you deed for owner ship of the ship.\n\r");
                        act(AtPlain,
                            "$n walks over to a terminal and makes a titleship transaction.",
                            ch, NULL, argument, ToRoom);

                        STRFREE(ship->owner);
                        ship->owner = STRALLOC(ch->name);
                        save_ship(ship);
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        return;
                }
        }


        if (ch->gold < price)
        {
                ch_printf(ch,
                          "&RThis ship costs %ld. You don't have enough credits!\n\r",
                          price);
                return;
        }

        ch->gold -= price;
        ch_printf(ch, "&GYou pay %ld credits to purchase the ship.\n\r",
                  price);

        act(AtPlain,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, ToRoom);

        STRFREE(ship->owner);
        ship->owner = STRALLOC(ch->name);
        save_ship(ship);

}

CMDF do_clanbuyship(CharData * ch, char *argument)
{
        long      price;
        ShipData *ship;
        ClanData *clan;
        ClanData *mainclan;

        if (IsNpc(ch) || !ch->pcdata)
        {
                send_to_char("&ROnly players can do that!\n\r", ch);
                return;
        }
        if (!ch->pcdata->clan)
        {
                send_to_char
                        ("&RYou aren't a member of any organizations!\n\r",
                         ch);
                return;
        }

        clan = ch->pcdata->clan;
        mainclan =
                ch->pcdata->clan->mainclan ? ch->pcdata->clan->
                mainclan : clan;

        if (!HasClanPerm(ch, clan, "clanbuyship"))
        {
                send_to_char
                        ("&RYour organization hasn't seen fit to bestow you with that ability.\n\r",
                         ch);
                return;
        }

        ship = ship_in_room(ch->in_room, argument);
        if (!ship)
        {
                ship = ship_from_cockpit(ch->in_room->vnum);

                if (!ship)
                {
                        act(AtPlain, "I see no $T here.", ch, NULL, argument,
                            ToChar);
                        return;
                }
        }

        if (str_cmp(ship->owner, "") || ship->type == MobShip)
        {
                send_to_char("&RThat ship isn't for sale!\n\r", ch);
                return;
        }

        price = get_ship_value(ship);

        if (ch->pcdata->clan->funds < price)
        {
                ch_printf(ch,
                          "&RThis ship costs %ld. You don't have enough credits!\n\r",
                          price);
                return;
        }

        clan->funds -= price;
        ch_printf(ch, "&G%s pays %ld credits to purchase the ship.\n\r",
                  clan->name, price);

        act(AtPlain,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, ToRoom);

        STRFREE(ship->owner);
        ship->owner = STRALLOC(clan->name);
        save_ship(ship);

        if (ship->ship_class <= ShipPlatform)
                clan->spacecraft++;
}

CMDF do_clansellship(CharData * ch, char *argument)
{
        long      price;
        ShipData *ship;
        ClanData *clan;

        if (IsNpc(ch) || !ch->pcdata)
        {
                send_to_char("&ROnly players can do that!\n\r", ch);
                return;
        }

        if (!ch->pcdata->clan)
        {
                send_to_char("&RYou aren't a member of an organization!\n\r",
                             ch);
                return;
        }

        clan = ch->pcdata->clan;

        if (!HasClanPerm(ch, clan, "clansellship"))
        {
                send_to_char
                        ("&RYour organization hasn't seen fit to bestow you with that ability.\n\r",
                         ch);
                return;
        }

        ship = ship_in_room(ch->in_room, argument);
        if (!ship)
        {
                ship = ship_from_cockpit(ch->in_room->vnum);
                if (!ship)
                {
                        act(AtPlain, "I see no $T here.", ch, NULL, argument,
                            ToChar);
                        return;
                }
        }

        if (!check_pilot(ch, ship) || !str_cmp(ship->owner, "Public"))
        {
                send_to_char("&RHey, thats not your ship!\n\r", ch);
                return;
        }

        price = get_ship_value(ship);
        clan->funds += (price - price / 10);
        ch_printf(ch, "&G%s receive %ld credits from selling %s's ship.\n\r",
                  clan->name, price - price / 10, clan->name);
        act(AtPlain,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, ToRoom);
        STRFREE(ship->owner);
        ship->owner = STRALLOC("");
        STRFREE(ship->pilot);
        ship->pilot = STRALLOC("");
        STRFREE(ship->copilot);
        ship->copilot = STRALLOC("");
        ship->selfdpass = number_range(10001, 99999);
        save_ship(ship);
}

CMDF do_sellship(CharData * ch, char *argument)
{
        long      price;
        ShipData *ship;

        ship = ship_in_room(ch->in_room, argument);
        if (!ship)
        {
                act(AtPlain, "I see no $T here.", ch, NULL, argument,
                    ToChar);
                return;
        }

        if (str_cmp(ship->owner, ch->name))
        {
                send_to_char("&RThat isn't your ship!", ch);
                return;
        }

        price = get_ship_value(ship);

        ch->gold += (price - price / 10);
        ch_printf(ch, "&GYou receive %ld credits from selling your ship.\n\r",
                  price - price / 10);

        act(AtPlain,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, ToRoom);

        STRFREE(ship->owner);
        ship->owner = STRALLOC("");
        STRFREE(ship->pilot);
        ship->pilot = STRALLOC("");
        STRFREE(ship->copilot);
        ship->copilot = STRALLOC("");
        ship->selfdpass = number_range(10001, 99999);
        save_ship(ship);
}

CMDF do_info(CharData * ch, char *argument)
{
        ShipData *ship;
        ShipData *target;
        bool      owner = FALSE;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                if (argument[0] == '\0')
                {
                        act(AtPlain, "Which ship do you want info on?.", ch,
                            NULL, NULL, ToChar);
                        return;
                }

                ship = ship_in_room(ch->in_room, argument);
                if (!ship)
                {
                        act(AtPlain, "I see no $T here.", ch, NULL, argument,
                            ToChar);
                        return;
                }

                target = ship;
        }
        else if (argument[0] == '\0')
                target = ship;
        else
                target = get_ship_here(argument, ship->starsystem);

        if (target == NULL)
        {
                send_to_char
                        ("&RI don't see that here.\n\rTry the radar, or type info by itself for info on this ship.\n\r",
                         ch);
                return;
        }

        owner = (check_pilot(ch, target) || IsImmortal(ch));
        if (abs((int) target->vx - (int) ship->vx) > 500 + ship->sensor * 2 ||
            abs((int) target->vy - (int) ship->vy) > 500 + ship->sensor * 2 ||
            abs((int) target->vz - (int) ship->vz) > 500 + ship->sensor * 2)
        {
                send_to_char("&RThat ship is to far away to scan.\n\r", ch);
                return;
        }

        ch_printf(ch, "&Y%s %s : %s\n\r&B",
                  target->type == Republic ? "New Republic" :
                  (target->type == Imperial ? "Imperial" : "Civilian"),
                  target->ship_class == FighterShip ? "Starfighter" :
                  (target->ship_class == MidsizeShip ? "Midtarget" :
                   (target->ship_class == CapitalShip ? "Capital Ship" :
                    (ship->ship_class ==
                     ShipPlatform ? "Platform" : "Unknown"))), target->name,
                  target->filename);
        ch_printf(ch,
                  "Description: %s\n\rOwner: %s   Pilot: %s   Copilot: %s\n\r",
                  target->description ? target->description : "(none)",
                  target->owner[0] ==
                  '\0' ? "Unowned" : owner ? target->owner : "---------",
                  owner ? target->pilot : "---------",
                  owner ? target->copilot : "---------");
        ch_printf(ch,
                  "Laser cannons: %d  Maximum Missiles: %d  Max Chaff: %d  Max Bombs: %d\n\r",
                  target->lasers, target->maxmissiles, target->maxchaff,
                  target->maxbombs);
        ch_printf(ch,
                  "Max Hull: %d  Max Shields: %d   Max Energy(fuel): %d  Manueverability: %d\n\r",
                  target->maxhull, target->maxshield, target->maxenergy,
                  target->manuever);
        ch_printf(ch,
                  "Maximum Speed: %d   Hyperspeed: %d  Maximum Cargo %d\n\r",
                  target->realspeed, target->hyperspeed, target->maxcargo);
        ch_printf(ch, "battalions: %d/%d   Ion cannons: %d Sensors: %d\n\r",
                  target->battalions, target->maxbattalions, target->ions,
                  target->sensor);

        if (owner)
        {
                ch_printf(ch, "Selfdestruct Pass: %d\n\r", target->selfdpass);
        }

        act(AtPlain,
            "$n checks various gages and displays on the control panel.", ch,
            NULL, argument, ToRoom);

}

CMDF do_autorecharge(CharData * ch, char *argument)
{
        int       percent_chance = 0;
        ShipData *ship;
        int       recharge;


        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYou must be in the co-pilots seat!\n\r", ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first.\n\r",
                         ch);
                return;
        }

        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_shipsystems]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYou fail to work the controls properly.\n\r",
                             ch);
                learn_from_failure(ch, gsn_shipsystems);
                return;
        }

        act(AtPlain, "$n flips a switch on the control panell.", ch,
            NULL, argument, ToRoom);

        if (!str_cmp(argument, "on"))
        {
                ship->autorecharge = TRUE;
                send_to_char("&GYou power up the shields.\n\r", ch);
                echo_to_cockpit(AtYellow, ship,
                                "Shields ON. Autorecharge ON.");
        }
        else if (!str_cmp(argument, "off"))
        {
                ship->autorecharge = FALSE;
                send_to_char("&GYou shutdown the shields.\n\r", ch);
                echo_to_cockpit(AtYellow, ship,
                                "Shields OFF. Shield Strength set to 0. Autorecharge OFF.");
                ship->shield = 0;
        }
        else if (!str_cmp(argument, "idle"))
        {
                ship->autorecharge = FALSE;
                send_to_char("&GYou let the shields idle.\n\r", ch);
                echo_to_cockpit(AtYellow, ship,
                                "Autorecharge OFF. Shields IDLEING.");
        }
        else
        {
                if (ship->autorecharge == TRUE)
                {
                        ship->autorecharge = FALSE;
                        send_to_char("&GYou toggle the shields.\n\r", ch);
                        echo_to_cockpit(AtYellow, ship,
                                        "Autorecharge OFF. Shields IDLEING.");
                }
                else
                {
                        ship->autorecharge = TRUE;
                        send_to_char("&GYou toggle the shields.\n\r", ch);
                        echo_to_cockpit(AtYellow, ship,
                                        "Shields ON. Autorecharge ON");
                }
        }

        if (ship->autorecharge)
        {
                recharge =
                        URange(1, ship->maxshield - ship->shield,
                               25 + ship->ship_class * 25);
                recharge = UMin(recharge, ship->energy * 5 + 100);
                ship->shield += recharge;
                ship->energy -= (recharge * 2 + recharge * ship->ship_class);
        }

        learn_from_success(ch, gsn_shipsystems);
}

CMDF do_autopilot(CharData * ch, char *argument)
{
        ShipData *ship;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if ((ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYou must be in the pilots seat!\n\r", ch);
                return;
        }

        if (!check_pilot(ch, ship))
        {
                send_to_char("&RHey! Thats not your ship!\n\r", ch);
                return;
        }

        if (ship->target0 || ship->target1 || ship->target2)
        {
                send_to_char
                        ("&RNot while the ship is enganged with an enemy!\n\r",
                         ch);
                return;
        }


        act(AtPlain, "$n flips a switch on the control panell.", ch,
            NULL, argument, ToRoom);

        if (ship->autopilot == TRUE)
        {
                ship->autopilot = FALSE;
                send_to_char("&GYou toggle the autopilot.\n\r", ch);
                echo_to_cockpit(AtYellow, ship, "Autopilot OFF.");
        }
        else
        {
                ship->autopilot = TRUE;
                ship->autorecharge = TRUE;
                send_to_char("&GYou toggle the autopilot.\n\r", ch);
                echo_to_cockpit(AtYellow, ship, "Autopilot ON.");
        }

}

CMDF do_openhatch(CharData * ch, char *argument)
{
        ShipData *ship;
        char      buf[MaxStringLength];

        if (!argument || argument[0] == '\0' || !str_cmp(argument, "hatch"))
        {
                if ((ship = ship_from_entrance(ch->in_room->vnum)) == NULL)
                {
                        send_to_char("&ROpen what?\n\r", ch);
                        return;
                }
                else
                {
                        if (!ship->hatchopen)
                        {
                                if (ship->ship_class == ShipPlatform)
                                {

                                        send_to_char
                                                ("&RTry one of the docking bays!\n\r",
                                                 ch);
                                        return;
                                }
                                if (ship->dockedto)
                                {
                                        if (check_pilot(ch, ship->dockedto))
                                                ship->dockedto->hatchopen =
                                                        TRUE;
                                        else
                                                send_to_char
                                                        ("You will have to get someone on the other side to open thier hatch.\n\r",
                                                         ch);
                                }
                                else if (ship->location != ship->lastdoc ||
                                         (ship->shipstate != ShipDocked
                                          && ship->shipstate !=
                                          ShipDisabled))
                                {
                                        send_to_char
                                                ("&RPlease wait till the ship lands!\n\r",
                                                 ch);
                                        return;
                                }
                                ship->hatchopen = TRUE;
                                send_to_char("&GYou open the hatch.\n\r", ch);
                                act(AtPlain, "$n opens the hatch.", ch, NULL,
                                    argument, ToRoom);
                                sound_to_room(get_room_index(ship->entrance),
                                              "door");
                                if (!ship->dockedto)
                                {
                                        snprintf(buf, MSL,
                                                 "The hatch on %s opens.",
                                                 ship->name);
                                        echo_to_room(AtYellow,
                                                     get_room_index(ship->
                                                                    location),
                                                     buf);
                                        sound_to_room(get_room_index
                                                      (ship->location),
                                                      "door");
                                }
                                else
                                {
                                        ship->dockedto->hatchopen = TRUE;
                                        echo_to_room(AtPlain,
                                                     get_room_index(ship->
                                                                    dockedto->
                                                                    entrance),
                                                     "&wThe hatch opens.");
                                }
                                return;
                        }
                        else
                        {
                                send_to_char("&RIt's already open.\n\r", ch);
                                return;
                        }
                }
        }

        ship = ship_in_room(ch->in_room, argument);
        if (!ship)
        {
                act(AtPlain, "I see no $T here.", ch, NULL, argument,
                    ToChar);
                return;
        }

        if (ship->shipstate != ShipDocked
            && ship->shipstate != ShipDisabled)
        {
                send_to_char("&RThat ship has already started to launch", ch);
                return;
        }

        if (!check_pilot(ch, ship))
        {
                send_to_char("&RHey! Thats not your ship!\n\r", ch);
                return;
        }

        if (!ship->hatchopen)
        {
                ship->hatchopen = TRUE;
                act(AtPlain, "You open the hatch on $T.", ch, NULL,
                    ship->name, ToChar);
                act(AtPlain, "$n opens the hatch on $T.", ch, NULL,
                    ship->name, ToRoom);
                echo_to_room(AtYellow, get_room_index(ship->entrance),
                             "The hatch opens from the outside.");
                sound_to_room(get_room_index(ship->entrance), "door");
                sound_to_room(get_room_index(ship->location), "door");
                return;
        }

        send_to_char("&GIts already open!\n\r", ch);

}



CMDF do_transship(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        int       arg3;
        ShipData *ship = NULL;
        SpaceData *starsystem = NULL;

        if (IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);


        if (arg1[0] == '\0')
        {
                send_to_char("Usage: transship <ship>\n\r", ch);
                send_to_char("Usage: transship <ship> <vnum/system>\n\r", ch);
                return;
        }

        ship = get_ship(arg1);

        if (!ship)
        {
                send_to_char("No such ship.\n\r", ch);
                return;
        }

        if (arg2 && arg2[0] != '\0')
                starsystem = starsystem_from_name(arg2);

        if ((!arg2 || arg2[0] == '\0') && ch->in_room)
                arg3 = ch->in_room->vnum;
        else
                arg3 = atoi(arg2);

        if (ship->starsystem)
                ship_from_starsystem(ship, ship->starsystem);
        if (starsystem)
        {
                extract_ship(ship);
                ship_to_starsystem(ship, starsystem);
                ship->shipyard = 0;
                ship->location = 0;
                ship->shipstate = ShipReady;
                ship->vx = number_range(-5000, 5000);
                ship->vy = number_range(-5000, 5000);
                ship->vz = number_range(-5000, 5000);
        }
        else
        {
                ship->shipyard = arg3;
                ship->shipstate = ShipReady;

                if (ship->ship_class != ShipPlatform
                    && ship->type != MobShip)
                {
                        extract_ship(ship);
                        ship_to_room(ship, ship->shipyard);
                        ship->location = ship->shipyard;
                        ship->lastdoc = ship->shipyard;
                        ship->shipstate = ShipDocked;
                }
        }

        if (ship->dockedto)
        {
                ship->dockedto->dockedto = NULL;
                ship->dockedto = NULL;
        }
        save_ship(ship);
        send_to_char("Ship Transfered.\n\r", ch);
}


CMDF do_closehatch(CharData * ch, char *argument)
{
        ShipData *ship;
        char      buf[MaxStringLength];

        if (!argument || argument[0] == '\0' || !str_cmp(argument, "hatch"))
        {
                ship = ship_from_entrance(ch->in_room->vnum);
                if (ship == NULL)
                {
                        send_to_char("&RClose what?\n\r", ch);
                        return;
                }
                else
                {

                        if (ship->ship_class == ShipPlatform)
                        {
                                send_to_char
                                        ("&RTry one of the docking bays!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->hatchopen)
                        {
                                ship->hatchopen = FALSE;
                                send_to_char("&GYou close the hatch.\n\r",
                                             ch);
                                act(AtPlain, "$n closes the hatch.", ch,
                                    NULL, argument, ToRoom);
                                snprintf(buf, MSL, "The hatch on %s closes.",
                                         ship->name);
                                echo_to_room(AtYellow,
                                             get_room_index(ship->location),
                                             buf);
                                sound_to_room(get_room_index(ship->entrance),
                                              "door");
                                sound_to_room(get_room_index(ship->location),
                                              "door");
                                return;
                        }
                        else
                        {
                                send_to_char("&RIt's already closed.\n\r",
                                             ch);
                                return;
                        }
                }
        }

        ship = ship_in_room(ch->in_room, argument);
        if (!ship)
        {
                act(AtPlain, "I see no $T here.", ch, NULL, argument,
                    ToChar);
                return;
        }

        if (ship->shipstate != ShipDocked
            && ship->shipstate != ShipDisabled)
        {
                send_to_char("&RThat ship has already started to launch", ch);
                return;
        }
        else
        {
                if (ship->hatchopen)
                {
                        ship->hatchopen = FALSE;
                        act(AtPlain, "You close the hatch on $T.", ch, NULL,
                            ship->name, ToChar);
                        act(AtPlain, "$n closes the hatch on $T.", ch, NULL,
                            ship->name, ToRoom);
                        echo_to_room(AtYellow,
                                     get_room_index(ship->entrance),
                                     "The hatch is closed from outside.");
                        sound_to_room(get_room_index(ship->entrance), "door");
                        sound_to_room(get_room_index(ship->location), "door");

                        return;
                }
                else
                {
                        send_to_char("&RIts already closed.\n\r", ch);
                        return;
                }
        }


}

CMDF do_status(CharData * ch, char *argument)
{
        int       percent_chance = 0;
        ShipData *ship;
        ShipData *target;
        bool      all = FALSE;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit, turret or engineroom of a ship to do that!\n\r",
                         ch);
                return;
        }


        if (!str_cmp(argument, "all"))
                all = TRUE;

        if (argument[0] == '\0' || all)
                target = ship;
        else
                target = get_ship_here(argument, ship->starsystem);

        if (!str_cmp(argument, "all"))
                all = TRUE;

        if (target == NULL && !all)
        {
                send_to_char
                        ("&RI don't see that here.\n\rTry the radar, or type status by itself for your ships status.\n\r",
                         ch);
                return;
        }

        if (abs((int) target->vx - (int) ship->vx) > 500 + ship->sensor * 2 ||
            abs((int) target->vy - (int) ship->vy) > 500 + ship->sensor * 2 ||
            abs((int) target->vz - (int) ship->vz) > 500 + ship->sensor * 2)
        {
                send_to_char("&RThat ship is to far away to scan.\n\r", ch);
                return;
        }

        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_shipsystems]);
        if (number_percent() > percent_chance)
        {
                send_to_char
                        ("&RYou cant figure out what the readout means.\n\r",
                         ch);
                learn_from_failure(ch, gsn_shipsystems);
                return;
        }

        act(AtPlain,
            "$n checks various gages and displays on the control panel.", ch,
            NULL, argument, ToRoom);
        send_to_char
                ("&Y<>&O---------------------------------------------------------------------&Y<>\n\r",
                 ch);
        send_to_char
                ("&O||---- &YNavigation &O-----------------------------------------------------||\n\r",
                 ch);
        ch_printf(ch, "|| Status on &Y%-57s &O||\n\r", target->name);
        if (all)
        {
                ch_printf(ch,
                          "|| Current Coordinates:\t&R[&Y%+7.0f&R]\t[&Y%+7.0f&R]\t[&Y%+7.0f&R]      &O||\n\r",
                          target->vx, target->vy, target->vz);
                ch_printf(ch,
                          "|| Current Heading:\t&R[&Y%+7.0f&R]\t[&Y%+7.0f&R]\t[&Y%+7.0f&R]      &O||\n\r",
                          target->hx, target->hy, target->hz);
        }
        ch_printf(ch,
                  "|| Current Speed:\t&R[&Y%3d&O&R/&O%3d&R]                                      &O||\n\r",
                  target->currspeed, target->realspeed);
        send_to_char
                ("&O||---- &YWeapons Systems &O------------------------------------------------||\n\r",
                 ch);
        ch_printf(ch,
                  "|| Hull:&Y \t\t %+7d&O/%+7d                               &O||\n\r",
                  target->hull, target->maxhull);
        ch_printf(ch,
                  "|| Shields:\t\t&Y %+7d&O/%+7d                               &O||\n\r",
                  target->shield, target->maxshield);
        {
                ch_printf(ch,
                          "|| Energy(fuel)\t\t&Y %+7d&O/%+7d                               &O||\n\r",
                          target->energy, target->maxenergy);
                ch_printf(ch,
                          "|| Missiles:&Y\t\t %+7d&O/%+7d                               &O||\n\r",
                          ship->missiles, ship->maxmissiles);
                ch_printf(ch,
                          "|| Torpedos:\t\t &Y%+7d&O/%+7d                               &O||\n\r",
                          ship->torpedos, ship->maxtorpedos);
                ch_printf(ch,
                          "|| Rockets:\t\t &Y%+7d&O/%+7d                               &O||\n\r",
                          ship->rockets, ship->maxrockets);
                ch_printf(ch,
                          "|| Bombs:&Y\t\t %+7d&O/%+7d                               &O||\n\r",
                          ship->bombs, ship->maxbombs);
        }
        ch_printf(ch,
                  "|| Laser Condition: &R[&Y%-10s&R]                                      &O||\n\r",
                  target->statet0 ==
                  LaserDamaged ? "Damaged" : "Operational");
        ch_printf(ch,
                  "|| Current Target:  &R[&Y%-11.11s&R]                                      &O||\n\r",
                  target->target0 ? target->target0->name : "None");
        send_to_char
                ("&O||---- &YStatistics &O-----------------------------------------------------||\n\r",
                 ch);
        if (all)
        {
                ch_printf(ch,
                          "|| Battalions:\t\t &Y%+7d&O/%+7d                               &O||\n\r",
                          ship->battalions, ship->maxbattalions);
                ch_printf(ch,
                          "|| Cargo:\t\t &Y%+7d&O/%+7d                               &O||\n\r",
                          ship->cargo, ship->maxcargo);
                ch_printf(ch,
                          "|| Current Manuever:\t&R[&Y%+7d&R]                                      &O||\n\r",
                          target->manuever);
                ch_printf(ch,
                          "|| Sensor Range:\t&R[&Y%+7d&R]                                      &O||\n\r",
                          URange(1500, ship->sensor * 50 + 500, 10000));
                ch_printf(ch,
                          "|| Cargo Type:      &R[&Y%-11.11s&R]                                      &O||\n\r",
                          capitalize(cargo_names[ship->cargotype]));
        }
        ch_printf(ch,
                  "|| Ship Condition:&Y  &R[&Y%-11s&R]                                      &O||\n\r",
                  target->shipstate ==
                  ShipDisabled ? "Disabled" : "Running");
        if (all)
        {
                if (ship->cloak > 0)
                        ch_printf(ch,
                                  "|| Cloak:           &R[&Y%-11s&R]                                      &O||\n\r",
                                  IsSet(ship->flags,
                                         ShipCloak) ? "On" : "Off");
                if (ship->stealth > 0)
                        ch_printf(ch,
                                  "|| Stealth:         &R[&Y%-11s&R]                                      &O||\n\r",
                                  IsSet(ship->flags,
                                         ShipStealth) ? "On" : "Off");
                if (ship->interdictor > 0)
                        ch_printf(ch,
                                  "|| Interdictor:     &R[&Y%-11s&R]                                      &O||\n\r",
                                  IsSet(ship->flags,
                                         ShipInterdictor) ? "On" : "Off");
        }
        send_to_char
                ("&O||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_char
                ("&Y<>&O---------------------------------------------------------------------&Y<>\n\r",
                 ch);
        learn_from_success(ch, gsn_shipsystems);
}

CMDF do_hyperspace(CharData * ch, char *argument)
{
        int       percent_chance = 0, distance;
        ShipData *target;
        ShipData *ship;
        ShipData *eShip;
        char      buf[MaxStringLength];

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }


        if (ship->pilotseat != ch->in_room->vnum)
        {
                send_to_char("&RYou aren't in the pilots seat.\n\r", ch);
                return;
        }

        if (ship->dockedto)
        {
                send_to_char("You are currently docked\n\r", ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first.\n\r",
                         ch);
                return;
        }


        if (ship->ship_class == ShipPlatform)
        {
                send_to_char("&RPlatforms can't move!\n\r", ch);
                return;
        }
        if (ship->hyperspeed == 0)
        {
                send_to_char
                        ("&RThis ship is not equipped with a hyperdrive!\n\r",
                         ch);
                return;
        }
        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou are already travelling lightspeed!\n\r",
                             ch);
                return;
        }
        if (ship->shipstate == ShipDisabled)
        {
                send_to_char
                        ("&RThe ships drive is disabled. Unable to manuever.\n\r",
                         ch);
                return;
        }
        if (ship->shipstate == ShipDocked)
        {
                send_to_char
                        ("&RYou can't do that until after you've launched!\n\r",
                         ch);
                return;
        }
        if (ship->shipstate != ShipReady)
        {
                send_to_char
                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                         ch);
                return;
        }
        if (!ship->currjump)
        {
                send_to_char("&RYou need to calculate your jump first!\n\r",
                             ch);
                return;
        }
        for (target = ship->starsystem->first_ship; target;
             target = target->next_in_starsystem)
        {
                if (IsSet(target->flags, ShipInterdictor))
                {
                        distance = distance_ship_ship(ship, target);
                        if (distance < 10000)
                        {
                                send_to_char
                                        ("&RYou cannot jump to Hyperspace!\n\r",
                                         ch);
                                ch_printf(ch,
                                          "The starsystem is interdicted by %s!\n\r",
                                          target->name);
                                return;
                        }
                }
        }





        if (ship->energy <
            (200 + ship->hyperdistance * (1 + ship->ship_class) / 3))
        {
                send_to_char("&RTheres not enough fuel!\n\r", ch);
                return;
        }

        if (ship->currspeed <= 0)
        {
                send_to_char("&RYou need to Speed up a little first!\n\r",
                             ch);
                return;
        }

        for (eShip = ship->starsystem->first_ship; eShip;
             eShip = eShip->next_in_starsystem)
        {
                if (eShip == ship)
                        continue;

                if (abs((int) eShip->vx - (int) ship->vx) < 500
                    && abs((int) eShip->vy - (int) ship->vy) < 500
                    && abs((int) eShip->vz - (int) ship->vz) < 500)
                {
                        if (!IsSet(eShip->flags, ShipCloak))
                        {
                                ch_printf(ch,
                                          "&RYou are too close to %s to make the jump to lightspeed.\n\r",
                                          eShip->name);
                                return;
                        }
                        if (IsSet(eShip->flags, ShipCloak))
                        {
                                ch_printf(ch,
                                          "&RYou are too close to an unknown stellar mass to make the jump to lightspeed.\n\r",
                                          eShip->name);
                                return;
                        }
                }
        }

        if (ship->ship_class == FighterShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_starfighters]);
        if (ship->ship_class == MidsizeShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_midships]);
        if (ship->ship_class == CapitalShip)
                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_capitalships]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYou can't figure out which lever to use.\n\r",
                             ch);
                if (ship->ship_class == FighterShip)
                        learn_from_failure(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_failure(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_failure(ch, gsn_capitalships);
                return;
        }

        if (!IsSet(ship->flags, ShipCloak)
            && !IsSet(ship->flags, ShipStealth))
        {
                snprintf(buf, MSL, "%s disapears from your scanner.",
                         ship->name);
                echo_to_system(AtYellow, ship, buf, NULL);
        }
        if (IsSet(ship->flags, ShipStealth))
        {
                snprintf(buf, MSL,
                         "You notice a flash out your viewport as a ship disapears into hyperspace.");
                echo_to_system(AtYellow, ship, buf, NULL);
        }

        ship_from_starsystem(ship, ship->starsystem);
        ship->shipstate = ShipHyperspace;

        send_to_char("&GYou push forward the hyperspeed lever.\n\r", ch);
        act(AtPlain, "$n pushes a lever forward on the control panel.", ch,
            NULL, argument, ToRoom);
        echo_to_ship(AtYellow, ship,
                     "The ship lurches slightly as it makes the jump to lightspeed.");
        echo_to_cockpit(AtYellow, ship,
                        "The stars become streaks of light as you enter hyperspace.");

        ship->energy -=
                (100 + ship->hyperdistance * (1 + ship->ship_class) / 3);

        ship->vx = ship->jx;
        ship->vy = ship->jy;
        ship->vz = ship->jz;

        if (ship->ship_class == FighterShip)
                learn_from_success(ch, gsn_starfighters);
        if (ship->ship_class == MidsizeShip)
                learn_from_success(ch, gsn_midships);
        if (ship->ship_class == CapitalShip)
                learn_from_success(ch, gsn_capitalships);

}


CMDF do_target(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent_chance = 0;
        ShipData *ship;
        ShipData *target;
        char      buf[MaxStringLength];

        mudstrlcpy(arg, argument, MIL);

        if (IsSet(ch->affected_by, AffRestrained))
        {
                send_to_char
                        ("How do you expect to do that while restrained?\n\r",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                if ((ship = ship_from_turret(ch->in_room->vnum)) == NULL)
                {
                        send_to_char
                                ("&RYou must be in the gunners seat or turret of a ship to do that!\n\r",
                                 ch);
                        return;
                }

                if (ship->ship_class > ShipPlatform)
                {
                        send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                        return;
                }

                if (ship->shipstate == ShipHyperspace)
                {
                        send_to_char
                                ("&RYou can only do that in realspace!\n\r",
                                 ch);
                        return;
                }
                if (!ship->starsystem)
                {
                        send_to_char
                                ("&RYou can't do that until you've finished launching!\n\r",
                                 ch);
                        return;
                }

                if (autofly(ship))
                {
                        send_to_char
                                ("&RYou'll have to turn off the ships autopilot first....\n\r",
                                 ch);
                        return;
                }

                if (arg[0] == '\0')
                {
                        send_to_char("&RYou need to specify a target!\n\r",
                                     ch);
                        return;
                }

                if (!str_cmp(arg, "none"))
                {
                        send_to_char("&GTarget set to none.\n\r", ch);
                        if (ch->in_room->vnum == ship->gunseat)
                                ship->target0 = NULL;
                        if (ch->in_room->vnum == ship->turret1)
                                ship->target1 = NULL;
                        if (ch->in_room->vnum == ship->turret2)
                                ship->target2 = NULL;
                        return;
                }

                target = get_ship_here(arg, ship->starsystem);
                if (target == NULL)
                {
                        send_to_char("&RThat ship isn't here!\n\r", ch);
                        return;
                }

                if (target == ship)
                {
                        send_to_char("&RYou can't target your own ship!\n\r",
                                     ch);
                        return;
                }

                if (!str_cmp(target->owner, ship->owner)
                    && str_cmp(target->owner, "")
                    && !IsSet(target->flags, ShipSimulator))
                {
                        send_to_char
                                ("&RThat ship has the same owner... try targetting an enemy ship instead!\n\r",
                                 ch);
                        return;
                }

                if (abs((int) ship->vx - (int) target->vx) > 5000 ||
                    abs((int) ship->vy - (int) target->vy) > 5000 ||
                    abs((int) ship->vz - (int) target->vz) > 5000)
                {
                        send_to_char
                                ("&RThat ship is too far away to target.\n\r",
                                 ch);
                        return;
                }

                if (IsSet(target->flags, ShipCloak))
                {
                        send_to_char("&RThat ship isn't here!\n\r", ch);
                        return;
                }

                if (IsSet(ship->flags, ShipCloak))
                {
                        send_to_char
                                ("&RYou cannot target anything while cloaked!\n\r",
                                 ch);
                        return;
                }

                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_weaponsystems]);
                if (number_percent() < percent_chance)
                {
                        send_to_char("&GTracking target.\n\r", ch);
                        act(AtPlain,
                            "$n makes some adjustments on the targeting computer.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 1, do_target, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou fail to work the controls properly.\n\r",
                             ch);
                learn_from_failure(ch, gsn_weaponsystems);
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
                if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
                        return;
                send_to_char
                        ("&RYour concentration is broken. You fail to lock onto your target.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;

        if ((ship = ship_from_turret(ch->in_room->vnum)) == NULL)
        {
                return;
        }

        target = get_ship_here(arg, ship->starsystem);
        if (target == NULL || target == ship)
        {
                send_to_char
                        ("&RThe ship has left the starsytem. Targeting aborted.\n\r",
                         ch);
                return;
        }

        if (ch->in_room->vnum == ship->gunseat)
                ship->target0 = target;

        if (ch->in_room->vnum == ship->turret1)
                ship->target1 = target;

        if (ch->in_room->vnum == ship->turret2)
                ship->target2 = target;

        send_to_char("&GTarget Locked.\n\r", ch);
        snprintf(buf, MSL, "You are being targetted by %s.", ship->name);
        echo_to_cockpit(AtBlood, target, buf);

        sound_to_room(ch->in_room, "targetlock");
        learn_from_success(ch, gsn_weaponsystems);

        if (autofly(target) && !target->target0)
        {
                snprintf(buf, MSL, "You are being targetted by %s.",
                         target->name);
                echo_to_cockpit(AtBlood, ship, buf);
                target->target0 = ship;
        }
}

CMDF do_fire(CharData * ch, char *argument)
{
        int       percent_chance, shots;
        ShipData *ship;
        ShipData *target;
        char      buf[MaxStringLength];

        if (IsSet(ch->affected_by, AffRestrained))
        {
                send_to_char
                        ("How do you expect to do that while restrained?\n\r",
                         ch);
                return;
        }

        if ((ship = ship_from_turret(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the gunners chair or turret of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->dockedto)
        {
                send_to_char("You are currently docked\n\r", ch);
                return;

        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }
        if (ship->starsystem == NULL)
        {
                send_to_char
                        ("&RYou can't do that until after you've finished launching!\n\r",
                         ch);
                return;
        }
        if (ship->energy < 5)
        {
                send_to_char("&RTheres not enough energy left to fire!\n\r",
                             ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first.\n\r",
                         ch);
                return;
        }


        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->perm_dex * 2 +
                         ch->pcdata->learned[gsn_spacecombat] / 3 +
                         ch->pcdata->learned[gsn_spacecombat2] / 3 +
                         ch->pcdata->learned[gsn_spacecombat3] / 3);

        if (ch->in_room->vnum == ship->gunseat
            && !str_prefix(argument, "lasers"))
        {

                if (ship->statet0 == LaserDamaged)
                {
                        send_to_char("&RThe ships main laser is damaged.\n\r",
                                     ch);
                        return;
                }
                if (ship->statet0 >= ship->lasers)
                {
                        send_to_char("&RThe lasers are still recharging.\n\r",
                                     ch);
                        return;
                }
                if (ship->target0 == NULL)
                {
                        send_to_char
                                ("&RYou need to choose a target first.\n\r",
                                 ch);
                        return;
                }
                target = ship->target0;
                if (ship->target0->starsystem != ship->starsystem)
                {
                        send_to_char("&RYour target seems to have left.\n\r",
                                     ch);
                        ship->target0 = NULL;
                        return;
                }
                if (abs((int) (target->vx - ship->vx)) > 1000 ||
                    abs((int) (target->vy - ship->vy)) > 1000 ||
                    abs((int) (target->vz - ship->vz)) > 1000)
                {
                        send_to_char("&RThat ship is out of laser range.\n\r",
                                     ch);
                        return;
                }
                if (ship->ship_class < 2 && !is_facing(ship, target))
                {
                        send_to_char
                                ("&RThe main laser can only fire forward. You'll need to turn your ship!\n\r",
                                 ch);
                        return;
                }
                ship->statet0++;
                percent_chance += target->ship_class * 25;
                percent_chance -= target->manuever / 10;
                percent_chance -= target->currspeed / 20;
                percent_chance -= (abs((int) (target->vx - ship->vx)) / 70);
                percent_chance -= (abs((int) (target->vy - ship->vy)) / 70);
                percent_chance -= (abs((int) (target->vz - ship->vz)) / 70);
                percent_chance -= target->evasive / 10;
                percent_chance = URange(10, percent_chance, 90);
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                if (number_percent() > percent_chance)
                {
                        snprintf(buf, MSL,
                                 "Lasers fire from %s at you but miss.",
                                 ship->name);
                        echo_to_cockpit(AtOrange, target, buf);
                        snprintf(buf, MSL,
                                 "The ships lasers fire at %s but miss.",
                                 target->name);
                        echo_to_cockpit(AtOrange, ship, buf);
                        learn_from_failure(ch, gsn_spacecombat);
                        learn_from_failure(ch, gsn_spacecombat2);
                        learn_from_failure(ch, gsn_spacecombat3);
                        snprintf(buf, MSL,
                                 "Laserfire from %s barely misses %s.",
                                 ship->name, target->name);
                        echo_to_system(AtOrange, ship, buf, target);
                        return;
                }
                snprintf(buf, MSL, "Laserfire from %s hits %s.", ship->name,
                         target->name);
                echo_to_system(AtOrange, ship, buf, target);
                snprintf(buf, MSL, "You are hit by lasers from %s!",
                         ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                snprintf(buf, MSL, "Your ships lasers hit %s!.",
                         target->name);
                echo_to_cockpit(AtYellow, ship, buf);
                learn_from_success(ch, gsn_spacecombat);
                learn_from_success(ch, gsn_spacecombat2);
                learn_from_success(ch, gsn_spacecombat3);
                echo_to_ship(AtRed, target,
                             "A small explosion vibrates through the ship.");

                for (shots = 0; shots <= ship->lasers; shots++)
                {
                        if (!ship->target0)
                                break;
                        ship->statet0 = ship->lasers;
                        damage_ship(target, 5, 10);
                }


                if (autofly(target) && target->target0 != ship)
                {
                        target->target0 = ship;
                        snprintf(buf, MSL, "You are being targetted by %s.",
                                 target->name);
                        echo_to_cockpit(AtBlood, ship, buf);
                }

                return;
        }

        if (ch->in_room->vnum == ship->gunseat
            && (!str_prefix(argument, "ion")
                || !str_prefix(argument, "ions")))
        {

                if (ship->statet0i == LaserDamaged)
                {
                        send_to_char("&RThe ships ion cannon is damaged.\n\r",
                                     ch);
                        return;
                }
                if (ship->statet0i >= ship->ions && ship->ions > 0)
                {
                        send_to_char
                                ("&RThe ion cannons are still recharging.\n\r",
                                 ch);
                        return;
                }
                if (ship->ions <= 0)
                {
                        send_to_char
                                ("&RThis ship has no ion cannons equipped!\n\r",
                                 ch);
                        return;
                }
                if (ship->target0 == NULL)
                {
                        send_to_char
                                ("&RYou need to choose a target first.\n\r",
                                 ch);
                        return;
                }
                target = ship->target0;
                if (ship->target0->starsystem != ship->starsystem)
                {
                        send_to_char("&RYour target seems to have left.\n\r",
                                     ch);
                        ship->target0 = NULL;
                        return;
                }
                if (abs((int) (target->vx - ship->vx)) > 1000 ||
                    abs((int) (target->vy - ship->vy)) > 1000 ||
                    abs((int) (target->vz - ship->vz)) > 1000)
                {
                        send_to_char("&RThat ship is out of ion range.\n\r",
                                     ch);
                        return;
                }
                if (ship->ship_class < 2 && !is_facing(ship, target))
                {
                        send_to_char
                                ("&RThe ion cannon can only fire forward. You'll need to turn your ship!\n\r",
                                 ch);
                        return;
                }
                ship->statet0i++;
                percent_chance += target->ship_class * 15;
                percent_chance -= target->manuever / 2;
                percent_chance -= target->currspeed / 4;
                percent_chance -= (abs((int) (target->vx - ship->vx)) / 70);
                percent_chance -= (abs((int) (target->vy - ship->vy)) / 70);
                percent_chance -= (abs((int) (target->vz - ship->vz)) / 70);
                percent_chance -= target->evasive / 10;
                percent_chance = URange(1, (percent_chance / 5), 90);
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                if (number_percent() > percent_chance)
                {
                        snprintf(buf, MSL,
                                 "&CIon cannons fire from %s at you but miss.",
                                 ship->name);
                        echo_to_cockpit(AtOrange, target, buf);
                        snprintf(buf, MSL,
                                 "&CThe ships ion cannons fire at %s but miss.",
                                 target->name);
                        echo_to_cockpit(AtOrange, ship, buf);
                        learn_from_failure(ch, gsn_spacecombat);
                        learn_from_failure(ch, gsn_spacecombat2);
                        learn_from_failure(ch, gsn_spacecombat3);
                        snprintf(buf, MSL,
                                 "&CIonfire from %s barely misses %s.",
                                 ship->name, target->name);
                        echo_to_system(AtOrange, ship, buf, target);
                        return;
                }
                snprintf(buf, MSL, "&CIonfire from %s hits %s.", ship->name,
                         target->name);
                echo_to_system(AtOrange, ship, buf, target);
                snprintf(buf, MSL, "&CYou are hit by ion blasts from %s!",
                         ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                snprintf(buf, MSL, "&CYour ships ion cannons hit %s!.",
                         target->name);
                echo_to_cockpit(AtYellow, ship, buf);
                learn_from_success(ch, gsn_spacecombat);
                learn_from_success(ch, gsn_spacecombat2);
                learn_from_success(ch, gsn_spacecombat3);
                echo_to_ship(AtRed, target,
                             "&CThe lights flicker and wires crackle as the ship is hit by an Ion blast.");
                damage_ship_ch_ion(target, 10, 15, ch);

                if (autofly(target) && target->target0 != ship)
                {
                        target->target0 = ship;
                        snprintf(buf, MSL, "You are being targetted by %s.",
                                 target->name);
                        echo_to_cockpit(AtBlood, ship, buf);
                }

                return;
        }

        if (ch->in_room->vnum == ship->gunseat
            && !str_prefix(argument, "missile"))
        {
                if (ship->missilestate == MissileDamaged)
                {
                        send_to_char
                                ("&RThe ships missile launchers are dammaged.\n\r",
                                 ch);
                        return;
                }
                if (ship->missiles <= 0)
                {
                        send_to_char("&RYou have no missiles to fire!\n\r",
                                     ch);
                        return;
                }
                if (ship->missilestate != MissileReady)
                {
                        send_to_char
                                ("&RThe missiles are still reloading.\n\r",
                                 ch);
                        return;
                }
                if (ship->target0 == NULL)
                {
                        send_to_char
                                ("&RYou need to choose a target first.\n\r",
                                 ch);
                        return;
                }
                target = ship->target0;
                if (ship->target0->starsystem != ship->starsystem)
                {
                        send_to_char("&RYour target seems to have left.\n\r",
                                     ch);
                        ship->target0 = NULL;
                        return;
                }
                if (abs((int) (target->vx - ship->vx)) > 1000 ||
                    abs((int) (target->vy - ship->vy)) > 1000 ||
                    abs((int) (target->vz - ship->vz)) > 1000)
                {
                        send_to_char
                                ("&RThat ship is out of missile range.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class < 2 && !is_facing(ship, target))
                {
                        send_to_char
                                ("&RMissiles can only fire in a forward. You'll need to turn your ship!\n\r",
                                 ch);
                        return;
                }
                percent_chance -= target->manuever / 5;
                percent_chance -= target->currspeed / 20;
                percent_chance +=
                        target->ship_class * target->ship_class * 25;
                percent_chance -= (abs((int) (target->vx - ship->vx)) / 70);
                percent_chance -= (abs((int) (target->vy - ship->vy)) / 70);
                percent_chance -= (abs((int) (target->vz - ship->vz)) / 70);

                percent_chance -= target->evasive / 10;
                percent_chance += (30);
                percent_chance = URange(20, percent_chance, 80);
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                if (number_percent() > percent_chance)
                {
                        send_to_char("&RYou fail to lock onto your target!",
                                     ch);
                        ship->missilestate = MissileReload2;
                        return;
                }
                new_missile(ship, target, ch, ConcussionMissile);
                ship->missiles--;
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                echo_to_cockpit(AtYellow, ship, "Missiles launched.");
                snprintf(buf, MSL, "Incoming missile from %s.", ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                snprintf(buf, MSL, "%s fires a missile towards %s.",
                         ship->name, target->name);
                echo_to_system(AtOrange, ship, buf, target);
                learn_from_success(ch, gsn_weaponsystems);
                if (ship->ship_class == CapitalShip
                    || ship->ship_class == ShipPlatform)
                        ship->missilestate = MissileReload;
                else
                        ship->missilestate = MissileFired;

                if (autofly(target) && target->target0 != ship)
                {
                        target->target0 = ship;
                        snprintf(buf, MSL, "You are being targetted by %s.",
                                 target->name);
                        echo_to_cockpit(AtBlood, ship, buf);
                }

                return;
        }
        if (ch->in_room->vnum == ship->gunseat
            && !str_prefix(argument, "torpedo"))
        {
                if (ship->missilestate == MissileDamaged)
                {
                        send_to_char
                                ("&RThe ships missile launchers are dammaged.\n\r",
                                 ch);
                        return;
                }
                if (ship->torpedos <= 0)
                {
                        send_to_char("&RYou have no torpedos to fire!\n\r",
                                     ch);
                        return;
                }
                if (ship->missilestate != MissileReady)
                {
                        send_to_char
                                ("&RThe torpedos are still reloading.\n\r",
                                 ch);
                        return;
                }
                if (ship->target0 == NULL)
                {
                        send_to_char
                                ("&RYou need to choose a target first.\n\r",
                                 ch);
                        return;
                }
                target = ship->target0;
                if (ship->target0->starsystem != ship->starsystem)
                {
                        send_to_char("&RYour target seems to have left.\n\r",
                                     ch);
                        ship->target0 = NULL;
                        return;
                }
                if (abs((int) (target->vx - ship->vx)) > 1000 ||
                    abs((int) (target->vy - ship->vy)) > 1000 ||
                    abs((int) (target->vz - ship->vz)) > 1000)
                {
                        send_to_char
                                ("&RThat ship is out of torpedo range.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class < 2 && !is_facing(ship, target))
                {
                        send_to_char
                                ("&RTorpedos can only fire in a forward direction. You'll need to turn your ship!\n\r",
                                 ch);
                        return;
                }
                percent_chance -= target->manuever / 5;
                percent_chance -= target->currspeed / 20;
                percent_chance +=
                        target->ship_class * target->ship_class * 25;
                percent_chance -= (abs((int) (target->vx - ship->vx)) / 70);
                percent_chance -= (abs((int) (target->vy - ship->vy)) / 70);
                percent_chance -= (abs((int) (target->vz - ship->vz)) / 70);
                percent_chance -= target->evasive / 10;
                percent_chance = URange(20, percent_chance, 80);
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                if (number_percent() > percent_chance)
                {
                        send_to_char("&RYou fail to lock onto your target!",
                                     ch);
                        ship->missilestate = MissileReload2;
                        return;
                }
                new_missile(ship, target, ch, ProtonTorpedo);
                ship->torpedos--;
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                echo_to_cockpit(AtYellow, ship, "Missiles launched.");
                snprintf(buf, MSL, "Incoming torpedo from %s.", ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                snprintf(buf, MSL, "%s fires a torpedo towards %s.",
                         ship->name, target->name);
                echo_to_system(AtOrange, ship, buf, target);
                learn_from_success(ch, gsn_weaponsystems);
                if (ship->ship_class == CapitalShip
                    || ship->ship_class == ShipPlatform)
                        ship->missilestate = MissileReload;
                else
                        ship->missilestate = MissileFired;

                if (autofly(target) && target->target0 != ship)
                {
                        target->target0 = ship;
                        snprintf(buf, MSL, "You are being targetted by %s.",
                                 target->name);
                        echo_to_cockpit(AtBlood, ship, buf);
                }

                return;
        }

        if (ch->in_room->vnum == ship->gunseat
            && !str_prefix(argument, "rocket"))
        {
                if (ship->missilestate == MissileDamaged)
                {
                        send_to_char
                                ("&RThe ships missile launchers are damaged.\n\r",
                                 ch);
                        return;
                }
                if (ship->rockets <= 0)
                {
                        send_to_char("&RYou have no rockets to fire!\n\r",
                                     ch);
                        return;
                }
                if (ship->missilestate != MissileReady)
                {
                        send_to_char
                                ("&RThe missiles are still reloading.\n\r",
                                 ch);
                        return;
                }
                if (ship->target0 == NULL)
                {
                        send_to_char
                                ("&RYou need to choose a target first.\n\r",
                                 ch);
                        return;
                }
                target = ship->target0;
                if (ship->target0->starsystem != ship->starsystem)
                {
                        send_to_char("&RYour target seems to have left.\n\r",
                                     ch);
                        ship->target0 = NULL;
                        return;
                }
                if (abs((int) (target->vx - ship->vx)) > 800 ||
                    abs((int) (target->vy - ship->vy)) > 800 ||
                    abs((int) (target->vz - ship->vz)) > 800)
                {
                        send_to_char
                                ("&RThat ship is out of rocket range.\n\r",
                                 ch);
                        return;
                }
                if (ship->ship_class < 2 && !is_facing(ship, target))
                {
                        send_to_char
                                ("&RRockets can only fire forward. You'll need to turn your ship!\n\r",
                                 ch);
                        return;
                }
                percent_chance -= target->manuever / 5;
                percent_chance -= target->currspeed / 20;
                percent_chance +=
                        target->ship_class * target->ship_class * 25;
                percent_chance -= (abs((int) (target->vx - ship->vx)) / 100);
                percent_chance -= (abs((int) (target->vy - ship->vy)) / 100);
                percent_chance -= (abs((int) (target->vz - ship->vz)) / 100);
                percent_chance -= target->evasive / 10;
                percent_chance -= 30;
                percent_chance = URange(20, percent_chance, 80);
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                if (number_percent() > percent_chance)
                {
                        send_to_char("&RYou fail to lock onto your target!",
                                     ch);
                        ship->missilestate = MissileReload2;
                        return;
                }
                new_missile(ship, target, ch, HeavyRocket);
                ship->rockets--;
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                echo_to_cockpit(AtYellow, ship, "Rocket launched.");
                snprintf(buf, MSL, "Incoming rocket from %s.", ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                snprintf(buf, MSL, "%s fires a heavy rocket towards %s.",
                         ship->name, target->name);
                echo_to_system(AtOrange, ship, buf, target);
                learn_from_success(ch, gsn_weaponsystems);
                if (ship->ship_class == CapitalShip
                    || ship->ship_class == ShipPlatform)
                        ship->missilestate = MissileReload;
                else
                        ship->missilestate = MissileFired;

                if (autofly(target) && target->target0 != ship)
                {
                        target->target0 = ship;
                        snprintf(buf, MSL, "You are being targetted by %s.",
                                 target->name);
                        echo_to_cockpit(AtBlood, ship, buf);
                }

                return;
        }

        if (ch->in_room->vnum == ship->turret1
            && !str_prefix(argument, "lasers"))
        {
                if (ship->statet1 == LaserDamaged)
                {
                        send_to_char("&RThe ships turret is damaged.\n\r",
                                     ch);
                        return;
                }
                if (ship->statet1 > ship->ship_class)
                {
                        send_to_char("&RThe turbolaser is recharging.\n\r",
                                     ch);
                        return;
                }
                if (ship->target1 == NULL)
                {
                        send_to_char
                                ("&RYou need to choose a target first.\n\r",
                                 ch);
                        return;
                }
                target = ship->target1;
                if (ship->target1->starsystem != ship->starsystem)
                {
                        send_to_char("&RYour target seems to have left.\n\r",
                                     ch);
                        ship->target1 = NULL;
                        return;
                }
                if (abs((int) (target->vx - ship->vx)) > 1000 ||
                    abs((int) (target->vy - ship->vy)) > 1000 ||
                    abs((int) (target->vz - ship->vz)) > 1000)
                {
                        send_to_char("&RThat ship is out of laser range.\n\r",
                                     ch);
                        return;
                }
                ship->statet1++;
                percent_chance -= target->manuever / 10;
                percent_chance += target->ship_class * 25;
                percent_chance -= target->currspeed / 20;
                percent_chance -= (abs((int) (target->vx - ship->vx)) / 70);
                percent_chance -= (abs((int) (target->vy - ship->vy)) / 70);
                percent_chance -= (abs((int) (target->vz - ship->vz)) / 70);
                percent_chance -= target->evasive / 10;
                percent_chance = URange(10, percent_chance, 90);
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                if (number_percent() > percent_chance)
                {
                        snprintf(buf, MSL,
                                 "Turbolasers fire from %s at you but miss.",
                                 ship->name);
                        echo_to_cockpit(AtOrange, target, buf);
                        snprintf(buf, MSL,
                                 "Turbolasers fire from the ships turret at %s but miss.",
                                 target->name);
                        echo_to_cockpit(AtOrange, ship, buf);
                        snprintf(buf, MSL, "%s fires at %s but misses.",
                                 ship->name, target->name);
                        echo_to_system(AtOrange, ship, buf, target);
                        learn_from_failure(ch, gsn_spacecombat);
                        learn_from_failure(ch, gsn_spacecombat2);
                        learn_from_failure(ch, gsn_spacecombat3);
                        return;
                }
                snprintf(buf, MSL, "Turboasers fire from %s, hitting %s.",
                         ship->name, target->name);
                echo_to_system(AtOrange, ship, buf, target);
                snprintf(buf, MSL, "You are hit by turbolasers from %s!",
                         ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                snprintf(buf, MSL,
                         "Turbolasers fire from the turret, hitting %s!.",
                         target->name);
                echo_to_cockpit(AtYellow, ship, buf);
                learn_from_success(ch, gsn_spacecombat);
                learn_from_success(ch, gsn_spacecombat2);
                learn_from_success(ch, gsn_spacecombat3);
                echo_to_ship(AtRed, target,
                             "A small explosion vibrates through the ship.");
                damage_ship_ch(target, 10, 25, ch);

                if (autofly(target) && target->target0 != ship)
                {
                        target->target0 = ship;
                        snprintf(buf, MSL, "You are being targetted by %s.",
                                 target->name);
                        echo_to_cockpit(AtBlood, ship, buf);
                }

                return;
        }

        if (ch->in_room->vnum == ship->turret2
            && !str_prefix(argument, "lasers"))
        {
                if (ship->statet2 == LaserDamaged)
                {
                        send_to_char("&RThe ships turret is damaged.\n\r",
                                     ch);
                        return;
                }
                if (ship->statet2 > ship->ship_class)
                {
                        send_to_char
                                ("&RThe turbolaser is still recharging.\n\r",
                                 ch);
                        return;
                }
                if (ship->target2 == NULL)
                {
                        send_to_char
                                ("&RYou need to choose a target first.\n\r",
                                 ch);
                        return;
                }
                target = ship->target2;
                if (ship->target2->starsystem != ship->starsystem)
                {
                        send_to_char("&RYour target seems to have left.\n\r",
                                     ch);
                        ship->target2 = NULL;
                        return;
                }
                if (abs((int) (target->vx - ship->vx)) > 1000 ||
                    abs((int) (target->vy - ship->vy)) > 1000 ||
                    abs((int) (target->vz - ship->vz)) > 1000)
                {
                        send_to_char("&RThat ship is out of laser range.\n\r",
                                     ch);
                        return;
                }
                ship->statet2++;
                percent_chance -= target->manuever / 10;
                percent_chance += target->ship_class * 25;
                percent_chance -= target->currspeed / 20;
                percent_chance -= (abs((int) (target->vx - ship->vx)) / 70);
                percent_chance -= (abs((int) (target->vy - ship->vy)) / 70);
                percent_chance -= (abs((int) (target->vz - ship->vz)) / 70);
                percent_chance -= target->evasive / 10;
                percent_chance = URange(10, percent_chance, 90);
                act(AtPlain, "$n presses the fire button.", ch,
                    NULL, argument, ToRoom);
                if (number_percent() > percent_chance)
                {
                        snprintf(buf, MSL,
                                 "Turbolasers fire from %s barely missing %s.",
                                 ship->name, target->name);
                        echo_to_system(AtOrange, ship, buf, target);
                        snprintf(buf, MSL,
                                 "Turbolasers fire from %s at you but miss.",
                                 ship->name);
                        echo_to_cockpit(AtOrange, target, buf);
                        snprintf(buf, MSL,
                                 "Turbolasers fire from the turret missing %s.",
                                 target->name);
                        echo_to_cockpit(AtOrange, ship, buf);
                        learn_from_failure(ch, gsn_spacecombat);
                        learn_from_failure(ch, gsn_spacecombat2);
                        learn_from_failure(ch, gsn_spacecombat3);
                        return;
                }
                snprintf(buf, MSL, "Turbolasers fire from %s, hitting %s.",
                         ship->name, target->name);
                echo_to_system(AtOrange, ship, buf, target);
                snprintf(buf, MSL, "You are hit by turbolasers from %s!",
                         ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                snprintf(buf, MSL,
                         "turbolasers fire from the turret hitting %s!.",
                         target->name);
                echo_to_cockpit(AtYellow, ship, buf);
                learn_from_success(ch, gsn_spacecombat);
                learn_from_success(ch, gsn_spacecombat2);
                learn_from_success(ch, gsn_spacecombat3);
                echo_to_ship(AtRed, target,
                             "A small explosion vibrates through the ship.");
                damage_ship_ch(target, 10, 25, ch);

                if (autofly(target) && target->target0 != ship)
                {
                        target->target0 = ship;
                        snprintf(buf, MSL, "You are being targetted by %s.",
                                 target->name);
                        echo_to_cockpit(AtBlood, ship, buf);
                }

                return;
        }

        send_to_char("&RYou can't fire that!\n\r", ch);

}

CMDF do_calculate(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        int       percent_chance, count = 0;
        ShipData *ship;
        SpaceData *starsystem;
        BodyData *body = NULL;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);


        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if ((ship = ship_from_navseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be at a nav computer to calculate jumps.\n\r",
                         ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first....\n\r",
                         ch);
                return;
        }

        if (ship->ship_class == ShipPlatform)
        {
                send_to_char
                        ("&RAnd what exactly are you going to calculate...?\n\r",
                         ch);
                return;
        }
        if (ship->hyperspeed == 0)
        {
                send_to_char
                        ("&RThis ship is not equipped with a hyperdrive!\n\r",
                         ch);
                return;
        }
        if (ship->shipstate == ShipDocked)
        {
                send_to_char
                        ("&RYou can't do that until after you've launched!\n\r",
                         ch);
                return;
        }
        if (ship->starsystem == NULL)
        {
                send_to_char("&RYou can only do that in realspace.\n\r", ch);
                return;
        }
        if (argument[0] == '\0')
        {
                send_to_char
                        ("&WFormat: Calculate <starsystem> <entry x> <entry y> <entry z>\n\r&wPossible destinations:\n\r",
                         ch);
                for (starsystem = first_starsystem; starsystem;
                     starsystem = starsystem->next)
                {
                        set_char_color(AtNote, ch);
                        if (IsSet(ship->flags, ShipSimulator))
                        {
                                if (!str_cmp(starsystem->name, "Simulator"))
                                {
                                        ch_printf(ch, "%-30s %d\n\r",
                                                  starsystem->name,
                                                  (abs
                                                   (starsystem->xpos -
                                                    ship->starsystem->xpos) +
                                                   abs(starsystem->ypos -
                                                       ship->starsystem->
                                                       ypos)) / 2);
                                        count++;
                                }
                        }
                        else
                        {
                                if (str_cmp(starsystem->name, "Simulator"))
                                {
                                        ch_printf(ch, "%-30s %d\n\r",
                                                  starsystem->name,
                                                  (abs
                                                   (starsystem->xpos -
                                                    ship->starsystem->xpos) +
                                                   abs(starsystem->ypos -
                                                       ship->starsystem->
                                                       ypos)) / 2);
                                        count++;
                                }
                        }
                }
                if (!count)
                {
                        send_to_char("No Starsystems found.\n\r", ch);
                }
                return;
        }
        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_navigation]);
        if (number_percent() > percent_chance)
        {
                send_to_char
                        ("&RYou cant seem to figure the charts out today.\n\r",
                         ch);
                learn_from_failure(ch, gsn_navigation);
                return;
        }



        ship->currjump = starsystem_from_name(arg1);
        ship->jx = atoi(arg2);
        ship->jy = atoi(arg3);
        ship->jz = atoi(argument);

        if (ship->currjump == NULL)
        {
                send_to_char
                        ("&RYou can't seem to find that starsytsem on your charts.\n\r",
                         ch);
                return;
        }
        if (IsSet(ship->flags, ShipSimulator)
            && ship->currjump != starsystem_from_name("Simulator"))
        {
                send_to_char
                        ("&RYou can't seem to find that starsytsem on your charts.\n\r",
                         ch);
                return;
        }
        else if (!IsSet(ship->flags, ShipSimulator)
                 && ship->currjump == starsystem_from_name("Simulator"))
        {
                send_to_char
                        ("&RYou can't seem to find that starsytsem on your charts.\n\r",
                         ch);
                return;
        }
        else
        {
                starsystem = ship->currjump;

                ForEachList(BodyList, starsystem->bodies, body)
                {
                        if ((body->hyperdistance(ship) < body->gravity()) &&
                            (body->type() == StarBody
                             || body->type() == PlanetBody
                             || body->type() == MoonBody)
                            && (starsystem == body->starsystem()))
                        {
                                echo_to_cockpit(AtRed, ship,
                                                "WARNING.. Jump coordinates too close to stellar object.");
                                echo_to_cockpit(AtRed, ship,
                                                "WARNING.. Hyperjump NOT set.");
                                ship->currjump = NULL;
                                return;
                        }
                }
                ship->jx += number_range(-250, 250);
                ship->jy += number_range(-250, 250);
                ship->jz += number_range(-250, 250);
        }

        ship->hyperdistance =
                abs(ship->starsystem->xpos - ship->currjump->xpos);
        ship->hyperdistance +=
                abs(ship->starsystem->ypos - ship->currjump->ypos);
        ship->hyperdistance /= 5;

        if (ship->hyperdistance < 100)
                ship->hyperdistance = 100;

        ship->hyperdistance += number_range(0, 200);


        sound_to_room(ch->in_room, "computer");

        send_to_char
                ("&GHyperspace course set. Ready for the jump to lightspeed.\n\r",
                 ch);
        act(AtPlain, "$n does some calculations using the ships computer.",
            ch, NULL, argument, ToRoom);

        learn_from_success(ch, gsn_navigation);

        WaitState(ch, 2 * PulseViolence);
}


CMDF do_recharge(CharData * ch, char *argument)
{
        int       recharge;
        int       percent_chance = 0;
        ShipData *ship;


        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }
        if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RThe controls must be at the co-pilot station.\n\r",
                         ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char("&R...\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipDisabled)
        {
                send_to_char
                        ("&RThe ships drive is disabled. Unable to manuever.\n\r",
                         ch);
                return;
        }

        if (ship->energy < 100)
        {
                send_to_char("&RTheres not enough energy!\n\r", ch);
                return;
        }

        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_shipsystems]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYou fail to work the controls properly.\n\r",
                             ch);
                learn_from_failure(ch, gsn_shipsystems);
                return;
        }

        send_to_char("&GRecharging shields..\n\r", ch);
        act(AtPlain, "$n pulls back a lever on the control panel.", ch,
            NULL, argument, ToRoom);

        learn_from_success(ch, gsn_shipsystems);

        recharge =
                UMin(ship->maxshield - ship->shield, ship->energy * 5 + 100);
        recharge = URange(1, recharge, 25 + ship->ship_class * 25);
        ship->shield += recharge;
        ship->energy -= (recharge * 2 + recharge * ship->ship_class);
}


CMDF do_repairship(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent_chance, change;
        ShipData *ship;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if ((ship = ship_from_engine(ch->in_room->vnum)) == NULL)
                {
                        send_to_char
                                ("&RYou must be in the engine room of a ship to do that!\n\r",
                                 ch);
                        return;
                }

                if (str_cmp(argument, "hull") && str_cmp(argument, "drive") &&
                    str_cmp(argument, "launcher")
                    && str_cmp(argument, "laser")
                    && str_cmp(argument, "turret 1")
                    && str_cmp(argument, "turret 2"))
                {
                        send_to_char
                                ("&RYou need to spceify something to repair:\n\r",
                                 ch);
                        send_to_char
                                ("&rTry: hull, drive, launcher, laser, turret 1, or turret 2\n\r",
                                 ch);
                        return;
                }

                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_shipmaintenance]);
                if (number_percent() < percent_chance)
                {
                        send_to_char("&GYou begin your repairs\n\r", ch);
                        act(AtPlain, "$n begins repairing the ships $T.", ch,
                            NULL, argument, ToRoom);
                        if (!str_cmp(arg, "hull"))
                                add_timer(ch, TimerDoFun, 15, do_repairship,
                                          1);
                        else
                                add_timer(ch, TimerDoFun, 5, do_repairship,
                                          1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou fail to locate the source of the problem.\n\r",
                         ch);
                learn_from_failure(ch, gsn_shipmaintenance);
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
                if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
                        return;
                send_to_char
                        ("&RYou are distracted and fail to finish your repairs.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;

        if ((ship = ship_from_engine(ch->in_room->vnum)) == NULL)
        {
                return;
        }

        if (!str_cmp(arg, "hull"))
        {
                change = URange(0,
                                number_range((int)
                                             (ch->pcdata->
                                              learned[gsn_shipmaintenance] /
                                              2),
                                             (int) (ch->pcdata->
                                                    learned
                                                    [gsn_shipmaintenance])),
                                (ship->maxhull - ship->hull));
                ship->hull += change;
                ch_printf(ch,
                          "&GRepair complete.. Hull Strength inreased by %d points.\n\r",
                          change);
        }

        if (!str_cmp(arg, "drive"))
        {
                if (ship->location == ship->lastdoc)
                        ship->shipstate = ShipDocked;
                else
                        ship->shipstate = ShipReady;
                send_to_char("&GShips drive repaired.\n\r", ch);
        }

        if (!str_cmp(arg, "launcher"))
        {
                ship->missilestate = MissileReady;
                send_to_char("&GMissile launcher repaired.\n\r", ch);
        }

        if (!str_cmp(arg, "laser"))
        {
                ship->statet0 = LaserReady;
                send_to_char("&GMain laser repaired.\n\r", ch);
        }

        if (!str_cmp(arg, "turret 1"))
        {
                ship->statet1 = LaserReady;
                send_to_char("&GLaser Turret 1 repaired.\n\r", ch);
        }

        if (!str_cmp(arg, "turret 2"))
        {
                ship->statet2 = LaserReady;
                send_to_char("&Laser Turret 2 repaired.\n\r", ch);
        }

        act(AtPlain, "$n finishes the repairs.", ch,
            NULL, argument, ToRoom);

        learn_from_success(ch, gsn_shipmaintenance);

}

CMDF do_addpilot(CharData * ch, char *argument)
{
        ShipData *ship;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class == ShipPlatform)
        {
                send_to_char("&RYou can't do that here.\n\r", ch);
                return;
        }

        if (str_cmp(ship->owner, ch->name))
        {

                if (!IsNpc(ch) && ch->pcdata && ch->pcdata->clan
                    && !str_cmp(ch->pcdata->clan->name, ship->owner))
                        if (!str_cmp(ch->pcdata->clan->leader, ch->name))
                                ;
                        else if (!str_cmp
                                 (ch->pcdata->clan->number1, ch->name))
                                ;
                        else if (!str_cmp
                                 (ch->pcdata->clan->number2, ch->name))
                                ;
                        else
                        {
                                send_to_char("&RThat isn't your ship!", ch);
                                return;
                        }
                else
                {
                        send_to_char("&RThat isn't your ship!", ch);
                        return;
                }

        }

        if (argument[0] == '\0')
        {
                send_to_char("&RAdd which pilot?\n\r", ch);
                return;
        }

        if (str_cmp(ship->pilot, ""))
        {
                if (str_cmp(ship->copilot, ""))
                {
                        send_to_char
                                ("&RYou are ready have a pilot and copilot..\n\r",
                                 ch);
                        send_to_char("&RTry rempilot first.\n\r", ch);
                        return;
                }

                STRFREE(ship->copilot);
                ship->copilot = STRALLOC(argument);
                send_to_char("Copilot Added.\n\r", ch);
                save_ship(ship);
                return;

                return;
        }

        STRFREE(ship->pilot);
        ship->pilot = STRALLOC(argument);
        send_to_char("Pilot Added.\n\r", ch);
        save_ship(ship);

}

CMDF do_rempilot(CharData * ch, char *argument)
{
        ShipData *ship;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class == ShipPlatform)
        {
                send_to_char("&RYou can't do that here.\n\r", ch);
                return;
        }

        if (str_cmp(ship->owner, ch->name))
        {

                if (!IsNpc(ch) && ch->pcdata && ch->pcdata->clan
                    && !str_cmp(ch->pcdata->clan->name, ship->owner))
                        if (!str_cmp(ch->pcdata->clan->leader, ch->name))
                                ;
                        else if (!str_cmp
                                 (ch->pcdata->clan->number1, ch->name))
                                ;
                        else if (!str_cmp
                                 (ch->pcdata->clan->number2, ch->name))
                                ;
                        else
                        {
                                send_to_char("&RThat isn't your ship!", ch);
                                return;
                        }
                else
                {
                        send_to_char("&RThat isn't your ship!", ch);
                        return;
                }

        }

        if (argument[0] == '\0')
        {
                send_to_char("&RRemove which pilot?\n\r", ch);
                return;
        }

        if (!str_cmp(ship->pilot, argument))
        {
                STRFREE(ship->pilot);
                ship->pilot = STRALLOC("");
                send_to_char("Pilot Removed.\n\r", ch);
                save_ship(ship);
                return;
        }

        if (!str_cmp(ship->copilot, argument))
        {
                STRFREE(ship->copilot);
                ship->copilot = STRALLOC("");
                send_to_char("Copilot Removed.\n\r", ch);
                save_ship(ship);
                return;
        }

        send_to_char
                ("&RThat person isn't listed as one of the ships pilots.\n\r",
                 ch);

}

CMDF do_radar(CharData * ch, char *argument)
{
        ShipData *target;
        ShipData *ship;
        MissileData *missile;
        int       percent_chance, distort, distance, plusmin = 0, chancescan,
                chancestealth, chancecloak;
        int       sensor = 0;
        bool      cansee = FALSE, cloaksee;
        BodyData *body = NULL;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit or turret of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipDocked)
        {
                send_to_char("&RWait until after you launch!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }

        if (ship->starsystem == NULL)
        {
                send_to_char
                        ("&RYou can't do that unless the ship is flying in realspace!\n\r",
                         ch);
                return;
        }

        percent_chance =
                IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_navigation]);

        if (number_percent() > percent_chance)
        {
                send_to_char("&RYou fail to work the controls properly.\n\r",
                             ch);
                learn_from_failure(ch, gsn_navigation);
                return;
        }

        sensor = URange(1500, ship->sensor * 50 + 500, 10000);
        act(AtPlain, "$n checks the radar.", ch, NULL, argument, ToRoom);

        set_char_color(AtLblue, ch);
        ch_printf(ch,
                  "----------------------------------------------------------------------------\n\r");
        ch_printf(ch, "|%s|\n\r",
                  center_str(ship->starsystem->name, 74));
        set_char_color(AtLblue, ch);
        ch_printf(ch,
                  "----------------------------------------------------------------------------\n\r");
        ch_printf(ch,
                  "|           System Names           |     Coordinates    |Distance|Direction|\n\r");
        ch_printf(ch,
                  "----------------------------------------------------------------------------\n\r");

        ForEachList(BodyList, ship->starsystem->bodies, body)
        {
                if (body->type() == PlanetBody || body->type() == StarBody
                    || body->type() == MoonBody)
                {
                        distance = body->distance(ship);
                        if (distance > sensor)
                                continue;
                        if (IsSet(ship->flags, ShipCloak))
                        {
                                distort =
                                        (number_percent() * plusmin * 3) +
                                        distance;
                                ch_printf(ch,
                                          "|%-34s| ----   ----   ---- | %-7d|         |\n\r",
                                          body->name(), distort,
                                          body->get_direction(ship));
                        }
                        else
                        {
                                ch_printf(ch,
                                          "|%-34s|%-6.6d %-6.6d %-6.6d| %-7d|%-9s|\n\r",
                                          body->name(), body->xpos(),
                                          body->ypos(), body->zpos(),
                                          distance,
                                          body->get_direction(ship));
                        }
                }
        }

        ch_printf(ch,
                  "----------------------------------------------------------------------------\n\r");
        ch_printf(ch,
                  "|           Ship Names             |     Coordinates    |Distance|Direction|\n\r");
        ch_printf(ch,
                  "----------------------------------------------------------------------------\n\r");

        for (target = ship->starsystem->first_ship; target;
             target = target->next_in_starsystem)
        {
                distance = distance_ship_ship(target, ship);
                if (distance > sensor)
                        continue;

                cansee = TRUE;
                cloaksee = TRUE;
                chancescan =
                        IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                            learned[gsn_scan1]
                                                            / 10 +
                                                            ch->pcdata->
                                                            learned[gsn_scan2]
                                                            / 5 +
                                                            ch->pcdata->
                                                            learned[gsn_scan3]
                                                            / 3);
                chancestealth = number_percent();
                chancecloak = number_percent();

                if (chancestealth < chancescan)
                        cansee = FALSE;

                if (ch->pcdata->learned[gsn_scan1] == 100 &&
                    ch->pcdata->learned[gsn_scan2] == 100 &&
                    ch->pcdata->learned[gsn_scan3] == 100)
                        if (chancecloak <= 5)
                                cloaksee = FALSE;

                if (target != ship
                    && (!IsSet(target->flags, ShipCloak) || (!cloaksee))
                    && (!IsSet(target->flags, ShipStealth) || (!cansee)))
                {
                        if (IsSet(ship->flags, ShipCloak))
                        {
                                if (number_percent() > 50)
                                {
                                        plusmin = 1;
                                }
                                else
                                {
                                        plusmin = -1;
                                }
                                distort =
                                        (number_percent() * plusmin * 3) +
                                        distance;
                                ch_printf(ch,
                                          "|%-34s| ----   ----   ---- | %-7d||%-9s|\n\r",
                                          target->name, distort,
                                          get_direction_ship(target, ship));
                        }

                        if (!IsSet(ship->flags, ShipCloak))
                        {
                                ch_printf(ch,
                                          "|%-34.34s|%-6.0f %-6.0f %-6.0f| %-7d|%-9s|\n\r",
                                          target->name, target->vx,
                                          target->vy, target->vz,
                                          distance_ship_ship(target, ship),
                                          get_direction_ship(target, ship));
                        }
                }
        }
        ch_printf(ch,
                  "----------------------------------------------------------------------------\n\r");
        ch_printf(ch, "\n\r");
        for (missile = ship->starsystem->first_missile; missile;
             missile = missile->next_in_starsystem)
        {
                distance = distance_missile_ship(missile, ship);
                if (distance > sensor)
                        continue;
                ch_printf(ch, "%s    %d %d %d\n\r",
                          missile->missiletype ==
                          ConcussionMissile ? "A Concusion missile"
                          : (missile->missiletype ==
                             ProtonTorpedo ? "A Torpedo" : (missile->
                                                             missiletype ==
                                                             HeavyRocket ?
                                                             "A Heavy Rocket"
                                                             :
                                                             "A Heavy Bomb")),
                          missile->mx, missile->my, missile->mz);
        }

        ch_printf(ch, "\n\r&WYour Coordinates: %.0f %.0f %.0f\n\r", ship->vx,
                  ship->vy, ship->vz);

        learn_from_success(ch, gsn_navigation);
        if (!cansee)
        {
                learn_from_success(ch, gsn_scan1);
                learn_from_success(ch, gsn_scan2);
                learn_from_success(ch, gsn_scan3);
        }
}

CMDF do_autotrack(CharData * ch, char *argument)
{
        ShipData *ship;
        int       percent_chance = 0;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }


        if (ship->ship_class == ShipPlatform)
        {
                send_to_char
                        ("&RPlatforms don't have autotracking systems!\n\r",
                         ch);
                return;
        }
        if (ship->ship_class == CapitalShip)
        {
                send_to_char("&RThis ship is too big for autotracking!\n\r",
                             ch);
                return;
        }

        if ((ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYou aren't in the pilots chair!\n\r", ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn off the ships autopilot first....\n\r",
                         ch);
                return;
        }

        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_shipsystems]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYour notsure which switch to flip.\n\r", ch);
                learn_from_failure(ch, gsn_shipsystems);
                return;
        }

        act(AtPlain, "$n flips a switch on the control panel.", ch,
            NULL, argument, ToRoom);
        if (ship->autotrack)
        {
                ship->autotrack = FALSE;
                echo_to_cockpit(AtYellow, ship, "Autotracking off.");
        }
        else
        {
                ship->autotrack = TRUE;
                echo_to_cockpit(AtYellow, ship, "Autotracking on.");
        }

        learn_from_success(ch, gsn_shipsystems);

}

CMDF do_closebay(CharData * ch, char *argument)
{
        ShipData *ship;
        int       percent_chance = 0;
        ShipData *target;
        char      arg[MaxStringLength];


        argument = one_argument(argument, arg);

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }


        if (!arg || arg[0] == '\0')
                target = ship_from_cockpit(ch->in_room->vnum);
        else
        {
                target = get_ship_here(arg, ship->starsystem);
                if (target == NULL)
                {
                        send_to_char("&RThat ship isn't here!\n\r", ch);
                        return;
                }
        }
        if (target->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }


        if (target->ship_class == ShipPlatform)
        {
                send_to_char("&RPlatforms don't have Bay Doors!\n\r", ch);
                return;
        }
        if (!check_pilot(ch, target))
        {
                send_to_char
                        ("&RYou do not have permission to access that ship command!\n\r",
                         ch);
                return;
        }


        /*
         * I have removed this because I don't think it is right to use
         * but put it back in if you want
         * 
         * if ( autofly(ship)  )
         * {
         * send_to_char("&RYou'll have to turn off the ships autopilot
         * first....\n\r",ch);
         * return;
         * }
         * 
         */


        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_shipsystems]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYour not sure which switch to flip.\n\r", ch);
                learn_from_failure(ch, gsn_shipsystems);
                return;
        }

        act(AtPlain, "$n flips a switch on the control panel.", ch,
            NULL, argument, ToRoom);

        if (!target->bayopen)
        {
                echo_to_cockpit(AtYellow, ship,
                                "Your bay doors are already closed.");
                return;
        }
        else
        {
                target->bayopen = FALSE;
                echo_to_cockpit(AtYellow, ship, "Closing Bay Doors.");
        }

        learn_from_success(ch, gsn_shipsystems);


}

CMDF do_openbay(CharData * ch, char *argument)
{
        ShipData *ship;
        int       percent_chance = 0;
        ShipData *target;
        char      arg[MaxStringLength];


        argument = one_argument(argument, arg);

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }


        if (!arg || arg[0] == '\0')
                target = ship_from_cockpit(ch->in_room->vnum);
        else
        {
                target = get_ship_here(arg, ship->starsystem);
                if (target == NULL)
                {
                        send_to_char("&RThat ship isn't here!\n\r", ch);
                        return;
                }
        }
        if (target->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }


        if (target->ship_class == ShipPlatform)
        {
                send_to_char("&RPlatforms don't have Bay Doors!\n\r", ch);
                return;
        }
        if (!check_pilot(ch, target))
        {
                send_to_char
                        ("&RYou do not have permission to access that ship command!\n\r",
                         ch);
                return;
        }


        /*
         * I have removed this because I don't think it is right to use
         * but put it back in if you want
         * 
         * if ( autofly(ship)  )
         * {
         * send_to_char("&RYou'll have to turn off the ships autopilot
         * first....\n\r",ch);
         * return;
         * }
         * 
         */


        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_shipsystems]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYour not sure which switch to flip.\n\r", ch);
                learn_from_failure(ch, gsn_shipsystems);
                return;
        }

        act(AtPlain, "$n flips a switch on the control panel.", ch,
            NULL, argument, ToRoom);

        if (target->bayopen)
        {
                echo_to_cockpit(AtYellow, ship,
                                "Your bay doors are already open.");
                return;
        }
        else
        {
                target->bayopen = TRUE;
                echo_to_cockpit(AtYellow, ship, "Opening Bay Doors.");
				echo_to_cockpit(AtYellow, target, "Hangar bay doors being opened remotely.");
        }

        learn_from_success(ch, gsn_shipsystems);


}

CMDF do_tractorbeam(CharData * ch, char *argument)
{

        char      arg[MaxInputLength];
        int       percent_chance = 0;
        ShipData *ship;
        ShipData *target;
        char      buf[MaxStringLength];

        mudstrlcpy(arg, argument, MIL);

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }


        if (!check_pilot(ch, ship))
        {
                send_to_char("This isn't your ship!\n\r", ch);
                return;
        }

        if (ship->tractorbeam == 0)
        {
                send_to_char("You might want to install a tractorbeam!\n\r",
                             ch);
                return;
        }

        if (ship->hanger == 0)
        {
                send_to_char("No hanger available.\n\r", ch);
                return;
        }

        if (!ship->bayopen)
        {
                send_to_char("Your hanger is closed.\n\r", ch);
                return;
        }


        if ((ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYou need to be in the pilot seat!\n\r", ch);
                return;
        }


        if (ship->shipstate == ShipDisabled)
        {
                send_to_char
                        ("&RThe ships drive is disabled. No power available.\n\r",
                         ch);
                return;
        }

        if (ship->shipstate == ShipDocked)
        {
                send_to_char("&RYour ship is docked!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {

                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }

        if (ship->shipstate != ShipReady)
        {
                send_to_char
                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                         ch);
                return;
        }




        if (argument[0] == '\0')
        {
                send_to_char("&RCapture what?\n\r", ch);
                return;
        }

        target = get_ship_here(argument, ship->starsystem);

        if (target == NULL)
        {
                send_to_char("&RI don't see that here.\n\r", ch);
                return;
        }

        if (target == ship)
        {
                send_to_char("&RYou can't yourself!\n\r", ch);
                return;
        }

        if (target->shipstate == ShipLand)
        {
                send_to_char
                        ("&RThat ship is already in a landing sequence.\n\r",
                         ch);
                return;
        }

        if ((target->vx > ship->vx + 200) || (target->vx < ship->vx - 200) ||
            (target->vy > ship->vy + 200) || (target->vy < ship->vy - 200) ||
            (target->vz > ship->vz + 200) || (target->vz < ship->vz - 200))
        {
                send_to_char
                        ("&R That ship is too far away! You'll have to fly a little closer.\n\r",
                         ch);
                return;
        }

        if (ship->ship_class <= target->ship_class)
        {
                send_to_char("&RThat ship is too big for your hanger.\n\r",
                             ch);
                return;
        }

        if (target->ship_class == ShipPlatform)
        {
                send_to_char("&RYou can't capture platforms.\n\r", ch);
                return;
        }

        if (target->ship_class == CapitalShip)
        {
                send_to_char("&RYou can't capture capital ships.\n\r", ch);
                return;
        }


        if (ship->energy < (25 + 25 * target->ship_class))
        {
                send_to_char("&RTheres not enough fuel!\n\r", ch);
                return;
        }




        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_tractorbeams]);

        /*
         * This is just a first guess percent_chance modifier, feel free to change if needed 
         */

        percent_chance =
                percent_chance * (ship->tractorbeam /
                                  (target->currspeed + 1));

        if (number_percent() < percent_chance)
        {
                set_char_color(AtGreen, ch);
                send_to_char("Capture sequence initiated.\n\r", ch);
                act(AtPlain, "$n begins the capture sequence.", ch,
                    NULL, argument, ToRoom);
                echo_to_ship(AtYellow, ship,
                             "ALERT: Ship is being captured, all hands to docking bay.");
                echo_to_ship(AtYellow, target,
                             "The ship shudders as a tractorbeam locks on.");
                snprintf(buf, MSL, "You are being captured by %s.",
                         ship->name);
                echo_to_cockpit(AtBlood, target, buf);

                if (autofly(target) && !target->target0)
                        target->target0 = ship;

                target->dest = STRALLOC(smash_color(ship->name));
                target->shipstate = ShipLand;
                target->currspeed = 0;

                learn_from_success(ch, gsn_tractorbeams);
                return;

        }
        send_to_char("You fail to work the controls properly.\n\r", ch);
        echo_to_ship(AtYellow, target,
                     "The ship shudders and then stops as a tractorbeam attemps to lock on.");
        snprintf(buf, MSL, "The %s attempted to capture your ship!",
                 ship->name);
        echo_to_cockpit(AtBlood, target, buf);
        if (autofly(target) && !target->target0)
                target->target0 = ship;


        learn_from_failure(ch, gsn_tractorbeams);

        return;
}

void output_shuttle(CharData * ch, ShuttleData * shuttle)
{
        StopData *stop = NULL;
        int       itt = 0;

        if (shuttle == NULL)
                return;
        if (shuttle->current == NULL)
                return;
        if (shuttle->first_stop == NULL)
                return;

        set_char_color(AtShip, ch);
        ch_printf(ch, "%s Schedule Information:\n\r", shuttle->name);

        stop = shuttle->current;
        /*
         * current port 
         */
        if (shuttle->state == ShuttleStateLanding
            || shuttle->state == ShuttleStateLanded)
        {
                ch_printf(ch, "Currently docked at %s.\n\r",
                          shuttle->current->stop_name);
                stop = stop->next;
        }

        send_to_char("Next stops: ", ch);
        /*
         * Safety Check 
         */
        if (stop == NULL)
                stop = shuttle->first_stop;

        itt = 0;
        while (1)
        {
                itt++;
                /*
                 * No stops i guess 
                 */
                if (stop == NULL)
                        break;

                /*
                 * WTF BUT IT CRASHES 
                 */
                if (shuttle == NULL)
                {
                        bug("SHUTTLE IS NULLLLLL", 0);
                        return;
                }

                if (itt > 4)
                        break;
                if (stop->stop_name)
                        ch_printf(ch, "%s  ", stop->stop_name);
                else
                        send_to_char("(unnamed)  ", ch);
                if ((stop = stop->next) == NULL)
                        stop = shuttle->first_stop;
        }
        send_to_char("\n\r", ch);
}

CMDF do_pluogus(CharData * ch, char *argument)
{
        ShuttleData *shuttle = NULL;

        argument = NULL;

        if (!has_comlink(ch))
        {
                send_to_char("You need a comlink to do that!\n\r", ch);
                return;
        }

        if ((shuttle = get_shuttle("Serin Pluogus")) != NULL)
        {
                output_shuttle(ch, shuttle);
                send_to_char("\n\r", ch);
        }

        if ((shuttle = get_shuttle("Serin Tocca")) != NULL)
        {
                output_shuttle(ch, shuttle);
                send_to_char("\n\r", ch);
        }

}

CMDF do_schedule(CharData * ch, char *argument)
{
        ShuttleData *shuttle = NULL;
        ShuttleData *found = NULL;
        StopData *stop = NULL;

        argument = NULL;

        if (!has_comlink(ch))
        {
                send_to_char("You need a comlink to do that!\n\r", ch);
                return;
        }

        if (!ch->in_room || !ch->in_room->vnum)
        {
                send_to_char
                        ("You are not in a situation to check the schedule for a shuttle.\n\r",
                         ch);
                return;
        }

        for (shuttle = first_shuttle; shuttle; shuttle = shuttle->next)
                for (stop = shuttle->first_stop; stop; stop = stop->next)
                        if (ch->in_room->vnum == stop->room)
                                found = shuttle;

        if (found != NULL)
        {
                output_shuttle(ch, found);
                send_to_char("\n\r", ch);
        }
        else
        {
                send_to_char
                        ("There is not shuttle scheduled to stop here.\n\r",
                         ch);
        }
}

CMDF do_chaff(CharData * ch, char *argument)
{
        int       percent_chance = 0;
        ShipData *ship;


        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }


        if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RThe controls are at the copilots seat!\n\r",
                             ch);
                return;
        }

        if (autofly(ship))
        {
                send_to_char
                        ("&RYou'll have to turn the autopilot off first...\n\r",
                         ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }
        if (ship->shipstate == ShipDocked)
        {
                send_to_char
                        ("&RYou can't do that until after you've launched!\n\r",
                         ch);
                return;
        }
        if (ship->chaff <= 0)
        {
                send_to_char("&RYou don't have any chaff to release!\n\r",
                             ch);
                return;
        }
        percent_chance = IsNpc(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_weaponsystems]);
        if (number_percent() > percent_chance)
        {
                send_to_char("&RYou can't figure out which switch it is.\n\r",
                             ch);
                learn_from_failure(ch, gsn_weaponsystems);
                return;
        }

        ship->chaff = FALSE;

        ship->chaff_released = TRUE;

        send_to_char("You flip the chaff release switch.\n\r", ch);
        act(AtPlain, "$n flips a switch on the control pannel", ch,
            NULL, argument, ToRoom);
        echo_to_cockpit(AtYellow, ship,
                        "A burst of chaff is released from the ship.");

        learn_from_success(ch, gsn_weaponsystems);

}

bool autofly(ShipData * ship)
{

        if (!ship)
                return FALSE;

        if (ship->type == MobShip)
                return TRUE;

        if (ship->autopilot)
                return TRUE;

        return FALSE;

}

CMDF do_selfdestruct(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        ShipData *ship;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if ((ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYou must be in the pilots seat!\n\r", ch);
                return;
        }

        if (ship->target0 || ship->target1 || ship->target2)
        {
                send_to_char
                        ("&RNot while the ship is enganged with an enemy!\n\r",
                         ch);
                return;
        }
        if (!str_cmp(ship->selfdestruct, "Not Installed"))
        {
                send_to_char
                        ("This ship is not equipped with a selfdestruct module!\n\r",
                         ch);
                return;
        }
        if (argument[0] == '\0')
        {
                send_to_char
                        ("&CUsage&R:&WSelfdestruct &R<&WPassword&R>&W\n\r",
                         ch);
                return;
        }
        if (atoi(argument) != ship->selfdpass)
        {
                send_to_char
                        ("Incorrect password! How dare you try to blow up this ship!&R'&W\n\r",
                         ch);
                return;
        }
        act(AtPlain, "$n flips a switch on the control panell.", ch,
            NULL, argument, ToRoom);

        echo_to_cockpit(AtBlood + AtBlink, ship,
                        "Your vision flashes white.... then fades to black.\n\r");
        snprintf(buf, MSL, "%s explodes in a burst of flames!", ship->name);
        echo_to_system(AtOrange, ship, buf, NULL);
        destroy_ship(ship, NULL);
}


CMDF do_unload_cargo(CharData * ch, char *argument)
{
        ShipData *ship;
        ShipData *target;
        int       cost;
        PlanetData *planet;

        if (argument[0] == '\0')
        {
                act(AtPlain, "Which ship do you want to unload?.", ch, NULL,
                    NULL, ToChar);
                return;
        }

        target = ship_in_room(ch->in_room, argument);

        if (!target)
        {
                act(AtPlain, "I see no $T here.", ch, NULL, argument,
                    ToChar);
                return;
        }
        if (!check_pilot(ch, target))
        {
                send_to_char("Hey, that's not your ship!\r\n", ch);
                return;
        }

        if (target->cargo == 0)
        {
                send_to_char("You don't have any cargo.\r\n", ch);
                return;
        }

        if (!IsSet(ch->in_room->RoomFlags, RoomImport)
            && !IsSet(ch->in_room->RoomFlags, RoomSpacecraft))
        {
                send_to_char("You can't do that here!", ch);
                return;
        }
        planet = ch->in_room->area->planet;

        if (!planet)
        {
                ship = ship_from_hanger(ch->in_room->vnum);

                if (!ship)
                {
                        send_to_char("You can't do that here!", ch);
                        return;
                }
                if ((ship->maxcargo - ship->cargo) < 1)
                {
                        send_to_char("There is no room for anymore cargo\r\n",
                                     ch);
                        return;
                }
                if (ship->cargo == 0)
                        ship->cargotype = CargoNone;

                if ((ship->cargo > 0) && (ship->cargotype != target->cargo))
                {
                        send_to_char
                                ("They have a differnt type of cargo.\n\r",
                                 ch);
                        return;
                }

                if (ship->cargotype >= CargoMax)
                {
                        send_to_char
                                ("You cannot unload contraband cargo. Try SELLCONTRABAND.\n\r",
                                 ch);
                        return;
                }
                if (ship->cargotype == CargoNone)
                        ship->cargotype = target->cargotype;
                if ((ship->maxcargo - ship->cargo) >= target->cargo)
                {
                        ship->cargo += target->cargo;
                        target->cargo = 0;
                        target->cargo = CargoNone;
                        send_to_char("Cargo unloaded.\r\n", ch);
                        return;
                }
                else
                {
                        target->cargo -= ship->maxcargo - ship->cargo;
                        ship->cargo = ship->maxcargo;
                        ch_printf(ch,
                                  "%s Loaded, %d tons still in %s hold.\r\n",
                                  ship->name, target->cargo, target->name);
                        return;
                }
        }

        if (target->cargotype >= CargoMax)
        {
                send_to_char
                        ("You cannot unload contraband cargo. Try SELLCONTRABAND.\n\r",
                         ch);
                return;
        }

        if (planet->cargoimport[target->cargotype] < 1)
        {
                send_to_char("You can't deliver that here.\r\n", ch);
                return;
        }
        cost = target->cargo;
        cost *= planet->cargoimport[target->cargotype];

        ch->gold += cost;
        target->cargo = 0;
        ch_printf(ch, "You recieve %d credits for a load of %s.\r\n", cost,
                  cargo_names[target->cargotype]);
        target->cargotype = CargoNone;
        save_ship(target);
        return;
}

CMDF do_load_cargo(CharData * ch, char *argument)
{
        ShipData *ship;
        ShipData *target;
        int       cost, cargo, i, amount;
        PlanetData *planet;
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);


        if (arg1[0] == '\0')
        {
                act(AtPlain, "Which ship do you want to load?.", ch, NULL,
                    NULL, ToChar);
                return;
        }

        target = ship_in_room(ch->in_room, arg1);

        if (!target)
        {
                act(AtPlain, "I don't see that ship here.", ch, NULL, NULL,
                    ToChar);
                return;
        }

        if (!check_pilot(ch, target))
        {
                send_to_char("Hey, that's not your ship!\r\n", ch);
                return;
        }

        if (!IsSet(ch->in_room->RoomFlags, RoomImport)
            && !IsSet(ch->in_room->RoomFlags, RoomSpacecraft))
        {
                send_to_char("You can't do that here!", ch);
                return;
        }
        if (arg3[0] == '\0')
                amount = target->maxcargo - target->cargo;
        else
                amount = atoi(arg3);


        planet = ch->in_room->area->planet;

        if (!planet)
        {
                ship = ship_from_hanger(ch->in_room->vnum);
                if (!ship)
                {
                        send_to_char("You can't do that here!", ch);
                        return;
                }
                if (ship->cargo == 0)
                {
                        send_to_char("They don't have any cargo\n\r", ch);
                        return;
                }
                if ((target->maxcargo - target->cargo) < 1)
                {
                        send_to_char("There is no room for anymore cargo\r\n",
                                     ch);
                        return;
                }
                if ((target->cargotype = !CargoNone)
                    && (ship->cargotype != target->cargotype))
                {
                        send_to_char
                                ("Maybe you should deliver your cargo first.\n\r",
                                 ch);
                        return;
                }

                if (target->cargotype >= CargoMax)
                {
                        send_to_char
                                ("You cannot load contraband cargo. Try BUYCONTRABAND.\n\r",
                                 ch);
                        return;
                }

                if (target->cargotype == CargoNone)
                        target->cargotype = ship->cargotype;

                if ((target->maxcargo - target->cargo) >= ship->cargo)
                {
                        target->cargo += ship->cargo;
                        ship->cargo = 0;
                        send_to_char("Cargo loaded.\r\n", ch);
                        return;
                }
                else
                {
                        ship->cargo -= target->maxcargo - target->cargo;
                        target->cargo = target->maxcargo;
                        send_to_char("Cargo Loaded.\r\n", ch);
                        return;
                }

        }
        if (arg2[0] == '\0')
        {
                send_to_char("&RWhat do you want to load&C&w\r\n", ch);
                return;
        }
        if (target->maxcargo - target->cargo < amount)
        {
                send_to_char("There is no room for that much Cargo.\r\n", ch);
                return;
        }

        cargo = CargoMax + 1;
        for (i = 1; i < CargoMax; i++)
        {
                if (!str_prefix(arg2, cargo_names[i]))
                        cargo = i;
        }

        if (cargo == CargoMax + 1)
        {
                send_to_char("That is not a cargo type.\r\n", ch);
                return;
        }

        if ((target->cargo > 0) && (target->cargotype != cargo))
        {
                send_to_char("Maybe you should deliver your cargo first\r\n",
                             ch);
                return;
        }

        if (planet->cargoexport[cargo] < 1)
        {
                send_to_char("We don't export those goods here\r\n", ch);
                return;
        }

        if (planet->resource[cargo] < amount)
        {
                send_to_char("&RSorry we do not have that much left.\r\n",
                             ch);
                return;
        }

        if (amount >= planet->resource[cargo])
                cost = planet->resource[cargo];
        else
                cost = amount;

        cost *= planet->cargoexport[cargo];

        if (ch->gold < cost)
        {
                send_to_char("You can't afford it!\r\n", ch);
                return;
        }
        ch->gold -= cost;

        if (amount >= planet->resource[cargo])
        {

                target->cargo += planet->resource[cargo];
                planet->resource[cargo] = 0;
                target->cargotype = cargo;
        }
        else
        {
                planet->resource[cargo] -= amount;
                target->cargo += amount;
                target->cargotype = cargo;
        }

        ch_printf(ch, "You pay %d credits for a load of %s.\r\n", cost,
                  cargo_names[cargo]);
        save_ship(target);
        return;
}

CMDF do_endsimulator(CharData * ch, char *argument)
{
        ShipData *ship;
        char      buf[MaxInputLength];

        argument = NULL;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char("You must be in the cockpit of a simulator.\n\r",
                             ch);
                return;
        }
        if (!IsSet(ship->flags, ShipSimulator))
        {
                send_to_char("You must be in the cockpit of a simulator.\n\r",
                             ch);
                return;
        }
        ship->shipyard = ship->sim_vnum;
        ship->shipstate = ShipReady;
        extract_ship(ship);
        ship_to_room(ship, ship->shipyard);
        ship->location = ship->shipyard;
        ship->lastdoc = ship->shipyard;
        ship->shipstate = ShipDocked;
        if (ship->starsystem)
                ship_from_starsystem(ship, ship->starsystem);
        save_ship(ship);
        send_to_char("The lights dim and the simulation ends.\n\r", ch);
        snprintf(buf, MSL,
                 "%s suddenly disapears from your viewcreen and off your radar.\n\r",
                 ship->name);
        echo_to_system(AtWhite, ship, buf, NULL);
}

CMDF do_cloak(CharData * ch, char *argument)
{
        int       percent_chance = 0;
        ShipData *ship;

        argument = NULL;

        if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the co-pilot seat of a ship to do that!\n\r",
                         ch);
                return;
        }

        /*
         * This can change while waiting :D 
         */
        if (ship->dockedto)
        {
                send_to_char("You are currently docked.", ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                if (ship->shipstate == ShipHyperspace)
                {
                        send_to_char
                                ("&RYou can only do that in realspace!\n\r",
                                 ch);
                        return;
                }
                if (ship->shipstate == ShipDocked)
                {
                        send_to_char
                                ("&RYou can't do that until after you've launched!\n\r",
                                 ch);
                        return;
                }
                if (ship->shipstate != ShipReady)
                {
                        send_to_char
                                ("&RPlease wait until the ship has finished its current manouver.\n\r",
                                 ch);
                        return;
                }
                if (ship->cloak == 0)
                {
                        send_to_char
                                ("&RThis ship is not equiped with a cloaking device.\n\r",
                                 ch);
                        return;
                }

                if (ship->energy < 1)
                {
                        send_to_char("&RThere is not enough fuel!\n\r", ch);
                        return;
                }


                percent_chance = IsNpc(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_cloak]);
                if (number_percent() < percent_chance)
                {
                        act(AtPlain, "$n pulls a series of levers.", ch,
                            NULL, NULL, ToRoom);
                        echo_to_room(AtYellow, get_room_index(ship->coseat),
                                     "");
                        add_timer(ch, TimerDoFun, 0, do_cloak, 1);
                        return;
                }
                learn_from_failure(ch, gsn_cloak);
                send_to_char
                        ("&RYou can't figure out which levers to use!\n\r",
                         ch);
                return;

        case 1:
                break;

        case SubTimerDoAbort:
                /*
                 * DISPOSE(ch->dest_buf); this is not nessecary as we are not passing arguments 
                 */
                ch->substate = SubNone;
                if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
                {
                        send_to_char("&Raborted.\n\r", ch);
                        echo_to_room(AtYellow, get_room_index(ship->coseat),
                                     "");
                        return;
                }
                if (ship->shipstate != ShipDisabled)
                        ship->shipstate = ShipReady;

                return;
        }

        ch->substate = SubNone;

        if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
        {
                return;
        }


        if (IsSet(ship->flags, ShipCloak))
        {
                act(AtYellow, "The ship's cloaking systems de-activate.", ch,
                    NULL, argument, ToRoom);
                act(AtYellow,
                    "The sensors flicker to life as they begin to detect ships.",
                    ch, NULL, argument, ToRoom);
                echo_to_room(AtYellow, get_room_index(ship->coseat), "");
        }
        /*
         * if ( !IsSet (ship->flags, ShipCloak)) .... - gavin 
         */
        else
        {
                act(AtYellow, "The ship's cloaking device activate.", ch,
                    NULL, argument, ToRoom);
                act(AtYellow,
                    "The sensors fade to dark as their beams are deflected by the cloaking field.",
                    ch, NULL, argument, ToRoom);
                echo_to_room(AtYellow, get_room_index(ship->coseat), "");
        }

        if (IsSet(ship->flags, ShipCloak))
        {
                RemoveBit(ship->flags, ShipCloak);
                send_to_char("&GCloaking Device De-activated\n\r", ch);
                send_to_char("&GSensors Enabled\n\r", ch);
        }
/*	if ( !IsSet (ship->flags, ShipCloak)) ... - Gavin */
        else
        {
                SetBit(ship->flags, ShipCloak);
                send_to_char("&GCloaking Device Activated\n\r", ch);
                send_to_char("&GSensors Disabled\n\r", ch);
        }
        learn_from_success(ch, gsn_cloak);
}

CMDF do_stealth(CharData * ch, char *argument)
{

        {
                char      arg[MaxInputLength];
                int       percent_chance = 0;
                ShipData *ship;

                mudstrlcpy(arg, argument, MIL);

                switch (ch->substate)
                {
                default:
                        if ((ship =
                             ship_from_coseat(ch->in_room->vnum)) == NULL)
                        {
                                send_to_char
                                        ("&RYou must be in the co-pilot seat of a ship to do that!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipHyperspace)
                        {
                                send_to_char
                                        ("&RYou can only do that in realspace!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipDocked)
                        {
                                send_to_char
                                        ("&RYou can't do that until after you've launched!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate != ShipReady)
                        {
                                send_to_char
                                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                                         ch);
                                return;
                        }
                        if (ship->stealth == 0)
                        {
                                send_to_char
                                        ("&RThis ship is not equiped with stealth technology.\n\r",
                                         ch);
                                return;
                        }

                        if (ship->energy < 1)
                        {
                                send_to_char
                                        ("&RThere is not enough fuel!\n\r",
                                         ch);
                                return;
                        }


                        percent_chance = IsNpc(ch) ? ch->top_level
                                : (int) (ch->pcdata->learned[gsn_stealth]);
                        if (number_percent() < percent_chance)
                        {
                                act(AtPlain, "$n pulls a series of levers.",
                                    ch, NULL, argument, ToRoom);
                                echo_to_room(AtYellow,
                                             get_room_index(ship->coseat),
                                             "");
                                add_timer(ch, TimerDoFun, 0, do_stealth, 1);
                                ch->dest_buf = str_dup(arg);
                                return;
                        }
                        learn_from_failure(ch, gsn_stealth);
                        send_to_char
                                ("&RYou can't figure out which levers to use!\n\r",
                                 ch);
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
                        if ((ship =
                             ship_from_coseat(ch->in_room->vnum)) == NULL)
                        {
                                send_to_char("&Raborted.\n\r", ch);
                                echo_to_room(AtYellow,
                                             get_room_index(ship->coseat),
                                             "");
                                return;
                        }
                        if (ship->shipstate != ShipDisabled)
                                ship->shipstate = ShipReady;

                        return;
                }

                ch->substate = SubNone;

                if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
                {
                        return;
                }


                if (IsSet(ship->flags, ShipStealth))
                {
                        act(AtYellow,
                            "The ship's stealth systems de-activate.", ch,
                            NULL, argument, ToRoom);
                        echo_to_room(AtYellow, get_room_index(ship->coseat),
                                     "");
                }
                if (!IsSet(ship->flags, ShipStealth))
                {
                        act(AtYellow, "The ship's stealth systems activate.",
                            ch, NULL, argument, ToRoom);
                        echo_to_room(AtYellow, get_room_index(ship->coseat),
                                     "");
                }

                learn_from_success(ch, gsn_stealth);

                if (IsSet(ship->flags, ShipStealth))
                {
                        RemoveBit(ship->flags, ShipStealth);
                        send_to_char("&GStealth Systems De-activated\n\r",
                                     ch);
                        return;
                }
                if (!IsSet(ship->flags, ShipStealth))
                {
                        SetBit(ship->flags, ShipStealth);
                        send_to_char("&GStealth Systems Activated\n\r", ch);
                        return;
                }


        }
}
CMDF do_juke(CharData * ch, char *argument)
{

        {
                char      arg[MaxInputLength];
                int       percent_chance = 0;
                ShipData *ship;
                long int  xp;

                mudstrlcpy(arg, argument, MIL);

                switch (ch->substate)
                {
                default:
                        if ((ship =
                             ship_from_cockpit(ch->in_room->vnum)) == NULL)
                        {
                                send_to_char
                                        ("&RYou must be in the cockpit seat of a ship to do that!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipHyperspace)
                        {
                                send_to_char
                                        ("&RYou can only do that in realspace!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipDocked)
                        {
                                send_to_char
                                        ("&RYou can't do that until after you've launched!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate != ShipReady)
                        {
                                send_to_char
                                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                                         ch);
                                return;
                        }

                        if (ship->energy < 1)
                        {
                                send_to_char
                                        ("&RThere is not enough fuel!\n\r",
                                         ch);
                                return;
                        }


                        percent_chance = IsNpc(ch) ? ch->top_level
                                : (int) (ch->pcdata->learned[gsn_juke]);
                        if (number_percent() < percent_chance)
                        {

                                send_to_char
                                        ("&YYou start to move the ship erratically\n\r",
                                         ch);
                                add_timer(ch, TimerDoFun, 0, do_juke, 1);
                                ch->dest_buf = str_dup(arg);
                                return;
                        }
                        learn_from_failure(ch, gsn_juke);
                        send_to_char
                                ("&RYou can't figure out which levers to use!\n\r",
                                 ch);
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

                        send_to_char("&RAborted.\n\r", ch);
                        ship = ship_from_cockpit(ch->in_room->vnum);
                        if (ship)
                                if (ship->shipstate != ShipDisabled)
                                        ship->shipstate = ShipReady;

                        return;
                }

                ch->substate = SubNone;

                if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
                {
                        return;
                }


                send_to_char
                        ("&YYou complete you maneuver and continue to move the ship erratically\n\r",
                         ch);
                xp = (exp_level(ch->skill_level[PilotingAbility] + 1) -
                      exp_level(ch->skill_level[PilotingAbility])) / 25;
                xp = UMin(get_ship_value(ship) / 100, xp);
                gain_exp(ch, xp, PilotingAbility);
                ch_printf(ch, "&YYou gain %ld piloting experience!", xp);
                ship->evasive = 25;
                learn_from_success(ch, gsn_juke);

        }
}


CMDF do_roll(CharData * ch, char *argument)
{

        {
                char      arg[MaxInputLength];
                int       percent_chance = 0;
                ShipData *ship;
                long int  xp;

                mudstrlcpy(arg, argument, MIL);

                switch (ch->substate)
                {
                default:
                        if ((ship =
                             ship_from_cockpit(ch->in_room->vnum)) == NULL)
                        {
                                send_to_char
                                        ("&RYou must be in the cockpit seat of a ship to do that!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipHyperspace)
                        {
                                send_to_char
                                        ("&RYou can only do that in realspace!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipDocked)
                        {
                                send_to_char
                                        ("&RYou can't do that until after you've launched!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate != ShipReady)
                        {
                                send_to_char
                                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                                         ch);
                                return;
                        }

                        if (ship->energy < 1)
                        {
                                send_to_char
                                        ("&RThere is not enough fuel!\n\r",
                                         ch);
                                return;
                        }


                        percent_chance = IsNpc(ch) ? ch->top_level
                                : (int) (ch->pcdata->learned[gsn_roll]);
                        if (number_percent() < percent_chance)
                        {
                                send_to_char
                                        ("&YYou start to move the ship in a roll maneuver.\n\r",
                                         ch);
                                add_timer(ch, TimerDoFun, 0, do_roll, 1);
                                ch->dest_buf = str_dup(arg);
                                return;
                        }
                        learn_from_failure(ch, gsn_roll);
                        send_to_char
                                ("&RYou can't figure out which levers to use!\n\r",
                                 ch);
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

                        send_to_char("&RAborted.\n\r", ch);
                        ship = ship_from_cockpit(ch->in_room->vnum);
                        if (ship)
                                if (ship->shipstate != ShipDisabled)
                                        ship->shipstate = ShipReady;

                        return;
                }

                ch->substate = SubNone;


                if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
                {
                        return;
                }


                send_to_char
                        ("&YYou complete you maneuver and continue to roll the ship to evade attack.\n\r",
                         ch);
                xp = (exp_level(ch->skill_level[PilotingAbility] + 1) -
                      exp_level(ch->skill_level[PilotingAbility])) / 25;
                xp = UMin(get_ship_value(ship) / 100, xp);
                gain_exp(ch, xp, PilotingAbility);
                ch_printf(ch, "&YYou gain %ld piloting experience!", xp);

                ship->evasive = 45;
                learn_from_success(ch, gsn_roll);

        }
}

CMDF do_evade(CharData * ch, char *argument)
{

        {
                char      arg[MaxInputLength];
                int       percent_chance = 0;
                ShipData *ship;
                long int  xp;

                mudstrlcpy(arg, argument, MIL);

                switch (ch->substate)
                {
                default:
                        if ((ship =
                             ship_from_cockpit(ch->in_room->vnum)) == NULL)
                        {
                                send_to_char
                                        ("&RYou must be in the cockpit seat of a ship to do that!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipHyperspace)
                        {
                                send_to_char
                                        ("&RYou can only do that in realspace!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate == ShipDocked)
                        {
                                send_to_char
                                        ("&RYou can't do that until after you've launched!\n\r",
                                         ch);
                                return;
                        }
                        if (ship->shipstate != ShipReady)
                        {
                                send_to_char
                                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                                         ch);
                                return;
                        }

                        if (ship->energy < 1)
                        {
                                send_to_char
                                        ("&RThere is not enough fuel!\n\r",
                                         ch);
                                return;
                        }


                        percent_chance = IsNpc(ch) ? ch->top_level
                                : (int) (ch->pcdata->learned[gsn_evade]);
                        if (number_percent() < percent_chance)
                        {
                                send_to_char
                                        ("&YYou start to move the ship to evade attack.\n\r",
                                         ch);
                                add_timer(ch, TimerDoFun, 0, do_evade, 1);
                                ch->dest_buf = str_dup(arg);
                                return;
                        }
                        learn_from_failure(ch, gsn_evade);
                        send_to_char
                                ("&RYou can't figure out which levers to use!\n\r",
                                 ch);
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

                        send_to_char("&RAborted.\n\r", ch);
                        ship = ship_from_cockpit(ch->in_room->vnum);
                        if (ship)
                                if (ship->shipstate != ShipDisabled)
                                        ship->shipstate = ShipReady;

                        return;
                }

                ch->substate = SubNone;

                if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
                {
                        return;
                }

                send_to_char
                        ("&YYou complete you maneuver and continue to move the ship to evade attack.\n\r",
                         ch);
                xp = (exp_level(ch->skill_level[PilotingAbility] + 1) -
                      exp_level(ch->skill_level[PilotingAbility])) / 25;
                xp = UMin(get_ship_value(ship) / 100, xp);
                gain_exp(ch, xp, PilotingAbility);
                ch_printf(ch, "&YYou gain %ld piloting experience!", xp);
                ship->evasive = 5;
                learn_from_success(ch, gsn_evade);

        }
}

CMDF do_interdictor(CharData * ch, char *argument)
{
        char      arg[MaxInputLength];
        int       percent_chance = 0;
        ShipData *ship;


        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
                {
                        send_to_char
                                ("&RYou must be in the co-pilot seat of a ship to do that!\n\r",
                                 ch);
                        return;
                }
                if (ship->shipstate == ShipHyperspace)
                {
                        send_to_char
                                ("&RYou can only do that in realspace!\n\r",
                                 ch);
                        return;
                }
                if (ship->shipstate == ShipDocked)
                {
                        send_to_char
                                ("&RYou can't do that until after you've launched!\n\r",
                                 ch);
                        return;
                }
                if (ship->shipstate != ShipReady)
                {
                        send_to_char
                                ("&RPlease wait until the ship has finished its current manouver.\n\r",
                                 ch);
                        return;
                }
                if (ship->interdictor == 0)
                {
                        send_to_char
                                ("&RThis ship is not equiped with a gravity cone.\n\r",
                                 ch);
                        return;
                }

                if (ship->energy < 5000)
                {
                        send_to_char
                                ("&RThat energy is needed for basic ship functions!\n\r",
                                 ch);
                        return;
                }


                if (ship->ship_class == FighterShip)
                        percent_chance = IsNpc(ch) ? ch->top_level
                                : (int) (ch->pcdata->
                                         learned[gsn_starfighters]);
                if (ship->ship_class == MidsizeShip)
                        percent_chance = IsNpc(ch) ? ch->top_level
                                : (int) (ch->pcdata->learned[gsn_midships]);
                if (ship->ship_class == CapitalShip)
                        percent_chance = IsNpc(ch) ? ch->top_level
                                : (int) (ch->pcdata->
                                         learned[gsn_capitalships]);
                if (number_percent() < percent_chance)
                {
                        act(AtPlain, "$n opens and throws a covered switch.",
                            ch, NULL, argument, ToRoom);
                        echo_to_room(AtYellow, get_room_index(ship->coseat),
                                     "");
                        add_timer(ch, TimerDoFun, 0, do_interdictor, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                if (ship->ship_class == FighterShip)
                        learn_from_failure(ch, gsn_starfighters);
                if (ship->ship_class == MidsizeShip)
                        learn_from_failure(ch, gsn_midships);
                if (ship->ship_class == CapitalShip)
                        learn_from_failure(ch, gsn_capitalships);
                send_to_char
                        ("&RYou can't figure out which switch to use!\n\r",
                         ch);
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
                if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
                        return;
                send_to_char("&Raborted.\n\r", ch);
                echo_to_room(AtYellow, get_room_index(ship->coseat), "");
                if (ship->shipstate != ShipDisabled)
                        ship->shipstate = ShipReady;
                return;
        }

        ch->substate = SubNone;

        if ((ship = ship_from_coseat(ch->in_room->vnum)) == NULL)
        {
                return;
        }

        act(AtYellow, "The lights flicker as the ships power is re-routed",
            ch, NULL, argument, ToRoom);
        echo_to_room(AtYellow, get_room_index(ship->coseat), "");


        if (ship->ship_class == FighterShip)
                learn_from_success(ch, gsn_starfighters);
        if (ship->ship_class == MidsizeShip)
                learn_from_success(ch, gsn_midships);
        if (ship->ship_class == CapitalShip)
                learn_from_success(ch, gsn_capitalships);

        if (IsSet(ship->flags, ShipInterdictor))
        {
                RemoveBit(ship->flags, ShipInterdictor);
                send_to_char("&GInterdictor Field Disabled\n\r", ch);
                return;
        }
        if (!IsSet(ship->flags, ShipInterdictor))
        {
                SetBit(ship->flags, ShipInterdictor);
                send_to_char("&GInterdictor Field Enabled\n\r", ch);
                return;
        }

}

CMDF do_boardship(CharData * ch, char *argument)
{
        ShipData *ship;
        int       percent_chance = 0;

        argument = NULL;

        if ((ship = ship_from_entrance(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be at the entrance to your ship!\n\r",
                         ch);
                return;
        }

        if (!ship->dockedto)
        {
                send_to_char("&RYou aren't docked.\n\r", ch);
                return;
        }

        if (ship->dockedto->starsystem != ship->starsystem)
        {
                send_to_char("Your ship is not docked properly", ch);
                /*
                 * Undocking 
                 */
                ship->dockedto->dockedto = NULL;
                ship->dockedto = NULL;
                return;
        }
        percent_chance =
                IsNpc(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_boardship]);
        if (number_percent() > percent_chance)
        {
                /*
                 * Failure 
                 */
                damage_ship(ship, 10, 20);
                damage_ship(ship->dockedto, 10, 20);
                send_to_char
                        ("You fail at your forced entry and end up damaging the ships.\n\r",
                         ch);
                echo_to_ship(AtRed, ship,
                             "The two ships slowly creep apart.");
                echo_to_ship(AtRed, ship->dockedto,
                             "The two ships slowly creep apart.");
                ship->dockedto->dockedto = NULL;
                ship->dockedto = NULL;
                learn_from_failure(ch, gsn_boardship);
                return;
        }

        send_to_char("You board you target ship!.\n\r", ch);
        echo_to_ship(AtYellow, ship->dockedto,
                     "WARNING! Your ship is being boarded!.");

        learn_from_success(ch, gsn_boardship);
        ship->dockedto->hatchopen = TRUE;
        ship->hatchopen = TRUE;
        send_to_char("Hatch forced open.\n\r", ch);
        do_board(ch, "");
}

void damage_ship_ch_ion(ShipData * ship, int min, int max, CharData * ch)
{
        int       damage_amount, numloop, x, shield_dmg;

        damage_amount = number_range(min, max) * 5;

        /*
         * Random # used for a loop. The loop will run through accordingly disabling a random system each time.
         * Could disable as many as all 11 systems in one hit though highly unlikely.
         *
         */
        numloop = number_range(1, 3);

        if (ch)
        {
                long      xp =
                        (exp_level(ch->skill_level[PilotingAbility] + 1) -
                         exp_level(ch->skill_level[PilotingAbility])) / 25;
                xp = UMin(get_ship_value(ship) / 100, xp);
                gain_exp(ch, xp, PilotingAbility);
        }

        if (ship->shield > 0)
        {
                shield_dmg = UMin(ship->shield, damage_amount);
                damage_amount -= shield_dmg;
                ship->shield -= shield_dmg;
                if (ship->shield == 0)
                        echo_to_cockpit(AtBlood, ship, "Shields down...");
        }


        for (x = 0; x <= numloop; x++)
        {
                int       range;

                range = number_range(1, 6);
                switch (range)
                {
                case 1:
                        if (number_range(1, 100) <= 80
                            && ship->shipstate != ShipDisabled)
                        {
                                echo_to_cockpit(AtBlood + AtBlink, ship,
                                                "&R[&WALERT&R] &WShips Drive DAMAGED!");
                                ship->shipstate = ShipDisabled;
                        }
                        break;
                case 2:
                        if (number_range(1, 100) <= 75
                            && ship->missilestate != MissileDamaged
                            && ship->maxmissiles > 0)
                        {
                                echo_to_room(AtBlood + AtBlink,
                                             get_room_index(ship->gunseat),
                                             "&R[&WALERT&R] &WShips Missile Launcher DAMAGED!");
                                echo_to_cockpit(AtBlood + AtBlink, ship,
                                                "&R[&WALERT&R] &WShips Missile Launcher DAMAGED!");
                                ship->missilestate = MissileDamaged;
                        }
                        break;
                case 3:
                        if (number_range(1, 100) <= 60
                            && ship->statet0i != LaserDamaged)
                        {
                                echo_to_room(AtBlood + AtBlink,
                                             get_room_index(ship->gunseat),
                                             "&R[&WALERT&R] &WIon Cannons DAMAGED!");
                                echo_to_cockpit(AtBlood + AtBlink, ship,
                                                "&R[&WALERT&R] &WIon Cannons Damaged!");
                                ship->statet0i = LaserDamaged;
                        }
                        break;
                case 4:
                        if (number_range(1, 100) <= 70
                            && ship->statet0 != LaserDamaged)
                        {
                                echo_to_room(AtBlood + AtBlink,
                                             get_room_index(ship->gunseat),
                                             "&R[&WALERT&R] &WLasers DAMAGED!");
                                echo_to_cockpit(AtBlood + AtBlink, ship,
                                                "&R[&WALERT&R] &WLasers DAMAGED!");
                                ship->statet0 = LaserDamaged;
                        }
                        break;
                case 5:
                        if (number_range(1, 100) <= 80
                            && ship->statet1 != LaserDamaged
                            && ship->turret1)
                        {
                                echo_to_room(AtBlood + AtBlink,
                                             get_room_index(ship->turret1),
                                             "&R[&WALERT&R] &WTurret DAMAGED!");
                                echo_to_cockpit(AtBlood + AtBlink, ship,
                                                "&R[&WALERT&R] &WTurret DAMAGED!");
                                ship->statet1 = LaserDamaged;
                        }
                        break;
                case 6:
                        if (number_range(1, 100) <= 80
                            && ship->statet2 != LaserDamaged
                            && ship->turret2)
                        {
                                echo_to_room(AtBlood + AtBlink,
                                             get_room_index(ship->turret2),
                                             "&R[&WALERT&R] &WTurret DAMAGED!");
                                echo_to_cockpit(AtBlood + AtBlink, ship,
                                                "&R[&WALERT&R] &WTurret DAMAGED!");
                                ship->statet2 = LaserDamaged;
                        }
                        break;

                default:
                        bug("Reached 'default' in damage_ship_ch_ion");
                        break;
                }
        }
}

void damage_ship_ion(ShipData * ship, int min, int max)
{
        damage_ship_ch_ion(ship, min, max, NULL);
}

CMDF do_giveship(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        ShipData *ship;
        CharData *victim;
        ClanData *clan;


        if (IsNpc(ch))
                return;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Usage: giveship <ship> <player>\n\r", ch);
                send_to_char("Usage: giveship <ship> clan\n\r", ch);
                send_to_char
                        ("Note: player must be in room, or you must belong to a clan.\n\r",
                         ch);
                return;
        }


        ship = ship_in_room(ch->in_room, arg1);
        if (!ship)
        {
                act(AtPlain, "I see no $T here.", ch, NULL, argument,
                    ToChar);
                return;
        }

        if (str_cmp(ship->owner, ch->name))
        {
                send_to_char("&RThat isn't your ship!", ch);
                return;
        }

        if (!str_cmp(arg2, "clan"))
        {
                clan = ch->pcdata->clan;
                if (!clan)
                {
                        send_to_char("You are not in a clan, sorry.", ch);
                        return;
                }

                send_to_char("You set the computer with the new owners name",
                             ch);

                ship->owner = clan->name;
                save_ship(ship);
                return;
        }

        if ((victim = get_char_room(ch, arg2)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        send_to_char("You set the computer with the new owners name", ch);

        ship->owner = victim->name;
        save_ship(ship);
}


CMDF do_taxi(CharData * ch, char *argument)
{
        CharData *rch;
        ShipData *ship;
        int       roomnum;
        RoomIndexData *room;
        sh_int    count = 0;
        sh_int    ship_count = 0;

        argument = NULL;

        if (ch->gold < 150)
        {
                send_to_char
                        ("You do not have enough money to call for a taxi!\n\r",
                         ch);
                return;
        }
        if (!IsSet(ch->in_room->RoomFlags, RoomImport) && !IsSet(ch->in_room->RoomFlags, RoomCanLand))
        {
                send_to_char("You must be at a spaceport to call a taxi.\n\r",
                             ch);
                return;
        }
        for (ship = first_ship; ship; ship = ship->next)
        {
                count = 0;
                if (IsSet(ship->flags, ShipTaxi))
                {
                        for (roomnum = ship->firstroom;
                             roomnum <= ship->lastroom; roomnum++)
                        {
                                room = get_room_index(roomnum);

                                if (room != NULL)
                                {
                                        rch = room->first_person;
                                        if (rch)
                                                count++;
                                }
                        }
                        if (count > 0)
                                continue;
                        else
                        {
                                transship(ship, ch->in_room->vnum);
                                ship_count++;
                                break;
                        }
                }
        }


        if (ship_count > 0)
        {
                send_to_char("Your ship arrives at the spaceport.\n\r", ch);
                return;
        }
        else
        {
                send_to_char("There are no taxi's available.\n\r", ch);
                return;
        }
}

CMDF do_renameships(CharData * ch, char *argument)
{
        ShipData *ship = NULL;
        char      arg[MSL];

        argument = one_argument(argument, arg);
        if (!arg || arg[0] == '\0' || !argument || argument[0] == '\0')
        {
                send_to_char("Syntax: renameships <oldname> <newname>", ch);
                return;
        }

        for (ship = first_ship; ship; ship = ship->next)
        {
                if (!str_cmp(arg, ship->owner))
                {
                        STRFREE(ship->owner);
                        ship->owner = STRALLOC(argument);
                }
                if (!str_cmp(arg, ship->pilot))
                {
                        STRFREE(ship->pilot);
                        ship->pilot = STRALLOC(argument);
                }
                if (!str_cmp(arg, ship->copilot))
                {
                        STRFREE(ship->copilot);
                        ship->copilot = STRALLOC(argument);
                }
                save_ship(ship);
        }
        send_to_char("Done", ch);
}

/* Generic Pilot Command To use as template

CMDF do_hmm( CharData *ch, char *argument )
{
    char arg[MaxInputLength];
    int percent_chance;
    ShipData *ship;
    
    mudstrlcpy( arg, argument, MSL );    
    
    switch( ch->substate )
    { 
    	default:
    	        if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    	        {
    	            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	            return;
    	        }
                if (ship->shipstate == ShipHyperspace)
                {
                  send_to_char("&RYou can only do that in realspace!\n\r",ch);
                  return;   
                }
                if (ship->shipstate == ShipDisabled)
    	        {
    	            send_to_char("&RThe ships drive is disabled. Unable to manuever.\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate == ShipDocked)
    	        {
    	            send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate != ShipReady)
    	        {
    	            send_to_char("&RPlease wait until the ship has finished its current manouver.\n\r",ch);
    	            return;
    	        }
        
                if ( ship->energy <1 )
    	        {
    	           send_to_char("&RTheres not enough fuel!\n\r",ch);
    	           return;
    	        }
    	        
                if ( ship->ship_class == FighterShip )
                    percent_chance = IsNpc(ch) ? ch->top_level
	                 : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
                if ( ship->ship_class == MidsizeShip )
                    percent_chance = IsNpc(ch) ? ch->top_level
	                 : (int)  (ch->pcdata->learned[gsn_midships]) ;
                if ( ship->ship_class == CapitalShip )
                    percent_chance = IsNpc(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_capitalships]);
                if ( number_percent( ) < percent_chance )
    		{
    		   send_to_char( "&G\n\r", ch);
    		   act( AtPlain, "$n does  ...", ch,
		        NULL, argument , ToRoom );
		   echo_to_room( AtYellow , get_room_index(ship->cockpit) , "");
    		   add_timer ( ch , TimerDoFun , 1 , do_hmm , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou fail to work the controls properly.\n\r",ch);
	        if ( ship->ship_class == FighterShip )
                    learn_from_failure( ch, gsn_starfighters );
                if ( ship->ship_class == MidsizeShip )
    	            learn_from_failure( ch, gsn_midships );
                if ( ship->ship_class == CapitalShip )
                    learn_from_failure( ch, gsn_capitalships );
    	   	return;	
    	
    	case 1:
    		if ( !ch->dest_buf )
    		   return;
    		mudstrlcpy(arg, ch->dest_buf, MIL);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SubTimerDoAbort:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SubNone;
    		if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    		      return;    		                                   
    	        send_to_char("&Raborted.\n\r", ch);
    	        echo_to_room( AtYellow , get_room_index(ship->cockpit) , "");
    		if (ship->shipstate != ShipDisabled)
    		   ship->shipstate = ShipReady;
    		return;
    }
    
    ch->substate = SubNone;
    
    if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    {  
       return;
    }

    send_to_char( "&G\n\r", ch);
    act( AtPlain, "$n does  ...", ch,
         NULL, argument , ToRoom );
    echo_to_room( AtYellow , get_room_index(ship->cockpit) , "");

         
    if ( ship->ship_class == FighterShip )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->ship_class == MidsizeShip )
        learn_from_success( ch, gsn_midships );
    if ( ship->ship_class == CapitalShip )
        learn_from_success( ch, gsn_capitalships );
    	
}





CMDF do_hmm( CharData *ch, char *argument )
{
    char arg[MaxInputLength];
    int percent_chance;
    ShipData *ship;
    
   
        if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
        {
            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
            return;
        }
        
                if (ship->shipstate == ShipHyperspace)
                {
                  send_to_char("&RYou can only do that in realspace!\n\r",ch);
                  return;   
                }
                if (ship->shipstate == ShipDisabled)
    	        {
    	            send_to_char("&RThe ships drive is disabled. Unable to manuever.\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate == ShipDocked)
    	        {
    	            send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate != ShipReady)
    	        {
    	            send_to_char("&RPlease wait until the ship has finished its current manouver.\n\r",ch);
    	            return;
    	        } 
        
        if ( ship->energy <1 )
        {
              send_to_char("&RTheres not enough fuel!\n\r",ch);
              return;
        }
    	        
        if ( ship->ship_class == FighterShip )
             percent_chance = IsNpc(ch) ? ch->top_level
             : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
        if ( ship->ship_class == MidsizeShip )
             percent_chance = IsNpc(ch) ? ch->top_level
                 : (int)  (ch->pcdata->learned[gsn_midships]) ;
        if ( ship->ship_class == CapitalShip )
              percent_chance = IsNpc(ch) ? ch->top_level
                 : (int) (ch->pcdata->learned[gsn_capitalships]);
        if ( number_percent( ) > percent_chance )
        {
            send_to_char("&RYou fail to work the controls properly.\n\r",ch);
            if ( ship->ship_class == FighterShip )
               learn_from_failure( ch, gsn_starfighters );
            if ( ship->ship_class == MidsizeShip )   
               learn_from_failure( ch, gsn_midships );
            if ( ship->ship_class == CapitalShip )
                learn_from_failure( ch, gsn_capitalships );
    	   return;	
        }
        
    send_to_char( "&G\n\r", ch);
    act( AtPlain, "$n does  ...", ch,
         NULL, argument , ToRoom );
    echo_to_room( AtYellow , get_room_index(ship->cockpit) , "");
	  
    
    
    if ( ship->ship_class == FighterShip )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->ship_class == MidsizeShip )
        learn_from_success( ch, gsn_midships );
    if ( ship->ship_class == CapitalShip )
        learn_from_success( ch, gsn_capitalships );
    	
}

*/

CMDF do_shipemote(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        ShipData *ship;

        if (argument[0] == '\0')
        {
                send_to_char("Shipemote what?\n\r", ch);
                return;
        }

        if (IsNpc(ch))
                return;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->shipstate == ShipDocked)
        {
                send_to_char
                        ("&RThe ship is docked! You must be in space.\n\r",
                         ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {
                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }
        if (ship->starsystem == NULL)
        {
                send_to_char("&RYou must be around other ships to do that!",
                             ch);
                return;
        }

        mudstrlcpy(buf, argument, MSL);
        if (argument[strlen(argument)] != '.'
            && argument[strlen(argument)] != '?'
            && argument[strlen(argument)] != '!')
                mudstrlcat(buf, ".", MSL);
        MOBtrigger = FALSE;
        if (!str_prefix("the", ship->name))
                snprintf(buf, MaxStringLength, "%s %s", ship->name,
                         argument);
        else
                snprintf(buf, MaxStringLength, "The %s %s", ship->name,
                         argument);
        echo_to_system(AtOrange, ship, buf, NULL);
        MOBtrigger = FALSE;
        snprintf(buf, MaxStringLength, "%s %s", ship->name, argument);
        echo_to_cockpit(AtBlood + AtBlink, ship, buf);
        if (IsSet(ch->in_room->RoomFlags, RoomLogspeech))
        {
                snprintf(buf, MSL, "%s %s (emote)",
                         IsNpc(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LogFile, buf);
        }
        return;
}

void free_system(SpaceData * system)
{
        if (system->name)
                STRFREE(system->name);
        if (system->filename)
                DISPOSE(system->filename);
        DELETE(system);
}


CMDF do_shiphail(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        char      arg[MaxStringLength];
        char      arg2[MaxStringLength];
        ShipData *target;
        ShipData *ship;

        argument = one_argument(argument, arg);
        strcpy(arg2, argument);

        if (!ch->in_room)
                return;

        if (argument[0] == '\0')
        {
                send_to_char("Hail who?", ch);
                return;
        }

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                send_to_char("&RUsage: hail <ship> <message>\n\r&w", ch);
                return;
        }

        if (!ship->starsystem)
        {
                send_to_char("&RYou need to launch first!\n\r&w", ch);
                return;
        }

        target = get_ship_here(arg, ship->starsystem);

        if (target == NULL)
        {

                send_to_char("&RThat ship isn't here!\n\r", ch);
                return;
        }

        if (target == ship)
        {
                send_to_char("&RWhy don't you just say it?\n\r", ch);
                return;
        }


        if ((abs((int) (target->vx - ship->vx)) >
             100 * (ship->comm + target->comm + 20))
            || (abs((int) (target->vx - ship->vx)) >
                100 * (ship->comm + target->comm + 20))
            || (abs((int) (target->vx - ship->vx)) >
                100 * (ship->comm + target->comm + 20)))
        {
                send_to_char
                        ("&RThat ship is out of the range of your comm system.\n\r&w",
                         ch);
                return;
        }

        {
            /* Use stacked buffer approach to avoid truncation warnings */
            char tmp[MSL*2]; /* Temporary buffer large enough for the format operation */
            snprintf(tmp, sizeof(tmp), "You hails the %s: &C%s&w\n\r", ship->name, arg2);
            tmp[sizeof(tmp)-1] = '\0'; /* Ensure null termination */
            strncpy(buf, tmp, MSL-1);
            buf[MSL-1] = '\0'; /* Ensure null termination */
            echo_to_ship(AtWhite, ship, buf);
        }
        {
            /* Use stacked buffer approach to avoid truncation warnings */
            char tmp[MSL*2]; /* Temporary buffer large enough for the format operation */
            snprintf(tmp, sizeof(tmp), "%s hails you: &C%s&w\n\r", ship->name, arg2);
            tmp[sizeof(tmp)-1] = '\0'; /* Ensure null termination */
            strncpy(buf, tmp, MSL-1);
            buf[MSL-1] = '\0'; /* Ensure null termination */
            echo_to_ship(AtWhite, target, buf);
        }
}
