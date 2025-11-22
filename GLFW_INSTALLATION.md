# Installing GLFW3 for Chat GUI

## Option 1: Using vcpkg (Recommended for CLion)

### Step 1: Clone vcpkg (if not already done)
```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

### Step 2: Install GLFW3
```powershell
.\vcpkg install glfw3:x64-windows
```

### Step 3: Integrate vcpkg with CLion

#### Method A: Command Line Integration (One-time)
```powershell
.\vcpkg integrate install
```

This adds vcpkg to your system PATH.

#### Method B: CMake Toolchain (Recommended for CLion)
When configuring the project in CLion:

1. Go to **Settings** → **Build, Execution, Deployment** → **CMake**
2. In the **CMake Options** field, add:
```
-DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Replace `C:\path\to\vcpkg` with your actual vcpkg installation path.

3. Click **OK** to apply

### Step 4: Rebuild in CLion

1. Click **File** → **Reload CMake Project**
2. Or press **Ctrl+T** to rebuild
3. CMake should now find GLFW3

---

## Option 2: Manual Installation

### Step 1: Download GLFW
1. Visit https://www.glfw.org/download.html
2. Download the Windows binaries (precompiled)

### Step 2: Extract and Setup
```
C:\Libraries\glfw-3.x.x.bin.WIN64\
  ├─ include/
  │  └─ GLFW/
  ├─ lib-vc2022/  (or your Visual Studio version)
  └─ lib-mingw-w64/
```

### Step 3: Configure CMake
Update your CMake call:
```powershell
cmake .. -DGLFW3_DIR="C:\Libraries\glfw-3.x.x.bin.WIN64" -DCMAKE_PREFIX_PATH="C:\Libraries\glfw-3.x.x.bin.WIN64"
```

---

## Option 3: Using Chocolatey

```powershell
choco install glfw
```

Then configure CMake with the installation path.

---

## Troubleshooting

### "Could not find glfw3Config.cmake"

**Solution**: Use Option 1 (vcpkg) - it's the easiest and most compatible.

### CMake can't find the toolchain file

**Solution**: Double-check the path to `vcpkg.cmake`:
```powershell
# Verify the file exists
Test-Path "C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

### Still getting errors after installation?

**Step 1**: Delete the build directory
```powershell
rm -r cmake-build-debug
```

**Step 2**: Reload CMake in CLion
- **File** → **Reload CMake Project**

**Step 3**: Rebuild
- **Build** → **Rebuild Project** (or Ctrl+F9)

---

## Verifying Installation

### Test 1: Check vcpkg
```powershell
.\vcpkg list glfw3
```

Should output something like:
```
glfw3:x64-windows       3.4
```

### Test 2: Test CMake
```powershell
cd ChatSystem_Sockets
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Should succeed without GLFW errors.

---

## What Changes After Installation?

Once GLFW3 is installed and CMake finds it:

1. **Server target**: Still builds (uses Winsock only)
2. **Client target**: Still builds (uses Winsock only)
3. **ChatGUI target**: NOW BUILDS! (uses GLFW + OpenGL + ImGui)

All three executables will be available:
- `.\Debug\server.exe`
- `.\Debug\client.exe`
- `.\Debug\ChatGUI.exe` ← NOW AVAILABLE!

---

## If You Don't Want to Install GLFW

The `CMakeLists.txt` is now **flexible**:
- If GLFW3 is found → ChatGUI builds
- If GLFW3 is NOT found → Only server and client build

This allows you to:
1. Build and run the **server** and **CLI client** without GLFW
2. Later install GLFW when you want the GUI

---

## Quick Summary

**For CLion users (recommended):**
```powershell
# 1. Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git

# 2. Bootstrap
cd vcpkg
.\bootstrap-vcpkg.bat

# 3. Install GLFW
.\vcpkg install glfw3:x64-windows

# 4. In CLion settings, add to CMake Options:
# -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake

# 5. Reload CMake project in CLion
# 6. Build!
```

---

**All done!** Now `ChatGUI` will build successfully. 🎉

If you have any issues, check the "Troubleshooting" section above.
