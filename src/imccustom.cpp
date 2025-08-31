/* Small safe additions: doublexp and gpoint command handlers and globals */
#include "mud.h"

/* Prototype for the info logging function used below */
void info(CHAR_DATA *ch, int level, const char *format, ...)
{
    // Simple stub implementation for info logging
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/* Globals */
bool double_exp = false;
int global_exp_ticks = 0;
sh_int display_ticks = 0;

/* do_doublexp: imm-level command to set ticks of double XP (safe bounds) */
CMDF do_doublexp(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument(argument, arg);

    if (!arg || arg[0] == '\0') {
        send_to_char("Syntax: doublexp on <ticks> | doublexp off\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "on")) {
        if (!is_number(argument)) {
            send_to_char("Specify number of ticks (1-10000).\n\r", ch);
            return;
        }
        int ticks = atoi(argument);
        if (ticks < 1 || ticks > 10000) {
            send_to_char("Ticks must be between 1 and 10000.\n\r", ch);
            return;
        }
        if (double_exp) {
            send_to_char("Double XP already active.\n\r", ch);
            return;
        }
        double_exp = true;
        global_exp_ticks = ticks;
        display_ticks = 0;
        info(NULL, 0, "{G[INFO]:{x {R%s has enabled %d ticks of double XP.{x\n\r", ch->name, ticks);
        send_to_char("Double XP enabled.\n\r", ch);
        return;
    }
    else if (!str_cmp(arg, "off")) {
        if (!double_exp) {
            send_to_char("Double XP is not active.\n\r", ch);
            return;
        }
        double_exp = false;
        global_exp_ticks = 0;
        display_ticks = 0;
        info(NULL, 0, "{G[INFO]:{x {R%s has disabled double XP.{x\n\r", ch->name);
        send_to_char("Double XP disabled.\n\r", ch);
        return;
    }
    send_to_char("Syntax: doublexp on <ticks> | doublexp off\n\r", ch);
}

/* do_gpoint: allows immortals to set a personal goto point, or clear it */
CMDF do_gpoint(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
        return;

    if (!argument || argument[0] == '\0') {
        ch->gpoint = 0;
        send_to_char("Your goto point has been cleared.\n\r", ch);
        return;
    }

    if (!is_number(argument)) {
        send_to_char("Usage: gpoint <room_vnum>\n\r", ch);
        return;
    }
    int v = atoi(argument);
    if (v <= 0) {
        send_to_char("Room vnum must be greater than zero.\n\r", ch);
        return;
    }
    ch->gpoint = v;
    ch_printf(ch, "Goto point set to %d\n\r", v);
}
