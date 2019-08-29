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
        [type](const UIClass * object) { return object->GetType() == type; });
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
    if (GetParent() != nullptr)
    {
        ApplyLayout();
    }
    
    OnUpdate(dt);
    std::for_each(_children.begin(), _children.end(),
                  std::bind(&UIClass::Update,
                  std::placeholders::_1, dt));

    //  刷新备份数据
    auto & data = GetState<UIState>().mData;
    SetUIData(data, _Move, GetUIData(data, Move));
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

void UIClass::ResetLayout()
{
    OnResetLayout();

    //  初始化备份数据
    auto & data = GetState<UIState>().mData;
    SetUIData(data, _Move, GetUIData(data, Move));

    std::for_each(_children.begin(), _children.end(),
                    std::bind(&UIClass::ResetLayout, 
                    std::placeholders::_1));
}

void UIClass::ApplyLayout()
{
    auto & parentData = GetParent()->GetState<UIState>().mData;
    auto & parentMoveOld = GetUIData(parentData, _Move);
    auto & parentMoveNew = GetUIData(parentData,  Move);
    auto & thisData = GetState<UIState>().mData;

    if (parentMoveOld.z != parentMoveNew.z || 
        parentMoveOld.w != parentMoveNew.w)
    {
        auto move    = GetUIData(thisData, Move);
        auto align   = GetUIData(thisData, Align);
        auto margin  = glm::vec4(move.x, move.y,
                                 move.x + move.z,
                                 move.y + move.w);
        if (align & (int)UIAlignEnum::kCLING_B)
        {
            margin.w += parentMoveNew.w - parentMoveOld.w;
        }
        if (align & (int)UIAlignEnum::kCLING_R)
        {
            margin.z += parentMoveNew.z - parentMoveOld.z;
        }
        if (align & (int)UIAlignEnum::kCENTER_H)
        {
            margin.x = margin.x / parentMoveOld.z * parentMoveNew.z;
            margin.z = margin.z / parentMoveOld.z * parentMoveNew.z;
        }
        if (align & (int)UIAlignEnum::kCENTER_V)
        {
            margin.y = margin.y / parentMoveOld.w * parentMoveNew.w;
            margin.w = margin.w / parentMoveOld.w * parentMoveNew.w;
        }

        if (!(align & (int)UIAlignEnum::kCLING_T))
        {
            margin.y = margin.w - move.w;
        }
        if (!(align & (int)UIAlignEnum::kCLING_L))
        {
            margin.x = margin.z - move.z;
        }
        move.x = margin.x;
        move.y = margin.y;
        move.z = margin.z - margin.x;
        move.w = margin.w - margin.y;
        SetUIData(thisData, Move, move);
    }
    
    OnApplyLayout();
}

//--------------------------------------------------------------------------------
//  Window
//--------------------------------------------------------------------------------
void UIClassWindow::OnResetLayout()
{
    auto layouts = GetChildren(UITypeEnum::kLAYOUT);
    ASSERT_LOG(!layouts.empty(), "Must Have At Least Layout");

    auto selfMove = GetUIData(layouts.at(0)->GetState<UIState>().mData, Move);
    for (auto layout : layouts)
    {
        auto & move = GetUIData(layout->GetState<UIState>().mData, Move);
        if (move.x < selfMove.x) selfMove.x = move.x;
        if (move.y < selfMove.y) selfMove.y = move.y;
        if (move.x + move.z > selfMove.x + selfMove.z)
            selfMove.z = move.x + move.z - selfMove.x;
        if (move.y + move.w > selfMove.y + selfMove.w)
            selfMove.w = move.y + move.w - selfMove.y;
    }

    for (auto layout : layouts)
    {
        auto & data = layout->GetState<UIState>().mData;
        auto move = GetUIData(data, Move);
        move.x -= selfMove.x;
        move.y -= selfMove.y;
        SetUIData(data, Move, move);
    }

    SetUIData(GetState<UIState>().mData, Move, selfMove);
}

void UIClassWindow::OnApplyLayout()
{
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
    auto & name = GetUIData(state.mData, Name);
    ImVec2 move = ImVec2(GetUIData(state.mData, Move).x, GetUIData(state.mData, Move).y);
    ImVec2 size = ImVec2(GetUIData(state.mData, Move).z, GetUIData(state.mData, Move).w);
    size_t flag = 0;
    if (GetUIData(state.mData, WindowIsFullScreen))
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
        if (!GetUIData(state.mData, WindowIsNav))        { flag |= ImGuiWindowFlags_NoNav; }
        if (!GetUIData(state.mData, WindowIsMove))       { flag |= ImGuiWindowFlags_NoMove; }
        if (!GetUIData(state.mData, WindowIsSize))       { flag |= ImGuiWindowFlags_NoResize; }
        if (!GetUIData(state.mData, WindowIsTitleBar))   { flag |= ImGuiWindowFlags_NoTitleBar; }
        if (!GetUIData(state.mData, WindowIsCollapse))   { flag |= ImGuiWindowFlags_NoCollapse; }
        if (!GetUIData(state.mData, WindowIsScrollBar))  { flag |= ImGuiWindowFlags_NoScrollbar; }
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

    ImGui::SetNextWindowPos(move);
    ImGui::SetNextWindowSize(size);
    return ImGui::Begin(name.empty()? nullptr: name.c_str(), nullptr, flag);
}

void UIClassWindow::OnLeave()
{
    ImGui::PopStyleVar(2);
    ImGui::End();
}

//--------------------------------------------------------------------------------
//  Layout
//--------------------------------------------------------------------------------
void UIClassLayout::OnResetLayout()
{
    auto thisState  = GetState<UIStateLayout>();
    auto thisUp     = GetUIData(thisState.mData, Move).y;
    auto thisDown   = thisUp + GetUIData(thisState.mData, Move).w;
    auto thisLeft   = GetUIData(thisState.mData, Move).x;
    auto thisRight  = thisLeft + GetUIData(thisState.mData, Move).z;
    for (auto layout : GetParent()->GetChildren(UITypeEnum::kLAYOUT))
    {
        if (layout == this) { continue; }
        auto state  = layout->GetState<UIState>();
        auto up     = GetUIData(state.mData, Move).y;
        auto down   = thisUp + GetUIData(state.mData, Move).w;
        auto left   = GetUIData(state.mData, Move).x;
        auto right  = thisLeft + GetUIData(state.mData, Move).z;

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

void UIClassLayout::OnApplyLayout()
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

    size_t flag = 0;
    if (!GetUIData(state.mData, WindowIsNav)) { flag |= ImGuiWindowFlags_NoNav; }
    if (!GetUIData(state.mData, WindowIsMove)) { flag |= ImGuiWindowFlags_NoMove; }
    if (!GetUIData(state.mData, WindowIsSize)) { flag |= ImGuiWindowFlags_NoResize; }
    if (!GetUIData(state.mData, WindowIsTitleBar)) { flag |= ImGuiWindowFlags_NoTitleBar; }
    if (!GetUIData(state.mData, WindowIsCollapse)) { flag |= ImGuiWindowFlags_NoCollapse; }
    if (!GetUIData(state.mData, WindowIsScrollBar)) { flag |= ImGuiWindowFlags_NoScrollbar; }
    
    auto & name = GetUIData(state.mData, Name);
    auto & move = GetUIData(state.mData, Move);
    ImGui::SetCursorPos(ImVec2(move.x,move.y));
    return ImGui::BeginChild(name.c_str(), ImVec2(move.z, move.w),
        GetUIData(state.mData, LayoutIsShowBorder), flag);
}

void UIClassLayout::OnLeave()
{
    ImGui::EndChild();
}