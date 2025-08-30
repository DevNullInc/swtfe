# SWR Server Management - Updated Organization

## Summary of Changes

The SWR server management scripts have been reorganized for better portability and ease of use. This resolves issues with hardcoded paths that only worked in specific environments.

## What's New

### 📁 New `scripts/` Directory

- **Location**: `/workspaces/swtfe/scripts/`
- **Purpose**: Contains portable, environment-independent management scripts
- **Benefits**: Works from any directory, no hardcoded paths

### 🔧 Improved Scripts

#### `scripts/mudctl.sh` - Enhanced Server Controller

- **Portable**: Automatically detects paths relative to script location
- **New Features**:
  - `logs [count]` - View recent log entries
  - `tail` - Follow live logs
  - `cleanup` - Clean up old files
  - `check` - Run system dependency check
  - `interactive` - Launch interactive menu interface
- **Colored Output**: Easy-to-read status messages
- **Better Error Handling**: More informative error messages

#### `scripts/mudctl-interactive.sh` - Interactive Menu Interface

- **Real-time Monitoring**: Live server status and system information display
- **Menu-driven Interface**: Easy navigation with numbered/lettered commands
- **Auto-refresh Mode**: Toggleable continuous monitoring
- **Visual Indicators**: Color-coded status for quick assessment
- **Non-blocking**: Monitor server while keeping interface accessible#### `scripts/check_system.sh` - System Compatibility Checker  
- **Portable**: Works from any directory
- **Enhanced Analysis**:
  - GLIBC version compatibility checking
  - Detailed library dependency analysis
  - Colored output with clear status indicators
- **Better Diagnostics**: Specific recommendations for fixing issues

### 🚀 Migration Guide

#### Recommended Usage (New)

```bash
# From anywhere:
/path/to/swtfe/scripts/mudctl.sh start

# From swtfe directory:
./scripts/mudctl.sh start

# Check system compatibility:
./scripts/check_system.sh
```

#### Legacy Usage (Still Works)

```bash
# Old way (requires being in src directory):
cd src && ./mudctl.sh start
cd src && ./startup.sh
```

## Quick Start

### 1. Check System Compatibility

```bash
cd /workspaces/swtfe
./scripts/check_system.sh
```

### 2. Start the Server

```bash
# Start normally:
./scripts/mudctl.sh start

# Or with custom port:
./scripts/mudctl.sh start 4949
```

### 3. Monitor the Server

```bash
# Check status:
./scripts/mudctl.sh status

# View recent logs:
./scripts/mudctl.sh logs 50

# Follow live logs:
./scripts/mudctl.sh tail

# Launch interactive interface:
./scripts/mudctl.sh interactive
```

### 4. Stop the Server

```bash
# Graceful stop:
./scripts/mudctl.sh stop

# Force kill if needed:
./scripts/mudctl.sh kill-all
```

## Troubleshooting

### GLIBC Version Issues

If you see errors like "version `GLIBC_2.38' not found":

1. **Check compatibility**:

   ```bash
   ./scripts/check_system.sh
   ```

2. **The issue**: Server was compiled on a newer system than yours

3. **Solutions**:
   - Recompile the server on your current system
   - Upgrade your OS to a newer version
   - Use a pre-compiled binary for your OS version

### Port Conflicts

If the server won't start due to port conflicts:

1. **Check what's using the port**:

   ```bash
   ./scripts/mudctl.sh status
   ```

2. **Kill conflicting processes**:

   ```bash
   ./scripts/mudctl.sh kill-all
   ```

3. **Try a different port**:

   ```bash
   ./scripts/mudctl.sh start 4949
   ```

### Permission Issues

If you get "Permission denied" errors:

1. **Check executable permissions**:

   ```bash
   ls -la src/swr
   ```

2. **Fix permissions**:

   ```bash
   chmod +x src/swr
   chmod +x scripts/*.sh
   ```

## File Locations

### New Organization

```txt
swtfe/
├── scripts/              # New portable scripts (RECOMMENDED)
│   ├── mudctl.sh         # Main server controller
│   ├── check_system.sh   # System checker
│   └── README.md         # Documentation
├── src/                  # Server source
│   ├── startup.sh        # Core startup script (enhanced)
│   ├── fallback_server.py # Maintenance server
│   ├── mudctl.sh         # Legacy script (still works)
│   └── swr               # Server executable
├── core/                 # Crash dump management
└── log/                  # Server logs
```

### Key Benefits

- **No hardcoded paths**: Scripts work regardless of installation location
- **Better organization**: Management scripts separated from source code
- **Improved features**: More commands and better error handling
- **Backwards compatibility**: Old scripts still work for existing workflows

## Examples

### Daily Operations

```bash
# Morning check:
./scripts/mudctl.sh status

# View overnight activity:
./scripts/mudctl.sh logs 100

# Clean up old files:
./scripts/mudctl.sh cleanup

# Launch interactive dashboard:
./scripts/mudctl.sh interactive
```

### Maintenance

```bash
# Start fallback server during maintenance:
./scripts/mudctl.sh fallback

# (Players see maintenance message)
# Fix issues, then restart:
./scripts/mudctl.sh kill-all
./scripts/mudctl.sh start
```

### Emergency Recovery

```bash
# If server is stuck:
./scripts/mudctl.sh kill-all

# Check for issues:
./scripts/check_system.sh

# Restart:
./scripts/mudctl.sh start
```

## Help and Documentation

- **Script Help**: `./scripts/mudctl.sh help`
- **Scripts Documentation**: `./scripts/README.md`
- **Recovery System**: `./RECOVERY_SYSTEM.md`
- **Core Management**: `./core/README.md`

## Migration Timeline

### Phase 1 (Current)

- ✅ New portable scripts available in `scripts/` directory
- ✅ Enhanced features and better error handling
- ✅ Documentation updated

### Phase 2 (Recommended)

- 🔄 Update your procedures to use `./scripts/mudctl.sh`
- 🔄 Test the new scripts with your workflow
- 🔄 Update any automation to use new paths

### Phase 3 (Future)

- ⏳ Consider removing old `src/mudctl.sh` once comfortable with new version
- ⏳ Update any external scripts or documentation

The old scripts will continue to work, so you can migrate at your own pace.
