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
 *                                 SWR HTML Header file                                  *
 ****************************************************************************************/
/**********************************************************************************
*Designship.c - Ship design skill for Engineers                                   *
*Coded By     - Ortluk  ortluk@hotmail.com                                        *
*Written for  - SWR 1.0  and derivitives                                          *
**********************************************************************************/
/* NOTE - If using SWFoTE you will have to change the ship classes and a couple of the ship 
   variables. I think I commented most of them.  Also if not using SWFoTE you can remove 
   the parts commented about SWFoTE */


#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <string_view>
#include <memory>
#include <format>
#include <sstream>
#include <vector>
#include <algorithm>

#include "mud.hpp"
#include "space2.hpp"
#include "installations.hpp"
#ifdef OlcHomes
#include "homes.hpp"
#endif



void write_ship_list();
void fleet_make(CharData* ch, const std::string& argument);
void smush_tilde(std::string& str);
std::shared_ptr<InstallationData> installation_from_room(int vnum);

int reserve_rooms_ship(int firstroom, int numrooms)
{
        AreaData* tarea = nullptr;
        RoomIndexData* room = nullptr;
        int i = 0;
        for (tarea = first_area; tarea; tarea = tarea->next)
                if (std::string_view(PshipArea) == std::string_view(tarea->filename))
                        break;
        for (i = firstroom; i < firstroom + numrooms; i++) {
                room = make_room(i, tarea);
                if (!room) {
                        bug("reserve_rooms: make_room failed");
                        return -1;
                }
                SetBit(room->RoomFlags, RoomSpacecraft);
        }
        fold_area(tarea, tarea->filename, true, false);
        return i;
}

int find_pvnum_block(int num_needed, const std::string& areaname)
{
        bool counting = false;
        int count = 0;
        AreaData* tarea = nullptr;
        int lrange = 0;
        int trange = 0;
        int vnum = 0;
        int startvnum = -1;
        RoomIndexData* room = nullptr;
        for (tarea = first_area; tarea; tarea = tarea->next)
                if (std::string_view(areaname) == std::string_view(tarea->filename))
                        break;
        lrange = tarea->low_r_vnum;
        trange = tarea->hi_r_vnum;
        for (vnum = lrange; vnum <= trange; vnum++) {
                room = get_room_index(vnum);
                if (room == nullptr) {
                        if (!counting) {
                                counting = true;
                                startvnum = vnum;
                        }
                        count++;
                        if (count == num_needed + 1)
                                break;
                } else if (counting) {
                        counting = false;
                        count = 0;
                        startvnum = -1;
                }
        }
        return startvnum;
}

/* this function borrowed from SWRip codebase thanx guys - comment it out if you already 
have one*/


void transship(ShipData* ship, int destination)
{
        if (!ship)
                return;
        int origShipyard = ship->shipyard;
        ship->shipyard = destination;
        ship->shipstate = ShipDocked;
        extract_ship(ship);
        ship_to_room(ship, ship->shipyard);
        ship->location = ship->shipyard;
        ship->lastdoc = ship->shipyard;
        ship->shipyard = origShipyard;
        if (ship->starsystem)
                ship_from_starsystem(ship, ship->starsystem);
        save_ship(ship);
}


CMDF do_designship(CharData* ch, const std::string& argument)
{
        std::string arg1;
        std::string arg2;
        std::string filename;
        int percentage = 0, numrooms = 0, ship_class = 0;
        bool checktool = false, checkdura = false, checkcir = false, checksuper = false;
        RoomIndexData* room = nullptr;
        ObjData* obj = nullptr;
        ShipData* ship = nullptr;
        PlanetData* planet = nullptr;
        int vnum = 0, durasteel = 0, transparisteel = 0, cost = 0, fee = 0;

        std::string rest = argument;
        arg1 = one_argument(rest, rest);
        arg2 = rest;


        switch (ch->substate)
        {
        default:

                if (!is_number(arg1.c_str()) || arg2.empty())
                {
                        send_to_char
                                ("&RUsage: &Gdesignship &C<&cnumber of rooms&C> <&cname of ship&C>&w\r\n",
                                 ch);
                        return;
                }
                numrooms = atoi(arg1.c_str());
                if (numrooms > 100 || numrooms < 1)
                {
                        send_to_char
                                ("&RNumber of rooms MUST be between 1 and 100&C&w",
                                 ch);
                        return;
                }
                for (ship = first_ship; ship; ship = ship->next)
                {
                        if (std::string_view(ship->name) == std::string_view(argument.c_str()))
                        {
                                send_to_char("&CThat ship name is already in use. Choose another.\r\n", ch);
                                return;
                        }
                }

                checktool = FALSE;
                checkdura = FALSE;
                checkcir = FALSE;
                checksuper = FALSE;

                /*
                 * used import flag to avoid adding an extra shipyard flg. 
                 * it can be changed if you don't have my cargo snippet - Ortluk 
                 * Also if you're installing in swfote you can uncomment the first if statement 
                 * and comment the second one out to use the shipyard flag instead
                 */

                if (!IsSet(ch->in_room->RoomFlags, RoomImport))
                {
                        send_to_char
                                ("You can't build that here!! Try a spaceport\r\n",
                                 ch);
                        return;
                }

/* uncomment these lines if swfote 
                if (numrooms > 100)
                   ship_class = ShipDestroyer;
                else if(numrooms > 75)
                   ship_class = ShipDreadnaught;
                else if(numrooms > 50)
                   ship_class = ShipCruiser;
                else if(numrooms > 25)
                   ship_class = ShipCorvette;
                else if(numrooms > 15)
                   ship_class = ShipFrigate;
                else if(numrooms > 5)
                   ship_class = ShipFreighter;
                else if (numrooms > 1) 
                   ship_class = ShipShuttle;
                else 
                   ship_class = ShipFighter; */

/* comment these for swfote */
                if (numrooms > 25)
                        ship_class = CapitalShip;
                else if (numrooms > 5)
                        ship_class = MidsizeShip;
                else
                        ship_class = FighterShip;

                /*
                 * these values come from  cargo v2 
                 */
                durasteel = ship_class * 150 + 100;
                transparisteel = ship_class * 20 + 10;

                if (IsSet(ch->in_room->RoomFlags, RoomInstallation))
                {
                        InstallationData *installation = installation_from_room(ch->in_room->vnum).get();
                        if (installation
                            && installation->type == ShipyardInstallation)
                                planet = installation->planet;
                }
                else
                        planet = ch->in_room->area->planet;

                if (!planet)
                {
                        send_to_char
                                ("&RAnd where do you think you're going to get the resources to build your ship?&C&w",
                                 ch);
                        return;
                }

                /*
                 * make sure the planet has the resources to build the ship 
                 */
                if (planet->resource[CargoDurasteel] < durasteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more durasteel&C&w\r\n",
                                 ch);
                        return;
                }

                if (planet->resource[CargoTransparisteel] < transparisteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more transparisteel&C&w\r\n",
                                 ch);
                        return;
                }
                cost = 10;
                if (planet->cargoimport[CargoTransparisteel] > 0)
                        cost += planet->cargoimport[CargoTransparisteel] +
                                planet->cargoimport[CargoTransparisteel] / 2;
                else if (planet->cargoexport[CargoTransparisteel] > 0)
                        cost += planet->cargoexport[CargoTransparisteel];
                else
                        cost += 10;

                if (planet->cargoimport[CargoDurasteel] > 0)
                        cost += planet->cargoimport[CargoDurasteel] +
                                planet->cargoimport[CargoDurasteel] / 2;
                else if (planet->cargoexport[CargoDurasteel] > 0)
                        cost += planet->cargoexport[CargoDurasteel];
                else
                        cost += 10;

                cost *= durasteel + transparisteel;
                fee = cost * ((ship_class * 5) / 100);
                cost += fee;
                if (ch->gold < cost)
                {
                        send_to_char
                                ("&RYou can't afford the materials to build that.\r\n",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ItemToolkit)
                                checktool = TRUE;
                        if (obj->item_type == ItemCircuit)
                                checkcir = TRUE;
                        if (obj->item_type == ItemSuperconductor)
                                checksuper = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RI'd like to see you build a ship with no tools.\r\n",
                                 ch);
                        return;
                }
                if (!checkcir)
                {
                        send_to_char
                                ("&RYou could really use a circuit to for the control systems.\r\n",
                                 ch);
                        return;
                }
                if (!checksuper)
                {
                        send_to_char
                                ("&RSuch advanced circuitry requires a superconducter to work properly.\r\n",
                                 ch);
                        return;
                }

                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_shipdesign]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the LONG Process of building a ship.\n\r",
                                 ch);
                                                act(AtPlain,
                                                        "$n takes $s tools and starts constructing a ship.\r\n",
                                                        ch, NULL, argument.c_str(), ToRoom);
                                                add_timer(ch, TimerDoFun, 35, do_designship, 1);
                                                ch->dest_buf = str_dup(arg1.c_str());
                                                ch->dest_buf_2 = str_dup(arg2.c_str());
                                                return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_shipdesign);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                {
                        bug("null ch->dest_buf2", 0);
                        return;
                }

                mudstrlcpy(arg1, static_cast<char *>(ch->dest_buf), MSL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, static_cast<char *>(ch->dest_buf_2), MSL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);

                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }
        ch->substate = SubNone;
        numrooms = atoi(arg1);

        if (numrooms > 25)
                ship_class = CapitalShip;
        else if (numrooms > 5)
                ship_class = MidsizeShip;
        else
                ship_class = FighterShip;

        /*
         * these values come from  cargo v2 
         */
        transparisteel = ship_class * 50 + 10;
        durasteel = ship_class * 100 + 100;
        planet = ch->in_room->area->planet;
        if (!planet)
        {
                send_to_char("&RYou must have been moved...I'd complain.&C&w",
                             ch);
                return;
        }

        cost = 10;
        if (planet->cargoimport[CargoTransparisteel] > 0)
                cost += planet->cargoimport[CargoTransparisteel] +
                        planet->cargoimport[CargoTransparisteel] / 2;
        else if (planet->cargoexport[CargoTransparisteel] > 0)
                cost += planet->cargoexport[CargoTransparisteel];
        else
                cost += 10;


        if (planet->cargoimport[CargoDurasteel] > 0)
                cost += planet->cargoimport[CargoDurasteel] +
                        planet->cargoimport[CargoDurasteel] / 2;
        else if (planet->cargoexport[CargoDurasteel] > 0)
                cost += planet->cargoexport[CargoDurasteel];
        else
                cost += 10;

        cost *= durasteel + transparisteel;
        fee = cost * ((ship_class * 5) / 100);
        cost += fee;
        if (ch->gold < cost)
        {
                send_to_char
                        ("&RYou can't afford the materials....Stop that THIEF!!!\r\n",
                         ch);
                return;
        }
        ch->gold -= cost;
        planet->resource[CargoTransparisteel] -= transparisteel;
        planet->resource[CargoDurasteel] -= durasteel;

        checktool = FALSE;
        checkdura = FALSE;
        checkcir = FALSE;
        checksuper = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ItemToolkit)
                        checktool = TRUE;
                if (obj->item_type == ItemDurasteel && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }
                if (obj->item_type == ItemCircuit && checkcir == FALSE)
                {
                        checkcir = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }
                if (obj->item_type == ItemSuperconductor
                    && checksuper == FALSE)
                {
                        checksuper = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }

        }

/* ok so far so good...everything is cool...try to build the ship */

        vnum = find_pvnum_block(numrooms, PshipArea);
        if (vnum < 0)
        {
                bug("player ship area out of vnums", 0);
                send_to_char("Not enough vnums report to a coder.\r\n", ch);
                return;
        }
        if (reserve_rooms_ship(vnum, numrooms) < 0)
        {
                bug("do_designship: reserve_rooms failed", 0);
                send_to_char("Couldn't build your rooms report to coder.\r\n",
                             ch);
                return;
        }
        filename = std::format("{}.pship", vnum);

        auto ship_ptr = std::make_shared<ShipData>();
        ship = ship_ptr.get();
        LINK(ship, first_ship, last_ship, next, prev);
        ship->filename = STRALLOC(filename.c_str());
        ship->name = STRALLOC(arg2.c_str());
        ship->owner = STRALLOC(ch->name);
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->type = PlayerShip;
        ship->prototype = get_protoship("generic");

/* you may want to adjust these to balance ships with your imm built ones 
   I use an array of maximum ship stats for the different classes of ships
   that's another project though */

        ship->maxenergy =
                (ship_class + 1) * (50 +
                                    (get_curr_int(ch) / 5)) *
                ch->PCData->learned[gsn_shipdesign];
        ship->energy = ship->maxenergy;
        ship->maxhull =
                (ship_class + 1 +
                 (get_curr_int(ch) / 5)) * 10 *
                ch->PCData->learned[gsn_shipdesign];
        ship->maxshield =
                (ship_class + 1) * 5 * ch->PCData->learned[gsn_shipdesign];
        ship->realspeed =
                2 * ch->PCData->learned[gsn_shipdesign] / (ship_class + 1);
        ship->hyperspeed =
                ch->PCData->learned[gsn_shipdesign] + (ship_class + 1) * 60;
        ship->lasers =
                (ship_class +
                 1) * (ch->PCData->learned[gsn_shipdesign] / 10) +
                (get_curr_int(ch) / 5);
        ship->manuever =
                ch->PCData->learned[gsn_shipdesign] * (get_curr_int(ch) / 5);
        ship->comm =
                ch->PCData->learned[gsn_shipdesign] * (get_curr_int(ch) / 5) * (ship_class + 1 );
        ship->sensor =
                ch->PCData->learned[gsn_shipdesign] * (get_curr_int(ch) / 5) * (ship_class + 1 );
        if (ship_class == FighterShip)
                ship->maxcargo = ch->PCData->learned[gsn_shipdesign] / 10;
        else
                ship->maxcargo =
                        ch->PCData->learned[gsn_shipdesign] * (ship_class +
                                                               (get_curr_int
                                                                (ch) / 5));
        if (ship_class == FighterShip)
                ship->maxbattalions = 0;
        else if (ship_class == MidsizeShip)
                ship->maxbattalions =
                        ch->PCData->learned[gsn_shipdesign] / 10;
        else if (ship_class == CapitalShip)
                ship->maxbattalions = ch->PCData->learned[gsn_shipdesign];

        ship->hull = ship->maxhull;
        ship->in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->ship_class = ship_class;
        ship->selfdestruct = STRALLOC("Installed");
        ship->selfdpass = number_range(10001, 99999);
        ship->firstroom = vnum;
        ship->lastroom = vnum + numrooms - 1;
        ship->entrance = vnum;
        ship->lastbuilt = vnum;
        if (numrooms == 1)
        {
                ship->cockpit = vnum;
                ship->navseat = vnum;
                ship->gunseat = vnum;
                ship->pilotseat = vnum;
                ship->engineroom = vnum;
                ship->coseat = vnum;
        }

        gain_exp(ch, 10000, EngineeringAbility);

/* I added this fee to balance the cost of building ships with that of selling them so as to 
   keep engis from making too much money too fast from just building and selling ships
*/
        ch_printf(ch,
                  "The planet's Government has assesed a Licence Fee of %d credits.\r\n",
                  fee);
        ch_printf(ch, "&WYou gain 10000 engineering experience.\r\n");
        learn_from_success(ch, gsn_shipdesign);
        transship(ship, ch->in_room->vnum);
                        act(AtPlain, "$n finishes building new ship, and climbs inside.", ch,
                                NULL, argument.c_str(), ToRoom);
        room = get_room_index(vnum);
        if (!room)
        {
                bug("designship..no such room", 0);
                return;
        }
                if (room->name)
                        STRFREE(room->name);
                if (numrooms > 1)
                        room->name = STRALLOC("Entrance Ramp");
                else
                        room->name = STRALLOC("Cockpit");
        fold_area(room->area, room->area->filename, TRUE, FALSE);
        save_ship(ship);
        write_ship_list();
        char_from_room(ch);
        char_to_room(ch, get_room_index(vnum));

}

CMDF do_addroom(CharData* ch, const std::string& argument)
{
        std::string arg, arg1, arg2, buf;
        RoomIndexData* room = nullptr;
        ShipData* ship = nullptr;
        bool match = false, tset = false;
        ExitData* pexit = nullptr;


		if (IsNpc(ch)) 
			return;

#ifdef OlcHomes
		if (ch->in_room && ch->in_room->home) {
			ch->in_room->home->add_room(ch, argument);
			return;
		}
#endif


        match = false;
        std::string rest = argument;
        arg = one_argument(rest, rest);
        arg1 = one_argument(rest, rest);
        arg2 = rest;

        if (arg1.empty() || arg2.empty() || arg.empty())
        {
                send_to_char
                        ("Usage: addroom <ship/installation> <direction> <type>\r\n",
                         ch);
                send_to_char
                        ("   HELP addroom for a list of room types and costs\r\n",
                         ch);
                return;
        }

/* To make them enter a decent room description. 80 characters a line * 5 lines == 400 */
        if (ch->in_room->description == nullptr || std::strlen(ch->in_room->description) < 6)
        {
                send_to_char
                        ("You must use decorateroom to make a description approximately 6 lines long.\r\n",
                         ch);
                return;
        }

        if (!str_cmp(arg.c_str(), "installation"))
        {
                buf = std::format("{} {}", arg1, arg2);
                addroominstallation(ch, buf.c_str());
                return;
        }

        ship = ship_from_room(ch->in_room->vnum);

        if (!ship)
        {
                send_to_char("You can only add rooms to your ships.\r\n", ch);
                return;
        }
        if (!check_pilot(ch, ship) || !str_cmp(ship->owner, "Public"))
        {
                send_to_char
                        ("You don't have permission to build on this ship\r\n",
                         ch);
                return;
        }
        if (argument.empty())
        {
                send_to_char("You really should name your room\r\n", ch);
                return;
        }
        if (ship->lastbuilt == ship->lastroom
            || ship->lastbuilt < ship->firstroom)
        {
                send_to_char
                        ("You have no more rooms to build on this ship\r\n",
                         ch);
                if (ship->lastbuilt < ship->firstroom)
                {
                        ship->lastbuilt = ship->lastroom;
                        save_ship(ship);
                }
                return;
        }
        room = get_room_index(ship->lastbuilt + 1);

        if (!room)
        {
                bug("addroom: no such room", 0);
                return;
        }

                        if (!str_cmp(arg1.c_str(), "n") || !str_cmp(arg1.c_str(), "north") ||
                                !str_cmp(arg1.c_str(), "s") || !str_cmp(arg1.c_str(), "south") ||
                                !str_cmp(arg1.c_str(), "e") || !str_cmp(arg1.c_str(), "east") ||
                                !str_cmp(arg1.c_str(), "w") || !str_cmp(arg1.c_str(), "west") ||
                                !str_cmp(arg1.c_str(), "sw") || !str_cmp(arg1.c_str(), "southwest") ||
                                !str_cmp(arg1.c_str(), "nw") || !str_cmp(arg1.c_str(), "northwest") ||
                                !str_cmp(arg1.c_str(), "se") || !str_cmp(arg1.c_str(), "southeast") ||
                                !str_cmp(arg1.c_str(), "ne") || !str_cmp(arg1.c_str(), "northeast") ||
                                !str_cmp(arg1.c_str(), "u") || !str_cmp(arg1.c_str(), "up") ||
                                !str_cmp(arg1.c_str(), "d") || !str_cmp(arg1.c_str(), "down"))
        {
                if (get_exit(ch->in_room, get_dir(arg1)))
                {
                        send_to_char
                                ("There is already an exit in that direction\r\n",
                                 ch);
                        return;
                }
        }
        else
        {
                send_to_char("Not a Valid direction\r\n", ch);
                return;
        }

        if (!str_cmp(arg2, "cockpit"))
        {
                if (ship->ship_class > MidsizeShip)
                {
                        send_to_char
                                ("Ships of this size don't have cockpits\r\n",
                                 ch);
                        return;
                }
                if (ch->gold < 3000)
                {
                        send_to_char
                                ("You haven't got the money for a cockpit...it's gonna be hard to fly this one\r\n",
                                 ch);
                        return;
                }

                ship->pilotseat = room->vnum;
                ship->coseat = room->vnum;
                ship->navseat = room->vnum;
                ship->gunseat = room->vnum;
                match = TRUE;
                ch->gold -= 3000;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Cockpit");
        }
        if (!str_cmp(arg2, "pilot"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                ship->pilotseat = room->vnum;
                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Pilot's Station");

        }
        if (!str_cmp(arg2, "nav"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                ship->navseat = room->vnum;
                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Navigator's Station");
        }
        if (!str_cmp(arg2, "gun"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                match = TRUE;
                ship->gunseat = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Gunner's Station");
        }
        if (!str_cmp(arg2, "copilot"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                match = TRUE;
                ship->coseat = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Co-Pilot's Station");
        }
        if (!str_cmp(arg2, "engine"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                match = TRUE;
                ship->engineroom = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("The Engine Room");
        }
        if (!str_cmp(arg2, "turret"))
        {
                if (ship->ship_class == FighterShip)
                {
                        send_to_char
                                ("starfighters are too small to have turrets\r\n",
                                 ch);
                        return;
                }
                if (ship->turret1 == 0)
                {
                        tset = TRUE;
                        ship->turret1 = room->vnum;
                }
                else if (ship->turret2 == 0)
                {
                        tset = TRUE;
                        ship->turret2 = room->vnum;
                }
                if (!tset)
                {
                        send_to_char("There is no more room for turrets\r\n",
                                     ch);
                        return;
                }
                if (ch->gold < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 10000;
                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Turret");
        }
        if (!str_cmp(arg2, "hanger"))
        {
                if (ship->ship_class < MidsizeShip)
                {
                        send_to_char("Starfighters don't have hangers!!\r\n",
                                     ch);
                        return;
                }
                if (ship->hanger != 0)
                {
                        send_to_char("sorry only one hanger per ship\r\n",
                                     ch);
                        return;
                }
                if (ch->gold < 5000)
                {
                        send_to_char
                                ("You haven't got the money for that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 5000;

                match = TRUE;
                ship->hanger = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("The Hanger");
        }
        if (!str_cmp(arg2, "workshop"))
        {
                if (ch->gold < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 10000;
                match = TRUE;
                SetBit(room->RoomFlags, RoomFactory);
                SetBit(room->RoomFlags, RoomRefinery);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Workshop");
        }
        if (!str_cmp(arg2, "bacta"))
        {
                if (ship->ship_class == FighterShip)
                {
                        send_to_char
                                ("starfighters are too small to have turrets\r\n",
                                 ch);
                        return;
                }

                if (ch->gold < 25000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 25000;
                match = TRUE;
                SetBit(room->RoomFlags, RoomBacta);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Bacta Tank");
        }
        if (!str_cmp(arg2, "meditate"))
        {
                if (ch->gold < 100000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 100000;

                match = TRUE;
                SetBit(room->RoomFlags, RoomSilence);
                SetBit(room->RoomFlags, RoomSafe);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Quiet Meditation Chamber");
        }
        if (!str_cmp(arg2, "hotel"))
        {
                if (ch->gold < 5000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 5000;

                match = TRUE;
                SetBit(room->RoomFlags, RoomHotel);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("The Passenger's Lounge");
        }
        if (!str_cmp(arg2, "home"))
        {
                if (ch->gold < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 10000;

                match = TRUE;
                SetBit(room->RoomFlags, RoomEmptyHome);
                SetBit(room->RoomFlags, RoomHotel);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("An Empty Apartment");

        }
        if (!str_cmp(arg2, "turbolift"))
        {
                if (ch->gold < 500)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 500;

                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Turbolift");
        }
        if (!str_cmp(arg2, "corridor"))
        {
                if (ch->gold < 500)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 500;

                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Corridor");
        }

        if (!match)
        {
                send_to_char("&RNo such room type\r\n", ch);
                return;
        }
                if (!str_cmp(arg1.c_str(), "n") || !str_cmp(arg1.c_str(), "north") ||
                        !str_cmp(arg1.c_str(), "s") || !str_cmp(arg1.c_str(), "south") ||
                        !str_cmp(arg1.c_str(), "e") || !str_cmp(arg1.c_str(), "east") ||
                        !str_cmp(arg1.c_str(), "w") || !str_cmp(arg1.c_str(), "west") ||
                        !str_cmp(arg1.c_str(), "sw") || !str_cmp(arg1.c_str(), "southwest") ||
                        !str_cmp(arg1.c_str(), "nw") || !str_cmp(arg1.c_str(), "northwest") ||
                        !str_cmp(arg1.c_str(), "se") || !str_cmp(arg1.c_str(), "southeast") ||
                        !str_cmp(arg1.c_str(), "ne") || !str_cmp(arg1.c_str(), "northeast") ||
                        !str_cmp(arg1.c_str(), "u") || !str_cmp(arg1.c_str(), "up") ||
                        !str_cmp(arg1.c_str(), "d") || !str_cmp(arg1.c_str(), "down"))
        {
                if (get_exit(ch->in_room, get_dir(arg1)))
                {
                        send_to_char
                                ("There is already an exit in that direction\r\n",
                                 ch);
                        return;
                }
                else
                {
                        pexit = make_exit(ch->in_room, room, get_dir(arg1));
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                        pexit = make_exit(room, ch->in_room,
                                          rev_dir[get_dir(arg1)]);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }
        }

        else
        {
                send_to_char("Not a Valid direction\r\n", ch);
                return;
        }
        ship->lastbuilt = room->vnum;
        save_ship(ship);
        fold_area(room->area, room->area->filename, TRUE, FALSE);
}

CMDF do_decorate(CharData* ch, const std::string& argument)
{
        ShipData *ship;
        RoomIndexData *room;
        AreaData *tarea;
        int tmplvl = 0;
        std::string arg1;
        std::string buf;
        InstallationData* installation = nullptr;

#ifdef OlcHomes
		if (ch->in_room && ch->in_room->home) {
			ch->in_room->home->decorate_room(ch, argument);
			return;
		}
#endif

        std::string rest = argument;
        arg1 = one_argument(rest, rest);

        if (!arg1.empty() && arg1 == "name")
        {
                room = ch->in_room;
                if (!room)
                {
                        bug("do_decorate: Trying to decorate NULL room", 0);
                        return;
                }

                ship = ship_from_room(room->vnum);
                installation = installation_from_room(room->vnum);
                if (!ship && !installation && !IsImmortal(ch))
                {
                        send_to_char
                                ("&RYou can currently only do this on ships an in installations.\r\n&C&w",
                                 ch);
                        return;
                }

                if (ship && !installation)
                        if (!check_pilot(ch, ship))
                        {
                                send_to_char
                                        ("&RTry asking the owner first&C&w\r\n",
                                         ch);
                                return;
                        }
                if (ship && !installation)
                        if (ship->type != PlayerShip)
                        {
                                send_to_char
                                        ("Currently you can only decorate custom ships...\r\n",
                                         ch);
                                send_to_char
                                        ("Petition the imms for a change of policy if ya don't like it\r\n",
                                         ch);
                                return;
                        }
                if (!ship && installation)
                        if (IsNpc(ch) || !ch->PCData || !ch->PCData->clan
                            || ch->PCData->clan != installation->clan)
                        {
                                send_to_char
                                        ("You must be in this installations clan to do that.\r\n",
                                         ch);
                                return;
                        }

                if (!ship && installation)
                {
                        if (installation
                            &&
                            ((ch->PCData && ch->PCData->bestowments
                              && is_name("installations",
                                         ch->PCData->bestowments))
                             || !str_cmp(ch->name, ch->PCData->clan->leader)
                             || !str_cmp(ch->name, ch->PCData->clan->number1)
                             || !str_cmp(ch->name,
                                         ch->PCData->clan->number2)))
                                ;
                        else
                        {
                                send_to_char
                                        ("Your clan has not empowered you with that ability.\n\r",
                                         ch);
                                return;
                        }
                }

/*possible security hole...temporarily sets the char to level 110 so they can use redit 
  desc..might want to either figure out another way to do this or make sure your dangerous
  imm commands check to make sure it's not coming from redit */

                buf = std::format("name {}", argument);
                tarea = room->area;
                tmplvl = ch->top_level;
                ch->top_level = 152;
                do_redit(ch, buf.c_str());
                ch->top_level = tmplvl;
                fold_area(tarea, tarea->filename, FALSE, FALSE);
                return;

        }

        if (arg1[0] != '\0' && !str_cmp(arg1, "ambiance"))
        {
                MProgData *mprog, *mprg;
                int       mptype, len;
                char      buf[MSL];

                if (strlen(arg1) > 80)
                {
                        send_to_char
                                ("Your line cannot be more than 80 characters long",
                                 ch);
                        return;
                }

                mprog = ch->in_room->mudprogs;
                mptype = get_mpflag("rand");

                if (mptype == -1)
                {
                        bug("do_decorateroom : %s in %d\n\r\tUnknown program type.\n\r", ch->name, ch->in_room->vnum);
                        return;
                }

                if (mprog)
                        for (; mprog->next; mprog = mprog->next);
                CREATE(mprg, MProgData, 1);

                if (mprog)
                        mprog->next = mprg;
                else
                        ch->in_room->mudprogs = mprg;

                ch->in_room->progtypes |= (1 << mptype);
                mprg->type = 1 << mptype;
                mprg->arglist = STRALLOC("100");

                if (mprg->comlist)
                        STRFREE(mprg->comlist);

                snprintf(buf, MSL, "mpecho %s", argument);
                smush_tilde(buf);
                len = strlen(buf);
                if (buf && buf[len - 1] == '~')
                        buf[len - 1] = '\0';
                else
                        mudstrlcat(buf, "\n\r", MSL);
                mprg->comlist = STRALLOC(buf);
                return;
        }

        room = ch->in_room;
        room = ch->in_room;
        if (!room)
        {
                bug("do_decorate: Trying to decorate NULL room", 0);
                return;
        }

        ship = ship_from_room(room->vnum);
        installation = installation_from_room(room->vnum);
        if (!ship && !installation)
        {
                send_to_char
                        ("&RYou can currently only do this on ships an in installations.\r\n&C&w",
                         ch);
                return;
        }

        if (ship && !installation)
                if (!check_pilot(ch, ship))
                {
                        send_to_char("&RTry asking the owner first&C&w\r\n",
                                     ch);
                        return;
                }
        if (ship && !installation)
                if (ship->type != PlayerShip)
                {
                        send_to_char
                                ("Currently you can only decorate custom ships...\r\n",
                                 ch);
                        send_to_char
                                ("Petition the imms for a change of policy if ya don't like it\r\n",
                                 ch);
                        return;
                }
        if (!ship && installation)
                if (IsNpc(ch) || !ch->PCData || !ch->PCData->clan
                    || ch->PCData->clan != installation->clan)
                {
                        send_to_char
                                ("You must be in this installations clan to do that.\r\n",
                                 ch);
                        return;
                }

        if (!ship && installation)
        {
                if (installation && ((ch->PCData && ch->PCData->bestowments
                                      && is_name("installations",
                                                 ch->PCData->bestowments))
                                     || !str_cmp(ch->name,
                                                 ch->PCData->clan->leader)
                                     || !str_cmp(ch->name,
                                                 ch->PCData->clan->number1)
                                     || !str_cmp(ch->name,
                                                 ch->PCData->clan->number2)))
                        ;
                else
                {
                        send_to_char
                                ("Your clan has not empowered you with that ability.\n\r",
                                 ch);
                        return;
                }
        }
/*possible security hole...temporarily sets the char to level 110 so they can use redit 
  desc..might want to either figure out another way to do this or make sure your dangerous
  imm commands check to make sure it's not coming from redit */

        tarea = room->area;
        tmplvl = ch->top_level;
        ch->top_level = 152;
        do_redit(ch, std::string("desc"));
        ch->top_level = tmplvl;
        fold_area(tarea, tarea->filename, FALSE, FALSE);
        return;
}

/* this was added by Request from the implementor of the mud i player tested it on
   gives engis an alternative to selling the ships to keep the vnums free and to 
   make some of the expense of building it back. 
   Note - You may also want to adjust do_sellship to severely cut the price on 
          player ships to give them a reson to recycle rather than sell.
*/

CMDF do_recycle(CharData* ch, const std::string& argument)
{
        long      price;
        ShipData *ship;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: recycle <ship>\r\n", ch);
                return;
        }

        ship = ship_in_room(ch->in_room, argument.c_str());
        if (!ship)
        {
                act(AtPlain, "I see no $T here.", ch, NULL, argument,
                    ToChar);
                return;
        }

        if (str_cmp(ship->owner, ch->name) && !IsImmortal(ch))
        {
                send_to_char("&RThat isn't your ship!", ch);
                return;
        }

        if (ship->type != PlayerShip)
        {
                send_to_char("You can only recycle custom built ships\r\n",
                             ch);
                return;
        }

        price = get_ship_value(ship) / 2;

        ch->gold += price;
        ch_printf(ch,
                  "&GYou receive %ld credits from recycling your ship.\n\r",
                  price);
        send_to_char
                ("\r\nSeveral heavy droids chop up and carry off your ship.\r\n",
                 ch);

        act(AtPlain,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, ToRoom);
        act(AtPlain, "Several heavy droids chop up and carry off a ship.",
            ch, NULL, argument, ToRoom);
        transship(ship, 45);
        really_destroy_ship(ship);
}



CMDF do_fleet(CharData* ch, const std::string& argument)
{
        std::string arg;
        std::string rest = argument;
        arg = one_argument(rest, rest);

                if (arg == "make")
                        fleet_make(ch, rest);
/*	else if ( !str_cmp(arg, "addship" ) )
		fleet_addship(ch, argument);
	else if ( !str_cmp(arg, "remship" ) )
		fleet_remship(ch, argument);
	else if ( !str_cmp(arg, "commandship" ) )
		fleet_commandship(ch, argument);
	else if ( !str_cmp(arg, "commandfleet" ) )
		fleet_commandfleet(ch, argument);
	else if ( !str_cmp(arg, "disbandfleet" ) )
		fleet_disbandfleet(ch, argument);	*/
}

void fleet_make(CharData* ch, const std::string& argument)
{
        std::string arg1, arg2, filename;
        int percentage = 0, ship_class = 0;
        ShipData* ship = nullptr;
        PlanetData* planet = nullptr;
        SpaceData* starsystem = nullptr;
        ClanData* clan = nullptr;
        int durasteel = 0, transparisteel = 0, cost = 0;
        std::string rest = argument;
        arg1 = one_argument(rest, rest);
        arg2 = rest;



        switch (ch->substate)
        {
        default:


                if (arg1[0] == '\0' || arg2[0] == '\0')
                {
                        send_to_char
                                ("&RSyntax: &GFleet make <type> <name>\r\n",
                                 ch);
                        send_to_char
                                ("&RSyntax: &GTypes: Fighter, Mid, Cap&w\r\n",
                                 ch);
                        return;
                }

                clan = ch->PCData->clan;
                if (!clan)
                {
                        send_to_char("You are not in a clan.\n\r", ch);
                        return;
                }



                for (ship = first_ship; ship; ship = ship->next)
                {
                        if (!str_cmp(ship->name, arg2.c_str()))
                        {
                                send_to_char
                                        ("&CThat ship name is already in use. Choose another.\r\n",
                                         ch);
                                return;
                        }
                }


                /*
                 * used import flag to avoid adding an extra shipyard flg. 
                 * it can be changed if you don't have my cargo snippet - Ortluk 
                 * Also if you're installing in swfote you can uncomment the first if statement 
                 * and comment the second one out to use the shipyard flag instead
                 */

                if (!IsSet(ch->in_room->RoomFlags, RoomImport))
                {
                        send_to_char
                                ("You can't do that here!! Try a spaceport\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg1, "cap"))
                        ship_class = CapitalShip;
                else if (!str_cmp(arg1, "mid"))
                        ship_class = MidsizeShip;
                else if (!str_cmp(arg1, "fighter"))
                        ship_class = FighterShip;
                else
                {
                        send_to_char("&GThats not a ship type.\n\r", ch);
                        return;
                }
                durasteel = ship_class * 15 + 10;
                transparisteel = ship_class * 2 + 1;
                if (IsSet(ch->in_room->RoomFlags, RoomInstallation))
                {
                        InstallationData *installation =
                                installation_from_room(ch->in_room->vnum);
                        if (installation
                            && installation->type == ShipyardInstallation)
                                planet = installation->planet;
                }
                else
                        planet = ch->in_room->area->planet;
                if (!planet)
                {
                        send_to_char
                                ("&RAnd where do you think you're going to get the resources to build your ship?&C&w",
                                 ch);
                        return;
                }

                if (planet->governed_by && planet->governed_by != clan)
                {
                        send_to_char
                                ("You do not belong to this planets government, how can you constuct ships here?",
                                 ch);
                        return;
                }

                /*
                 * make sure the planet has the resources to build the ship 
                 */
                if (planet->resource[CargoDurasteel] < durasteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more durasteel&C&w\r\n",
                                 ch);
                        return;
                }

                if (planet->resource[CargoTransparisteel] < transparisteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more transparisteel&C&w\r\n",
                                 ch);
                        return;
                }
                cost = 10;
                if (planet->cargoimport[CargoTransparisteel] > 0)
                        cost += planet->cargoimport[CargoTransparisteel] +
                                planet->cargoimport[CargoTransparisteel] / 2;
                else if (planet->cargoexport[CargoTransparisteel] > 0)
                        cost += planet->cargoexport[CargoTransparisteel];
                else
                        cost += 10;

                if (planet->cargoimport[CargoDurasteel] > 0)
                        cost += planet->cargoimport[CargoDurasteel] +
                                planet->cargoimport[CargoDurasteel] / 2;
                else if (planet->cargoexport[CargoDurasteel] > 0)
                        cost += planet->cargoexport[CargoDurasteel];
                else
                        cost += 10;

                cost *= durasteel + transparisteel;
                if (clan->funds < cost)
                {
                        send_to_char
                                ("&RYour government can't afford the materials to build that.\r\n",
                                 ch);
                        return;
                }



                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_fleet_command1]);
                if (number_percent() < percentage)
                {
                        send_to_char("&GYou begin to launch a ship.\n\r", ch);
                        add_timer(ch, TimerDoFun, 45, fleet_make, 1);
                        ch->dest_buf = str_dup(arg1);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to launch the ship.\n\r",
                         ch);
                learn_from_failure(ch, gsn_fleet_command1);
                return;

        case 1:

                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                {
                        bug("null ch->dest_buf2", 0);
                        return;
                }

                mudstrlcpy(arg1, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SubTimerDoAbort:

                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;

        /*
         * these values come from  cargo v2 
         */

        if (!str_cmp(arg1, "cap"))
                ship_class = CapitalShip;
        else if (!str_cmp(arg1, "mid"))
                ship_class = MidsizeShip;
        else
                ship_class = FighterShip;

        transparisteel = ship_class * 5 + 1;
        durasteel = ship_class * 10 + 10;
        planet = ch->in_room->area->planet;
        starsystem = planet->starsystem;
        if (!planet)
        {
                send_to_char
                        ("&RYou must have been moved...I'd complain about the planet.&C&w",
                         ch);
                return;
        }

        if (!starsystem)
        {
                send_to_char
                        ("&RYou must have been moved...I'd complain about the starsystem.&C&w",
                         ch);
                return;
        }


        clan = ch->PCData->clan;
        if (!clan)
        {
                send_to_char("You are not in a clan.\n\r", ch);
                return;
        }

        cost = 10;
        if (planet->cargoimport[CargoTransparisteel] > 0)
                cost += planet->cargoimport[CargoTransparisteel] +
                        planet->cargoimport[CargoTransparisteel] / 2;
        else if (planet->cargoexport[CargoTransparisteel] > 0)
                cost += planet->cargoexport[CargoTransparisteel];
        else
                cost += 10;


        if (planet->cargoimport[CargoDurasteel] > 0)
                cost += planet->cargoimport[CargoDurasteel] +
                        planet->cargoimport[CargoDurasteel] / 2;
        else if (planet->cargoexport[CargoDurasteel] > 0)
                cost += planet->cargoexport[CargoDurasteel];
        else
                cost += 10;

        cost *= durasteel + transparisteel;
        if (clan->funds < cost)
        {
                send_to_char
                        ("&RYour clan can't afford the materials....Stop that THIEF!!!\r\n",
                         ch);
                return;
        }
        clan->funds -= cost;
        planet->resource[CargoTransparisteel] -= transparisteel;
        planet->resource[CargoDurasteel] -= durasteel;

        filename = std::format("{}.mship", arg2);

        ship = new ShipData();
        LINK(ship, first_ship, last_ship, next, prev);
        ship->filename = STRALLOC(smash_space(filename).c_str());
        ship->name = STRALLOC(arg2.c_str());
        ship->owner = clan->name;
        ship->clan = clan;
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->type = ClanMobShip;
        ship->selfdestruct = STRALLOC("Installed");
        ship->selfdpass = number_range(10001, 99999);

/* you may want to adjust these to balance ships with your imm built ones 
   I use an array of maximum ship stats for the different classes of ships
   that's another project though */

        ship->maxenergy =
                (ship_class +
                 1) * 50 * ch->PCData->learned[gsn_fleet_command1];
        ship->energy = ship->maxenergy;
        ship->maxhull =
                (ship_class +
                 1) * 10 * ch->PCData->learned[gsn_fleet_command1];
        ship->maxshield =
                (ship_class +
                 1) * 5 * ch->PCData->learned[gsn_fleet_command1];
        ship->realspeed =
                2 * ch->PCData->learned[gsn_fleet_command1] / (ship_class +
                                                               1);
        ship->hyperspeed =
                ch->PCData->learned[gsn_fleet_command1] + (ship_class +
                                                           1) * 20;
        ship->lasers =
                (ship_class +
                 1) * (ch->PCData->learned[gsn_fleet_command1] / 20);
        ship->manuever =
                ch->PCData->learned[gsn_fleet_command1] * 2 / (ship_class +
                                                               1);
        ship->comm =
                ch->PCData->learned[gsn_fleet_command1] * 2 / (ship_class +
                                                               1);
        ship->sensor =
                ch->PCData->learned[gsn_fleet_command1] * 2 / (ship_class +
                                                               1);


        ship->description = "";
        ship->hull = ship->maxhull;
        ship->in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->ship_class = ship_class;
        ship->home = starsystem->name;
        ship->vx = number_range(-5000, 5000);
        ship->vy = number_range(-5000, 5000);
        ship->vz = number_range(-5000, 5000);
        ship->hx = 1;
        ship->hy = 1;
        ship->hz = 1;
        ship->currspeed = 0;


        send_to_char
                ("You complete your work constructing part of your fleet\n\r",
                 ch);
        gain_exp(ch, 100000, PilotingAbility);
        ship_to_starsystem(ship, starsystem_from_name(ship->home));
        ch_printf(ch, "&WYou gain 100000 engineering experience.\r\n");
        learn_from_success(ch, gsn_fleet_command1);

        act(AtPlain, "$n finishes launching the new ship.", ch,
            NULL, NULL, ToRoom);
        save_ship(ship);
        write_ship_list();
}

CMDF do_modifyship(CharData* ch, const std::string& argument)
{
        std::string arg, arg1, arg2;
        ShipData* ship = nullptr;
        bool match = false, checktool = false;
        int percentage = 0;
        ObjData* obj = nullptr;
        std::string rest = argument;
        arg = one_argument(rest, rest);
        arg1 = one_argument(rest, rest);
        arg2 = rest;

        switch (ch->substate)
        {
        default:
                checktool = false;
                if (arg.empty() || arg1.empty() || arg2.empty())
                {
                        send_to_char("Usage: modifyship <ship> <add> <remove>\r\n", ch);
                        send_to_char("Options: Speed manuever hyperspeed cargo battalions\r\n", ch);
                        send_to_char("Options: lasers ions missiles rockets torpedos\r\n", ch);
                        send_to_char("Options: tractors chaff shields hull energy\r\n", ch);
                        send_to_char("Options: stealth cloak interdictor\r\n", ch);
                        return;
                }
                ship = ship_in_room(ch->in_room, arg);
                if (!ship)
                {
                        send_to_char("That ship is not here.\r\n", ch);
                        return;
                }
                if (!check_pilot(ch, ship) || !str_cmp(ship->owner, "public"))
                {
                        send_to_char
                                ("You don't have permission to modify this ship\r\n",
                                 ch);
                        return;
                }
                if (str_cmp(arg1, "Speed") && str_cmp(arg1, "cargo")
                    && str_cmp(arg1, "battalions") && str_cmp(arg1, "lasers")
                    && str_cmp(arg1, "ions") && str_cmp(arg1, "missiles")
                    && str_cmp(arg1, "torpedos")
                    && str_cmp(arg1, "hyperspeed") && str_cmp(arg1, "hull")
                    && str_cmp(arg1, "shields") && str_cmp(arg1, "cloak")
                    && str_cmp(arg1, "stealth") && str_cmp(arg1, "energy")
                    && str_cmp(arg1, "tractor") && str_cmp(arg1, "rockets")
                    && str_cmp(arg1, "selfdestruct") && str_cmp(arg1, "chaff")
                    && str_cmp(arg1, "manuever")
                    && str_cmp(arg1, "interdictor"))
                {
                        send_to_char
                                ("That is not a Valid ship field to modify. Try help modifyship\r\n",
                                 ch);
                        return;
                }
                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ItemToolkit)
                                checktool = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RI'd like to see you modify a ship with no tools.\r\n",
                                 ch);
                        return;
                }

                percentage =
                        IsNpc(ch) ? ch->top_level : (int) (ch->PCData->
                                                            learned
                                                            [gsn_modifyship]);


                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin to tinker with the ship.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n takes $s tools and starts to work on a ship.\r\n",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 35, do_modifyship, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg1);
                        ch->dest_buf_3 = str_dup(arg2);
                        return;
                }

                send_to_char("&RYou can't figure out where to begin.\n\r",
                             ch);
                learn_from_failure(ch, gsn_modifyship);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                {
                        bug("null ch->dest_buf2", 0);
                        return;
                }
                if (!ch->dest_buf_3)
                {
                        bug("null ch->dest_buf3", 0);
                        return;
                }
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg1, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                mudstrlcpy(arg2, (char *) ch->dest_buf_3, MIL);
                DISPOSE(ch->dest_buf_3);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                DISPOSE(ch->dest_buf_3);

                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }
        ch->substate = SubNone;

        checktool = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ItemToolkit)
                        checktool = TRUE;
        }

        percentage =
                IsNpc(ch) ? ch->top_level : (int) (ch->PCData->
                                                    learned[gsn_modifyship]);

        if (number_percent() > percentage * 2 || (!checktool))
        {
                send_to_char
                        ("&RYou stand back and gather your diagnostic tools.\n\r",
                         ch);
                send_to_char
                        ("&RWith your tools, you realize that you just wasted a lot of time.\n\r",
                         ch);
                learn_from_failure(ch, gsn_modifyship);
                return;
        }

        if ((ship = ship_in_room(ch->in_room, arg)) == NULL)
        {
                send_to_char
                        ("The ship you were working on seems to have moved.\r\n",
                         ch);
                return;
        }

        if (!str_cmp(arg1, "Speed"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ships Speed\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ships Speed\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ships Speed\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->realspeed >= 255)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for Speed enhancements\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->realspeed >= 200)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for Speed enhancements\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->realspeed >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for Speed enhancements\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for Speed\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, battalions, manuever, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to convert into engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to convert into engines\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->realspeed += 25;
                send_to_char("Ships Speed increased by 25.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "cargo"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxcargo >= 10)
                        {
                                send_to_char
                                        ("This ship has reached its maximum cargo capacity.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxcargo >= 800)
                        {
                                send_to_char
                                        ("This ship has reached its maximum cargo capacity.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->maxcargo >= 5000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum cargo capacity.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "Speed")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for cargo\r\n",
                                 ch);
                        send_to_char
                                ("Options are: Speed, battalions, manuever, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "Speed"))
                {
                        if (ship->realspeed < 25)
                        {
                                send_to_char
                                        ("There are not enough engines to free up any cargo space..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships Speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to free up any cargo space.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to free up any cargo space.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to free up any cargo space.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->maxcargo += 50;
                send_to_char("Ships cargo space increased by 50.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }


        if (!str_cmp(arg1, "battalions"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxbattalions >= 0)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for battalions.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxbattalions >= 10)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for battalions.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->maxbattalions >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for battalions.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "Speed"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for battalion holds\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, Speed, manuever, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to fit more battalion holds..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "Speed"))
                {
                        if (ship->realspeed < 25)
                        {
                                send_to_char
                                        ("There are not enough engines to add a battalion hold.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships Speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to fit more battalion holds.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to fit more battalion holds.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->maxbattalions += 1;
                send_to_char("Ships battalion holds increased by 1.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }




        if (!str_cmp(arg1, "manuever"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->manuever >= 255)
                        {
                                send_to_char
                                        ("This ship has reached its maximum manueverability.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->manuever >= 200)
                        {
                                send_to_char
                                        ("This ship has reached its maximum manueverability.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->manuever >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum manueverability.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "Speed"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for manueverability\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, battalions, Speed, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "Speed"))
                {
                        if (ship->realspeed < 25)
                        {
                                send_to_char
                                        ("There are not enough engines to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships Speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->manuever += 25;
                send_to_char("Ships manueverability increased by 25.\r\n",
                             ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "hyperspeed"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->hyperspeed >= 255)
                        {
                                send_to_char
                                        ("This ship has reached its maximum hyperspeed.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->realspeed >= 200)
                        {
                                send_to_char
                                        ("This ship has reached its maximum hyperspeed.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->realspeed >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum hyperspeed.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "Speed")))
                {
                        send_to_char
                                ("You may not substitute that for hyperspeed\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, battalions, manuever, Speed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to convert into engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "Speed"))
                {
                        if (ship->realspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough engines to convert into hyperspace drives.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 26)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships Speed reduced by 25\r\n",
                                             ch);
                        }
                }

                ship->hyperspeed += 25;
                send_to_char("Ships hyperspeed increased by 25.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "lasers"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->lasers >= 6)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of lasers.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->lasers >= 15)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of lasers.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->lasers >= 30)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of lasers.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for lasers\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, rockets, torpedos, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff > 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with lasers\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char("Ships missiles by 6\r\n", ch);
                        }
                }

                ship->lasers += 1;
                send_to_char("Ships lasers increased by 1.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "ions"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->ions >= 6)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of ion cannons.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->ions >= 15)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of ion cannons.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->ions >= 30)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of ion cannons.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "lasers")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for ion cannons\r\n",
                                 ch);
                        send_to_char
                                ("Options are: lasers, rockets, torpedos, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not any lasers to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with ion cannons\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char
                                        ("Ships missiles reduced by 6\r\n",
                                         ch);
                        }
                }

                ship->ions += 1;
                send_to_char("Ships ion cannons increased by 1.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                ch->gold -= 30000;
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "missiles"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxmissiles >= 10)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of missiles.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxmissiles >= 30)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of missiles.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->maxmissiles >= 90)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of missiles.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "lasers")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for missiles\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, rockets, torpedos, lasers, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not any lasers to replace with missiles\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                ship->maxmissiles += 6;
                send_to_char("Ships missiles increased by 6.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "torpedos"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxtorpedos >= 5)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of torpedos.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxtorpedos >= 15)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of torpedos.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->maxtorpedos >= 45)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of torpedos.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for torpedos\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, rockets, lasers, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not any lasers to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with torpedos\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char
                                        ("Ships missiles reduced by 6\r\n",
                                         ch);
                        }
                }

                ship->maxtorpedos += 4;
                send_to_char("Ships torpedos increased by 4.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "rockets"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxrockets >= 2)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of rockets.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->rockets >= 6)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of rockets.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->rockets >= 18)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of rockets.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for rockets\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, lasers, torpedos, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }


                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not enough lasers to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with rockets\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char("Ships missiles by 6\r\n", ch);
                        }
                }

                ship->maxrockets += 2;
                send_to_char("Ships rockets increased by 2.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "chaff"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxchaff >= 5)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of chaff pods.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxchaff >= 20)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of chaff pods.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->lasers >= 50)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of chaff pods.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles"))
                    && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for chaff pods\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, lasers, rockets, torpedos, missiles, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with chaff pods..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }


                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not enough lasers to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with rockets\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char
                                        ("Ships missiles reduced by 6\r\n",
                                         ch);
                        }
                }

                ship->maxchaff += 1;
                send_to_char("Ships chaff increased by 1.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "tractor"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        send_to_char
                                ("Fighter ships cannot use tractor beams.\r\n",
                                 ch);
                        return;
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->tractorbeam >= 1)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number tractor beams.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->tractorbeam >= 5)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of tractor beams.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "rockets")))
                {
                        send_to_char
                                ("You may not substitute that for tractor beams\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, lasers, rockets, torpedos, missiles, chaff\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with tractor beams..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char
                                        ("Ships rockets beams reduced by 2\r\n",
                                         ch);
                        }
                }


                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not enough lasers to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with rockets\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char("Ships missiles by 6\r\n", ch);
                        }
                }

                ship->tractorbeam += 1;
                send_to_char("Ships tractpr beams increased by 1.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }


        if (!str_cmp(arg1, "hull"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxhull >= 1000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum armour.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxhull >= 3000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum armour.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->maxhull >= 10000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum armour.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "shields")) && (str_cmp(arg2, "energy")))
                {
                        send_to_char
                                ("You may not substitute that for hull armour\r\n",
                                 ch);
                        send_to_char("Options are: shields, energy\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with hull armour.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield >= 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with hull armour.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships fuel cells reduced by 100\r\n",
                                         ch);
                        }
                }

                ship->maxhull += 100;
                send_to_char("Ships hull armour increased by 100.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "shields"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxshield >= 300)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of shield generators.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxshield >= 900)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of shield generators.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->maxshield >= 3000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of shield generators.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "energy")))
                {
                        send_to_char
                                ("You may not substitute that for shield generators\r\n",
                                 ch);
                        send_to_char("Options are: hull, energy\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull Armor to replace with shield generators.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with shield generators.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships fuel cells reduced by 100\r\n",
                                         ch);
                        }
                }

                ship->maxshield += 10;
                send_to_char("Ships shield generators increased by 10.\r\n",
                             ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                ch->gold -= 30000;
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "energy"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->maxenergy >= 5000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number fuel cells.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->maxenergy >= 15000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of fuel cells.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->maxenergy >= 36000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of fuel cells.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields")))
                {
                        send_to_char
                                ("You may not substitute that for fuel cells\r\n",
                                 ch);
                        send_to_char("Options are: hull, shields\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull Armor to replace with fuel cells.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with fuel cells.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                ship->maxenergy += 100;
                send_to_char("Ships fuel cells increased by 100.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "stealth"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 500000)
                        {
                                send_to_char
                                        ("You need 500000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 1000000)
                        {
                                send_to_char
                                        ("You need 1000000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->stealth >= 1)
                        {
                                send_to_char
                                        ("This ship already has a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->stealth >= 1)
                        {
                                send_to_char
                                        ("This ship already has a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->stealth >= 1 && str_cmp(arg2, "remove"))
                        {
                                send_to_char
                                        ("This ship already has a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields"))
                    && (str_cmp(arg2, "energy")) && (str_cmp(arg2, "cargo"))
                    && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "Speed")) && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed"))
                    && (str_cmp(arg2, "remove")))

                {
                        send_to_char
                                ("You may not substitute that for a stealth drive\r\n",
                                 ch);
                        send_to_char
                                ("Options are: hull, shields, energy, cargo,\r\n",
                                 ch);
                        send_to_char
                                ("             battalions, Speed, manuever, hyperspeed.\r\n",
                                 ch);
                        send_to_char
                                ("             &Rremove&w will remove stealth from the ship.\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "remove"))
                {
                        if (ship->stealth <= 0)
                        {
                                send_to_char
                                        ("There is no stealth drive to remove.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->stealth > 0)
                        {
                                ship->stealth -= 1;
                                send_to_char("Stealth drive removed.\r\n",
                                             ch);
                                learn_from_success(ch, gsn_modifyship);
                                return;
                        }
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull Armor to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships energy reduced by 100\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough cargo space to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char
                                        ("Ships cargo capacity reduced by 50\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not battalion holds to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "Speed"))
                {
                        if (ship->realspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough engines to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships Speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 25)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manueverability reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyper drives to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 25)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->stealth += 1;
                send_to_char("Stealth drive installed.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 100000;
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 500000;
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 1000000;
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "cloak"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 500000)
                        {
                                send_to_char
                                        ("You need 500000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 1000000)
                        {
                                send_to_char
                                        ("You need 1000000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        if (ship->cloak >= 1)
                        {
                                send_to_char
                                        ("This ship already has a cloaking device.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ship->cloak >= 1)
                        {
                                send_to_char
                                        ("This ship already has a cloaking device.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->cloak >= 1 && str_cmp(arg2, "remove"))
                        {
                                send_to_char
                                        ("This ship already has a cloaking device.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields"))
                    && (str_cmp(arg2, "energy")) && (str_cmp(arg2, "cargo"))
                    && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "Speed")) && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed"))
                    && (str_cmp(arg2, "remove")))

                {
                        send_to_char
                                ("You may not substitute that for a stealth drive\r\n",
                                 ch);
                        send_to_char
                                ("Options are: hull, shields, energy, cargo,\r\n",
                                 ch);
                        send_to_char
                                ("             battalions, Speed, manuever, hyperspeed.\r\n",
                                 ch);
                        send_to_char
                                ("             &Rremove&w will remove cloak from the ship.\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "remove"))
                {
                        if (ship->cloak <= 0)
                        {
                                send_to_char
                                        ("There is no cloaking device to remove.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->cloak > 0)
                        {
                                ship->cloak -= 1;
                                send_to_char("Cloaking device removed.\r\n",
                                             ch);
                                learn_from_success(ch, gsn_modifyship);
                                return;
                        }
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull Armor to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships energy reduced by 100\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough cargo space to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char
                                        ("Ships cargo capacity reduced by 50\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not battalion holds to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "Speed"))
                {
                        if (ship->realspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough engines to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships Speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 25)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manueverability reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyper drives to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 25)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->cloak += 1;
                send_to_char("Cloaking device installed.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 100000;
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 500000;
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 1000000;
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "interdictor"))
        {

                if (ship->ship_class == FighterShip)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MidsizeShip)
                {
                        if (ch->gold < 500000)
                        {
                                send_to_char
                                        ("You need 500000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ch->gold < 1000000)
                        {
                                send_to_char
                                        ("You need 1000000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FighterShip)
                {
                        send_to_char
                                ("Starfighters cannot house gravity cones.\r\n",
                                 ch);
                        return;
                }
                if (ship->ship_class == MidsizeShip)
                {
                        send_to_char
                                ("Midships cannot house gravity cones.\r\n",
                                 ch);
                        return;
                }
                if (ship->ship_class == CapitalShip)
                {
                        if (ship->interdictor >= 1 && str_cmp(arg2, "remove"))
                        {
                                send_to_char
                                        ("This ship already has a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields"))
                    && (str_cmp(arg2, "energy")) && (str_cmp(arg2, "cargo"))
                    && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "Speed")) && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed"))
                    && (str_cmp(arg2, "remove")))
                {
                        send_to_char
                                ("You may not substitute that for a stealth drive\r\n",
                                 ch);
                        send_to_char
                                ("Options are: hull, shields, energy, cargo,\r\n",
                                 ch);
                        send_to_char
                                ("             battalions, Speed, manuever, hyperspeed.\r\n",
                                 ch);
                        send_to_char
                                ("             &Rremove&w will remove the gravity cone from the ship.\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "remove"))
                {
                        if (ship->interdictor <= 0)
                        {
                                send_to_char
                                        ("There is no gravity cone to remove.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->interdictor > 0)
                        {
                                ship->interdictor -= 1;
                                send_to_char("Gravity cone removed.\r\n", ch);
                                learn_from_success(ch, gsn_modifyship);
                                return;
                        }
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 501)
                        {
                                send_to_char
                                        ("There is not enough hull Armor to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 501)
                        {
                                ship->maxhull -= 500;
                                send_to_char("Ships hull reduced by 500\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 51)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 51)
                        {
                                ship->maxshield -= 50;
                                send_to_char
                                        ("Ships shields reduced by 50\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 501)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 501)
                        {
                                ship->maxenergy -= 500;
                                send_to_char
                                        ("Ships energy reduced by 500\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 500)
                        {
                                send_to_char
                                        ("There is not enough cargo space to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 500)
                        {
                                ship->maxcargo -= 500;
                                send_to_char
                                        ("Ships cargo capacity reduced by 500\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 10)
                        {
                                send_to_char
                                        ("There are not enough battalion holds to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions > 10)
                        {
                                ship->maxbattalions -= 10;
                                send_to_char
                                        ("Ships battalion holds reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "Speed"))
                {
                        if (ship->realspeed < 56)
                        {
                                send_to_char
                                        ("There are not enough engines to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 55)
                        {
                                ship->realspeed -= 55;
                                send_to_char("Ships Speed reduced by 55\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 56)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 55)
                        {
                                ship->manuever -= 55;
                                send_to_char
                                        ("Ships manueverability reduced by 55\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 56)
                        {
                                send_to_char
                                        ("There are not enough hyper drives to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 55)
                        {
                                ship->hyperspeed -= 55;
                                send_to_char
                                        ("Ships hyperspeed reduced by 55\r\n",
                                         ch);
                        }
                }

                ship->interdictor += 1;
                send_to_char("Gravity cone installed.\r\n", ch);
                if (ship->ship_class == FighterShip)
                {
                        ch->gold -= 100000;
                }
                if (ship->ship_class == MidsizeShip)
                {
                        ch->gold -= 500000;
                }
                if (ship->ship_class == CapitalShip)
                {
                        ch->gold -= 1000000;
                }
                learn_from_success(ch, gsn_modifyship);
        }

        save_ship(ship);

}


CMDF do_modifyexit(CharData * ch, char *argument)
{
        ExitData *xit;
        char      arg[MaxInputLength];
        char      arg2[MaxInputLength];
        int       percentage, type = 0;
        bool      checktool, checkdura;
        ObjData *obj;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);
        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: modifyexit <direction> <type>\n\r&w",
                                 ch);
                        send_to_char("&RAvailable options for type are:\n\r",
                                     ch);
                        send_to_char("\t door, hidden\n\r", ch);
                        return;
                }

                if (!ship_from_room(ch->in_room->vnum)
                    && !installation_from_room(ch->in_room->vnum))
                {
                        send_to_char
                                ("&RThis isn't a good place to try that...\n\r&w",
                                 ch);
                        return;
                }
                if (!str_cmp(arg, "n") || !str_cmp(arg, "north") ||
                    !str_cmp(arg, "s") || !str_cmp(arg, "south") ||
                    !str_cmp(arg, "e") || !str_cmp(arg, "east") ||
                    !str_cmp(arg, "w") || !str_cmp(arg, "west") ||
                    !str_cmp(arg, "sw") || !str_cmp(arg, "southwest") ||
                    !str_cmp(arg, "nw") || !str_cmp(arg, "northwest") ||
                    !str_cmp(arg, "se") || !str_cmp(arg, "southeast") ||
                    !str_cmp(arg, "ne") || !str_cmp(arg, "northeast") ||
                    !str_cmp(arg, "u") || !str_cmp(arg, "up") ||
                    !str_cmp(arg, "d") || !str_cmp(arg, "down"))
                {
                        if (!get_exit(ch->in_room, get_dir(arg)))
                        {
                                send_to_char
                                        ("There is no an exit in that direction\r\n",
                                         ch);
                                return;
                        }
                }

                else
                {
                        send_to_char("Not a Valid direction\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "door"))
                        type = ExIsdoor;
                else if (!str_cmp(arg2, "hidden"))
                        type = ExHidden;
                else
                {
                        send_to_char
                                ("&RUsage: modifyexit <direction> <type>\n\r&w",
                                 ch);
                        send_to_char("&RAvailable options for type are:\n\r",
                                     ch);
                        send_to_char("\t door, hidden\n\r", ch);
                        return;
                }



                checktool = FALSE;
                checkdura = FALSE;

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ItemToolkit)
                                checktool = TRUE;
                        if (obj->item_type == ItemDurasteel)
                                checkdura = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a new door and lock.\n\r",
                                 ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char
                                ("&RYou need something to make it out of.\n\r",
                                 ch);
                        return;
                }


                percentage = IsNpc(ch) ? ch->top_level
                        : (int) (ch->PCData->learned[gsn_modifyexit]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou take your equipment and begin to modify your exit.\n\r",
                                 ch);
                        act(AtPlain,
                            "$n takes $s tools and begins to work on something.",
                            ch, NULL, argument, ToRoom);
                        add_timer(ch, TimerDoFun, 25, do_modifyexit, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_modifyexit);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SubTimerDoAbort:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SubNone;
                send_to_char
                        ("&RSuddenly stop your actions, and drop your toolkit onto the floor.\n\r",
                         ch);
                return;
        }

        ch->substate = SubNone;
        if (!str_cmp(arg2, "door"))
                type = ExIsdoor;
        else if (!str_cmp(arg2, "hidden"))
                type = ExHidden;
        checktool = FALSE;
        checkdura = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ItemToolkit)
                        checktool = TRUE;
                if (obj->item_type == ItemDurasteel && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }

        percentage = IsNpc(ch) ? ch->top_level
                : (int) (ch->PCData->learned[gsn_modifyexit]);
        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura))
        {
                send_to_char
                        ("&RYou finish your  construction and stand back to take a look.\n\r",
                         ch);
                send_to_char
                        ("&RAs you look at it, it falls off the frame and almost hits you.\n\r",
                         ch);
                learn_from_failure(ch, gsn_modifyship);
                return;
        }

        if (!str_cmp(arg, "n") || !str_cmp(arg, "north") ||
            !str_cmp(arg, "s") || !str_cmp(arg, "south") ||
            !str_cmp(arg, "e") || !str_cmp(arg, "east") ||
            !str_cmp(arg, "w") || !str_cmp(arg, "west") ||
            !str_cmp(arg, "sw") || !str_cmp(arg, "southwest") ||
            !str_cmp(arg, "nw") || !str_cmp(arg, "northwest") ||
            !str_cmp(arg, "se") || !str_cmp(arg, "southeast") ||
            !str_cmp(arg, "ne") || !str_cmp(arg, "northeast") ||
            !str_cmp(arg, "u") || !str_cmp(arg, "up") ||
            !str_cmp(arg, "d") || !str_cmp(arg, "down"))
        {
                if ((xit = find_door(ch, arg, TRUE)) == NULL)
                {
                        send_to_char
                                ("There is already an exit in that direction, but this should not have happened. Talk to Greven.\r\n",
                                 ch);
                        return;
                }
                else
                        set_bexit_flag(xit, type);
        }

        else
        {
                send_to_char
                        ("Not a Valid direction, but this should not have happened. Talk to Greven.\r\n",
                         ch);
                return;
        }

        send_to_char
                ("&GYou finish your work and examine the modifications you've performed.&w\n\r",
                 ch);
        act(AtPlain, "$n finishes their work.", ch, NULL, argument, ToRoom);

        {
                long      xpgain;

                xpgain = UMIN(20000,
                              (exp_level
                               (ch->skill_level[EngineeringAbility] + 1) -
                               exp_level(ch->
                                         skill_level[EngineeringAbility])));
                gain_exp(ch, xpgain, EngineeringAbility);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_modifyexit);

        if (ship_from_room(ch->in_room->vnum) != NULL)
                fold_area(ch->in_room->area, ch->in_room->area->filename,
                          FALSE, FALSE);

        if (installation_from_room(ch->in_room->vnum) != NULL)
                fold_area(ch->in_room->area, ch->in_room->area->filename,
                          FALSE, FALSE);
}

CMDF do_addmprogstring(CharData * ch, char *argument)
{
        MProgData *mprog, *mprg;
        int       mptype, len;
        char      buf[MSL];

        argument = NULL;

        mprog = ch->in_room->mudprogs;
        mptype = get_mpflag("rand");
        if (mptype == -1)
        {
                send_to_char("Unknown program type.\n\r", ch);
                return;
        }
        if (mprog)
                for (; mprog->next; mprog = mprog->next);
        CREATE(mprg, MProgData, 1);
        if (mprog)
                mprog->next = mprg;
        else
                ch->in_room->mudprogs = mprg;
        ch->in_room->progtypes |= (1 << mptype);
        mprg->type = 1 << mptype;
        mprg->arglist = STRALLOC("100");
        if (!mprg->comlist)
                mprg->comlist = STRALLOC("");
        if (mprg->comlist)
                STRFREE(mprg->comlist);
        snprintf(buf, MSL, "%s", "mpecho This is a test done by the code");
        smush_tilde(buf);
        len = strlen(buf);
        if (buf && buf[len - 1] == '~')
                buf[len - 1] = '\0';
        else
                mudstrlcat(buf, "\n\r", MSL);
        mprg->comlist = STRALLOC(buf);
        send_to_char(buf, ch);
        return;
}

bool dismantle_values(ShipData * ship, PlanetData * planet,int * durasteel, int * transparisteel, int * cost, int * fee)
{
        *durasteel = 0;
        *transparisteel = 0;
        *cost = 0;
        *fee = 0;

        if (!ship || !planet)
                return FALSE;

        /*
         * these values come from  cargo v2 
         */
        *durasteel = ship->ship_class * 150 + 100;
        *transparisteel = ship->ship_class * 20 + 10;

        /*
         * This stuff needs to be tweaked for balancing, i  really don't understand the system though. - Gavin 
         */
        *cost = 10;
        if (planet->cargoexport[CargoTransparisteel] > 0)
                *cost += planet->cargoexport[CargoTransparisteel] +
                        planet->cargoexport[CargoTransparisteel] / 2;
        else if (planet->cargoimport[CargoTransparisteel] > 0)
                *cost += planet->cargoimport[CargoTransparisteel];
        else
                *cost += 10;

        if (planet->cargoexport[CargoDurasteel] > 0)
                *cost += planet->cargoexport[CargoDurasteel] +
                        planet->cargoexport[CargoDurasteel] / 2;
        else if (planet->cargoimport[CargoDurasteel] > 0)
                *cost += planet->cargoimport[CargoDurasteel];
        else
                *cost += 10;

        *cost *= *durasteel + *transparisteel;
        *fee = *cost * ((ship->ship_class * 5) / 100);

/*         *fee = get_ship_value(ship) / 2; */
        return TRUE;
}

CMDF do_dismantle_ship(CharData * ch, char *argument)
{
        int       percentage;
        bool      checktool;
        int       durasteel = 0;
        int       transparisteel = 0;
        int       cost = 0;
        int       fee = 0;
        RoomIndexData *room;
        ObjData *obj;
        ShipData *ship = NULL;
        PlanetData *planet;

        if (IsSet(ch->in_room->RoomFlags, RoomInstallation))
        {
                InstallationData *installation =
                        installation_from_room(ch->in_room->vnum);
                if (installation
                    && installation->type == ShipyardInstallation)
                        planet = installation->planet;
        }
        else
                planet = ch->in_room->area->planet;

        if (!planet)
        {
                /*
                 * Installations might eventually be spacestations 
                 */
                send_to_char
                        ("Better to actually do this on a planet or installation.",
                         ch);
                return;
        }

        if (!IsSet(ch->in_room->RoomFlags, RoomImport))
        {
                send_to_char
                        ("You can't build that here!! Try a spaceport\r\n",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                {
                        if (argument[0] == '\0')
                        {
                                send_to_char
                                        ("&RUsage: &Gdismantle_ship &C<&cname of ship&C>&w\r\n",
                                         ch);
                                return;
                        }
                        ship = ship_in_room(ch->in_room, argument);

                        if (!ship)
                        {
                                send_to_char("Which ship?\n\r", ch);
                                return;
                        }

                        if (!check_pilot(ch, ship))
                        {
                                send_to_char
                                        ("You don't have permission to build on this ship\r\n",
                                         ch);
                                return;
                        }
						dismantle_values(ship, planet,&cost,&durasteel,&transparisteel,&fee);

                        checktool = FALSE;

                        if (ch->gold < cost+fee)
                        {
                                send_to_char
                                        ("&RYou can't afford to hire the workforce for that.\r\n",
                                         ch);
                                return;
                        }

                        for (obj = ch->last_carrying; obj;
                             obj = obj->prev_content)
                        {
                                if (obj->item_type == ItemToolkit)
                                        checktool = TRUE;
                        }

                        if (!checktool)
                        {
                                send_to_char
                                        ("Unable to get any work done without a set of tools.\r\n",
                                         ch);
                                return;
                        }
                        percentage =
                                IsNpc(ch) ? ch->top_level : (int) (ch->
                                                                    PCData->
                                                                    learned
                                                                    [gsn_dismantle_ship]);
                        if (number_percent() < percentage)
                        {
                                send_to_char
                                        ("&GYou begin the LONG Process of dismantling a ship.\n\r",
                                         ch);
                                act(AtPlain,
                                    "$n takes $s tools and starts dismantling a ship.\r\n",
                                    ch, NULL, argument, ToRoom);
                                add_timer(ch, TimerDoFun, 35,
                                          do_dismantle_ship, 1);
                                ch->dest_buf = ship;
                                return;
                        }
                        send_to_char("&RYou can't figure how to start.\n\r",
                                     ch);
                        learn_from_failure(ch, gsn_dismantle_ship);
                        return;
                }

        case 1:
                ch->substate = SubNone;
                if (!ch->dest_buf)
                        return;
                break;

        case SubTimerDoAbort:
                ch->dest_buf = NULL;

                ch->substate = SubNone;
                send_to_char
                        ("&RYou are interupted and fail to finish your work, luckly you didn't damage the ship matirials.\n\r",
                         ch);
                return;
        }
        ship = NULL;

        for (ShipData * tmp = ch->in_room->first_ship; tmp;
             tmp = tmp->next_in_room)
        {
                if (tmp == (ShipData *) ch->dest_buf)
                {
                        ship = tmp;
                        break;
                }
        }
        ch->substate = SubNone;
        ch->dest_buf = NULL;


        if (!ship)
        {
                send_to_char
                        ("&RShip must have left. Make sure the ship is stationary and landed next time.\n\r",
                         ch);
                return;
        }

        if (!check_pilot(ch, ship))
        {
                send_to_char
                        ("You don't have permission to build on this ship\r\n",
                         ch);
                return;
        }
        dismantle_values(ship, planet,&cost,&durasteel,&transparisteel,&fee);

        if (ch->gold < cost+fee)
        {
                send_to_char
                        ("&RYou can't afford the materials....Stop that THIEF!!!\r\n",
                         ch);
                return;
        }

        room = get_room_index(ship->firstroom);
        if (room)
        {
                AreaData *area;

                area = room->area;
                /*
                 * Just in case we try to use this after we destroy the ship, that wouldn't be fun 
                 */
                room = NULL;
                fold_area(area, area->filename, TRUE, FALSE);
        }
		
		/* Remove the ships */
		really_destroy_ship(ship);
		ship = NULL;
		write_ship_list();

		/* Remove costs + fees and add stuff back to the planet */
		ch->gold -= cost;
		ch->gold -= fee;
		planet->resource[CargoTransparisteel] += transparisteel;
		planet->resource[CargoDurasteel] += durasteel;


        /*
         * I added this fee to balance the cost of building ships with that of selling them so as to 
         * keep engis from making too much money too fast from just building and selling ships
         */
        ch_printf(ch,
                  "The planet's Government has assesed a decommision Licence Fee of %d credits.\r\n",
                  fee);
        ch_printf(ch,
                  "It costs you %d credits for materials, tools, and labour.\r\n",
                  fee);
        gain_exp(ch, 10000, EngineeringAbility);
        ch_printf(ch, "&WYou gain 10000 engineering experience.\r\n");
        learn_from_success(ch, gsn_dismantle_ship);
        act(AtPlain, "$n finishes dismantling the ship..", ch, NULL,
            argument, ToRoom);
        return;
}
