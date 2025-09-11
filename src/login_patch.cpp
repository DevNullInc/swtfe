/*
 * Login Process Streamlining Patch
 * 
 * This patch modifies the login Process to streamline Account creation and character linking.
 * 
 * Instead of requiring users to enter both Account and character credentials separately,
 * this patch will:
 * 1. Allow character creation to automatically use the Account password
 * 2. Auto-link newly created characters to their Account
 * 3. Simplify the overall login flow
 * 
 * To apply this patch:
 * 1. Include this file in your main comm.cpp or nanny function
 * 2. Add calls to the streamlined login functions where appropriate
 */

#include "mud.hpp"
#include "Account.hpp"

#ifndef ConGetCharSelection
#define ConGetCharSelection  76765  // Use an unused integer value appropriate for your codebase
#endif

// Forward declaration for get_account if not included in Account.h
AccountData *get_account(const char *name);

// Store Account password temporarily for auto-linking
char AccountPassword[MaxStringLength];

/*
 * Function to auto-link character to Account
 * Call this after character creation instead of asking for a second password
 */
void auto_link_character(DescriptorData *d, CharData *ch, AccountData *Account)
{
    if (!d || !ch || !Account)
        return;
    
    // Link character to Account
    ch->PCData->Account = Account;
    
    // Skip password prompt for character
    d->connected = ConGetNewSex; // Move directly to gender selection
    
    // Inform the player
    write_to_buffer(d, "Your character has been automatically linked to your Account.\r\n", 0);
}

/*
 * Replace the Account login Process with this streamlined version
 * This simplifies the initial Account selection/creation
 */
void streamlined_account_login(DescriptorData *d, char *argument)
{
    AccountData *Account;
    char buf[MaxStringLength];
    
    // Check if Account exists
    Account = get_account(argument);
    
    if (Account) {
        // Account exists
        write_to_buffer(d, "Password: ", 0);
        // Disable echo for password input (implementation may vary)
        if (d->character && d->character->desc)
            write_to_buffer(d, "\xFF\xFB\x01", 3); // TELNET WILL SUPPRESS GO AHEAD (example)
        // If your codebase has a macro or function for echo off, use it here.
        d->connected = ConGetOldPassword;
        return;
    }
    
    // New Account
    snprintf(buf, MaxStringLength, "Account '%s' doesn't exist. Create it? (Y/N) ", argument);
    write_to_buffer(d, buf, 0);
    d->connected = ConConfirmNewName;
}

/*
 * Handle existing character selection - streamlined version
 * This lets the user select a character or create a new one
 */
void streamlined_character_selection(DescriptorData *d, AccountData *Account)
{
    int count = 0;
    char buf[MaxStringLength];
    
    write_to_buffer(d, "Characters linked to this Account:\r\n", 0);
    
    // List characters linked to this Account
    // Implementation depends on how characters are stored in Account
    
    // For illustration:
    // for (each character linked to Account) {
    //     count++;
    //     snprintf(buf, MaxStringLength, "%d. %s\r\n", count, character_name);
    //     write_to_buffer(d, buf, 0);
    // }
    
    if (count == 0) {
        write_to_buffer(d, "No characters found. Create a new one.\r\n", 0);
        // Move to character creation
        d->connected = ConGetName;
    d->connected = ConGetCharSelection;
}

#ifndef ConGetCharSelection
#define ConGetCharSelection  76765  // Use an unused integer value appropriate for your codebase
#endif

/*
 * Apply these changes in your nanny function where the connection state 
 * handling happens. The exact implementation will depend on your code structure.
 */
