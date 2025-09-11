/* Modernized for C++23/x64 */
#pragma once
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
 *                                SWR OLC Commands module                                *
 ****************************************************************************************/

/* Only need to define functions that other functions call */

#define DeclareDoFun( fun ) CMDFV (fun)( CharData * ch, const char * argument)
#define DeclareSpellFun( fun ) SPELLF (fun)( int sn, int level, CharData *ch, void *vo )

DeclareDoFun(skill_notfound);

DeclareDoFun(do_at);
DeclareDoFun(do_auction);
DeclareDoFun(do_board);
DeclareDoFun(do_boardlook);
DeclareDoFun(do_cedit);
DeclareDoFun(do_closehatch);
DeclareDoFun(do_command);
DeclareDoFun(do_exits);
DeclareDoFun(do_get);
DeclareDoFun(do_glance);
DeclareDoFun(do_goto);
DeclareDoFun(do_hail);
DeclareDoFun(do_help);
DeclareDoFun(do_leaveship);
DeclareDoFun(do_loadarea);
DeclareDoFun(do_look);
DeclareDoFun(do_ls);
DeclareDoFun(do_memory);
DeclareDoFun(do_openhatch);
DeclareDoFun(do_quit);
DeclareDoFun(do_recall);
DeclareDoFun(do_remove);
DeclareDoFun(do_restoretime);
DeclareDoFun(do_return);
DeclareDoFun(do_return);
DeclareDoFun(do_revert);
DeclareDoFun(do_sacrifice);
DeclareDoFun(do_say);
DeclareDoFun(do_sit);
DeclareDoFun(do_split);
DeclareDoFun(do_save);
DeclareDoFun(do_quaff);
DeclareDoFun(do_takedrug);
DeclareDoFun(do_oldscore);
DeclareDoFun(do_rreset);
DeclareDoFun(do_reset);
DeclareDoFun(do_yell);
DeclareDoFun(do_hide);
DeclareDoFun(do_emote);
DeclareDoFun(do_sleep);
DeclareDoFun(do_setblaster);
DeclareDoFun(do_pick);
DeclareDoFun(do_makeshop);
DeclareDoFun(do_redit);
DeclareDoFun(do_backup);
DeclareDoFun(do_wake);
DeclareDoFun(do_comment);
DeclareDoFun(do_affected);
DeclareDoFun(do_immscore);
DeclareDoFun(do_who);
DeclareDoFun(do_train);
DeclareDoFun(do_savearea);

/*
 * Fight Commands
 */
DeclareDoFun(do_bite);
DeclareDoFun(do_claw);
DeclareDoFun(do_flee);
DeclareDoFun(do_kick);
DeclareDoFun(do_punch);
DeclareDoFun(do_shout);
DeclareDoFun(do_sting);
DeclareDoFun(do_tail);



/* MXP */
DeclareDoFun(do_mxp);

/* Spells */
DeclareSpellFun(spell_notfound);
DeclareSpellFun(spell_smaug);
DeclareSpellFun(spell_null);
DeclareSpellFun(spell_fireball);
DeclareSpellFun(spell_lightning_bolt);
DeclareSpellFun(spell_cure_poison);
DeclareSpellFun(spell_poison);
DeclareSpellFun(spell_cure_blindness);
