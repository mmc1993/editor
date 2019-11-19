#include "uievent_delegate_main.h"

bool UIEventDelegateMainObjList::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object)
{
    UIEventDelegateMain::OnCallEventMessage(e, param, object);

    if (UIEventEnum::kDelegate == e)
    {
        auto & arg = (const UIEvent::Delegate &)param;
        if (arg.mType == 0)
        {
            _listener.Add(EventSys::TypeEnum::kOpenProject, std::bind(&UIEventDelegateMainObjList::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kFreeProject, std::bind(&UIEventDelegateMainObjList::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kInsertObject, std::bind(&UIEventDelegateMainObjList::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kDeleteObject, std::bind(&UIEventDelegateMainObjList::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kRenameObject, std::bind(&UIEventDelegateMainObjList::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kSelectObject, std::bind(&UIEventDelegateMainObjList::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kStateObject, std::bind(&UIEventDelegateMainObjList::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
        }
        return true;
    }

    if (!Global::Ref().mEditorSys->IsOpenProject()) { return false; }

    switch (e)
    {
    case UIEventEnum::kMouse:
        {
            auto & mouse = (const UIEvent::Mouse &)param;
            //  右键单击
            if (mouse.mKey == 1 && mouse.mAct == 3)
            {
                std::vector<std::string> buffer;
                if (mouse.mObject == object)
                {
                    buffer.push_back("Add Object");
                    UIMenu::PopMenu(mouse.mObject, buffer);
                }
                else
                {
                    auto & name = mouse.mObject->GetParent()->GetState()->Name;
                    buffer.push_back(        "Add Object");
                    buffer.push_back(        "Del Object");
                    buffer.push_back(SFormat("Rename Object/{0}~", name));

                    std::transform(
                        std::begin(Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")),
                        std::end(  Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")),
                        std::back_inserter(buffer), [](const auto & pair) { return "Add Component/" + pair.mVal->ToString(); });
                    UIMenu::PopMenu(mouse.mObject->GetParent(), buffer);
                }
            }

            //  左键单击
            if (mouse.mKey == 0 && mouse.mAct == 3 && mouse.mObject != object)
            {
                auto objectID = _obj2id.at(mouse.mObject->GetParent());
                if      (mouse.mObject->GetParent()->GetObjects().at(0) == mouse.mObject)
                {
                    //  激活
                    if (auto glo = Global::Ref().mEditorSys->GetProject()->GetObject(objectID); glo->HasState(GLObject::StateEnum::kActive))
                    {
                        Global::Ref().mEditorSys->OptStateSubObject(objectID, GLObject::StateEnum::kActive);
                    }
                    else
                    {
                        Global::Ref().mEditorSys->OptStateAddObject(objectID, GLObject::StateEnum::kActive);
                    }
                }
                else if (mouse.mObject->GetParent()->GetObjects().at(1) == mouse.mObject)
                {
                    //  锁定
                    if (auto glo = Global::Ref().mEditorSys->GetProject()->GetObject(objectID); glo->HasState(GLObject::StateEnum::kLocked))
                    {
                        Global::Ref().mEditorSys->OptStateSubObject(objectID, GLObject::StateEnum::kLocked);
                    }
                    else
                    {
                        Global::Ref().mEditorSys->OptStateAddObject(objectID, GLObject::StateEnum::kLocked);
                    }
                }
            }

            if (    mouse.mObject != object
                &&  (mouse.mKey == 0 || mouse.mKey == 1)
                &&  (mouse.mAct == 2 || mouse.mAct == 3) 
                &&  mouse.mObject->GetParent()->GetObjects().at(2) == mouse.mObject)
            {
                Global::Ref().mEditorSys->OptSelectObject(_obj2id.at(mouse.mObject->GetParent()), true);
            }
        }
        return true;
    case UIEventEnum::kMenu:
        {
            auto & menu = (const UIEvent::Menu &)param;
            if (menu.mPath == "Add Object")
            {
                auto objectID = _obj2id.at(menu.mObject);
                auto parent = Global::Ref().mEditorSys->GetProject()->GetObject(objectID);
                auto object = Global::Ref().mEditorSys->GetProject()->NewObject();
                auto name = Global::Ref().mEditorSys->ObjectName(parent);
                object->SetName(name);
                Global::Ref().mEditorSys->OptInsertObject(object,parent);
            }
            else if (menu.mPath == "Del Object")
            {
                Global::Ref().mEditorSys->OptDeleteObject(_obj2id.at(menu.mObject));
            }
            else if (
                menu.mPath.at(0) == 'R' && menu.mPath.at(1) == 'e' && 
                menu.mPath.at(2) == 'n' && menu.mPath.at(3) == 'a' &&
                menu.mPath.at(4) == 'm' && menu.mPath.at(5) == 'e')
            {
                Global::Ref().mEditorSys->OptRenameObject(_obj2id.at(menu.mObject), menu.mEdit);
            }
            else if (menu.mPath.at(0) == 'A' && menu.mPath.at(1) == 'd' &&
                     menu.mPath.at(2) == 'd' && menu.mPath.at(3) == ' ' &&
                     menu.mPath.at(4) == 'C' && menu.mPath.at(5) == 'o' &&
                     menu.mPath.at(6) == 'm' && menu.mPath.at(7) == 'p')
            {
                Global::Ref().mEditorSys->OptAppendComponent(_obj2id.at(menu.mObject), Component::Create(tools::GetFileFullName(menu.mPath)));
            }
            std::cout
                << "Menu Key: " << menu.mPath << ' '
                << "Menu Edit: " << menu.mEdit << std::endl;
        }
        return true;
    case UIEventEnum::kDrag:
        {
            auto & drag = (const UIEvent::Drag &)param;
            if (drag.mAct == 1 || 
                drag.mAct == 2 && 
                drag.mDragObj->GetParent() == GetOwner())
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

void UIEventDelegateMainObjList::InitObjects(const SharePtr<UIObject>& uiobject, const SharePtr<GLObject>& globject)
{
    for (auto & object : globject->GetObjects())
    {
        auto obj = NewObject(object->GetID(), object->GetName());
        InitObjects(obj, object);
        uiobject->AddObject(obj);
    }
}

SharePtr<UIObject> UIEventDelegateMainObjList::NewObject(uint id, const std::string & name)
{
    auto layout = mmc::Json::Hash();
    layout->Insert(mmc::Json::List(), "__Children");
    layout->Insert(mmc::Json::Hash(), "__Property");
    layout->Insert(mmc::Json::FromValue("0"), "__Property", "Type");
    layout->Insert(mmc::Json::FromValue(name), "__Property", "Name");
    layout->Insert(mmc::Json::FromValue("true"), "__Property", "IsSameline");
    layout->Insert(mmc::Json::FromValue("true"), "__Property", "IsCanDragMove");
    layout->Insert(mmc::Json::FromValue("true"), "__Property", "IsCanDragFree");
    layout->Insert(mmc::Json::FromValue("false"), "__Property", "IsShowBorder");

    //  Active
    auto active = mmc::Json::Hash();
    active->Insert(mmc::Json::List(), "__Children");
    active->Insert(mmc::Json::Hash(), "__Property");
    active->Insert(mmc::Json::FromValue("3"), "__Property", "Type");
    active->Insert(mmc::Json::FromValue("| X |"), "__Property", "Name");
    active->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragMove");
    active->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragFree");
    layout->Insert(active, "__Children", 0);

    //  Locked
    auto locked = mmc::Json::Hash();
    locked->Insert(mmc::Json::List(), "__Children");
    locked->Insert(mmc::Json::Hash(), "__Property");
    locked->Insert(mmc::Json::FromValue("3"), "__Property", "Type");
    locked->Insert(mmc::Json::FromValue("| O |"), "__Property", "Name");
    locked->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragMove");
    locked->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragFree");
    layout->Insert(locked, "__Children", 1);

    //  Tree
    auto tree = mmc::Json::Hash();
    tree->Insert(mmc::Json::List(), "__Children");
    tree->Insert(mmc::Json::Hash(), "__Property");
    tree->Insert(mmc::Json::FromValue("2"), "__Property", "Type");
    tree->Insert(mmc::Json::FromValue(name), "__Property", "Name");
    tree->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragMove");
    tree->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragFree");
    layout->Insert(tree, "__Children", 2);

    auto object = UIParser::Parse(layout);
    _id2obj.insert(std::make_pair(id, object));
    _obj2id.insert(std::make_pair(object, id));
    return object;
}

void UIEventDelegateMainObjList::OnEvent(EventSys::TypeEnum type, const std::any & param)
{
    switch (type)
    {
    case EventSys::TypeEnum::kOpenProject:
        OnEventOpenProject();
        break;
    case EventSys::TypeEnum::kFreeProject:
        OnEventFreeProject();
        break;
    case EventSys::TypeEnum::kInsertObject:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>> &>(param);
            OnEventInsertObject(std::get<0>(value));
        }
        break;
    case EventSys::TypeEnum::kDeleteObject:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>> &>(param);
            OnEventDeleteObject(std::get<0>(value));
        }
        break;
    case EventSys::TypeEnum::kRenameObject:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>,std::string> &>(param);
            OnEventRenameObject(std::get<0>(value), std::get<1>(value));
        }
        break;
    case EventSys::TypeEnum::kStateObject:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>, uint, uint> &>(param);
            OnEventStateObject(std::get<0>(value), std::get<1>(value), std::get<2>(value));
        }
        break;
    case EventSys::TypeEnum::kSelectObject:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>, bool, bool> &>(param);
            OnEventSelectObject(std::get<0>(value), std::get<1>(value), std::get<2>(value));
        }
        break;
    }
}

void UIEventDelegateMainObjList::OnEventOpenProject()
{
    ASSERT_LOG(_obj2id.empty(), "");
    ASSERT_LOG(_id2obj.empty(), "");
    auto root = Global::Ref().mEditorSys->GetProject()->GetRoot();
    _id2obj.insert(std::make_pair(root->GetID(), GetOwner()));
    _obj2id.insert(std::make_pair(GetOwner(), root->GetID()));
    InitObjects(GetOwner(), Global::Ref().mEditorSys->GetProject()->GetRoot());
}

void UIEventDelegateMainObjList::OnEventFreeProject()
{
    _obj2id.clear();
    _id2obj.clear();
    GetOwner()->ClearObjects();
}

void UIEventDelegateMainObjList::OnEventInsertObject(const SharePtr<GLObject> & object)
{
    if (object->GetParent() == Global::Ref().mEditorSys->GetProject()->GetRoot())
    {
        auto uiobject = NewObject(object->GetID(), object->GetName());
        _id2obj.at(object->GetParent()->GetID())->AddObject(uiobject);
    }
    else
    {
        auto parent = _id2obj.at(object->GetParent()->GetID());
        auto uiobject = NewObject(object->GetID(), object->GetName());
        parent->GetObject({ parent->GetState()->Name })->AddObject(uiobject);
    }

    auto stage = CastPtr<UIObjectGLCanvas>(GetOwner()->GetParent()->GetObject({"EditorObjStage", "Canvas"}));
    auto coord = glm::vec2(stage->GetState()->Move.z * 0.5f,
                           stage->GetState()->Move.w * 0.5f);
    coord = object->GetParent()->WorldToLocal(stage->ProjectWorld(coord));
    object->GetTransform()->Position(coord.x, coord.y);
}

void UIEventDelegateMainObjList::OnEventDeleteObject(const SharePtr<GLObject>& object)
{
    auto uiobject = _id2obj.at(object->GetID());
    _id2obj.erase(object->GetID());
    _obj2id.erase(uiobject);
    uiobject->DelThis();
}

void UIEventDelegateMainObjList::OnEventRenameObject(const SharePtr<GLObject>& object, const std::string & name)
{
    auto uiobject0 = _id2obj.at(object->GetID());
    auto uiobject1 = uiobject0->GetObject({name});
    uiobject0->GetState()->Name = object->GetName();
    uiobject1->GetState()->Name = object->GetName();
}

void UIEventDelegateMainObjList::OnEventSelectObject(const SharePtr<GLObject> & object, bool select, bool multi)
{
    auto uiobject = _id2obj.at(object->GetID());
    uiobject->GetObject(
        { uiobject->GetState()->Name }
    )->GetState()->IsSelect = select;
}

void UIEventDelegateMainObjList::OnEventStateObject(const SharePtr<GLObject> & object, uint state0, uint state1)
{
    auto active = object->HasState(GLObject::StateEnum::kActive);
    auto locked = object->HasState(GLObject::StateEnum::kLocked);
    _id2obj.at(object->GetID())->GetObjects().at(0)->GetState()->Color = active ? glm::vec4(1) : glm::vec4(1, 0, 0, 1);
    _id2obj.at(object->GetID())->GetObjects().at(1)->GetState()->Color = locked ? glm::vec4(1, 0, 0, 1) : glm::vec4(1);
}

bool UIEventDelegateMainResList::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object)
{
    UIEventDelegateMain::OnCallEventMessage(e, param, object);
    return true;
}

bool UIEventDelegateMainComList::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object)
{
    UIEventDelegateMain::OnCallEventMessage(e, param, object);

    if (UIEventEnum::kDelegate == e)
    {
        auto & arg = (const UIEvent::Delegate &)param;
        if (arg.mType == 0)
        {
            _listener.Add(EventSys::TypeEnum::kSelectObject, std::bind(
                &UIEventDelegateMainComList::OnEvent, this, 
                std::placeholders::_1, 
                std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kAppendComponent, std::bind(
                &UIEventDelegateMainComList::OnEvent, this,
                std::placeholders::_1,
                std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kDeleteComponent, std::bind(
                &UIEventDelegateMainComList::OnEvent, this,
                std::placeholders::_1,
                std::placeholders::_2));
        }
    }
    return false;
}

void UIEventDelegateMainComList::OnEvent(EventSys::TypeEnum type, const std::any & param)
{
    switch (type)
    {
    case EventSys::TypeEnum::kSelectObject:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>, bool, bool> &>(param);
            OnEventSelectObject(std::get<0>(value), std::get<1>(value), std::get<2>(value));
        }
        break;
    case EventSys::TypeEnum::kAppendComponent:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>, SharePtr<Component>> &>(param);
            OnEventAppendComponent(std::get<0>(value), std::get<1>(value));
        }
        break;
    case EventSys::TypeEnum::kDeleteComponent:
        {
            auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>, SharePtr<Component>> &>(param);
            OnEventDeleteComponent(std::get<0>(value), std::get<1>(value));
        }
        break;
    }
}

void UIEventDelegateMainComList::OnEventSelectObject(const SharePtr<GLObject> & object, bool select, bool multi)
{
    GetOwner()->ClearObjects();

    if (select)
    {
        for (auto component : object->GetComponents())
        {
            auto header = std::create_ptr<UIComponentHeader>(component->GetName());
            GetOwner()->AddObject(header);

            for (auto property : component->CreateUIPropertys())
            {
                header->AddObject(property);
            }
        }
    }
}

void UIEventDelegateMainComList::OnEventAppendComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component)
{
    auto header = std::create_ptr<UIComponentHeader>(component->GetName());
    GetOwner()->AddObject(header);

    for (auto property : component->CreateUIPropertys())
    {
        header->AddObject(property);
    }
}

void UIEventDelegateMainComList::OnEventDeleteComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component)
{
    auto & components = component->GetOwner()->GetComponents();
    auto it = std::find(components.begin(), components.end(), component);
    GetOwner()->DelObject(std::distance(components.begin(), it));
}

bool UIEventDelegateMainStage::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object)
{
    UIEventDelegateMain::OnCallEventMessage(e, param, object);

    if (UIEventEnum::kDelegate == e)
    {
        auto & arg = (const UIEvent::Delegate &)param;
        if (arg.mType == 0)
        {
            _listener.Add(EventSys::TypeEnum::kOpenProject, std::bind(
                &UIEventDelegateMainStage::OnEvent, this,
                std::placeholders::_1,
                std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kFreeProject, std::bind(
                &UIEventDelegateMainStage::OnEvent, this,
                std::placeholders::_1,
                std::placeholders::_2));
            _listener.Add(EventSys::TypeEnum::kSelectObject, std::bind(
                &UIEventDelegateMainStage::OnEvent, this,
                std::placeholders::_1,
                std::placeholders::_2));
        }
        return true;
    }
    return false;
}

void UIEventDelegateMainStage::OnEvent(EventSys::TypeEnum type, const std::any & param)
{
    switch (type)
    {
    case EventSys::TypeEnum::kOpenProject:
        OnEventOpenProject();
        break;
    case EventSys::TypeEnum::kFreeProject:
        OnEventFreeProject();
        break;
    case EventSys::TypeEnum::kSelectObject:
        auto & value = std::any_cast<const std::tuple<SharePtr<GLObject>, bool, bool> &>(param);
        OnEventSelectObject(std::get<0>(value), std::get<1>(value), std::get<2>(value));
        break;
    }
}

void UIEventDelegateMainStage::OnEventOpenProject()
{
}

void UIEventDelegateMainStage::OnEventFreeProject()
{
    CastPtr<UIObjectGLCanvas>(GetOwner())->OpSelectedClear();
}

void UIEventDelegateMainStage::OnEventSelectObject(const SharePtr<GLObject> & object, bool select, bool multi)
{
    CastPtr<UIObjectGLCanvas>(GetOwner())->OpEditObject(nullptr);
    CastPtr<UIObjectGLCanvas>(GetOwner())->OpSelected(object, select);
}

bool UIEventDelegateMainGlobal::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object)
{
    UIEventDelegateMain::OnCallEventMessage(e, param, object);
    
    if (e == UIEventEnum::kMenu)
    {
        auto & menu = (const UIEvent::Menu &)param;
        if (menu.mPath == "Menu/New Project")
        {
            Global::Ref().mEditorSys->OptNewProject("1.proj");
        }
        else if (menu.mPath == "Menu/Open Project")
        {
            Global::Ref().mEditorSys->OptOpenProject("1.proj");
        }
        else if (menu.mPath == "Menu/Save Project")
        {
            Global::Ref().mEditorSys->OptSaveProject("");
        }
        else if (menu.mPath == "Menu/Free Project")
        {
            Global::Ref().mEditorSys->OptFreeProject();
        }
        else if (menu.mPath == "Style/Drak")
        {
            ImGui::StyleColorsDark();
        }
        else if (menu.mPath == "Style/Light")
        {
            ImGui::StyleColorsLight();
        }
        else if (menu.mPath == "Style/Classic")
        {
            ImGui::StyleColorsClassic();
        }
        return true;
    }
    return false;
}
