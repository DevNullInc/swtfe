/****************************************************************************************
 * ████████╗██╗  ██╗███████╗    ███████╗██╗███╗   ██╗ █████╗ ██╗                        *
 * ╚══██╔══╝██║  ██║██╔════╝    ██╔════╝██║████╗  ██║██╔══██╗██║                        *
 *    ██║   ███████║█████╗      █████╗  ██║██╔██╗ ██║███████║██║                        *
 *    ██║   ██╔══██║██╔══╝      ██╔══╝  ██║██║╚██╗██║██╔══██║██║                        *
 *    ██║   ██║  ██║███████╗    ██║     ██║██║ ╚████║██║  ██║███████╗                   *
 *    ╚═╝   ╚═╝  ╚═╝╚══════╝    ╚═╝     ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝                   *
 *                                                                                       *
 * ███████╗██████╗ ██╗███████╗ ██████╗ ██████╗ ███████╗                                 *
 * ██╔════╝██╔══██╗██║██╔════╝██╔═══██╗██╔══██╗██╔════╝                                 *
 * █████╗  ██████╔╝██║███████╗██║   ██║██║  ██║█████╗                                   *
 * ██╔══╝  ██╔═══╝ ██║╚════██║██║   ██║██║  ██║██╔══╝                                   *
 * ███████╗██║     ██║███████║╚██████╔╝██████╔╝███████╗                                 *
 * ╚══════╝╚═╝     ╚═╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝                                 *
 *                                                                                       *
 *                       /dev/null Industries Presents                                  *
 *                                                                                       *
 ****************************************************************************************
 *                                                                                       *
 * THE FINAL EPISODE Code additions and changes from the Star Wars Reality code copyright (c) *
 * 2003 by Michael Ervin, Mark Gottselig, Gavin Mogan                                    *
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
 *                $Id: starsystem.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#include "mud.h"
#include "body.h"

space_data::space_data()
{
        next = NULL;
        prev = NULL;
        first_ship = NULL;
        last_ship = NULL;
        first_missile = NULL;
        last_missile = NULL;
        first_planet = NULL;
        last_planet = NULL;
        first_dock = NULL;
        last_dock = NULL;
        filename = NULL;
        name = NULL;

}

space_data::~space_data()
{
        if (name != NULL)
                STRFREE(name);
        if (filename != NULL)
                STRFREE(filename);
        name = NULL;
        filename = NULL;
}
