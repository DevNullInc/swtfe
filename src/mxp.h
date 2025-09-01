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
 * MXP protocol header for enhanced client communication and display features. *
 ****************************************************************************************/

/* strings */
#define MXP_SECURE "\x1B[1"
#define MXP_BEG "\x03"  /* becomes < */
#define MXP_END "\x04"  /* becomes > */
#define MXP_AMP "\x05"  /* becomes & */

/* characters */

#define MXP_BEGc '\x03' /* becomes < */
#define MXP_ENDc '\x04' /* becomes > */
#define MXP_AMPc '\x05' /* becomes & */

/* constructs an MXP tag with < and > around it */

#define MXPTAG(arg) MXP_BEG arg MXP_END

#define ESC "\x1B"  /* esc character */

#define MXPMODE(arg) ESC "[" #arg "z"

/* flags for show_list_to_char */

enum
{
        eItemNothing,   /* item is not readily accessible */
        eItemGet,   /* item on ground */
        eItemDrop,  /* item in inventory */
        eItemBid    /* auction item */
};

#define  TELOPT_MXP        '\x5B'
extern const unsigned char will_mxp_str[];
extern const unsigned char start_mxp_str[];
extern const unsigned char do_mxp_str[];
extern const unsigned char dont_mxp_str[];

/* DECLARE_DO_FUN(do_mxp); */
void      convert_mxp_tags
args((DESCRIPTOR_DATA * d, char *dest, const char *src, int length));
int count_mxp_tags args((DESCRIPTOR_DATA * d, const char *txt, int length));
void send_mxp_stylesheet args((DESCRIPTOR_DATA * d));

/*
 void free_mxpobj_cmds( void );
 */

#define MXP_STYLESHEET_FILE     "../system/mxp.style"

#define IS_MXP(ch) ( (ch) && IS_SET((ch)->act, PLR_MXP) && (ch)->desc && (ch)->desc->mxp_detected == TRUE )
