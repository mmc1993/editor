#include "ui_class.h"
#include "../ui_state/ui_state.h"
#include "imgui_impl_glfw.h"

std::vector<UIClass*> UIClass::GetChildren(UITypeEnum type)
{
    std::vector<UIClass *> result;
    std::copy_if(_children.begin(), _children.end(), std::back_inserter(result), 
        [type](const UIClass * object)
        {
            return object->GetType() == type;
        });
    return std::move(result);
}

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

void UIClass::Render(float dt)
{
    if (OnEnter())
    {
        OnRender(dt);
        std::for_each(_children.begin(), _children.end(),
                      std::bind(&UIClass::Render, 
                      std::placeholders::_1, dt));
    }
    OnLeave();
}

void UIClass::ApplyLayout()
{
}

//--------------------------------------------------------------------------------
//  Window
//--------------------------------------------------------------------------------
void UIClassWindow::ApplyLayout()
{
    auto layouts = GetChildren(UITypeEnum::kLAYOUT);

    //  确定自身大小
    UIClass::ApplyLayout();
}

void UIClassWindow::OnUpdate(float dt)
{
}

void UIClassWindow::OnRender(float dt)
{
}

bool UIClassWindow::OnEnter()
{
    auto state = GetState<UIStateWindow>();
    ImVec2 move = ImVec2(GET_DATA(state.mData, Move).x, GET_DATA(state.mData, Move).y);
    ImVec2 size = ImVec2(GET_DATA(state.mData, Move).z, GET_DATA(state.mData, Move).w);
    size_t flag = 0;
    if (GET_DATA(state.mData, WindowIsFullScreen))
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
        if (GET_DATA(state.mData, WindowIsNav))        { flag |= ImGuiWindowFlags_NoNav; }
        if (GET_DATA(state.mData, WindowIsMove))       { flag |= ImGuiWindowFlags_NoMove; }
        if (GET_DATA(state.mData, WindowIsSize))       { flag |= ImGuiWindowFlags_NoResize; }
        if (GET_DATA(state.mData, WindowIsTitleBar))   { flag |= ImGuiWindowFlags_NoTitleBar; }
        if (GET_DATA(state.mData, WindowIsCollapse))   { flag |= ImGuiWindowFlags_NoCollapse; }
        if (GET_DATA(state.mData, WindowIsScrollBar))  { flag |= ImGuiWindowFlags_NoScrollbar; }
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

    return ImGui::Begin(GET_DATA(state.mData, Name).c_str(), nullptr, flag);
}

void UIClassWindow::OnLeave()
{
    ImGui::PopStyleVar();
    ImGui::End();
}

//--------------------------------------------------------------------------------
//  Layout
//--------------------------------------------------------------------------------
void UIClassLayout::ApplyLayout()
{
}

void UIClassLayout::OnUpdate(float dt)
{
}

void UIClassLayout::OnRender(float dt)
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
    if (GET_DATA(state.mData, WindowIsNav)) { flag |= ImGuiWindowFlags_NoNav; }
    if (GET_DATA(state.mData, WindowIsMove)) { flag |= ImGuiWindowFlags_NoMove; }
    if (GET_DATA(state.mData, WindowIsSize)) { flag |= ImGuiWindowFlags_NoResize; }
    if (GET_DATA(state.mData, WindowIsTitleBar)) { flag |= ImGuiWindowFlags_NoTitleBar; }
    if (GET_DATA(state.mData, WindowIsCollapse)) { flag |= ImGuiWindowFlags_NoCollapse; }
    if (GET_DATA(state.mData, WindowIsScrollBar)) { flag |= ImGuiWindowFlags_NoScrollbar; }
    return ImGui::BeginChild(
        GET_DATA(state.mData, Name).c_str(), 
        ImVec2(GET_DATA(state.mData, Move).z, 
               GET_DATA(state.mData, Move).w),
        GET_DATA(state.mData, LayoutIsShowBorder), flag);
}

void UIClassLayout::OnLeave()
{
    ImGui::EndChild();
}