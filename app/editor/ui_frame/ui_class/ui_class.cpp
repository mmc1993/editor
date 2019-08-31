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

UIClass * UIClass::GetRoot()
{
    auto ret = this;
    while (ret->GetParent() != nullptr)
        ret = ret->GetParent();
    return ret;
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
    auto data = GetState<UIState>()->mData;
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
    auto & data = GetState<UIState>()->mData;
    SetUIData(data, _Move, GetUIData(data, Move));

    std::for_each(_children.begin(), _children.end(),
                    std::bind(&UIClass::ResetLayout, 
                    std::placeholders::_1));
}

void UIClass::ApplyLayout()
{
    auto & parentData = GetParent()->GetState<UIState>()->mData;
    auto & parentMoveOld = GetUIData(parentData, _Move);
    auto & parentMoveNew = GetUIData(parentData,  Move);
    auto & thisData = GetState<UIState>()->mData;

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

        if (!(align & (int)UIAlignEnum::kCLING_T)) { margin.y = margin.w - move.w; }
        if (!(align & (int)UIAlignEnum::kCLING_L)) { margin.x = margin.z - move.z; }
        move.x = margin.x;
        move.y = margin.y;
        move.z = margin.z - margin.x;
        move.w = margin.w - margin.y;
        SetUIData(thisData, Move, move);
    }
    
    OnApplyLayout();
}

glm::vec2 UIClass::ToWorldCoord(const glm::vec2 & coord)
{
    auto move = GetUIData(GetState<UIState>()->mData, Move);
    move.x += coord.x;
    move.y += coord.y;

    auto parent = GetParent();
    while (parent != nullptr)
    {
        move.x += GetUIData(parent->GetState<UIState>()->mData, Move).x;
        move.y += GetUIData(parent->GetState<UIState>()->mData, Move).y;
        parent = parent->GetParent();
    }
    return glm::vec2(move.x, move.y);
}

glm::vec4 UIClass::ToLocalCoord(const glm::vec4 & coord)
{
    const auto & world = ToWorldCoord();
    return glm::vec4(coord.x - world.x,
                     coord.y - world.y,
                     coord.z, coord.w);
}

glm::vec2 UIClass::ToLocalCoord(const glm::vec2 & coord)
{
    const auto & world = ToWorldCoord();
    return glm::vec2(coord.x - world.x,
                     coord.y - world.y);
}

//--------------------------------------------------------------------------------
//  Layout
//--------------------------------------------------------------------------------
void UIClassLayout::OnResetLayout()
{
    if (GetParent() == nullptr) { return; }

    auto thisState  = GetState<UIStateLayout>();
    auto thisUp     = GetUIData(thisState->mData, Move).y;
    auto thisDown   = thisUp + GetUIData(thisState->mData, Move).w;
    auto thisLeft   = GetUIData(thisState->mData, Move).x;
    auto thisRight  = thisLeft + GetUIData(thisState->mData, Move).z;
    for (auto layout : GetParent()->GetChildren(UITypeEnum::kLAYOUT))
    {
        if (layout == this) { continue; }
        auto index  = (size_t)~0;
        auto state  = layout->GetState<UIStateLayout>();
        auto up     = GetUIData(state->mData, Move).y;
        auto left   = GetUIData(state->mData, Move).x;
        auto down   = thisUp    + GetUIData(state->mData, Move).w;
        auto right  = thisLeft  + GetUIData(state->mData, Move).z;
        if ((thisUp == up || thisUp == down) && thisState->mJoin[(size_t)DirectEnum::kU].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kU].mOut.push_back(layout);
            auto dir = thisUp == up ? DirectEnum::kU : DirectEnum::kD;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kU;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
        if ((thisDown == down || thisDown == up) && thisState->mJoin[(size_t)DirectEnum::kD].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kD].mOut.push_back(layout);
            auto dir = thisDown == down ? DirectEnum::kD : DirectEnum::kU;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kD;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
        if ((thisLeft == left || thisLeft == right) && thisState->mJoin[(size_t)DirectEnum::kL].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kL].mOut.push_back(layout);
            auto dir = thisLeft == left ? DirectEnum::kL : DirectEnum::kR;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kL;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
        if ((thisRight == right || thisRight == left) && thisState->mJoin[(size_t)DirectEnum::kR].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kR].mOut.push_back(layout);
            auto dir = thisRight == right ? DirectEnum::kR : DirectEnum::kL;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kR;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
    }
}

void UIClassLayout::OnApplyLayout()
{
    auto thisState = GetState<UIStateLayout>();

    for (auto direct = 0; direct != (int)DirectEnum::LENGTH; ++direct)
    {
        auto thisState = GetState<UIStateLayout>();
        if (thisState->mJoin[(int)direct].mIn.first != nullptr)
        {
            const auto & edge = thisState->mJoin[(int)direct].mIn;
            const auto & move = GetUIData(edge.first->GetState<UIState>()->mData, Move);
            switch ((DirectEnum)direct)
            {
            case DirectEnum::kU: SetUIData(thisState->mData, Move, StretchU(edge, move)); break;
            case DirectEnum::kD: SetUIData(thisState->mData, Move, StretchD(edge, move)); break;
            case DirectEnum::kL: SetUIData(thisState->mData, Move, StretchL(edge, move)); break;
            case DirectEnum::kR: SetUIData(thisState->mData, Move, StretchR(edge, move)); break;
            }
        }
    }
}

void UIClassLayout::OnUpdate(float dt)
{
}

void UIClassLayout::OnRender(float dt)
{
    auto thisState =            GetState<UIStateLayout>();
    auto rootState = GetRoot()->GetState<UIStateLayout>();

    if (rootState->mStretchFocus.mObject == nullptr && ImGui::IsWindowHovered())
    {
        const auto & world = ToWorldCoord();
        const auto & mouse = ImGui::GetMousePos();
        const auto & move = GetUIData(thisState->mData, Move);
        const auto direct = math_tool::IsHitEdge(
            glm::vec4(world.x, world.y, move.z, move.w),
            glm::vec2(mouse.x, mouse.y), LAYOUT_DRAG_PADDING);
        if (direct != -1 && thisState->mJoin[direct].mIn.first == nullptr)
        {
            switch ((DirectEnum)direct)
            {
            case DirectEnum::kU: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS); break;
            case DirectEnum::kD: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS); break;
            case DirectEnum::kL: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW); break;
            case DirectEnum::kR: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW); break;
            }
            if (ImGui::IsMouseDown(0))
            {
                rootState->mStretchFocus.mObject = this;
                rootState->mStretchFocus.mDirect = (DirectEnum)direct;
            }
        }
    }

    if (rootState->mStretchFocus.mObject == this)
    {
        const auto & offset = ImGui::GetIO().MouseDelta;
        auto move   = GetUIData(thisState->mData, Move);
        switch ((int)rootState->mStretchFocus.mDirect)
        {
        case (int)DirectEnum::kU: move.y += offset.y; break;
        case (int)DirectEnum::kD: move.w += offset.y; break;
        case (int)DirectEnum::kL: move.x += offset.x; break;
        case (int)DirectEnum::kR: move.z += offset.x; break;
        }
        SetUIData(thisState->mData, Move, move);
    }

    if (ImGui::IsMouseReleased(0))
    {
        thisState->mStretchFocus.mObject = nullptr;
    }
}

bool UIClassLayout::OnEnter()
{
    auto state = GetState<UIStateLayout>();
    if (GetUIData(state->mData, IsWindow))
    {
        //  悬浮窗口
        auto & name = GetUIData(state->mData, Name);
        ImVec2 move = ImVec2(GetUIData(state->mData, Move).x, GetUIData(state->mData, Move).y);
        ImVec2 size = ImVec2(GetUIData(state->mData, Move).z, GetUIData(state->mData, Move).w);
        size_t flag = 0;
        if (GetUIData(state->mData, WindowIsFullScreen))
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
            if (!GetUIData(state->mData, WindowIsNav))        { flag |= ImGuiWindowFlags_NoNav; }
            if (!GetUIData(state->mData, WindowIsMove))       { flag |= ImGuiWindowFlags_NoMove; }
            if (!GetUIData(state->mData, WindowIsSize))       { flag |= ImGuiWindowFlags_NoResize; }
            if (!GetUIData(state->mData, WindowIsTitleBar))   { flag |= ImGuiWindowFlags_NoTitleBar; }
            if (!GetUIData(state->mData, WindowIsCollapse))   { flag |= ImGuiWindowFlags_NoCollapse; }
            if (!GetUIData(state->mData, WindowIsScrollBar))  { flag |= ImGuiWindowFlags_NoScrollbar; }
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

        ImGui::SetNextWindowPos(move);
        ImGui::SetNextWindowSize(size);
        return ImGui::Begin(name.empty()? nullptr: name.c_str(), nullptr, flag);
    }
    else
    {
        size_t flag = 0;
        if (!GetUIData(state->mData, WindowIsNav)) { flag |= ImGuiWindowFlags_NoNav; }
        if (!GetUIData(state->mData, WindowIsMove)) { flag |= ImGuiWindowFlags_NoMove; }
        if (!GetUIData(state->mData, WindowIsSize)) { flag |= ImGuiWindowFlags_NoResize; }
        if (!GetUIData(state->mData, WindowIsTitleBar)) { flag |= ImGuiWindowFlags_NoTitleBar; }
        if (!GetUIData(state->mData, WindowIsCollapse)) { flag |= ImGuiWindowFlags_NoCollapse; }
        if (!GetUIData(state->mData, WindowIsScrollBar)) { flag |= ImGuiWindowFlags_NoScrollbar; }
    
        auto & name = GetUIData(state->mData, Name);
        auto & move = GetUIData(state->mData, Move);
        ImGui::SetCursorPos(ImVec2(move.x,move.y));
        return ImGui::BeginChild(name.c_str(), ImVec2(move.z, move.w),
            GetUIData(state->mData, LayoutIsShowBorder), flag);
    }
}

void UIClassLayout::OnLeave()
{
    auto state = GetState<UIStateLayout>();
    if (GetUIData(state->mData, IsWindow))
    {
        ImGui::PopStyleVar(2);
        ImGui::End();
    }
    else
    {
        ImGui::EndChild();
    }
}

glm::vec4 UIClassLayout::StretchU(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move)
{
    auto thisState = GetState<UIStateLayout>();
    auto thisMove = GetUIData(thisState->mData, Move);
    if      (edge.second == DirectEnum::kU)
    {
        thisMove.w += (thisMove.y - move.y);
        thisMove.y = move.y;
    }
    else if (edge.second == DirectEnum::kD)
    {
        thisMove.w += (thisMove.y - move.y - move.w);
        thisMove.y = move.y + move.w;
    }
    return thisMove;
}

glm::vec4 UIClassLayout::StretchD(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move)
{
    auto thisState = GetState<UIStateLayout>();
    auto thisMove = GetUIData(thisState->mData, Move);
    if      (edge.second == DirectEnum::kU)
    {
        thisMove.w += (move.y - thisMove.y - thisMove.w);
    }
    else if (edge.second == DirectEnum::kD)
    {
        thisMove.w += (move.y + move.w - thisMove.y - thisMove.w);
    }
    return thisMove;
}

glm::vec4 UIClassLayout::StretchL(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move)
{
    auto thisState = GetState<UIStateLayout>();
    auto thisMove = GetUIData(thisState->mData, Move);
    if      (edge.second == DirectEnum::kL)
    {
        thisMove.z += (thisMove.x - move.x);
        thisMove.x = move.x;
    }
    else if (edge.second == DirectEnum::kR)
    {
        thisMove.z += (thisMove.x - move.x - move.z);
        thisMove.x = move.x + move.z;
    }
    return thisMove;
}

glm::vec4 UIClassLayout::StretchR(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move)
{
    auto thisState = GetState<UIStateLayout>();
    auto thisMove = GetUIData(thisState->mData, Move);
    if      (edge.second == DirectEnum::kL)
    {
        thisMove.z += (move.x - thisMove.x - thisMove.z);
    }
    else if (edge.second == DirectEnum::kR)
    {
        thisMove.z += (move.x + move.z - thisMove.x - thisMove.z);
    }
    return thisMove;
}
