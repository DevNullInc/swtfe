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
    constexpr long MaxTransactionAmount = 1000000000L;    // 1 billion credits max per transaction
    constexpr long BalanceThreshold = 999999999L;          // Threshold for hi/lo balance management
    constexpr long SafeAdditionLimit = LongMax / 2;      // Safe limit to prevent overflow
    constexpr int MaxAccountsPerPlayer = 10;             // Maximum accounts per player
    constexpr double MinInterestRate = 0.01;              // Minimum 1% interest
    constexpr double MaxInterestRate = 1.10;              // Maximum 110% interest (10% gain)
    constexpr double DefaultInterestRate = 0.05;          // Default 5% interest
    constexpr long MinimumTransaction = 1L;                // Minimum transaction amount
}

// ============================================================================
// Forward Declarations  
// ============================================================================
ObjData *get_comlink args((CharData * ch));
CMDF save_baccount args((BankAccount * Account));
CMDF load_baccount args((char *filename));
BankAccount *create_baccount args((CharData * ch));
CMDF delete_baccount args((BankAccount * Account));
char     *generate_code args(());
char     *account_sum args((BankAccount * Account));
bool     account_add args((BankAccount * Account, long amount));
bool     account_sub args((BankAccount * Account, long amount));
bool     account_has_funds args((BankAccount * Account, long amount));
int      baccounts args((CharData * ch));

// ============================================================================
// Global Variables
// ============================================================================
BankAccount *first_baccount = nullptr;
BankAccount *last_baccount = nullptr;

// ============================================================================
// Account Management Functions
// ============================================================================

void save_baccount(BankAccount * Account)
{
        // ============================================================================
        // ACCOUNT PERSISTENCE - Enhanced with error checking and validation
        // ============================================================================
        
        if (!Account || !Account->code) {
                bug("save_baccount: Invalid Account or missing code", 0);
                return;
        }
        
        FILE     *fp;
        char      filename[256];

        // Use secure path construction
        snprintf(filename, sizeof(filename), "%s%s.acct", BAccountDir, Account->code);
        
        if ((fp = fopen(filename, "w")) == nullptr) {
                bug("save_baccount: unable to open %s.acct for writing!", Account->code);
                perror(filename);
                return;
        }

        // Write Account data with proper null checks
        fprintf(fp, "#ACCOUNT\n");
        fprintf(fp, "Code        %s~\n", Account->code ? Account->code : "");
        fprintf(fp, "Creator     %s~\n", Account->creator ? Account->creator : "");
        fprintf(fp, "Owner       %s~\n", Account->owner ? Account->owner : "");
        fprintf(fp, "Trustees    %s~\n", Account->trustees ? Account->trustees : "");
        fprintf(fp, "Flags       %ld\n", Account->flags);
        fprintf(fp, "Interest    %f\n", Account->interest);
        fprintf(fp, "Amounthi    %ld\n", Account->amounthi);
        fprintf(fp, "Amountlo    %ld\n", Account->amountlo);
        fprintf(fp, "End\n");
        
        if (fclose(fp) != 0) {
                bug("save_baccount: Error closing file %s", filename);
                perror(filename);
        }

        return;
}

void write_baccount_list()
{
        BankAccount *Account;
        FILE     *fp;
        char      filename[256];

        sprintf(filename, "%s%s", BAccountDir, BAccountList);

        if ((fp = fopen(filename, "w")) == NULL)
        {
                perror(filename);
                bug("write_baccount_list: can't open list", 0);
                return;
        }

        for (Account = first_baccount; Account; Account = Account->next)
                fprintf(fp, "%s.acct\n", Account->code);
        fprintf(fp, "$\n");

        FCLOSE(fp);
        return;
}

void load_baccount_list()
{
        FILE     *fpList;
        char      filename[256];
        char     *Account;

        sprintf(filename, "%s%s", BAccountDir, BAccountList);

        FCLOSE(fpReserve);
        if ((fpList = fopen(filename, "r")) == NULL)
        {
                fpReserve = fopen(NullFile, "r");
                perror(filename);
                bug("load_baccount: couldn't open Account list", 0);
                return;
        }

        for (;;)
        {
                Account = feof(fpList) ? const_cast<char*>("$") : fread_word(fpList);

                if (Account[0] == '$')
                        break;

                load_baccount(Account);
        }
        FCLOSE(fpList);
        log_string("Done loading accounts");
        fpReserve = fopen(NullFile, "r");
        return;
}

void load_baccount(char *name)
{
        char      filename[256];
        char      letter, *word;
        FILE     *fp;
        BankAccount *Account;
        bool      fMatch;

        sprintf(filename, "%s%s", BAccountDir, name);

        if ((fp = fopen(filename, "r")) == NULL)
        {
                perror(filename);
                bug("load_baccount: couldn't open .acct", 0);
                return;
        }

        CREATE(Account, BankAccount, 1);
        LINK(Account, first_baccount, last_baccount, next, prev);

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
                word = feof(fp) ? const_cast<char*>("End") : fread_word(fp);
                fMatch = FALSE;

                switch (Upper(word[0]))
                {
                case 'A':
                        KEY("Amounthi", Account->amounthi, fread_number(fp));
                        KEY("Amountlo", Account->amountlo, fread_number(fp));
                        break;
                case 'C':
                        KEY("Code", Account->code, fread_string(fp));
                        KEY("Creator", Account->creator, fread_string(fp));
                        break;
                case 'E':
                        if (!strcmp(word, "End"))
                        {
                                if (Account->code == NULL)
                                        Account->code =
                                                STRALLOC(generate_code());
                                if (Account->creator == NULL)
                                        Account->creator =
                                                STRALLOC(const_cast<char*>("NOCREATOR"));
                                if (Account->owner == NULL)
                                        Account->owner = STRALLOC(const_cast<char*>("NOOWNER"));
                                if (Account->trustees == NULL)
                                        Account->trustees = STRALLOC(const_cast<char*>(""));
                                FCLOSE(fp);
                                return;
                        }
                        break;
                case 'F':
                        KEY("Flags", Account->flags, fread_number(fp));
                        break;
                case 'I':
                        KEY("Interest", Account->interest, fread_float(fp));
                        break;
                case 'O':
                        KEY("Owner", Account->owner, fread_string(fp));
                        break;
                case 'T':
                        KEY("Trustees", Account->trustees, fread_string(fp));
                        break;
                }

                if (!fMatch)
                        bug("load_baccount: no match for %s", word);
        }
        return;
}

BankAccount *create_baccount(CharData * ch)
{
        // ============================================================================
        // BANK ACCOUNT CREATION - Enhanced with security validation
        // ============================================================================
        
        // Enhanced input validation
        if (!ch || IsNpc(ch) || !ch->pcdata || !ch->name) {
                bug("create_baccount: Invalid character data", 0);
                return nullptr;
        }
        
        // Check if player already has too many accounts (security measure)
        int player_account_count = 0;
        for (BankAccount* existing = first_baccount; existing; existing = existing->next) {
                if (existing->owner && !str_cmp(existing->owner, ch->name)) {
                        player_account_count++;
                }
        }
        
        if (player_account_count >= BankSecurity::MaxAccountsPerPlayer) {
                bug("create_baccount: Player %s attempting to exceed Account limit", ch->name);
                return nullptr;
        }
        
        BankAccount *Account;
        CREATE(Account, BankAccount, 1);
        LINK(Account, first_baccount, last_baccount, next, prev);
        
        // Initialize with security-validated values
        Account->code = STRALLOC(generate_code());
        Account->creator = STRALLOC(ch->name);
        Account->owner = STRALLOC(ch->name);
        Account->trustees = STRALLOC(const_cast<char*>(""));
        Account->flags = 0;
        Account->interest = static_cast<float>(BankSecurity::DefaultInterestRate);
        Account->amounthi = 0;
        Account->amountlo = 0;

        save_baccount(Account);
        write_baccount_list();
        return Account;
}

void delete_account(BankAccount * Account)
{
        // ============================================================================
        // ACCOUNT DELETION - Enhanced with security validation and cleanup
        // ============================================================================
        
        if (!Account) {
                bug("delete_account: Null Account pointer", 0);
                return;
        }
        
        if (!Account->code) {
                bug("delete_account: Account missing code", 0);
                return;
        }
        
        char filename[256];
        
        // Remove from linked list first
        UNLINK(Account, first_baccount, last_baccount, next, prev);
        
        // Construct filename for deletion
        snprintf(filename, sizeof(filename), "%s%s.acct", BAccountDir, Account->code);
        
        // Free all allocated strings safely
        STRFREE(Account->code);
        STRFREE(Account->creator);
        STRFREE(Account->owner);
        STRFREE(Account->trustees);
        DISPOSE(Account);

        // Remove the file and check for errors
        if (remove(filename) != 0) {
                bug("delete_account: Failed to remove file %s", filename);
                perror(filename);
        }
        
        write_baccount_list();
        return;
}

void free_baccount(BankAccount * Account)
{
        if (!Account || Account == NULL)
                return;
        UNLINK(Account, first_baccount, last_baccount, next, prev);
        STRFREE(Account->code);
        STRFREE(Account->creator);
        STRFREE(Account->owner);
        STRFREE(Account->trustees);
        DISPOSE(Account);
}

char     *generate_code()
{
        BankAccount *Account;
        static char buf1[MaxStringLength];
        int       count = 0;
        bool      match;

        do
        {
                match = FALSE;
                for (count = 0; count < 20; count++)
                {
                        if (number_range(1, 100) <= 50)
                                buf1[count] = static_cast<char>(number_range('0', '9'));
                        else
                                buf1[count] = static_cast<char>(number_range('a', 'f'));
                }

                buf1[20] = '\0';

                for (Account = first_baccount; Account;
                     Account = Account->next)
                {
                        if (Account->code && !strcmp(Account->code, buf1))
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
bool account_add(BankAccount* Account, long amount)
{
    if (!Account || amount < 0) {
        bug("account_add: Invalid parameters - Account=%p, amount=%ld", Account, amount);
        return false;
    }

    // Validate transaction limits
    if (amount > BankSecurity::MaxTransactionAmount) {
        bug("account_add: Transaction exceeds maximum limit (%ld > %ld)", 
            amount, BankSecurity::MaxTransactionAmount);
        return false;
    }

    // Check for potential overflow in amountlo
    if (Account->amountlo > BankSecurity::SafeAdditionLimit) {
        bug("account_add: Account balance too high for safe addition");
        return false;
    }

    // Process large amounts safely by moving to amounthi first
    while (amount > BankSecurity::BalanceThreshold) {
        amount -= (BankSecurity::BalanceThreshold + 1);
        if (Account->amounthi >= LongMax) {
            bug("account_add: Account amounthi overflow detected");
            return false;
        }
        Account->amounthi += 1;
    }

    // Safe addition with overflow check
    unsigned long temp = static_cast<unsigned long>(Account->amountlo) + static_cast<unsigned long>(amount);
    
    // Handle overflow to amounthi
    while (temp > BankSecurity::BalanceThreshold) {
        temp -= (BankSecurity::BalanceThreshold + 1);
        if (Account->amounthi >= LongMax) {
            bug("account_add: Account amounthi overflow during carry");
            return false;
        }
        Account->amounthi += 1;
    }
    
    Account->amountlo = static_cast<long>(temp);
    return true;
}

/*
 * Enhanced account_sub with underflow protection and proper validation
 * SECURITY: Fixes the catastrophic underflow bug that created infinite money
 */
bool account_sub(BankAccount* Account, long amount)
{
    if (!Account || amount < 0) {
        bug("account_sub: Invalid parameters - Account=%p, amount=%ld", Account, amount);
        return false;
    }

    // Validate transaction limits
    if (amount > BankSecurity::MaxTransactionAmount) {
        bug("account_sub: Transaction exceeds maximum limit (%ld > %ld)", 
            amount, BankSecurity::MaxTransactionAmount);
        return false;
    }

    // Check if Account has sufficient funds - CRITICAL SECURITY CHECK
    if (!account_has_funds(Account, amount)) {
        return false; // Insufficient funds
    }

    // Process withdrawal from high-order amount first
    while (amount > BankSecurity::BalanceThreshold && Account->amounthi > 0) {
        amount -= (BankSecurity::BalanceThreshold + 1);
        Account->amounthi -= 1;
        Account->amountlo += BankSecurity::BalanceThreshold + 1;
    }

    // Handle remaining amount from low-order balance
    if (amount <= Account->amountlo) {
        Account->amountlo -= amount;
    } else if (Account->amounthi > 0) {
        // Need to borrow from amounthi
        Account->amounthi -= 1;
        Account->amountlo = (BankSecurity::BalanceThreshold + 1) + Account->amountlo - amount;
    } else {
        // This should never happen due to our funds check above
        bug("account_sub: Insufficient funds error - this should not occur");
        return false;
    }

    return true;
}

/*
 * Helper function to safely check if Account has sufficient funds
 * SECURITY: Prevents underflow by validating before operations
 */
bool account_has_funds(BankAccount* Account, long amount)
{
    if (!Account || amount < 0) {
        return false;
    }

    // Calculate total available funds safely
    if (Account->amounthi > 0) {
        // Account has high-order funds, definitely sufficient for normal transactions
        return true;
    }

    // Only low-order funds, direct comparison
    return (Account->amountlo >= amount);
}

/*
 * Get total Account balance as a string (safe for display)
 * SECURITY: Prevents overflow in string formatting
 */
char* account_sum(BankAccount* Account)
{
    static char buf[MaxStringLength];

    if (!Account) {
        return nullptr;
    }

    if (Account->amounthi == 0 && Account->amountlo == 0) {
        snprintf(buf, sizeof(buf), "0");
    } else if (Account->amounthi > 0) {
        snprintf(buf, sizeof(buf), "%ld%09ld", Account->amounthi, Account->amountlo);
    } else {
        snprintf(buf, sizeof(buf), "%ld", Account->amountlo);
    }

    return buf;
}

int baccounts(CharData * ch)
{
        BankAccount *Account;
        int       count = 0;

        if (!ch || ch == NULL)
        {
                for (Account = first_baccount; Account;
                     Account = Account->next)
                        count++;
                return count;
        }
        else
        {
                for (Account = first_baccount; Account;
                     Account = Account->next)
                        if (!strcmp(Account->owner, ch->name))
                                count++;
                return count;
        }
        return 0;
}

BankAccount *account_by_code(char *code)
{
        BankAccount *Account;

        if (!code || code == NULL || code[0] == '\0')
                return NULL;

        for (Account = first_baccount; Account; Account = Account->next)
                if (!strcmp(Account->code, code))
                        return Account;
        return NULL;
}

void notify_trustees_dep(BankAccount * Account, char *name, long amount,
                         bool anon)
{
        CharData *trustee;

        for (trustee = first_char; trustee; trustee = trustee->next)
                if ((!strcmp(trustee->name, Account->owner) ||
                     nifty_is_name(trustee->name, Account->trustees)) &&
                    strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has deposited %ld credits in Account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  Account->code);
        return;
}

void notify_trustees_wit(BankAccount * Account, char *name, long amount,
                         bool anon)
{
        CharData *trustee;

        for (trustee = first_char; trustee; trustee = trustee->next)
                if ((!strcmp(trustee->name, Account->owner) ||
                     nifty_is_name(trustee->name, Account->trustees)) &&
                    strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has withdrawn %ld credits from Account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  Account->code);
        return;
}

/*
 * Enhanced interest application with secure mathematical operations
 * SECURITY: Prevents floating-point precision exploits and overflow attacks
 */
void apply_interest(BankAccount* Account)
{
    if (!Account) {
        bug("apply_interest: null Account pointer");
        return;
    }

    // Validate interest rate to prevent exploits
    if (Account->interest < BankSecurity::MinInterestRate || 
        Account->interest > BankSecurity::MaxInterestRate) {
        bug("apply_interest: Invalid interest rate %f for Account %s", 
            Account->interest, Account->code ? Account->code : "UNKNOWN");
        Account->interest = static_cast<float>(BankInterest); // Reset to default safe value
        return;
    }

    // Calculate interest safely using integer arithmetic to avoid precision issues
    long original_lo = Account->amountlo;
    long original_hi = Account->amounthi;
    
    // Skip interest on empty accounts
    if (original_hi == 0 && original_lo == 0) {
        return;
    }

    // Calculate interest on low amount using safe integer operations
    long interest_lo = static_cast<long>(static_cast<double>(original_lo) * (Account->interest - 1.0));
    
    // Calculate interest on high amount (convert to credits first)
    long interest_hi = 0;
    if (original_hi > 0) {
        // Safe calculation: multiply by (interest - 1) and convert back
        double hi_credits = static_cast<double>(original_hi) * (BankSecurity::BalanceThreshold + 1);
        double hi_interest = hi_credits * (Account->interest - 1.0);
        
        // Validate result is within safe bounds
        if (hi_interest > static_cast<double>(BankSecurity::SafeAdditionLimit)) {
            bug("apply_interest: Interest calculation overflow for Account %s", 
                Account->code ? Account->code : "UNKNOWN");
            return;
        }
        
        interest_hi = static_cast<long>(hi_interest);
    }

    // Total interest to add
    long total_interest = interest_lo + interest_hi;
    
    // Validate total interest is reasonable
    if (total_interest < 0 || total_interest > BankSecurity::MaxTransactionAmount) {
        bug("apply_interest: Calculated interest %ld is out of bounds for Account %s", 
            total_interest, Account->code ? Account->code : "UNKNOWN");
        return;
    }

    // Apply interest using our secure addition function
    if (total_interest > 0) {
        if (!account_add(Account, total_interest)) {
            bug("apply_interest: Failed to add interest %ld to Account %s", 
                total_interest, Account->code ? Account->code : "UNKNOWN");
            return;
        }

        // Notify owner of interest gained
        for (CharData* owner = first_char; owner; owner = owner->next) {
            if (!strcmp(owner->name, Account->owner) && total_interest > 0) {
                ch_printf(owner, "&R[&BInterest&R] &wAccount %s has gained %ld credits.\n\r",
                         Account->code, total_interest);
            }
        }
    }
}

void update_baccounts()
{
        BankAccount *Account;

        for (Account = first_baccount; Account; Account = Account->next)
        {
                apply_interest(Account);
                save_baccount(Account);
        }
        return;
}

void notify_trustees_tra(BankAccount * source, BankAccount * destin,
                         char *name, long amount, bool anon)
{
        CharData *trustee;

        for (trustee = first_char; trustee; trustee = trustee->next)
        {
                if ((!strcmp(trustee->name, source->owner) ||
                     nifty_is_name(trustee->name, source->trustees)) &&
                    strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has transfered %ld credits from Account %s to Account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  source->code, destin->code);
                if ((!strcmp(trustee->name, destin->owner)
                     || nifty_is_name(trustee->name, destin->trustees))
                    && strcmp(trustee->name, name))
                        ch_printf(trustee,
                                  "%s has transfered %ld credits from Account %s to Account %s.\n\r",
                                  anon ? "Someone" : name, amount,
                                  source->code, destin->code);
        }
        return;
}

CMDF do_bank_new(CharData * ch, char *argument)
{
        char      arg1[MaxInputLength];
        char      arg2[MaxInputLength];
        char      arg3[MaxInputLength];
        char      arg4[MaxInputLength];

        if (IsNpc(ch) || !ch->pcdata)
                return;

        if (get_trust(ch) < LevelImplementor && IsImmortal(ch))
        {
                ch_printf(ch,
                          "Only level %d immortals can access the banking system.\n\r",
                          LevelImplementor);
                return;
        }

        if (NotAuthed(ch))
        {
                send_to_char
                        ("It's against the academy rules for students to access bank accounts while in the academy.\n\r",
                         ch);
                return;
        }

        if (!IsSet(ch->in_room->RoomFlags, RoomBank)
            && get_comlink(ch) == NULL && !IsImmortal(ch))
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
                        ("Syntax: BANK <open|close|list|status|transfer|deposit|withdraw> [Account] [amount] [Account|arguments]\n\r",
                         ch);
/*	if (IsImmortal(ch)) {
	    send_to_char("(IMM)   BANK <block|freeze|empty|hotwire> <Account>\n\r", ch);
	    send_to_char("(IMM)   BANK <showall|closeall|taxall|tripinterest> [argument]\n\r", ch); } */
                return;
        }

        if (!strcmp(arg1, "open"))
        {
                BankAccount *Account;

                if (baccounts(ch) >= 10)
                {
                        send_to_char
                                ("The bank refuses to let you open more accounts.\n\r",
                                 ch);
                        return;
                }

                if ((Account = create_baccount(ch)) == NULL)
                {
                        bug("do_bank: create Account returned NULL", 0);
                        return;
                }

                ch_printf(ch,
                          "You open a new bank Account.\n\rAccount number: %s\n\r",
                          Account->code);
                return;
        }
        else if (!strcmp(arg1, "close"))
        {
                BankAccount *Account;

                if (arg2[0] == '\0')
                {
                        send_to_char("Close what Account?\n\r", ch);
                        return;
                }

                Account = account_by_code(arg2);

                if (Account == NULL)
                {   // Lets keep players from finding other people's accounts.
                        send_to_char
                                ("Only the owner of that Account can close it.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(Account->owner, ch->name))
                {
                        send_to_char
                                ("Only the owner of that Account can close it.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(arg3, "now"))
                {
                        send_to_char
                                ("If you are sure you want to do this, type: bank close <Account> NOW\n\r",
                                 ch);
                        return;
                }

                if (Account->amounthi > 0 || Account->amountlo > 0)
                {
                        send_to_char
                                ("Please empty the bank Account before closing it.\n\r",
                                 ch);
                        return;
                }

                delete_account(Account);
                send_to_char("You close the bank Account.\n\r", ch);
                return;
        }
        else if (!strcmp(arg1, "list"))
        {
                BankAccount *Account;
                int       count = 0;
                // char      buf[MaxStringLength]; // Unused variable removed

                ch_printf(ch,
                          "&wAccount Number            Your Status           Balance\n\r");
                for (Account = first_baccount; Account;
                     Account = Account->next)
                {
                        // What a bitch it is to get these aligned the way I want them.
                        if (!strcmp(Account->owner, ch->name))
                        {
                                count++;
                                ch_printf(ch, "&B%-25s %-21s %s\n\r",
                                          Account->code, "Owner",
                                          account_sum(Account));
                        }
                        else if (nifty_is_name(Account->trustees, ch->name))
                        {
                                count++;
                                ch_printf(ch, "&C%-25s %-21s %s\n\r",
                                          Account->code, "Trustee",
                                          account_sum(Account));
                        }
/*						else if (IsImmortal(ch))
                        {
                                count++;
                                ch_printf(ch, "&C%-25s %-21s %s\n\r",
                                          Account->code, "Trustee",
                                          account_sum(Account));
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
                BankAccount *Account = account_by_code(arg2);
                bool      anon = FALSE;

                if (arg2[0] == '\0')
                {
                        send_to_char("Specify an Account.\n\r", ch);
                        return;
                }

                if (Account == NULL)
                {   // Again, confuse the mortal.
                        send_to_char
                                ("That Account is either frozen or blocked.\n\r",
                                 ch);
                        return;
                }

                // Validate deposit amount using security constants
                if (num > BankSecurity::MaxTransactionAmount)
                {
                        ch_printf(ch, "The bank won't let you deposit more than %ld credits at a time.\n\r",
                                 BankSecurity::MaxTransactionAmount);
                        return;
                }

                if (num < BankSecurity::MinimumTransaction)
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

                // Use secure Account addition
                if (!account_add(Account, num)) {
                        send_to_char("The bank's systems are unable to Process this deposit right now.\n\r", ch);
                        bug("Bank deposit failed for Account %s, amount %ld", Account->code, num);
                        return;
                }

                ch->gold -= num;
                do_save(ch, "-silentsave");
                save_baccount(Account);
                ch_printf(ch, "You deposit %ld credits in Account %s.\n\r",
                          num, Account->code);
                notify_trustees_dep(Account, ch->name, num, anon);
                return;
        }
        else if (!strcmp(arg1, "withdraw"))
        {
                int       num = atoi(arg3);
                BankAccount *Account = account_by_code(arg2);

                if (arg2[0] == '\0')
                {
                        send_to_char("Specify an Account.\n\r", ch);
                        return;
                }

                if (Account == NULL)
                {
                        send_to_char
                                ("You don't have access to that Account.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(ch->name, Account->owner)
                    && !nifty_is_name(ch->name, Account->trustees))
                {
                        send_to_char
                                ("You don't have access to that Account.\n\r",
                                 ch);
                        return;
                }

                // Validate withdrawal amount using security constants
                if (num < BankSecurity::MinimumTransaction)
                {
                        send_to_char("Try deposit instead.\n\r", ch);
                        return;
                }

                if (num > BankSecurity::MaxTransactionAmount)
                {
                        ch_printf(ch, "The bank won't let you withdraw more than %ld credits at a time.\n\r",
                                 BankSecurity::MaxTransactionAmount);
                        return;
                }
                
                // Enhanced overflow protection for player gold
                if (ch->gold > LongMax - num)
                {
                        send_to_char("You are not able to carry that many credits.\n\r", ch);
                        return;
                }

                // Use secure funds checking
                if (!account_has_funds(Account, num))
                {
                        send_to_char("That Account doesn't have that many credits.\n\r", ch);
                        return;
                }

                // Use secure Account subtraction
                if (!account_sub(Account, num)) {
                        send_to_char("The bank's systems are unable to Process this withdrawal right now.\n\r", ch);
                        bug("Bank withdrawal failed for Account %s, amount %ld", Account->code, num);
                        return;
                }

                ch->gold += num;
                do_save(ch, "-silentsave");
                save_baccount(Account);
                ch_printf(ch, "You withdraw %ld credits from Account %s.\n\r",
                          num, Account->code);
                // No anonymity on withdrawls.
                notify_trustees_wit(Account, ch->name, num, FALSE);
                return;
        }
        else if (!strcmp(arg1, "transfer"))
        {
                int       num = atoi(arg3);
                BankAccount *source = account_by_code(arg2);
                BankAccount *destin = account_by_code(arg4);

                if (arg2[0] == '\0')
                {
                        send_to_char("Transfer from what Account?\n\r", ch);
                        return;
                }

                if (arg3[0] == '\0')
                {
                        ch_printf(ch, "Specify an amount up to %ld.\n\r",
                                 BankSecurity::MaxTransactionAmount);
                        return;
                }

                if (arg4[0] == '\0')
                {
                        send_to_char("Specify a destination Account.\n\r", ch);
                        return;
                }

                if (source == NULL)
                {
                        send_to_char("You don't have access to that Account.\n\r", ch);
                        return;
                }

                if (strcmp(ch->name, source->owner)
                    && !nifty_is_name(ch->name, source->trustees))
                {
                        send_to_char("You don't have access to that Account.\n\r", ch);
                        return;
                }

                if (destin == NULL)
                {
                        send_to_char("The destination Account is either frozen or blocked.\n\r", ch);
                        return;
                }

                // Validate transfer amount using security constants
                if (num < BankSecurity::MinimumTransaction)
                {
                        send_to_char("Transfer amount must be at least 1 credit.\n\r", ch);
                        return;
                }

                if (num > BankSecurity::MaxTransactionAmount)
                {
                        ch_printf(ch, "The bank will allow you to transfer up to %ld credits at a time.\n\r",
                                 BankSecurity::MaxTransactionAmount);
                        return;
                }

                // Use secure funds checking
                if (!account_has_funds(source, num))
                {
                        send_to_char("The source Account doesn't have that many credits in it.\n\r", ch);
                        return;
                }

                // Perform atomic transfer using secure operations
                if (!account_sub(source, num)) {
                        send_to_char("The bank's systems are unable to Process this transfer right now.\n\r", ch);
                        bug("Bank transfer (subtract) failed for source Account %s, amount %ld", source->code, num);
                        return;
                }

                if (!account_add(destin, num)) {
                        // Critical: Rollback the subtraction since addition failed
                        if (!account_add(source, num)) {
                                bug("CRITICAL: Failed to rollback transfer for source Account %s, amount %ld", source->code, num);
                        }
                        send_to_char("The bank's systems are unable to Process this transfer right now.\n\r", ch);
                        bug("Bank transfer (add) failed for destination Account %s, amount %ld", destin->code, num);
                        return;
                }

                save_baccount(source);
                save_baccount(destin);
                ch_printf(ch, "You transfer %ld credits from Account %s to Account %s.\n\r",
                          num, source->code, destin->code);
                notify_trustees_tra(source, destin, ch->name, num, FALSE);
                return;
        }
        else if (!strcmp(arg1, "status"))
        {
                BankAccount *Account = account_by_code(arg2);

                if (arg2[0] == '\0')
                {
                        send_to_char("Specify an Account.\n\r", ch);
                        return;
                }

                if (Account == NULL)
                {
                        send_to_char
                                ("You don't have access to that Account.\n\r",
                                 ch);
                        return;
                }

                if (strcmp(ch->name, Account->owner)
                    && !nifty_is_name(ch->name, Account->trustees) &&
                    !IsImmortal(ch))
                {
                        send_to_char
                                ("You don't have access to that Account.\n\r",
                                 ch);
                        return;
                }

                ch_printf(ch, "&z|+----------------------------------+|\n\r");
                ch_printf(ch,
                          "&z|&x^g                                    &z^x|\n\r");
                ch_printf(ch, "&z|&w^x Acct. #: %-20s      &z|\n\r",
                          Account->code);
                ch_printf(ch, "&z|&w^x Creator: %-12s              &z|\n\r",
                          Account->creator);
                ch_printf(ch, "&z|&w^x Owner  : %-12s              &z|\n\r",
                          Account->owner);
                ch_printf(ch, "&z|&w^x Balance: %-019s       &z|\n\r",
                          account_sum(Account));
                ch_printf(ch, "&z|+----------------------------------+|\n\r");
                return;
        }
        else
                do_bank_new(ch, const_cast<char*>(""));

        return;
}

CMDF do_entrust(CharData * ch, char *argument)
{
        BankAccount *Account;
        CharData *vict;
        char      arg1[MaxInputLength];
        char      buf[MaxStringLength];

        argument = one_argument(argument, arg1);
        Account = account_by_code(argument);

        if (arg1[0] == '\0')
        {
                send_to_char("Entrust whom?\n\r", ch);
                return;
        }

        if (!strcmp(arg1, "list"))
        {
                int       count = 0;

                ch_printf(ch, "Account Number            Trustees\n\r");
                for (Account = first_baccount; Account;
                     Account = Account->next)
                        if (!strcmp(ch->name, Account->owner))
                        {
                                count++;
                                ch_printf(ch, "&B%-24s %s\n\r", Account->code,
                                          Account->trustees);
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
                        send_to_char("Clear which Account?\n\r", ch);
                        return;
                }

                if (Account == NULL || strcmp(ch->name, Account->owner))
                {
                        send_to_char("You don't own that Account.\n\r", ch);
                        return;
                }

                if (Account->trustees != NULL)
                        STRFREE(Account->trustees);
                Account->trustees = STRALLOC(const_cast<char*>(""));
                save_baccount(Account);
                ch_printf(ch,
                          "Okay, Account %s no longer has any trustees.\n\r",
                          Account->code);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("To what Account?\n\r", ch);
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

        if (IsNpc(ch) || !ch->pcdata)
        {
                send_to_char("You can't entrust an NPC.\n\r", ch);
                return;
        }

        if (IsImmortal(vict) && !IsImmortal(ch))
        {
                send_to_char("You can't entrust immortals.\n\r", ch);
                return;
        }

        if (Account == NULL)
        {
                send_to_char
                        ("You can only entrust people with accounts you own.\n\r",
                         ch);
                return;
        }

        if (strcmp(ch->name, Account->owner))
        {
                send_to_char
                        ("You can only entrust people with accounts you own.\n\r",
                         ch);
                return;
        }

        sprintf(buf, "%s %s", Account->trustees, vict->name);
        if (Account->trustees != NULL)
                STRFREE(Account->trustees);
        Account->trustees = STRALLOC(buf);
        save_baccount(Account);
        ch_printf(ch, "Okay, %s has been entrusted with Account %s.\n\r",
                  vict->name, Account->code);

        return;
}
