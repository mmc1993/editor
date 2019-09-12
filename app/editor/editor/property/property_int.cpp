#include "property_int.h"

void PropertyInt::OnRender(float dt)
{
    ImGui::Text(mTitle.c_str());
    if (ImGui::InputInt(nullptr, GetState<PropertyState<int>>()->mValue))
    {
        std::cout << "aaa" << std::endl;
    }
}
