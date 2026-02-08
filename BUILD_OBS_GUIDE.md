# Building OBS Studio from Source - Complete Guide for macOS

## What You're Doing

You're downloading and compiling the OBS Studio source code to get:
- **Header files** (`.h`) - Tells your plugin how to use OBS functions
- **Compiled libraries** (`.dylib`) - The actual OBS code your plugin links to
- **Build system** - CMake configuration for macOS

**Important**: You're NOT replacing your installed OBS. This is just for building plugins.

---

## Prerequisites (Already Have These ✅)

- [x] Xcode Command Line Tools
- [x] Homebrew
- [x] CMake
- [x] Qt6

---

## Step-by-Step Instructions

### Step 1: Create Development Directory

```bash
# Create a place to store the OBS source code
mkdir -p ~/Development
cd ~/Development
```

**What this does**: Creates a folder to keep your development files organized.

---

### Step 2: Clone OBS Studio Repository

```bash
# Download OBS Studio source code from GitHub
git clone --recursive https://github.com/obsproject/obs-studio.git

# This will take 2-5 minutes depending on your internet speed
```

**What this does**: Downloads all the OBS source code (~500MB) from GitHub.

**The `--recursive` flag** also downloads sub-dependencies that OBS needs.

**Expected output**:
```
Cloning into 'obs-studio'...
remote: Enumerating objects: 123456, done.
remote: Counting objects: 100% (5678/5678), done.
...
Submodule path 'deps/...': checked out '...'
```

---

### Step 3: Enter OBS Directory

```bash
cd obs-studio
```

**What this does**: Moves you into the downloaded source code folder.

---

### Step 4: Check Out Stable Version (Recommended)

```bash
# See available versions
git tag | tail -20

# Use the latest stable version (e.g., 30.2.2)
git checkout 30.2.2

# Update submodules for this version
git submodule update --recursive
```

**What this does**: Switches to a stable release instead of development code.

**Why?** The `master` branch might have unstable changes. A tagged release is tested and stable.

---

### Step 5: Install OBS Dependencies

OBS needs several libraries. Install them via Homebrew:

```bash
# Core dependencies
brew install \
    mbedtls \
    qt \
    cmake \
    ninja \
    pkg-config \
    ffmpeg \
    librist \
    jansson \
    speexdsp \
    swig \
    python

# Optional but recommended
brew install \
    jack \
    webrtc \
    nlohmann-json \
    asio
```

**What this does**: Installs all the libraries OBS needs to compile.

**Time**: 5-15 minutes depending on what's already installed.

---

### Step 6: Create Build Directory

```bash
# Create a separate build directory (keeps source clean)
mkdir build
cd build
```

**What this does**: Creates a folder where compiled files will go. This is a CMake best practice.

---

### Step 7: Configure with CMake

```bash
# Configure the build system
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_BROWSER=OFF \
    -DENABLE_AJA=OFF \
    -DENABLE_SCRIPTING=OFF \
    -G Ninja

# Explanation of flags:
# -DCMAKE_BUILD_TYPE=Release  = Optimized build (not for debugging)
# -DENABLE_BROWSER=OFF        = Skip CEF browser (not needed for plugin dev)
# -DENABLE_AJA=OFF            = Skip AJA hardware support
# -DENABLE_SCRIPTING=OFF      = Skip Python/Lua scripting
# -G Ninja                    = Use Ninja build system (faster than make)
```

**What this does**: Configures how OBS will be built for your Mac.

**Expected output**:
```
-- The C compiler identification is AppleClang 15.0.0.15000100
-- The CXX compiler identification is AppleClang 15.0.0.15000100
-- Found Qt6: /opt/homebrew/opt/qt@6 (found version "6.x.x")
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/.../obs-studio/build
```

**Time**: 1-2 minutes

**⚠️ Common Issues**:

| Error | Solution |
|-------|----------|
| "Qt6 not found" | `brew install qt` |
| "FFmpeg not found" | `brew install ffmpeg` |
| "Python not found" | `brew install python` |

---

### Step 8: Build OBS

```bash
# Build with Ninja (much faster than make)
ninja

# Or if you didn't use Ninja in Step 7:
# make -j8
```

**What this does**: Compiles all the OBS source code into working libraries.

**Time**: 10-30 minutes depending on your Mac

**Expected output** (will scroll by quickly):
```
[1/2456] Building C object libobs/CMakeFiles/obs.dir/obs.c.o
[2/2456] Building C object libobs/CMakeFiles/obs.dir/obs-source.c.o
[3/2456] Building C object libobs/CMakeFiles/obs.dir/obs-output.c.o
...
[2456/2456] Linking CXX executable UI/obs
```

**Progress indicators**:
- Numbers like `[500/2456]` show progress
- Should reach `[2456/2456]` or similar at the end
- Final line: `Linking CXX executable UI/obs`

**⚠️ Build Issues**:

If build fails, try:
```bash
# Clean and rebuild
ninja clean
ninja
```

---

### Step 9: Verify Build Success

```bash
# Check if main OBS library was built
ls -lh libobs/libobs.dylib

# Should show something like:
# -rwxr-xr-x  1 user  staff   3.5M Feb  8 10:30 libobs/libobs.dylib

# Check frontend API library
ls -lh UI/obs-frontend-api/libobs-frontend-api.dylib

# Check the OBS executable
ls -lh UI/obs

# Try running OBS from source (optional)
./UI/obs
```

**What this does**: Confirms that OBS compiled successfully.

---

### Step 10: Note Your OBS Path

```bash
# Print your OBS source path
echo "Your OBS_PATH is: $(pwd | sed 's/\/build$//')"

# Or just:
pwd
# Then remove '/build' from the end

# Example output:
# /Users/yphichdok/Development/obs-studio
```

**Save this path!** You'll need it to build the LowerThirdsPlus plugin.

---

## 🎯 What You Now Have

After completing these steps:

```
~/Development/obs-studio/
├── libobs/                        # Core OBS library
│   ├── obs-module.h              # Plugin API header ✅
│   ├── obs.h                     # Main OBS header ✅
│   └── ... (many more headers)
│
├── UI/obs-frontend-api/           # Frontend API
│   └── obs-frontend-api.h        # Frontend header ✅
│
└── build/                         # Compiled binaries
    ├── libobs/
    │   └── libobs.dylib          # Core library ✅
    └── UI/obs-frontend-api/
        └── libobs-frontend-api.dylib  # Frontend library ✅
```

---

## 📍 Using This with LowerThirdsPlus

Now you can build your plugin:

```bash
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus"

# Use the build script with your OBS path
./build.sh ~/Development/obs-studio

# Or manually:
mkdir build && cd build
cmake .. -DOBS_PATH=~/Development/obs-studio
make -j4
make install
```

---

## 🔍 Understanding the OBS Source Structure

### Key Directories:

| Directory | What It Contains | Why You Need It |
|-----------|------------------|-----------------|
| `libobs/` | Core OBS headers | Plugin API definitions |
| `UI/obs-frontend-api/` | Frontend headers | Dock panel integration |
| `build/libobs/` | Compiled libraries | Your plugin links to these |
| `plugins/` | Example plugins | Learn from these! |
| `deps/` | Dependencies | Third-party libraries |

### Key Files Your Plugin Uses:

```cpp
#include <obs-module.h>           // from libobs/
#include <obs-frontend-api.h>     // from UI/obs-frontend-api/
#include <graphics/graphics.h>    // from libobs/graphics/
#include <util/platform.h>        // from libobs/util/
```

---

## ⚡ Quick Reference Commands

### Full Build from Scratch

```bash
# All steps in one script
cd ~/Development
git clone --recursive https://github.com/obsproject/obs-studio.git
cd obs-studio
git checkout 30.2.2
git submodule update --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_BROWSER=OFF -G Ninja
ninja
echo "OBS built successfully at: $(pwd | sed 's/\/build$//')"
```

### Rebuild After Changes

```bash
cd ~/Development/obs-studio/build
ninja
```

### Clean Build

```bash
cd ~/Development/obs-studio
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_BROWSER=OFF -G Ninja
ninja
```

---

## 🐛 Troubleshooting

### Problem: "git clone" is very slow

**Solution**: Your internet might be slow. You can:
```bash
# Use shallow clone (faster, downloads less history)
git clone --recursive --depth 1 https://github.com/obsproject/obs-studio.git
```

### Problem: CMake can't find Qt6

**Solution**:
```bash
# Set Qt path manually
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DQt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6 \
    -G Ninja
```

### Problem: Build fails with "No space left on device"

**Solution**: Free up disk space. OBS build needs ~10GB.
```bash
# Check available space
df -h

# Clean Homebrew cache
brew cleanup
```

### Problem: Build is very slow

**Solution**: Use Ninja (faster) and more CPU cores:
```bash
# Install ninja if not already
brew install ninja

# Configure with Ninja
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build with all CPU cores
ninja -j$(sysctl -n hw.ncpu)
```

### Problem: Linker errors about missing symbols

**Solution**: Make sure all dependencies are installed:
```bash
brew install mbedtls ffmpeg qt cmake ninja
```

---

## 💡 Pro Tips

### Tip 1: Keep Source and Build Separate

```bash
# Good structure:
~/Development/
├── obs-studio/           # Source code (never modify)
│   └── build/           # Build output
└── OBS PLUGINS/         # Your plugins
    └── LowerThirdsPlus/
```

### Tip 2: Update OBS Source

```bash
cd ~/Development/obs-studio
git pull
git submodule update --recursive
cd build
ninja
```

### Tip 3: Build Debug Version for Testing

```bash
# In build directory
cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja
ninja
```

This gives you better error messages when developing plugins.

### Tip 4: Explore OBS Plugins for Examples

```bash
# Look at built-in plugins for examples
cd ~/Development/obs-studio/plugins
ls -la

# Great examples:
# - image-source/
# - text-freetype2/
# - obs-transitions/
```

---

## 📚 Additional Resources

- **OBS Studio GitHub**: https://github.com/obsproject/obs-studio
- **OBS Building Guide**: https://github.com/obsproject/obs-studio/wiki/Building-OBS-Studio
- **OBS Plugin API Docs**: https://obsproject.com/docs/
- **OBS Discord**: https://obsproject.com/discord

---

## ✅ Summary

You now understand that **building OBS from source** means:

1. ✅ Downloading OBS source code from GitHub
2. ✅ Installing required dependencies (Qt, FFmpeg, etc.)
3. ✅ Running CMake to configure the build
4. ✅ Compiling with Ninja/Make (10-30 minutes)
5. ✅ Getting header files and libraries for plugin development

**You don't replace your installed OBS** - this is purely for plugin development.

**Total Time**: ~30-60 minutes (mostly waiting for compilation)

**Disk Space**: ~10GB

---

## 🎬 Next: Build Your Plugin

Once OBS is built, run:

```bash
cd "/Users/yphichdok/Desktop/OBS PLUGINS/LowerThirdsPlus"
./build.sh ~/Development/obs-studio
```

Then test in your regular OBS Studio app! 🚀

---

**Questions?** Check the troubleshooting section or refer to the official OBS build guide.
