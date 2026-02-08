#!/bin/bash

# Lower Thirds Plus - Automatic Installer
# This script will install the plugin to OBS Studio

clear
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Lower Thirds Plus - OBS Plugin Installer"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PLUGIN_PATH="$SCRIPT_DIR/LowerThirdsPlus.plugin"
INSTALL_PATH="$HOME/Library/Application Support/obs-studio/plugins"

echo "🔍 Checking requirements..."
echo ""

# Check if plugin exists
if [ ! -d "$PLUGIN_PATH" ]; then
    echo "❌ ERROR: Plugin not found at:"
    echo "   $PLUGIN_PATH"
    echo ""
    echo "Make sure LowerThirdsPlus.plugin is in the same folder as this installer."
    echo ""
    read -p "Press Enter to exit..."
    exit 1
fi

echo "✅ Plugin found: LowerThirdsPlus.plugin"

# Check if OBS is running
if pgrep -x "OBS" > /dev/null; then
    echo "⚠️  WARNING: OBS Studio is currently running"
    echo ""
    echo "Please close OBS Studio before installing the plugin."
    echo ""
    read -p "Press Enter after closing OBS, or Ctrl+C to cancel..."
fi

echo ""
echo "📦 Installing plugin..."
echo ""

# Create plugins directory if it doesn't exist
if [ ! -d "$INSTALL_PATH" ]; then
    echo "Creating plugins directory..."
    mkdir -p "$INSTALL_PATH"
fi

# Remove old version if it exists
if [ -d "$INSTALL_PATH/LowerThirdsPlus.plugin" ]; then
    echo "Removing old version..."
    rm -rf "$INSTALL_PATH/LowerThirdsPlus.plugin"
fi

# Copy plugin
echo "Copying plugin to OBS..."
cp -R "$PLUGIN_PATH" "$INSTALL_PATH/"

# Check if installation was successful
if [ -d "$INSTALL_PATH/LowerThirdsPlus.plugin" ]; then
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  ✅ Installation Complete!"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Plugin installed to:"
    echo "  $INSTALL_PATH/LowerThirdsPlus.plugin"
    echo ""
    echo "🚀 Next steps:"
    echo "  1. Launch OBS Studio"
    echo "  2. Add a new source: +"
    echo "  3. Select 'Lower Thirds Plus Source'"
    echo "  4. Right-click → Properties to configure"
    echo ""
    echo "📖 For detailed instructions, see INSTALL.md"
    echo ""
else
    echo ""
    echo "❌ Installation failed!"
    echo ""
    echo "Please try manual installation:"
    echo "  cp -R LowerThirdsPlus.plugin ~/Library/Application\\ Support/obs-studio/plugins/"
    echo ""
fi

read -p "Press Enter to exit..."
