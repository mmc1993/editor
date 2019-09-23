#include "uievent_delegate_main.h"

bool UIEventDelegateMainObjList::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object)
{
    if (!Global::Ref().mEditorSys->IsOpenProject())
    {
        return false;
    }

    switch (e)
    {
    case UIEventEnum::kMOUSE:
        {
            auto & mouse = (const UIEvent::Mouse &)param;
            if (mouse.mKey == 1 && mouse.mAct == 3)
            {
                object->GetState()->Pointer = Global::Ref().mEditorSys->mProject->mRoot;

                std::vector<std::string> buffer;
                if (mouse.mObject == object)
                {
                    buffer.push_back("Add Object");
                }
                else
                {
                    auto & name = mouse.mObject->GetState()->Name;
                    buffer.push_back(        "Add Object");
                    buffer.push_back(        "Del Object");
                    buffer.push_back(SFormat("Rename Object/{0}~", name));

                    std::transform(
                        std::begin(Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")),
                        std::end(Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")),
                        std::back_inserter(buffer), [](const auto & pair) { return "Add Component/" + pair.mValue->ToString(); });
                }
                UIMenu::PopMenu(mouse.mObject, buffer);
            }

            if ((mouse.mKey == 0 || mouse.mKey == 1) && 
                (mouse.mAct == 2 || mouse.mAct == 3) && 
                mouse.mObject != object)
            {
                //  选中节点, 通知
                Global::Ref().mEditorSys->mSelected.clear();
                Global::Ref().mEditorSys->mSelected.push_back(mouse.mObject);
                Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSELECT_GLOBJECT, nullptr);
            }
        }
        break;
    case UIEventEnum::kMENU:
        {
            auto menu = (const UIEvent::Menu &)param;
            if (menu.mPath == "Add Object")
            {
                auto glObject = (GLObject *)menu.mObject->GetState()->Pointer;
                auto name = Global::Ref().mEditorSys->ObjectName(glObject);

                //  添加GLObject
                auto newGLObject = new GLObject();
                glObject->AddObject(newGLObject, name);

                //  添加UIObject
                auto raw  = mmc::JsonValue::Hash();
                raw->Insert(mmc::JsonValue::List(), "__Children");
                raw->Insert(mmc::JsonValue::Hash(), "__Property");
                raw->Insert(mmc::JsonValue::FromValue("2"), "__Property", "Type");
                raw->Insert(mmc::JsonValue::FromValue("0"), "__Property", "Align");
                raw->Insert(mmc::JsonValue::FromValue(name), "__Property", "Name");
                auto newUIObject = UIParser::Parse(raw);
                menu.mObject->AddObject(newUIObject);
                
                newUIObject->GetState()->Pointer = newGLObject;
            }
            else if (menu.mPath == "Del Object")
            {
                auto delGLObject = (GLObject *)menu.mObject->GetState()->Pointer;
                menu.mObject->DelThis();
                delGLObject->DelThis();

                //  删除节点, 通知
                Global::Ref().mEditorSys->mSelected.clear();
                Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSELECT_GLOBJECT, nullptr);
            }
            else if (
                menu.mPath.at(0) == 'R' && menu.mPath.at(1) == 'e' && 
                menu.mPath.at(2) == 'n' && menu.mPath.at(3) == 'a' &&
                menu.mPath.at(4) == 'm' && menu.mPath.at(5) == 'e')
            {
                auto renameGLObject = (GLObject *)menu.mObject->GetState()->Pointer;
                if (Global::Ref().mEditorSys->ObjectName(renameGLObject, menu.mEdit))
                {
                    menu.mObject->GetState()->Name = menu.mEdit;
                }
            }
            else
            {
                auto name = menu.mPath.substr(std::strlen("Add Component/"));
                auto glObject = (GLObject*)menu.mObject->GetState()->Pointer;
                glObject->AddComponent(Component::Create(name));

                Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSELECT_GLOBJECT, nullptr);
            }
            std::cout
                << "Menu Key: " << menu.mPath << ' '
                << "Menu Edit: " << menu.mEdit << std::endl;
        }
        break;
    }
    return true;
}

bool UIEventDelegateMainResList::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object)
{
    return true;
}

bool UIEventDelegateMainComList::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object)
{
    if (UIEventEnum::kDELEGATE == e)
    {
        auto & arg = (const UIEvent::Delegate &)param;
        if (arg.mType == 0)
        {
            _onwer = object;

            _listener.Add(EventSys::TypeEnum::kSELECT_GLOBJECT, std::bind(&UIEventDelegateMainComList::OnRefreshComponent, this));
        }
    }
    return true;
}

void UIEventDelegateMainComList::OnRefreshComponent()
{ 
    _onwer->ClearObjects();

    //  清理
    if (!Global::Ref().mEditorSys->mSelected.empty())
    {
        auto selUIObject = Global::Ref().mEditorSys->mSelected.at(0);
        auto selGLObject = (GLObject *)selUIObject->GetState()->Pointer;
        for (auto component : selGLObject->GetComponents())
        {
            auto header = new UIComponentHeader(component->GetName());
            _onwer->AddObject(header);

            auto modifyfunc = std::bind(
                &Component::OnModifyProperty, component, 
                std::placeholders::_1, 
                std::placeholders::_2, 
                std::placeholders::_3);
            auto propertys = component->CollectProperty();
            for (const auto & property : propertys)
            {
                switch (property.mType)
                {
                case Interface::Serializer::StringValueTypeEnum::kVEC2:
                    header->AddObject(new UIPropertyVector2(*(glm::vec2 *)property.mMember, property.mName, modifyfunc));
                    break;
                case Interface::Serializer::StringValueTypeEnum::kFLOAT:
                    header->AddObject(new UIPropertyFloat(*(float *)property.mMember, property.mName, modifyfunc));
                    break;
                }
            }
        }
    }
}

bool UIEventDelegateMainStage::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object)
{
    return true;
}
