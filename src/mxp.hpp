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
 *                              Mud Extension Protocol Module                            *
 *****************************************************************************************
 * MXP protocol header for enhanced Client communication and display features. *
 ****************************************************************************************/


#pragma once
#include <string>
#include <array>

constexpr auto MXP_SECURE = "\x1B[1";
constexpr auto MXP_BEG = "\x03";  // becomes <
constexpr auto MXP_END = "\x04";  // becomes >
constexpr auto MXP_AMP = "\x05";  // becomes &

constexpr char MXP_BEGc = '\x03'; // becomes <
constexpr char MXP_ENDc = '\x04'; // becomes >
constexpr char MXP_AMPc = '\x05'; // becomes &

inline std::string MXPTAG(const std::string& arg) { return MXP_BEG + arg + MXP_END; } // for <tag>

constexpr auto ESC = "\x1B"; // esc character
inline std::string MXPMODE(const std::string& arg) { return ESC + "[" + arg + "z"; } // for setting modes

/* flags for show_list_to_char */


enum class ItemShowType : int {
        Nothing,   // item is not readily accessible
        Get,       // item on ground
        Drop,      // item in inventory
        Bid        // auction item
};


constexpr char TELOPT_MXP = '\x5B';
extern const std::array<unsigned char, 3> will_mxp_str;
extern const std::array<unsigned char, 3> start_mxp_str;
extern const std::array<unsigned char, 3> do_mxp_str;
extern const std::array<unsigned char, 3> dont_mxp_str;

void convert_mxp_tags(DescriptorData* d, std::string& dest, const std::string& src, int length); // convert < > & to MXP safe versions
int count_mxp_tags(DescriptorData* d, const std::string& txt, int length); // count number of MXP tags in a string
void send_mxp_stylesheet(DescriptorData* d); // send the MXP stylesheet to the Client

constexpr auto MXP_STYLESHEET_FILE = "../system/mxp.style";

inline bool IS_MXP(const CharData* ch) { // is the character using MXP?
        return ch && IS_SET(ch->act, PLR_MXP) && ch->desc && ch->desc->MxpDetected == TRUE; // and the Client supports it
}
