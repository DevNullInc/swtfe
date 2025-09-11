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
 *                             SWTFE Astral Body Module                                  *
 ****************************************************************************************/

#pragma once


#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// Directory and file constants - modernized
namespace BodyConstants {
    constexpr const char* BODY_DIR = "../body/";
    constexpr const char* FILE_BODY_LIST = "body.lst";
}

// Legacy macros for compatibility
#define BODY_DIR       "../body/"
#define FILE_BODY_LIST	"body.lst"
typedef std::list < DockData * >DockList;
extern DockData *first_dock;
extern DockData *last_dock;

// Body type enumeration - modernized enum class
enum class BodyTypes : int {
    STAR = 0,
    PLANET = 1,
    MOON = 2,
    COMET = 3,
    ASTEROID = 4,
    BLACKHOLE = 5,
    NEBULA = 6,
    ALL = 7
};

// Legacy compatibility typedef


// Modern utility functions for body system
namespace BodyUtils {
    inline std::string safe_string(const char* str) {
        return str ? str : "";
    }
    
    inline bool is_valid_body_type(int type) {
        return type >= 0 && type <= static_cast<int>(BodyTypes::ALL);
    }
}


/**
 * @class BodyData
 * @brief Modernized celestial body management system
 * 
 * Represents a celestial body (planet, star, moon, etc.) in the space simulation.
 * Features modern C++ with RAII, std::string usage, and improved safety.
 * 
 * Key modernizations:
 * - std::string replaces char* for automatic memory management
 * - Constructor uses member initialization lists
 * - Const-correct accessors with backward compatibility
 * - Modern enum class support with legacy compatibility
 * - RAII pattern for automatic resource cleanup
 */
class BodyData
{
      private:
        std::string _filename;    // Modernized from char*
        int _gravity;
        std::string _name;        // Modernized from char*
        BodyTypes _type;
        int _xpos;
        int _ypos;
        int _zpos;
        int _orbitcount;
        int _xmove;
        int _ymove;
        int _zmove;
        int _centerx;
        int _centery;
        int _centerz;

        PlanetData *_planet;
        SpaceData *_starsystem;
        AreaList _areas;
        DockList _docks;

        /*
         * Get Functions 
         */
      public:
        BodyData();
        ~BodyData();
        inline SpaceData *starsystem()
        {
                return this->_starsystem;
        }
        void starsystem(SpaceData * s);

        inline PlanetData *planet()
        {
                return this->_planet;
        }
        inline void planet(PlanetData * p)
        {
                this->_planet = p;
        } inline DockList & docks(void)
        {
                return this->_docks;
        }
        inline AreaList & areas(void)
        {
                return this->_areas;
        }
        // Modern string accessors with backward compatibility
        inline const char* name() const
        {
                return this->_name.c_str();
        }
        inline const std::string& name_string() const
        {
                return this->_name;
        }
        inline void name(const char* newname)
        {
                this->_name = newname ? newname : "";
        }
        inline void name(const std::string& newname)
        {
                this->_name = newname;
        }
        // Modern filename accessors with backward compatibility
        inline const char* filename() const
        {
                return this->_filename.c_str();
        }
        inline const std::string& filename_string() const
        {
                return this->_filename;
        }
        inline void filename(const char* newfilename)
        {
                this->_filename = newfilename ? newfilename : "";
        }
        inline void filename(const std::string& newfilename)
        {
                this->_filename = newfilename;
        }
        inline int gravity()
        {
                return this->_gravity;
        }
        inline void gravity(int a)
        {
                this->_gravity = a;
        } inline int type() const
        {
                return this->_type;
        }
        inline void type(int a)
        {
                this->_type = a;
        } inline int xpos()
        {
                return this->_xpos;
        }
        inline void xpos(int a)
        {
                this->_xpos = a;
        } inline int ypos()
        {
                return this->_ypos;
        }
        inline void ypos(int a)
        {
                this->_ypos = a;
        } inline int zpos()
        {
                return this->_zpos;
        }
        inline void zpos(int a)
        {
                this->_zpos = a;
        } inline int xmove()
        {
                return this->_xmove;
        }
        inline void xmove(int a)
        {
                this->_xmove = a;
        } inline int ymove()
        {
                return this->_ymove;
        }
        inline void ymove(int a)
        {
                this->_ymove = a;
        } inline int zmove()
        {
                return this->_zmove;
        }
        inline void zmove(int a)
        {
                this->_zmove = a;
        } inline int centerx()
        {
                return this->_centerx;
        }
        inline void centerx(int a)
        {
                this->_centerx = a;
        } inline int centery()
        {
                return this->_centery;
        }
        inline void centery(int a)
        {
                this->_centery = a;
        } inline int centerz()
        {
                return this->_centerz;
        }
        inline void centerz(int a)
        {
                this->_centerz = a;
        } inline int orbitcount()
        {
                return this->_orbitcount;
        }
        inline void orbitcount(int a)
        {
                this->_orbitcount = a;
        }
        /*
         * Other Functions 
         */
      public:void save();

        BodyData *load(FILE *);
        void remove_area(AreaData * pArea);
        void add_area(AreaData * pArea);
        void add_dock(DockData *);
        void remove_dock(DockData *);
        char     *get_direction(ShipData * ship);
        const char *type_name() const;
        int distance(ShipData * ship);
        int distance(BodyData * pbody);
        int hyperdistance(ShipData * ship);
        RoomIndexData *get_rand_room(int bit, bool include);
};


// Modern function declarations
BodyData *get_body(const char *name);
BodyData *get_body(std::string_view name);
BodyData *get_body(std::shared_ptr<std::string> name);
DockData *get_dock(const char *name);
DockData *get_dock(std::string_view name);
DockData *get_dock(std::shared_ptr<std::string> name);
DockData *get_dock_isname(ShipData *ship, const char *name);
DockData *get_dock_isname(ShipData *ship, std::string_view name);
DockData *get_dock_isname(std::shared_ptr<ShipData> ship, std::string_view name);
void load_bodies();

