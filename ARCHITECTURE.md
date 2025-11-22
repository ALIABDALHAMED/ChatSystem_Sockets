# Architecture & Design Documentation

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        CHAT SYSTEM                              │
└─────────────────────────────────────────────────────────────────┘

Server Side:
┌──────────────────────────────────────┐
│         server.cpp (main)            │
├──────────────────────────────────────┤
│  - Listens on port 54000             │
│  - Accepts multiple connections      │
│  - Broadcasts messages to all        │
│  - Thread-per-client model           │
│  - Uses Winsock2                     │
└──────────────────────────────────────┘

Client Side:
┌──────────────────────────────────────────────────────────────────┐
│                  CLI Client             │      GUI Client       │
├─────────────────────────────────────┼──────────────────────────┤
│  client.cpp (main)                  │  gui/main_gui.cpp        │
│  └─> ChatClient                     │  └─> ChatGui             │
│      ├─ recv_loop()                 │      ├─ render_chat()    │
│      ├─ send_message()              │      ├─ render_input()   │
│      └─ msg queue (thread-safe)     │      └─ connect/send     │
│                                      │                          │
│  Uses: Threads, Winsock2           │  Uses: ImGui, GLFW       │
│  Output: Console                    │  Output: OpenGL window   │
└─────────────────────────────────────┴──────────────────────────┘
```

## Layered Architecture

### Layer 1: Networking Abstraction
```
┌────────────────────────────────────┐
│   ChatClient (include/networking)  │
├────────────────────────────────────┤
│ Public API:                        │
│  - connect(host, port): bool       │
│  - send_message(msg): bool         │
│  - receive_message(): string       │
│  - is_connected(): bool            │
│  - has_pending_messages(): bool    │
├────────────────────────────────────┤
│ Implementation:                    │
│  - SOCKET management (Winsock2)    │
│  - Receive thread (recv_loop)      │
│  - Thread-safe queue (queue_mutex) │
│  - Non-blocking socket operations  │
├────────────────────────────────────┤
│ No dependencies on GUI or app      │
└────────────────────────────────────┘
```

### Layer 2: GUI Abstraction
```
┌────────────────────────────────────┐
│     ChatGui (include/gui)          │
├────────────────────────────────────┤
│ Public API:                        │
│  - init(title, width, height)      │
│  - render()                        │
│  - connect(host, port)             │
│  - is_running(): bool              │
│  - shutdown()                      │
├────────────────────────────────────┤
│ Implementation:                    │
│  - ImGui rendering                 │
│  - GLFW window management          │
│  - Chat message display            │
│  - Input handling                  │
├────────────────────────────────────┤
│ Uses ChatClient for networking     │
│ No direct socket operations        │
└────────────────────────────────────┘
```

### Layer 3: Application
```
┌────────────────────────────────────┐
│    CLI Client (src/client.cpp)     │
├────────────────────────────────────┤
│  Creates ChatClient                │
│  Reads stdin, displays stdout      │
│  Simple UI thread                  │
└────────────────────────────────────┘

┌────────────────────────────────────┐
│    GUI Client (gui/main_gui.cpp)   │
├────────────────────────────────────┤
│  Creates ChatGui                   │
│  Runs render loop                  │
│  Handles window events             │
└────────────────────────────────────┘
```

## Message Flow

### Sending Messages

```
User Types Message
       │
       ▼
GUI Input Field
       │
       ▼
[Send Button Click]
       │
       ▼
ChatGui::render_input_area()
       │
       ▼
if message not empty:
  ChatClient::send_message(msg)
       │
       ▼
Winsock2 send() ─────────────────> [Network] ─────────────> Server
       │
       ▼
Message Added to Chat Log
       │
       ▼
Display "[You]: message"
```

### Receiving Messages

```
Server Broadcasts Message
       │
       ▼
[Network]
       │
       ▼
Winsock2 recv() in ChatClient::recv_loop()
       │
       ▼
Message Added to Queue (thread-safe)
       │
       ▼
Main Thread:
  ChatGui::render()
    └─> handle_incoming_messages()
       │
       ▼
ChatClient::receive_message()
       │
       ▼
Add to Chat Log
       │
       ▼
Display "[Remote]: message"
       │
       ▼
Auto-scroll to newest message
```

## Thread Synchronization

### ChatClient Threading Model

```
Main Thread                    Receive Thread
    │                               │
    │                               │
    ├─ connect()                    │
    │   └─ starts recv_loop()       │
    │       (detached as thread)    │
    │                     ▼─────────┘
    │                     │
    │                     │ (continuous loop)
    │                     ├─ recv() from socket
    │                     ├─ lock queue_mutex
    │                     ├─ push to message_queue
    │                     ├─ unlock queue_mutex
    │                     └─ sleep 50ms
    │                               │
    ├─ send_message(msg)     (async)
    │   ├─ send() to socket          │
    │   └─ return success            │
    │                               │
    ├─ receive_message()            │
    │   ├─ lock queue_mutex         │
    │   ├─ pop from queue    ◄──────┘
    │   ├─ unlock queue_mutex
    │   └─ return message
    │
    └─ disconnect()
        ├─ set running_ = false
        ├─ join recv_thread
        └─ cleanup sockets
```

### Mutex Protection

```
ThreadSafe Queue:
┌─────────────────────────────┐
│  std::queue<std::string>    │ (message_queue_)
├─────────────────────────────┤
│  std::mutex queue_mutex_    │
├─────────────────────────────┤
│  Operations:                │
│  - push (receiver thread)   │ lock
│  - pop (main thread)        │ lock
│  - size (both threads)      │ lock
└─────────────────────────────┘
```

## Data Flow Diagram

```
┌─────────────┐                 ┌─────────────┐
│ CLI Client  │                 │ GUI Client  │
└─────┬───────┘                 └─────┬───────┘
      │                               │
      ├─ stdin ─────┐         ┌───────┤ UI Events
      │             │         │       │
      ▼             ▼         ▼       ▼
   ┌────────────────────────────────────┐
   │       ChatClient (Network)         │
   ├────────────────────────────────────┤
   │  Socket     │ Queue      │ Thread  │
   │  SOCKET ───► message_Q ─► recv    │
   └────────────────────────────────────┘
      ▲             │
      │             │ messages
      └─────────────┤
          send()    │
                    ▼
              [Network Stack]
                    │
                    ▼
              [Server Broadcast]
                    │
                    └─────────► [Other Clients]
```

## Class Diagrams

### ChatClient Class

```
ChatClient
├─ Private Members:
│  ├─ SOCKET socket_
│  ├─ std::atomic<bool> connected_
│  ├─ std::atomic<bool> running_
│  ├─ std::queue<std::string> message_queue_
│  ├─ std::mutex queue_mutex_
│  └─ std::unique_ptr<std::thread> recv_thread_
│
├─ Public Methods:
│  ├─ ChatClient()
│  ├─ ~ChatClient()
│  ├─ bool connect(host, port)
│  ├─ void disconnect()
│  ├─ bool is_connected() const
│  ├─ bool send_message(msg) const
│  ├─ bool has_pending_messages() const
│  └─ std::string receive_message()
│
└─ Private Methods:
   ├─ void recv_loop()
   └─ void cleanup()
```

### ChatGui Class

```
ChatGui
├─ Private Members:
│  ├─ std::unique_ptr<ChatClient> client_
│  ├─ std::vector<std::string> chat_log_
│  ├─ char input_buffer_[512]
│  ├─ bool connected_
│  ├─ bool show_connection_status_
│  └─ float scroll_to_bottom_
│
├─ Public Methods:
│  ├─ ChatGui()
│  ├─ ~ChatGui()
│  ├─ bool init(title, width, height)
│  ├─ void shutdown()
│  ├─ bool is_running() const
│  ├─ void render()
│  ├─ void connect(host, port)
│  ├─ void disconnect()
│  └─ bool is_connected() const
│
└─ Private Methods:
   ├─ void render_menu_bar()
   ├─ void render_chat_window()
   ├─ void render_input_area()
   ├─ void handle_incoming_messages()
   └─ void add_chat_message(sender, msg)
```

## State Machine

### ChatClient Connection States

```
┌──────────────────────────────┐
│   DISCONNECTED (initial)     │
│ connected_=false, running_=f │
└────────────────┬─────────────┘
                 │ connect() called
                 ▼
        ┌────────────────────┐
        │  CONNECTING        │
        │  Winsock init      │
        │  socket() created  │
        │  non-blocking mode │
        └────────────┬───────┘
                     │ on success
                     ▼
        ┌────────────────────┐
        │  CONNECTED         │
        │ connected_=true    │
        │ recv_thread active │
        │ running_=true      │
        └─────┬──────────────┘
              │ (running)
              │ ┌─────────────────┐
              │ │ recv_loop()     │
              │ │ Waiting for msg │
              │ └─────────────────┘
              │
         ┌────┴──────────────────┐
         │ disconnect() called   │
         │ socket error occurs   │
         │ server closes conn    │
         ▼
    ┌─────────────────────────┐
    │ DISCONNECTED (cleanup)  │
    │ running_=false          │
    │ thread joined           │
    │ socket closed           │
    └─────────────────────────┘
```

## Performance Characteristics

### Memory Usage
- **ChatClient**: ~1-5 KB (excluding message queue)
- **Message Queue**: ~100 bytes per message + buffer
- **ChatGui**: ~10-20 KB (excluding chat log)
- **Chat Log (1000 msgs)**: ~50-100 KB
- **Total**: ~200-300 KB typical usage

### CPU Usage
- **Idle**: <1% (recv thread sleeps)
- **Sending Message**: <2% spike
- **Receiving Messages**: <1-2% depending on rate
- **GUI Rendering**: ~5-10% at 60 FPS

### Network
- **Header Overhead**: ~20 bytes TCP/IP
- **Per Message Bandwidth**: message size + 20 bytes
- **Latency**: ~1-10ms LAN, higher over internet
- **Throughput**: Limited by network, not app

## Error Handling Strategy

```
Critical Failures:
  ├─ Socket creation fails → Return false, log error
  ├─ Connect fails → Return false, log with error code
  ├─ Receive errors → Mark disconnected, log error code
  └─ Send fails → Return false, log with error code

Recovery:
  ├─ Auto-reconnect: NO (manual via menu)
  ├─ Partial sends: Retried (Winsock handles)
  ├─ Dropped connections: Detected, marked as disconnected
  └─ Queue overflow: Never (unlimited size)

Logging:
  ├─ All errors logged to stderr
  ├─ Format: "[ChatClient] error message"
  ├─ Windows error codes included
  └─ User notified via GUI status messages
```

## Security Implications

⚠️ **Not Production Safe**

Vulnerabilities:
- No authentication (anyone can join)
- No encryption (plaintext over network)
- No input validation (no limits on message size)
- No rate limiting (DoS possible)
- No user identification (can spoof sender)

Mitigations needed:
- SSL/TLS encryption
- User authentication
- Message size limits
- Rate limiting per client
- Message validation
- Logging and monitoring

---

**This architecture ensures clean separation of concerns, thread safety, and maintainability.**
