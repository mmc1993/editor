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

void UIClassWindow::OnUpdate(float dt)
{
}

void UIClassWindow::OnDraw(float dt)
{
}

bool UIClassWindow::OnEnter()
{
    auto state = GetState<UIStateWindow>();
    ImVec2 move = ImVec2(state.mMove.x, state.mMove.y);
    ImVec2 size = ImVec2(state.mMove.z, state.mMove.w);
    size_t flag = 0;
    if (__GET_USER_DATA(state.mUserData, __WindowIsFullScreen))
    {
        size = ImGui_ImplGlfw_GetWindowSize();
        move.x = 0;
        move.y = 0;
        flag = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;
    }
    else
    {
        if (__GET_USER_DATA(state.mUserData, __WindowIsNav))        { flag |= ImGuiWindowFlags_NoNav; }
        if (__GET_USER_DATA(state.mUserData, __WindowIsMove))       { flag |= ImGuiWindowFlags_NoMove; }
        if (__GET_USER_DATA(state.mUserData, __WindowIsSize))       { flag |= ImGuiWindowFlags_NoResize; }
        if (__GET_USER_DATA(state.mUserData, __WindowIsTitleBar))   { flag |= ImGuiWindowFlags_NoTitleBar; }
        if (__GET_USER_DATA(state.mUserData, __WindowIsCollapse))   { flag |= ImGuiWindowFlags_NoCollapse; }
        if (__GET_USER_DATA(state.mUserData, __WindowIsScrollBar))  { flag |= ImGuiWindowFlags_NoScrollbar; }
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    return ImGui::Begin(state.mName.c_str(), nullptr, flag);
}

void UIClassWindow::OnLeave()
{
    ImGui::PopStyleVar();
    ImGui::End();
}

void UIClassLayout::OnUpdate(float dt)
{
}

void UIClassLayout::OnDraw(float dt)
{
}

bool UIClassLayout::OnEnter()
{
    auto state = GetState<UIStateLayout>();
    //ImVec2 points[] = {
    //    ImVec2(state.mMove.x + 0, state.mMove.y + 0),
    //    ImVec2(state.mMove.x + state.mMove.z - 1, state.mMove.y + 0),
    //    ImVec2(state.mMove.x + state.mMove.z - 1, state.mMove.y + state.mMove.w - 1),
    //    ImVec2(state.mMove.x + 0, state.mMove.y + state.mMove.w - 1),
    //};
    //ImGui::GetWindowDrawList()->AddLine(points[0], points[1], ImColor(0.75f, 0.75f, 0.75f));
    //ImGui::GetWindowDrawList()->AddLine(points[1], points[2], ImColor(0.75f, 0.75f, 0.75f));
    //ImGui::GetWindowDrawList()->AddLine(points[2], points[3], ImColor(0.75f, 0.75f, 0.75f));
    //ImGui::GetWindowDrawList()->AddLine(points[3], points[0], ImColor(0.75f, 0.75f, 0.75f));

    size_t flag = 0;
    if (__GET_USER_DATA(state.mUserData, __WindowIsNav)) { flag |= ImGuiWindowFlags_NoNav; }
    if (__GET_USER_DATA(state.mUserData, __WindowIsMove)) { flag |= ImGuiWindowFlags_NoMove; }
    if (__GET_USER_DATA(state.mUserData, __WindowIsSize)) { flag |= ImGuiWindowFlags_NoResize; }
    if (__GET_USER_DATA(state.mUserData, __WindowIsTitleBar)) { flag |= ImGuiWindowFlags_NoTitleBar; }
    if (__GET_USER_DATA(state.mUserData, __WindowIsCollapse)) { flag |= ImGuiWindowFlags_NoCollapse; }
    if (__GET_USER_DATA(state.mUserData, __WindowIsScrollBar)) { flag |= ImGuiWindowFlags_NoScrollbar; }
    return ImGui::BeginChild(state.mName.c_str(),  ImVec2(state.mMove.z, state.mMove.w), 
                             __GET_USER_DATA(state.mUserData, __LayoutIsShowBorder), flag);
}
