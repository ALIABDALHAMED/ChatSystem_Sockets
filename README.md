# Chat System - Socket-based Client/Server with ImGui GUI

A modern C++17 chat application featuring:
- **Server**: Multi-client broadcast server using Windows Sockets
- **CLI Client**: Command-line chat client with thread-safe networking
- **GUI Client**: Cross-platform ImGui + GLFW chat client with scrollable message history

## Project Structure

```
ChatSystem_Sockets/
├── CMakeLists.txt              # Build configuration
├── include/
│   ├── gui/
│   │   └── ChatGui.hpp         # GUI abstraction layer
│   └── networking/
│       └── ChatClient.hpp      # Networking abstraction
├── src/
│   ├── client.cpp              # CLI client entry point
│   ├── server.cpp              # Server (unchanged)
│   ├── gui/
│   │   └── ChatGui.cpp         # GUI implementation
│   └── networking/
│       └── ChatClient.cpp      # Networking implementation
├── gui/
│   ├── main_gui.cpp            # GUI client entry point
│   └── imgui/                  # ImGui + backends
└── cmake-build-debug/          # Build output
```

## Key Improvements

### Networking Layer (`ChatClient`)
- **Thread-safe message queue** for non-blocking receive
- **Proper resource cleanup** with RAII patterns
- **Non-blocking socket operations** to prevent UI freezing
- **Better error handling** and connection status tracking

### Client Architecture
- **Separation of concerns**: GUI, networking, and core logic are decoupled
- **Modern C++17**: Smart pointers, atomic variables, lambdas
- **Clean interfaces**: Well-defined public APIs

### GUI (`ChatGui`)
- **ImGui-based**: Cross-platform, lightweight UI framework
- **GLFW windowing**: Native window management
- **Real-time message display**: Scrollable chat log with auto-scroll
- **Connection menu**: Easy server connection management
- **Message history**: Limits to 1000 messages to prevent memory issues

## Dependencies

### Required:
- **CMake** 3.10+
- **C++17 compiler** (MSVC, GCC, or Clang)
- **Windows SDK** (for Winsock2)
- **GLFW3** (for GUI client)
- **OpenGL 3.0+** (for GUI rendering)

### Installation (vcpkg):
```powershell
vcpkg install glfw3:x64-windows
vcpkg integrate install
```

## Building

```bash
cd ChatSystem_Sockets
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Debug
```

Or in CLion: CMake will auto-configure.

## Running

### Start the Server
```bash
./build/Debug/server
```

### Run CLI Client (in new terminal)
```bash
./build/Debug/client
```

### Run GUI Client (in new terminal)
```bash
./build/Debug/ChatGUI
```

The GUI client will auto-connect to `127.0.0.1:54000`.

## Features

### GUI Client
- **Real-time messaging**: Send and receive messages instantly
- **Connection status**: Visual indicator (red = disconnected, connected = green)
- **Menu bar**: Connect/Disconnect options
- **Auto-scroll**: Chat log follows new messages
- **Input validation**: Enter key sends messages
- **Message formatting**: Shows sender and timestamp for each message

### Server
- **Multi-client support**: Broadcasts to all connected clients
- **Thread-per-client**: Handles multiple connections concurrently
- **Clean shutdown**: Removes disconnected clients properly

## Code Quality

- **Modern C++ patterns**: Smart pointers, RAII, const-correctness
- **Thread safety**: Mutex-protected message queues
- **Error handling**: Comprehensive error checking and reporting
- **Documentation**: Header-level inline documentation
- **Compiler warnings**: `-Wall -Wextra -Werror` enabled

## Architecture Notes

### ChatClient Class
```cpp
class ChatClient {
    bool connect(const std::string& host, int port);
    bool send_message(const std::string& message);
    std::string receive_message();  // Non-blocking
    bool has_pending_messages() const;
};
```

### ChatGui Class
```cpp
class ChatGui {
    bool init(const std::string& title, int width, int height);
    void render();
    void connect(const std::string& host, int port);
    bool is_connected() const;
};
```

Both classes are thread-safe and handle resource cleanup automatically.

## Troubleshooting

### "Failed to connect"
- Ensure server is running: `./server`
- Check port 54000 is not blocked by firewall
- Try explicit IP: use `Connection > Connect` menu

### GUI freezes
- Due to the recv thread design, the GUI should remain responsive
- If it freezes, check Windows Event Viewer for crashes

### Messages not appearing
- Ensure network messages end with `\n` (auto-added by client)
- Check server console for errors
- Verify all three programs are on same machine/network

## Future Enhancements

- UDP support for lower latency
- SSL/TLS encryption
- User authentication
- Persistent message history (database)
- Cross-platform file transfer
- Dark/Light theme toggle
- Emoji support

---

**Author**: Chat System Project  
**C++ Version**: 17  
**Date**: November 2025
