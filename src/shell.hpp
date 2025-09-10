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
 *                           ^     +----- |  / ^     ^ |     | +-\                       *
 *                          / \    |      | /  |\   /| |     | |  \                      *
 *                         /   \   +---   |<   | \ / | |     | |  |                      *
 *                        /-----\  |      | \  |  v  | |     | |  /                      *
 *                       /       \ |      |  \ |     | +-----+ +-/                       *
 *****************************************************************************************
 *                                                                                       *
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson), Levi Beckerson (Whir),           *
 * Michael Ward (Tarl), Erik Wolfe (Dwip), Cameron Carroll (Cam), Cyberfox, Karangi,     *
 * Rathian, Raine, and Adjani. All Rights Reserved.                                      *
 *                                                                                       *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                         Internal server shell command module                          *
 ****************************************************************************************/


#include <string_view>
#include <array>
#include <algorithm>


namespace shell {
	// Change this to update the base directory everywhere
	inline constexpr std::string_view BASE_DIR = "/workspaces/swtfe/tfeadmins/";
	inline constexpr std::string_view BINARYFILE = "swr";

	inline constexpr std::string_view CODEZONEDIR    = "/workspaces/swtfe/tfeadmins/dist3/area/";
	inline constexpr std::string_view BUILDZONEDIR   = "/workspaces/swtfe/tfeadmins/dist2/area/";
	inline constexpr std::string_view MAINZONEDIR    = "/workspaces/swtfe/tfeadmins/dist/area/";
	inline constexpr std::string_view TESTCODEDIR    = "/workspaces/swtfe/tfeadmins/dist3/src/";
	inline constexpr std::string_view BUILDCODEDIR   = "/workspaces/swtfe/tfeadmins/dist2/src/";
	inline constexpr std::string_view MAINCODEDIR    = "/workspaces/swtfe/tfeadmins/dist/src/";
	inline constexpr std::string_view CODESYSTEMDIR  = "/workspaces/swtfe/tfeadmins/dist3/system/";
	inline constexpr std::string_view BUILDSYSTEMDIR = "/workspaces/swtfe/tfeadmins/dist2/system/";
	inline constexpr std::string_view MAINSYSTEMDIR  = "/workspaces/swtfe/tfeadmins/dist/system/";
	inline constexpr std::string_view CODECLASSDIR   = "/workspaces/swtfe/tfeadmins/dist3/classes/";
	inline constexpr std::string_view BUILDCLASSDIR  = "/workspaces/swtfe/tfeadmins/dist2/classes/";
	inline constexpr std::string_view MAINCLASSDIR   = "/workspaces/swtfe/tfeadmins/dist/classes/";
	inline constexpr std::string_view CODERACEDIR    = "/workspaces/swtfe/tfeadmins/dist3/races/";
	inline constexpr std::string_view BUILDRACEDIR   = "/workspaces/swtfe/tfeadmins/dist2/races/";
	inline constexpr std::string_view MAINRACEDIR    = "/workspaces/swtfe/tfeadmins/dist/races/";
	inline constexpr std::string_view MAIN_STARTUP   = "swr";
	inline constexpr std::string_view PORT_STARTUP   = "startup";
	inline constexpr std::string_view MAIN_PORT      = "/workspaces/swtfe/tfeadmins/";
	inline constexpr std::string_view CODING_PORT    = "/workspaces/swtfe/tfeadmins/port/";
	inline constexpr std::string_view SOURCE_DIR     = "/workspaces/swtfe/tfeadmins/src/";

#ifdef OVERLANDCODE
	inline constexpr std::string_view MAINMAPDIR    = BASE_DIR + std::string_view{"dist/maps/"};
	inline constexpr std::string_view BUILDMAPDIR   = BASE_DIR + std::string_view{"dist2/maps/"};
	inline constexpr std::string_view CODEMAPDIR    = BASE_DIR + std::string_view{"dist3/maps/"};
#endif


	// Function signatures modernized for safety
	struct DescriptorData; // Forward declaration

	void send_telcode(int desc, int ddww, int code) noexcept;
	[[nodiscard]] bool check_forks(const DescriptorData& d, std::string_view cmdline) noexcept;

	// Locks as inline variables (should be encapsulated in a singleton in implementation)
	inline bool compilelock = false;
	inline bool bootlock = false;
}
