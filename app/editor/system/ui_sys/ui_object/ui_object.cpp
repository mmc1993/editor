#include "ui_object.h"
#include "imgui_impl_glfw.h"
#include "../../ui_sys/ui_sys.h"
#include "../../raw_sys/raw_sys.h"
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
        if (it == query->GetObjects().end())
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

void UIObject::InsertObject(const SharePtr<UIObject> & object)
{
    ASSERT_LOG(object->GetParent() == nullptr, "");
    _children.push_back(object);
    object->_parent = this;
}

void UIObject::DeleteObject(const SharePtr<UIObject> & object)
{
    auto it = std::find(_children.begin(), _children.end(), object);
    if (it != _children.end()) 
    { 
        (*it)->_parent = nullptr;
        _children.erase(it); 
    }
}

void UIObject::DeleteObject(size_t index)
{
    DeleteObject(*(_children.begin() + index));
}

void UIObject::DeleteThis()
{
    ASSERT_LOG(GetParent() != nullptr, GetState()->Name.c_str());
    GetParent()->DeleteObject(shared_from_this());
}

void UIObject::ClearObjects()
{
    _children.clear();
}

SharePtr<UIObject> UIObject::GetParent()
{
    return _parent != nullptr
        ? _parent->shared_from_this()
        : nullptr;
}

SharePtr<UIObject> UIObject::GetObject()
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
    state->LSkin_ = state->LSkin;

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
        ASSERT_LOG(thisState->IsWindow, "");
        if (thisState->IsFullScreen)
        {
            const auto & size0 = ImGui_ImplGlfw_GetWindowSize();
            const auto & size1 = thisState->StretchMin;
            thisState->Move.z = std::max(size0.x, size1.x);
            thisState->Move.w = std::max(size0.y, size1.y);
        }
        else
        {
            const auto & size0 = thisState->Move;
            const auto & size1 = thisState->StretchMin;
            thisState->Move.z = std::max(size0.z, size1.x);
            thisState->Move.w = std::max(size0.w, size1.y);
        }
    }
    OnApplyLayout();
}

void UIObject::Render(float dt, bool visible)
{
    auto state = GetState();
    _visible = visible;
    ApplyLayout();

    auto ret = false;
    if (visible)
    { 
        UpdateMove();
        ret = OnEnter();
        UpdateSize();
    }

    if (state->IsSameline && !_children.empty())
    {
        if (_children.size() > 1)
        {
            _children.at(0)->Render(dt, ret);
        }
        for (auto i = 1; i != _children.size(); ++i)
        {
            if (visible) {ImGui::SameLine();}
            _children.at(i)->Render(dt, ret);
        }
    }
    else
    {
        for (auto child : _children)
        {
            child->Render(dt, ret);
        }
    }

    if (visible) { OnLeave(ret); }

    //  刷新备份数据
    state->Move_  = state->Move;
    state->LSkin_ = state->LSkin;
    state->IsFirstRender = false;
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
    while (parent->GetType() == UITypeEnum::kOther
        || parent->GetType() == UITypeEnum::kTextBox
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
    ASSERT_LOG(_delegate == nullptr, "");
    _delegate.reset(delegate);
}

void UIObject::WakeInit(const std::any & param)
{
    auto event = UIEvent::Init(param);
    for (std::deque<SharePtr<UIObject>> list{shared_from_this()}; !list.empty(); list.pop_front())
    {
        auto & front = list.front();
        std::copy(
            front->GetObjects().begin(),
            front->GetObjects().end(),
            std::back_inserter(list));
        front->OnCallEventMessage(UIEventEnum::kInit, event);
    }
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
    auto state = GetState();
    if (!state->IsWindow && (UIAlignEnum)state->Align == UIAlignEnum::kDefault && GetType() != UITypeEnum::kOther)
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
    auto state = GetState<UIStateLayout>();
    if (state->mMouseFocus.mFocus.expired())
    {
        return PostEventMessage(UIEventEnum::kKey, param);
    }
    return state->mMouseFocus.mFocus.lock()->PostEventMessage(UIEventEnum::kKey, param);
}

void UIObject::DispatchEventDrag()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        auto state = GetState<UIStateLayout>();
        if (ImGui::IsMouseClicked(0))
        {
            //  锁定目标
            state->mDrag.mBegWorld = ImGui::GetMousePos();
            state->mDrag.mEndWorld = ImGui::GetMousePos();
            UIEvent::Drag drag(0, state->mDrag.mBegWorld);
            state->mDrag.mDragObj = DispatchEventDrag(drag).get();
        }

        if (state->mDrag.mDragObj != nullptr)
        {
            //  拖动目标
            auto hitFreeObject = DispatchEventDrag(
                UIEvent::Drag(1, state->mDrag.mBegWorld, 
                state->mDrag.mDragObj->shared_from_this()));
            state->mDrag.mEndWorld = ImGui::GetMousePos();
            if (state->mDrag.mFreeObj == nullptr)
            {
                state->mDrag.mFreeObj = hitFreeObject.get();
            }
            else
            {
                auto [distance, direct] = tools::PointToRectEdge(
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

        if (ImGui::IsMouseReleased(0))
        {
            //  释放目标
            auto state = GetState<UIStateLayout>();
            if (state->mDrag.mFreeObj != nullptr)
            {
                ASSERT_LOG(state->mDrag.mDragObj != nullptr, "");
                state->mDrag.mFreeObj->PostEventMessage(UIEventEnum::kDrag, 
                    UIEvent::Drag(2, state->mDrag.mBegWorld, 
                    state->mDrag.mDragObj->shared_from_this(),
                    state->mDrag.mFreeObj->shared_from_this(),
                    state->mDrag.mDirect));
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

    if (IsVisible() && tools::IsContains(ToWorldRect(), param.mEndWorld))
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
    auto state = GetState<UIStateLayout>();
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
        if (state->mMouseFocus.mDown.expired())
        {
            if (ImGui::IsMouseClicked(0)) { state->mMouseFocus.mKey = 0; state->mMouseFocus.mFocus = state->mMouseFocus.mDown = DispatchEventMouse(UIEvent::Mouse(3, 0)); }
            if (ImGui::IsMouseClicked(1)) { state->mMouseFocus.mKey = 1; state->mMouseFocus.mFocus = state->mMouseFocus.mDown = DispatchEventMouse(UIEvent::Mouse(3, 1)); }
            if (ImGui::IsMouseClicked(2)) { state->mMouseFocus.mKey = 2; state->mMouseFocus.mFocus = state->mMouseFocus.mDown = DispatchEventMouse(UIEvent::Mouse(3, 2)); }
        }
        if (state->mMouseFocus.mDown.expired()) { DispatchEventMouse(UIEvent::Mouse(0, -1)); }
    }
    
    if (!state->mMouseFocus.mDown.expired())
    {
        if (ImGui::IsMouseDoubleClicked(0)) { state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(4, 0)); }
        if (ImGui::IsMouseDoubleClicked(1)) { state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(4, 1)); }
        if (ImGui::IsMouseDoubleClicked(2)) { state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(4, 2)); }

        auto hoverKey = -1;
        if (ImGui::IsMouseDown(0)) { state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(1, 0)); hoverKey = 0; }
        if (ImGui::IsMouseDown(1)) { state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(1, 1)); hoverKey = 1; }
        if (ImGui::IsMouseDown(2)) { state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(1, 2)); hoverKey = 2; }
        state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(0, hoverKey));
    }

    if (!state->mMouseFocus.mDown.expired() && ImGui::IsMouseReleased(0))
    {
        state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(2, 0));
        if (state->mMouseFocus.mKey == 0) { state->mMouseFocus.mDown.reset(); }
    }
    if (!state->mMouseFocus.mDown.expired() && ImGui::IsMouseReleased(1))
    {
        state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(2, 1));
        if (state->mMouseFocus.mKey == 1) { state->mMouseFocus.mDown.reset(); }
    }
    if (!state->mMouseFocus.mDown.expired() && ImGui::IsMouseReleased(2))
    {
        state->mMouseFocus.mDown.lock()->PostEventMessage(UIEventEnum::kMouse, UIEvent::Mouse(2, 2));
        if (state->mMouseFocus.mKey == 2) { state->mMouseFocus.mDown.reset(); }
    }
}

SharePtr<UIObject> UIObject::DispatchEventMouse(const UIEvent::Mouse & param)
{
    for (auto child : GetObjects())
    {
        auto result = child->DispatchEventMouse(param);
        if (result != nullptr) { return result; }
    }

    if (IsVisible() && tools::IsContains(ToWorldRect(), param.mMouse))
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
UIObjectLayout::UIObjectLayout() : UIObject(UITypeEnum::kLayout, new UIStateLayout())
{ }

bool UIObjectLayout::OnEnter()
{
    auto state = GetState<UIStateLayout>();

    //  窗口flag
    size_t flag = ImGuiWindowFlags_NoCollapse;
    if ( state->IsShowMenuBar)      { flag |= ImGuiWindowFlags_MenuBar; }
    if (!state->IsShowTitleBar)     { flag |= ImGuiWindowFlags_NoTitleBar; }
    if (!state->IsShowScrollBar)    { flag |= ImGuiWindowFlags_NoScrollbar; }
    if (!state->IsShowScrollBar)    { flag |= ImGuiWindowFlags_NoScrollWithMouse; }
    if ( state->IsShowScrollBar)    { flag |= ImGuiWindowFlags_HorizontalScrollbar; }

    auto ret = false;
    if (state->IsWindow)
    {
        if (!state->IsShowNav)          { flag |= ImGuiWindowFlags_NoNav; }
        if (!state->IsCanMove)          { flag |= ImGuiWindowFlags_NoMove; }
        if (!state->IsCanStretch)       { flag |= ImGuiWindowFlags_NoResize; }

        //  悬浮窗口
        const auto & name = state->Name;
        ImVec2 move = ImVec2(state->Move.x, state->Move.y);
        ImVec2 size = ImVec2(state->Move.z, state->Move.w);

        if (state->IsFullScreen)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            size = ImGui_ImplGlfw_GetWindowSize();
            move.x = 0;
            move.y = 0;
            flag |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                 | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
                 | ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoNav
                 | ImGuiWindowFlags_NoBringToFrontOnFocus;
            ImGui::SetNextWindowPos(move);
            ImGui::SetNextWindowSize(size);
            ret = ImGui::Begin(ImID(name).c_str(), nullptr, flag);
        }
        else
        {
            if (state->IsFirstRender)
            {
                ImGui::SetNextWindowPos(move);
                ImGui::SetNextWindowSize(size);
            }

            bool isOpen = true;
            ret = ImGui::Begin(ImID(name).c_str(), &isOpen, flag);

            //  计算正确的坐标和尺寸(Window通过ImGui管理坐标和尺寸, 在这里只需要更新它就行了)
            if (isOpen)
            {
                move = ImGui::GetWindowPos();
                size = ImGui::GetWindowSize();
                size.x = std::max(state->StretchMin.x, size.x);
                size.y = std::max(state->StretchMin.y, size.y);
                state->Move = glm::vec4(move.x, move.y, size.x, size.y);
                if (size.x <= state->StretchMin.x || size.y <= state->StretchMin.y)
                {
                    size.x = std::max(size.x, state->StretchMin.x);
                    size.y = std::max(size.y, state->StretchMin.y);
                    ImGui::SetWindowSize(size);
                }
            }
            else
            {
                Global::Ref().mUISys->FreeWindow(shared_from_this());
            }

            ret = ret && isOpen;
        }
    }
    else
    {
        ret = ImGui::BeginChild(ImID(state->Name).c_str(),
            ImVec2(state->Move.z, state->Move.w),
            state->IsShowBorder, flag);
    }
    if (ret && GetState()->IsShowMenuBar)
    {
        UIMenu::BarMenu(shared_from_this(), GetState()->MenuBar);
    }
    return ret;
}

void UIObjectLayout::OnLeave(bool ret)
{
    auto state = GetState<UIStateLayout>();
    if (state->IsWindow)
    {
        if (state->IsFullScreen)
        {
            ImGui::PopStyleVar();
        }

        if (GetObject().get() == this)
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

void UIObjectLayout::OnResetLayout()
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

void UIObjectLayout::OnApplyLayout()
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

bool UIObjectLayout::IsCanStretch(DirectEnum edge)
{
    auto parent = GetParent();
    CHECK_RET(parent != nullptr, false);
    ASSERT_LOG(std::dynamic_pointer_cast<UIObjectLayout>(parent) != nullptr, "");
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

bool UIObjectLayout::IsCanStretch(DirectEnum edge, const glm::vec2 & offset)
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
                !((UIObjectLayout *)object)->IsCanStretch((DirectEnum)i, offset))
            { return false; }
        }
    }
    return true;
}

void UIObjectLayout::HandleStretch()
{
    auto thisState =              GetState<UIStateLayout>();
    auto rootState = GetObject()->GetState<UIStateLayout>();
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
//  UIObjectTreeBox
// ---
UIObjectTreeBox::UIObjectTreeBox() : UIObject(UITypeEnum::kTreeBox, new UIStateTreeBox())
{ }

bool UIObjectTreeBox::OnEnter()
{
    auto state = GetState<UIStateTreeBox>();
    ImGui::SetNextItemWidth(state->Move.z);

    size_t flag = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (state->IsSelect) { flag |= ImGuiTreeNodeFlags_Selected; }
    if (GetObjects().empty()) { flag |= ImGuiTreeNodeFlags_Leaf; }
    auto ret = ImGui::TreeNodeEx(state->Name.c_str(), flag /* | ImGuiTreeNodeFlags_Framed*/);
    return ret;
}

void UIObjectTreeBox::OnLeave(bool ret)
{
    if (ret) { ImGui::TreePop(); }
}

// ---
//  UIObjectTextBox
// ---
UIObjectTextBox::UIObjectTextBox() : UIObject(UITypeEnum::kTextBox, new UIStateTextBox())
{ }

bool UIObjectTextBox::OnEnter()
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
        ImGui::PushStyleColor(ImGuiCol_Text, GetState()->Color);
        ImGui::Text(state->Name.c_str());
        ImGui::PopStyleColor();
    }
    return true;
}

// ---
//  UIObjectImageBox
//----
UIObjectImageBox::UIObjectImageBox() : UIObject(UITypeEnum::kImageBox, new UIStateImageBox())
{ }

bool UIObjectImageBox::OnEnter()
{
    auto state = GetState<UIStateImageBox>();
    //  刷新纹理
    if (!state->LSkin.empty() && (state->mSkinTex == nullptr || state->LSkin != state->LSkin_))
    {
        state->mSkinTex = Global::Ref().mRawSys->Get<GLTexture>(state->LSkin);
    }

    if (state->IsButton)
    {
        if (!state->LSkin.empty())
        {
            if (ImGui::ImageButton(
                (ImTextureID)state->mSkinTex->GetID(), 
                ImVec2(state->Move.z, state->Move.w),
                ImVec2(state->mSkinTex->GetOffset().x, state->mSkinTex->GetOffset().y),
                ImVec2(state->mSkinTex->GetOffset().z, state->mSkinTex->GetOffset().w), 0))
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
        ImGui::Image(
            (ImTextureID)state->mSkinTex->GetID(),
            ImVec2(state->Move.z, state->Move.w),
            ImVec2(state->mSkinTex->GetOffset().x, state->mSkinTex->GetOffset().y),
            ImVec2(state->mSkinTex->GetOffset().z, state->mSkinTex->GetOffset().w));
    }
    return true;
}

// ---
//  UIObjectComboBox
// ---
UIObjectComboBox::UIObjectComboBox() : UIObject(UITypeEnum::kComboBox, new UIStateComboBox())
{ }

bool UIObjectComboBox::OnEnter()
{
    auto  state = GetState<UIStateComboBox>();
    ImGui::SetNextItemWidth(state->Move.z);

    if (state->mSelected.empty() && !GetObjects().empty())
    {
        state->mSelected = GetObjects().at(0)->GetState()->Name;
    }
    return ImGui::BeginCombo(state->Name.c_str(), state->mSelected.c_str());
}

void UIObjectComboBox::OnLeave(bool ret)
{
    if (ret) { ImGui::EndCombo(); }
}

bool UIObjectComboBox::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param)
{
    if (e == UIEventEnum::kMouse)
    {
        auto & object = std::any_cast<const UIEvent::Mouse &>(param).mObject;
        GetState<UIStateComboBox>()->mSelected = object->GetState()->Name;
        ImGui::CloseCurrentPopup();
    }
    return false;
}

// ---
//  UIObjectUICanvas
// ---
UIObjectUICanvas::UIObjectUICanvas() : UIObject(UITypeEnum::kUICanvas, new UIStateUICanvas())
{ }

