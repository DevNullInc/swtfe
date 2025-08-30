# SWR MUD Server Recovery System

## /dev/null Industries

## Overview

A comprehensive crash recovery and fallback system for the Star Wars Reality MUD server that provides:

- **Automatic Hotboot Recovery**: Attempts to recover from crashes using the MUD's built-in hotboot system
- **Intelligent Port Management**: Automatically detects and cleans up port conflicts before starting
- **Enhanced Fallback Server**: Python-based maintenance server that uses the same port as the main server
- **Core Dump Management**: Automated analysis and storage of crash dumps
- **Administrative Tools**: Comprehensive management utility for server operations
- **Enhanced Monitoring**: Detailed logging and crash reporting

## System Architecture

```txt
Main Server Crash
        ↓
   Core Dump Analysis
        ↓
   Hotboot Attempt 1 ──→ Success? ──→ Continue Running
        ↓ Fail
   Hotboot Attempt 2 ──→ Success? ──→ Continue Running  
        ↓ Fail
   Hotboot Attempt 3 ──→ Success? ──→ Continue Running
        ↓ Fail
   Fallback Server ────→ Maintenance Mode
```

## Configuration

### Key Settings (in startup.sh)

```bash
MAX_HOTBOOT_ATTEMPTS=3      # Number of hotboot recovery attempts
HOTBOOT_DELAY=5             # Seconds between attempts
ALERT_EMAIL="crashalert@renegadeinc.net"  # Crash notification email
```

### Directory Structure

```txt
/workspaces/swtfe/
├── scripts/                # Portable management scripts (RECOMMENDED)
│   ├── mudctl.sh          # Main server control script
│   ├── check_system.sh    # System dependency checker
│   └── README.md          # Scripts documentation
├── src/
│   ├── startup.sh          # Main startup script with recovery
│   ├── fallback_server.py  # Python fallback server
│   ├── mudctl.sh          # Original script (legacy)
│   └── swr                 # Main MUD executable
├── core/                   # Core dump management
│   ├── analyze_core.sh     # GDB-based crash analysis
│   ├── cleanup_cores.sh    # Automated cleanup
│   ├── status.sh          # Status overview
│   └── README.md          # Core management docs
├── log/                   # Server logs (numbered: 1000.log, 1001.log, etc.)
└── system/               # System files including hotboot data
```

## Recovery Process

### Phase 1: Crash Detection

- Monitor server process for abnormal termination
- Detect and collect core dumps automatically
- Move core dumps to timestamped storage in `/core/`
- Generate comprehensive crash reports with GDB analysis

### Phase 2: Hotboot Recovery

The system attempts up to 3 hotboot recoveries:

1. **Check for Hotboot Data**: Verify `system/copyover.dat` exists
2. **Attempt Recovery**: Start server with hotboot arguments
3. **Monitor Success**: Verify server stability for extended period
4. **Fallback**: If hotboot fails, try normal restart
5. **Repeat**: Up to MAX_HOTBOOT_ATTEMPTS total attempts

### Phase 3: Fallback Server

If all recovery attempts fail:

1. **Start Fallback**: Launch Python maintenance server on same port
2. **Notify Players**: Display maintenance message to connecting users
3. **Log Status**: Create fallback status file for administrators
4. **Wait for Manual**: Require manual intervention to restore main server

## Hotboot Recovery Details

### How Hotboot Works

The MUD's built-in hotboot system:

- Saves all player connections and game state to `system/copyover.dat`
- Performs `execl()` to restart the process with special arguments
- Restores connections and game state on restart
- Provides seamless experience for connected players

### Startup Arguments

```bash
# Normal startup
./swr <port>

# Hotboot recovery
./swr <port> hotboot <control_socket> <imc_socket>
```

### Recovery Strategy

1. **Check Data Validity**: Ensure hotboot file exists and is recent
2. **Use Dummy Sockets**: Pass -1 for socket arguments (they're reconstructed)
3. **Extended Monitoring**: Give hotboot 15 seconds to stabilize
4. **Graceful Degradation**: Fall back to normal restart if hotboot fails

## Fallback Server Features

### Connection Handling

- **Multi-threaded**: Handle up to 10 concurrent connections
- **Timeout Management**: 30-second connection limit
- **Resource Protection**: Automatic cleanup of connections
- **Color Output**: ANSI-colored status messages

### Player Experience

```txt
═══════════════════════════════════════════════════════════════════════════════
                    Star Wars Reality MUD - Server Status
═══════════════════════════════════════════════════════════════════════════════

SERVER TEMPORARILY UNAVAILABLE

The main game server is currently down for maintenance or experiencing
technical difficulties. Our administrators have been notified and are
working to restore service.

What you can do:
• Check back in a few minutes
• Visit our website for updates  
• Contact us for urgent issues: crashalert@renegadeinc.net

Server Status: Maintenance Mode
Last Update: 2025-08-30 14:30:00

Connection will close in 30 seconds...
```

## Management Utility (mudctl.sh)

### Mud Control Overview

The `mudctl.sh` script provides comprehensive server management capabilities:

- **Port Management**: Detect and resolve port conflicts automatically
- **Process Control**: Start, stop, and restart server components
- **Status Monitoring**: Real-time server and port status information
- **Emergency Controls**: Force kill all processes when needed

**New Portable Version**: Use `./scripts/mudctl.sh` for improved portability and features.
**Legacy Version**: `./src/mudctl.sh` is the original version with hardcoded paths.

### Commands

```bash
# RECOMMENDED: Use portable scripts (work from any directory)
./scripts/mudctl.sh status             # Show detailed server status
./scripts/mudctl.sh start [port]       # Start main server (default: 4848)
./scripts/mudctl.sh stop               # Stop server gracefully
./scripts/mudctl.sh restart [port]     # Stop and restart server
./scripts/mudctl.sh cleanup            # Clean up old logs and cores
./scripts/mudctl.sh kill-all           # Emergency stop all processes
./scripts/mudctl.sh fallback [port]    # Start fallback server manually
./scripts/mudctl.sh logs [count]       # Show recent log entries
./scripts/mudctl.sh tail               # Follow live logs
./scripts/mudctl.sh check              # Run system dependency check

# LEGACY: Original commands (require running from src directory)
./src/mudctl.sh status [port]          # Show detailed server status  
./src/mudctl.sh start [port]           # Start main server
./src/mudctl.sh stop [port]            # Stop processes on port
./src/mudctl.sh restart [port]         # Stop and restart server
./src/mudctl.sh cleanup [port]         # Clean up port conflicts
./src/mudctl.sh kill-all               # Emergency stop all processes
./src/mudctl.sh fallback [port]        # Start fallback server manually
```

### Port Conflict Resolution

The system automatically handles port conflicts by:

1. **Detection**: Scan for processes using the target port
2. **Identification**: Show process details (PID, name, command)
3. **Graceful Termination**: Send SIGTERM to conflicting processes
4. **Force Termination**: Send SIGKILL if graceful shutdown fails
5. **Verification**: Confirm port is available before proceeding

## Usage Examples

### Normal Server Startup

```bash
cd /workspaces/swtfe/src
./startup.sh          # Default port 4848
./startup.sh 4850     # Custom port
```

### Manual Fallback Testing

```bash
# Start fallback server on test port
python3 src/fallback_server.py 4849

# Connect to test
telnet localhost 4849
```

### Core Dump Management

```bash
# View current status
./core/status.sh

# Analyze latest crash
./core/analyze_core.sh

# Analyze specific core
./core/analyze_core.sh core.swr.20250830_143000.12345

# Clean old dumps (30+ days)
./core/cleanup_cores.sh

# Preview cleanup
./core/cleanup_cores.sh --dry-run --verbose
```

### System Monitoring

```bash
# Check if main server is running
pgrep -f "swr.*4848"

# Check if fallback is running  
pgrep -f "fallback_server"

# View recent logs
tail -f log/$(ls log/*.log | tail -1)

# Check fallback status
cat fallback.status 2>/dev/null || echo "No fallback active"
```

## Administrative Tasks

### After a Crash

1. **Review Logs**: Check latest log file for crash details
2. **Analyze Core**: Use `./core/analyze_core.sh` for detailed analysis
3. **Check Recovery**: Verify if hotboot succeeded or fallback activated
4. **Fix Issues**: Address root cause before manual restart
5. **Monitor**: Watch for recurring crashes

### Manual Recovery

```bash
# Stop fallback server if running
pkill -f fallback_server

# Remove fallback status
rm -f fallback.status

# Clean up old hotboot data if corrupted
rm -f system/copyover.dat

# Restart main server
./startup.sh
```

### Maintenance

```bash
# Regular core dump cleanup (suggested cron job)
./core/cleanup_cores.sh --days 14

# Log rotation (if needed)
find log -name "*.log" -mtime +30 -delete

# Check system health
./test_recovery_system.sh
```

## Troubleshooting

### Common Issues

#### Hotboot Always Fails

- Check if `system/copyover.dat` exists and is valid
- Verify MUD was compiled with hotboot support
- Review server logs for hotboot-specific errors

#### Fallback Won't Start

- Ensure Python3 is installed: `python3 --version`
- Check port availability: `ss -tlnp | grep :4848`
- Verify script permissions: `ls -la src/fallback_server.py`

#### Core Dumps Not Generated

- Check ulimit settings: `ulimit -c` (should be "unlimited")
- Verify core dump location: `cat /proc/sys/kernel/core_pattern`
- Ensure disk space available in `/core/` directory

### Recovery Loop

- Check for corrupted hotboot data: `rm -f system/copyover.dat`
- Review crash patterns in multiple log files
- Consider increasing HOTBOOT_DELAY for slower systems

### Debug Mode

```bash
# Run startup script with debug output
set -x
./startup.sh

# Test individual components
bash -n startup.sh                    # Syntax check
python3 src/fallback_server.py --help # Fallback test
./core/analyze_core.sh --help         # Analysis test
```

## Security Considerations

- **Port Binding**: Both main and fallback servers bind to specified port
- **Resource Limits**: Fallback server limits concurrent connections
- **File Permissions**: Ensure proper permissions on scripts and data files
- **Email Alerts**: Configure proper SMTP for crash notifications
- **Log Rotation**: Implement rotation to prevent disk space issues

## Performance Impact

- **CPU Usage**: Minimal overhead during normal operation
- **Memory Usage**: Fallback server uses ~10-20MB RAM
- **Disk Usage**: Core dumps can be large; cleanup regularly
- **Network**: Fallback server handles basic telnet connections only
- **Recovery Time**: Hotboot typically takes 5-15 seconds, fallback 2-5 seconds

---

*This recovery system provides enterprise-grade crash recovery for the SWR MUD server while maintaining player experience during outages.*
