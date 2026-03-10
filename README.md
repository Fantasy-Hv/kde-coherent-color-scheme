# kde-coherent-color-scheme
A script dynamically set kde color scheme  consistent with your wallpaper.
=======


## 🌟 Features

- **🎨 Dynamic Color Mapping** - Read 16 base colors from color files and apply transformations
- **🔧 Flexible Configuration** - INI-style configuration files with custom expression syntax
- **📝 Expression Syntax** - Use `$(index,value,saturation)` for dynamic color mapping
- **🚀 Daemon Mode** - Background monitoring with automatic wallpaper change detection
- **🔄 Pywal Integration** - Automatic color scheme generation from wallpapers
- **💼 KDE Plasma Support** - Integration with KDE color themes



## 🛠️ Requirements

### System Requirements
- **OS**: Linux (tested on KDE Plasma)
- **Compiler**: C++20 compatible (GCC 11+, Clang 13+)
- **Build System**: CMake >= 4.1

### Runtime Dependencies
- **pywal**: Color scheme generator (`sudo pacman -S pywal`)
- **plasma-workspace**: KDE Plasma components (`sudo pacman -S plasma-workspace`)

##  Usage

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
this project needs three types of config files.
### project config file

The project config file uses INI format with special directives:

```ini
/path/to/target1.colors
/path/to/target2.colors
[dycolor]
poll_interval_seconds=1800
```

**Configuration Options:**

- Line 1: Primary .colors file path (required)
- Line 2: Secondary .colors file path (required)
- `poll_interval_seconds`: Wallpaper check interval (default: 1800s = 30min)

### color mapping config file 
use this file to assign pywal colors to kde .colors fields.It's format is almost the same as that of .colors .you can use  color processing expression in syntax like "BackgroundNormal=$(index,value,saturation)",where
- **index**: Color index from pywal colors file (1-16)
  - 1 = Background color (darkest)
  - 7 = Foreground color (brightest)
  - 2-6,8-16 = Other colors
- **value**: Brightness/lightness percentage (0-100)
- **saturation**: Saturation percentage (0-100)

for more details see the shell/color-config.txt
### Target .colors File Format
it is merely the format kde color scheme used.
KDE color scheme files use this format:

```ini
[ColorEffects:Disabled]
Color=56,56,56
ColorAmount=0

[Colors:Button]
BackgroundAlternate=64,69,82
BackgroundNormal=33,33,33
DecorationFocus=0,114,255  
```
you  just need to copy two .colors files from ~/.local/share/color-scheme and modify the \[General] name filed and the file name, and then specify their paths in the project config file.


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


## 📝 Configuration Examples

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

### Adding New Features

1. Add new `.cpp` and `.h` files to `lib/`
2. Update `CMakeLists.txt`
3. Rebuild: `cmake --build .`

## 📄 License

This project is provided as-is for educational and personal use.

## 🤝 Contributing

Feel free to submit issues or pull requests for improvements.



**Made with ❤️ for KDE Plasma enthusiasts**

