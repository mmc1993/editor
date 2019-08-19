#include "ui_class.h"
#include "../ui_state/ui_state.h"
#include "imgui_impl_glfw.h"

std::vector<UIClass*>& UIClass::GetChildren()
{
    return _children;
}

void UIClass::AddChild(UIClass * child)
{
    _children.push_back(child);
}

void UIClass::DelChild(UIClass * child)
{
    auto it = std::find(_children.begin(), _children.end(), child);
    if (it != _children.end()) { delete *it; _children.erase(it); }
}

void UIClass::ClearChild()
{
    while (!_children.empty())
    {
        delete _children.back();
        _children.pop_back();
    }
}

void UIClass::Update(float dt)
{
    OnUpdate(dt);
    std::for_each(_children.begin(), _children.end(),
                  std::bind(&UIClass::Update,
                  std::placeholders::_1, dt));
}

void UIClass::Draw(float dt)
{
    if (OnEnter())
    {
        OnDraw(dt);
        std::for_each(_children.begin(), _children.end(),
                      std::bind(&UIClass::Draw, 
                      std::placeholders::_1, dt));
    }
    OnLeave();
}

bool UIClass::OnEnter()
{
    switch (GetState<UIState>().mUIType)
    {
    case UITypeEnum::kTREE:
        break;
    case UITypeEnum::kWINDOW:
        {
            auto state = GetState<UIStateWindow>();
            ImVec2 pos;
            ImVec2 size;
            size_t flag = 0;
            if (state.mFullScreen)
            {
                size    = ImGui_ImplGlfw_GetWindowSize();
                pos.x   = 0;
                pos.y   = 0;
                flag    = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove 
                    | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse 
                    | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            }
            else
            {
                pos.x = state.mMove.x;
                pos.y = state.mMove.y;
                size.x = state.mMove.z;
                size.y = state.mMove.w;
            }
            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize(size);
            return ImGui::Begin(state.mName.c_str(), nullptr, ImVec2(state.mMove.z, state.mMove.w), -1.0, flag);
        }
    case UITypeEnum::kDIALOG:
        break;
    case UITypeEnum::kCONTAINER:
        {
            auto state = GetState<UIStateDDContainer>();
            ImGui::SetCursorPos(ImVec2(state.mMove.x, state.mMove.y));
            return ImGui::BeginChild(state.mName.c_str(), ImVec2(state.mMove.z, state.mMove.w));
        }
    }
    ASSERT_LOG(false, "OnEnter Error: {0}.", (int)GetState<UIState>().mUIType);
    return false;
}

void UIClass::OnLeave()
{
    switch (GetState<UIState>().mUIType)
    {
    case UITypeEnum::kTREE:
        break;
    case UITypeEnum::kWINDOW:
        {
            ImGui::PopStyleVar();
            return ImGui::End();
        }
    case UITypeEnum::kDIALOG:
        break;
    case UITypeEnum::kCONTAINER:
        {
            auto state = GetState<UIState>();
            ImVec2 points[] = {
                ImVec2(state.mMove.x                 + 0, state.mMove.y                 + 0),
                ImVec2(state.mMove.x + state.mMove.z - 1, state.mMove.y                 + 0),
                ImVec2(state.mMove.x + state.mMove.z - 1, state.mMove.y + state.mMove.w - 1),
                ImVec2(state.mMove.x                 + 0, state.mMove.y + state.mMove.w - 1),
            };
            ImGui::GetWindowDrawList()->AddLine(points[0], points[1], ImColor(0.75f, 0.75f, 0.75f));
            ImGui::GetWindowDrawList()->AddLine(points[1], points[2], ImColor(0.75f, 0.75f, 0.75f));
            ImGui::GetWindowDrawList()->AddLine(points[2], points[3], ImColor(0.75f, 0.75f, 0.75f));
            ImGui::GetWindowDrawList()->AddLine(points[3], points[0], ImColor(0.75f, 0.75f, 0.75f));
            return ImGui::EndChild();
        }
    }
}

void UIClassWindow::OnUpdate(float dt)
{
}

void UIClassWindow::OnDraw(float dt)
{
}

void UIClassDDContainer::OnUpdate(float dt)
{
}

void UIClassDDContainer::OnDraw(float dt)
{
    auto & state = GetState<UIStateDDContainer>();
    ImGui::Button("aaa");
}

void UIClassButton::OnUpdate(float dt)
{
}

void UIClassButton::OnDraw(float dt)
{
}

