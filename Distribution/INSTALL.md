# Lower Thirds Plus - Installation Guide

## 📦 What's Included

This package contains the **Lower Thirds Plus** plugin for OBS Studio on macOS.

## ✅ Requirements

- **OBS Studio 28.0 or later** (built from source or official release)
- **macOS 10.15 (Catalina) or later**
- **Qt6** (if you built OBS from source)

## 🚀 Installation Steps

### Method 1: Automatic Installation (Recommended)

1. **Close OBS Studio** completely (quit the application)

2. **Double-click** the `INSTALL.command` file
   - This will automatically copy the plugin to the correct location
   - You may need to enter your password

3. **Launch OBS Studio**

### Method 2: Manual Installation

1. **Close OBS Studio** completely

2. **Copy the plugin folder:**
   ```bash
   cp -R LowerThirdsPlus.plugin ~/Library/Application\ Support/obs-studio/plugins/
   ```

3. **Launch OBS Studio**

## 🎯 How to Use

### Adding the Lower Third

1. In OBS, go to your **Sources** panel
2. Click **+ (Add Source)**
3. Select **"Lower Thirds Plus Source"**
4. Give it a name (e.g., "Lower Third")
5. Click **OK**

### Using the Plugin

1. **Right-click** the source → **Properties**
2. You'll see **5 Quick Play Tabs** at the top:
   - **[▶ Tab 1]**, **[▶ Tab 2]**, etc.
3. Fill in the text fields for each tab:
   - Tab 1 · Title
   - Tab 1 · Subtitle
   - Tab 1 · Title (Right) - optional
   - Tab 1 · Subtitle (Right) - optional
4. Click any **[▶ Play]** button to show that tab's content
5. Customize appearance in **⚙️ Advanced Settings**

## ⚙️ Features

- **5 Independent Tabs** - Pre-configure 5 different lower thirds
- **Instant Switching** - Click tab buttons to switch between profiles
- **Auto-Hide** - Automatically hides after set duration (default: 5 seconds)
- **13 Animation Styles** - Slide, fade, zoom, expand, push, wipe, instant
- **Customizable Design:**
  - Custom fonts (system font picker)
  - Colors (background, text, gradients)
  - Sizes (title, subtitle, bar height, padding)
  - Optional logo image (left side)
  - Optional background image
  - Toggle background on/off
- **Preview Mode** - See live preview while editing
- **Modern Animations** - Smooth, professional entrance effects
- **Responsive** - Optional auto-scale for different resolutions

## 🎨 Customization

Open **Properties** → **⚙️ Advanced Settings** to customize:

- **Animation Style** - 13 different entrance animations
- **Auto-Hide** - Automatic hide after duration
- **Font** - Choose any system font
- **Text Sizes** - Title (72px default), Subtitle (48px default)
- **Colors** - Background, text, gradient colors
- **Logo** - Add custom logo image (PNG, JPG, BMP)
  - Size, opacity, padding controls
- **Background** - Solid color, gradient, or custom image
- **Bar Height** - Adjust lower third height (200px default)
- **Padding** - Text spacing controls

## 🐛 Troubleshooting

### Plugin doesn't appear in OBS

1. Make sure OBS is completely closed before installation
2. Check the plugin is in the correct location:
   ```bash
   ls ~/Library/Application\ Support/obs-studio/plugins/
   ```
   You should see `LowerThirdsPlus.plugin`

3. Check OBS logs:
   - Help → Log Files → View Current Log
   - Look for any errors related to "LowerThirdsPlus"

### Text not showing

1. Make sure you filled in the text fields (Tab 1 · Title, etc.)
2. Check that text color isn't the same as background color
3. Make sure "Show Background" is enabled (or text color is visible)

### Animation too fast/slow

1. Open **Properties** → **⚙️ Advanced Settings**
2. Current animation duration is **1.0 second**
3. This is hardcoded but can be adjusted if needed

## 📝 Version Info

- **Version:** 1.0
- **Build Date:** February 2026
- **Plugin ID:** `lowerthirds_plus_source`

## 💡 Tips

- **Tab buttons show the title** - If you enter "John Smith" in Tab 1 Title, the button will show "[▶ John Smith]"
- **Preview mode** - Toggle "👁️ Preview" to see live updates while editing
- **Background toggle** - Turn off background to show only text and logo
- **Right-side text** - Optional fields for right-aligned content
- **Auto-scale OFF by default** - Text maintains exact pixel sizes when resizing

## 🙏 Support

If you encounter issues, check:
1. OBS version (28.0 or later)
2. macOS version (10.15 or later)
3. Plugin is in correct folder
4. OBS logs for error messages

Enjoy your professional lower thirds! 🎬
