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
 *                            Enhanced Betting System Module                             *
 * FEATURES: Advanced number parsing with k/m notation and relative betting              *
 ****************************************************************************************/

#include "mud.hpp"
#include "bet.hpp"

// ============================================================================
// Advanced Number Parsing - Enhanced with security and validation
// ============================================================================

/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.
*/

int advatoi(char *s)
{
        // Enhanced input validation
        if (!s || s[0] == '\0') {
                return 0;
        }
        
        int number = 0;   /* number to be returned */
        int multiplier = 0;   /* multiplier used to get the extra digits right */

        /*
         * as long as the current character is a digit add to current number
         */
        while (isdigit(s[0])) {
                // Security: Prevent overflow during digit accumulation
                if (number > 214748364) { // (INT_MAX / 10) - 1 for safety
                        return 0; // Return 0 on potential overflow
                }
                number = (number * 10) + (*s++ - '0');
        }

        switch (UPPER(s[0]))
        {
        case 'K':
                // Security: Check for multiplication overflow
                if (number > 2147483) { // Max safe value for * 1000
                        return 0;
                }
                number *= (multiplier = 1000);
                ++s;
                break;
        case 'M':
                // Security: Check for multiplication overflow
                if (number > 2147) { // Max safe value for * 1000000
                        return 0;
                }
                number *= (multiplier = 1000000);
                ++s;
                break;
        case '\0':
                break;
        default:
                return 0;   /* not k nor m nor NULL - return 0! */
        }

        /*
         * if any digits follow k/m, add those too 
         */
        while (isdigit(s[0]) && (multiplier > 1))
        {
                /*
                 * the further we get to right, the less the digit 'worth' 
                 */
                multiplier /= 10;
                int additional = (*s++ - '0') * multiplier;
                
                // Security: Check for addition overflow
                if (number > INT_MAX - additional) {
                        return 0; // Return 0 on potential overflow
                }
                number = number + additional;
        }

        /*
         * return 0 if non-digit character was found, other than NULL 
         */
        if (s[0] != '\0' && !isdigit(s[0]))
                return 0;

        /*
         * anything left is likely extra digits (ie: 14k4443  -> 3 is extra) 
         */

        return number;
}

// ============================================================================
// Enhanced Betting Parser - Supports relative and absolute betting
// ============================================================================

/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/
int parsebet(const int currentbet, char *s)
{
        // Enhanced input validation
        if (!s || s[0] == '\0') {
                return 0;
        }
        
        /*
         * if first char is a digit, use advatoi 
         */
        if (isdigit(s[0])) {
                return advatoi(s);
        }
        
        if (s[0] == '+')    /* add percent (default 25%) */
        {
                int percentage;
                if (s[1] == '\0') {
                        percentage = 25; // Default 25%
                } else {
                        percentage = atoi(s + 1);
                        if (percentage < 0 || percentage > 10000) { // Sanity check: 0-10000%
                                return 0;
                        }
                }
                
                // Security: Check for multiplication overflow
                if (currentbet > INT_MAX / (100 + percentage) * 100) {
                        return 0;
                }
                
                return (currentbet * (100 + percentage)) / 100;
        }
        
        if (s[0] == '*' || s[0] == 'x') /* multiply (default is by 2) */
        {
                int multiplier;
                if (s[1] == '\0') {
                        multiplier = 2; // Default x2
                } else {
                        multiplier = atoi(s + 1);
                        if (multiplier < 0 || multiplier > 1000) { // Sanity check: 0-1000x
                                return 0;
                        }
                }
                
                // Security: Check for multiplication overflow
                if (currentbet > 0 && multiplier > INT_MAX / currentbet) {
                        return 0;
                }
                
                return currentbet * multiplier;
        }
        
        return 0;
}
