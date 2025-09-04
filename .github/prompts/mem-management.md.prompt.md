---
mode: ask
---
# Copilot Prompt: Modernize MUD Memory Management (C++23)

**You are refactoring a legacy MUD codebase for C++23 standards. All memory handling must use modern containers, smart pointers, and memory resources. NO raw pointers. NO manual malloc/free. Follow precise conversion patterns below.**

## Overview

Legacy C/C++ MUD code is filled with unsafe allocations and leaks. Your job: systematically upgrade all memory handling to use C++23 features. Prioritize safety, efficiency, and maintainability. Do NOT change existing business logic—focus on memory patterns ONLY.

***

## Convert Legacy Allocations

- Replace all `malloc`, `free`, and raw pointers with smart pointers and memory resources.
- Use `std::unique_ptr`, `std::shared_ptr`, and `std::pmr::*` everywhere.
- All strings become `std::pmr::string` allocated from a `std::pmr::unsynchronized_pool_resource`.
- Do not use raw `char *` or legacy string buffers.

***

### Example Upgrades

#### Strings
```cpp
// BEFORE:
char *player_name = malloc(strlen(name) + 1);
strcpy(player_name, name);

// AFTER (C++23, pools):
#include <memory_resource>
std::pmr::string player_name{name, &string_pool};
```

#### Objects
```cpp
// BEFORE:
BOUNTY_DATA *bounty = malloc(sizeof(BOUNTY_DATA));

// AFTER:
auto bounty = std::make_unique<BOUNTY_DATA>();
```

#### Temporary Buffers
```cpp
// BEFORE:
char *buf = malloc(1024);
sprintf(buf, "...");

// AFTER (C++20+):
auto buf = std::format("...", ...); // Use PMR string if pool required
```

***

## Memory Pools (Arena Allocators)

- Create thread_local `std::pmr::unsynchronized_pool_resource` objects in a mud namespace.
- Use pools for high-churn objects: strings, temp buffers, game entities.
- All allocations use PMR-aware containers: `std::pmr::vector`, `std::pmr::unordered_map`, etc.

#### Example Pool Setup
```cpp
namespace mud {
inline thread_local std::pmr::unsynchronized_pool_resource string_pool;
inline thread_local std::pmr::unsynchronized_pool_resource object_pool;
}
```
- All allocated objects default to these pools via `std::pmr::polymorphic_allocator<T>`.

***

## Code Migration Rules

- Convert all `STRALLOC`, `STRFREE`, and custom string pools to `std::pmr::string`.
- Convert all game objects to use smart pointers (`std::unique_ptr`, `std::shared_ptr`).
- Periodically reset pool resources as required.
- NEVER change save/load logic—leave migration for later phases.
- Use RAII everywhere. Objects clean up on scope exit.

***

## Monitoring & Debugging

- Track allocation metrics via static variables.
- Use Admin functions for memory stats via pool resource queries.
- NO raw pointer arithmetic, no manual lifecycles.

***

## Safety

- If a function migrates memory, preserve the original code in comments until pool system is stable.
- Any fallback logic should be clearly marked.

***

## Expectation

You will produce **modern, idiomatic C++23 code** that:  
- Eliminates memory leaks and buffer overruns  
- Is readable, maintainable, and type-safe  
- Runs efficiently using standard library pool allocators and smart pointers  
- Is compatible with incremental integration and regression testing

***

**Do not guess. Do not invent new logic. Rewrite legacy memory management exactly as shown for C++23. Let the rest of the game’s design remain unchanged until further instruction.**