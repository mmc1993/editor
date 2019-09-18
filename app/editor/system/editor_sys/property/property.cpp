#include "property.h"

bool ComponentHeader::OnEnter()
{
    return ImGui::CollapsingHeader(GetState()->Name.c_str());
}

void PropertyInt::OnRender(float dt)
{
    PropertyObject::OnRender(dt);

    if (ImGui::InputInt(ImID(this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyBool::OnRender(float dt)
{
    PropertyObject::OnRender(dt);

    if (ImGui::Checkbox(ImID(this).c_str(), &GetBackup()))
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

    if (ImGui::InputFloat(ImID(this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyString::OnRender(float dt)
{
    PropertyObject::OnRender(dt);
    
    if (ImGui::InputText(
        ImID(this).c_str(), 
        GetBackup().data(), GetBackup().size(), 
        ImGuiInputTextFlags_CallbackResize,
        &imgui_tools::OnResizeBuffer,
        &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyCombo::OnRender(float dt)
{
    PropertyObject::OnRender(dt);

    auto & select = GetBackup().first;
    auto & values = GetBackup().second;
    if (ImGui::BeginCombo(
        ImID(this).c_str(),
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
    PropertyObject::OnRender(dt);

    if (ImGui::InputFloat2(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyVector3::OnRender(float dt)
{
    PropertyObject::OnRender(dt);

    if (ImGui::InputFloat3(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyVector4::OnRender(float dt)
{
    PropertyObject::OnRender(dt);

    if (ImGui::InputFloat4(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void PropertyColor4::OnRender(float dt)
{
    PropertyObject::OnRender(dt);

    if (ImGui::ColorEdit4(ImID(this).c_str(), &GetBackup().x))
    {
        Modify();
    }
    ImGui::Columns(1);
}


