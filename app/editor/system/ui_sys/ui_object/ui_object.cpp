#include "ui_object.h"
#include "../ui_menu.h"
#include "../ui_state/ui_state.h"
#include "../../atlas_sys/atlas_sys.h"
#include "imgui_impl_glfw.h"

// ---
//  EventDetails
// ---
std::vector<int> UIObject::UIEventDetails::Key::Hotkeys = {
    'A', 'B', 'C', 'D'
};

int UIObject::UIEventDetails::CheckStateKey()
{
    int flag = 0;
    if (ImGui::GetIO().KeyAlt) { flag |= 1; }
    if (ImGui::GetIO().KeyCtrl) { flag |= 2; }
    if (ImGui::GetIO().KeyShift) { flag |= 4; }
    return flag;
}

// ---
//  UIObject
// ---
UIObject * UIObject::GetObjects(const std::initializer_list<std::string>& list)
{
    ASSERT_LOG(list.size() != 0, "");
    auto parent = this;
    for (auto & name : list)
    {
        auto it = std::find_if(
            parent->GetObjects().begin(), 
            parent->GetObjects().end(), 
            [&name](UIObject * object) {
                return object->GetState()->Name == name;
            });
        if (it == _children.end())
        {
            return nullptr;
        }
        parent = *it;
    }
    return parent;
}

std::vector<UIObject*> UIObject::GetObjects(UITypeEnum type) const
{
    std::vector<UIObject *> result;
    std::copy_if(_children.begin(), _children.end(), std::back_inserter(result), 
        [type](const UIObject * object) { return object->GetType() == type; });
    return std::move(result);
}

std::vector<UIObject*>& UIObject::GetObjects()
{
    return _children;
}

void UIObject::AddObject(UIObject * child)
{
    ASSERT_LOG(child->GetParent() == nullptr, "");
    _children.push_back(child);
    child->_parent = this;
}

void UIObject::DelObject(UIObject * child)
{
    auto it = std::find(_children.begin(), _children.end(), child);
    if (it != _children.end()) { delete *it; _children.erase(it); }
}

void UIObject::DelThis()
{
    ASSERT_LOG(GetParent() != nullptr, GetState()->Name.c_str());
    GetParent()->DelObject(this);
}

void UIObject::ClearObjects()
{
    while (!_children.empty())
    {
        delete _children.back();
        _children.pop_back();
    }
}

UIObject * UIObject::GetRoot()
{
    auto ret = this;
    while (ret->GetParent() != nullptr)
    {
        ret = ret->GetParent();
    }
    return ret;
}

UIObject * UIObject::GetParent()
{
    return _parent;
}

bool UIObject::IsVisible() const
{
    return _visible;
}

void UIObject::ResetLayout()
{
    OnResetLayout();

    //  初始化备份数据
    auto state = GetState();
    state->Move_ = state->Move;

    std::for_each(_children.begin(),_children.end(),
                    std::bind(&UIObject::ResetLayout, 
                    std::placeholders::_1));
}

void UIObject::ApplyLayout()
{
    auto thisState = GetState();
    if (GetParent() != nullptr)
    {
        auto parentState = GetParent()->GetState();
        const auto & parentMoveOld = parentState->Move_;
        const auto & parentMoveNew = parentState->Move;
        if (!tools::Equal(parentMoveOld.z, parentMoveNew.z) || 
            !tools::Equal(parentMoveOld.w, parentMoveNew.w))
        {
            const auto & move    = thisState->Move;
            const auto & align   = thisState->Align;
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
            thisState->Move.x = margin.x;
            thisState->Move.y = margin.y;
            thisState->Move.z = margin.z - margin.x;
            thisState->Move.w = margin.w - margin.y;
        }
    }
    else
    {
        const auto & wsize = ImGui_ImplGlfw_GetWindowSize();
        const auto & size = thisState->StretchMin;
        thisState->Move.z = std::max(wsize.x, size.x);
        thisState->Move.w = std::max(wsize.y, size.y);
    }
    OnApplyLayout();
}

void UIObject::Render(float dt, bool visible)
{
    _visible = visible;

    ApplyLayout();

    auto ret = false;
    if (visible)
    { 
        if (ret = OnEnter())
        {
            OnRender(dt);
        }
    }

    for (auto child : _children)
    {
        child->Render(dt, ret);
    }

    if (visible) { OnLeave(ret); }

    //  刷新备份数据
    auto state = GetState();
    state->Move_ = state->Move;
}


glm::vec4 UIObject::CalcStretech(DirectEnum direct, const glm::vec2 & offset) const
{
    auto move = GetState()->Move;
    switch (direct)
    {
    case DirectEnum::kU: move.y += offset.y; move.w -= offset.y; break;
    case DirectEnum::kD: move.w += offset.y; break;
    case DirectEnum::kL: move.x += offset.x; move.z -= offset.x; break;
    case DirectEnum::kR: move.z += offset.x; break;
    }
    return move;
}

glm::vec2 UIObject::ToWorldCoord(const glm::vec2 & coord) const
{
    auto move = GetState()->Move;
    move.x += coord.x;
    move.y += coord.y;

    auto parent = GetParent();
    while (parent != nullptr)
    {
        move.x += parent->GetState()->Move.x;
        move.y += parent->GetState()->Move.y;
        parent = parent->GetParent();
    }
    return glm::vec2(move.x, move.y);
}

glm::vec4 UIObject::ToLocalCoord(const glm::vec4 & coord) const
{
    const auto & world = ToWorldCoord();
    return glm::vec4(coord.x - world.x,
                     coord.y - world.y,
                     coord.z, coord.w);
}

glm::vec2 UIObject::ToLocalCoord(const glm::vec2 & coord) const
{
    const auto & world = ToWorldCoord();
    return glm::vec2(coord.x - world.x,
                     coord.y - world.y);
}

glm::vec4 UIObject::ToWorldRect() const
{
    return glm::vec4(ToWorldCoord(), GetState()->Move.z, GetState()->Move.w);
}

glm::vec2 UIObject::ToLocalCoordFromImGUI() const
{
    //  对于内部没有ImGui::Begin的组件, 需要使用这个接口返回ImGui::GetCursorPos
    auto pos = ImGui::GetCursorPos();
    auto parent = GetParent();
    while (parent->GetType() == UITypeEnum::kTEXTBOX
        || parent->GetType() == UITypeEnum::kTREEBOX
        || parent->GetType() == UITypeEnum::kIMAGEBOX)
    {
        pos.x -= parent->GetState()->Move.x;
        pos.y -= parent->GetState()->Move.y;
        parent = parent->GetParent();
    }
    return pos;
}

void UIObject::BindDelegate(UIEventDelegate * delegate)
{
    SAFE_DELETE(_delegate);
    _delegate = delegate;
}

void UIObject::AdjustSize()
{
    if ((UIAlignEnum)GetState()->Align == UIAlignEnum::kDEFAULT)
    {
        const auto & size = ImGui::GetItemRectSize();
        GetState()->Move.z = size.x;
        GetState()->Move.w = size.y;
    }
}

void UIObject::LockPosition()
{
    if ((UIAlignEnum)GetState()->Align != UIAlignEnum::kDEFAULT)
    {
        ImGui::SetCursorPos(ImVec2(GetState()->Move.x, GetState()->Move.y));
    }
    else
    {
        const auto & pos = ToLocalCoordFromImGUI();
        GetState()->Move.x = pos.x;
        GetState()->Move.y = pos.y;
    }
}

bool UIObject::OnEnter()
{
    return true;
}

void UIObject::OnLeave(bool ret)
{ }

void UIObject::OnRender(float dt)
{ }

void UIObject::OnResetLayout()
{ }

void UIObject::OnApplyLayout()
{ }

void UIObject::DispatchEventK()
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

bool UIObject::DispatchEventK(const UIEventDetails::Key & param)
{
    std::vector<UIObject *> objects{ this };
    for (auto i = 0; objects.size() != i; ++i)
    {
        std::copy(
            objects.at(i)->GetObjects().begin(),
            objects.at(i)->GetObjects().end(),
            std::back_inserter(objects));
    }
    return std::find_if(objects.rbegin(), objects.rend(), std::bind(
        &UIObject::PostEventMessage, std::placeholders::_1,
        UIEventEnum::kKEY, param)) != objects.rend();
}

void UIObject::DispatchEventM()
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

bool UIObject::DispatchEventM(const UIEventDetails::Mouse & param)
{
    for (auto child : GetObjects())
    {
        if (child->DispatchEventM(param))
        {
            return true;
        }
    }

    if (IsVisible() && tools::IsContain(ToWorldRect(), param.mMouse))
    {
        return PostEventMessage(UIEventEnum::kMOUSE, param);
    }
    return false;
}

bool UIObject::OnCallEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
{
    return false;
}

bool UIObject::CallEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
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

bool UIObject::PostEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
{
    param.mObject = this;
    return CallEventMessage(e, param);
}

// ---
//  Layout
// ---
UIClassLayout::UIClassLayout() : UIObject(UITypeEnum::kLAYOUT, new UIStateLayout())
{ }

bool UIClassLayout::OnEnter()
{
    auto state = GetState<UIStateLayout>();

    //  窗口flag
    size_t flag = ImGuiWindowFlags_NoCollapse;
    if (!state->IsShowNav)          { flag |= ImGuiWindowFlags_NoNav; }
    if (!state->IsCanMove)          { flag |= ImGuiWindowFlags_NoMove; }
    if (!state->IsCanStretch)       { flag |= ImGuiWindowFlags_NoResize; }
    if ( state->IsShowMenuBar)      { flag |= ImGuiWindowFlags_MenuBar; }
    if (!state->IsShowTitleBar)     { flag |= ImGuiWindowFlags_NoTitleBar; }
    if (!state->IsShowScrollBar)    { flag |= ImGuiWindowFlags_NoScrollbar; }

    if (state->IsWindow)
    {
        //  悬浮窗口
        const auto & name = state->Name;
        ImVec2 move = ImVec2(state->Move.x, state->Move.y);
        ImVec2 size = ImVec2(state->Move.z, state->Move.w);
        if (state->IsFullScreen)
        {
            size = ImGui_ImplGlfw_GetWindowSize();
            move.x = 0;
            move.y = 0;
            flag |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                 | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
                 | ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoNav;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

        if (state->IsFullScreen || (UIAlignEnum)state->Align != UIAlignEnum::kDEFAULT)
        {
            ImGui::SetNextWindowPos(move);
        }
        ImGui::SetNextWindowSize(size);
        return ImGui::Begin(name.empty()? nullptr: name.c_str(), nullptr, flag);
    }
    else
    {
        LockPosition();
        return ImGui::BeginChild(state->Name.c_str(), 
            ImVec2(state->Move.z, state->Move.w), 
            state->IsShowBorder, flag);
    }
}

void UIClassLayout::OnLeave(bool ret)
{
    auto state = GetState<UIStateLayout>();
    if (state->IsWindow)
    {
        ImGui::PopStyleVar();
        if (GetRoot() == this)
        {
            UIMenu::RenderPopup();
        }
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
    auto thisUp     = thisState->Move.y;
    auto thisDown   = thisUp + thisState->Move.w;
    auto thisLeft   = thisState->Move.x;
    auto thisRight  = thisLeft + thisState->Move.z;
    for (auto layout : GetParent()->GetObjects(UITypeEnum::kLAYOUT))
    {
        if (layout == this) { continue; }
        auto state  = layout->GetState<UIStateLayout>();
        auto up     = state->Move.y;
        auto down   = up + state->Move.w;
        auto left   = state->Move.x;
        auto right  = left + state->Move.z;
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
        const auto & edge = thisState->mJoin[(int)direct].mIn;
        if (edge.first != nullptr)
        {
            const auto & move = edge.first->GetState()->Move;
            glm::vec2 offset(0, 0);
            if      ((DirectEnum)direct == DirectEnum::kU && edge.second == DirectEnum::kU) { offset.y = move.y - thisState->Move.y; }
            else if ((DirectEnum)direct == DirectEnum::kU && edge.second == DirectEnum::kD) { offset.y = move.y + move.w - thisState->Move.y; }
            else if ((DirectEnum)direct == DirectEnum::kD && edge.second == DirectEnum::kD) { offset.y = move.y + move.w - thisState->Move.y - thisState->Move.w; }
            else if ((DirectEnum)direct == DirectEnum::kD && edge.second == DirectEnum::kU) { offset.y = move.y - thisState->Move.y - thisState->Move.w; }

            if      ((DirectEnum)direct == DirectEnum::kL && edge.second == DirectEnum::kL) { offset.x = move.x - thisState->Move.x; }
            else if ((DirectEnum)direct == DirectEnum::kL && edge.second == DirectEnum::kR) { offset.x = move.x + move.z - thisState->Move.x; }
            else if ((DirectEnum)direct == DirectEnum::kR && edge.second == DirectEnum::kR) { offset.x = move.x + move.z - thisState->Move.x - thisState->Move.z; }
            else if ((DirectEnum)direct == DirectEnum::kR && edge.second == DirectEnum::kL) { offset.x = move.x - thisState->Move.x - thisState->Move.z; }
            thisState->Move = CalcStretech((DirectEnum)direct, offset);
        }
    }
}

void UIClassLayout::OnRender(float dt)
{
    if (GetState()->IsShowMenuBar)
    {
        UIMenu::BarMenu(this, GetState()->MenuBar);
    }
}

bool UIClassLayout::IsCanStretch(DirectEnum edge)
{
    auto parent = GetParent();
    CHECK_RET(parent != nullptr, false);
    ASSERT_LOG(dynamic_cast<UIClassLayout *>(parent) != nullptr, "");
    CHECK_RET(GetState()->IsCanStretch, false);

    auto cling = GetState<UIStateLayout>()->mJoin[(int)edge].mIn.first != nullptr
               ? GetState<UIStateLayout>()->mJoin[(int)edge].mIn.first : this;
    const auto & clingMove  = cling->GetState()->Move;
    const auto & parentMove = parent->GetState()->Move;
    switch (edge)
    {
    case DirectEnum::kU: return !tools::Equal(clingMove.y, 0);
    case DirectEnum::kD: return !tools::Equal(clingMove.y + clingMove.w, parentMove.w);
    case DirectEnum::kL: return !tools::Equal(clingMove.x, 0);
    case DirectEnum::kR: return !tools::Equal(clingMove.x + clingMove.z, parentMove.z);
    }
    ASSERT_LOG(false, "{0}", (int)edge);
}

bool UIClassLayout::IsCanStretch(DirectEnum edge, const glm::vec2 & offset)
{
    CHECK_RET((edge == DirectEnum::kU || edge == DirectEnum::kD) && offset.y != 0 ||
              (edge == DirectEnum::kL || edge == DirectEnum::kR) && offset.x != 0, false);

    auto thisState = GetState<UIStateLayout>();
    const auto & newMove = CalcStretech(edge, offset);
    const auto & oldMove = thisState->Move;
    const auto & minMove = thisState->StretchMin;
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
        const auto & move = thisState->Move;
        const auto direct = tools::IsOnEdge(
            glm::vec4(world.x, world.y, move.z, move.w),
            glm::vec2(mouse.x, mouse.y), 5);
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
        if (IsCanStretch(rootState->mStretchFocus.mDirect, glm::vec2(offset.x, offset.y)))
        {
            switch ((int)rootState->mStretchFocus.mDirect)
            {
            case (int)DirectEnum::kU: thisState->Move.y += offset.y; break;
            case (int)DirectEnum::kD: thisState->Move.w += offset.y; break;
            case (int)DirectEnum::kL: thisState->Move.x += offset.x; break;
            case (int)DirectEnum::kR: thisState->Move.z += offset.x; break;
            }
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
UIClassTreeBox::UIClassTreeBox() : UIObject(UITypeEnum::kTREEBOX, new UIStateTreeBox())
{ }

bool UIClassTreeBox::OnEnter()
{
    LockPosition();

    auto  state = GetState<UIStateTreeBox>();
    ImGui::SetNextItemWidth(state->Move.z);

    size_t flag = 0;
    if (state->mSelect) { flag |= ImGuiTreeNodeFlags_Selected; }
    if (GetObjects().empty()) { flag |= ImGuiTreeNodeFlags_Leaf; }
    return ImGui::TreeNodeEx(state->Name.c_str(), flag);
}

void UIClassTreeBox::OnLeave(bool ret)
{
    if (ret)
    {
        ImGui::TreePop();
    }
    AdjustSize();
}

// ---
//  UIClassTextBox
// ---
UIClassTextBox::UIClassTextBox() : UIObject(UITypeEnum::kTEXTBOX, new UIStateTextBox())
{ }

void UIClassTextBox::OnRender(float dt)
{
    LockPosition();
    
    auto state = GetState<UIStateTextBox>();
    if (state->IsEditBox)
    {
        auto flag = ImGuiInputTextFlags_EnterReturnsTrue
            | ImGuiInputTextFlags_CallbackResize
            | ImGuiInputTextFlags_AutoSelectAll;
        if (state->IsMulti)
        {
            if (ImGui::InputTextMultiline(
                state->Name.c_str(), 
                state->mBuffer.data(), 
                state->mBuffer.size(), 
                ImVec2(state->Move.z, state->Move.w), 
                flag, &imgui_tools::OnResizeBuffer, &state->mBuffer))
            {
                PostEventMessage(UIEventEnum::kEDIT_TEXT_FINISH, UIEventDetails::EditText(state->mBuffer, this));
            }
        }
        else
        {
            ImGui::PushItemWidth(state->Move.z);
            if (ImGui::InputText(
                state->Name.c_str(), 
                state->mBuffer.data(), 
                state->mBuffer.size(), flag,
                &imgui_tools::OnResizeBuffer, &state->mBuffer))
            {
                PostEventMessage(UIEventEnum::kEDIT_TEXT_FINISH, UIEventDetails::EditText(state->mBuffer, this));
            }
            ImGui::PopItemWidth();
        }
    }
    else
    {
        ImGui::Text(state->Name.c_str());
    }

    AdjustSize();
}

// ---
//  UIClassImage
//----
UIClassImageBox::UIClassImageBox() : UIObject(UITypeEnum::kIMAGEBOX, new UIStateImageBox())
{ }

void UIClassImageBox::OnRender(float dt)
{
    LockPosition();
    auto state = GetState<UIStateImageBox>();
    if (state->IsButton)
    {
        if (!state->LSkin.empty())
        {
            auto & imgSkin = Global::Ref().mAtlasSys->Get(state->LSkin);
            if (ImGui::ImageButton(
                (ImTextureID)imgSkin.mID, ImVec2(state->Move.z, state->Move.w),
                ImVec2(imgSkin.mQuat.x, imgSkin.mQuat.y),
                ImVec2(imgSkin.mQuat.z, imgSkin.mQuat.w), 0))
            {
                PostEventMessage(UIEventEnum::kMOUSE, UIEventDetails::Mouse(3, 0, this));
            }
        }
        else
        {
            if (ImGui::Button(state->Name.c_str(), ImVec2(state->Move.z, state->Move.y)))
            {
                PostEventMessage(UIEventEnum::kMOUSE, UIEventDetails::Mouse(3, 0, this));
            }
        }
    }
    else
    {
        ASSERT_LOG(!state->LSkin.empty(), "");
        auto & imgSkin = Global::Ref().mAtlasSys->Get(state->LSkin);
        ImGui::Image(
            (ImTextureID)imgSkin.mID,
            ImVec2(state->Move.z, state->Move.w),
            ImVec2(imgSkin.mQuat.x, imgSkin.mQuat.y),
            ImVec2(imgSkin.mQuat.z, imgSkin.mQuat.w));
    }

    AdjustSize();
}

// ---
//  UIClassComboBox
// ---
UIClassComboBox::UIClassComboBox() : UIObject(UITypeEnum::kCOMBOBOX, new UIStateComboBox())
{ }

bool UIClassComboBox::OnEnter()
{
    LockPosition();

    auto  state = GetState<UIStateComboBox>();
    ImGui::SetNextItemWidth(state->Move.z);

    if (state->mSelected.empty() && !GetObjects().empty())
    {
        state->mSelected = GetObjects().at(0)->GetState()->Name;
    }
    return ImGui::BeginCombo(state->Name.c_str(), state->mSelected.c_str());
}

void UIClassComboBox::OnLeave(bool ret)
{
    if (ret) { ImGui::EndCombo(); }

    AdjustSize();
}

void UIClassComboBox::OnRender(float dt)
{ }

bool UIClassComboBox::OnCallEventMessage(UIEventEnum e, const UIEventDetails::Base & param)
{
    if (e == UIEventEnum::kMOUSE)
    {
        auto & object = std::any_cast<const UIEventDetails::Mouse &>(param).mObject;
        GetState<UIStateComboBox>()->mSelected = object->GetState()->Name;
        ImGui::CloseCurrentPopup();
    }
    return false;
}

UIClassUICanvas::UIClassUICanvas() : UIObject(UITypeEnum::kUICONVAS, new UIStateUICanvas())
{ }

void UIClassUICanvas::OnRender(float dt)
{ }

UIClassGLCanvas::UIClassGLCanvas() : UIObject(UITypeEnum::kGLCONVAS, new UIStateGLCanvas())
{ }

void UIClassGLCanvas::OnRender(float dt)
{ }
