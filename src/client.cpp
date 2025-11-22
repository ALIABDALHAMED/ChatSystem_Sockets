#include "networking/ChatClient.hpp"
#include <iostream>
#include <thread>

int main() {
    ChatClient client;

    std::cout << "Connecting to server...\n";
    if (!client.connect("127.0.0.1", 54000)) {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }

    std::cout << "Connected! Type messages (Ctrl+C to exit):\n";

    // Start receive display thread
    std::atomic<bool> running(true);
    std::thread display_thread([&client, &running]() {
        while (running) {
            if (client.has_pending_messages()) {
                std::string msg = client.receive_message();
                if (!msg.empty()) {
                    std::cout << "[remote] " << msg;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    // Main input loop
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (!client.send_message(line)) {
            std::cerr << "Failed to send message\n";
            break;
        }
    }

    running = false;
    if (display_thread.joinable()) {
        display_thread.join();
    }

    client.disconnect();
    return 0;
}
