# GUI Client User Guide

## Overview

The Chat GUI Client (`ChatGUI`) is a modern, cross-platform chat application built with ImGui and GLFW. It provides a user-friendly interface for real-time messaging with a broadcast server.

## Main Window

### Layout

```
┌─────────────────────────────────────────────────────────┐
│  File  Connection  Help                            [_][□][X]  │  <- Menu Bar
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │ Chat Log (Scrollable)                            │  │
│  │                                                  │  │
│  │ [System]: Welcome to Chat Client                 │  │
│  │           Type your messages below...            │  │
│  │                                                  │  │
│  │ [You]: Hello everyone!                           │  │
│  │ [Remote]: Hi there!                              │  │
│  │ [Remote]: How are you?                           │  │
│  │                                                  │  │
│  │ ↓ (auto-scrolls to newest messages)              │  │
│  └──────────────────────────────────────────────────┘  │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │ Type message here... [Send]                     │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

## Features

### 1. Connection Management

**Menu Bar → Connection**
- **Connect (localhost:54000)**: Auto-connects to local server
- **Disconnect**: Closes connection gracefully
- Status shows "Not connected" (red) when disconnected

**Auto-Connect**
- GUI automatically attempts to connect on startup
- Look for "Connected!" message in chat log

### 2. Sending Messages

**Method 1**: Click Send Button
1. Type message in input field at bottom
2. Click the blue "Send" button
3. Message appears in chat with "[You]:" prefix

**Method 2**: Press Enter
1. Type message
2. Press Enter/Return key
3. Message sends immediately

**Features**:
- Empty messages are ignored
- Messages auto-append newline
- Input field clears after send
- Send disabled when not connected (red "Not connected" warning)

### 3. Viewing Messages

**Chat Log Display**:
- Shows all messages with sender prefix: `[Sender]: Message`
- Message format: `[System]` for server messages, `[You]` for yours, `[Remote]` for others
- Automatically scrolls to latest message
- Supports text wrapping for long messages
- Maintains history (up to 1000 messages)

**Scrolling**:
- Manual scroll with mouse wheel or scrollbar
- Auto-scrolls when new messages arrive
- "Sticky" scroll: if you scroll up to read old messages, auto-scroll stops until new message arrives

### 4. System Messages

**Automatic Messages** appear as `[System]`:
- "Connected!" - Successfully connected to server
- "Connection failed" - Failed to reach server
- "Disconnected" - Server closed or user clicked disconnect
- Welcome message on startup

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Enter | Send message |
| Ctrl+A | Select all text in input |
| Ctrl+C/V/X | Copy/paste/cut in input |
| Esc | Close any popup (if implemented) |
| Tab | Switch UI focus |

## Troubleshooting

### "Not connected" appears in red

**Cause**: GUI couldn't connect to server
**Solution**:
1. Make sure server is running: `./server`
2. Check firewall allows port 54000
3. Try menu: Connection → Disconnect → Connection → Connect
4. Check Windows Event Viewer for detailed errors

### No messages appear

**Cause 1**: Not actually connected
- Check status bar shows "Connected!"

**Cause 2**: Server not running
- Start server in another terminal: `./server`

**Cause 3**: Messages not formatted correctly
- Our GUI adds `\n` automatically, should work fine

**Solution**: Use CLI client to test:
1. Open new terminal
2. Run `./client`
3. Type a message
4. Should appear in GUI from all clients

### GUI is frozen

**This shouldn't happen** due to thread design, but if it does:
1. Press Alt+F4 to force close
2. Check server is still running
3. Restart GUI client
4. Report as bug with details

### Very old messages disappear

**Normal behavior**: Chat log limited to 1000 messages
- Oldest messages deleted when limit reached
- Prevents memory issues for long sessions
- Solution: Save important messages externally

## Advanced Usage

### Connecting to Different Servers

**Default**: `127.0.0.1:54000` (localhost)

**To connect to different server** (requires recompilation):
1. Edit `gui/main_gui.cpp` line ~15
2. Change: `gui.connect("YOUR_IP", YOUR_PORT);`
3. Recompile

### Multiple GUIs on Same Machine

You can run multiple GUI instances:
1. Terminal 1: `./server`
2. Terminal 2: `./ChatGUI`
3. Terminal 3: `./ChatGUI` (another instance)
4. All instances see the same messages (broadcast model)

### Using with CLI Client

1. Terminal 1: `./server`
2. Terminal 2: `./ChatGUI`
3. Terminal 3: `./client` (CLI)
4. Type in any client, all receive the message
5. Great for testing/debugging!

## Window Controls

- **Resize**: Drag corner or edge of window
- **Move**: Drag title bar
- **Minimize/Maximize**: Standard window buttons
- **Close**: Click X or Alt+F4

## Messages Format

**What you send**:
```
Hello world
```

**How it appears in chat**:
```
[You]: Hello world
```

**How others see it**:
```
[Remote]: Hello world
```

**System messages**:
```
[System]: Connected!
```

## Performance Tips

- **Large chat history**: Scroll performance may degrade past 1000 messages
- **Network lag**: All messages go through server (slight delay expected)
- **High message rate**: GUI remains responsive due to threading
- **CPU usage**: Should be very low (minimal rendering when idle)

## Limitations

- ❌ No username customization (currently "You" vs "Remote")
- ❌ No private messages (broadcast only)
- ❌ No message timestamps
- ❌ No user list
- ❌ No file transfer
- ❌ No emoji support
- ❌ No read receipts
- ❌ No message editing/deletion

## Future Features

- User authentication and usernames
- Direct messaging
- Message timestamps
- User presence indicator
- Typing status ("User is typing...")
- Message reactions
- File sharing
- Rich text formatting
- Dark/Light theme toggle

## Technical Details

- **UI Framework**: ImGui (immediate mode)
- **Windowing**: GLFW 3
- **Rendering**: OpenGL 3.0+
- **Threading**: Separate receive thread for networking
- **Language**: Modern C++17
- **Platform**: Windows (adaptable to Linux/macOS with cross-platform GLFW)

---

**Enjoy using Chat GUI! For issues, check README.md for setup instructions.**
