---
applyTo: '**'
---

# Copilot/AI Agent Instructions for Modernizing a Large Hybrid MUD Codebase (AFKMud/DikuMUD/SMAUG, C++23/x64)

## Mission Statement

Modernize this **multi-file, multi-module legacy codebase** (AFKMud, DikuMUD, SMAUG origins) for C++23 and x64. Prioritize idiomatic C++ ownership, containers, safety, performance, and maintainability. Remove legacy patterns unless essential for persistence or compatibility. The codebase must evolve as a clean, modern C++23/x64 project—**not just update one file at a time**.

---

## Commit Message Style: MAKE IT FUNNY

- Commit messages MUST be quirky, witty, or reference Star Wars or MUD humor.
- No dry, corporate messages allowed!
- Use puns, jokes, or cheeky commentary.
- Examples:
  - "Made the code less buggy than a Tatooine cantina"
  - "Refactored attributes—now with extra Force alignment"
  - "Banished legacy pointers to the Sarlacc pit"
  - "Renamed stuff so even Yoda would approve"
  - "Fixed a bug—Han shot first"
- If unsure, err on the side of humor and fun.

---

## Explicit Scope & Context

- **This is NOT a single-file project.** You are working with hundreds of .cpp/.hpp files, many with legacy C/C++ styles.
- Expect mixed code patterns, global variables, raw pointers, macros, and legacy build scripts.
- Modernization is a **systemic, multi-phase process**; do not assume the code is already modern or that only the open file matters.

---

## Modernization Playbook

- **Strict C++23/x64:** Code must build with `-std=c++23 -m64 -Werror` and zero warnings.
- **Smart pointers:** Replace raw pointers with smart pointers everywhere except for legacy persistence or external APIs.
- **No manual memory (new/delete/malloc/free):** Only allowed in wrappers/compatibility layers.
- **Replace C strings/buffers:** Use safe C++ containers and formatting (`std::string`, `std::string_view`, `std::pmr::string`).
- **Encapsulate legacy structs:** Refactor to classes with RAII and proper encapsulation.
- **Replace macros:** Use `constexpr`, `inline`, or templates for type safety.
- **Refactor globals:** Remove or convert to singletons except where persistence demands.
- **Audit integer/pointer types for x64:** Remove 32-bit assumptions.
- **Refactor void*/unsafe APIs:** Use type-safe alternatives.
- **Modernize build scripts:** All targets must support x64, C++23.
- **BUILD CLEAN after major edits. Fix all warnings/errors.**

---

## Header File (hpp) Error Handling Rule

- **ALWAYS cross-reference errors/warnings with the `.hpp` files included in the file being edited.**
- **If pointer/type errors, mismatches, or macro issues are detected,** first check if the root cause is in the relevant `.hpp` file(s).
- **Fix the root cause in the header file(s)** if possible, rather than patching each individual error in every `.cpp` file.
- **Batch Proposal:** If header file changes may resolve errors in several files, prepare a preview of all related changes and prompt for user review/approval before applying.
- Do NOT blindly fix errors line-by-line in the current file when a header change could resolve the issue globally.

---

## Legacy MUD Codebase Considerations

- **Game Data & Persistence:** Never break or change save/load formats or persistent data structures without explicit user approval. Always batch-preview changes affecting file I/O, game state, or player data.
- **Networking:** Legacy socket code may use outdated APIs. Suggest modern, portable abstractions (e.g., asio, C++ networking TS), but always preview protocol or buffer changes.
- **Threading:** Old code may be single-threaded or use unsafe patterns. Recommend modern concurrency carefully and batch-preview related changes.
- **Command/Scripting:** If command tables or scripts are present, review and update `/area/help.are` and related documentation with each change.
- **Testing:** Propose adding automated tests and modern CI/CD configurations for regression and unit testing.
- **Documentation:** Add Doxygen-style comments and module headers to legacy code as it is modernized.
- **Build System:** Migrate legacy build scripts to CMake or similar, with full support for x64 and C++23.
- **Platform Compatibility:** Audit and refactor OS/compiler-dependent code for portability.
- **Dead Code Audit:** Regularly propose batch removal or archiving of obsolete or unused code, with user sign-off.
- **Legacy Artifacts:** Expect legacy game logic, networking, and persistence code using outdated paradigms. Flag magic numbers, hardcoded limits, and old file formats for review and possible refactoring.
- **Game Data:** Audit serialization/deserialization routines and propose modern alternatives, but only change formats with explicit user approval.

---

## Agent Workflow

- **Within the open file:** Batch-fix all matching errors, warnings, and legacy patterns per modernization rules. Narrate intentions; do NOT pause for each edit.
- **Batch previews:** Use Copilot’s batch preview/review features so the user sees all proposed changes in one coherent diff before applying.
- **Multi-file/global edits:** Prepare and preview the complete change list. Request explicit user review/confirmation before proceeding.
- **Destructive/global actions:** Always halt and request user sign-off before mass renames, deletions, or global refactors.
- **Editing commands:** Review `/area/help.are` first; update/add help entries as commands are changed.
- **After batch changes:** Trigger "buildclean" and "clangcheck" for verification.

---

## Prohibited Actions

- Never run or suggest terminal-wide regex, shell scripts, or batch refactors without previewed diffs and user consent.
- Do not edit command structures without reviewing `/area/help.are` and updating documentation.
- Do not preserve legacy identifiers except for persistence/scripts.
- Never automate mass deletions or refactors without explicit preview and user approval.

---

## Allowed/Expected Actions

- Batch and preview all safe, context-matching edits within the open file—apply unless rejected.
- Use Copilot's review interface to preview all changes for a file in one step.
- For multi-file/global operations, always propose the full change set and wait for explicit user approval.
- Proactive warning/error fixes encouraged, only within the file being edited.

---

## CODE/ERROR CONTEXT RULE

- Only match error/warning context to the open file unless explicitly instructed otherwise.
- Only automate fixes inside the open/active file; queue other fixes for user review.

---

## TERMINAL COMMANDS

- Only run terminal/batch commands (e.g., extension renames, buildclean, clangcheck) after explicit user instruction.
- ALWAYS make a versioned backup before any mass/batch operation.

---

## Console commands to use

- `"buildclean"`: Clean build, log to build.log.
- `"clangcheck <file>"`: Static analysis, log to clang.log.

---

## DO THE FOLLOWING

- Verify all command and code structure changes before editing.
- Review and update `/area/help.are` for migrated/renamed commands.
- If `/area/help.are` does not exist, infer structure and add entries alphabetically.
- Narrate rationale for every agent-driven change or batch proposal.

---

## What NOT to Assume

- Do **NOT** assume this is a single file or a modern codebase.
- Do **NOT** assume all files use the same style, or that the code already builds clean.
- Do **NOT** assume legacy code is safe or idiomatic—**audit everything**.

---

## Your Motto

**If it’s not C++23 and ready for x64, it’s getting deleted, rewritten, or replaced. No mercy. No C-lingo left behind. Build/test after every major edit. Narrate critical changes and respect user review for all big moves.**

---
