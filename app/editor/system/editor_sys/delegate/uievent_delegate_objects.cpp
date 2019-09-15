#include "uievent_delegate_objects.h"

bool UIEventDelegateObjects::OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param)
{
    switch (e)
    {
    case UIEventEnum::kMOUSE:
        {
            auto & mouse = (const UIClass::UIEventDetails::Mouse &)param;
            if (mouse.mKey == 1 && mouse.mAct == 3)
            {
                std::vector<std::string> buffer;
                if (mouse.mObject == object)
                {
                    buffer.push_back("Add Object");
                }
                else
                {
                    buffer.push_back("Add Object");
                    buffer.push_back("Del Object");
                    buffer.push_back("Rename Object");
                    buffer.push_back("Add Component");

                    std::transform(
                        std::begin(Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")),
                        std::end(Global::Ref().mCfgSys->At("res/cfg/editor/component.json", "Order")), 
                        std::back_inserter(buffer), [] (const auto & pair) { return "Add Component/" + pair.mValue->ToString(); });
                }
                UIMenu::PopMenu(mouse.mObject, buffer);
            }
        }
        break;
    case UIEventEnum::kMENU:
        {
            auto menu = (const UIClass::UIEventDetails::Menu &)param;
            if (menu.mPath == "Add Object")
            {
                static size_t count = 0;
                auto raw = mmc::JsonValue::Hash();
                raw->Insert(mmc::JsonValue::List(), "__Children");
                raw->Insert(mmc::JsonValue::Hash(), "__Property");
                raw->Insert(mmc::JsonValue::FromValue("2"), "__Property", "Type");
                raw->Insert(mmc::JsonValue::FromValue("0"), "__Property", "Align");
                raw->Insert(mmc::JsonValue::FromValue(SFormat("Object_{0}", count++)), "__Property", "Name");
                menu.mObject->AddChild(UIParser::Parse(raw));
            }
            else if (menu.mPath == "Del Object")
            {
                menu.mObject->DelThis();
            }
            else
            {

            }
            std::cout << "Menu Key" << menu.mPath << std::endl;
        }
        break;
    }
    return true;
}
