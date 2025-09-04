# Copilot Instructions for C23 MUD Modernization with Server Management

## Project Overview
- **Language**: C (C11/C23), GCC, single `src/Makefile`. Build only from `src/`.
- **Protocols**: Telnet + GMCP (Generic MUD Communication Protocol) + MSP/MXP
- **Runtime**: Long-running text MUD. Core systems: networking, command parsing, persistence, game logic.
- **Security**: Argon2 password hashing (NOT crypt), buffer overflow prevention via snprintf/vsnprintf
- **Codebase lineage**: DikuMUD -> Merc -> Smaug -> SWR
- **Development Environment**: `/workspaces/swtfe/` - use server management tools in `scripts/` directory

## Critical Rules - Never Violate These

### 1. Preserve Legacy Architecture
- **NEVER** delete functions that appear "unused" - they may be called by area files, scripts, or MOBprograms
- **NEVER** rename variables to modern conventions unless explicitly requested
- **MAINTAIN** all existing function signatures and calling conventions
- **PRESERVE** namespace conventions even if inconsistent (ch, pObj, pRoom, etc.)

### 2. Build and Testing Requirements
- Use safe string functions: `snprintf()`, `vsnprintf()` - NEVER `sprintf()` or `strcpy()`
- Continue Argon2 password hashing in `secure_password.c` - avoid legacy `crypt()`
- Build from `src/` only: `cd src && make clean && make`
- Zero compilation warnings tolerated - fix type mismatches and buffer risks first
- **After each change**: Use server management tools to test functionality

### 3. Server Management Integration
- **Base Directory**: `/workspaces/swtfe/`
- **Server Tools**: `/workspaces/swtfe/scripts/mudctl.sh` - primary server control
- **Test Changes**: Always restart server and verify functionality after code changes
- **Check Dependencies**: Use `/workspaces/swtfe/scripts/check_system.sh` for system validation

## Development Workflow

### 1. Code Modification Process
```bash
# 1. Make code changes in src/
cd /workspaces/swtfe/src
# Edit C files...

# 2. Clean build with zero warnings
make clean && make 2>&1 | tee build.log | grep -E "warning|error"

# 3. Stop current server
/workspaces/swtfe/scripts/mudctl.sh stop

# 4. Test server startup
/workspaces/swtfe/scripts/mudctl.sh start

# 5. Verify functionality
/workspaces/swtfe/scripts/mudctl.sh status
/workspaces/swtfe/scripts/mudctl.sh logs 20
```

### 2. Testing and Validation Commands
```bash
# Check server status and processes
/workspaces/swtfe/scripts/mudctl.sh status

# Monitor server logs for errors
/workspaces/swtfe/scripts/mudctl.sh tail

# View recent log entries (look for crashes, errors)
/workspaces/swtfe/scripts/mudctl.sh logs 50

# System dependency validation
/workspaces/swtfe/scripts/check_system.sh

# Force cleanup if something goes wrong
/workspaces/swtfe/scripts/mudctl.sh kill-all
/workspaces/swtfe/scripts/mudctl.sh cleanup
```

### 3. Interactive Development Mode
```bash
# Launch interactive interface for live monitoring
/workspaces/swtfe/scripts/mudctl.sh interactive

# Advanced interface with live log streaming
/workspaces/swtfe/scripts/mudctl.sh advanced
```

## C17 Modernization Guidelines

### Buffer Overflow Prevention
```bash
# ALWAYS use safe string functions
char buffer[MAX_STRING_LENGTH];
snprintf(buffer, sizeof(buffer), "Player %s has %d hit points", 
         ch->name, ch->hit);

# NOT sprintf(buffer, "Player %s has %d hit points", ch->name, ch->hit);

# Test immediately after changes:
# /workspaces/swtfe/scripts/mudctl.sh restart
```

### Handle Deprecated Features
```c
// Inline functions - use static inline in headers:
static inline bool IS_NPC(CHAR_DATA *ch) {
    return (ch->npc_index != NULL);
}

// For variable-length arrays, use malloc:
void process_data(int size) {
    char *buffer = malloc(size);
    if (!buffer) {
        bug("Memory allocation failed");
        return;
    }
    // ... use buffer ...
    free(buffer);
}
```

### Compiler Flags and Testing
```makefile
CFLAGS = -std=c17 -Wall -Wextra -Wno-deprecated-declarations
# Use -Wno-deprecated-declarations during transition only

# After compilation, ALWAYS test:
# cd /workspaces/swtfe && scripts/mudctl.sh restart
```

## Debugging and Troubleshooting

### Common Issues Resolution
```bash
# Server won't start after code changes:
/workspaces/swtfe/scripts/mudctl.sh status
/workspaces/swtfe/scripts/mudctl.sh logs 100 | grep -i error

# GLIBC compatibility issues:
/workspaces/swtfe/scripts/check_system.sh

# Port conflicts or stuck processes:
/workspaces/swtfe/scripts/mudctl.sh kill-all
/workspaces/swtfe/scripts/mudctl.sh start

# Core dumps or crashes:
ls -la /workspaces/swtfe/core/
/workspaces/swtfe/scripts/mudctl.sh logs 200 | grep -i "crash\|segmentation\|core"
```

### Development Testing Protocol
1. **Pre-change**: Note current server status with `scripts/mudctl.sh status`
2. **Build**: Compile with zero warnings in `src/`
3. **Stop**: Use `scripts/mudctl.sh stop` (graceful shutdown)
4. **Start**: Use `scripts/mudctl.sh start` (test new code)
5. **Verify**: Check `scripts/mudctl.sh status` and `scripts/mudctl.sh logs 20`
6. **Monitor**: If issues arise, use `scripts/mudctl.sh tail` for live monitoring

## GMCP Implementation (Minimal)

### Core Functions (add to `src/gmcp.c`)
```c
void gmcp_init(void);
void send_gmcp(CHAR_DATA *ch, const char *namespace, const char *json);
void handle_gmcp_input(CHAR_DATA *ch, const char *input);

// After implementing, test with:
// /workspaces/swtfe/scripts/mudctl.sh restart
// /workspaces/swtfe/scripts/mudctl.sh logs 50 | grep -i gmcp
```

### Testing GMCP Implementation
```bash
# Start server and monitor for GMCP-related logs
/workspaces/swtfe/scripts/mudctl.sh restart
/workspaces/swtfe/scripts/mudctl.sh tail | grep -i "gmcp\|telnet\|negotiation"

# Test with GMCP-capable client or telnet manual testing
```

## Security Patterns

### Password Handling
```c
// Continue using Argon2 - example from secure_password.c:
bool verify_password(const char *password, const char *hash) {
    return argon2_verify(hash, password, strlen(password), 
                        Argon2_id) == ARGON2_OK;
}

// Test password functionality after changes:
// /workspaces/swtfe/scripts/mudctl.sh restart
// Connect and test login process
```

### Input Validation
```c
// ALWAYS bounds-check user input
void do_say(CHAR_DATA *ch, char *argument) {
    char buf[MAX_INPUT_LENGTH];
    
    if (!argument || !*argument) {
        send_to_char("Say what?\n\r", ch);
        return;
    }
    
    // Truncate safely if needed
    snprintf(buf, sizeof(buf), "%s says '%.*s'\n\r", 
             ch->name, MAX_SAY_LENGTH, argument);
}
```

## What NOT to Change

### Preserve These Patterns
```c
// MUD command structure - NEVER modify:
typedef void DO_FUN(CHAR_DATA *ch, char *argument);
#define DECLARE_DO_FUN(fun) DO_FUN fun

// Data structures - keep linked lists as-is:
struct char_data {
    struct char_data *next;          // Don't optimize to arrays
    struct char_data *next_in_room;  // These patterns are intentional
    // ... rest of structure unchanged
};

// Global variables - NEVER remove:
extern CHAR_DATA *first_char;  // Even if they seem unused
extern bool fBootDb;           // Area loading depends on these
```

## Migration Strategy

### Phase 1: Compilation
- Update compiler flags to `-std=c17`
- Fix inline function definitions
- Replace sprintf/strcpy with safe variants
- **Test each change**: `scripts/mudctl.sh restart` after each fix

### Phase 2: Protocol Support  
- Add minimal GMCP telnet negotiation
- Implement core character/room data namespaces
- **Test with clients**: Use `scripts/mudctl.sh tail` to monitor connections

### Phase 3: Validation
- Full compilation with zero warnings
- **Server stability**: Use `scripts/mudctl.sh status` to verify uptime
- **Functionality test**: Basic commands work (who, look, say)
- **Data integrity**: Areas load correctly, player files compatible

## Server Management Quick Reference

### Essential Commands
```bash
# Status and monitoring
/workspaces/swtfe/scripts/mudctl.sh status
/workspaces/swtfe/scripts/mudctl.sh logs [count]
/workspaces/swtfe/scripts/mudctl.sh tail

# Server control
/workspaces/swtfe/scripts/mudctl.sh start [port]
/workspaces/swtfe/scripts/mudctl.sh stop
/workspaces/swtfe/scripts/mudctl.sh restart [port]

# Development helpers
/workspaces/swtfe/scripts/mudctl.sh interactive  # Menu interface
/workspaces/swtfe/scripts/mudctl.sh cleanup      # Clean old logs/cores
/workspaces/swtfe/scripts/check_system.sh        # System validation

# Emergency recovery
/workspaces/swtfe/scripts/mudctl.sh kill-all     # Force kill everything
/workspaces/swtfe/scripts/mudctl.sh fallback     # Maintenance mode
```

### File Locations
- **Source Code**: `/workspaces/swtfe/src/`
- **Server Tools**: `/workspaces/swtfe/scripts/`
- **Game Data**: `/workspaces/swtfe/area/`, `/workspaces/swtfe/planets/`, `/workspaces/swtfe/space/`
- **Logs**: `/workspaces/swtfe/log/`
- **Core Dumps**: `/workspaces/swtfe/core/`

## Testing Checklist
- [ ] Builds with no warnings: `cd src && make clean && make`
- [ ] Server starts without crashes: `scripts/mudctl.sh start`
- [ ] Status shows running: `scripts/mudctl.sh status`
- [ ] No error logs: `scripts/mudctl.sh logs 50`
- [ ] Telnet connections work: Test basic connectivity
- [ ] Basic commands functional: who, look, say, tell
- [ ] GMCP negotiation works: Check with supporting clients
- [ ] Password authentication: Test login process
- [ ] No memory issues: Monitor for crashes or unusual behavior

## Remember
This is battle-tested legacy code with decades of accumulated functionality. Use the server management tools to **validate every change immediately**. The `/workspaces/swtfe/scripts/mudctl.sh` tool is your primary interface for testing - use it after every code modification to ensure the server starts and runs correctly.

Focus on **security** (buffer overflows, password hashing) and **protocol support** (GMCP) while preserving the existing game logic that players depend on. The server management tools will help you catch issues quickly during development.