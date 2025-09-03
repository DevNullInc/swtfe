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
 *                             SWTFE Corpse/Body Module                                  *
 ****************************************************************************************/
#ifndef __BODY_DATA__
#define __BODY_DATA__

#include <list>
#include <memory>
#include <string>
#include <vector>

// Directory and file constants - modernized
namespace BodyConstants {
    constexpr const char* BODY_DIR = "../body/";
    constexpr const char* FILE_BODY_LIST = "body.lst";
}

// Legacy macros for compatibility
#define BODY_DIR       "../body/"
#define FILE_BODY_LIST	"body.lst"
typedef std::list < DOCK_DATA * >DOCK_LIST;
extern DOCK_DATA *first_dock;
extern DOCK_DATA *last_dock;

// Modern utility functions for body system
namespace BodyUtils {
    inline std::string safe_string(const char* str) {
        return str ? str : "";
    }
    
    inline bool is_valid_body_type(int type) {
        return type >= 0 && type <= static_cast<int>(BodyType::ALL);
    }
}


/**
 * @class BODY_DATA
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
class BODY_DATA
{
      private:
        std::string _filename;    // Modernized from char*
        int _gravity;
        std::string _name;        // Modernized from char*
        int _type;
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

        PLANET_DATA *_planet;
        SPACE_DATA *_starsystem;
        AREA_LIST _areas;
        DOCK_LIST _docks;

        /*
         * Get Functions 
         */
      public:
        BODY_DATA();
        ~BODY_DATA();
        inline SPACE_DATA *starsystem()
        {
                return this->_starsystem;
        }
        void starsystem(SPACE_DATA * s);

        inline PLANET_DATA *planet()
        {
                return this->_planet;
        }
        inline void planet(PLANET_DATA * p)
        {
                this->_planet = p;
        } inline DOCK_LIST & docks(void)
        {
                return this->_docks;
        }
        inline AREA_LIST & areas(void)
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
        } inline int type()
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

        BODY_DATA *load(FILE *);
        void remove_area(AREA_DATA * pArea);
        void add_area(AREA_DATA * pArea);
        void add_dock(DOCK_DATA *);
        void remove_dock(DOCK_DATA *);
        char     *get_direction(SHIP_DATA * ship);
        char     *type_name();
        int distance(SHIP_DATA * ship);
        int distance(BODY_DATA * pbody);
        int hyperdistance(SHIP_DATA * ship);
        ROOM_INDEX_DATA *get_rand_room(int bit, bool include);
};
// Body type enumeration - modernized enum class
enum class BodyType : int {
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
typedef enum { 
    STAR_BODY = 0, PLANET_BODY = 1, MOON_BODY = 2, COMET_BODY = 3, 
    ASTEROID_BODY = 4, BLACKHOLE_BODY = 5, NEBULA_BODY = 6, BODY_ALL = 7
} BODY_TYPES;

// Modern function declarations
BODY_DATA *get_body(char *name);
DOCK_DATA *get_dock(char *name);
DOCK_DATA *get_dock_isname(SHIP_DATA *ship, char *name);
void load_bodies();

// Legacy macro-style declarations for compatibility
BODY_DATA *get_body args((char *name));
DOCK_DATA *get_dock args((char *name));
DOCK_DATA *get_dock_isname args((SHIP_DATA * ship, char *name));
void load_bodies args((void));


#endif /*  */