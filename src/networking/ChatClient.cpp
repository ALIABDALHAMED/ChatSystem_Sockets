#include "networking/ChatClient.hpp"
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

ChatClient::ChatClient()
    : socket_(INVALID_SOCKET), connected_(false), running_(false) {
}

ChatClient::~ChatClient() {
    disconnect();
}

bool ChatClient::connect(const std::string& host, int port) {
    if (connected_) return true;

    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[ChatClient] WSAStartup failed\n";
        return false;
    }

    // Create socket (initially blocking for connect)
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET) {
        std::cerr << "[ChatClient] socket() failed\n";
        WSACleanup();
        return false;
    }

    // Connect to server (BEFORE making non-blocking)
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) != 1) {
        std::cerr << "[ChatClient] inet_pton() failed for host: " << host << "\n";
        closesocket(socket_);
        WSACleanup();
        return false;
    }

    std::cerr << "[ChatClient] Attempting to connect to " << host << ":" << port << "\n";
    
    if (::connect(socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        std::cerr << "[ChatClient] connect() failed with error: " << err << "\n";
        closesocket(socket_);
        WSACleanup();
        return false;
    }

    std::cerr << "[ChatClient] Connected successfully to " << host << ":" << port << "\n";

    // NOW set socket to non-blocking (after successful connect)
    u_long mode = 1;
    if (ioctlsocket(socket_, FIONBIO, &mode) == SOCKET_ERROR) {
        std::cerr << "[ChatClient] ioctlsocket() failed\n";
        closesocket(socket_);
        WSACleanup();
        return false;
    }

    connected_ = true;
    running_ = true;

    // Start receive thread AFTER connection is confirmed
    recv_thread_ = std::make_unique<std::thread>(&ChatClient::recv_loop, this);

    return true;
}

void ChatClient::disconnect() {
    if (!connected_) return;

    running_ = false;
    connected_ = false;

    if (recv_thread_ && recv_thread_->joinable()) {
        recv_thread_->join();
    }

    cleanup();
}

bool ChatClient::is_connected() const {
    return connected_;
}

bool ChatClient::send_message(const std::string& message) {
    if (!connected_) {
        std::cerr << "[ChatClient] Not connected, cannot send\n";
        return false;
    }

    std::string msg = message;
    if (!msg.empty() && msg.back() != '\n') {
        msg.push_back('\n');
    }

    int sent = send(socket_, msg.c_str(), (int)msg.size(), 0);
    if (sent == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAECONNRESET || err == WSAECONNABORTED) {
            connected_ = false;
            std::cerr << "[ChatClient] send() - Connection reset by server (error: " << err << ")\n";
        } else {
            std::cerr << "[ChatClient] send() failed: " << err << "\n";
        }
        return false;
    }

    return true;
}

bool ChatClient::has_pending_messages() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return !message_queue_.empty();
}

std::string ChatClient::receive_message() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (message_queue_.empty()) return "";

    std::string msg = message_queue_.front();
    message_queue_.pop();
    return msg;
}

void ChatClient::recv_loop() {
    char buffer[BUFFER_SIZE];

    while (running_) {
        if (!connected_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int n = recv(socket_, buffer, BUFFER_SIZE - 1, 0);

        if (n > 0) {
            buffer[n] = '\0';
            std::string msg(buffer);

            // Queue the message
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                message_queue_.push(msg);
            }
        } else if (n == 0) {
            // Connection closed by server gracefully
            connected_ = false;
            std::cerr << "[ChatClient] Server closed connection\n";
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                message_queue_.push("[SYSTEM] Server disconnected");
            }
            break;
        } else {
            int err = WSAGetLastError();
            // Handle connection errors
            if (err == WSAECONNRESET || err == WSAECONNABORTED) {
                // Connection was forcibly closed
                connected_ = false;
                std::cerr << "[ChatClient] Connection reset by server (error: " << err << ")\n";
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    message_queue_.push("[SYSTEM] Connection lost");
                }
                break;
            } else if (err != WSAEWOULDBLOCK && err != WSAEINTR) {
                // Other error
                std::cerr << "[ChatClient] recv() error: " << err << "\n";
                connected_ = false;
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    message_queue_.push("[SYSTEM] Network error");
                }
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


void ChatClient::cleanup() {
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }
    WSACleanup();
}
