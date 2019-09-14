#include "ui_class.h"
#include "../ui_menu.h"
#include "../ui_state/ui_state.h"
#include "../../atlas_sys/atlas_sys.h"
#include "imgui_impl_glfw.h"

// ---
//  EventDetails
// ---
std::vector<int> UIClass::UIEventDetails::Key::Hotkeys = {
    'A', 'B', 'C', 'D'
};

int UIClass::UIEventDetails::CheckStateKey()
{
    int flag = 0;
    if (ImGui::GetIO().KeyAlt) { flag |= 1; }
    if (ImGui::GetIO().KeyCtrl) { flag |= 2; }
    if (ImGui::GetIO().KeyShift) { flag |= 4; }
    return flag;
}

// ---
//  UIClass
// ---
UIClass * UIClass::GetChildren(const std::initializer_list<std::string>& list)
{
    ASSERT_LOG(list.size() != 0, "");
    auto parent = this;
    for (auto & name : list)
    {
        auto it = std::find_if(
            parent->GetChildren().begin(), 
            parent->GetChildren().end(), 
            [&name](UIClass * object) {
                return GetUIData(object->GetState()->mData, Name) == name;
            });
        if (it == _children.end())
        {
            return nullptr;
        }
        parent = *it;
    }
    return parent;
}

std::vector<UIClass*> UIClass::GetChildren(UITypeEnum type) const
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
    {
        ret = ret->GetParent();
    }
    return ret;
}

UIClass * UIClass::GetParent()
{
    return _parent;
}

void UIClass::Render(float dt)
{
    ApplyLayout();
    auto ret = OnEnter();
    if (ret) { OnRender(dt); }

    for (auto child : _children)
    {
        child->Render(dt);
    }

    OnLeave(ret);

    //  刷新备份数据
    auto & data = GetState()->mData;
    SetUIData(data, _Move, GetUIData(data, Move));
}

void UIClass::ResetLayout()
{
    OnResetLayout();

    //  初始化备份数据
    auto & data = GetState()->mData;
    SetUIData(data, _Move, GetUIData(data, Move));

    std::for_each(_children.begin(),_children.end(),
                    std::bind(&UIClass::ResetLayout, 
                    std::placeholders::_1));
}

void UIClass::ApplyLayout()
{
    auto & thisData = GetState()->mData;
    if (GetParent() != nullptr)
    {
        auto & parentData = GetParent()->GetState()->mData;
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
                margin.x = (parentMoveNew.z - (margin.z - margin.x)) * 0.5f;
                margin.z = margin.x + move.z;
            }
            if (align & (int)UIAlignEnum::kCENTER_V)
            {
                margin.y = (parentMoveNew.w - (margin.w - margin.y)) * 0.5f;
                margin.w = margin.y + move.w;
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
        const auto & wsize = ImGui_ImplGlfw_GetWindowSize();
        auto & size = GetUIData(thisData, StretchMin);
        auto   move = GetUIData(thisData, Move);
        move.z = std::max(wsize.x, size.x);
        move.w = std::max(wsize.y, size.y);
        SetUIData(thisData, Move, move);
    }
    OnApplyLayout();
}

glm::vec4 UIClass::CalcStretech(DirectEnum direct, const glm::vec2 & offset) const
{
    auto move = GetUIData(GetState()->mData, Move);
    switch (direct)
    {
    case DirectEnum::kU: move.y += offset.y; move.w -= offset.y; break;
    case DirectEnum::kD: move.w += offset.y; break;
    case DirectEnum::kL: move.x += offset.x; move.z -= offset.x; break;
    case DirectEnum::kR: move.z += offset.x; break;
    }
    return move;
}

glm::vec2 UIClass::ToWorldCoord(const glm::vec2 & coord) const
{
    auto move = GetUIData(GetState()->mData, Move);
    move.x += coord.x;
    move.y += coord.y;

    auto parent = GetParent();
    while (parent != nullptr)
    {
        move.x += GetUIData(parent->GetState()->mData, Move).x;
        move.y += GetUIData(parent->GetState()->mData, Move).y;
        parent = parent->GetParent();
    }
    return glm::vec2(move.x, move.y);
}

glm::vec4 UIClass::ToLocalCoord(const glm::vec4 & coord) const
{
    const auto & world = ToWorldCoord();
    return glm::vec4(coord.x - world.x,
                     coord.y - world.y,
                     coord.z, coord.w);
}

glm::vec2 UIClass::ToLocalCoord(const glm::vec2 & coord) const
{
    const auto & world = ToWorldCoord();
    return glm::vec2(coord.x - world.x,
                     coord.y - world.y);
}

glm::vec4 UIClass::ToWorldRect() const
{
    auto & move = GetUIData(GetState()->mData, Move);
    return glm::vec4(ToWorldCoord(), move.z, move.w);
}

void UIClass::BindDelegate(UIEventDelegate * delegate)
{
    SAFE_DELETE(_delegate);
    _delegate = delegate;
}

void UIClass::LockPosition()
{
    auto & move = GetUIData(GetState()->mData, Move);
    auto align = GetUIData(GetState()->mData, Align);
    if (align != (int)UIAlignEnum::kDEFAULT)
    {
        ImGui::SetCursorPos(ImVec2(move.x, move.y));
    }
}

bool UIClass::OnEnter()
{
    return true;
}

void UIClass::OnLeave(bool ret)
{ }

void UIClass::OnRender(float dt)
{ }

void UIClass::OnResetLayout()
{ }

void UIClass::OnApplyLayout()
{ }

void UIClass::DispatchEventK()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        for (auto key : UIEventDetails::Key::Hotkeys)
        {
            if (ImGui::IsKeyDown(key)) { DispatchEventK(UIEventDetails::Key(0, key)); }
            if (ImGui::IsKeyReleased(key)) { DispatchEventK(UIEventDetails::Key(1, key)); }
            if (ImGui::IsKeyPressed(key, false)) { DispatchEventK(UIEventDetails::Key(2, key)); }
        }
    }
}

bool UIClass::DispatchEventK(const UIEventDetails::Key & param)
{
    std::vector<UIClass *> objects{ this };
    for (auto i = 0; objects.size() != i; ++i)
    {
        std::copy(
            objects.at(i)->GetChildren().begin(),
            objects.at(i)->GetChildren().end(),
            std::back_inserter(objects));
    }
    return std::find_if(objects.rbegin(), objects.rend(), std::bind(
        &UIClass::PostEventMessage, std::placeholders::_1,
        UIEventEnum::kKEY, param)) != objects.rend();
}

void UIClass::DispatchEventM()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsMouseReleased(0)) { DispatchEventM(UIEventDetails::Mouse(2, 0)); }
        if (ImGui::IsMouseReleased(1)) { DispatchEventM(UIEventDetails::Mouse(2, 1)); }
        if (ImGui::IsMouseReleased(2)) { DispatchEventM(UIEventDetails::Mouse(2, 2)); }

        if (ImGui::IsMouseClicked(0)) { DispatchEventM(UIEventDetails::Mouse(3, 0)); }
        if (ImGui::IsMouseClicked(1)) { DispatchEventM(UIEventDetails::Mouse(3, 1)); }
        if (ImGui::IsMouseClicked(2)) { DispatchEventM(UIEventDetails::Mouse(3, 2)); }

        if (ImGui::IsMouseDoubleClicked(0)) { DispatchEventM(UIEventDetails::Mouse(4, 0)); }
        if (ImGui::IsMouseDoubleClicked(1)) { DispatchEventM(UIEventDetails::Mouse(4, 1)); }
        if (ImGui::IsMouseDoubleClicked(2)) { DispatchEventM(UIEventDetails::Mouse(4, 2)); }

        auto hoverKey = -1;
        if (ImGui::IsMouseDown(0)) { DispatchEventM(UIEventDetails::Mouse(1, 0)); hoverKey = 0; }
        if (ImGui::IsMouseDown(1)) { DispatchEventM(UIEventDetails::Mouse(1, 1)); hoverKey = 1; }
        if (ImGui::IsMouseDown(2)) { DispatchEventM(UIEventDetails::Mouse(1, 2)); hoverKey = 2; }
        DispatchEventM(UIEventDetails::Mouse(0, hoverKey));
    }
}

bool UIClass::DispatchEventM(const UIEventDetails::Mouse & param)
{
    if (math_tool::IsContain(ToWorldRect(), param.mMouse))
    {
        for (auto child : GetChildren())
        {
            if (child->DispatchEventM(param))
            {
                return true;
            }
        }
        return PostEventMessage(UIEventEnum::kMOUSE, param);
    }
    return false;
}

bool UIClass::OnCallEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
{
    return false;
}

bool UIClass::CallEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
{
    auto ret = OnCallEventMessage(e, param);

    if (_delegate != nullptr)
    {
        ret = _delegate->OnCallEventMessage(this, e, param) || ret;
    }

    if (!ret && GetParent() != nullptr)
    {
        ret = GetParent()->CallEventMessage(e, param);
    }
    return ret;
}

bool UIClass::PostEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
{
    param.mObject = this;
    return CallEventMessage(e, param);
}

// ---
//  Layout
// ---
bool UIClassLayout::OnEnter()
{
    auto state = GetState<UIStateLayout>();

    //  窗口flag
    size_t flag = ImGuiWindowFlags_NoCollapse;
    if (!GetUIData(state->mData, IsShowNav)) { flag |= ImGuiWindowFlags_NoNav; }
    if (!GetUIData(state->mData, IsCanMove)) { flag |= ImGuiWindowFlags_NoMove; }
    if (!GetUIData(state->mData, IsCanStretch)) { flag |= ImGuiWindowFlags_NoResize; }
    if ( GetUIData(state->mData, IsShowMenuBar)) { flag |= ImGuiWindowFlags_MenuBar; }
    if (!GetUIData(state->mData, IsShowTitleBar)) { flag |= ImGuiWindowFlags_NoTitleBar; }
    if (!GetUIData(state->mData, IsShowScrollBar)) { flag |= ImGuiWindowFlags_NoScrollbar; }

    if (GetUIData(state->mData, IsWindow))
    {
        //  悬浮窗口
        auto & name = GetUIData(state->mData, Name);
        ImVec2 move = ImVec2(GetUIData(state->mData, Move).x, GetUIData(state->mData, Move).y);
        ImVec2 size = ImVec2(GetUIData(state->mData, Move).z, GetUIData(state->mData, Move).w);
        if (GetUIData(state->mData, IsFullScreen))
        {
            size = ImGui_ImplGlfw_GetWindowSize();
            move.x = 0;
            move.y = 0;
            flag |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                 | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
                 | ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoNav;
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
        LockPosition();
        auto & name = GetUIData(state->mData, Name);
        auto & move = GetUIData(state->mData, Move);
        return ImGui::BeginChild(name.c_str(), ImVec2(move.z, move.w),
            GetUIData(state->mData, IsShowBorder), flag);
    }
}

void UIClassLayout::OnLeave(bool ret)
{
    auto state = GetState<UIStateLayout>();
    if (GetUIData(state->mData, IsWindow))
    {
        if (GetRoot() == this)
        {
            UIMenu::RenderPopup();
        }
        ImGui::PopStyleVar(2);
        DispatchEventM();
        DispatchEventK();
        ImGui::End();
    }
    else
    {
        ImGui::EndChild();
    }
    HandleStretch();
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
        auto state  = layout->GetState<UIStateLayout>();
        auto up     = GetUIData(state->mData, Move).y;
        auto down   = up + GetUIData(state->mData, Move).w;
        auto left   = GetUIData(state->mData, Move).x;
        auto right  = left + GetUIData(state->mData, Move).z;
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
        auto & thisMove   = GetUIData(thisState->mData, Move);
        const auto & edge = thisState->mJoin[(int)direct].mIn;
        if (edge.first != nullptr)
        {
            const auto & move = GetUIData(edge.first->GetState()->mData, Move);
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
    if (GetUIData(GetState()->mData, IsShowMenuBar))
    {
        UIMenu::BarMenu(this, GetUIData(GetState()->mData, MenuBar));
    }
}

bool UIClassLayout::IsCanStretch(DirectEnum edge)
{
    auto parent = GetParent();
    CHECK_RET(parent != nullptr, false);
    ASSERT_LOG(dynamic_cast<UIClassLayout *>(parent) != nullptr, "");
    CHECK_RET(GetUIData(GetState()->mData, IsCanStretch), false);

    auto cling = GetState<UIStateLayout>()->mJoin[(int)edge].mIn.first != nullptr
               ? GetState<UIStateLayout>()->mJoin[(int)edge].mIn.first : this;
    const auto & clingMove  = GetUIData(cling->GetState()->mData, Move);
    const auto & parentMove = GetUIData(parent->GetState()->mData, Move);
    switch (edge)
    {
    case DirectEnum::kU: return !math_tool::Equal(clingMove.y, 0);
    case DirectEnum::kD: return !math_tool::Equal(clingMove.y + clingMove.w, parentMove.w);
    case DirectEnum::kL: return !math_tool::Equal(clingMove.x, 0);
    case DirectEnum::kR: return !math_tool::Equal(clingMove.x + clingMove.z, parentMove.z);
    }
    ASSERT_LOG(false, "{0}", (int)edge);
}

bool UIClassLayout::IsCanStretch(DirectEnum edge, const glm::vec2 & offset)
{
    CHECK_RET((edge == DirectEnum::kU || edge == DirectEnum::kD) && offset.y != 0 ||
              (edge == DirectEnum::kL || edge == DirectEnum::kR) && offset.x != 0, false);

    auto thisState = GetState<UIStateLayout>();
    const auto & newMove = CalcStretech(edge, offset);
    const auto & oldMove = GetUIData(thisState->mData, Move);
    const auto & minMove = GetUIData(thisState->mData, StretchMin);
    if (newMove.z < oldMove.z && newMove.z < minMove.x ||
        newMove.w < oldMove.w && newMove.w < minMove.y) { return false; }

    for (auto object : thisState->mJoin[(int)edge].mOut)
    {
        auto state = object->GetState<UIStateLayout>();
        for (auto i = 0; i != (int)DirectEnum::LENGTH; ++i)
        {
            if (state->mJoin[i].mIn.first  == this && 
                state->mJoin[i].mIn.second == edge &&
                !((UIClassLayout *)object)->IsCanStretch((DirectEnum)i, offset))
            { return false; }
        }
    }
    return true;
}

void UIClassLayout::HandleStretch()
{
    auto thisState =            GetState<UIStateLayout>();
    auto rootState = GetRoot()->GetState<UIStateLayout>();
    if (rootState->mStretchFocus.mObject == nullptr && ImGui::IsItemHovered())
    {
        const auto & world = ToWorldCoord();
        const auto & mouse = ImGui::GetMousePos();
        const auto & move = GetUIData(thisState->mData, Move);
        const auto direct = math_tool::IsOnEdge(
            glm::vec4(world.x, world.y, move.z, move.w),
            glm::vec2(mouse.x, mouse.y), LAYOUT_STRETCH_BORDER);
        if (direct != -1 && IsCanStretch((DirectEnum)direct))
        {
            switch ((DirectEnum)direct)
            {
            case DirectEnum::kU: case DirectEnum::kD: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS); break;
            case DirectEnum::kL: case DirectEnum::kR: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW); break;
            }
            if (ImGui::IsMouseDown(0))
            {
                rootState->mStretchFocus.mObject = GetState<UIStateLayout>()->mJoin[direct].mIn.first != nullptr
                                                ? GetState<UIStateLayout>()->mJoin[direct].mIn.first : this;
                rootState->mStretchFocus.mDirect = GetState<UIStateLayout>()->mJoin[direct].mIn.first != nullptr
                                                ? GetState<UIStateLayout>()->mJoin[direct].mIn.second : (DirectEnum)direct;
            }
        }
    }

    if (rootState->mStretchFocus.mObject == this)
    {
        const auto & offset = ImGui::GetIO().MouseDelta;
        auto move   = GetUIData(thisState->mData, Move);
        if (IsCanStretch(rootState->mStretchFocus.mDirect, glm::vec2(offset.x, offset.y)))
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

// ---
//  UIClassTree
// ---
bool UIClassTreeBox::OnEnter()
{
    LockPosition();

    auto  state = GetState<UIStateTreeBox>();
    auto & move = GetUIData(state->mData, Move);

    ImGui::SetNextItemWidth(move.z);
    if (ImGui::TreeNodeEx(GetUIData(state->mData, Name).c_str()))
    {
        ImGui::Indent(move.x + ImGui::GetStyle().IndentSpacing);
        return true;
    }
    return false;
}

void UIClassTreeBox::OnLeave(bool ret)
{
    if (ret)
    {
        ImGui::TreePop();
        auto  state = GetState<UIStateTreeBox>();
        auto & move = GetUIData(state->mData, Move);
        ImGui::Unindent(move.x + ImGui::GetStyle().IndentSpacing);
    }
}

// ---
//  UIClassTextBox
// ---
void UIClassTextBox::OnRender(float dt)
{
    LockPosition();
    
    auto state = GetState<UIStateTextBox>();
    if (GetUIData(state->mData, IsEditBox))
    {
        auto & move = GetUIData(state->mData, Move);
        auto flag = ImGuiInputTextFlags_EnterReturnsTrue
                  | ImGuiInputTextFlags_AutoSelectAll;
        if (GetUIData(state->mData, IsMulti))
        {
            if (ImGui::InputTextMultiline(
                GetUIData(state->mData, Name).c_str(), 
                state->mBuffer.data(), 
                state->mBuffer.size(), 
                ImVec2(move.z, move.w), flag))
            {
                PostEventMessage(UIEventEnum::kEDIT_TEXT_FINISH, UIEventDetails::EditText(state->mBuffer, this));
            }
        }
        else
        {
            ImGui::PushItemWidth(move.z);
            if (ImGui::InputText(
                GetUIData(state->mData, Name).c_str(), 
                state->mBuffer.data(), 
                state->mBuffer.size(), flag))
            {
                PostEventMessage(UIEventEnum::kEDIT_TEXT_FINISH, UIEventDetails::EditText(state->mBuffer, this));
            }
            ImGui::PopItemWidth();
        }
    }
    else
    {
        ImGui::Text(GetUIData(state->mData, Name).c_str());
    }
}

//--------------------------------------------------------------------------------
//  UIClassImage
//--------------------------------------------------------------------------------
void UIClassImageBox::OnRender(float dt)
{
    LockPosition();

    auto  state    = GetState<UIStateImageBox>();
    auto & move    = GetUIData(state->mData, Move);
    auto & strSkin = GetUIData(state->mData, LSkin);
    if (GetUIData(state->mData, IsButton))
    {
        if (!strSkin.empty())
        {
            auto & imgSkin = Global::Ref().mAtlasSys->Get(strSkin);
            if (ImGui::ImageButton(
                (ImTextureID)imgSkin.mID, ImVec2(move.z, move.w),
                ImVec2(imgSkin.mQuat.x, imgSkin.mQuat.y),
                ImVec2(imgSkin.mQuat.z, imgSkin.mQuat.w), 0))
            {
                PostEventMessage(UIEventEnum::kMOUSE, UIEventDetails::Mouse(3, 0, this));
            }
        }
        else
        {
            if (ImGui::Button(GetUIData(state->mData, Name).c_str(), ImVec2(move.z, move.y)))
            {
                PostEventMessage(UIEventEnum::kMOUSE, UIEventDetails::Mouse(3, 0, this));
            }
        }
    }
    else
    {
        ASSERT_LOG(!strSkin.empty(), "");
        auto & imgSkin = Global::Ref().mAtlasSys->Get(strSkin);
        ImGui::Image(
            (ImTextureID)imgSkin.mID,
            ImVec2(move.z, move.w),
            ImVec2(imgSkin.mQuat.x, imgSkin.mQuat.y),
            ImVec2(imgSkin.mQuat.z, imgSkin.mQuat.w));
    }
}

// ---
//  UIClassComboBox
// ---
bool UIClassComboBox::OnEnter()
{
    LockPosition();

    auto  state = GetState<UIStateComboBox>();
    auto & move = GetUIData(state->mData, Move);
    ImGui::SetNextItemWidth(move.z);

    if (state->mSelected.empty() && !GetChildren().empty())
    {
        state->mSelected = GetUIData(GetChildren().at(0)->GetState()->mData, Name);
    }
    return ImGui::BeginCombo(GetUIData(state->mData, Name).c_str(), state->mSelected.c_str());
}

void UIClassComboBox::OnLeave(bool ret)
{
    if (ret) { ImGui::EndCombo(); }
}

void UIClassComboBox::OnRender(float dt)
{ }

bool UIClassComboBox::OnCallEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
{
    if (e == UIEventEnum::kMOUSE)
    {
        auto & object = std::any_cast<const UIEventDetails::Mouse &>(param).mObject;
        auto & name = GetUIData(object->GetState()->mData, Name);
        GetState<UIStateComboBox>()->mSelected = name;
        ImGui::CloseCurrentPopup();
    }
    return false;
}
