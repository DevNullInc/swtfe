---
applyTo: '**'
---
# Copilot Instructions for C++23/x64 Full Modernization (No Mercy)

## Mission Statement
Take the crusty SWR/MUD codebase and goddamn **obliterate** all outdated, C-style shit. Rip out and refactor every legacy pattern for C++23. Target: native x64, smart pointers everywhere, RAII for life, bulletproof safety, and a codebase that looks and *acts* like it was written for the next decade—NOT 1997. Assume nobody cares about backwards compatibility except for persistent data and gameplay logic. If it doesn’t scream “modern,” kill it.

---

## Scorched Earth Modernization Rules

- **All .c/.h become .cpp/.hpp.** Convert source and header files as you go. Update every include and build script. Nuke C-only dependencies.
- **Strict C++23.** No C-style memory handling, pointer math, or hacks. All code must compile with `-std=c++23` and pass x64 builds without warnings.
- **All memory uses smart pointers!** No more raw pointers. Use `std::unique_ptr`, `std::shared_ptr`, and `std::pmr::*` for pools.
- **No malloc/free/new/delete** in core logic—use containers and ownership semantics.
- **All string management:** Move everything to `std::string`, `std::string_view`, and (where performance is hot) `std::pmr::string`.
- **Replace all fixed-size and legacy buffer code:** Every buffer or string manipulation must be safe and/or use standard C++ containers or format functions like `std::format`.
- **Every struct becomes a class** where applicable, leverage real encapsulation, ctors, dtors, and access control.
- **Replace macros with constexpr, inline, or templates** where sane. No more preprocessor acrobatics for core code.
- **Global variables refactored into singleton classes or just vanish** unless they're critical for persistence or scripting.
- **All 32-bit shenanigans must die:** Update integer types (`size_t`, `uint64_t`, etc.), audit pointer arithmetic, and make every critical path x64-native.
- **Any function using void* gets refactored for type safety**—ditch generic casting for template magic or base class pointers.
- **All builds target x64. No weird legacy flags. No 32-bit assumptions allowed. Warnings for anything must be crushed**

---

## Development Workflow

1. **Rename every relevant file to .cpp/.hpp.** Update all include guards and `#include` directives. Break builds intentionally so you can fix them.
2. **Work branch-by-branch:** Don’t half-ass upgrades—convert and clean each module entirely before merging.
3. **Every code change must build cleanly on x64, C++23, zero warnings** (`-Werror` is your friend).
4. **Unit tests:** Add them as you rewrite. Use modern frameworks (e.g., Catch2 or doctest).
5. **Manual testing:** After major feature rewrites, reconnect and hammer with real player data and GMCP/telnet as before.
6. **Automate your migration:** Write scripts to mass-replace file extensions and header references. Don’t waste time clicking.

---

## Data and Protocols

- **Only preserve binary/player/area file compatibility (as needed).**
- All communication and protocol handling should be upgraded to C++ networking APIs where possible, with full 64-bit safety.
- Password logic remains Argon2, but now inside a real class with resource management.

---

## What You Can and *Should* Break

- Legacy functions? If they're unsafe, obsolete, or pure C-hackery, refactor or torch them.
- Ugly macros? Nuke from orbit, rewrite as modern, type-safe constructs.
- C-style arrays and malloc’d memory? Gone. Use vectors, hashmaps, or whatever is best in C++23.
- If it’s not thread-safe or reentrant and needs to be, refactor for concurrency.

---

## DON’T:

- DON’T leave any part of the main game codebase in C; migrate all core modules.
- DON’T “just wrap” malloc patterns—rip and replace.
- DON’T keep old function and type names unless legacy persistence or scripting REQUIRES the same names/type sizes.
- DON’T keep any code that “feels legacy” in pattern or idiom.
- DON’T waste time backward-porting for old compilers or 32-bit support.

---

## Your Motto

**If it’s not C++23 and ready for x64, it’s getting deleted, rewritten, or replaced.**
No mercy. No C-lingo left behind. And always build/test after every chunk is modernized.