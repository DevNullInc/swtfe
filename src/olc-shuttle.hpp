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
 *                                SWR OLC Shuttle module                                 *
 ****************************************************************************************/


#pragma once

#include <vector>
#include <memory>
#include <string>
#include <string_view>

constexpr std::string_view ShuttleDir = "../shuttle/";
constexpr std::string_view ShuttleList = "shuttle.lst";

class StopData {
public:
        std::string stop_name;
        int room{0};

        StopData(std::string name, int RoomVnum) : stop_name(std::move(name)), room(RoomVnum) {}
        StopData() = default;
        ~StopData() = default;
};

enum class ShuttleState : int {
        Landing,
        Landed,
        TakingOff,
        InSpace,
        HyperspaceLaunch,
        HyperspaceEnd
};

enum class ShuttleClass : int {
        TurboCar,
        Space,
        Hyperspace
};

class ShuttleData {
public:
        // Room pointer types are left as-is for compatibility
        RoomIndexData* in_room{nullptr};
        std::shared_ptr<StopData> current;
        int current_number{0};
        ShuttleState state{ShuttleState::Landing};
        std::vector<std::shared_ptr<StopData>> stops;
        ShuttleClass type{ShuttleClass::Space};
        std::string filename;
        std::string name;
        int delay{0};
        int current_delay{0};
        int start_room{0};
        int end_room{0};
        int entrance{0};
        std::string takeoff_desc;
        std::string land_desc;
        std::string approach_desc;

        ShuttleData() = default;
        ShuttleData(std::string filename, std::string name) : filename(std::move(filename)), name(std::move(name)) {}
        ~ShuttleData() = default;
};

using ShuttleList = std::vector<std::shared_ptr<ShuttleData>>;
extern ShuttleList shuttles;

void update_shuttle();
std::shared_ptr<ShuttleData> get_shuttle(std::string_view argument);
void write_shuttle_list();
bool save_shuttle(const std::shared_ptr<ShuttleData>& shuttle);
std::shared_ptr<ShuttleData> make_shuttle(std::string_view filename, std::string_view name);
bool extract_shuttle(const std::shared_ptr<ShuttleData>& shuttle);
bool insert_shuttle(const std::shared_ptr<ShuttleData>& shuttle, RoomIndexData* room);
void load_shuttles();
bool load_shuttle_file(std::string_view shuttlefile);
void fread_shuttle(const std::shared_ptr<ShuttleData>& shuttle, FILE* fp);
void fread_stop(const std::shared_ptr<StopData>& stop, FILE* fp);
void destroy_shuttle(const std::shared_ptr<ShuttleData>& shuttle);
void show_shuttles_to_char(CharData* ch, const std::shared_ptr<ShuttleData>& shuttle);
std::shared_ptr<ShuttleData> shuttle_in_room(RoomIndexData* room, std::string_view name);
std::shared_ptr<ShuttleData> shuttle_from_entrance(int vnum);
