# Lower Thirds Plus - Complete User Guide

**Author:** Y Phic Hdok  
**Company:** MTD Technologies  
**Version:** 1.0  
**Date:** February 2026

---

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Quick Start](#quick-start)
4. [Features Overview](#features-overview)
5. [Step-by-Step Usage](#step-by-step-usage)
6. [Customization Guide](#customization-guide)
7. [Animation Styles](#animation-styles)
8. [Tips & Best Practices](#tips--best-practices)
9. [Troubleshooting](#troubleshooting)

---

## Introduction

**Lower Thirds Plus** is a professional lower thirds plugin for OBS Studio, designed for streamers, broadcasters, and content creators who want broadcast-quality graphics with minimal setup.

### Key Benefits
- **5 Pre-configured Profiles** - Switch between different lower thirds instantly
- **13 Modern Animations** - Professional entrance effects
- **Full Customization** - Colors, fonts, sizes, logo, backgrounds
- **Easy to Use** - Simple tabbed interface with one-click playback

---

## Installation

### Requirements
- macOS 10.15 (Catalina) or later
- OBS Studio 28.0 or later

### Installation Steps

**Method 1: Automatic (Recommended)**
1. Close OBS Studio completely
2. Double-click `INSTALL.command`
3. Follow the prompts
4. Launch OBS Studio

**Method 2: Manual**
1. Close OBS Studio
2. Copy `LowerThirdsPlus.plugin` to:
   ```
   ~/Library/Application Support/obs-studio/plugins/
   ```
3. Launch OBS Studio

### Verification
1. Open OBS Studio
2. Click **+ (Add Source)**
3. Look for **"Lower Thirds Plus Source"** in the list
4. If you see it, installation was successful! ✅

---

## Quick Start

### 1. Add the Source

1. In OBS, go to your **Sources** panel
2. Click **+ (Add Source)**
3. Select **"Lower Thirds Plus Source"**
4. Name it (e.g., "Lower Third", "Guest Names", etc.)
5. Click **OK**

### 2. Configure Your First Lower Third

1. **Right-click** the source → **Properties**
2. Fill in the text fields:
   - **Tab 1 · Title:** Enter the main name (e.g., "John Smith")
   - **Tab 1 · Subtitle:** Enter the subtitle (e.g., "CEO")
3. Click **[▶ John Smith]** button at the top
4. The lower third will appear with a smooth animation! 🎉

### 3. Add More Profiles

1. Scroll down to see **Tab 2, Tab 3, Tab 4, Tab 5** fields
2. Fill in different content for each tab:
   - Tab 2: "Jane Doe" / "CTO"
   - Tab 3: "Bob Lee" / "Designer"
   - etc.
3. Click any **[▶ Play]** button to switch between them instantly

---

## Features Overview

### 5 Independent Tabs
- Pre-configure 5 different lower thirds
- Each tab has its own Title, Subtitle, and optional right-side text
- Instant switching with one click
- Tab buttons show the title for easy identification

### 13 Animation Styles
- ↖ Slide from Left
- ↗ Slide from Right
- ↑ Slide from Bottom
- ↓ Slide from Top
- ⊙ Fade In
- ⊕ Zoom In
- ⇤ Expand from Left
- ⇥ Expand from Right
- ⟵ Push from Left
- ⟶ Push from Right
- ⇐ Wipe from Left
- ⇒ Wipe from Right
- ⚡ Instant

### Customization Options
- **Fonts:** Choose any system font
- **Colors:** Background, text, gradient colors
- **Sizes:** Title, subtitle, bar height, padding
- **Logo:** Add custom logo image (PNG, JPG, BMP)
- **Background:** Solid color, gradient, or custom image
- **Auto-Hide:** Automatically hides after set duration
- **Preview Mode:** See live changes while editing

---

## Step-by-Step Usage

### Basic Workflow

#### Step 1: Set Up Your Profiles

1. **Open Properties**
   - Right-click the source → Properties

2. **Fill in Tab 1 (Required)**
   - **Tab 1 · Title:** Main text (e.g., guest name)
   - **Tab 1 · Subtitle:** Secondary text (e.g., job title)
   - **Optional:** Fill in right-side text fields if needed

3. **Fill in Additional Tabs (Optional)**
   - Repeat for Tab 2, Tab 3, Tab 4, Tab 5
   - Each tab is independent
   - Leave unused tabs empty

#### Step 2: Play Lower Thirds

1. **Show a Lower Third**
   - Click any **[▶ Play]** button at the top
   - The lower third will animate in smoothly

2. **Switch Between Tabs**
   - Click a different **[▶ Play]** button
   - It will instantly switch to that tab's content

3. **Auto-Hide**
   - By default, it will hide after 5 seconds
   - You can replay by clicking the button again

#### Step 3: Customize Appearance

1. **Open Advanced Settings**
   - Scroll down to **⚙️ Advanced Settings**
   - Click to expand the collapsible section

2. **Choose Animation Style**
   - Select from 13 different animation styles
   - Try different ones to see which fits your style

3. **Adjust Colors & Sizes**
   - Background Color: Click color picker
   - Text Color: Choose contrasting color
   - Sizes: Adjust title, subtitle, bar height

---

## Customization Guide

### Text Styling

#### Fonts
1. Open **⚙️ Advanced Settings**
2. Click **Font** → Choose any system font
3. Common choices:
   - **Helvetica Neue** - Clean, modern (default)
   - **Arial** - Classic, readable
   - **Futura** - Bold, geometric
   - **Avenir** - Professional, elegant

#### Text Sizes
- **Title Size:** 72px (default) - Main name/text
- **Subtitle Size:** 48px (default) - Secondary text
- Adjust based on your design needs

#### Text Colors
- **Text Color:** White (#FFFFFF) by default
- Click color picker to change
- Ensure good contrast with background

### Background Styling

#### Solid Color
1. **Background Color:** Light blue (#F5A542) by default
2. Click color picker to change
3. **Opacity:** 100% by default (fully opaque)

#### Gradient Background
1. Select **Gradient Style:**
   - **Horizontal** → Left to right gradient
   - **Vertical** ↓ - Top to bottom gradient
   - **Diagonal** ↘ - Top-left to bottom-right
   - **Diagonal** ↗ - Bottom-left to top-right
2. Set **Gradient End Color**
3. Adjust both colors to create your gradient

#### Custom Background Image
1. Click **Background Image (Optional)**
2. Browse and select an image file (PNG, JPG, BMP)
3. Image will stretch to fill the bar

#### Hide Background
- Toggle **Show Background** OFF
- Shows only text and logo (transparent background)
- Useful for custom overlays

### Logo Customization

#### Adding a Logo
1. Scroll to **Logo** section in Advanced Settings
2. Click **Logo Image**
3. Browse and select your logo file (PNG recommended)
4. Logo appears on the left side

#### Logo Settings
- **Logo Size:** 140px (default) - Adjust for your design
- **Logo Opacity:** 100% (default) - Make semi-transparent if needed
- **Logo Padding (Horizontal):** 20px - Space from left edge
- **Logo Padding (Vertical):** 0 (auto-centers) - Or set custom value

### Layout & Positioning

#### Bar Height
- **Bar Height:** 200px (default)
- Range: 80px to 300px
- Taller bars accommodate larger text

#### Text Padding
- **Text Padding (Left/Right):** 60px (default)
- Space between text and edges
- **Text Padding (Top/Bottom):** 25px (default)
- Vertical spacing

#### Auto-Scale
- **OFF by default** - Recommended for stability
- When OFF: Text maintains exact pixel sizes
- When ON: Text scales with canvas resolution

---

## Animation Styles

### Slide Animations
**Best for:** Clean, professional look

- **↖ Slide from Left** - Classic entrance from left edge
- **↗ Slide from Right** - Entrance from right edge
- **↑ Slide from Bottom** - Upward reveal
- **↓ Slide from Top** - Downward reveal

### Modern Effects
**Best for:** Attention-grabbing entrances

- **⊙ Fade In** - Smooth fade with subtle scale
- **⊕ Zoom In** - Dramatic zoom from center

### Dynamic Expands
**Best for:** Modern, tech-focused streams

- **⇤ Expand from Left** - Horizontal stretch from left
- **⇥ Expand from Right** - Horizontal stretch from right

### Push & Wipe
**Best for:** Smooth, sophisticated transitions

- **⟵ Push from Left** - Slide with scale combo
- **⟶ Push from Right** - Slide with scale combo
- **⇐ Wipe from Left** - Reveal effect from left
- **⇒ Wipe from Right** - Reveal effect from right

### Instant
**Best for:** No animation, immediate display

- **⚡ Instant** - No animation (appears instantly)

### Animation Timing
- **Duration:** 1.0 second (smooth and modern)
- **Fade Out:** 0.67 seconds (slightly faster)
- All animations use professional easing curves

---

## Tips & Best Practices

### Design Tips

1. **Keep Text Readable**
   - Use high contrast (white text on dark background)
   - Avoid very thin fonts
   - Test visibility on different backgrounds

2. **Logo Best Practices**
   - Use PNG with transparency
   - Square aspect ratio works best (1:1)
   - Keep it simple and recognizable

3. **Color Schemes**
   - Match your brand colors
   - Use 2-3 colors maximum
   - Test on both light and dark scenes

4. **Text Length**
   - Keep titles under 25 characters
   - Subtitles under 40 characters
   - Longer text may get cut off

### Workflow Tips

1. **Pre-configure All Tabs**
   - Set up all 5 tabs before your stream
   - Test each one to ensure they look good
   - Write down which tab is for which guest

2. **Use Preview Mode**
   - Toggle **👁️ Preview** to see live changes
   - Adjust settings without disrupting stream
   - Turn OFF before going live

3. **Hotkeys (Optional)**
   - Set up OBS hotkeys to show/hide the source
   - Combine with tab buttons for quick control

4. **Multiple Scenes**
   - Add the same source to multiple scenes
   - Settings persist across scenes
   - Great for different layouts

### Performance Tips

1. **Background Images**
   - Use compressed images (< 1MB)
   - Optimize resolution (1920x200px is enough)
   - PNG for transparency, JPG for photos

2. **Logo Images**
   - Keep logo file size small (< 500KB)
   - Use appropriate resolution (500x500px max)
   - PNG with transparency works best

---

## Troubleshooting

### Plugin Doesn't Appear in OBS

**Problem:** Can't find "Lower Thirds Plus Source" in add source menu

**Solutions:**
1. Make sure OBS was closed during installation
2. Verify plugin location:
   ```
   ~/Library/Application Support/obs-studio/plugins/LowerThirdsPlus.plugin
   ```
3. Check OBS logs: Help → Log Files → View Current Log
4. Look for errors related to "LowerThirdsPlus"

### Text Not Showing

**Problem:** Lower third appears but no text is visible

**Solutions:**
1. Check text fields are filled in (Tab 1 · Title, etc.)
2. Verify text color isn't same as background
3. Make sure "Show Background" is ON (or text color is visible)
4. Check if text is moving off-screen (reduce padding)

### Animation Not Working

**Problem:** Lower third appears instantly without animation

**Solutions:**
1. Check Animation Style isn't set to "⚡ Instant"
2. Try a different animation style
3. Make sure to click the **[▶ Play]** button (not eye icon)

### Tab 1 Appears Before Switching

**Problem:** See Tab 1 content briefly before showing other tabs

**Solutions:**
1. Update to latest version (v1.0)
2. This was a bug that has been fixed
3. Make sure you installed the latest distribution

### Logo Not Appearing

**Problem:** Logo image not showing up

**Solutions:**
1. Check file format (PNG, JPG, BMP only)
2. Verify file isn't corrupted (open in image viewer)
3. Check Logo Opacity isn't set to 0%
4. Try increasing Logo Size if it's too small

### Colors Look Wrong

**Problem:** Background color doesn't match what I picked

**Solutions:**
1. Use the latest version (v1.0) - color bug was fixed
2. OBS uses ABGR color format (now handled correctly)
3. Try picking color again after updating

### Performance Issues

**Problem:** OBS lags when showing lower third

**Solutions:**
1. Reduce background image file size
2. Use simpler animation styles (Fade, Instant)
3. Reduce bar height if very large
4. Check other sources aren't causing lag

---

## Advanced Usage

### Right-Side Text Fields

Each tab has optional right-side text fields:
- **Title (Right):** Appears on right side, same size as title
- **Subtitle (Right):** Appears below right title

**Use Cases:**
- **Two-person interviews:** Left person | Right person
- **Versus displays:** Team A vs Team B
- **Dual information:** Name | Location
- **Score displays:** Player | Score

### Preview Mode Details

**👁️ Preview (Show Live While Editing)**
- Toggle ON to see changes in real-time
- Lower third stays visible while you adjust settings
- Perfect for fine-tuning design
- **Important:** Turn OFF before going live to avoid accidental visibility

### Auto-Hide Configuration

**Auto-Hide After Duration**
- Toggle ON/OFF
- **Duration:** 1.0 to 30.0 seconds (default: 5.0)
- Lower third automatically hides after duration
- Click **[▶ Play]** again to replay

**Use Cases:**
- **ON:** Guest introductions (show once, hide automatically)
- **OFF:** Persistent displays (stays until manually hidden)

---

## Keyboard Shortcuts & Workflow Integration

### Recommended OBS Hotkey Setup

1. Go to OBS Settings → Hotkeys
2. Find "Lower Thirds Plus Source"
3. Set hotkeys for:
   - **Show Source:** Show the lower third
   - **Hide Source:** Hide the lower third

**Pro Tip:** Use hotkeys + tab buttons for fastest workflow

---

## Credits & Support

**Lower Thirds Plus v1.0**

**Created by:**  
Y Phic Hdok  
MTD Technologies

**Built with:**
- C++ and OBS Studio API
- Qt6 for text rendering
- Modern animation techniques
- Professional easing functions

**Special Thanks:**
- OBS Studio development team
- OBS plugin community
- Beta testers and users

---

## Version History

### Version 1.0 (February 2026)
- Initial release
- 5 tabbed profiles
- 13 animation styles
- Full customization options
- Logo support
- Background images
- Gradient backgrounds
- Modern smooth animations
- Fixed profile switching
- Color picker fix
- Text stability improvements

---

## Contact & Feedback

For questions, feedback, or support:
- Created by Y Phic Hdok
- MTD Technologies

Thank you for using Lower Thirds Plus! 🎬
