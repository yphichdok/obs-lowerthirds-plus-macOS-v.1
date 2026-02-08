# How to Install on macOS

## ⚠️ Security Warning: "Apple could not verify..."

**This is normal!** You'll see this message:

> "Apple could not verify '🚀 Install.command' is free of malware..."

This happens because the installer is not code-signed with an Apple Developer certificate ($99/year). The plugin is open-source and safe.

---

## ✅ Solution: How to Run the Installer

### **Method 1: Right-Click → Open** (Easiest)

1. **Mount the DMG** - Double-click `LowerThirdsPlus-v1.0-macOS.dmg`
2. **Right-click** (or Control-click) on `🚀 Install.command`
3. Select **"Open"** from the menu
4. Click **"Open"** again in the security dialog
5. The installer will run in Terminal
6. Follow the on-screen instructions

**Why this works:** Right-clicking and choosing "Open" bypasses Gatekeeper for that specific file.

---

### **Method 2: System Settings** (If Method 1 doesn't work)

1. Try to double-click `🚀 Install.command` (it will be blocked)
2. Open **System Settings** (or System Preferences)
3. Go to **Privacy & Security**
4. Scroll down to the **Security** section
5. Click **"Open Anyway"** next to the blocked message
6. Confirm by clicking **"Open"**

---

### **Method 3: Terminal Command** (Advanced)

If you're comfortable with Terminal:

```bash
# Remove the quarantine flag
cd "/Volumes/Lower Thirds Plus v1.0"
xattr -d com.apple.quarantine "🚀 Install.command"
xattr -d com.apple.quarantine "🗑️ Uninstall.command"

# Now double-click the installer normally
```

---

### **Method 4: Manual Installation** (No scripts needed)

If you prefer not to run any scripts:

1. Mount the DMG: Double-click `LowerThirdsPlus-v1.0-macOS.dmg`
2. **Close OBS Studio** if it's running
3. Open Finder and press: **Cmd + Shift + G**
4. Type: `~/Library/Application Support/obs-studio/plugins/`
5. Press **Enter**
6. **Drag and drop** `LowerThirdsPlus.plugin` into that folder
7. Restart OBS Studio

---

## 🔒 Why Does This Happen?

macOS Gatekeeper checks for:
- **Code signing** - Requires Apple Developer Program membership ($99/year)
- **Notarization** - Apple's malware scanning service

Since this is a **free, open-source plugin**, it's not code-signed. The source code is publicly available on GitHub for transparency.

---

## ✅ Is It Safe?

**Yes!** You can verify:
- ✅ **Open source:** All code is on GitHub
- ✅ **Readable scripts:** Open `🚀 Install.command` in a text editor to see exactly what it does
- ✅ **Simple actions:** It only copies a file to the OBS plugins folder
- ✅ **No network access:** Doesn't download or upload anything
- ✅ **No system modifications:** Only touches OBS plugin folder

---

## 📖 What the Installer Does

The installer script simply:
1. Checks if OBS is running (warns you to close it)
2. Creates the plugins directory if needed
3. Copies `LowerThirdsPlus.plugin` to `~/Library/Application Support/obs-studio/plugins/`
4. Shows a success message

That's it! Nothing hidden, nothing dangerous.

---

## 🗑️ Uninstalling

The same process applies to `🗑️ Uninstall.command`:
- **Right-click → Open** to run it
- Or use **Manual Uninstall:** Delete `~/Library/Application Support/obs-studio/plugins/LowerThirdsPlus.plugin`

---

## 🔑 Code Signing (For Developers)

To eliminate this warning, the plugin would need:
1. Apple Developer Program membership ($99/year)
2. Code signing certificate
3. Notarization by Apple

Since this is a free plugin, this overhead isn't included. The right-click method is the standard workaround for open-source Mac software.

---

## ❓ Still Having Issues?

- **Check:** You're using macOS 10.15 or later
- **Check:** OBS Studio is closed before installing
- **Try:** Manual installation method (no scripts required)
- **Read:** The full USER_GUIDE.md for complete instructions

---

**Created by Y Phic Hdok, MTD Technologies**
