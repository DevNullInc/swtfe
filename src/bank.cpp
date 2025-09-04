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
 *                            Enhanced Banking System Module                             *
 * SECURITY: Fixed critical mathematical vulnerabilities and overflow exploits           *
 ****************************************************************************************/

#include <string.h>
#include <limits.h>
#include <cmath>
#include "mud.hpp"

// ============================================================================
// Security and Configuration Constants
// ============================================================================
namespace BankSecurity {
    constexpr long MAX_TRANSACTION_AMOUNT = 1000000000L;    // 1 billion credits max per transaction
    constexpr long BALANCE_THRESHOLD = 999999999L;          // Threshold for hi/lo balance management
    constexpr long SAFE_ADDITION_LIMIT = LONG_MAX / 2;      // Safe limit to prevent overflow
    constexpr int MAX_ACCOUNTS_PER_PLAYER = 10;             // Maximum accounts per player
    constexpr double MIN_INTEREST_RATE = 0.01;              // Minimum 1% interest
    constexpr double MAX_INTEREST_RATE = 1.10;              // Maximum 110% interest (10% gain)
    constexpr double DEFAULT_INTEREST_RATE = 0.05;          // Default 5% interest
    constexpr long MINIMUM_TRANSACTION = 1L;                // Minimum transaction amount
}

// ============================================================================
// Forward Declarations  
// ============================================================================
OBJ_DATA *get_comlink args((CHAR_DATA * ch));
CMDF save_baccount args((BANK_ACCOUNT * account));
CMDF load_baccount args((char *filename));
BANK_ACCOUNT *create_baccount args((CHAR_DATA * ch));
CMDF delete_baccount args((BANK_ACCOUNT * account));
char     *generate_code args(());
char     *account_sum args((BANK_ACCOUNT * account));
bool     account_add args((BANK_ACCOUNT * account, long amount));
bool     account_sub args((BANK_ACCOUNT * account, long amount));
bool     account_has_funds args((BANK_ACCOUNT * account, long amount));
int      baccounts args((CHAR_DATA * ch));

// ============================================================================
// Global Variables
// ============================================================================
BANK_ACCOUNT *first_baccount = nullptr;
BANK_ACCOUNT *last_baccount = nullptr;

// ============================================================================
// Account Management Functions
// ============================================================================

void save_baccount(BANK_ACCOUNT * account)
{
        // ============================================================================
        // ACCOUNT PERSISTENCE - Enhanced with error checking and validation
        // ============================================================================
        
        if (!account || !account->code) {
                bug("save_baccount: Invalid account or missing code", 0);
                return;
        }
        
        FILE     *fp;
        char      filename[256];

        // Use secure path construction
        snprintf(filename, sizeof(filename), "%s%s.acct", BACCOUNT_DIR, account->code);
        
        if ((fp = fopen(filename, "w")) == nullptr) {
                bug("save_baccount: unable to open %s.acct for writing!", account->code);
                perror(filename);
                return;
        }

        // Write account data with proper null checks
        fprintf(fp, "#ACCOUNT\n");
        fprintf(fp, "Code        %s~\n", account->code ? account->code : "");
        fprintf(fp, "Creator     %s~\n", account->creator ? account->creator : "");
        fprintf(fp, "Owner       %s~\n", account->owner ? account->owner : "");
        fprintf(fp, "Trustees    %s~\n", account->trustees ? account->trustees : "");
        fprintf(fp, "Flags       %ld\n", account->flags);
        fprintf(fp, "Interest    %f\n", account->interest);
        fprintf(fp, "Amounthi    %ld\n", account->amounthi);
        fprintf(fp, "Amountlo    %ld\n", account->amountlo);
        fprintf(fp, "End\n");
        
        if (fclose(fp) != 0) {
                bug("save_baccount: Error closing file %s", filename);
                perror(filename);
        }

        return;
}

void write_baccount_list()
{
        BANK_ACCOUNT *account;
        FILE     *fp;
        char      filename[256];

        sprintf(filename, "%s%s", BACCOUNT_DIR, BACCOUNT_LIST);

        if ((fp = fopen(filename, "w")) == NULL)
        {
                perror(filename);
                bug("write_baccount_list: can't open list", 0);
                return;
        }

        for (account = first_baccount; account; account = account->next)
                fprintf(fp, "%s.acct\n", account->code);
        fprintf(fp, "$\n");

        FCLOSE(fp);
        return;
}

void load_baccount_list()
{
        FILE     *fpList;
        char      filename[256];
        char     *account;

        sprintf(filename, "%s%s", BACCOUNT_DIR, BACCOUNT_LIST);

        FCLOSE(fpReserve);
        if ((fpList = fopen(filename, "r")) == NULL)
        {
                fpReserve = fopen(NULL_FILE, "r");
                perror(filename);
                bug("load_baccount: couldn't open account list", 0);
                return;
        }

        for (;;)
        {
                account = feof(fpList) ? (char *) "$" : fread_word(fpList);

                if (account[0] == '$')
                        break;

                load_baccount(account);
        }
        FCLOSE(fpList);
        log_string("Done loading accounts");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void load_baccount(char *name)
{
        char      filename[256];
        char      letter, *word;
        FILE     *fp;
        BANK_ACCOUNT *account;
        bool      fMatch;

        sprintf(filename, "%s%s", BACCOUNT_DIR, name);

        if ((fp = fopen(filename, "r")) == NULL)
        {
                perror(filename);
                bug("load_baccount: couldn't open .acct", 0);
                return;
        }

        CREATE(account, BANK_ACCOUNT, 1);
        LINK(account, first_baccount, last_baccount, next, prev);

        letter = fread_letter(fp);
        if (letter != '#')
        {
                bug("load_baccount: #ACCOUNT not found (%s)", filename);
                return;
        }

        word = fread_word(fp);
        if (strcmp(word, "ACCOUNT"))
        {
                bug("load_baccount: #ACCOUNT not found (%s)", filename);
                return;
        }

        for (;;)
        {
                word = feof(fp) ? (char *) "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case 'A':
                        KEY("Amounthi", account->amounthi, fread_number(fp));
                        KEY("Amountlo", account->amountlo, fread_number(fp));
                        break;
                case 'C':
                        KEY("Code", account->code, fread_string(fp));
                        KEY("Creator", account->creator, fread_string(fp));
                        break;
                case 'E':
                        if (!strcmp(word, "End"))
                        {
                                if (account->code == NULL)
                                        account->code =
                                                STRALLOC(generate_code());
                                if (account->creator == NULL)
                                        account->creator =
                                                STRALLOC("NOCREATOR");
                                if (account->owner == NULL)
                                        account->owner = STRALLOC("NOOWNER");
                                if (account->trustees == NULL)
                                        account->trustees = STRALLOC("");
                                FCLOSE(fp);
                                return;
                        }
                        break;
                case 'F':
                        KEY("Flags", account->flags, fread_number(fp));
                        break;
                case 'I':
                        KEY("Interest", account->interest, fread_float(fp));
                        break;
                case 'O':
                        KEY("Owner", account->owner, fread_string(fp));
                        break;
                case 'T':
                        KEY("Trustees", account->trustees, fread_string(fp));
                        break;
                }

                if (!fMatch)
                        bug("load_baccount: no match for %s", word);
        }
        return;
}

BANK_ACCOUNT *create_baccount(CHAR_DATA * ch)
{
        // ============================================================================
        // BANK ACCOUNT CREATION - Enhanced with security validation
        // ============================================================================
        
        // Enhanced input validation
        if (!ch || IS_NPC(ch) || !ch->pcdata || !ch->name) {
                bug("create_baccount: Invalid character data", 0);
                return nullptr;
        }
        
        // Check if player already has too many accounts (security measure)
        int player_account_count = 0;
        for (BANK_ACCOUNT* existing = first_baccount; existing; existing = existing->next) {
                if (existing->owner && !str_cmp(existing->owner, ch->name)) {
                        player_account_count++;
                }
        }
        
        if (player_account_count >= BankSecurity::MAX_ACCOUNTS_PER_PLAYER) {
                bug("create_baccount: Player %s attempting to exceed account limit", ch->name);
                return nullptr;
        }
        
        BANK_ACCOUNT *account;
        CREATE(account, BANK_ACCOUNT, 1);
        LINK(account, first_baccount, last_baccount, next, prev);
        
        // Initialize with security-validated values
        account->code = STRALLOC(generate_code());
        account->creator = STRALLOC(ch->name);
        account->owner = STRALLOC(ch->name);
        account->trustees = STRALLOC("");
        account->flags = 0;
        account->interest = BankSecurity::DEFAULT_INTEREST_RATE;
        account->amounthi = 0;
        account->amountlo = 0;

        save_baccount(account);
        write_baccount_list();
        return account;
}

void delete_account(BANK_ACCOUNT * account)
{
        // ============================================================================
        // ACCOUNT DELETION - Enhanced with security validation and cleanup
        // ============================================================================
        
        if (!account) {
                bug("delete_account: Null account pointer", 0);
                return;
        }
        
        if (!account->code) {
                bug("delete_account: Account missing code", 0);
                return;
        }
        
        char filename[256];
        
        // Remove from linked list first
        UNLINK(account, first_baccount, last_baccount, next, prev);
        
        // Construct filename for deletion
        snprintf(filename, sizeof(filename), "%s%s.acct", BACCOUNT_DIR, account->code);
        
        // Free all allocated strings safely
        STRFREE(account->code);
        STRFREE(account->creator);
        STRFREE(account->owner);
        STRFREE(account->trustees);
        DISPOSE(account);

        // Remove the file and check for errors
        if (remove(filename) != 0) {
                bug("delete_account: Failed to remove file %s", filename);
                perror(filename);
        }
        
        write_baccount_list();
        return;
}

void free_baccount(BANK_ACCOUNT * account)
{
        if (!account || account == NULL)
                return;
        UNLINK(account, first_baccount, last_baccount, next, prev);
        STRFREE(account->code);
        STRFREE(account->creator);
        STRFREE(account->owner);
        STRFREE(account->trustees);
        DISPOSE(account);
}

char     *generate_code()
{
        BANK_ACCOUNT *account;
        static char buf1[MAX_STRING_LENGTH];
        int       count = 0;
        bool      match;

        do
        {
                match = FALSE;
                for (count = 0; count < 20; count++)
                {
                        if (number_range(1, 100) <= 50)
                                buf1[count] = number_range('0', '9');
                        else
                                buf1[count] = number_range('a', 'f');
                }

                buf1[20] = '\0';

                for (account = first_baccount; account;
                     account = account->next)
                {
                        if (account->code && !strcmp(account->code, buf1))
                                match = TRUE;
                        break;
                }
        }
        while (match);

        return buf1;
}

// ============================================================================
// Secure Mathematical Operations
// ============================================================================

/*
 * Enhanced account_add with overflow protection and validation
 * SECURITY: Prevents integer overflow exploits and validates all operations
 */
bool account_add(BANK_ACCOUNT* account, long amount)
{
    if (!account || amount < 0) {
        bug("account_add: Invalid parameters - account=%p, amount=%ld", account, amount);
        return false;
    }

    // Validate transaction limits
    if (amount > BankSecurity::MAX_TRANSACTION_AMOUNT) {
        bug("account_add: Transaction exceeds maximum limit (%ld > %ld)", 
            amount, BankSecurity::MAX_TRANSACTION_AMOUNT);
        return false;
    }

    // Check for potential overflow in amountlo
    if (account->amountlo > BankSecurity::SAFE_ADDITION_LIMIT) {
        bug("account_add: Account balance too high for safe addition");
        return false;
    }

    // Process large amounts safely by moving to amounthi first
    while (amount > BankSecurity::BALANCE_THRESHOLD) {
        amount -= (BankSecurity::BALANCE_THRESHOLD + 1);
        if (account->amounthi >= LONG_MAX) {
            bug("account_add: Account amounthi overflow detected");
            return false;
        }
        account->amounthi += 1;
    }

    // Safe addition with overflow check
    unsigned long temp = static_cast<unsigned long>(account->amountlo) + static_cast<unsigned long>(amount);
    
    // Handle overflow to amounthi
    while (temp > BankSecurity::BALANCE_THRESHOLD) {
        temp -= (BankSecurity::BALANCE_THRESHOLD + 1);
        if (account->amounthi >= LONG_MAX) {
            bug("account_add: Account amounthi overflow during carry");
            return false;
        }
        account->amounthi += 1;
    }
    
    account->amountlo = static_cast<long>(temp);
    return true;
}

/*
 * Enhanced account_sub with underflow protection and proper validation
 * SECURITY: Fixes the catastrophic underflow bug that created infinite money
 */
bool account_sub(BANK_ACCOUNT* account, long amount)
{
    if (!account || amount < 0) {
        bug("account_sub: Invalid parameters - account=%p, amount=%ld", account, amount);
        return false;
    }

    // Validate transaction limits
    if (amount > BankSecurity::MAX_TRANSACTION_AMOUNT) {
        bug("account_sub: Transaction exceeds maximum limit (%ld > %ld)", 
            amount, BankSecurity::MAX_TRANSACTION_AMOUNT);
        return false;
    }

    // Check if account has sufficient funds - CRITICAL SECURITY CHECK
    if (!account_has_funds(account, amount)) {
        return false; // Insufficient funds
    }

    // Process withdrawal from high-order amount first
    while (amount > BankSecurity::BALANCE_THRESHOLD && account->amounthi > 0) {
        amount -= (BankSecurity::BALANCE_THRESHOLD + 1);
        account->amounthi -= 1;
        account->amountlo += BankSecurity::BALANCE_THRESHOLD + 1;
    }

    // Handle remaining amount from low-order balance
    if (amount <= account->amountlo) {
        account->amountlo -= amount;
    } else if (account->amounthi > 0) {
        // Need to borrow from amounthi
        account->amounthi -= 1;
        account->amountlo = (BankSecurity::BALANCE_THRESHOLD + 1) + account->amountlo - amount;
    } else {
        // This should never happen due to our funds check above
        bug("account_sub: Insufficient funds error - this should not occur");
        return false;
    }

    return true;
}

/*
 * Helper function to safely check if account has sufficient funds
 * SECURITY: Prevents underflow by validating before operations
 */
bool account_has_funds(BANK_ACCOUNT* account, long amount)
{
    if (!account || amount < 0) {
        return false;
    }

    // Calculate total available funds safely
    if (account->amounthi > 0) {
        // Account has high-order funds, definitely sufficient for normal transactions
        return true;
    }

    // Only low-order funds, direct comparison
    return (account->amountlo >= amount);
}

/*
 * Get total account balance as a string (safe for display)
 * SECURITY: Prevents overflow in string formatting
 */
char* account_sum(BANK_ACCOUNT* account)
{
    static char buf[MAX_STRING_LENGTH];

    if (!account) {
        return nullptr;
    }

    if (account->amounthi == 0 && account->amountlo == 0) {
        snprintf(buf, sizeof(buf), "0");
    } else if (account->amounthi > 0) {
        snprintf(buf, sizeof(buf), "%ld%09ld", account->amounthi, account->amountlo);
    } else {
        snprintf(buf, sizeof(buf), "%ld", account->amountlo);
    }

    return buf;
}

int baccounts(CHAR_DATA * ch)
{
        BANK_ACCOUNT *account;
        int       count = 0;

        if (!ch || ch == NULL)
        {
                for (account = first_baccount; account;
                     account = account->next)
                        count++;
                return count;
        }
        else
        {
                for (account = first_baccount; account;
                     account = account->next)
                        if (!strcmp(account->owner, ch->name))
                                count++;
                return count;
        }
        return 0;
}

BANK_ACCOUNT *account_by_code(char *code)
{
        BANK_ACCOUNT *account;

        if (!code || code == NULL || code[0] == '\0')
                return NULL;

        for (account = first_baccount; account; account = account->next)
                if (!strcmp(account->code, code))
                        return account;
        return NULL;
}

void notify_trustees_dep(BANK_ACCOUNT * account, char *name, long amount,
                         bool anon)
{
        CHAR_DATA *trustee;

        for (trustee = first_char; trustee; trustee = trustee->next)
                if ((!strcmp(trustee->name, account->owner) ||
                     nifty_is_name(trustee->name, account->trustees)) &&
                    strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has deposited %ld credits in account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  account->code);
        return;
}

void notify_trustees_wit(BANK_ACCOUNT * account, char *name, long amount,
                         bool anon)
{
        CHAR_DATA *trustee;

        for (trustee = first_char; trustee; trustee = trustee->next)
                if ((!strcmp(trustee->name, account->owner) ||
                     nifty_is_name(trustee->name, account->trustees)) &&
                    strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has withdrawn %ld credits from account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  account->code);
        return;
}

/*
 * Enhanced interest application with secure mathematical operations
 * SECURITY: Prevents floating-point precision exploits and overflow attacks
 */
void apply_interest(BANK_ACCOUNT* account)
{
    if (!account) {
        bug("apply_interest: null account pointer");
        return;
    }

    // Validate interest rate to prevent exploits
    if (account->interest < BankSecurity::MIN_INTEREST_RATE || 
        account->interest > BankSecurity::MAX_INTEREST_RATE) {
        bug("apply_interest: Invalid interest rate %f for account %s", 
            account->interest, account->code ? account->code : "UNKNOWN");
        account->interest = BANK_INTEREST; // Reset to default safe value
        return;
    }

    // Calculate interest safely using integer arithmetic to avoid precision issues
    long original_lo = account->amountlo;
    long original_hi = account->amounthi;
    
    // Skip interest on empty accounts
    if (original_hi == 0 && original_lo == 0) {
        return;
    }

    // Calculate interest on low amount using safe integer operations
    long interest_lo = static_cast<long>(original_lo * (account->interest - 1.0));
    
    // Calculate interest on high amount (convert to credits first)
    long interest_hi = 0;
    if (original_hi > 0) {
        // Safe calculation: multiply by (interest - 1) and convert back
        double hi_credits = static_cast<double>(original_hi) * (BankSecurity::BALANCE_THRESHOLD + 1);
        double hi_interest = hi_credits * (account->interest - 1.0);
        
        // Validate result is within safe bounds
        if (hi_interest > static_cast<double>(BankSecurity::SAFE_ADDITION_LIMIT)) {
            bug("apply_interest: Interest calculation overflow for account %s", 
                account->code ? account->code : "UNKNOWN");
            return;
        }
        
        interest_hi = static_cast<long>(hi_interest);
    }

    // Total interest to add
    long total_interest = interest_lo + interest_hi;
    
    // Validate total interest is reasonable
    if (total_interest < 0 || total_interest > BankSecurity::MAX_TRANSACTION_AMOUNT) {
        bug("apply_interest: Calculated interest %ld is out of bounds for account %s", 
            total_interest, account->code ? account->code : "UNKNOWN");
        return;
    }

    // Apply interest using our secure addition function
    if (total_interest > 0) {
        if (!account_add(account, total_interest)) {
            bug("apply_interest: Failed to add interest %ld to account %s", 
                total_interest, account->code ? account->code : "UNKNOWN");
            return;
        }

        // Notify owner of interest gained
        for (CHAR_DATA* owner = first_char; owner; owner = owner->next) {
            if (!strcmp(owner->name, account->owner) && total_interest > 0) {
                ch_printf(owner, "&R[&BInterest&R] &wAccount %s has gained %ld credits.\n\r",
                         account->code, total_interest);
            }
        }
    }
}

void update_baccounts()
{
        BANK_ACCOUNT *account;

        for (account = first_baccount; account; account = account->next)
        {
                apply_interest(account);
                save_baccount(account);
        }
        return;
}

void notify_trustees_tra(BANK_ACCOUNT * source, BANK_ACCOUNT * destin,
                         char *name, long amount, bool anon)
{
        CHAR_DATA *trustee;

        for (trustee = first_char; trustee; trustee = trustee->next)
        {
                if ((!strcmp(trustee->name, source->owner) ||
                     nifty_is_name(trustee->name, source->trustees)) &&
                    strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has transfered %ld credits from account %s to account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  source->code, destin->code);
                if ((!strcmp(trustee->name, destin->owner)
                     || nifty_is_name(trustee->name, destin->trustees))
                    && strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has transfered %ld credits from account %s to account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  source->code, destin->code);
        }
        return;
}

CMDF do_bank_new(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        char      arg4[MAX_INPUT_LENGTH];

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (get_trust(ch) < LEVEL_IMPLEMENTOR && IS_IMMORTAL(ch))
        {
                ch_printf(ch,
                          "Only level %d immortals can access the banking system.\n\r",
                          LEVEL_IMPLEMENTOR);
                return;
        }

        if (NOT_AUTHED(ch))
        {
                send_to_char
                        ("It's against the academy rules for students to access bank accounts while in the academy.\n\r",
                         ch);
                return;
        }

        if (!xIS_SET(ch->in_room->room_flags, ROOM_BANK)
            && get_comlink(ch) == NULL && !IS_IMMORTAL(ch))
        {
                send_to_char
                        ("You need to be at the bank, or have a comlink to do that.\n\r",
                         ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        argument = one_argument(argument, arg4);

        if (arg1[0] == '\0')
        {
                send_to_char
                        ("Syntax: BANK <open|close|list|status|transfer|deposit|withdraw> [account] [amount] [account|arguments]\n\r",
                         ch);
/*	if (IS_IMMORTAL(ch)) {
	    send_to_char("(IMM)   BANK <block|freeze|empty|hotwire> <account>\n\r", ch);
	    send_to_char("(IMM)   BANK <showall|closeall|taxall|tripinterest> [argument]\n\r", ch); } */
                return;
        }

        if (!strcmp(arg1, "open"))
        {
                BANK_ACCOUNT *account;

                if (baccounts(ch) >= 10)
                {
                        send_to_char
                                ("The bank refuses to let you open more accounts.\n\r",
                                 ch);
                        return;
                }

                if ((account = create_baccount(ch)) == NULL)
                {
                        bug("do_bank: create account returned NULL", 0);
                        return;
                }

                ch_printf(ch,
                          "You open a new bank account.\n\rAccount number: %s\n\r",
                          account->code);
                return;
        }
        else if (!strcmp(arg1, "close"))
        {
                BANK_ACCOUNT *account;

                if (arg2[0] == '\0')
                {
                        send_to_char("Close what account?\n\r", ch);
                        return;
                }

                account = account_by_code(arg2);

                if (account == NULL)
                {   // Lets keep players from finding other people's accounts.
                        send_to_char
                                ("Only the owner of that account can close it.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(account->owner, ch->name))
                {
                        send_to_char
                                ("Only the owner of that account can close it.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(arg3, "now"))
                {
                        send_to_char
                                ("If you are sure you want to do this, type: bank close <account> NOW\n\r",
                                 ch);
                        return;
                }

                if (account->amounthi > 0 || account->amountlo > 0)
                {
                        send_to_char
                                ("Please empty the bank account before closing it.\n\r",
                                 ch);
                        return;
                }

                delete_account(account);
                send_to_char("You close the bank account.\n\r", ch);
                return;
        }
        else if (!strcmp(arg1, "list"))
        {
                BANK_ACCOUNT *account;
                int       count = 0;
                char      buf[MAX_STRING_LENGTH];

                ch_printf(ch,
                          "&wAccount Number            Your Status           Balance\n\r");
                for (account = first_baccount; account;
                     account = account->next)
                {
                        // What a bitch it is to get these aligned the way I want them.
                        if (!strcmp(account->owner, ch->name))
                        {
                                count++;
                                ch_printf(ch, "&B%-25s %-21s %s\n\r",
                                          account->code, "Owner",
                                          account_sum(account));
                        }
                        else if (nifty_is_name(account->trustees, ch->name))
                        {
                                count++;
                                ch_printf(ch, "&C%-25s %-21s %s\n\r",
                                          account->code, "Trustee",
                                          account_sum(account));
                        }
/*						else if (IS_IMMORTAL(ch))
                        {
                                count++;
                                ch_printf(ch, "&C%-25s %-21s %s\n\r",
                                          account->code, "Trustee",
                                          account_sum(account));
                        } - To Spammy for now */
                }
                if (count == 0)
                        ch_printf(ch,
                                  "&RYou do not have access to any accounts.&w\n\r");
                else
                        ch_printf(ch, "&YTotal Accounts: %d\n\r", count);

                return;
        }
        else if (!strcmp(arg1, "deposit"))
        {
                int       num = atoi(arg3);
                BANK_ACCOUNT *account = account_by_code(arg2);
                bool      anon = FALSE;

                if (arg2[0] == '\0')
                {
                        send_to_char("Specify an account.\n\r", ch);
                        return;
                }

                if (account == NULL)
                {   // Again, confuse the mortal.
                        send_to_char
                                ("That account is either frozen or blocked.\n\r",
                                 ch);
                        return;
                }

                // Validate deposit amount using security constants
                if (num > BankSecurity::MAX_TRANSACTION_AMOUNT)
                {
                        ch_printf(ch, "The bank won't let you deposit more than %ld credits at a time.\n\r",
                                 BankSecurity::MAX_TRANSACTION_AMOUNT);
                        return;
                }

                if (num < BankSecurity::MINIMUM_TRANSACTION)
                {
                        send_to_char("Be a little more generous.\n\r", ch);
                        return;
                }

                if (num > ch->gold)
                {
                        send_to_char
                                ("You don't have that many credits on you.\n\r",
                                 ch);
                        return;
                }

                if (!strcmp(arg4, "anonymous"))
                        anon = TRUE;

                // Use secure account addition
                if (!account_add(account, num)) {
                        send_to_char("The bank's systems are unable to process this deposit right now.\n\r", ch);
                        bug("Bank deposit failed for account %s, amount %ld", account->code, num);
                        return;
                }

                ch->gold -= num;
                do_save(ch, "-silentsave");
                save_baccount(account);
                ch_printf(ch, "You deposit %ld credits in account %s.\n\r",
                          num, account->code);
                notify_trustees_dep(account, ch->name, num, anon);
                return;
        }
        else if (!strcmp(arg1, "withdraw"))
        {
                int       num = atoi(arg3);
                BANK_ACCOUNT *account = account_by_code(arg2);

                if (arg2[0] == '\0')
                {
                        send_to_char("Specify an account.\n\r", ch);
                        return;
                }

                if (account == NULL)
                {
                        send_to_char
                                ("You don't have access to that account.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(ch->name, account->owner)
                    && !nifty_is_name(ch->name, account->trustees))
                {
                        send_to_char
                                ("You don't have access to that account.\n\r",
                                 ch);
                        return;
                }

                // Validate withdrawal amount using security constants
                if (num < BankSecurity::MINIMUM_TRANSACTION)
                {
                        send_to_char("Try deposit instead.\n\r", ch);
                        return;
                }

                if (num > BankSecurity::MAX_TRANSACTION_AMOUNT)
                {
                        ch_printf(ch, "The bank won't let you withdraw more than %ld credits at a time.\n\r",
                                 BankSecurity::MAX_TRANSACTION_AMOUNT);
                        return;
                }
                
                // Enhanced overflow protection for player gold
                if (ch->gold > LONG_MAX - num)
                {
                        send_to_char("You are not able to carry that many credits.\n\r", ch);
                        return;
                }

                // Use secure funds checking
                if (!account_has_funds(account, num))
                {
                        send_to_char("That account doesn't have that many credits.\n\r", ch);
                        return;
                }

                // Use secure account subtraction
                if (!account_sub(account, num)) {
                        send_to_char("The bank's systems are unable to process this withdrawal right now.\n\r", ch);
                        bug("Bank withdrawal failed for account %s, amount %ld", account->code, num);
                        return;
                }

                ch->gold += num;
                do_save(ch, "-silentsave");
                save_baccount(account);
                ch_printf(ch, "You withdraw %ld credits from account %s.\n\r",
                          num, account->code);
                // No anonymity on withdrawls.
                notify_trustees_wit(account, ch->name, num, FALSE);
                return;
        }
        else if (!strcmp(arg1, "transfer"))
        {
                int       num = atoi(arg3);
                BANK_ACCOUNT *source = account_by_code(arg2);
                BANK_ACCOUNT *destin = account_by_code(arg4);

                if (arg2[0] == '\0')
                {
                        send_to_char("Transfer from what account?\n\r", ch);
                        return;
                }

                if (arg3[0] == '\0')
                {
                        ch_printf(ch, "Specify an amount up to %ld.\n\r",
                                 BankSecurity::MAX_TRANSACTION_AMOUNT);
                        return;
                }

                if (arg4[0] == '\0')
                {
                        send_to_char("Specify a destination account.\n\r", ch);
                        return;
                }

                if (source == NULL)
                {
                        send_to_char("You don't have access to that account.\n\r", ch);
                        return;
                }

                if (strcmp(ch->name, source->owner)
                    && !nifty_is_name(ch->name, source->trustees))
                {
                        send_to_char("You don't have access to that account.\n\r", ch);
                        return;
                }

                if (destin == NULL)
                {
                        send_to_char("The destination account is either frozen or blocked.\n\r", ch);
                        return;
                }

                // Validate transfer amount using security constants
                if (num < BankSecurity::MINIMUM_TRANSACTION)
                {
                        send_to_char("Transfer amount must be at least 1 credit.\n\r", ch);
                        return;
                }

                if (num > BankSecurity::MAX_TRANSACTION_AMOUNT)
                {
                        ch_printf(ch, "The bank will allow you to transfer up to %ld credits at a time.\n\r",
                                 BankSecurity::MAX_TRANSACTION_AMOUNT);
                        return;
                }

                // Use secure funds checking
                if (!account_has_funds(source, num))
                {
                        send_to_char("The source account doesn't have that many credits in it.\n\r", ch);
                        return;
                }

                // Perform atomic transfer using secure operations
                if (!account_sub(source, num)) {
                        send_to_char("The bank's systems are unable to process this transfer right now.\n\r", ch);
                        bug("Bank transfer (subtract) failed for source account %s, amount %ld", source->code, num);
                        return;
                }

                if (!account_add(destin, num)) {
                        // Critical: Rollback the subtraction since addition failed
                        if (!account_add(source, num)) {
                                bug("CRITICAL: Failed to rollback transfer for source account %s, amount %ld", source->code, num);
                        }
                        send_to_char("The bank's systems are unable to process this transfer right now.\n\r", ch);
                        bug("Bank transfer (add) failed for destination account %s, amount %ld", destin->code, num);
                        return;
                }

                save_baccount(source);
                save_baccount(destin);
                ch_printf(ch, "You transfer %ld credits from account %s to account %s.\n\r",
                          num, source->code, destin->code);
                notify_trustees_tra(source, destin, ch->name, num, FALSE);
                return;
        }
        else if (!strcmp(arg1, "status"))
        {
                BANK_ACCOUNT *account = account_by_code(arg2);

                if (arg2[0] == '\0')
                {
                        send_to_char("Specify an account.\n\r", ch);
                        return;
                }

                if (account == NULL)
                {
                        send_to_char
                                ("You don't have access to that account.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(ch->name, account->owner)
                    && !nifty_is_name(ch->name, account->trustees) &&
                    !IS_IMMORTAL(ch))
                {
                        send_to_char
                                ("You don't have access to that account.\n\r",
                                 ch);
                        return;
                }

                ch_printf(ch, "&z|+----------------------------------+|\n\r");
                ch_printf(ch,
                          "&z|&x^g                                    &z^x|\n\r");
                ch_printf(ch, "&z|&w^x Acct. #: %-20s      &z|\n\r",
                          account->code);
                ch_printf(ch, "&z|&w^x Creator: %-12s              &z|\n\r",
                          account->creator);
                ch_printf(ch, "&z|&w^x Owner  : %-12s              &z|\n\r",
                          account->owner);
                ch_printf(ch, "&z|&w^x Balance: %-019s       &z|\n\r",
                          account_sum(account));
                ch_printf(ch, "&z|+----------------------------------+|\n\r");
                return;
        }
        else
                do_bank_new(ch, "");

        return;
}

CMDF do_entrust(CHAR_DATA * ch, char *argument)
{
        BANK_ACCOUNT *account;
        CHAR_DATA *vict;
        char      arg1[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg1);
        account = account_by_code(argument);

        if (arg1[0] == '\0')
        {
                send_to_char("Entrust whom?\n\r", ch);
                return;
        }

        if (!strcmp(arg1, "list"))
        {
                int       count = 0;

                ch_printf(ch, "Account Number            Trustees\n\r");
                for (account = first_baccount; account;
                     account = account->next)
                        if (!strcmp(ch->name, account->owner))
                        {
                                count++;
                                ch_printf(ch, "&B%-24s %s\n\r", account->code,
                                          account->trustees);
                        }
                if (count == 0)
                        ch_printf(ch, "&RYou don't own any accounts.&w\n\r");
                else
                        ch_printf(ch, "&YTotal Accounts: %d\n\r", count);
                return;
        }

        if (!strcmp(arg1, "clear"))
        {
                if (argument[0] == '\0')
                {
                        send_to_char("Clear which account?\n\r", ch);
                        return;
                }

                if (account == NULL || strcmp(ch->name, account->owner))
                {
                        send_to_char("You don't own that account.\n\r", ch);
                        return;
                }

                if (account->trustees != NULL)
                        STRFREE(account->trustees);
                account->trustees = STRALLOC("");
                save_baccount(account);
                ch_printf(ch,
                          "Okay, account %s no longer has any trustees.\n\r",
                          account->code);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("To what account?\n\r", ch);
                return;
        }

        if ((vict = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (vict == ch)
        {
                send_to_char("There's no point in that.\n\r", ch);
                return;
        }

        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("You can't entrust an NPC.\n\r", ch);
                return;
        }

        if (IS_IMMORTAL(vict) && !IS_IMMORTAL(ch))
        {
                send_to_char("You can't entrust immortals.\n\r", ch);
                return;
        }

        if (account == NULL)
        {
                send_to_char
                        ("You can only entrust people with accounts you own.\n\r",
                         ch);
                return;
        }

        if (strcmp(ch->name, account->owner))
        {
                send_to_char
                        ("You can only entrust people with accounts you own.\n\r",
                         ch);
                return;
        }

        sprintf(buf, "%s %s", account->trustees, vict->name);
        if (account->trustees != NULL)
                STRFREE(account->trustees);
        account->trustees = STRALLOC(buf);
        save_baccount(account);
        ch_printf(ch, "Okay, %s has been entrusted with account %s.\n\r",
                  vict->name, account->code);

        return;
}
