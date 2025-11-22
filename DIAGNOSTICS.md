# Diagnosing Error 10057 (Connection Reset)

## What's Happening

When you get `[ChatClient] recv() error: 10057`, it means the server closed the connection. This could be:

1. **Expected**: Server process crashed or was stopped
2. **Unexpected**: Server closed connection unexpectedly (bug)
3. **Network**: Physical network disconnection
4. **Firewall**: Network firewall reset the connection

## How to Diagnose

### Look at Server Console Output

The improved server now shows:
```
Server listening on port 54000
New client connected. Total clients: 1
Broadcasting: Hello
Send error to client (error: X), may need to disconnect
Client gracefully disconnected
Client removed. Active clients: 0
```

### Look at Client Console Output

```
[ChatClient] Connection reset by server (error: 10057)
```

## Common Scenarios

### Scenario 1: Normal Disconnect ✅
**Server output:**
```
Client gracefully disconnected
```

**Client output:**
```
[ChatClient] Server closed connection
```

**What happened:** Client cleanly disconnected (pressed Ctrl+C)

---

### Scenario 2: Server Crashed ❌
**Server output:**
```
(nothing - just stops)
```

**Client output:**
```
[ChatClient] Connection reset by server (error: 10057)
```

**What happened:** Server process crashed or was killed

**Fix:** Restart server, then clients can reconnect

---

### Scenario 3: Send Error (Bug) ⚠️
**Server output:**
```
Send error to client (error: 10054), may need to disconnect
```

**Client output:**
```
[ChatClient] Connection reset by server (error: 10057)
```

**What happened:** Server failed to send message, connection broke

**Fix:** This improved server logs these errors now

---

### Scenario 4: Multiple Clients
**Server output:**
```
New client connected. Total clients: 1
New client connected. Total clients: 2
Broadcasting: Message from client 1
Client removed. Active clients: 1
```

**What happened:** Working correctly with multiple clients

---

## Testing

### Test 1: Normal Operation
```
Terminal 1: .\Debug\server.exe
Terminal 2: .\Debug\client.exe
Terminal 3: .\Debug\client.exe

Type message in each client
All should receive it
Press Ctrl+C in a client
Server should show: Client gracefully disconnected
```

### Test 2: Server Crash
```
Terminal 1: .\Debug\server.exe
Terminal 2: .\Debug\client.exe

Wait a few seconds...
In Terminal 1: Press Ctrl+C (crash server)

Terminal 2 should show:
[ChatClient] Connection reset by server (error: 10057)
[SYSTEM] Connection lost
```

### Test 3: Multiple Messages
```
Terminal 1: .\Debug\server.exe
Terminal 2: .\Debug\client.exe
Terminal 3: .\Debug\client.exe

Rapidly send messages from each client
Server should broadcast to all
Check for any Send errors
```

## Improvements Made

✅ **Server now logs:**
- Client connections/disconnections
- Broadcast attempts
- Send errors
- Active client count

✅ **Client now logs:**
- Specific error codes (10057, 10054, etc.)
- Connection reset vs graceful close
- System messages for UI

## What to Check

| Issue | Check This |
|-------|-----------|
| Frequent disconnects | Network connectivity, firewall |
| Error 10054 (different from 10057) | Check TCP keep-alive settings |
| Server stops without message | Check for exceptions/crashes |
| Clients not receiving broadcasts | Check send errors on server |

## Rebuilding

```powershell
cd cmake-build-debug
cmake --build . --config Debug
```

Now run with improved logging:
```powershell
.\Debug\server.exe
.\Debug\client.exe
```

Watch the console output to understand what's happening!

---

**Note:** Error 10057 is often NORMAL when server closes. The client now handles it gracefully with system messages.
