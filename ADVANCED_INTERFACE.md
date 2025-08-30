# Advanced Monitoring Interface Implementation

## Overview

The advanced monitoring interface (`mudctl-advanced.sh`) provides sophisticated real-time monitoring capabilities for the SWR MUD server with live log streaming, split-screen display, and error highlighting.

## Key Features

### 1. FIFO-Based Log Streaming

- Uses named pipes (FIFOs) for real-time log processing
- Non-blocking log data flow
- Automatic FIFO cleanup on exit
- Support for multiple log sources

### 2. Split-Screen Display

- Main server logs displayed in real-time
- Fallback server logs shown side-by-side
- Configurable screen layout (40% logs / 60% menu)
- Terminal resize detection and adaptation

### 3. Error Highlighting

- Critical errors highlighted with color coding
- Pattern-based error detection
- Grep-based color enhancement for better visibility
- Support for custom error patterns

### 4. Terminal Management

- Dynamic screen layout control
- Cursor positioning for optimal display
- Non-blocking input handling
- Graceful terminal restoration on exit

## Technical Implementation

### FIFO Pipe Architecture

```bash
# Create named pipes for log streaming
MAIN_FIFO="/tmp/mudctl_main_$$"
FALLBACK_FIFO="/tmp/mudctl_fallback_$$"

# Stream logs through FIFO with error highlighting
tail -n 100 -F "$LOGFILE" | grep --color=always -E "(ERROR|CRITICAL|FATAL|[Ee]rror|[Ff]atal)" &
```

### Screen Layout Management

```bash
# Calculate screen dimensions
TERM_HEIGHT=$(tput lines)
LOG_HEIGHT=$((TERM_HEIGHT * 40 / 100))
MENU_HEIGHT=$((TERM_HEIGHT - LOG_HEIGHT - 2))

# Position cursor for different sections
tput cup $LOG_HEIGHT 0  # Menu area
tput cup 0 0            # Log area
```

### Process Coordination

- Background processes for log streaming
- PID tracking for cleanup
- Signal handling for graceful shutdown
- Resource cleanup on exit

## Usage Instructions

### Starting the Advanced Interface

```bash
# From scripts directory
./mudctl.sh advanced

# Or directly
./mudctl-advanced.sh
```

### Interface Controls

- **q/Q** - Quit the interface
- **r** - Refresh display
- **s** - Show server status
- **1-9** - Execute server commands
- **Enter** - Refresh current view

### Display Modes

1. **Single Log Mode** - Main server logs only
2. **Split Mode** - Main + fallback logs side-by-side
3. **Status Mode** - Live server status monitoring

## Error Pattern Detection

The interface automatically highlights the following error patterns:

- ERROR, CRITICAL, FATAL (case insensitive)
- Exception traces
- Core dump notifications
- Memory allocation failures
- Network connection errors

## Performance Considerations

- FIFO pipes minimize memory usage
- Background processes handle log streaming
- Efficient terminal updates using cursor positioning
- Automatic cleanup prevents resource leaks

## Integration with Existing System

- Uses existing mudctl.sh infrastructure
- Compatible with current logging system
- Integrates with fallback server monitoring
- Maintains backwards compatibility

## Troubleshooting

### Common Issues

1. **FIFO Creation Errors** - Check /tmp permissions
2. **Terminal Display Issues** - Verify TERM environment variable
3. **Log File Access** - Ensure proper file permissions
4. **Process Cleanup** - Kill orphaned background processes

### Debug Mode

Set `DEBUG=1` environment variable for verbose output:

```bash
DEBUG=1 ./mudctl-advanced.sh
```

## Future Enhancements

- Configurable error patterns
- Log filtering capabilities
- Custom screen layouts
- Network monitoring integration
- Performance metrics display
