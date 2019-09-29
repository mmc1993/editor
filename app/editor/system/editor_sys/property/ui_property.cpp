#include "ui_property.h"

bool UIComponentHeader::OnEnter()
{
    return ImGui::CollapsingHeader(GetState()->Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
}

bool UIPropertyInt::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputInt(ImID(this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyBool::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::Checkbox(ImID(this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyFloat::OnEnter()
{
    UIPropertyObject::OnEnter();

    ImGui::Columns(2, nullptr, false);
    ImGui::Text(GetTitle().c_str());
    ImGui::NextColumn();

    if (ImGui::InputFloat(ImID(this).c_str(), &GetBackup()))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyString::OnEnter()
{
    UIPropertyObject::OnEnter();
    
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
    return true;
}

bool UIPropertyCombo::OnEnter()
{
    UIPropertyObject::OnEnter();

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
    return true;
}

bool UIPropertyVector2::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputFloat2(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyVector3::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputFloat3(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyVector4::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputFloat4(ImID(this).c_str(), &GetBackup().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyColor4::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::ColorEdit4(ImID(this).c_str(), &GetBackup().x))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}


