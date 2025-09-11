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
 *                      Color Module -- Allow user customizable Colors.                  *
 *                                         --Matthew                                     *
 *                             Enhanced ANSI parser by Samson                            *
 ****************************************************************************************/


#pragma once
#include <string>
#include <string_view>
#include <memory>

#define SAMSONCOLOR /* To interact with other snippets */
#ifndef MSL
#define MSL MaxStringLength
#endif
#ifndef MIL
#define MIL MaxInputLength
#endif


void reset_colors(CharData *ch);
void reset_colors(std::shared_ptr<CharData> ch);
const char *color_str(sh_int AType, CharData *ch);
std::string color_str(sh_int AType, std::shared_ptr<CharData> ch);
const char *const_color_align(const char *argument, int size, int align);
std::string const_color_align(std::string_view argument, int size, int align);
void send_to_desc_color(const char *txt, DescriptorData *d);
void send_to_desc_color(std::string_view txt, std::shared_ptr<DescriptorData> d);


/*
 * Color Alignment Parameters
 */
#define AlignLeft	1
#define AlignCenter	2
#define AlignRight	3

/* These are the ANSI codes for foreground text colors */
#define AnsiBlack    	"\033[0;30m"
#define AnsiDred    	"\033[0;31m"
#define AnsiDgreen     "\033[0;32m"
#define AnsiOrange    	"\033[0;33m"
#define AnsiDblue    	"\033[0;34m"
#define AnsiPurple    	"\033[0;35m"
#define AnsiCyan	  	"\033[0;36m"
#define AnsiGrey		"\033[0;37m"
#define AnsiDgrey	"\033[1;30m"
#define AnsiRed		"\033[1;31m"
#define AnsiGreen	"\033[1;32m"
#define AnsiYellow   	"\033[1;33m"
#define AnsiBlue		"\033[1;34m"
#define AnsiPink		"\033[1;35m"
#define AnsiLblue   	"\033[1;36m"
#define AnsiWhite   	"\033[1;37m"
#define AnsiReset	"\033[0m"

/* These are the ANSI codes for blinking foreground text colors */
#define BlinkBlack		"\033[0;5;30m"
#define BlinkDred		"\033[0;5;31m"
#define BlinkDgreen		"\033[0;5;32m"
#define BlinkOrange		"\033[0;5;33m"
#define BlinkDblue		"\033[0;5;34m"
#define BlinkPurple		"\033[0;5;35m"
#define BlinkCyan		"\033[0;5;36m"
#define BlinkGrey		"\033[0;5;37m"
#define BlinkDgrey		"\033[1;5;30m"
#define BlinkRed			"\033[1;5;31m"
#define BlinkGreen		"\033[1;5;32m"
#define BlinkYellow		"\033[1;5;33m"
#define BlinkBlue		"\033[1;5;34m"
#define BlinkPink		"\033[1;5;35m"
#define BlinkLblue		"\033[1;5;36m"
#define BlinkWhite		"\033[1;5;37m"

/* These are the ANSI codes for background colors */
#define BackBlack 	"\033[40m"
#define BackDred  	"\033[41m"
#define BackDgreen	"\033[42m"
#define BackOrange     "\033[43m"
#define BackDblue      "\033[44m"
#define BackPurple     "\033[45m"
#define BackCyan       "\033[46m"
#define BackGrey       "\033[47m"

/* Other miscelaneous ANSI tags that can be used */
#define AnsiBold		"\033[1m"   /* For bright color stuff */
#define AnsiItalic	"\033[3m"   /* Italic text */
#define AnsiUnderline  "\033[4m"   /* Underline text */
#define AnsiBlink	"\033[5m"   /* Blinking text */
#define AnsiReverse    "\033[7m"   /* Reverse colors */
#define AnsiStrikeout  "\033[9m"   /* Overstrike line */

#define AtBlack    	0
#define AtBlood    	1
#define AtDgreen       2
#define AtOrange    	3
#define AtDblue    	4
#define AtPurple    	5
#define AtCyan	  	6
#define AtGrey		7
#define AtDgrey		8
#define AtRed		9
#define AtGreen		10
#define AtYellow   	11
#define AtBlue		12
#define AtPink		13
#define AtLblue   	14
#define AtWhite   	15
#define AtBlink   	16

#define AtWhiteBlink	   AtWhite + AtBlink
#define AtRedBlink	   AtRed + AtBlink

#define AtPlain		17
#define AtAction		18
#define AtSay		19
#define AtGossip		20
#define AtYell		21
#define AtTell		22
#define AtHit		23
#define AtHitme		24
#define AtImmort		25
#define AtHurt		26
#define AtFalling	27
#define AtDanger		28
#define AtMagic		29
#define AtConsider	30
#define AtReport		31
#define AtPoison		32
#define AtSocial		33
#define AtDying		34
#define AtDead		35
#define AtSkill		36
#define AtLines	37
#define AtDamage		38
#define AtFlee		39
#define AtRmname		40
#define AtRmdesc		41
#define AtObject		42
#define AtPerson		43
#define AtList		44
#define AtBye		45
#define AtGold		46
#define AtGtell		47
#define AtNote		48
#define AtHungry		49
#define AtThirsty	50
#define AtFire		51
#define AtSober		52
#define AtWearoff	53
#define AtExits		54
#define AtScore		55
#define AtReset		56
#define AtLog		57
#define AtDiemsg		58
#define AtWartalk      59
#define AtArena        60
#define AtMuse         61
#define AtThink        62
#define AtAflags      	63  /* Added by Samson 9-29-98 for area flag display line */
#define AtWho	    	64  /* Added by Samson 9-29-98 for wholist */
#define AtRacetalk   	65  /* Added by Samson 9-29-98 for version 1.4 code */
#define AtIgnore     	66  /* Added by Samson 9-29-98 for version 1.4 code */
#define AtWhisper    	67  /* Added by Samson 9-29-98 for version 1.4 code */
#define AtDivider    	68  /* Added by Samson 9-29-98 for version 1.4 code */
#define AtMorph      	69  /* Added by Samson 9-29-98 for version 1.4 code */
#define AtShout		70  /* Added by Samson 9-29-98 for shout channel */
#define AtRflags		71  /* Added by Samson 12-20-98 for room flag display line */
#define AtStype		72  /* Added by Samson 12-20-98 for sector display line */
#define AtAname		73  /* Added by Samson 12-20-98 for filename display line */
#define AtAuction      74  /* Added by Samson 12-25-98 for auction channel */
#define AtScore2		75  /* Added by Samson 2-3-99 for DOTD code */
#define AtScore3		76  /* Added by Samson 2-3-99 for DOTD code */
#define AtScore4		77  /* Added by Samson 2-3-99 for DOTD code */
#define AtWho2		78  /* Added by Samson 2-3-99 for DOTD code */
#define AtWho3		79  /* Added by Samson 2-3-99 for DOTD code */
#define AtWho4		80  /* Added by Samson 2-3-99 for DOTD code */
#define AtIntermud     81  /* Added by Samson 1-15-01 for Intermud3 Channels */
#define AtHelp		82  /* Added by Samson 1-15-01 for helpfiles */
#define AtWho5		83  /* Added by Samson 2-7-01 for guild names on who */
#define AtScore5       84  /* Added by Samson 1-14-02 */
#define AtWho6		85  /* Added by Samson 1-14-02 */
#define AtWho7		86  /* Added by Samson 1-14-02 */
#define AtPrac		87  /* Added by Samson 1-21-02 */
#define AtPrac2		88  /* Added by Samson 1-21-02 */
#define AtPrac3		89  /* Added by Samson 1-21-02 */
#define AtPrac4		90  /* Added by Samson 1-21-02 */
#define AtMxpprompt    91  /* Added by Samson 2-27-02 */
#define AtGuildtalk    92  /* Added by Tarl 28 Nov 02 */
#define AtShip         93  /* Added by Greven August 19 03 for SWR port */
#define AtClan         94  /* Added by Greven August 19 03 for SWR port */
#define AtOoc          95  /* Added by Greven August 19 03 for SWR port */
#define AtAvatar	    96  /* Added by Greven August 19 03 for SWR port */
#define AtShuttle		97  /* Added by Gavin - 18/12/03 */

/* Should ALWAYS be one more than the last numerical value in the list */
#define MaxColors    98

extern const sh_int default_set[MaxColors];
