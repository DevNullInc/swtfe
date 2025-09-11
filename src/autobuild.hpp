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
 *                               SWR Autobuild module                                    *
 ****************************************************************************************/

#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <cstddef>

// Forward declarations for types used in function signatures
struct CharData; // Align with typedef in account.hpp and imccfg.hpp
using CharDataPtr = std::shared_ptr<CharData>;
using CMDF = void(*)(CharDataPtr, std::string_view);

// ============================================================================
// AutoBuild System - Function Declarations
// ============================================================================

/*
 * List management commands for building and maintaining text-based lists
 * Used primarily for autobuild functionality and data management
 */

// Command function declarations
CMDF do_create_list(std::shared_ptr<CharData> ch, std::string_view argument);   // Create new list file
CMDF do_addto_list(std::shared_ptr<CharData> ch, std::string_view argument);    // Add text to existing list
CMDF do_showlist(std::shared_ptr<CharData> ch, std::string_view argument);      // Display list contents with line numbers
CMDF do_remlist(std::shared_ptr<CharData> ch, std::string_view argument);       // Remove specific line from list

// ============================================================================
// Configuration Constants
// ============================================================================

// Note: Implementation constants are in autobuild.cpp anonymous namespace
// These are the public interface limits

constexpr std::size_t AutobuildMaxFilenameSize = 512;      // Maximum filename length including path
constexpr std::size_t AutobuildMaxLinesPerList = 10000;   // Safety limit for list file size
