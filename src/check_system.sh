#!/bin/bash
#*****************************************************************************************
# *                      .___________. __    __   _______                                 *
# *                      |           ||  |  |  | |   ____|                                *
# *                      `---|  |----`|  |__|  | |  |__                                   *
# *                          |  |     |   __   | |   __|                                  *
# *                          |  |     |  |  |  | |  |____                                 *
# *                          |__|     |__|  |__| |_______|                                *
# *                                                                                       *
# *                _______  __  .__   __.      ___       __                               *
# *               |   ____||  | |  \ |  |     /   \     |  |                              *
# *               |  |__   |  | |   \|  |    /  ^  \    |  |                              *
# *               |   __|  |  | |  . `  |   /  /_\  \   |  |                              *
# *               |  |     |  | |  |\   |  /  _____  \  |  `----.                         *
# *               |__|     |__| |__| \__| /__/     \__\ |_______|                         *
# *                                                                                       *
# *      _______ .______    __       _______.  ______    _______   _______                *
# *     |   ____||   _  \  |  |     /       | /  __  \  |       \ |   ____|               *
# *     |  |__   |  |_)  | |  |    |   (----`|  |  |  | |  .--.  ||  |__                  *
# *     |   __|  |   ___/  |  |     \   \    |  |  |  | |  |  |  ||   __|                 *
# *     |  |____ |  |      |  | .----)   |   |  `--'  | |  '--'  ||  |____                *
# *     |_______|| _|      |__| |_______/     \______/  |_______/ |_______|               *
# *****************************************************************************************
# *                                                                                       *
# * Star Wars: The Final Episode additions and changes from the Star Wars Reality code    *
# * copyright (c) 2025 /dev/null Industries - StygianRenegade                             *
# *                                                                                       *
# * Star Wars Reality Code Additions and changes from the Smaug Code copyright (c) 1997   *
# * by Sean Cooper                                                                        *
# *                                                                                       *
# * Starwars and Starwars Names copyright(c) Lucas Film Ltd.                              *
# *****************************************************************************************
# * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
# * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
# *                                                                                       *
# * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
# *                                                                                       *
# * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
# * and Sebastian Hammer.                                                                 *
# *****************************************************************************************
# System Dependency Check Script for SWR Server
# This script checks if the server can run on the current system
#
# Usage:
#   ./check_system.sh
#
# Prerequisites:
#   - The SWR server executable (swr) should be present in the same directory as this script.
#   - The script should be run on a Unix-like system (Linux).
#   - The following commands should be available: ldd, timeout, chmod, grep, sed.
#
# This script will:
#   - Check for the presence and permissions of the server executable.
#   - Verify required shared libraries are available.
#   - Attempt a basic execution test of the server.

# Get the script directory and set up paths dynamically
SCRIPT_DIR="$(dirname "$0")"
SRC_DIR="$(cd "$SCRIPT_DIR" && pwd)"
SWR_EXECUTABLE="$SRC_DIR/swr"

echo "========================================"
echo "SWR Server System Dependency Check"
echo "========================================"

# Basic system info
echo "System Information:"
echo "  OS: $(uname -s)"
echo "  Architecture: $(uname -m)"
echo "  Kernel: $(uname -r)"
echo ""

# Check if server executable exists
echo "Server Executable Check:"
if [[ -f "$SWR_EXECUTABLE" ]]; then
    echo "  ✓ Server executable found: $SWR_EXECUTABLE"
    
    # Check permissions
    if [[ -x "$SWR_EXECUTABLE" ]]; then
        echo "  ✓ Server executable has execute permissions"
    else
        echo "  ✗ Server executable is not executable"
        echo "    Fix with: chmod +x $SWR_EXECUTABLE"
        exit 1
    fi
else
    echo "  ✗ Server executable not found: $SWR_EXECUTABLE"
    echo "    Please compile the server or check the path"
    exit 1
fi

echo ""

# Check library dependencies
echo "Library Dependency Check:"
if command -v ldd >/dev/null 2>&1; then
    echo "  Checking with ldd..."
    
    # Capture ldd output
    ldd_output=$(ldd "$SWR_EXECUTABLE" 2>&1)
    ldd_exit_code=$?
    
    if [[ $ldd_exit_code -eq 0 ]]; then
        echo "  ✓ Library dependency check completed"
        
        # Check for missing libraries
        missing_libs=$(echo "$ldd_output" | grep "not found" || true)
        if [[ -n "$missing_libs" ]]; then
            echo "  ✗ Missing library dependencies:"
            echo "$missing_libs" | sed 's/^/    /'
            echo ""
            echo "  Common solutions:"
            echo "    - Install missing packages using your package manager"
            echo "    - Recompile the server on this system"
            echo "    - Use a compatible binary"
            exit 1
        else
            echo "  ✓ All required libraries are available"
        fi
        
        # Check glibc version specifically
        glibc_reqs=$(echo "$ldd_output" | grep "GLIBC_" || true)
        if [[ -n "$glibc_reqs" ]]; then
            echo "  GLIBC Requirements:"
            echo "$glibc_reqs" | sed 's/^/    /'
            
            # Get system glibc version
            if command -v ldd >/dev/null 2>&1; then
                system_glibc=$(ldd --version 2>&1 | head -1 | grep -o '[0-9]\+\.[0-9]\+' || echo "unknown")
                echo "  System GLIBC Version: $system_glibc"
            fi
        fi
        
    else
        echo "  ✗ Library dependency check failed"
        echo "  Error output:"
        echo "$ldd_output" | sed 's/^/    /'
        exit 1
    fi
else
    echo "  ⚠ ldd command not available - cannot check dependencies"
fi

echo ""

# Test basic execution (very brief)
echo "Basic Execution Test:"
echo "  Testing if server can start (will exit quickly)..."

# Try to run the server with immediate exit
timeout 2s "$SWR_EXECUTABLE" --help >/dev/null 2>&1 || true
exit_code=$?

case $exit_code in
    0|1|2)
        echo "  ✓ Server executable can run (exit code: $exit_code)"
        ;;
    124)
        echo "  ✓ Server started successfully (timeout reached)"
        ;;
    126)
        echo "  ✗ Permission denied when trying to execute"
        echo "    Fix with: chmod +x $SWR_EXECUTABLE"
        exit 1
        ;;
    127)
        echo "  ✗ Command not found or missing dependencies"
        echo "    Check library dependencies above"
        exit 1
        ;;
    *)
        echo "  ⚠ Unexpected exit code: $exit_code"
        echo "    This may indicate system compatibility issues"
        ;;
esac

echo ""
echo "========================================"
echo "System Check Complete"
echo "========================================"

# Final assessment
echo "Overall Assessment:"
if [[ -f "$SWR_EXECUTABLE" ]] && [[ -x "$SWR_EXECUTABLE" ]]; then
    if command -v ldd >/dev/null 2>&1; then
        missing_libs=$(ldd "$SWR_EXECUTABLE" 2>&1 | grep "not found" || true)
        if [[ -z "$missing_libs" ]]; then
            echo "  ✓ System appears ready to run SWR server"
            echo ""
            echo "You can now start the server with:"
            echo "  ./startup.sh"
            echo "  or"
            echo "  ./mudctl.sh start"
        else
            echo "  ✗ Missing dependencies prevent server startup"
            echo "  Please resolve the library issues shown above"
        fi
    else
        echo "  ⚠ Cannot fully verify - ldd not available"
        echo "  Try starting the server to see if it works"
    fi
else
    echo "  ✗ Server executable issues prevent startup"
    echo "  Please resolve the file permission issues shown above"
fi
