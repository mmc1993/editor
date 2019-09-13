#include "test.h"
#include "../property/property.h"

bool UIEventDelegateTest::OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param)
{
    if (e == UIEventEnum::kMOUSE)
    {
        auto mouse = (const UIClass::UIEventDetails::Mouse &)param;
        if (mouse.mAct == 3)
        {
            static size_t v = 0;
            static std::pair<size_t *, std::vector<std::string>> n = {
                &v, {"1", "2", "3"}
            };
            auto ccc = new PropertyCombo(&n, "aaa", [](const std::pair<size_t *, std::vector<std::string>>  & value, const std::string & title)
                {
                    std::cout
                        << "title: " << title << ' '
                        << "value: " << *value.first << std::endl;
                });
            object->GetChildren({ "C_0" })->AddChild(ccc);
        }
    }
    return true;
}
