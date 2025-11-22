#include "gui/ChatGui.hpp"

int main() {
    ChatGui gui;

    if (!gui.init("Chat Client", 1000, 700)) {
        return 1;
    }

    // Auto-connect to localhost
    gui.connect("127.0.0.1", 54000);

    // Main render loop
    while (gui.is_running()) {
        gui.render();
    }

    gui.shutdown();
    return 0;
}
