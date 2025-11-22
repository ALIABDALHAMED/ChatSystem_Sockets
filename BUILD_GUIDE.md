# Quick Fix: Build Server & Client WITHOUT GLFW (ChatGUI)

## ✅ What You Can Do RIGHT NOW

The server and client build **without GLFW**. Here's the quick setup:

### Step 1: Build server and client
```powershell
cd E:\EELU\Semester 5\Operating systems\PROJECT\ChatSystem_Sockets
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Debug
```

### Step 2: Run them
```powershell
# Terminal 1: Start server
.\Debug\server.exe

# Terminal 2: Start CLI client
.\Debug\client.exe
```

### What you get:
✅ `server.exe` - Multi-client broadcast server  
✅ `client.exe` - CLI chat client  
⏳ `ChatGUI.exe` - Skipped (needs GLFW)

---

## 🔧 For ChatGUI (with GLFW) - Pick One Option

### Option A: Use vcpkg (Easiest for CLion)

```powershell
# 1. Install GLFW via vcpkg
vcpkg install glfw3:x64-windows

# 2. In CLion, go to Settings → Build → CMake
# Add to CMake Options:
# -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake

# 3. Reload CMake project (File → Reload CMake Project)

# 4. Build again - ChatGUI should now build!
```

### Option B: Build GLFW manually for MinGW

```powershell
# 1. Download GLFW source
cd C:\libs
git clone https://github.com/glfw/glfw.git
cd glfw

# 2. Create build directory
mkdir build-mingw
cd build-mingw

# 3. Configure with MinGW
# Use the CMake GUI or:
cmake .. -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=C:\libs\glfw-build

# 4. Build GLFW
cmake --build . --config Release

# 5. Install
cmake --install .

# 6. Now update CMakeLists.txt line 51:
# set(GLFW_DIR "C:/libs/glfw-build")  # instead of C:/libs/glfw

# 7. Reload CMake in CLion
```

### Option C: Use Pre-built GLFW (Fastest)

1. Download pre-built GLFW from https://www.glfw.org/download.html
2. Extract to `C:/libs/glfw/`
3. Verify structure:
   ```
   C:/libs/glfw/
   ├─ include/GLFW/glfw3.h
   ├─ lib/libglfw3.a
   └─ ...
   ```
4. In CLion: **File → Reload CMake Project**

---

## 📊 Current Status

```
Server:  ✅ READY TO BUILD
Client:  ✅ READY TO BUILD
ChatGUI: ⏳ Waiting for GLFW

All three should appear in CLion's Build menu
once you pick an option above.
```

---

## 🎯 Next Steps

**Immediate**: Build and test server + client (no GLFW needed)  
**Later**: Pick one option above to add ChatGUI

The CMakeLists.txt is now flexible - it builds what it can!

---

## ❓ Troubleshooting

**Q: "ChatGUI target will NOT be built" message**  
A: Pick one of the 3 options above to install GLFW

**Q: server and client don't show in CLion build menu**  
A: They will once CMake finishes configuring. Reload project: **File → Reload CMake Project**

**Q: Still having issues?**  
A: Just build server and client for now. Add ChatGUI GUI later when you have GLFW.
