#include "ui_class.h"
#include "../ui_state/ui_state.h"
#include "imgui_impl_glfw.h"

UITypeEnum UIClass::GetType() const
{
    return _type;
}

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
    ASSERT_LOG(child->GetParent() == nullptr, "");
    _children.push_back(child);
    child->_parent = this;
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

UIClass * UIClass::GetParent()
{
    return _parent;
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
    auto & thisMove = GET_DATA(GetState<UIState>().mData, Move);
    for (auto child : GetChildren())
    {
        auto & move  = GET_DATA(child->GetState<UIState>().mData, Move);
        glm::vec4 margin = {
            move.x, move.y,
            thisMove.z - move.x + move.z,
            thisMove.w - move.y + move.w 
        };
        SET_DATA(child->GetState<UIState>().mData, Margin, margin);
    }
}

//--------------------------------------------------------------------------------
//  Window
//--------------------------------------------------------------------------------
void UIClassWindow::ApplyLayout()
{
    auto layouts = GetChildren(UITypeEnum::kLAYOUT);
    ASSERT_LOG(!layouts.empty(), "Must Have At Least Layout");

    auto selfMove = GET_DATA(layouts.at(0)->GetState<UIState>().mData, Move);
    for (auto layout : layouts)
    {
        auto & move = GET_DATA(layout->GetState<UIState>().mData, Move);
        if (move.x < selfMove.x) selfMove.x = move.x;
        if (move.y < selfMove.y) selfMove.y = move.y;
        if (move.x + move.z > selfMove.x + selfMove.z)
            selfMove.z = move.x + move.z - selfMove.x;
        if (move.y + move.w > selfMove.y + selfMove.w)
            selfMove.w = move.y + move.w - selfMove.y;
    }

    SET_DATA(GetState<UIState>().mData, Move, selfMove);

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
    auto thisState  = GetState<UIStateLayout>();
    auto thisUp     = GET_DATA(thisState.mData, Move).y;
    auto thisDown   = thisUp + GET_DATA(thisState.mData, Move).w;
    auto thisLeft   = GET_DATA(thisState.mData, Move).x;
    auto thisRight  = thisLeft + GET_DATA(thisState.mData, Move).z;
    for (auto layout : GetParent()->GetChildren(UITypeEnum::kLAYOUT))
    {
        if (layout == this) { continue; }
        auto state  = layout->GetState<UIState>();
        auto up     = GET_DATA(state.mData, Move).y;
        auto down   = thisUp + GET_DATA(state.mData, Move).w;
        auto left   = GET_DATA(state.mData, Move).x;
        auto right  = thisLeft + GET_DATA(state.mData, Move).z;

        if      (thisUp == down)    { thisState.mLayoutInfo.mLinks[(size_t)DirectEnum::kU].push_back(layout); }
        else if (thisDown == up)    { thisState.mLayoutInfo.mLinks[(size_t)DirectEnum::kD].push_back(layout); }
        else if (thisLeft == right) { thisState.mLayoutInfo.mLinks[(size_t)DirectEnum::kL].push_back(layout); }
        else if (thisRight == left) { thisState.mLayoutInfo.mLinks[(size_t)DirectEnum::kR].push_back(layout); }

        if (thisUp == up)       { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kU].emplace_back(DirectEnum::kU, layout); }
        if (thisUp == down)     { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kU].emplace_back(DirectEnum::kD, layout); }
        if (thisDown == down)   { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kD].emplace_back(DirectEnum::kD, layout); }
        if (thisDown == up)     { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kD].emplace_back(DirectEnum::kU, layout); }
        if (thisLeft == left)   { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kL].emplace_back(DirectEnum::kL, layout); }
        if (thisLeft == right)  { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kL].emplace_back(DirectEnum::kR, layout); }
        if (thisRight == right) { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kR].emplace_back(DirectEnum::kR, layout); }
        if (thisRight == left)  { thisState.mLayoutInfo.mEdges[(size_t)DirectEnum::kR].emplace_back(DirectEnum::kL, layout); }
    }
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