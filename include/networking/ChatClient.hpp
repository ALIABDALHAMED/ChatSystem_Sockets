#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <winsock2.h>
#include <ws2tcpip.h>

/**
 * Thread-safe chat client using Windows Sockets
 * Manages connection, sending, and receiving messages in non-blocking mode
 */
class ChatClient {
public:
    ChatClient();
    ~ChatClient();

    // Connection management
    bool connect(const std::string& host, int port);
    void disconnect();
    bool is_connected() const;

    // Message operations
    bool send_message(const std::string& message);
    bool has_pending_messages() const;
    std::string receive_message();

private:
    SOCKET socket_;
    std::atomic<bool> connected_;
    std::atomic<bool> running_;

    // Thread-safe message queue
    std::queue<std::string> message_queue_;
    mutable std::mutex queue_mutex_;

    // Receive thread
    std::unique_ptr<std::thread> recv_thread_;

    // Internal methods
    void recv_loop();
    void cleanup();

    static constexpr int BUFFER_SIZE = 4096;
    static constexpr int PORT_DEFAULT = 54000;
};
