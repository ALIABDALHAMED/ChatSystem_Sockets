#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declaration
class ChatClient;

/**
 * Modern ImGui-based chat client GUI
 * Features: Clean layout, startup dialog, real-time messaging
 */
class ChatGui {
public:
    // Application modes
    enum class AppMode {
        STARTUP_DIALOG,      // Initial mode - choose app
        CHAT_SYSTEM,         // Chat system active
        SHARED_MEMORY,       // Placeholder (not implemented)
        EXITING
    };

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

    // Get current mode
    AppMode get_mode() const { return current_mode_; }

private:
    std::unique_ptr<ChatClient> client_;
    std::vector<std::string> chat_log_;
    char input_buffer_[512];
    char search_buffer_[256];
    bool connected_;
    float scroll_to_bottom_;
    AppMode current_mode_;
    bool show_startup_dialog_;
    bool show_settings_dialog_;
    int selected_message_index_;
    float message_hover_alpha_;
    bool auto_scroll_;
    bool timestamps_enabled_;
    int theme_selection_;

    // Rendering methods
    void render_startup_dialog();
    void render_chat_system();
    void render_menu_bar();
    void render_toolbar();
    void render_chat_window();
    void render_input_area();
    void render_connection_status();
    void render_settings_dialog();
    void render_search_box();
    void render_message_context_menu();
    
    // Message handling
    void handle_incoming_messages();
    void add_chat_message(const std::string& sender, const std::string& message);
    void handle_message_hover(int index);
    
    // UI Styling
    void apply_modern_style();
    void apply_theme(int theme_id);
    
    // Utility methods
    std::string get_current_timestamp();
    void copy_to_clipboard(const std::string& text);
    std::vector<std::string> search_messages(const std::string& query);
};

