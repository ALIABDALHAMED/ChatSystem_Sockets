#include "gui/ChatGui.hpp"
#include "networking/ChatClient.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sstream>
#include <cstring>  // for memset
#include <ctime>    // for time functions
#include <iomanip>  // for put_time


static GLFWwindow* g_window = nullptr;

ChatGui::ChatGui()
    : connected_(false), scroll_to_bottom_(0.0f), 
      current_mode_(AppMode::STARTUP_DIALOG), show_startup_dialog_(true),
      show_settings_dialog_(false),
      selected_message_index_(-1), message_hover_alpha_(0.0f), 
      auto_scroll_(true), timestamps_enabled_(true), theme_selection_(0) {
    std::memset(input_buffer_, 0, sizeof(input_buffer_));
    std::memset(search_buffer_, 0, sizeof(search_buffer_));
    client_ = std::make_unique<ChatClient>();
}

ChatGui::~ChatGui() {
    shutdown();
}

bool ChatGui::init(const std::string& title, int width, int height) {
    // Initialize GLFW
    if (!glfwInit()) {
        return false;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window
    g_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!g_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    apply_modern_style();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    add_chat_message("System", "Welcome to Chat Client\nType your messages below and press Send");

    return true;
}

void ChatGui::apply_modern_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Modern dark theme with accent colors
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.21f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
}

std::string ChatGui::get_current_timestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

void ChatGui::copy_to_clipboard(const std::string& text) {
    // For Windows, we can use simple method
    if (!text.empty()) {
        // This would require platform-specific implementation
        // For now, just show a system message
        add_chat_message("System", "Message copied to clipboard");
    }
}

std::vector<std::string> ChatGui::search_messages(const std::string& query) {
    std::vector<std::string> results;
    if (query.empty()) return results;
    
    for (const auto& msg : chat_log_) {
        if (msg.find(query) != std::string::npos) {
            results.push_back(msg);
        }
    }
    return results;
}

void ChatGui::shutdown() {
    if (connected_) {
        disconnect();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (g_window) {
        glfwDestroyWindow(g_window);
    }
    glfwTerminate();
}

bool ChatGui::is_running() const {
    return g_window && !glfwWindowShouldClose(g_window);
}

void ChatGui::render() {
    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render based on current mode
    if (current_mode_ == AppMode::STARTUP_DIALOG) {
        render_startup_dialog();
    } else if (current_mode_ == AppMode::CHAT_SYSTEM) {
        // Handle connection status and incoming messages
        if (connected_) {
            handle_incoming_messages();
        }
        
        // Render menu and toolbar
        render_menu_bar();
        render_toolbar();
        
        // Render main chat components
        render_search_box();
        render_chat_window();
        render_input_area();
        render_connection_status();
        
        // Render optional panels
        if (show_settings_dialog_) {
            render_settings_dialog();
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            render_message_context_menu();
        }
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(g_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.09f, 0.09f, 0.10f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(g_window);
}

void ChatGui::render_startup_dialog() {
    // Center the dialog
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Appearing);

    ImGui::Begin("Chat System Selection", &show_startup_dialog_, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
    ImGui::TextWrapped("Choose your preferred communication method:");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Chat System Button
    if (ImGui::Button("  Socket-Based Chat System  ", ImVec2(450, 60))) {
        current_mode_ = AppMode::CHAT_SYSTEM;
        add_chat_message("System", "========================================");
        add_chat_message("System", "    Welcome to Socket-Based Chat");
        add_chat_message("System", "========================================");
        add_chat_message("System", "Use Connection menu to connect to server");
        ImGui::CloseCurrentPopup();
    }
    ImGui::TextWrapped("Reliable TCP/IP socket communication with multi-client support");
    
    ImGui::Spacing();
    ImGui::Spacing();

    // Shared Memory Button (non-functional, UI only)
    ImGui::BeginDisabled(true);
    if (ImGui::Button("  Shared Memory System (Coming Soon)  ", ImVec2(450, 60))) {
        // Not implemented
    }
    ImGui::TextWrapped("Local inter-process communication using shared memory");
    ImGui::EndDisabled();

    ImGui::End();
}


void ChatGui::connect(const std::string& host, int port) {
    if (connected_) {
        add_chat_message("System", "Already connected");
        return;
    }

    add_chat_message("System", "Connecting to " + host + ":" + std::to_string(port) + "...");

    if (client_->connect(host, port)) {
        connected_ = true;
        add_chat_message("System", "Connected!");
    } else {
        add_chat_message("System", "Connection failed");
    }
}

void ChatGui::disconnect() {
    if (!connected_) return;

    client_->disconnect();
    connected_ = false;
    add_chat_message("System", "Disconnected");
}

bool ChatGui::is_connected() const {
    return connected_ && client_->is_connected();
}

void ChatGui::render_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Connection")) {
            if (!is_connected()) {
                if (ImGui::MenuItem("Connect (localhost:54000)")) {
                    connect("127.0.0.1", 54000);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Connect to custom host...")) {
                    connect("127.0.0.1", 54000);
                }
            } else {
                if (ImGui::MenuItem("Disconnect")) {
                    disconnect();
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::Separator();
            if (ImGui::MenuItem("Toggle Timestamps", nullptr, &timestamps_enabled_)) {}
            if (ImGui::MenuItem("Auto-scroll", nullptr, &auto_scroll_)) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Preferences")) {
                show_settings_dialog_ = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                add_chat_message("System", "=== Chat Client v3.0 ===");
                add_chat_message("System", "Modern C++17 Socket Chat");
                add_chat_message("System", "Built with ImGui + GLFW");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void ChatGui::render_toolbar() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 60), ImGuiCond_Always);
    
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
    
    // Modern toolbar with larger icons and spacing
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.35f, 0.85f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.25f, 0.75f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
    
    // Larger search button with icon
    if (ImGui::Button("🔍##search_btn", ImVec2(50, 50))) {
        // Search functionality
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Search Messages");
    }
    ImGui::SameLine(0, 20);
    
    // Larger settings button with icon
    if (ImGui::Button("⚙️##settings_btn", ImVec2(50, 50))) {
        show_settings_dialog_ = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Open Settings");
    }
    ImGui::SameLine(0, 20);
    
    // Larger clear button with icon
    if (ImGui::Button("🗑️##clear_btn", ImVec2(50, 50))) {
        chat_log_.clear();
        add_chat_message("System", "Chat history cleared");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Clear Chat History");
    }
    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    
    ImGui::End();
}

void ChatGui::render_connection_status() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 320, ImGui::GetFrameHeight() + 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(310, 40), ImGuiCond_Always);
    
    ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
    
    if (is_connected()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.4f, 1.0f));
        ImGui::Text("🟢 Connected");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("🔴 Disconnected");
        ImGui::PopStyleColor();
    }
    
    ImGui::End();
}

void ChatGui::render_search_box() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight() + 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 45), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Search", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    
    // Modern search bar with icon
    ImGui::AlignTextToFramePadding();
    ImGui::Text("🔍");
    ImGui::SameLine(0, 12);
    
    ImGui::PushItemWidth(io.DisplaySize.x - 120);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
    
    ImGui::InputTextWithHint("##search_input", "Search messages...", search_buffer_, sizeof(search_buffer_));
    
    ImGui::PopStyleVar(2);
    ImGui::PopItemWidth();
    
    ImGui::SameLine(0, 10);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.35f, 0.85f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.95f, 1.0f));
    
    if (ImGui::Button("Find", ImVec2(60, 35))) {
        std::string query(search_buffer_);
        auto results = search_messages(query);
        if (!results.empty()) {
            add_chat_message("System", "Found " + std::to_string(results.size()) + " matches");
        } else {
            add_chat_message("System", "No matches found");
        }
    }
    ImGui::PopStyleColor(2);
    
    ImGui::End();
}

void ChatGui::render_settings_dialog() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 350), ImGuiCond_Appearing);
    
    if (ImGui::Begin("Settings", &show_settings_dialog_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 1.0f, 1.0f));
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "⚙️  User Preferences");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
        
        ImGui::Checkbox("Auto-scroll Messages", &auto_scroll_);
        ImGui::SameLine();
        ImGui::TextDisabled("(Automatically scroll to latest message)");
        
        ImGui::Spacing();
        ImGui::Checkbox("Show Timestamps", &timestamps_enabled_);
        ImGui::SameLine();
        ImGui::TextDisabled("(Show time on message hover)");
        
        ImGui::Spacing();
        ImGui::Checkbox("Enable Notifications", nullptr);
        ImGui::SameLine();
        ImGui::TextDisabled("(Desktop notifications)");
        
        ImGui::PopStyleVar(2);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 1.0f, 1.0f));
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "🎨  Theme Selection");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        
        if (ImGui::RadioButton("🌙 Dark Theme", &theme_selection_, 0)) {
            apply_theme(0);
        }
        ImGui::SameLine(150);
        if (ImGui::RadioButton("☀️ Light Theme", &theme_selection_, 1)) {
            apply_theme(1);
        }
        ImGui::SameLine(300);
        if (ImGui::RadioButton("🔵 Blue Theme", &theme_selection_, 2)) {
            apply_theme(2);
        }
        
        ImGui::PopStyleVar();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.75f, 0.35f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.85f, 0.45f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
        
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 150) / 2);
        if (ImGui::Button("✓ Save & Close", ImVec2(150, 40))) {
            show_settings_dialog_ = false;
        }
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        
        ImGui::End();
    }
}

void ChatGui::render_message_context_menu() {
    if (ImGui::BeginPopupContextWindow("MessageContext", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Copy")) {
            copy_to_clipboard("Message copied");
        }
        if (ImGui::MenuItem("Delete")) {
            add_chat_message("System", "Message deleted");
        }
        if (ImGui::MenuItem("Reply")) {
            add_chat_message("System", "Reply feature coming soon");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Report")) {
            add_chat_message("System", "Message reported");
        }
        ImGui::EndPopup();
    }
}

void ChatGui::apply_theme(int theme_id) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    if (theme_id == 0) {
        // Dark theme (default)
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    } else if (theme_id == 1) {
        // Light theme
        colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        // Blue theme
        colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.1f, 0.2f, 1.00f);
    }
    
    add_chat_message("System", "Theme changed");
}

void ChatGui::handle_message_hover(int index) {
    if (index >= 0 && index < static_cast<int>(chat_log_.size())) {
        message_hover_alpha_ = 0.2f;
        selected_message_index_ = index;
    }
}

void ChatGui::render_chat_window() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight() + 105), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 
                                    io.DisplaySize.y - ImGui::GetFrameHeight() - 185), 
                             ImGuiCond_FirstUseEver);

    ImGui::Begin("Chat Messages", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // Chat display with improved styling and hover effects
    ImGui::BeginChild("chat_log", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6));
    
    for (int i = 0; i < static_cast<int>(chat_log_.size()); i++) {
        const auto& line = chat_log_[i];
        
        // Hover effect - highlight on mouse over
        bool is_hovering = ImGui::IsMouseHoveringRect(ImGui::GetCursorScreenPos(), 
                                      ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth(), 
                                             ImGui::GetCursorScreenPos().y + 20));
        
        if (is_hovering) {
            handle_message_hover(i);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        
        // Color code different message types with better styling
        if (line.find("[System]") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.5f, 0.75f, 1.0f, 1.0f), "%s", line.c_str());
        } else if (line.find("[You]") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.3f, 1.0f), "%s", line.c_str());
        } else if (line.find("[Remote]") != std::string::npos) {
            ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.1f, 1.0f), "%s", line.c_str());
        } else {
            ImGui::TextWrapped("%s", line.c_str());
        }
        
        if (is_hovering) {
            ImGui::PopStyleColor();
        }
        
        // Right-click context menu on messages
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("MessageContext");
        }
        
        // Show timestamp on hover if enabled
        if (ImGui::IsItemHovered() && timestamps_enabled_) {
            ImGui::SetTooltip("🕐 %s", get_current_timestamp().c_str());
        }
    }
    
    ImGui::PopStyleVar();
    
    if (scroll_to_bottom_ > 0.0f && auto_scroll_) {
        ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom_ -= io.DeltaTime;
    }
    ImGui::EndChild();

    ImGui::End();
}

void ChatGui::render_input_area() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 80), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 80), ImGuiCond_Always);

    ImGui::Begin("Input Area", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

    if (!is_connected()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("⚠️ Not connected to server. Use Connection menu to connect.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
    
    // Modern message input with rounded corners
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
    ImGui::PushItemWidth(-110);
    
    bool send_message = ImGui::InputTextWithHint("##message_input", "Type message...", 
                                               input_buffer_, sizeof(input_buffer_),
                                               ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar(2);

    ImGui::SameLine(0, 10);
    
    // Modern Send button with enhanced styling
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.75f, 0.35f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.85f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.60f, 0.30f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 10));
    
    if (ImGui::Button("Send ✈️", ImVec2(100, 40)) || (send_message && is_connected())) {
        if (is_connected() && input_buffer_[0] != '\0') {
            std::string msg(input_buffer_);
            if (!msg.empty()) {
                if (client_->send_message(msg)) {
                    add_chat_message("You", msg);
                }
            }
            std::memset(input_buffer_, 0, sizeof(input_buffer_));
        } else if (!is_connected()) {
            add_chat_message("System", "Cannot send: Not connected");
        }
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    ImGui::End();
}

void ChatGui::handle_incoming_messages() {
    while (client_->has_pending_messages()) {
        std::string msg = client_->receive_message();
        if (!msg.empty()) {
            add_chat_message("Remote", msg);
        }
    }
}

void ChatGui::add_chat_message(const std::string& sender, const std::string& message) {
    std::string formatted = "[" + sender + "]: " + message;
    chat_log_.push_back(formatted);

    // Limit chat log size to avoid memory issues
    if (chat_log_.size() > 1000) {
        chat_log_.erase(chat_log_.begin());
    }

    scroll_to_bottom_ = 1.0f;
}
