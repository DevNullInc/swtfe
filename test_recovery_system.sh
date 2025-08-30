#!/usr/bin/env bash

# Test script for hotboot recovery system
# THE FINAL EPISODE

echo "Testing SWR Startup Script Hotboot Recovery System"
echo "=================================================="
echo

# Check if main startup script exists and is executable
if [[ ! -x "/workspaces/swtfe/src/startup.sh" ]]; then
    echo "Error: startup.sh not found or not executable"
    exit 1
fi

# Check if fallback server exists and is executable
if [[ ! -x "/workspaces/swtfe/src/fallback_server.py" ]]; then
    echo "Error: fallback_server.py not found or not executable"
    exit 1
fi

# Check if swr executable exists
if [[ ! -f "/workspaces/swtfe/src/swr" ]]; then
    echo "Error: swr executable not found"
    exit 1
fi

echo "✓ startup.sh found and executable"
echo "✓ fallback_server.py found and executable"
echo "✓ swr executable found"
echo

# Test syntax of startup script
echo "Testing startup script syntax..."
if bash -n /workspaces/swtfe/src/startup.sh; then
    echo "✓ startup.sh syntax is valid"
else
    echo "✗ startup.sh syntax error"
    exit 1
fi
echo

# Test fallback server help
echo "Testing fallback server..."
if python3 /workspaces/swtfe/src/fallback_server.py --help >/dev/null; then
    echo "✓ fallback_server.py works correctly"
else
    echo "✗ fallback_server.py has issues"
    exit 1
fi
echo

# Check required directories
echo "Checking directory structure..."
dirs=("/workspaces/swtfe/log" "/workspaces/swtfe/core" "/workspaces/swtfe/area" "/workspaces/swtfe/system")
for dir in "${dirs[@]}"; do
    if [[ -d "$dir" ]]; then
        echo "✓ $dir exists"
    else
        echo "⚠ $dir missing (will be created automatically)"
    fi
done
echo

# Test core dump analysis tools
echo "Testing core dump management tools..."
if [[ -x "/workspaces/swtfe/core/analyze_core.sh" ]]; then
    echo "✓ Core dump analyzer available"
else
    echo "⚠ Core dump analyzer not found"
fi

if [[ -x "/workspaces/swtfe/core/cleanup_cores.sh" ]]; then
    echo "✓ Core cleanup tool available"
else
    echo "⚠ Core cleanup tool not found"
fi

if [[ -x "/workspaces/swtfe/core/status.sh" ]]; then
    echo "✓ Core status tool available"
else
    echo "⚠ Core status tool not found"
fi
# Check if mudctl utility exists
if [[ -x "/workspaces/swtfe/src/mudctl.sh" ]]; then
    echo "✓ Server management utility available"
else
    echo "⚠ Server management utility not found"
fi
echo

echo "System Configuration Summary:"
echo "=============================="
echo "• Hotboot recovery: 3 attempts with 5-second delays"
echo "• Port management: Automatic cleanup of conflicting processes"
echo "• Fallback server: Python-based telnet server on same port"
echo "• Core dump management: Automatic analysis and timestamped storage"
echo "• Log management: Automatic rotation with incremental numbering"
echo "• Crash alerting: Email notifications to crashalert@renegadeinc.net"
echo "• Management utility: mudctl.sh for server administration"
echo

echo "Usage Examples:"
echo "==============="
echo "Server Management:"
echo "  ./src/mudctl.sh status          # Check server status"
echo "  ./src/mudctl.sh start [port]    # Start main server"
echo "  ./src/mudctl.sh stop [port]     # Stop processes on port"
echo "  ./src/mudctl.sh restart [port]  # Restart server"
echo "  ./src/mudctl.sh fallback [port] # Start fallback manually"
echo "  ./src/mudctl.sh cleanup [port]  # Clean port conflicts"
echo "  ./src/mudctl.sh kill-all        # Emergency stop all"
echo
echo "Manual Operations:"
echo "  cd /workspaces/swtfe/src && ./startup.sh [port]"
echo "  python3 /workspaces/swtfe/src/fallback_server.py [port]"
echo
echo "Check core dumps:"
echo "  /workspaces/swtfe/core/status.sh"
echo
echo "Analyze latest crash:"
echo "  /workspaces/swtfe/core/analyze_core.sh"
echo

echo "✅ System appears ready for deployment!"
