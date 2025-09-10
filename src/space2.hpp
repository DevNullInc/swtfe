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
 * Space system header for ship and travel definitions and data structures.              *
 ****************************************************************************************/


#include <string_view>
#include <memory>
#include <string>

class BodyData;
class ClanData;
class InstallationData;
class PlanetData;
class ShipData;
class MissileData;

namespace space2 {
        inline constexpr std::string_view DOCK_DIR = "../dock/";
        inline constexpr std::string_view FILE_DOCK_LIST = "dock.lst";
        inline constexpr std::string_view SHIPIMAGE_DIR = "../shipimages/";

        class Dock {
        public:
                std::shared_ptr<ClanData> clan;
                int vnum = 0;
                bool hidden = false;
                bool temporary = false; // For installations and beacons
                std::shared_ptr<BodyData> body;
                std::string name;

                std::shared_ptr<Dock> next;
                std::shared_ptr<Dock> prev;
                std::shared_ptr<Dock> next_in_body;
                std::shared_ptr<Dock> prev_in_body;
                std::shared_ptr<Dock> next_in_installation;
                std::shared_ptr<Dock> prev_in_installation;

                Dock() = default;
                ~Dock() = default;
                Dock(const Dock&) = delete;
                Dock& operator=(const Dock&) = delete;
        };
}



// Modern C++23 prototypes and overloads
namespace space2 {
        void free_dock(std::shared_ptr<Dock> dock);
        void fread_planet(std::shared_ptr<PlanetData> planet, FILE* fp);
        bool load_planet_file(std::string_view planetfile);
        void write_planet_list();
        void fread_dock(std::shared_ptr<Dock> dock, FILE* fp);
        bool load_dock_file(std::string_view dockfile);
        void write_dock_list();
        void fread_body(std::shared_ptr<BodyData> body, FILE* fp);
        bool load_body_file(std::string_view bodyfile);
        void write_body_list();
        std::shared_ptr<PlanetData> get_planet(std::string_view name);
        void load_planets();
        void save_planet(std::shared_ptr<PlanetData> planet, bool copyover);
        void load_docks();
        void fwrite_dock(FILE* fp, std::shared_ptr<Dock> dock);
        void save_body(std::shared_ptr<BodyData> body);
        void makedock(std::shared_ptr<InstallationData> installation);

        int distance_ship_ship(std::shared_ptr<ShipData> target, std::shared_ptr<ShipData> ship);
        int distance_missile_ship(std::shared_ptr<MissileData> missile, std::shared_ptr<ShipData> ship);
        std::string get_direction_body(std::shared_ptr<BodyData> body, std::shared_ptr<ShipData> ship);
        std::string get_direction_ship(std::shared_ptr<ShipData> target, std::shared_ptr<ShipData> ship);
}
