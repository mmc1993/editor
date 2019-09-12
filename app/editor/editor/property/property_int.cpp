#include "property_int.h"

void PropertyInt::OnRender(float dt)
{
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(mTitle.c_str());
    ImGui::NextColumn();

    if (ImGui::InputInt(
        ToImGuiID((size_t)this).c_str(),
        GetState<PropertyState<int>>()->mValue))
    {
        std::cout << "aaa" << std::endl;
    }
    ImGui::Columns(1);
}
