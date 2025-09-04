/*
 * Login Process Streamlining Patch
 * 
 * This patch modifies the login process to streamline account creation and character linking.
 * 
 * Instead of requiring users to enter both account and character credentials separately,
 * this patch will:
 * 1. Allow character creation to automatically use the account password
 * 2. Auto-link newly created characters to their account
 * 3. Simplify the overall login flow
 * 
 * To apply this patch:
 * 1. Include this file in your main comm.cpp or nanny function
 * 2. Add calls to the streamlined login functions where appropriate
 */

#include "mud.hpp"
#include "account.hpp"

#ifndef CON_GET_CHAR_SELECTION
#define CON_GET_CHAR_SELECTION  76765  // Use an unused integer value appropriate for your codebase
#endif

// Forward declaration for get_account if not included in account.h
ACCOUNT_DATA *get_account(const char *name);

// Store account password temporarily for auto-linking
char account_password[MAX_STRING_LENGTH];

/*
 * Function to auto-link character to account
 * Call this after character creation instead of asking for a second password
 */
void auto_link_character(DESCRIPTOR_DATA *d, CHAR_DATA *ch, ACCOUNT_DATA *account)
{
    if (!d || !ch || !account)
        return;
    
    // Link character to account
    ch->pcdata->account = account;
    
    // Skip password prompt for character
    d->connected = CON_GET_NEW_SEX; // Move directly to gender selection
    
    // Inform the player
    write_to_buffer(d, "Your character has been automatically linked to your account.\r\n", 0);
}

/*
 * Replace the account login process with this streamlined version
 * This simplifies the initial account selection/creation
 */
void streamlined_account_login(DESCRIPTOR_DATA *d, char *argument)
{
    ACCOUNT_DATA *account;
    char buf[MAX_STRING_LENGTH];
    
    // Check if account exists
    account = get_account(argument);
    
    if (account) {
        // Account exists
        write_to_buffer(d, "Password: ", 0);
        // Disable echo for password input (implementation may vary)
        if (d->character && d->character->desc)
            write_to_buffer(d, "\xFF\xFB\x01", 3); // TELNET WILL SUPPRESS GO AHEAD (example)
        // If your codebase has a macro or function for echo off, use it here.
        d->connected = CON_GET_OLD_PASSWORD;
        return;
    }
    
    // New account
    snprintf(buf, MAX_STRING_LENGTH, "Account '%s' doesn't exist. Create it? (Y/N) ", argument);
    write_to_buffer(d, buf, 0);
    d->connected = CON_CONFIRM_NEW_NAME;
}

/*
 * Handle existing character selection - streamlined version
 * This lets the user select a character or create a new one
 */
void streamlined_character_selection(DESCRIPTOR_DATA *d, ACCOUNT_DATA *account)
{
    int count = 0;
    char buf[MAX_STRING_LENGTH];
    
    write_to_buffer(d, "Characters linked to this account:\r\n", 0);
    
    // List characters linked to this account
    // Implementation depends on how characters are stored in account
    
    // For illustration:
    // for (each character linked to account) {
    //     count++;
    //     snprintf(buf, MAX_STRING_LENGTH, "%d. %s\r\n", count, character_name);
    //     write_to_buffer(d, buf, 0);
    // }
    
    if (count == 0) {
        write_to_buffer(d, "No characters found. Create a new one.\r\n", 0);
        // Move to character creation
        d->connected = CON_GET_NAME;
    d->connected = CON_GET_CHAR_SELECTION;
}

#ifndef CON_GET_CHAR_SELECTION
#define CON_GET_CHAR_SELECTION  76765  // Use an unused integer value appropriate for your codebase
#endif

/*
 * Apply these changes in your nanny function where the connection state 
 * handling happens. The exact implementation will depend on your code structure.
 */
