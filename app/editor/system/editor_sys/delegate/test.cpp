#include "test.h"
#include "../property/property.h"

bool UIEventDelegateTest::OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param)
{
    if (e == UIEventEnum::kMOUSE)
    {
        auto mouse = (const UIClass::UIEventDetails::Mouse &)param;
        if (mouse.mAct == 3)
        {
            static int n = 0;


            static glm::vec4 v4;
            auto ccc3 = new PropertyColor4(v4, std::to_string(n++), [] (const std::any & value, const std::string & title, const std::any & backup)
                {
                    std::cout
                        << "title: " << title << ' '
                        << "value: " << std::any_cast<glm::vec4>(value).x << std::endl;
                    return true;
                });
            object->GetObjects({ "C_0" })->AddObject(ccc3);



            //static int i;
            //auto ccc = new PropertyInt(i, std::to_string(n++), [](const std::any & value, const std::string & title, const std::any & backup)
            //    {
            //        std::cout
            //            << "title: " << title << ' '
            //            << "value: " << std::any_cast<int>(value) << std::endl;
            //        return true;
            //    });
            //object->GetChildren({ "C_2" })->AddChild(ccc);
        }
    }
    return true;
}
