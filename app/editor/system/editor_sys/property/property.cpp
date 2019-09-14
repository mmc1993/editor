#include "property.h"

void PropertyInt::OnRender(float dt)
{
    PropertyClass::OnRender(dt);

    if (ImGui::InputInt(ToImGuiID((size_t)this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyBool::OnRender(float dt)
{
    PropertyClass::OnRender(dt);

    if (ImGui::Checkbox(ToImGuiID((size_t)this).c_str(), &GetBackup()))
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

    if (ImGui::InputFloat(ToImGuiID((size_t)this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyString::OnRender(float dt)
{
    PropertyClass::OnRender(dt);
    
    if (ImGui::InputText(
        ToImGuiID((size_t)this).c_str(), 
        GetBackup().data(), GetBackup().size(), 
        ImGuiInputTextFlags_CallbackResize,
        &PropertyString::OnResizeBuffer,
        &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyCombo::OnRender(float dt)
{
    PropertyClass::OnRender(dt);

    auto & select = GetBackup().first;
    auto & values = GetBackup().second;
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

void PropertyVector2::OnRender(float dt)
{
    PropertyClass::OnRender(dt);

    if (ImGui::InputFloat2(ToImGuiID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyVector3::OnRender(float dt)
{
    PropertyClass::OnRender(dt);

    if (ImGui::InputFloat3(ToImGuiID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyVector4::OnRender(float dt)
{
    PropertyClass::OnRender(dt);

    if (ImGui::InputFloat4(ToImGuiID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyColor4::OnRender(float dt)
{
    PropertyClass::OnRender(dt);

    if (ImGui::ColorEdit4(ToImGuiID(this).c_str(), &GetBackup().x))
    {
        Modify();
    }
    ImGui::Columns(1);
}
