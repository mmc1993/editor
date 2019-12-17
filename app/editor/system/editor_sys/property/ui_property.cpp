#include "ui_property.h"
#include "../../ui_sys/ui_sys.h"
#include "../../ui_sys/ui_list.h"

bool UIComponentHeader::OnEnter()
{
    return ImGui::CollapsingHeader(GetState()->Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
}

bool UIPropertyInt::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputInt(ImID(this).c_str(), &GetNewValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyBool::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::Checkbox(ImID(this).c_str(), &GetNewValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyFlag::OnEnter()
{
    UIPropertyObject::OnEnter();

    auto flag = (GetNewValue() & _mask) == _mask;
    if (ImGui::Checkbox(ImID(this).c_str(), &flag))
    {
        if (flag) { GetNewValue() |=  _mask; }
        else      { GetNewValue() &= ~_mask; }
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyFloat::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputFloat(ImID(this).c_str(), &GetNewValue()))
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
        GetNewValue().data(), GetNewValue().size(), 
        ImGuiInputTextFlags_CallbackResize,
        &imgui_tools::OnResizeBuffer,
        &GetNewValue()))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyCombo::OnEnter()
{
    UIPropertyObject::OnEnter();

    auto & value = GetNewValue();
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

    if (ImGui::InputFloat2(ImID(this).c_str(), &GetNewValue().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyVector3::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputFloat3(ImID(this).c_str(), &GetNewValue().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyVector4::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::InputFloat4(ImID(this).c_str(), &GetNewValue().x, 3))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyColor4::OnEnter()
{
    UIPropertyObject::OnEnter();

    if (ImGui::ColorEdit4(ImID(this).c_str(), &GetNewValue().x))
    {
        Modify();
    }
    ImGui::Columns(1);
    return true;
}

bool UIPropertyAsset::OnEnter()
{
    UIPropertyObject::OnEnter();

    auto path = GetNewValue().Path();
    ImGui::InputText(ImID(this).c_str(), path.data(),
        path.size(), ImGuiInputTextFlags_ReadOnly,
        &imgui_tools::OnResizeBuffer, &GetNewValue());
    ImGui::SameLine();

    if (ImGui::Button("Select"))
    {
        auto self = CastPtr<UIPropertyAsset>(shared_from_this());
        std::function<void(Res::Ref)> func = [self](Res::Ref ref)
        {
            self->GetNewValue() = ref;
            self->Modify();
        };
        Global::Ref().mUISys->OpenWindow(UIFile_Explorer, std::make_tuple(mSearch, func));
    }

    ImGui::Columns(1);
    return true;
}
