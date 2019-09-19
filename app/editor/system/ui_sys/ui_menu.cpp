#include "ui_menu.h"
#include "ui_object/ui_object.h"

UIMenu::Popup UIMenu::s_popup;

//  _TODO
//      一次全部解析
std::vector<UIMenu::MenuItem> UIMenu::MenuItem::Parse(const std::string & parent, const std::vector<std::string> & list)
{
    std::vector<std::string> children;
    std::vector<MenuItem> result;
    for (const auto & str : list)
    {
        auto disabled = false;
        auto selected = false;
        auto editing  = false;
        auto leaf     = false;

        //  是否叶子节点, 是否已勾选, 是否已禁用
        auto pos = str.find_first_of('/');
        if (pos == std::string::npos)
        {
            leaf = true;
            pos = str.find_first_of('@');
            if (pos != std::string::npos) { selected = true; }
            pos = str.find_first_of('!');
            if (pos != std::string::npos) { disabled = true; }
            pos = str.find_first_of('~');
            if (pos != std::string::npos) { editing = true; }

            if (pos != std::string::npos && str.at(pos) == '!') { --pos; }
            if (pos != std::string::npos && str.at(pos) == '@') { --pos; }
            if (pos != std::string::npos && str.at(pos) == '~') { --pos; }
            if (pos != std::string::npos) { ++pos; }
        }

        auto name = str.substr(0, pos);

        //  是否同一个父节点
        if (result.empty() || result.back().mName != name)
        {
            if (!result.empty())
            {
                result.back().mChildren = std::move(Parse(result.back().mPath, children));
            }
            children.clear();
            result.emplace_back();
        }

        auto & item  = result.back();
        item.mName = std::move(name);
        item.mBuffer = item.mName;
        item.mEditing = editing;
        item.mSelected = selected;
        item.mDisabled = disabled;
        item.mPath = !parent.empty()
            ? parent + '/' + item.mName
            :                item.mName;
        if (!leaf) { children.push_back(str.substr(pos + 1)); }
    }

    if (!children.empty())
    {
        result.back().mChildren = std::move(Parse(result.back().mPath, children));
    }

    return std::move(result);
}

void UIMenu::BarMenu(UIObject * object, const std::vector<std::string> & list)
{
    auto items = MenuItem::Parse("", list);
    ImGui::BeginMenuBar();
    RenderMenu(object, items);
    ImGui::EndMenuBar();
}

void UIMenu::PopMenu(UIObject * object, const std::vector<std::string>& list)
{
    s_popup.mObject = object;
    s_popup.mMouse.x = ImGui::GetMousePos().x;
    s_popup.mMouse.y = ImGui::GetMousePos().y;
    s_popup.mItems = std::move(MenuItem::Parse("",list));
    ImGui::OpenPopup(s_popup.mItems.at(0).mName.c_str());
}

void UIMenu::RenderPopup()
{
    if (s_popup.mObject != nullptr)
    {
        ImGui::SetCursorScreenPos(ImVec2(s_popup.mMouse.x,s_popup.mMouse.y));
        if (ImGui::BeginPopup(s_popup.mItems.at(0).mName.c_str()))
        {
            RenderMenu(s_popup.mObject, s_popup.mItems);
            ImGui::EndPopup();
        }
        else
        {
            s_popup.mObject = nullptr;
        }
    }
}

void UIMenu::RenderMenu(UIObject * object, std::vector<MenuItem> & items)
{
    for (auto & item : items)
    {
        if (item.mChildren.empty())
        {
            if (item.mEditing)
            {
                if (ImGui::InputText(("##" + item.mName).c_str(), item.mBuffer.data(), item.mBuffer.size(),
                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize,
                    &imgui_tools::OnResizeBuffer, &item.mBuffer))
                {
                    ImGui::CloseCurrentPopup();
                    object->PostEventMessage(UIEventEnum::kMENU, UIObject::UIEventDetails::Menu(item.mPath, item.mBuffer.c_str()));
                }
            }
            else
            {
                if (ImGui::MenuItem(item.mName.c_str(), nullptr, item.mSelected, !item.mDisabled))
                {
                    ImGui::CloseCurrentPopup();
                    object->PostEventMessage(UIEventEnum::kMENU, UIObject::UIEventDetails::Menu(item.mPath, item.mSelected));
                }
            }
        }
        else
        {
            if (ImGui::BeginMenu(item.mName.c_str(), !item.mDisabled))
            {
                RenderMenu(object, item.mChildren);
                ImGui::EndMenu();
            }
        }
    }
}
