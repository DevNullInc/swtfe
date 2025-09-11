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
 * Homes system header for player housing and property management.                       *
 ****************************************************************************************/


#pragma once
#include <list>
#include <algorithm>
#include <string>
#include <memory>
#include "grid.hpp"


struct ROOMMATE_DATA {
	int type = 0; // Unused legacy field
	std::string name;
	ROOMMATE_DATA();
	~ROOMMATE_DATA();
};


using ROOM_LIST = std::list<RoomIndexData*>;
using ROOMMATE_LIST = std::list<std::shared_ptr<ROOMMATE_DATA>>;

struct HOME_DATA {
	HOME_DATA* next = nullptr;
	HOME_DATA* prev = nullptr;
	std::string filename;
	std::string name;
	std::string description;
	std::string owner; // Can be clan, player, or 'public'
	long int price = 0;
	std::shared_ptr<GRID_WRAPPER> grid;

	ROOMMATE_LIST roommates;
	ROOM_LIST rooms;

	// Status: Public, private
	// (Default) Private - Only allowed list of people allowed in
	//           Public  - Anyone allowed in, good for shops and stuff

	HOME_DATA();
	~HOME_DATA();
	void save();
	void add_room(CharData* ch, const std::string& argument);
	void decorate_room(CharData* ch, const std::string& argument);
	bool check_member(CharData* ch);
	bool can_enter(CharData* ch);
	void echo(int color, const std::string& argument);
	void reset();

	// Remove a roommate
	inline void remove(const std::shared_ptr<ROOMMATE_DATA>& roomie) {
		roommates.remove(roomie);
	}
	// Add a roommate
	inline void add(const std::shared_ptr<ROOMMATE_DATA>& roomie) {
		roommates.push_back(roomie);
	}
	// Remove a room
	inline void remove(RoomIndexData* room) {
		rooms.remove(room);
		room->home = nullptr;
	}
	// Add a room
	inline void add(RoomIndexData* room) {
		rooms.push_back(room);
		room->home = this;
	}
};


constexpr auto HOMEDIR = "../homes/";
constexpr auto HOME_LIST = "homes.lst";
constexpr int HOME_SAVE_TIME = 60 * 20; // 20 Minutes

// homes.c
HOME_DATA* get_home(const std::string& name);
void save_home2(HOME_DATA* home);
void write_home_list();
void fwrite_roommates(FILE* fp, HOME_DATA* home);
void load_homes();
void fread_roommate(std::shared_ptr<ROOMMATE_DATA> roomie, FILE* fp);
void fread_home(HOME_DATA* home, FILE* fp);
void save_homes_check();
long get_home_value(HOME_DATA* home);
bool load_home_file(const std::string& homefile);

extern HOME_DATA* first_home;
extern HOME_DATA* last_home;
extern const char* const home_flags[];
extern time_t save_homes_time;

DECLARE_DO_FUN(do_homes);
DECLARE_DO_FUN(do_makehome);
DECLARE_DO_FUN(do_showhome);
DECLARE_DO_FUN(do_sethome);
DECLARE_DO_FUN(do_resethome);
DECLARE_DO_FUN(do_buyhome2);
DECLARE_DO_FUN(do_sellhome2);
DECLARE_DO_FUN(do_freehomes);
