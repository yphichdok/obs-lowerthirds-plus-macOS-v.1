# Installing LowerThirdsPlus Plugin - Complete Guide

## ⚠️ IMPORTANT: You Cannot Copy Source Code Directly!

**❌ This will NOT work:**
```bash
# DON'T DO THIS - Won't work!
cp LowerThirdsPlus/src/*.cpp ~/Library/Application\ Support/obs-studio/plugins/
```

**Why?** OBS needs a compiled `.dylib` file, not `.cpp` source code.

---

## ✅ **Correct Installation Process**

### Overview

```
Source Code (.cpp)  →  Compile  →  Plugin (.dylib)  →  OBS loads it
```

### What You Need

1. ✅ **OBS Studio Installed** (you have this!)
2. ⚠️ **OBS Source Code** (need to download once for building)
3. ✅ **Build Tools** (CMake, Qt, etc. - you have these!)

---

## 🎯 **Complete Step-by-Step Installation**

### Step 1: Get OBS Source Code (One-Time Setup)

Even though you have OBS installed, you need the source code to compile plugins.

```bash
# This takes about 20-40 minutes total
cd ~/Development
git clone --recursive https://github.com/obsproject/obs-studio.git
cd obs-studio
git checkout 30.2.2
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_BROWSER=OFF -G Ninja
ninja
```

**Time**: 20-40 minutes (one time only)  
**Why**: Provides header files and libraries your plugin needs

---

### Step 2: Build the Plugin

```bash
# Navigate to plugin directory
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus"

# Use the automated build script
./build.sh ~/Development/obs-studio
```

**What happens:**
1. Compiles all `.cpp` files
2. Links with OBS libraries
3. Creates `LowerThirdsPlus.so` file
4. **Automatically installs to OBS plugin directory**

**Time**: 30-60 seconds

---

### Step 3: Verify Installation

The build script automatically installs the plugin to:

```bash
# Check if plugin was installed
ls -la ~/Library/Application\ Support/obs-studio/plugins/LowerThirdsPlus.plugin/

# You should see:
# LowerThirdsPlus.plugin/
#   └── Contents/
#       └── MacOS/
#           └── LowerThirdsPlus.so  ← This is the compiled plugin!
```

---

### Step 4: Launch OBS and Use Plugin

```bash
# Just open OBS normally
open -a "OBS"

# Or launch from Applications folder
```

**In OBS:**

1. Click `+` in Sources panel
2. Look for "**Lower Thirds Plus**" or "**LowerThirdsPlusSource**"
3. Add it to your scene
4. Go to **View → Docks → Lower Thirds Control**
5. Use the control panel!

---

## 🔍 **Understanding the Plugin Location**

### Where Plugins Live

OBS looks for plugins in these locations:

```
~/Library/Application Support/obs-studio/plugins/    ← User plugins (your plugin goes here!)
/Applications/OBS.app/Contents/PlugIns/              ← Built-in plugins
```

### What Gets Installed

```
~/Library/Application Support/obs-studio/plugins/
└── LowerThirdsPlus.plugin/           ← Plugin bundle
    ├── Contents/
    │   ├── Info.plist               ← Plugin metadata
    │   └── MacOS/
    │       └── LowerThirdsPlus.so   ← Compiled binary (the actual plugin!)
    └── data/
        └── locale/
            └── en-US.ini            ← Text strings
```

**The `.so` file** is what OBS actually loads. It's compiled from your `.cpp` files.

---

## 🚫 **What DOESN'T Work**

### ❌ Copying Source Files

```bash
# THIS DOESN'T WORK!
cp src/*.cpp ~/Library/Application\ Support/obs-studio/plugins/
```

**Why?** OBS can't run C++ source code directly. It needs compiled machine code.

### ❌ Copying Just the .so File

```bash
# THIS DOESN'T FULLY WORK!
cp build/LowerThirdsPlus.so ~/Library/...
```

**Why?** macOS needs the proper bundle structure with `Info.plist`, etc.

### ❌ Manual Bundle Creation

```bash
# DON'T DO THIS MANUALLY!
mkdir -p ~/Library/.../LowerThirdsPlus.plugin/Contents/MacOS
cp build/*.so ~/Library/.../
```

**Why?** The build script does this correctly. Manual copying can miss files.

---

## ✅ **The Right Way (Automated)**

```bash
# ONE command does everything:
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus"
./build.sh ~/Development/obs-studio
```

**This script:**
1. ✅ Checks dependencies
2. ✅ Configures build
3. ✅ Compiles source code
4. ✅ Creates proper bundle structure
5. ✅ Installs to correct location
6. ✅ Sets proper permissions

---

## 🔄 **Updating the Plugin**

After making changes to source code:

```bash
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus"

# Rebuild and reinstall
./build.sh ~/Development/obs-studio

# Then restart OBS
```

---

## 🐛 **Troubleshooting**

### Plugin Doesn't Appear in OBS

**Check 1:** Verify plugin was installed
```bash
ls ~/Library/Application\ Support/obs-studio/plugins/LowerThirdsPlus.plugin/Contents/MacOS/LowerThirdsPlus.so

# Should show the file
```

**Check 2:** Check OBS logs
```bash
# Open latest log
open ~/Library/Application\ Support/obs-studio/logs/

# Look for:
# "LowerThirdsPlus plugin loaded"  ← Good!
# Or error messages              ← Fix these
```

**Check 3:** Plugin permissions
```bash
# Make sure it's executable
chmod +x ~/Library/Application\ Support/obs-studio/plugins/LowerThirdsPlus.plugin/Contents/MacOS/LowerThirdsPlus.so
```

### "LowerThirdsPlus plugin loaded" but Source Doesn't Appear

The plugin loaded but source registration failed. Check logs for:
```
[error] Failed to register source...
```

Rebuild the plugin:
```bash
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus/build"
make clean
cd ..
./build.sh ~/Development/obs-studio
```

### Build Script Fails

**Error: "OBS_PATH not found"**
```bash
# Make sure you built OBS first
ls ~/Development/obs-studio/libobs/obs-module.h

# If not found, build OBS first (see BUILD_OBS_GUIDE.md)
```

**Error: "Qt6 not found"**
```bash
brew install qt
```

---

## 📊 **Installation Checklist**

- [ ] OBS Studio app installed (from obsproject.com)
- [ ] OBS source code downloaded and built
- [ ] Plugin compiled successfully
- [ ] Plugin installed to user plugins directory
- [ ] OBS launched (or restarted if already running)
- [ ] "Lower Thirds Plus" appears in sources list
- [ ] "Lower Thirds Control" appears in View → Docks menu

---

## 🎯 **Quick Reference**

### First Time Setup
```bash
# 1. Build OBS source (one time, 30 min)
cd ~/Development
git clone --recursive https://github.com/obsproject/obs-studio.git
cd obs-studio && git checkout 30.2.2
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
ninja

# 2. Build plugin (every time, 30 sec)
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus"
./build.sh ~/Development/obs-studio

# 3. Launch OBS
open -a OBS
```

### After Code Changes
```bash
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus"
./build.sh ~/Development/obs-studio
# Restart OBS
```

### Uninstall Plugin
```bash
rm -rf ~/Library/Application\ Support/obs-studio/plugins/LowerThirdsPlus.plugin
# Restart OBS
```

---

## 💡 **Key Concepts**

### Source Code vs. Compiled Binary

| Type | Format | Can OBS Use? |
|------|--------|--------------|
| **Source Code** | `.cpp`, `.hpp` files | ❌ No |
| **Compiled Plugin** | `.dylib`, `.so` file | ✅ Yes |

Think of it like:
- **Source code** = Recipe (human-readable)
- **Compiled binary** = Cooked meal (machine-readable)

OBS needs the "cooked meal" not the recipe!

### Why You Need OBS Source

```
Your Plugin Source Code
    ↓ includes
OBS Header Files (.h)    ← From OBS source
    ↓ links with
OBS Libraries (.dylib)   ← From OBS source
    ↓ talks to
OBS Application (.app)   ← Your installed OBS
```

You need OBS source to **build**, then the result works with your **installed OBS**.

---

## 🎬 **Visual Guide**

### What You're Doing

```
STEP 1: Get OBS Source
┌─────────────────────┐
│   GitHub            │
│   obsproject/obs    │
└──────────┬──────────┘
           │ git clone
           ↓
┌─────────────────────┐
│ ~/Development/      │
│   obs-studio/       │  ← Header files & libraries
└─────────────────────┘

STEP 2: Build Plugin
┌─────────────────────┐
│ LowerThirdsPlus/    │
│   src/*.cpp         │  ← Your source code
└──────────┬──────────┘
           │ cmake + make
           ↓
┌─────────────────────┐
│ build/              │
│   LowerThirdsPlus.so│  ← Compiled binary
└──────────┬──────────┘
           │ make install
           ↓
STEP 3: Auto-Install
┌─────────────────────┐
│ ~/Library/.../      │
│   plugins/          │
│     LowerThirdsPlus │  ← Installed plugin
│       .plugin/       │
└──────────┬──────────┘
           │ OBS finds it
           ↓
STEP 4: Use in OBS
┌─────────────────────┐
│ OBS Studio          │
│  ✅ Plugin Loaded   │  ← Your installed app
└─────────────────────┘
```

---

## ❓ **FAQ**

**Q: Can I skip building OBS from source?**  
A: No. You need the header files and libraries to compile plugins.

**Q: Can I use a pre-built OBS binary for plugin development?**  
A: No. Pre-built apps don't include development files.

**Q: Do I need to rebuild OBS every time?**  
A: No! Build OBS once, then only rebuild your plugin.

**Q: Can I develop plugins without building OBS?**  
A: Unfortunately no, not on macOS.

**Q: Where do I find OBS's built-in plugins?**  
A: `/Applications/OBS.app/Contents/PlugIns/`

**Q: Can I install plugins by dragging to Applications?**  
A: No. Plugins go in `~/Library/Application Support/obs-studio/plugins/`

**Q: Do I need to restart OBS after installing plugin?**  
A: Yes, or plugins won't load.

---

## 📚 **Additional Help**

- **BUILD_OBS_GUIDE.md** - How to build OBS from source
- **QUICKSTART.md** - Fast building guide
- **VERIFICATION.md** - Testing checklist
- **README.md** - Complete documentation

---

## ✅ **Success Checklist**

After following this guide, you should have:

- [x] OBS source code downloaded and built
- [x] LowerThirdsPlus plugin compiled
- [x] Plugin installed to user plugins directory
- [x] OBS recognizes and loads the plugin
- [x] "Lower Thirds Plus" source available
- [x] "Lower Thirds Control" dock panel available

**Congratulations!** Your plugin is installed and ready to use! 🎉

---

**Need Help?** Check the troubleshooting section or OBS logs for error messages.
