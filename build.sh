#!/bin/bash

# LowerThirdsPlus Build Script for macOS
# Usage: ./build.sh [OBS_PATH]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}==================================${NC}"
echo -e "${GREEN}LowerThirdsPlus Build Script${NC}"
echo -e "${GREEN}==================================${NC}"
echo ""

# Check for OBS_PATH argument
if [ -z "$1" ]; then
    echo -e "${RED}Error: OBS_PATH not provided${NC}"
    echo "Usage: ./build.sh /path/to/obs-studio"
    echo ""
    echo "Example:"
    echo "  ./build.sh ~/Development/obs-studio"
    exit 1
fi

OBS_PATH="$1"

# Verify OBS path exists
if [ ! -d "$OBS_PATH" ]; then
    echo -e "${RED}Error: OBS path does not exist: $OBS_PATH${NC}"
    exit 1
fi

if [ ! -f "$OBS_PATH/libobs/obs-module.h" ]; then
    echo -e "${RED}Error: Not a valid OBS source directory${NC}"
    echo "Make sure you point to the OBS Studio source code directory"
    exit 1
fi

echo -e "${GREEN}✓${NC} OBS path verified: $OBS_PATH"

# Check dependencies
echo ""
echo "Checking dependencies..."

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}✗${NC} CMake not found. Install with: brew install cmake"
    exit 1
fi
echo -e "${GREEN}✓${NC} CMake found: $(cmake --version | head -n1)"

# Check Qt6
if ! brew list qt &> /dev/null; then
    echo -e "${YELLOW}!${NC} Qt6 not found. Installing..."
    brew install qt
fi
echo -e "${GREEN}✓${NC} Qt6 found"

# Check nlohmann-json (optional, will be downloaded if not found)
if brew list nlohmann-json &> /dev/null; then
    echo -e "${GREEN}✓${NC} nlohmann-json found"
else
    echo -e "${YELLOW}!${NC} nlohmann-json not found (will be downloaded via CMake)"
fi

# Create build directory
echo ""
echo "Setting up build directory..."
rm -rf build
mkdir build
cd build

# Configure
echo ""
echo -e "${GREEN}Configuring project...${NC}"
cmake .. -DOBS_PATH="$OBS_PATH" -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo -e "${GREEN}Building plugin...${NC}"
make -j$(sysctl -n hw.ncpu)

# Install
echo ""
echo -e "${GREEN}Installing plugin...${NC}"
make install

echo ""
echo -e "${GREEN}==================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}==================================${NC}"
echo ""
echo "Plugin installed to:"
echo "  ~/Library/Application Support/obs-studio/plugins/LowerThirdsPlus.plugin/"
echo ""
echo "Next steps:"
echo "  1. Launch OBS Studio"
echo "  2. Add a 'LowerThirdsPlusSource' to your scene"
echo "  3. Open the control panel: View → Docks → Lower Thirds Control"
echo ""
