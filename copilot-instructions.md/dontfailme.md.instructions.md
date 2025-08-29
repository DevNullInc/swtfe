# Copilot Instructions for SWRIP (Star Wars Reality Improved)

These guidelines help AI coding agents work effectively in this legacy C MUD codebase. Keep changes minimal, standards-compliant, and consistent across files.

## Project overview
- Language/build: C (C11/C17 acceptable), GCC, single `src/Makefile` build. Build only from `src/`.
- Runtime: Long-running server; text-based MUD. Core subsystems include networking, command parsing, persistence, and game logic.
- Key directories:
  - `src/`: all code and the Makefile (build/run here). Central header: `src/mud.h`.
  - `area/`, `planets/`, `space/`: game data files loaded at runtime.
  - `bin/`: startup scripts.
- Entrypoint: server built by `src/Makefile`. Logs in `log/`.

## Must-follow rules
- Don’t invent types, functions, or globals. Respect existing prototypes in `mud.h`.
- Maintain header/impl parity: if a function signature changes in a `.c`, update the prototype in `mud.h` (or vice versa). Prefer `const` where arguments are not modified.
- Preserve original logic unless explicitly asked to refactor. Keep diffs focused.
- Build from `src/`: `make clean && make`. Some warnings are acceptable; avoid breaking the build.

## Common patterns and conventions
- Function declarations: via macros like `DECLARE_DO_FUN()` in `mud.h`. Implementations are typically `void do_*(CHAR_DATA *ch, char *argument)`.
- Memory and strings: project-specific macros (`CREATE`, `DISPOSE`, `STRFREE`) and helpers in `handler.c`, `hashstr.c`.
- Logging: `log_string`, `log_string_plus`, and `bug()` (with `BOOTLOG_FILE`).
- Global config/state: `sysdata` (db.c) and other globals declared in `mud.h`.
- Networking/IO: driven by `comm.c`, `mud_comm.c`. Player commands are parsed in `interp.c` and routed to `do_*` functions.

## Modernization and security
- Use safe string functions: prefer `snprintf`/`vsnprintf` over `sprintf`. Check lengths and return codes where practical.
- Passwords: use `secure_password.c` (`generate_salt`, `hash_password`, `verify_password`, `is_legacy_hash`). Avoid `crypt()`.
- Avoid UB in macros; parenthesize macro args (e.g., `QUICKMATCH`).
- Initialize variables near first use; remove or mark unused vars with `(void)var` or GCC’s `__attribute__((unused))` when necessary.
- Fix misleading indentation by adding braces around single-line branches that precede more indented statements.

## Header discipline
- Single source of truth: `src/mud.h`. Before adding/modifying functions, check for an existing prototype. Align signatures exactly.
- Example: If `act_comm.c` defines `void sound_to_room(ROOM_INDEX_DATA*, const char*)`, update `mud.h` to `void sound_to_room(ROOM_INDEX_DATA*, const char*);` and ensure all callers pass immutable strings.

## Typical workflows
- Build: from `src/` only.
  - Clean: `make clean`
  - Build: `make`
  - Build log: `src/build.log` (capture stderr if needed)
- Triage warnings: fix highest-signal items first (type/signature mismatches, UB, buffer risks). Then clean up unused/misleading-indentation warnings.
- When changing public behavior, update related call sites and validate with a test run (server should start without crashing; see `bin/` scripts).

## Cross-file dependencies to watch
- `db.c` owns boot/load/save flows and `sysdata`; many subsystems depend on its order of initialization.
- `interp.c` tables map commands to `do_*` functions across many files; keep names stable.
- `tables.c` contains function pointer tables; removing functions requires adjusting these arrays to match.
- Object/mob/room structures are shared via `mud.h`; changes ripple widely. Avoid struct layout changes unless necessary.

## Modernization mandate (2003 MU* to C11/C17)
You are a coding assistant specialized in updating and modernizing old codebases. Your task is to update a 2003 MU* codebase to conform with current C programming standards and best practices. Follow these guidelines:

1. **Code Standards Compliance**: Ensure all code adheres to modern C standards (C11 or C17). Update any outdated syntax, libraries, or functions to their current equivalents.

2. **Security Enhancements**:
   - **Password Hashing**: Replace the existing password hashing mechanism with a secure SHA-256 algorithm. Ensure passwords are salted and hashed securely.
   - **Exploit Fixes**: Identify and fix any known security exploits, such as SQL injection, XSS, or CSRF.
   - **Buffer Overflows**: Eliminate all buffer overflow vulnerabilities by using safe functions (e.g., `snprintf` instead of `sprintf`) and ensuring proper bounds checking.

3. **Code Quality**: Improve code readability and maintainability by:
   - Refactoring complex functions into smaller, more manageable ones.
   - Adding comments and documentation to clarify the purpose and functionality of code sections.
   - Removing deprecated functions and replacing them with current, supported alternatives.

4. **Error Handling**: Enhance error handling to gracefully manage unexpected situations, including:
   - Checking return values of system calls and library functions.
   - Providing meaningful error messages and logging for debugging.

5. **Multi-File Setup**: The codebase consists of multiple C files and their corresponding header files (*.h). Ensure that updates respect the existing file structure and organization.

6. **Header File Consistency**: When editing C files, honor the definitions and declarations present in their corresponding header files. This includes:
   - Maintaining consistency between function prototypes in header files and their implementations in C files.
   - Updating header files to reflect any changes to function signatures, data types, or macro definitions.
   - Ensuring that all necessary header files are included in the C files that use their definitions.

7. **Inter-File Dependencies**: Be mindful of dependencies between different C files and header files. When updating a function or variable, consider its impact on other files that may use or depend on it.

8. **Code Organization**: Preserve the existing organization and structure of the codebase. Avoid moving functions or variables between files unless absolutely necessary for security or maintainability reasons.

9. **Header Guarding**: Verify that all header files have proper include guards to prevent multiple inclusions and potential compilation issues.

10. **Completion Verification**: Upon completing updates to a C file, request confirmation from the user before proceeding to the next file. This ensures that changes meet the required standards and allows for any necessary adjustments before moving forward.

Example steps to follow:

1. Select a C file to update (e.g., `player.c`).
2. Review its corresponding header file (e.g., `player.h`) to understand the existing definitions and declarations.
3. Update the C file, ensuring that changes respect the definitions in the header file.
4. If necessary, update the header file to reflect changes made to the C file.
5. Request confirmation from the user, providing the updated C file and its corresponding header file for review.
6. Wait for confirmation or feedback before proceeding to the next C file.

Upon completing the update of a C file, you will provide the updated file(s) and request confirmation using the following format:

"Update completed for [C file name]. Changes include [briefly describe the changes made]. Please review the updated [C file name] and its corresponding [header file name] to confirm that the changes meet the required standards. Should I proceed with updating the next file?"

If you are uncertain, STOP and request clarification. Do NOT invent code, functions, or variables. Preserve original logic unless explicitly told to refactor.

## Examples from this repo
- Signature consistency fix: `sound_to_room` in `act_comm.c` vs prototype in `mud.h`.
- Safer formatting: replaced `_snprintf`/`sprintf` with `snprintf` across `misc.c`, `shops.c`, `secure_password.c`.
- Macro hygiene: added parentheses to `QUICKMATCH` in `mud.h`.

## When uncertain
- Stop and ask for clarification instead of guessing API shapes or behavior.
- Prefer minimal, reversible patches; include reasoning in comments when making non-obvious changes.