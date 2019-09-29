#include "ui_property.h"

bool UIComponentHeader::OnEnter()
{
    return ImGui::CollapsingHeader(GetState()->Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
}

void UIPropertyInt::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);

    if (ImGui::InputInt(ImID(this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void UIPropertyBool::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);

    if (ImGui::Checkbox(ImID(this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void UIPropertyFloat::OnRender(float dt)
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

void UIPropertyString::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);
    
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

void UIPropertyCombo::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);

    auto & value = GetBackup();
    if (ImGui::BeginCombo(ImID(this).c_str(), _list.at(value).c_str()))
    {
        for (auto i = 0; i != _list.size(); ++i)
        {
            if (ImGui::Selectable(_list.at(i).c_str(), value == i))
            {
                value =i;
                Modify();
            }
            if (value == i)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Columns(1);
}

void UIPropertyVector2::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);

    if (ImGui::InputFloat2(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void UIPropertyVector3::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);

    if (ImGui::InputFloat3(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void UIPropertyVector4::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);

    if (ImGui::InputFloat4(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
}

void UIPropertyColor4::OnRender(float dt)
{
    UIPropertyObject::OnRender(dt);

    if (ImGui::ColorEdit4(ImID(this).c_str(), &GetBackup().x))
    {
        Modify();
    }
    ImGui::Columns(1);
}


