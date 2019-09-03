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

void UIClass::Render(float dt)
{
    ApplyLayout();
    if (OnEnter())
    {
        OnRender(dt);
        std::for_each(_children.begin(), _children.end(),
                      std::bind(&UIClass::Render, 
                      std::placeholders::_1, dt));
    }
    OnLeave();

    //  刷新备份数据
    auto & data = GetState<UIState>()->mData;
    SetUIData(data, _Move, GetUIData(data, Move));
}

void UIClass::ResetLayout()
{
    OnResetLayout();

    //  初始化备份数据
    auto & data = GetState<UIState>()->mData;
    SetUIData(data, _Move, GetUIData(data, Move));

    std::for_each(_children.begin(),_children.end(),
                    std::bind(&UIClass::ResetLayout, 
                    std::placeholders::_1));
}

void UIClass::ApplyLayout()
{
    auto & thisData = GetState<UIState>()->mData;
    if (GetParent() != nullptr)
    {
        auto & parentData = GetParent()->GetState<UIState>()->mData;
        auto & parentMoveOld = GetUIData(parentData, _Move);
        auto & parentMoveNew = GetUIData(parentData,  Move);

        if (!math_tool::Equal(parentMoveOld.z, parentMoveNew.z) || 
            !math_tool::Equal(parentMoveOld.w, parentMoveNew.w))
        {
            auto move    = GetUIData(thisData, Move);
            auto align   = GetUIData(thisData, Align);
            auto margin  = glm::vec4(move.x, move.y, move.x + move.z, move.y + move.w);
            //  上下贴边, 排斥垂直居中, 垂直拉伸
            ASSERT_LOG((align & ((int)UIAlignEnum::kCLING_T | (int)UIAlignEnum::kCLING_B)) == 0
                    || (align & (int)UIAlignEnum::kCENTER_V)  == 0
                    && (align & (int)UIAlignEnum::kSTRETCH_V) == 0, "{0}", align);
            //  左右贴边, 排斥水平居中, 水平拉伸
            ASSERT_LOG((align & ((int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kCLING_R)) == 0
                    || (align & (int)UIAlignEnum::kCENTER_H)  == 0
                    && (align & (int)UIAlignEnum::kSTRETCH_H) == 0, "{0}", align);
            //  水平居中, 排斥水平拉伸
            ASSERT_LOG((align & (int)UIAlignEnum::kCENTER_H) == 0
                    || (align & (int)UIAlignEnum::kSTRETCH_H) == 0, "{0}", align);
            //  垂直居中, 排斥垂直拉伸
            ASSERT_LOG((align & (int)UIAlignEnum::kCENTER_V) == 0
                    || (align & (int)UIAlignEnum::kSTRETCH_V) == 0, "{0}", align);
            if (align & (int)UIAlignEnum::kCLING_B)
            {
                margin.w += parentMoveNew.w - parentMoveOld.w;
                if ((align & (int)UIAlignEnum::kCLING_T) == 0) { margin.y = margin.w - move.w; }
            }
            if (align & (int)UIAlignEnum::kCLING_R)
            {
                margin.z += parentMoveNew.z - parentMoveOld.z;
                if ((align & (int)UIAlignEnum::kCLING_L) == 0) { margin.x = margin.z - move.z; }
            }
            if (align & (int)UIAlignEnum::kCENTER_H)
            {
                margin.x = margin.x / parentMoveOld.z * parentMoveNew.z 
                         + margin.z / parentMoveOld.z * parentMoveNew.z;
            }
            if (align & (int)UIAlignEnum::kCENTER_V)
            {
                margin.y = margin.y / parentMoveOld.w * parentMoveNew.w
                         + margin.w / parentMoveOld.w * parentMoveNew.w;
            }
            if (align & (int)UIAlignEnum::kSTRETCH_H)
            {
                margin.x = (margin.x / parentMoveOld.z * parentMoveNew.z);
                margin.z = (margin.z / parentMoveOld.z * parentMoveNew.z);
            }
            if (align & (int)UIAlignEnum::kSTRETCH_V)
            {
                margin.y = (margin.y / parentMoveOld.w * parentMoveNew.w);
                margin.w = (margin.w / parentMoveOld.w * parentMoveNew.w);
            }
            move.x = margin.x;
            move.y = margin.y;
            move.z = margin.z - margin.x;
            move.w = margin.w - margin.y;
            SetUIData(thisData, Move, move);
        }
    }
    else
    {
        const auto & size = ImGui_ImplGlfw_GetWindowSize();
        auto move = GetUIData(thisData, Move);
        move.z = size.x;move.w = size.y;
        SetUIData(thisData, Move, move);
    }
    OnApplyLayout();
}

glm::vec4 UIClass::CalcStretech(DirectEnum direct, const glm::vec2 & offset)
{
    auto move = GetUIData(GetState<UIState>()->mData, Move);
    switch (direct)
    {
    case DirectEnum::kU: move.y += offset.y; move.w -= offset.y; break;
    case DirectEnum::kD: move.w += offset.y; break;
    case DirectEnum::kL: move.x += offset.x; move.z -= offset.x; break;
    case DirectEnum::kR: move.z += offset.x; break;
    }
    return move;
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

void UIClass::LockPosition()
{
    auto & move = GetUIData(GetState<UIState>()->mData, Move);
    auto align = GetUIData(GetState<UIState>()->mData, Align);
    if (align != (int)UIAlignEnum::kDEFAULT)
    {
        ImGui::SetCursorPos(ImVec2(move.x, move.y));
    }
}

bool UIClass::OnEnter()
{
    return true;
}

void UIClass::OnLeave()
{ }

void UIClass::OnResetLayout()
{ }

void UIClass::OnApplyLayout()
{ }

//--------------------------------------------------------------------------------
//  Layout
//--------------------------------------------------------------------------------
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
        if (GetUIData(state->mData, IsFullScreen))
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
            if (!GetUIData(state->mData, IsShowNav))        { flag |= ImGuiWindowFlags_NoNav; }
            if (!GetUIData(state->mData, IsCanMove))        { flag |= ImGuiWindowFlags_NoMove; }
            if (!GetUIData(state->mData, IsCanStretch))     { flag |= ImGuiWindowFlags_NoResize; }
            if (!GetUIData(state->mData, IsShowTitleBar))   { flag |= ImGuiWindowFlags_NoTitleBar; }
            if (!GetUIData(state->mData, IsShowCollapse))   { flag |= ImGuiWindowFlags_NoCollapse; }
            if (!GetUIData(state->mData, IsShowScrollBar))  { flag |= ImGuiWindowFlags_NoScrollbar; }
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

        if (GetUIData(state->mData, IsFullScreen) || 
            GetUIData(state->mData, Align) != (int)UIAlignEnum::kDEFAULT)
        {
            ImGui::SetNextWindowPos(move);
        }
        ImGui::SetNextWindowSize(size);
        return ImGui::Begin(name.empty()? nullptr: name.c_str(), nullptr, flag);
    }
    else
    {
        size_t flag = 0;
        if (!GetUIData(state->mData, IsShowNav))        { flag |= ImGuiWindowFlags_NoNav; }
        if (!GetUIData(state->mData, IsCanMove))        { flag |= ImGuiWindowFlags_NoMove; }
        if (!GetUIData(state->mData, IsCanStretch))     { flag |= ImGuiWindowFlags_NoResize; }
        if (!GetUIData(state->mData, IsShowTitleBar))   { flag |= ImGuiWindowFlags_NoTitleBar; }
        if (!GetUIData(state->mData, IsShowCollapse))   { flag |= ImGuiWindowFlags_NoCollapse; }
        if (!GetUIData(state->mData, IsShowScrollBar))  { flag |= ImGuiWindowFlags_NoScrollbar; }
    
        LockPosition();
        auto & name = GetUIData(state->mData, Name);
        auto & move = GetUIData(state->mData, Move);
        return ImGui::BeginChild(name.c_str(), ImVec2(move.z, move.w),
            GetUIData(state->mData, IsShowBorder), flag);
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
        auto & thisMove = GetUIData(thisState->mData, Move);
        if (thisState->mJoin[(int)direct].mIn.first != nullptr)
        {
            const auto & edge = thisState->mJoin[(int)direct].mIn;
            const auto & move = GetUIData(edge.first->GetState<UIState>()->mData, Move);
            glm::vec2 offset(0, 0);
            if      ((DirectEnum)direct == DirectEnum::kU && edge.second == DirectEnum::kU) { offset.y = move.y - thisMove.y; }
            else if ((DirectEnum)direct == DirectEnum::kU && edge.second == DirectEnum::kD) { offset.y = move.y + move.w - thisMove.y; }
            else if ((DirectEnum)direct == DirectEnum::kD && edge.second == DirectEnum::kD) { offset.y = move.y + move.w - thisMove.y - thisMove.w; }
            else if ((DirectEnum)direct == DirectEnum::kD && edge.second == DirectEnum::kU) { offset.y = move.y - thisMove.y - thisMove.w; }

            if      ((DirectEnum)direct == DirectEnum::kL && edge.second == DirectEnum::kL) { offset.x = move.x - thisMove.x; }
            else if ((DirectEnum)direct == DirectEnum::kL && edge.second == DirectEnum::kR) { offset.x = move.x + move.z - thisMove.x; }
            else if ((DirectEnum)direct == DirectEnum::kR && edge.second == DirectEnum::kR) { offset.x = move.x + move.z - thisMove.x - thisMove.z; }
            else if ((DirectEnum)direct == DirectEnum::kR && edge.second == DirectEnum::kL) { offset.x = move.x - thisMove.x - thisMove.z; }
            SetUIData(thisState->mData, Move, CalcStretech((DirectEnum)direct, offset));
        }
    }
}

void UIClassLayout::OnRender(float dt)
{
    auto thisState =            GetState<UIStateLayout>();
    auto rootState = GetRoot()->GetState<UIStateLayout>();
    if (rootState->mStretchFocus.mObject == nullptr && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        const auto & world = ToWorldCoord();
        const auto & mouse = ImGui::GetMousePos();
        const auto & move = GetUIData(thisState->mData, Move);
        const auto direct = math_tool::IsOnEdge(
            glm::vec4(world.x, world.y, move.z, move.w),
            glm::vec2(mouse.x, mouse.y), LAYOUT_DRAG_PADDING);
        if (direct != -1 && IsCanDrag((DirectEnum)direct))
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
        if (IsCanDrag(rootState->mStretchFocus.mDirect, glm::vec2(offset.x, offset.y)))
        {
            switch ((int)rootState->mStretchFocus.mDirect)
            {
            case (int)DirectEnum::kU: move.y += offset.y; break;
            case (int)DirectEnum::kD: move.w += offset.y; break;
            case (int)DirectEnum::kL: move.x += offset.x; break;
            case (int)DirectEnum::kR: move.z += offset.x; break;
            }
            SetUIData(thisState->mData, Move, move);
        }
    }

    if (ImGui::IsMouseReleased(0))
    {
        thisState->mStretchFocus.mObject = nullptr;
    }
}

bool UIClassLayout::IsCanDrag(DirectEnum edge)
{
    auto parent = GetParent();
    CHECK_RET(parent != nullptr, false);
    ASSERT_LOG(dynamic_cast<UIClassLayout *>(parent) != nullptr, "");
    if (GetState<UIStateLayout>()->mJoin[(int)edge].mIn.first == nullptr)
    {
        const auto & thisMove   = GetUIData(        GetState<UIState>()->mData, Move);
        const auto & parentMove = GetUIData(parent->GetState<UIState>()->mData, Move);
        switch (edge)
        {
        case DirectEnum::kU: return !math_tool::Equal(thisMove.y, 0);
        case DirectEnum::kD: return !math_tool::Equal(thisMove.y + thisMove.w, parentMove.w);
        case DirectEnum::kL: return !math_tool::Equal(thisMove.x, 0);
        case DirectEnum::kR: return !math_tool::Equal(thisMove.x + thisMove.z, parentMove.z);
        }
        ASSERT_LOG(false, "{0}", (int)edge);
    }
    return false;
}

bool UIClassLayout::IsCanDrag(DirectEnum edge, const glm::vec2 & offset)
{
    CHECK_RET((edge == DirectEnum::kU || edge == DirectEnum::kD) && offset.y != 0 ||
              (edge == DirectEnum::kL || edge == DirectEnum::kR) && offset.x != 0, false);

    auto thisState = GetState<UIStateLayout>();
    const auto & move = CalcStretech(edge, offset);
    const auto & min  = GetUIData(thisState->mData, StretchMin);
    if (move.z < min.x || move.w < min.y) { return false; }

    for (auto object : thisState->mJoin[(int)edge].mOut)
    {
        auto state = object->GetState<UIStateLayout>();
        for (auto i = 0; i != (int)DirectEnum::LENGTH; ++i)
        {
            if (state->mJoin[i].mIn.first  == this && 
                state->mJoin[i].mIn.second == edge &&
                !((UIClassLayout *)object)->IsCanDrag((DirectEnum)i, offset))
            { return false; }
        }
    }
    return true;
}

void UIClassTextBox::OnRender(float dt)
{
    auto state = GetState<UIStateTextBox>();
    auto & move = GetUIData(state->mData, Move);
    ASSERT_LOG(GetUIData(state->mData, IsTextBox) || GetUIData(state->mData, IsEditBox), "");

    LockPosition();

    //  渲染前
    auto & bgColor = GetUIData(state->mData, BgColor);
    auto & color = GetUIData(state->mData, Color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(bgColor.x, bgColor.y, bgColor.z, bgColor.w));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.x, color.y, color.z, color.w));

    if      (GetUIData(state->mData, IsTextBox))
    {
        ImGui::Text(GetUIData(state->mData, Title).c_str());
    }
    else if (GetUIData(state->mData, IsEditBox))
    {
        auto flag = ImGuiInputTextFlags_EnterReturnsTrue
                  | ImGuiInputTextFlags_AutoSelectAll;
        if (GetUIData(state->mData, IsMulti))
        {
            if (ImGui::InputTextMultiline(
                GetUIData(state->mData, Title).c_str(), 
                state->mBuffer.data(), 
                state->mBuffer.size(), 
                ImVec2(move.z, move.w), flag))
            {
                std::cout << "Input Text" << std::endl;
            }
        }
        else
        {
            ImGui::PushItemWidth(move.z);
            if (ImGui::InputText(
                GetUIData(state->mData, Title).c_str(), 
                state->mBuffer.data(), 
                state->mBuffer.size(), flag))
            {
                std::cout << "Input Text" << std::endl;
            }
            ImGui::PopItemWidth();
        }
    }

    if (GetUIData(state->mData, IsShowBorder))
    {
        const auto & min = ImGui::GetItemRectMin();
        const auto & max = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(min, max, 0xFFFFFFFF);
    }

    //  渲染后
    ImGui::PopStyleColor(2);
}
