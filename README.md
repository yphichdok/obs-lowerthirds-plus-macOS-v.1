# Lower Thirds Plus - OBS Studio Plugin for macOS

Professional lower thirds plugin with 5 tabbed profiles, 16 animation styles, and extensive customization.

**Author:** Y Phic Hdok  
**Company:** MTD Technologies  
**Version:** 1.0  
**Release Date:** February 2026

---

## 🎯 Features

- **5 Independent Tabs** - Pre-configure different lower thirds, switch with one click
- **16 Animation Styles** - Slide, fade, zoom, expand, push, wipe, spin, scroll, roll, instant
- **Independent Animations** - Separate animation controls for background, logo, and text
- **Modern Animations** - Smooth 1.4-second animations with professional easing
- **Full Customization** - Fonts, colors, sizes, logo, backgrounds
- **Shadow Effects** - Customizable shadows for text and logo with color, opacity, and offset controls
- **Auto-Hide** - Automatically hides after set duration
- **Preview Mode** - See live preview while editing
- **Logo Support** - Optional left-side logo with size and opacity controls
- **Gradient Backgrounds** - 4 gradient directions or solid colors
- **Background Images** - Custom PNG/JPG backgrounds
- **Right-Side Text** - Optional right-aligned text fields
- **Stable Positioning** - Text and logo maintain exact positions when resizing

---

## 💿 Easy Installation (DMG)

**For end users:** Download the pre-built DMG installer from the **Distribution** folder or GitHub releases.

1. Download `LowerThirdsPlus-v1.0-macOS.dmg`
2. Double-click to mount the disk image
3. Double-click `Install Lower Thirds Plus.command`
4. Follow the on-screen instructions
5. Launch OBS Studio

The DMG includes:
- Pre-built plugin bundle
- Automatic installer script
- Complete documentation (User Guide, Quick Reference)
- No compilation required!

---

## 📦 Requirements

### Runtime Requirements
- macOS 10.15 (Catalina) or later
- OBS Studio 28.0 or later

### Build Requirements
- Xcode Command Line Tools
- CMake 3.16 or later
- OBS Studio source code
- C++17 compiler

---

## 🚀 Quick Build

```bash
# Build and install
./build.sh ~/Development/obs-studio

# The plugin will be installed to:
# ~/Library/Application Support/obs-studio/plugins/LowerThirdsPlus.plugin
```

---

## 📖 Usage

### 1. Add the Source
1. In OBS: **+ (Add Source)**
2. Select **"Lower Thirds Plus Source"**
3. Click **OK**

### 2. Configure Profiles
1. Right-click source → **Properties**
2. Fill in text for each tab (Tab 1-5)
3. Click **[▶ Play]** buttons to show lower thirds

### 3. Customize (Optional)
- Open **⚙️ Advanced Settings**
- Choose animation style
- Adjust colors, fonts, sizes
- Add logo and background images

**See `USER_GUIDE.md` for complete documentation**

---

## 🎨 Customization Options

### Text & Typography
- System font selector
- Title size: 20-120px (default: 72px)
- Subtitle size: 16-100px (default: 48px)
- Text color picker
- Bold toggle
- **Shadow effects** with color, opacity, and offset controls

### Background
- Solid color or gradient
- 4 gradient directions (horizontal, vertical, diagonal)
- Custom background image (PNG, JPG, BMP)
- Opacity control (0-100%)
- Toggle background on/off

### Logo
- Custom logo image (PNG, JPG, BMP)
- Size control: 140px default
- Opacity: 0-100%
- Padding controls (horizontal, vertical)
- Auto-center vertically
- **Shadow effects** with color, opacity, and offset controls

### Layout
- Bar height: 80-300px (default: 200px)
- Text padding: horizontal and vertical
- Auto-scale option (OFF by default for stability)

### Animation
- 16 different animation styles (slide, fade, zoom, expand, push, wipe, spin, scroll, roll, instant)
- Independent animation selection for background, logo, and text
- 1.4 second smooth duration
- Professional easing curves (ease-out-expo, ease-in-out-quint)
- Auto-hide after duration (1-30 seconds)

---

## 📁 Project Structure

```
LowerThirdsPlus/
├── src/
│   ├── plugin-main-simple.cpp          # Plugin entry point
│   ├── lowerthirds-source-simple.cpp   # Source implementation
│   ├── lowerthirds-source-simple.hpp
│   ├── json-loader.cpp                 # Utilities
│   └── json-loader.hpp
├── data/locale/
│   └── en-US.ini                       # Localization
├── obs-headers/                        # OBS API headers
├── CMakeLists.txt                      # Build configuration
├── Info.plist.in                       # macOS bundle info
├── build.sh                            # Build script
└── README.md                           # This file
```

---

## 🔧 Building from Source

### Prerequisites

1. **Install Xcode Command Line Tools:**
   ```bash
   xcode-select --install
   ```

2. **Clone OBS Studio:**
   ```bash
   cd ~/Development
   git clone --recursive https://github.com/obsproject/obs-studio.git
   cd obs-studio
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j8
   ```

3. **Build the Plugin:**
   ```bash
   cd /path/to/LowerThirdsPlus
   ./build.sh ~/Development/obs-studio
   ```

### Build Script Usage

```bash
./build.sh <OBS_SOURCE_PATH>

# Example:
./build.sh ~/Development/obs-studio
```

The plugin will be:
1. Compiled with CMake
2. Automatically installed to OBS plugins folder
3. Ready to use after restarting OBS

---

## 🎬 Animation Styles

Each animation has distinct characteristics for professional variety:

1. **↖ Slide from Left** - Pure horizontal slide from left, no scaling
2. **↗ Slide from Right** - Pure horizontal slide from right, no scaling
3. **↑ Slide from Bottom** - Pure vertical slide upward, no scaling
4. **↓ Slide from Top** - Pure vertical slide downward, no scaling
5. **⊙ Fade In** - Minimal scale fade (98%-100%), opacity-driven
6. **⊕ Zoom In** - Dramatic zoom from 30%-100%, centered
7. **⇤ Expand from Left** - Horizontal stretch anchored to left edge
8. **⇥ Expand from Right** - Horizontal stretch anchored to right edge
9. **⟵ Push from Left** - Slide + perspective squeeze with 3D depth effect
10. **⟶ Push from Right** - Slide + perspective squeeze with 3D depth effect
11. **⇐ Wipe from Left** - Diagonal wipe with sweep motion
12. **⇒ Wipe from Right** - Diagonal wipe with sweep motion
13. **↻ Spin** - 720° logo rotation / 45° background rotation with zoom
14. **⇢ Scroll** - Extended smooth scroll (120% travel), no scaling
15. **⟲ Roll** - 360° tumble with sine wave bobbing effect
16. **⚡ Instant** - No animation, immediate appearance

**Independent Control:** Set different animations for background, logo, and text separately for creative combinations!

All animations use professional easing curves (ease-out-expo, ease-in-out-quint) for broadcast-quality results.

---

## 🐛 Troubleshooting

### Build Issues

**CMake can't find OBS:**
```bash
# Make sure OBS_PATH is correct
./build.sh ~/Development/obs-studio
```

**Missing dependencies:**
```bash
# Install CMake and other tools
brew install cmake
```

### Runtime Issues

**Plugin doesn't load:**
- Check OBS logs: `~/Library/Application Support/obs-studio/logs/`
- Verify installation: `~/Library/Application Support/obs-studio/plugins/`
- Make sure OBS version is 28.0 or later

**Text not visible:**
- Check text fields are filled in
- Verify text color contrasts with background
- Ensure "Show Background" is ON

---

## 📝 Version History

### Version 1.0 (February 2026)
- 5 tabbed profiles for quick switching
- 16 professional animation styles (slide, fade, zoom, expand, push, wipe, spin, scroll, roll, instant)
- Independent animation controls for background, logo, and text
- Modern smooth animations (1.4s duration) with staggered element timing
- Full text customization with system fonts
- **Shadow effects for text and logo** with customizable color, opacity, and offset
- Logo support (left side, optional) with independent animation and spin effect
- Background images and gradients
- Right-side text fields (optional)
- Preview mode for live editing
- Auto-hide functionality
- Fixed profile switching behavior
- Color picker accuracy fix (ABGR format)
- Stable text/logo positioning on resize
- Enhanced text transformations with unique scale effects per style
- Professional easing functions (ease-out-expo, ease-in-out-quint)
- Distinct visual characteristics for every animation style

---

## 👨‍💻 Development

### Technical Details

**Built with:**
- C++17
- OBS Studio Plugin API
- OBS Graphics Subsystem (libobs-graphics)
- CMake build system

**Key Technologies:**
- Direct graphics rendering with `gs_*` functions
- Matrix transformations for animations
- Image file support with `gs_image_file_t`
- Text rendering via OBS text sources
- Advanced easing functions for smooth animations

**API Usage:**
- `obs_source_*` - Source management
- `gs_*` - Graphics rendering
- `obs_properties_*` - UI properties
- `obs_data_*` - Settings management

---

## 📄 License

This plugin is provided for educational and commercial use.

**Created by:**  
Y Phic Hdok  
MTD Technologies

Copyright © 2026 MTD Technologies. All rights reserved.

---

## 🙏 Acknowledgments

- OBS Studio development team
- OBS plugin community
- Qt Framework team

---

## 📞 Support

**Created by Y Phic Hdok, MTD Technologies**

For detailed usage instructions, see the included `USER_GUIDE.md`.

---

**Enjoy creating professional lower thirds for your streams!** 🎬✨
