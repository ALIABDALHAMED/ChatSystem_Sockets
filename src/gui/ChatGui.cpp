#include "gui/ChatGui.hpp"
#include "networking/ChatClient.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sstream>
#include <cstring>  // for memset


static GLFWwindow* g_window = nullptr;

ChatGui::ChatGui()
    : connected_(false), show_connection_status_(true), scroll_to_bottom_(0.0f) {
    std::memset(input_buffer_, 0, sizeof(input_buffer_));
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

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    add_chat_message("System", "Welcome to Chat Client\nType your messages below and press Send");

    return true;
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

    // Handle connection status and incoming messages
    if (connected_) {
        handle_incoming_messages();
    }

    // Render UI
    render_menu_bar();
    render_chat_window();
    render_input_area();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(g_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(g_window);
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
            } else {
                if (ImGui::MenuItem("Disconnect")) {
                    disconnect();
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                add_chat_message("System", "Chat Client v1.0 - C++17 with ImGui");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void ChatGui::render_chat_window() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 
                                    ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight() - 80), 
                             ImGuiCond_FirstUseEver);

    ImGui::Begin("Chat Log", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // Chat display
    ImGui::BeginChild("chat_log", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& line : chat_log_) {
        ImGui::TextWrapped("%s", line.c_str());
    }
    if (scroll_to_bottom_ > 0.0f) {
        ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom_ -= ImGui::GetIO().DeltaTime;
    }
    ImGui::EndChild();

    ImGui::End();
}

void ChatGui::render_input_area() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 50), ImGuiCond_Always);

    ImGui::Begin("Input", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

    if (!is_connected()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not connected");
    }

    ImGui::PushItemWidth(-100);
    bool send_message = ImGui::InputText("##message_input", input_buffer_, sizeof(input_buffer_),
                                         ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();

    ImGui::SameLine();

    if (ImGui::Button("Send", ImVec2(90, 0)) || (send_message && is_connected())) {
        if (is_connected() && input_buffer_[0] != '\0') {
            std::string msg(input_buffer_);
            if (!msg.empty()) {
                if (client_->send_message(msg)) {
                    add_chat_message("You", msg);
                }
            }
            std::memset(input_buffer_, 0, sizeof(input_buffer_));
        }
    }

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
