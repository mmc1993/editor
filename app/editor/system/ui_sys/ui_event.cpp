#include "ui_event.h"

std::vector<int> UIEvent::Key::Hotkeys = {
    GLFW_KEY_ENTER,
    GLFW_KEY_DELETE,
};

int UIEvent::CheckStateKey()
{
    int flag = 0;
    if (ImGui::GetIO().KeyAlt) { flag |= 1; }
    if (ImGui::GetIO().KeyCtrl) { flag |= 2; }
    if (ImGui::GetIO().KeyShift) { flag |= 4; }
    return flag;
}
