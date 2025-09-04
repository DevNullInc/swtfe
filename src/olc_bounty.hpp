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
 * Header file for Online Creation Bounty System definitions and data structures. *
 ****************************************************************************************/
#ifndef __OLC_BOUNTY_DATA_H__
#define __OLC_BOUNTY_DATA_H__

#include <list>
#include "mud.hpp"

enum
{
        BOUNTY_ALIVE, BOUNTY_DEAD, MAX_BOUNTY_TYPES
};

const char *const olc_bounty_types[] = {
        "Alive", "Dead", "MAX_TYPE"
};

#define OLC_BOUNTY_FILE SYSTEM_DIR "olcbounty.dat"

class     OLC_BOUNTY_DATA
{
      private:
        int       _owner;
        int       _vnum;
        int       _corpse;
        sh_int    _type;
        int       _amount;
        int       _exp;

      public:
                  inline sh_int type()
        {
                return this->_type;
        }
        inline bool type(int i)
        {
                if (MAX_BOUNTY_TYPES <= i)
                {
                        return FALSE;
                }

                this->_type = static_cast<sh_int>(i);
                return TRUE;
        }

        inline int owner()
        {
                return this->_owner;
        }

        inline bool owner(int vnum)
        {
                if (vnum < 0 || vnum > MAX_VNUMS)
                {
                        return FALSE;
                }
                this->_owner = vnum;
                return TRUE;
        }

        inline int vnum()
        {
                return this->_vnum;
        }

        inline bool vnum(int vnum)
        {
                if (vnum < 0 || vnum > MAX_VNUMS)
                {
                        return FALSE;
                }
                this->_vnum = vnum;
                return TRUE;
        }
        inline int corpse()
        {
                return this->_corpse;
        }

        inline bool corpse(int vnum)
        {
                if (vnum < 0 || vnum > MAX_VNUMS)
                {
                        return FALSE;
                }
                this->_corpse = vnum;
                return TRUE;
        }
        inline int amount()
        {
                return this->_amount;
        }

        inline void amount(int credits)
        {
                this->_amount = credits;
        }

        inline int experience()
        {
                return this->_exp;
        }

        inline void experience(int experience)
        {
                this->_exp = experience;
        }

      public:
        OLC_BOUNTY_DATA();
        OLC_BOUNTY_DATA(int vnum);

        ~OLC_BOUNTY_DATA();
        void      save();
        void      load(FILE * fp);
        static void load_olc_bounties(void);
};


typedef std::list < OLC_BOUNTY_DATA * >OLC_BOUNTY_LIST;
extern OLC_BOUNTY_LIST olc_bounties;
OLC_BOUNTY_DATA *has_olc_bounty(CHAR_DATA * victim);
void mset_bounty(CHAR_DATA * ch, CHAR_DATA * mob, char *argument);
void print_olc_bounties_mob(CHAR_DATA * ch, CHAR_DATA * mob);
void load_olc_bounties(void);
bool check_given_bounty(CHAR_DATA * ch, CHAR_DATA * hunter, OBJ_DATA * obj);
int print_olc_bounties(CHAR_DATA * ch);
bool check_olc_bounties(ROOM_INDEX_DATA * room);

#endif
