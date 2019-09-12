#include "test.h"
#include "../property/property_int.h"

bool UIEventDelegateTest::OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param)
{
    if (e == UIEventEnum::kMOUSE)
    {
        auto mouse = (const UIClass::UIEventDetails::Mouse &)param;
        if (mouse.mAct == 3)
        {
            static float n = 0;
            auto ccc = new PropertyFloat(&n, std::to_string(n), [](const float & value, const std::string & title)
                {
                    std::cout
                        << "title: " << title << ' '
                        << "value: " << value << std::endl;
                });
            object->GetChildren({ "C_0" })->AddChild(ccc);
        }
    }
    return true;
}
