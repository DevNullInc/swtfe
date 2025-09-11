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


struct RoommateData {
	int type = 0; // Unused legacy field
	std::string name;
	RoommateData();
	~RoommateData();
};


using RoomList = std::list<RoomIndexData*>;
using RoommateList = std::list<std::shared_ptr<RoommateData>>;

struct HomeData {
	HomeData* next = nullptr;
	HomeData* prev = nullptr;
	std::string filename;
	std::string name;
	std::string description;
	std::string owner; // Can be clan, player, or 'public'
	long int price = 0;
	std::shared_ptr<GridWrapper> grid;

	RoommateList roommates;
	RoomList rooms;

	// Status: Public, private
	// (Default) Private - Only allowed list of people allowed in
	//           Public  - Anyone allowed in, good for shops and stuff

	HomeData();
	~HomeData();
	void save();
	void add_room(CharData* ch, const std::string& argument);
	void decorate_room(CharData* ch, const std::string& argument);
	bool check_member(CharData* ch);
	bool can_enter(CharData* ch);
	void echo(int color, const std::string& argument);
	void reset();

	// Remove a roommate
	inline void remove(const std::shared_ptr<RoommateData>& roomie) {
		roommates.remove(roomie);
	}
	// Add a roommate
	inline void add(const std::shared_ptr<RoommateData>& roomie) {
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
constexpr auto HomeList = "homes.lst";
constexpr int HomeSaveTime = 60 * 20; // 20 Minutes

// homes.c
HomeData* get_home(const std::string& name);
void save_home2(HomeData* home);
void write_home_list();
void fwrite_roommates(FILE* fp, HomeData* home);
void load_homes();
void fread_roommate(std::shared_ptr<RoommateData> roomie, FILE* fp);
void fread_home(HomeData* home, FILE* fp);
void save_homes_check();
long get_home_value(HomeData* home);
bool load_home_file(const std::string& homefile);

extern HomeData* first_home;
extern HomeData* last_home;
extern const char* const home_flags[];
extern time_t save_homes_time;

DeclareDoFun(do_homes);
DeclareDoFun(do_makehome);
DeclareDoFun(do_showhome);
DeclareDoFun(do_sethome);
DeclareDoFun(do_resethome);
DeclareDoFun(do_buyhome2);
DeclareDoFun(do_sellhome2);
DeclareDoFun(do_freehomes);
