#include "property_int.h"

void PropertyInt::OnRender(float dt)
{
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(GetTitle().c_str());
    ImGui::NextColumn();

    if (ImGui::InputInt(ToImGuiID((size_t)this).c_str(), &GetValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyBool::OnRender(float dt)
{
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(GetTitle().c_str());
    ImGui::NextColumn();

    if (ImGui::Checkbox(ToImGuiID((size_t)this).c_str(), &GetValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyFloat::OnRender(float dt)
{
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(GetTitle().c_str());
    ImGui::NextColumn();

    if (ImGui::InputFloat(ToImGuiID((size_t)this).c_str(), &GetValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyString::OnRender(float dt)
{
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(GetTitle().c_str());
    ImGui::NextColumn();

    if (ImGui::InputText(ToImGuiID((size_t)this).c_str(), &GetValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
}
