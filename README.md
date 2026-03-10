# Dynamic Color Config Loader

A powerful C++ tool for dynamic color theme management in KDE Plasma environments. Automatically generates and applies color schemes based on wallpaper colors using pywal integration.

## 🌟 Features

- **🎨 Dynamic Color Mapping** - Read 16 base colors from color files and apply transformations
- **🔧 Flexible Configuration** - INI-style configuration files with custom expression syntax
- **🌈 HSL Color Manipulation** - Adjust brightness (Value) and saturation programmatically
- **📝 Expression Syntax** - Use `$(index,value,saturation)` for dynamic color generation
- **🚀 Daemon Mode** - Background monitoring with automatic wallpaper change detection
- **🔄 Pywal Integration** - Automatic color scheme generation from wallpapers
- **💼 KDE Plasma Support** - Seamless integration with KDE color themes

## 📁 Project Structure

```
dy-config-loader/
├── lib/                          # C++ Core Library
│   ├── main.cpp                  # Application entry point
│   ├── ConfigLoader.h            # Configuration parser and writer
│   ├── ColorReader.h             # Color file reader and HSL converter
│   └── Logger.h                  # Logging utilities
├── shell/                        # Shell Scripts
│   ├── dycolor-theme-manager.sh  # Main theme manager script
│   ├── start-daemon.sh           # Start daemon process
│   ├── stop-daemon.sh            # Stop daemon process
│   └── color-config.txt          # Color mapping configuration
├── test/                         # Test Files
│   ├── *.colors                  # Color theme files
│   └── testupdator.sh            # Test scripts
└── CMakeLists.txt                # Build configuration
```

## 🛠️ Requirements

### System Requirements
- **OS**: Linux (tested on KDE Plasma)
- **Compiler**: C++20 compatible (GCC 11+, Clang 13+)
- **Build System**: CMake >= 4.1

### Runtime Dependencies
- **pywal**: Color scheme generator (`sudo pacman -S pywal`)
- **plasma-workspace**: KDE Plasma components (`sudo pacman -S plasma-workspace`)
- **qdbus6**: D-Bus communication tool (optional, for advanced features)

## 📦 Installation & Build

### Clone and Build

```bash
# Navigate to project directory
cd /home/yian/CLionProjects/dy-config-loader

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .
```

After successful build, the executable `color-config-injector` will be available in the build directory.

### Install Location

The built executable and shell scripts should be placed in a common directory:

```bash
# Example installation
cp build/color-config-injector ~/bin/
cp shell/*.sh ~/bin/
chmod +x ~/bin/*.sh
```

## 🚀 Usage

### Standalone Mode

Run the config injector directly:

```bash
./color-config-injector <source_config> <target_config>
```

**Parameters:**
- `<source_config>`: Path to source configuration file (with color mapping rules)
- `<target_config>`: Path to target KDE color scheme file (.colors)

**Example:**
```bash
./color-config-injector ./shell/color-config.txt ~/.local/share/color-themes/theme.colors
```

### Daemon Mode (Recommended)

Use the theme manager for automatic wallpaper-based theme switching:

```bash
# Start the daemon (background monitoring)
./shell/start-daemon.sh

# Or use the full theme manager
./shell/dycolor-theme-manager.sh --start

# Stop the daemon
./shell/stop-daemon.sh

# Run once immediately
./shell/dycolor-theme-manager.sh --once

# Check status
./shell/dycolor-theme-manager.sh --status
```

## 📖 Configuration Guide

### Source Configuration File Format

The source config file uses INI format with special directives:

```ini
# Line 1: Path to pywal colors file (must start with /)
/home/yian/.cache/wal/colors

# Color mapping rules
[Colors:Button]
BackgroundNormal=$(13,12,65)    # Color index 13, 12% brightness, 65% saturation
ForegroundNormal=$(7,90,10)     # Color index 7, 90% brightness, 10% saturation

[Colors:View]
BackgroundNormal=$(1,11,17)     # Dark background from pywal color 1
```

### Expression Syntax

```
$(index, value, saturation)
```

- **index**: Color index from pywal colors file (1-16)
  - 1 = Background color (darkest)
  - 7 = Foreground color (brightest)
  - 2-6,8-16 = Other colors
- **value**: Brightness/lightness percentage (0-100)
- **saturation**: Saturation percentage (0-100)

### Target .colors File Format

KDE color scheme files use this format:

```ini
[ColorEffects:Disabled]
Color=56,56,56
ColorAmount=0

[Colors:Button]
BackgroundAlternate=64,69,82
BackgroundNormal=RGB_VALUE
DecorationFocus=0,114,255
```

RGB values are comma-separated: `R,G,B`

## 🔧 Advanced Configuration

### Theme Manager Config

Create `dycolor-config.ini`:

```ini
/path/to/target1.colors
/path/to/target2.colors
[dycolor]
poll_interval_seconds=1800
```

**Configuration Options:**
- Line 1: Primary .colors file path
- Line 2: Secondary .colors file path (for dual-theme support)
- `poll_interval_seconds`: Wallpaper check interval (default: 1800s = 30min)

### Dual Theme Support

The theme manager supports two .colors files and alternates between them:
- Tag 1: First .colors file
- Tag 2: Second .colors file

This prevents conflicts when multiple applications read the same color file.

## 💻 How It Works

### Workflow

1. **Monitor**: Watches for wallpaper changes (D-Bus events + polling)
2. **Generate**: Runs `pywal` to extract colors from current wallpaper
3. **Process**: Reads source config and evaluates color expressions
4. **Inject**: Writes computed RGB values to target .colors file
5. **Apply**: Uses `plasma-apply-colorscheme` to activate the theme

### Architecture

```
┌─────────────────┐
│ Wallpaper Change│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Pywal (wal)   │ ← Generates 16 colors
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ color-config.txt│ ← Mapping rules
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│color-config-    │ ← C++ processor
│injector         │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  .colors file   │ ← Updated RGB values
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│plasma-apply-    │ ← Apply to KDE
│colorscheme      │
└─────────────────┘
```

## 🧪 Testing

Test files are provided in the `test/` directory:

```bash
# Run test script
cd test
./testupdator.sh

# Manual testing
./color-config-injector test/color-config.txt test/dynamic_color.colors
```

## 📝 Examples

### Example 1: Simple Color Mapping

Source config (`myconfig.txt`):
```ini
/home/yian/.cache/wal/colors

[Colors:Window]
BackgroundNormal=$(0,50,50)
ForegroundNormal=$(15,90,10)
```

### Example 2: Complete Theme

See `shell/color-config.txt` for a complete example with all KDE color sections.

### Example 3: Custom Polling Interval

In `dycolor-config.ini`:
```ini
/home/user/themes/mytheme.colors
[dycolor]
poll_interval_seconds=900
```

This checks for wallpaper changes every 15 minutes instead of the default 30.

## 🐛 Troubleshooting

### Common Issues

**Issue**: "Cannot find color-config-injector"
- **Solution**: Ensure the executable is built and in PATH or specify full path

**Issue**: "pywal not found"
- **Solution**: Install pywal: `sudo pacman -S pywal`

**Issue**: Colors not updating in applications
- **Solution**: Restart affected applications or send D-Bus config reload signal

**Issue**: Daemon not detecting wallpaper changes
- **Solution**: Check if qdbus6 is installed and D-Bus is running

### Log Files

Check logs for debugging:
```bash
# View theme manager logs
tail -f ./shell/dycolor.log

# Check daemon status
./shell/dycolor-theme-manager.sh --status
```

## 🎯 Development

### Building from Source

```bash
cd /home/yian/CLionProjects/dy-config-loader
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Code Structure

- **ConfigLoader**: Parses INI files and processes color expressions
- **ColorReader**: Reads color files and performs HSL conversions
- **Logger**: Utility for debug output

### Adding New Features

1. Add new `.cpp` and `.h` files to `lib/`
2. Update `CMakeLists.txt`
3. Rebuild: `cmake --build .`

## 📄 License

This project is provided as-is for educational and personal use.

## 🤝 Contributing

Feel free to submit issues or pull requests for improvements.

## 📞 Support

For questions or problems:
1. Check the troubleshooting section
2. Review log files
3. Open an issue on the project repository

---

**Made with ❤️ for KDE Plasma enthusiasts**
