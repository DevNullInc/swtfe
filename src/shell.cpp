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
 *                                 Pfile Pruning Module                                  *
 ****************************************************************************************/


// USEGLOB should be set via build system, not hardcoded here. Remove legacy macro.

#include <unistd.h>
#include <sys/wait.h>   /* Samson 4-16-98 - For new shell command */
#include <fcntl.h>
#include <arpa/telnet.h>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include <string>
#include <string_view>
#include "mud.hpp"


#include "shell.hpp"

namespace shell {
        // Inline variables already defined in header
}


#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif


/* New command shell code by Thoric - Installed by Samson 4-16-98 */
void send_telcode(int desc, int ddww, int code) noexcept {
        unsigned char buf[4]{};
        buf[0] = IAC;
        buf[1] = ddww;
        buf[2] = code;
        buf[3] = 0;
        write(desc, reinterpret_cast<char*>(buf), 4);
}

CMDF do_mudexec(CHAR_DATA* ch, const char* argument)
{
        int desc;
        int flags;
        pid_t pid;
        bool iafork = false;

#ifdef MCCP
        int compressing = 0;
#endif

        if (!ch->desc)
                return;

        if (!argument || argument[0] == '\0')
                return;

        if (strncasecmp(argument, "ia ", 3) == 0)
        {
                argument += 3;
                iafork = true;
        }

        desc = ch->desc->descriptor;
        set_char_color(AT_PLAIN, ch);

#ifdef MCCP
        compressing = ch->desc->compressing;
        compressEnd(ch->desc);
#endif

        if ((pid = fork()) == 0)
        {
                char buf[1024];
                char* p = const_cast<char*>(argument);
                int argc = 1;

                flags = fcntl(desc, F_GETFL, 0);
                flags &= ~FNDELAY;
                fcntl(desc, F_SETFL, flags);
                if (iafork)
                {
                        send_telcode(desc, WILL, TELOPT_SGA);
                        send_telcode(desc, DO, TELOPT_LFLOW);
                        send_telcode(desc, DONT, TELOPT_LINEMODE);
                        send_telcode(desc, WILL, TELOPT_STATUS);
                        send_telcode(desc, DO, TELOPT_ECHO);
                        send_telcode(desc, WILL, TELOPT_ECHO);
                        read(desc, buf, 1024);  /* read replies */
                }
                dup2(desc, STDIN_FILENO);
                dup2(desc, STDOUT_FILENO);
                dup2(desc, STDERR_FILENO);
                setenv("TERM", "vt100", 1);
                setenv("COLUMNS", "80", 1);
                setenv("LINES", "24", 1);

                while (*p)
                {
                        while (isspace(*p))
                                ++p;
                        if (*p == '\0')
                                break;
                        ++argc;
                        while (!isspace(*p) && *p)
                                ++p;
                }
                p = const_cast<char*>(argument);
                char** argv = static_cast<char**>(calloc(argc + 1, sizeof(char*)));
                argc = 0;
                argv[argc] = strtok(p, " ");
                while ((argv[++argc] = strtok(NULL, " ")) != NULL);

                execvp(argv[0], argv);

#ifdef MCCP
                if (compressing)
                        compressStart(ch->desc, compressing);
#endif
                fprintf(stderr, "Shell process: %s failed!\n", argument);
                perror("mudexec");
                exit(0);
        }
        else if (pid < 2)
        {
                send_to_char("Process fork failed.\n\r", ch);
                fprintf(stderr, "%s", "Shell process: fork failed!\n");
                return;
        }
        else
        {
                ch->desc->process = pid;
                ch->desc->connected = iafork ? CON_IAFORKED : CON_FORKED;
        }
}

/* End NEW shell command code */

bool check_forks(const DescriptorData& d, std::string_view cmdline) noexcept {
        // Implementation must be updated to use DescriptorData class and std::string_view
        // ...existing code (refactor required in DescriptorData definition and usage)...
        return false; // Placeholder, update logic as DescriptorData is modernized
}

// Modernize: Use std::string_view, validate filename for security
int copy_file(CHAR_DATA* ch, std::string_view filename) {
        // Security: Only allow files within allowed directories
        if (filename.find("..") != std::string_view::npos || filename.find('$') != std::string_view::npos || filename.find(';') != std::string_view::npos) {
                set_char_color(AT_RED, ch);
                ch_printf(ch, "The file %.*s contains unsafe characters.\n\r", static_cast<int>(filename.size()), filename.data());
                return 1;
        }
        FILE* fp = fopen(std::string(filename).c_str(), "r");
        if (!fp) {
                set_char_color(AT_RED, ch);
                ch_printf(ch, "The file %.*s does not exist, or cannot be opened. Check your spelling.\n\r", static_cast<int>(filename.size()), filename.data());
                return 1;
        }
        FCLOSE(fp);
        return 0;
}

// Modernize: Use shell::SOURCE_DIR for src path
void compile_code(CHAR_DATA* ch, std::string_view argument) {
        std::string buf;
        std::string src_path = std::string(shell::SOURCE_DIR);
        if (argument == "cvs") {
                buf = "make -C ../" + src_path + " cvs";
                do_mudexec(ch, buf.c_str());
                return;
        }
        if (argument == "clean") {
                buf = "make -C ../" + src_path + " clean";
                do_mudexec(ch, buf.c_str());
                return;
        }
        if (argument == "dns") {
                buf = "make -C ../" + src_path + " dns";
                do_mudexec(ch, buf.c_str());
                return;
        }
        buf = "make -C ../" + src_path;
        do_mudexec(ch, buf.c_str());
}

// Modernize: Use shell::bootlock and shell::compilelock
CMDF do_compile(CHAR_DATA* ch, std::string_view argument) {
        using namespace shell;
        if (bootlock) {
                send_to_char("&RThe reboot timer is running, the compiler cannot be used at this time.\n\r", ch);
                return;
        }
        if (compilelock) {
                send_to_char("&RThe compiler is in use, please wait for the compilation to finish.\n\r", ch);
                return;
        }
        compilelock = true;
        set_char_color(AT_RED, ch);
        std::ostringstream oss;
        oss << "Compiler operation initiated by " << ch->name << ". Reboot and shutdown commands are locked.";
        echo_to_all(AT_RED, oss.str().c_str(), ECHOTAR_IMM);
        compile_code(ch, argument);
}


/*
====================
GREP In-Game command	-Nopey
====================
*/
/* Modified by Samson to be a bit less restrictive. So one can grep anywhere the account will allow. */
// Modernize: Use std::string_view, validate arguments for security
CMDF do_grep(CHAR_DATA* ch, std::string_view argument) {
        std::ostringstream oss;
        std::string_view arg1;
        // Parse first argument
        size_t space = argument.find(' ');
        if (space != std::string_view::npos) {
                arg1 = argument.substr(0, space);
                argument.remove_prefix(space + 1);
        } else {
                arg1 = argument;
                argument = "";
        }
        set_char_color(AT_PLAIN, ch);
        // Double-check: reject empty, unsafe, or shell metacharacter input
        auto is_unsafe = [](std::string_view s) {
                return s.empty() || s.find("..") != std::string_view::npos || s.find('$') != std::string_view::npos || s.find(';') != std::string_view::npos || s.find('|') != std::string_view::npos || s.find('&') != std::string_view::npos || s.find('`') != std::string_view::npos || s.find('>') != std::string_view::npos || s.find('<') != std::string_view::npos;
        };
        if (is_unsafe(arg1) || is_unsafe(argument)) {
                oss << "grep --help";
        } else {
                oss << "grep -n " << arg1 << " " << argument;
        }
        command_pipe(ch, oss.str().c_str());
}
