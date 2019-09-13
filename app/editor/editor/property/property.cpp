#include "property.h"

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
    
    if (ImGui::InputText(
        ToImGuiID((size_t)this).c_str(), 
        GetValue().data(), GetValue().size(), 
        ImGuiInputTextFlags_CallbackResize,
        &PropertyString::OnResizeBuffer,
        &GetValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyCombo::OnRender(float dt)
{
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(GetTitle().c_str());
    ImGui::NextColumn();

    auto & select = *GetValue().first;
    auto & values = GetValue().second;
    if (ImGui::BeginCombo(
        ToImGuiID(this).c_str(),
        values.at(select).c_str()))
    {
        for (auto i = 0; i != values.size(); ++i)
        {
            if (ImGui::Selectable(values.at(i).c_str(), select == i))
            {
                select=i;
                Modify();
            }
            if (select == i)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Columns(1);
}
