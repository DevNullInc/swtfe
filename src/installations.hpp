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
 *                              SWR Installation Module                                  *
 ****************************************************************************************/

//typedef struct dock_data DOCK_DATA;
//typedef struct installation_data INSTALLATION_DATA;


#pragma once
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

class DockData;
class ClanData;
class PlanetData;
class CharData;

using InstallationPtr = std::shared_ptr<class Installation>;
using InstallationList = std::vector<InstallationPtr>;

extern InstallationList all_installations;

/* installation structure */

class Installation {
public:
        InstallationPtr next;
        InstallationPtr prev;
        InstallationPtr next_on_planet;
        InstallationPtr prev_on_planet;
        std::shared_ptr<DOCK_DATA> first_dock;
        std::shared_ptr<DOCK_DATA> last_dock;
        std::shared_ptr<ClanData> clan;
        std::string filename;
        int32_t first_room = 0;
        int32_t flags = 0;
        int32_t last_built = 0;
        int32_t last_room = 0;
        std::shared_ptr<PLANET_DATA> planet;
        int32_t type = 0;
        int32_t mainroom = 0;
        int16_t timer = 0;
        int32_t weapon_time = 0;

        Installation() = default;
        ~Installation() = default;
        Installation(const Installation&) = delete;
        Installation& operator=(const Installation&) = delete;
        Installation(Installation&&) = default;
        Installation& operator=(Installation&&) = default;
};

/* Type and flag declarations */


enum class InstallationType : int32_t {
        Battery,
        Ion,
        Turbolaser,
        Research,
        ClanHQ,
        Training,
        Government,
        Shipyard,
        Mining,
        Max
};


enum class InstallationLocation : int32_t {
        Underground = 0,
        Aboveground = 1,
        Underwater = 2
};

constexpr int INS_SECURE = 1 << 0;
constexpr int MAX_INSTALLATION = static_cast<int>(InstallationType::Max);

inline constexpr auto INSTALLATIONS_DIR   = "../installations/";
inline constexpr auto INSTALLATION_LIST   = "installations.lst";
inline constexpr auto INSTALLATION_AREA   = "pinstalls.are";

inline constexpr int MOB_VNUM_INSTALL_GUARD           = 33001;
inline constexpr int MOB_VNUM_INSTALL_ENTRANCE_GUARD  = 33002;
inline constexpr int MOB_VNUM_INSTALL_DOCTOR          = 33003;
inline constexpr int MOB_VNUM_INSTALL_CUSTOMS         = 33004;

inline constexpr int OBJ_VNUM_INSTALL_BACTA_SPRAY     = 33001;
inline constexpr int OBJ_VNUM_INSTALL_MEDPAC          = 33002;


int find_pvnum_block(int num_needed, const std::string& areaname);
void load_installations();
void echo_to_installation(int16_t at_color, const std::string& argument, const InstallationPtr& installation);
bool mob_reset(std::shared_ptr<CharData> ch, const std::string& type, bool check);
InstallationPtr installation_from_room(int vnum);
void add_room_installation(std::shared_ptr<CharData> ch, const std::string& argument);
void fireplanet_update();
int planetary_installations(const std::shared_ptr<PlanetData>& planet, InstallationType type);



struct InstallationTypeInfo {
        std::string installation_name;
        int16_t shields = 0;
        int16_t ions = 0;
        int16_t turbo = 0;
        int16_t rooms = 0;
        std::string main_name;
};

extern const std::vector<InstallationTypeInfo> installation_table;
