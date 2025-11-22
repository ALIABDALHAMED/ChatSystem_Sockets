#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declaration
class ChatClient;

/**
 * ImGui-based chat client GUI
 * Manages rendering, user input, and integration with ChatClient
 */
class ChatGui {
public:
    ChatGui();
    ~ChatGui();

    // Initialize and cleanup
    bool init(const std::string& title, int width, int height);
    void shutdown();

    // Main loop
    bool is_running() const;
    void render();

    // Chat operations
    void connect(const std::string& host, int port);
    void disconnect();
    bool is_connected() const;

private:
    std::unique_ptr<ChatClient> client_;
    std::vector<std::string> chat_log_;
    char input_buffer_[512];
    bool connected_;
    bool show_connection_status_;
    float scroll_to_bottom_;

    // Internal methods
    void render_menu_bar();
    void render_chat_window();
    void render_input_area();
    void handle_incoming_messages();
    void add_chat_message(const std::string& sender, const std::string& message);
};
