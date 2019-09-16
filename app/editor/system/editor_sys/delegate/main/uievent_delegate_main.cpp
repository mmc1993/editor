#include "uievent_delegate_main.h"
#include "../../editor_sys.h"

bool UIEventDelegateMainObjList::OnCallEventMessage(UIObject * object, UIEventEnum e, const UIObject::UIEventDetails::Base & param)
{
    switch (e)
    {
    case UIEventEnum::kMOUSE:
        {
            auto & mouse = (const UIObject::UIEventDetails::Mouse &)param;
            if (mouse.mKey == 1 && mouse.mAct == 3)
            {
                object->GetState()->mPointer = Global::Ref().mEditorSys->mRootObject;

                std::vector<std::string> buffer;
                if (mouse.mObject == object)
                {
                    buffer.push_back("Add Object");
                }
                else
                {
                    auto & name = GetUIData(mouse.mObject->GetState()->mData, Name);
                    buffer.push_back(        "Add Object");
                    buffer.push_back(        "Del Object");
                    buffer.push_back(        "Rename Object");
                    buffer.push_back(SFormat("Rename Object/{0}~", name));
                    buffer.push_back(        "Add Component");

                    std::transform(
                        std::begin(Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")),
                        std::end(Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")),
                        std::back_inserter(buffer), [](const auto & pair) { return "Add Component/" + pair.mValue->ToString(); });
                }
                UIMenu::PopMenu(mouse.mObject, buffer);
            }
        }
        break;
    case UIEventEnum::kMENU:
        {
            auto menu = (const UIObject::UIEventDetails::Menu &)param;
            if (menu.mPath == "Add Object")
            {
                auto insert = (GLObject *)menu.mObject->GetState()->mPointer;
                auto name = Global::Ref().mEditorSys->GenerateObjectName(insert);

                //  Ìí¼ÓObject
                auto glObject = new GLObject();
                insert->AddObject(glObject, name);

                //  Ìí¼ÓUIObject
                auto raw  = mmc::JsonValue::Hash();
                raw->Insert(mmc::JsonValue::List(), "__Children");
                raw->Insert(mmc::JsonValue::Hash(), "__Property");
                raw->Insert(mmc::JsonValue::FromValue("2"), "__Property", "Type");
                raw->Insert(mmc::JsonValue::FromValue("0"), "__Property", "Align");
                raw->Insert(mmc::JsonValue::FromValue(name), "__Property", "Name");
                auto uiObject = UIParser::Parse(raw);
                uiObject->GetState()->mPointer = glObject;
                menu.mObject->AddObject(uiObject);
            }
            else if (menu.mPath == "Del Object")
            {
                auto glObject = (GLObject *)menu.mObject->GetState()->mPointer;
                menu.mObject->DelThis();
                glObject->DelThis();
            }
            else if (
                menu.mPath.at(0) == 'R' && menu.mPath.at(1) == 'e' && 
                menu.mPath.at(2) == 'n' && menu.mPath.at(3) == 'a' &&
                menu.mPath.at(4) == 'm' && menu.mPath.at(5) == 'e')
            {
                auto glObject = (GLObject *)menu.mObject->GetState()->mPointer;
                if (Global::Ref().mEditorSys->CheckRename(glObject, menu.mEdit))
                {
                    SetUIData(menu.mObject->GetState()->mData, Name, menu.mEdit);
                }
            }
            else
            {

            }
            std::cout
                << "Menu Key: " << menu.mPath << ' '
                << "Menu Edit: " << menu.mEdit << std::endl;
        }
        break;
    }
    return true;
}

bool UIEventDelegateMainResList::OnCallEventMessage(UIObject * object, UIEventEnum e, const UIObject::UIEventDetails::Base & param)
{
    std::cout << "Name: " << GetUIData(object->GetState()->mData, Name) << std::endl;
    return true;
}

bool UIEventDelegateMainComList::OnCallEventMessage(UIObject * object, UIEventEnum e, const UIObject::UIEventDetails::Base & param)
{
    std::cout << "Name: " << GetUIData(object->GetState()->mData, Name) << std::endl;
    return true;
}

bool UIEventDelegateMainStage::OnCallEventMessage(UIObject * object, UIEventEnum e, const UIObject::UIEventDetails::Base & param)
{
    std::cout << "Name: " << GetUIData(object->GetState()->mData, Name) << std::endl;
    return true;
}
