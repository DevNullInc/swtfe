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
 *                                 AutoBuild Module                                      *
 ****************************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.hpp"
#include "autobuild.hpp"

// ============================================================================
// Constants and Configuration
// ============================================================================
namespace {
    // File handling constants
    constexpr size_t MaxFilenameLength = 512;    // Increased buffer size for safety
    constexpr size_t MaxLineLength = 1024;       // Maximum line length in list files
    constexpr int MaxLinesPerList = 10000;      // Safety limit for list files
    
    // Status messages
    const char* const FileExistsMsg = "File already exists.\n\r";
    const char* const FileNotExistMsg = "File does not exist.\n\r";
    const char* const FileCreatedMsg = "File created successfully.\n\r";
    const char* const TextAddedMsg = "Text added to list.\n\r";
    const char* const LineInvalidMsg = "Line number invalid.\n\r";
    const char* const LineRemovedMsg = "Line removed successfully.\n\r";
    
    // Error messages
    const char* const CreateSyntax = "Syntax: createlist <list filename>\n\r";
    const char* const AddtoSyntax = "Syntax: addtolist <list filename> <text to add>\n\r";
    const char* const ShowSyntax = "Syntax: showlist <list filename>\n\r";
    const char* const RemSyntax = "Syntax: remlist <list filename> <line number>\n\r";
    const char* const LineNumberSyntax = "Line must be referenced by a number.\n\r";
}

// ============================================================================
// Utility Functions
// ============================================================================

/*
 * Build a safe filename path with proper bounds checking
 */
static bool build_list_filename(char* buffer, size_t buffer_size, const char* filename)
{
    if (!buffer || !filename || buffer_size < MaxFilenameLength)
        return false;
        
    // Clear buffer and build path safely
    buffer[0] = '\0';
    int result = snprintf(buffer, buffer_size, "%s%s", ListDir, filename);
    
    // Check for truncation
    if (result < 0 || static_cast<size_t>(result) >= buffer_size)
    {
        buffer[0] = '\0';  // Clear on error
        return false;
    }
    
    return true;
}

/*
 * Safe file operations with proper resource management
 */
static FILE* safe_file_open(const char* filename, const char* mode)
{
    if (!filename || !mode)
        return nullptr;
        
    FCLOSE(fpReserve);
    FILE* fp = fopen(filename, mode);
    
    if (!fp)
    {
        // Restore reserve file handle on failure
        fpReserve = fopen(NullFile, "r");
        return nullptr;
    }
    
    return fp;
}

static void safe_file_close(FILE* fp)
{
    if (fp)
    {
        FCLOSE(fp);
        fpReserve = fopen(NullFile, "r");
    }
}

// ============================================================================
// Section: List Management Commands
// ============================================================================

/*
 * Create a new list file
 * Usage: createlist <filename>
 */
CMDF do_create_list(CharData* ch, char* argument)
{
    char filename[MaxFilenameLength];
    
    // Validate input
    if (!argument || argument[0] == '\0')
    {
        send_to_char(CreateSyntax, ch);
        return;
    }
    
    // Build safe filename
    if (!build_list_filename(filename, sizeof(filename), argument))
    {
        send_to_char("Invalid filename or path too long.\n\r", ch);
        return;
    }
    
    // Check if file already exists
    if (file_exist(filename))
    {
        send_to_char(FileExistsMsg, ch);
        return;
    }
    
    // Create the file
    FILE* fp = safe_file_open(filename, "w");
    if (!fp)
    {
        bug("do_create_list: failed to create file %s", filename);
        send_to_char("Failed to create file.\n\r", ch);
        return;
    }
    
    safe_file_close(fp);
    send_to_char(FileCreatedMsg, ch);
}

/*
 * Add text to an existing list file
 * Usage: addtolist <filename> <text>
 */
CMDF do_addto_list(CharData* ch, char* argument)
{
    char filename[MaxFilenameLength];
    char arg1[MaxInputLength];
    
    argument = one_argument(argument, arg1);
    
    // Validate input
    if (arg1[0] == '\0' || argument[0] == '\0')
    {
        send_to_char(AddtoSyntax, ch);
        return;
    }
    
    // Build safe filename
    if (!build_list_filename(filename, sizeof(filename), arg1))
    {
        send_to_char("Invalid filename or path too long.\n\r", ch);
        return;
    }
    
    // Check if file exists
    if (!file_exist(filename))
    {
        send_to_char(FileNotExistMsg, ch);
        return;
    }
    
    // Append text to file
    append_to_file(filename, argument);
    send_to_char(TextAddedMsg, ch);
}

/*
 * Display contents of a list file with line numbers
 * Usage: showlist <filename>
 */
CMDF do_showlist(CharData* ch, char* argument)
{
    char filename[MaxFilenameLength];
    FILE* fp;
    int line_count = 0;
    char* line_text;
    
    // Validate input
    if (!argument || argument[0] == '\0')
    {
        send_to_char(ShowSyntax, ch);
        return;
    }
    
    // Build safe filename
    if (!build_list_filename(filename, sizeof(filename), argument))
    {
        send_to_char("Invalid filename or path too long.\n\r", ch);
        return;
    }
    
    // Check if file exists
    if (!file_exist(filename))
    {
        send_to_char(FileNotExistMsg, ch);
        return;
    }
    
    // Open and display file contents
    fp = safe_file_open(filename, "r");
    if (!fp)
    {
        send_to_char("Failed to open file for reading.\n\r", ch);
        return;
    }
    
    set_char_color(AtWhite, ch);
    send_to_char("&WList Contents:\n\r", ch);
    set_char_color(AtCyan, ch);
    
    // Read and display each line with line numbers
    while (!feof(fp) && line_count < MaxLinesPerList)
    {
        line_text = fread_line(fp);
        if (!line_text || !strcmp(line_text, ""))
            break;
            
        ch_printf(ch, "&C%4d) &w%s", ++line_count, line_text);
    }
    
    safe_file_close(fp);
    
    // Display summary
    set_char_color(AtBlue, ch);
    ch_printf(ch, "&B[&W%d&B] &zlines in %s\n\r", line_count, argument);
}

/*
 * Remove a specific line from a list file
 * Usage: remlist <filename> <line_number>
 */
CMDF do_remlist(CharData* ch, char* argument)
{
    char filename[MaxFilenameLength];
    char temp_filename[MaxFilenameLength];
    char arg1[MaxInputLength];
    FILE* fp = nullptr;
    FILE* temp_fp = nullptr;
    int total_lines = 0;
    int current_line = 0;
    int target_line;
    char* line_text;
    bool success = false;
    
    argument = one_argument(argument, arg1);
    
    // Validate input
    if (arg1[0] == '\0' || argument[0] == '\0')
    {
        send_to_char(RemSyntax, ch);
        return;
    }
    
    if (!is_number(argument))
    {
        send_to_char(LineNumberSyntax, ch);
        return;
    }
    
    target_line = atoi(argument);
    if (target_line <= 0)
    {
        send_to_char("Line number must be greater than 0.\n\r", ch);
        return;
    }
    
    // Build safe filenames
    if (!build_list_filename(filename, sizeof(filename), arg1))
    {
        send_to_char("Invalid filename or path too long.\n\r", ch);
        return;
    }
    
    if (!build_list_filename(temp_filename, sizeof(temp_filename), arg1))
    {
        send_to_char("Failed to create temporary filename.\n\r", ch);
        return;
    }
    
    // Append .tmp to temp filename
    size_t len = strlen(temp_filename);
    if (len + 5 < sizeof(temp_filename))  // ".tmp" + null terminator
    {
        strcat(temp_filename, ".tmp");
    }
    else
    {
        send_to_char("Temporary filename too long.\n\r", ch);
        return;
    }
    
    // Check if original file exists
    if (!file_exist(filename))
    {
        send_to_char(FileNotExistMsg, ch);
        return;
    }
    
    // First pass: count total lines and validate target line
    fp = safe_file_open(filename, "r");
    if (!fp)
    {
        send_to_char("Failed to open file for reading.\n\r", ch);
        return;
    }
    
    while (!feof(fp) && total_lines < MaxLinesPerList)
    {
        line_text = fread_line(fp);
        if (!line_text || !strcmp(line_text, ""))
            break;
        total_lines++;
    }
    
    safe_file_close(fp);
    
    // Validate target line number
    if (target_line > total_lines)
    {
        send_to_char(LineInvalidMsg, ch);
        return;
    }
    
    // Second pass: copy all lines except target line to temp file
    fp = safe_file_open(filename, "r");
    temp_fp = safe_file_open(temp_filename, "w");
    
    if (!fp || !temp_fp)
    {
        if (fp) safe_file_close(fp);
        if (temp_fp) safe_file_close(temp_fp);
        send_to_char("Failed to open files for processing.\n\r", ch);
        return;
    }
    
    while (!feof(fp))
    {
        line_text = fread_line(fp);
        if (!line_text || !strcmp(line_text, ""))
            break;
            
        current_line++;
        
        // Skip the target line
        if (current_line == target_line)
            continue;
            
        fprintf(temp_fp, "%s", line_text);
    }
    
    safe_file_close(fp);
    safe_file_close(temp_fp);
    
    // Replace original file with temp file
    if (unlink(filename) == 0)
    {
        if (rename(temp_filename, filename) == 0)
        {
            success = true;
        }
        else
        {
            // If rename fails, try to restore from temp
            rename(temp_filename, filename);
        }
    }
    
    // Clean up temp file if it still exists
    unlink(temp_filename);
    
    if (success)
    {
        ch_printf(ch, "Line %d removed successfully.\n\r", target_line);
    }
    else
    {
        send_to_char("Failed to remove line from file.\n\r", ch);
    }
}
