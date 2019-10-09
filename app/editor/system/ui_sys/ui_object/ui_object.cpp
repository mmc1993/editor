#include "ui_object.h"
#include "../ui_menu.h"
#include "../../atlas_sys/atlas_sys.h"
#include "imgui_impl_glfw.h"

// ---
//  UIObject
// ---
SharePtr<UIObject> UIObject::GetObject(const std::initializer_list<std::string> & list)
{
    ASSERT_LOG(list.size() != 0, "");
    auto query = shared_from_this();
    for (auto & name : list)
    {
        auto it = std::find_if(
            query->GetObjects().begin(), 
            query->GetObjects().end(), 
            [&name](const SharePtr<UIObject> & object) {
                return object->GetState()->Name == name;
            });
        if (it == _children.end())
        {
            return nullptr;
        }
        query = *it;
    }
    return query;
}

std::vector<SharePtr<UIObject>> UIObject::GetObjects(UITypeEnum type)
{
    std::vector<SharePtr<UIObject>> result;
    std::copy_if(_children.begin(), _children.end(), std::back_inserter(result), 
        [type](const SharePtr<UIObject> & object) 
        { return object->GetType() == type; });
    return std::move(result);
}

std::vector<SharePtr<UIObject>> & UIObject::GetObjects()
{
    return _children;
}

void UIObject::AddObject(const SharePtr<UIObject> & object)
{
    ASSERT_LOG(object->GetParent() == nullptr, "");
    object->_parent = shared_from_this();
    _children.push_back(object);
}

void UIObject::DelObject(const SharePtr<UIObject> & object)
{
    auto it = std::find(_children.begin(), _children.end(), object);
    if (it != _children.end()) 
    { 
        _children.erase(it); 
    }
}

void UIObject::DelObject(size_t index)
{
    DelObject(*(_children.begin() + index));
}

void UIObject::DelThis()
{
    ASSERT_LOG(GetParent() != nullptr, GetState()->Name.c_str());
    GetParent()->DelObject(shared_from_this());
}

void UIObject::ClearObjects()
{
    _children.clear();
}

SharePtr<UIObject> UIObject::GetParent()
{
    if (_parent.expired())
    {
        return nullptr;
    }
    return _parent.lock();
}

SharePtr<UIObject> UIObject::GetRoot()
{
    auto ret = shared_from_this();
    while (ret->GetParent())
    {
        ret = ret->GetParent();
    }
    return ret;
}

UITypeEnum UIObject::GetType()
{
    return _type;
}

bool UIObject::IsVisible()
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
            ASSERT_LOG((align & ((int)UIAlignEnum::kClingT | (int)UIAlignEnum::kClingB)) == 0
                    || (align & (int)UIAlignEnum::kCenterV)  == 0
                    && (align & (int)UIAlignEnum::kStretchV) == 0, "{0}", align);
            //  左右贴边, 排斥水平居中, 水平拉伸
            ASSERT_LOG((align & ((int)UIAlignEnum::kClingL | (int)UIAlignEnum::kClingR)) == 0
                    || (align & (int)UIAlignEnum::kCenterH)  == 0
                    && (align & (int)UIAlignEnum::kStretchH) == 0, "{0}", align);
            //  水平居中, 排斥水平拉伸
            ASSERT_LOG((align & (int)UIAlignEnum::kCenterH) == 0
                    || (align & (int)UIAlignEnum::kStretchH) == 0, "{0}", align);
            //  垂直居中, 排斥垂直拉伸
            ASSERT_LOG((align & (int)UIAlignEnum::kCenterV) == 0
                    || (align & (int)UIAlignEnum::kStretchV) == 0, "{0}", align);
            if (align & (int)UIAlignEnum::kClingB)
            {
                margin.w += parentMoveNew.w - parentMoveOld.w;
                if ((align & (int)UIAlignEnum::kClingT) == 0) { margin.y = margin.w - move.w; }
            }
            if (align & (int)UIAlignEnum::kClingR)
            {
                margin.z += parentMoveNew.z - parentMoveOld.z;
                if ((align & (int)UIAlignEnum::kClingL) == 0) { margin.x = margin.z - move.z; }
            }
            if (align & (int)UIAlignEnum::kCenterH)
            {
                margin.x = (parentMoveNew.z - (margin.z - margin.x)) * 0.5f;
                margin.z = margin.x + move.z;
            }
            if (align & (int)UIAlignEnum::kCenterV)
            {
                margin.y = (parentMoveNew.w - (margin.w - margin.y)) * 0.5f;
                margin.w = margin.y + move.w;
            }
            if (align & (int)UIAlignEnum::kStretchH)
            {
                margin.x = (margin.x / parentMoveOld.z * parentMoveNew.z);
                margin.z = (margin.z / parentMoveOld.z * parentMoveNew.z);
            }
            if (align & (int)UIAlignEnum::kStretchV)
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
        UpdateMove();
        ret = OnEnter();
        UpdateSize();
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


glm::vec4 UIObject::CalcStretech(DirectEnum direct, const glm::vec2 & offset)
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

glm::vec2 UIObject::ToWorldCoord(const glm::vec2 & coord)
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

glm::vec4 UIObject::ToLocalCoord(const glm::vec4 & coord)
{
    const auto & world = ToWorldCoord();
    return glm::vec4(coord.x - world.x,
                     coord.y - world.y,
                     coord.z, coord.w);
}

glm::vec2 UIObject::ToLocalCoord(const glm::vec2 & coord)
{
    const auto & world = ToWorldCoord();
    return glm::vec2(coord.x - world.x,
                     coord.y - world.y);
}

glm::vec2 UIObject::ToLocalCoordFromImGUI()
{
    //  对于内部没有ImGui::Begin的组件, 需要使用这个接口返回ImGui::GetCursorPos
    auto pos = ImGui::GetCursorPos();
    auto parent = GetParent();
    while (parent->GetType() == UITypeEnum::kTextBox
        || parent->GetType() == UITypeEnum::kTreeBox
        || parent->GetType() == UITypeEnum::kImageBox)
    {
        pos.x -= parent->GetState()->Move.x;
        pos.y -= parent->GetState()->Move.y;
        parent = parent->GetParent();
    }
    return pos;
}

glm::vec4 UIObject::ToWorldRect()
{
    return glm::vec4(ToWorldCoord(), GetState()->Move.z, GetState()->Move.w);
}

void UIObject::BindDelegate(UIEvent::DelegateHandler * delegate)
{
    if (_delegate != nullptr) { _delegate->OnCallEventMessage(UIEventEnum::kDelegate, UIEvent::Delegate(1), shared_from_this()); }
    _delegate.reset(delegate);
    if (_delegate != nullptr) { _delegate->OnCallEventMessage(UIEventEnum::kDelegate, UIEvent::Delegate(0), shared_from_this()); }
}

void UIObject::RenderDrag()
{
    //  拖动节点
    //  释放节点
    //  上个节点

    //  可拖动
    //      1. IsCanDragMove

    //  可释放
    //      1. IsCanDragFree
    //      2. CallEventFunc

    //  上个节点:
    //      1. 当节点为空, 将节点重置为释放节点
    //      2. 当前位置距离节点超过N 像素, 置空

    //  释放逻辑:
    //      1. 当前位置处于 上个节点左|上方, 插入在前
    //      2. 当前位置处于 上个节点右|下方, 插入在后

    //  不可释放
    //      1. 释放节点 == null
    //      2. 上个节点 == null

    static const auto LINE_COLOR_SELECT_OK = ImColor(1.0f, 1.0f, 1.0f);
    static const auto LINE_COLOR_SELECT_NO = ImColor(1.0f, 0.0f, 0.0f);
    static const auto DRAG_COLOR = ImColor(1.0f, 1.0f, 1.0f);
    static const auto FREE_COLOR = ImColor(0.0f, 1.0f, 0.0f);
    static const auto LAST_COLOR = ImColor(0.0f, 0.0f, 1.0f);

    auto state = GetState<UIStateLayout>();
    if (state->mDrag.mDragObj != nullptr)
    {
        auto drawList = ImGui::GetWindowDrawList();
        //  绘制拖动节点
        const auto & rect = state->mDrag.mDragObj->ToWorldRect();
        drawList->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.z, rect.y + rect.w), DRAG_COLOR, 3);

        //  绘制目标节点
        if      (state->mDrag.mFreeObj != nullptr)
        {
            if (state->mDrag.mDirect == DirectEnum::kNone)
            {
                const auto & rect = state->mDrag.mFreeObj->ToWorldRect();
                drawList->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.z, rect.y + rect.w), FREE_COLOR, 3);
            }
            else
            {
                const auto & rect = state->mDrag.mFreeObj->ToWorldRect();
                drawList->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.z, rect.y + rect.w), LAST_COLOR, 3);
            }
        }

        auto color = state->mDrag.mFreeObj != nullptr
            ? LINE_COLOR_SELECT_OK
            : LINE_COLOR_SELECT_NO;
        drawList->AddLine(state->mDrag.mBegWorld, state->mDrag.mEndWorld, color, 3);
    }
}

void UIObject::UpdateSize()
{
    if ((UIAlignEnum)GetState()->Align == UIAlignEnum::kDefault)
    {
        const auto & size = ImGui::GetItemRectSize();
        GetState()->Move.z = size.x;
        GetState()->Move.w = size.y;
    }
}

void UIObject::UpdateMove()
{
    if (!GetState()->IsWindow)
    {
        if ((UIAlignEnum)GetState()->Align != UIAlignEnum::kDefault)
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
}

bool UIObject::OnEnter()
{
    return true;
}

void UIObject::OnLeave(bool ret)
{ }

void UIObject::OnResetLayout()
{ }

void UIObject::OnApplyLayout()
{ }

void UIObject::DispatchEventKey()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        for (auto key : UIEvent::Key::Hotkeys)
        {
            if (ImGui::IsKeyDown(key)) { DispatchEventKey(UIEvent::Key(0, key)); }
            if (ImGui::IsKeyReleased(key)) { DispatchEventKey(UIEvent::Key(1, key)); }
            if (ImGui::IsKeyPressed(key, false)) { DispatchEventKey(UIEvent::Key(2, key)); }
        }
    }
}

SharePtr<UIObject> UIObject::DispatchEventKey(const UIEvent::Key & param)
{
    std::vector<SharePtr<UIObject>> objects{ shared_from_this() };
    for (auto i = 0; objects.size() != i; ++i)
    {
        std::copy(
            objects.at(i)->GetObjects().begin(),
            objects.at(i)->GetObjects().end(),
            std::back_inserter(objects));
    }
    for (auto it = objects.rbegin(); it != objects.rend(); ++it)
    {
        auto result = (*it)->PostEventMessage(UIEventEnum::kKey, param);
        if (result != nullptr) { return result; }
    }
    return nullptr;
}

void UIObject::DispatchEventDrag()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsMouseDown(0))
        {
            auto state = GetState<UIStateLayout>();
            if (state->mDrag.mDragObj == nullptr)
            {
                //  锁定目标
                state->mDrag.mBegWorld = ImGui::GetMousePos();
                state->mDrag.mEndWorld = ImGui::GetMousePos();
                UIEvent::Drag drag(0, state->mDrag.mBegWorld);
                state->mDrag.mDragObj = DispatchEventDrag(drag).get();
            }
            else
            {
                //  拖动目标
                auto hitFreeObject = DispatchEventDrag(
                    UIEvent::Drag(1, state->mDrag.mBegWorld, 
                    state->mDrag.mDragObj->shared_from_this()));

                //  当前没有释放节点
                //  位置在新节点内部
                //  脱离旧释放节点

                state->mDrag.mEndWorld = ImGui::GetMousePos();
                if (state->mDrag.mFreeObj == nullptr)
                {
                    state->mDrag.mFreeObj = hitFreeObject.get();
                }
                else
                {
                    auto [distance, direct] = tools::RectInDistance(
                        state->mDrag.mFreeObj->ToWorldRect(),
                        state->mDrag.mEndWorld);
                    if (distance == 0 || distance > 10)
                    {
                        //  在目标内|脱离目标
                        state->mDrag.mFreeObj = hitFreeObject.get();
                        state->mDrag.mDirect = DirectEnum::kNone;
                    }
                    else
                    {
                        //  目标边缘
                        state->mDrag.mDirect = (DirectEnum)direct;
                    }
                }
            }
        }
        if (ImGui::IsMouseReleased(0))
        {
            //  释放目标
            auto state = GetState<UIStateLayout>();
            if (state->mDrag.mFreeObj != nullptr)
            {
                ASSERT_LOG(state->mDrag.mDragObj != nullptr, "");
                state->mDrag.mFreeObj->PostEventMessage(UIEventEnum::kDrag, 
                    UIEvent::Drag(2, state->mDrag.mBegWorld, 
                    state->mDrag.mDragObj->shared_from_this()));
            }
            state->mDrag.mDragObj = nullptr;
            state->mDrag.mFreeObj = nullptr;
            state->mDrag.mDirect = DirectEnum::kNone;
        }
    }
}

SharePtr<UIObject> UIObject::DispatchEventDrag(const UIEvent::Drag & param)
{
    for (auto child : GetObjects())
    {
        auto result = child->DispatchEventDrag(param);
        if (result != nullptr) { return result; }
    }

    if (IsVisible() && tools::IsContain(ToWorldRect(), param.mEndWorld))
    {
        ASSERT_LOG(param.mAct == 0 || param.mAct == 1, "");
        if (param.mAct == 0 && GetState()->IsCanDragMove)
        {
            return GetState()->IsCanDragMove ? shared_from_this() : nullptr;
        }
        else if (param.mAct == 1)
        {
            return GetState()->IsCanDragFree ? shared_from_this()
                : PostEventMessage(UIEventEnum::kDrag, param);
        }
    }
    return nullptr;
}

void UIObject::DispatchEventMouse()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsMouseReleased(0)) { DispatchEventMouse(UIEvent::Mouse(2, 0)); }
        if (ImGui::IsMouseReleased(1)) { DispatchEventMouse(UIEvent::Mouse(2, 1)); }
        if (ImGui::IsMouseReleased(2)) { DispatchEventMouse(UIEvent::Mouse(2, 2)); }

        if (ImGui::IsMouseClicked(0)) { DispatchEventMouse(UIEvent::Mouse(3, 0)); }
        if (ImGui::IsMouseClicked(1)) { DispatchEventMouse(UIEvent::Mouse(3, 1)); }
        if (ImGui::IsMouseClicked(2)) { DispatchEventMouse(UIEvent::Mouse(3, 2)); }

        if (ImGui::IsMouseDoubleClicked(0)) { DispatchEventMouse(UIEvent::Mouse(4, 0)); }
        if (ImGui::IsMouseDoubleClicked(1)) { DispatchEventMouse(UIEvent::Mouse(4, 1)); }
        if (ImGui::IsMouseDoubleClicked(2)) { DispatchEventMouse(UIEvent::Mouse(4, 2)); }

        auto hoverKey = -1;
        if (ImGui::IsMouseDown(0)) { DispatchEventMouse(UIEvent::Mouse(1, 0)); hoverKey = 0; }
        if (ImGui::IsMouseDown(1)) { DispatchEventMouse(UIEvent::Mouse(1, 1)); hoverKey = 1; }
        if (ImGui::IsMouseDown(2)) { DispatchEventMouse(UIEvent::Mouse(1, 2)); hoverKey = 2; }
        DispatchEventMouse(UIEvent::Mouse(0, hoverKey));
    }
}

SharePtr<UIObject> UIObject::DispatchEventMouse(const UIEvent::Mouse & param)
{
    for (auto child : GetObjects())
    {
        auto result = child->DispatchEventMouse(param);
        if (result != nullptr) { return result; }
    }

    if (IsVisible() && tools::IsContain(ToWorldRect(), param.mMouse))
    {
        return PostEventMessage(UIEventEnum::kMouse, param);
    }
    return nullptr;
}

bool UIObject::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param)
{
    return false;
}

SharePtr<UIObject> UIObject::CallEventMessage(UIEventEnum e, const UIEvent::Event & param)
{
    auto was = OnCallEventMessage(e, param);

    if (_delegate != nullptr)
    {
        was = _delegate->OnCallEventMessage(e, param, shared_from_this()) || was;
    }
    
    if (!was && GetParent() != nullptr)
    {
        return GetParent()->CallEventMessage(e, param);
    }
    return was ? shared_from_this() : nullptr;
}

SharePtr<UIObject> UIObject::PostEventMessage(UIEventEnum e, const UIEvent::Event & param)
{
    param.mObject = shared_from_this();
    return CallEventMessage(e, param);
}

// ---
//  Layout
// ---
UIClassLayout::UIClassLayout() : UIObject(UITypeEnum::kLayout, new UIStateLayout())
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
    if ( state->IsShowScrollBar)    { flag |= ImGuiWindowFlags_HorizontalScrollbar; }

    auto ret = false;
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

        if (state->IsFullScreen || (UIAlignEnum)state->Align != UIAlignEnum::kDefault)
        {
            ImGui::SetNextWindowPos(move);
        }
        ImGui::SetNextWindowSize(size);
        ret = ImGui::Begin(name.empty()? nullptr: name.c_str(), nullptr, flag);
    }
    else
    {
        ret = ImGui::BeginChild(state->Name.c_str(), 
            ImVec2(state->Move.z, state->Move.w), 
            state->IsShowBorder, flag);
    }
    if (GetState()->IsShowMenuBar)
    {
        UIMenu::BarMenu(shared_from_this(), GetState()->MenuBar);
    }
    return ret;
}

void UIClassLayout::OnLeave(bool ret)
{
    auto state = GetState<UIStateLayout>();
    if (state->IsWindow)
    {
        ImGui::PopStyleVar();
        if (GetRoot().get() == this)
        {
            UIMenu::RenderPopup();
        }
        DispatchEventMouse();
        DispatchEventDrag();
        DispatchEventKey();
        RenderDrag();
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
    for (auto layout : GetParent()->GetObjects(UITypeEnum::kLayout))
    {
        if (layout.get() == this) { continue; }
        auto state  = layout->GetState<UIStateLayout>();
        auto up     = state->Move.y;
        auto down   = up + state->Move.w;
        auto left   = state->Move.x;
        auto right  = left + state->Move.z;
        if ((thisUp == up || thisUp == down) && thisState->mJoin[(size_t)DirectEnum::kU].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kU].mOut.push_back(layout.get());
            auto dir = thisUp == up ? DirectEnum::kU : DirectEnum::kD;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kU;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
        if ((thisDown == down || thisDown == up) && thisState->mJoin[(size_t)DirectEnum::kD].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kD].mOut.push_back(layout.get());
            auto dir = thisDown == down ? DirectEnum::kD : DirectEnum::kU;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kD;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
        if ((thisLeft == left || thisLeft == right) && thisState->mJoin[(size_t)DirectEnum::kL].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kL].mOut.push_back(layout.get());
            auto dir = thisLeft == left ? DirectEnum::kL : DirectEnum::kR;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kL;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
        if ((thisRight == right || thisRight == left) && thisState->mJoin[(size_t)DirectEnum::kR].mIn.first == nullptr)
        {
            thisState->mJoin[(size_t)DirectEnum::kR].mOut.push_back(layout.get());
            auto dir = thisRight == right ? DirectEnum::kR : DirectEnum::kL;
            state->mJoin[(size_t)dir].mIn.second = DirectEnum::kR;
            state->mJoin[(size_t)dir].mIn.first = this;
        }
    }
}

void UIClassLayout::OnApplyLayout()
{
    auto thisState = GetState<UIStateLayout>();
    for (auto direct = 0; direct != (int)DirectEnum::Length; ++direct)
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

bool UIClassLayout::IsCanStretch(DirectEnum edge)
{
    auto parent = GetParent();
    CHECK_RET(parent != nullptr, false);
    ASSERT_LOG(std::dynamic_pointer_cast<UIClassLayout>(parent) != nullptr, "");
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
        for (auto i = 0; i != (int)DirectEnum::Length; ++i)
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
        const auto direct = tools::IsOnRect(
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
UIClassTreeBox::UIClassTreeBox() : UIObject(UITypeEnum::kTreeBox, new UIStateTreeBox())
{ }

bool UIClassTreeBox::OnEnter()
{
    auto  state = GetState<UIStateTreeBox>();
    ImGui::SetNextItemWidth(state->Move.z);

    size_t flag = 0;
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
    if (state->IsSelect) { flag |= ImGuiTreeNodeFlags_Selected; }
    if (GetObjects().empty()) { flag |= ImGuiTreeNodeFlags_Leaf; }
    auto ret = ImGui::TreeNodeEx(state->Name.c_str(), flag | ImGuiTreeNodeFlags_Framed);
    ImGui::PopStyleColor();
    return ret;
}

void UIClassTreeBox::OnLeave(bool ret)
{
    if (ret) { ImGui::TreePop(); }
}

// ---
//  UIClassTextBox
// ---
UIClassTextBox::UIClassTextBox() : UIObject(UITypeEnum::kTextBox, new UIStateTextBox())
{ }

bool UIClassTextBox::OnEnter()
{
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
                PostEventMessage(UIEventEnum::kEditTextFinish, UIEvent::EditText(state->mBuffer, shared_from_this()));
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
                PostEventMessage(UIEventEnum::kEditTextFinish, UIEvent::EditText(state->mBuffer, shared_from_this()));
            }
            ImGui::PopItemWidth();
        }
    }
    else
    {
        ImGui::Text(state->Name.c_str());
    }
    return true;
}

// ---
//  UIClassImage
//----
UIClassImageBox::UIClassImageBox() : UIObject(UITypeEnum::kImageBox, new UIStateImageBox())
{ }

bool UIClassImageBox::OnEnter()
{
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
                PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(3, 0, shared_from_this()));
            }
        }
        else
        {
            if (ImGui::Button(state->Name.c_str(), ImVec2(state->Move.z, state->Move.y)))
            {
                PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(3, 0, shared_from_this()));
            }
        }
    }
    else
    {
        ASSERT_LOG(!state->LSkin.empty(), "");
        auto & imgSkin = Global::Ref().mAtlasSys->Get(state->LSkin);
        ImGui::Image((ImTextureID)imgSkin.mID,
            ImVec2(state->Move.z, state->Move.w),
            ImVec2(imgSkin.mQuat.x, imgSkin.mQuat.y),
            ImVec2(imgSkin.mQuat.z, imgSkin.mQuat.w));
    }
    return true;
}

// ---
//  UIClassComboBox
// ---
UIClassComboBox::UIClassComboBox() : UIObject(UITypeEnum::kComboBox, new UIStateComboBox())
{ }

bool UIClassComboBox::OnEnter()
{
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
}

bool UIClassComboBox::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param)
{
    if (e == UIEventEnum::kMouse)
    {
        auto & object = std::any_cast<const UIEvent::Mouse &>(param).mObject;
        GetState<UIStateComboBox>()->mSelected = object->GetState()->Name;
        ImGui::CloseCurrentPopup();
    }
    return false;
}

UIClassUICanvas::UIClassUICanvas() : UIObject(UITypeEnum::kUICanvas, new UIStateUICanvas())
{ }

UIClassGLCanvas::UIClassGLCanvas() : UIObject(UITypeEnum::kGLCanvas, new UIStateGLCanvas())
{ }

void UIClassGLCanvas::HandlePostCommands()
{ }

void UIClassGLCanvas::HandlePreCommands()
{ }

void UIClassGLCanvas::Post(const UIStateGLCanvas::PreCommand & cmd)
{
    GetState<UIStateGLCanvas>()->mPreCommands.push_back(cmd);
}

void UIClassGLCanvas::Post(const UIStateGLCanvas::PostCommand & cmd)
{ 
    GetState<UIStateGLCanvas>()->mPostCommands.push_back(cmd);
}

glm::mat4 UIClassGLCanvas::GetMatrixMVP()
{
    return GetMatrixProj()
         * GetMatrixView()
         * GetMatrixModel();
}

const glm::mat4 & UIClassGLCanvas::GetMatrixView()
{
    return GetState<UIStateGLCanvas>()->mMatrixStack[(size_t)UIStateGLCanvas::MatrixTypeEnum::kView].top();
}

const glm::mat4 & UIClassGLCanvas::GetMatrixProj()
{
    return GetState<UIStateGLCanvas>()->mMatrixStack[(size_t)UIStateGLCanvas::MatrixTypeEnum::kProj].top();
}

const glm::mat4 & UIClassGLCanvas::GetMatrixModel()
{
    return GetState<UIStateGLCanvas>()->mMatrixStack[(size_t)UIStateGLCanvas::MatrixTypeEnum::kModel].top();
}

void UIClassGLCanvas::HandleCommands()
{
    auto state = GetState<UIStateGLCanvas>();
    HandlePreCommands();
    state->mPreCommands.clear();
    HandlePostCommands();
    state->mPostCommands.clear();
}
