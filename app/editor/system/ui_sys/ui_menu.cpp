#include "ui_menu.h"
#include "ui_object/ui_object.h"

UIMenu::Popup UIMenu::s_popup;

std::vector<UIMenu::MenuItem> UIMenu::MenuItem::Parse(const std::string & parent, const std::vector<std::string> & list)
{
    std::string markName;
    std::vector<MenuItem>    result;
    std::vector<std::string> backup;
    for (const auto & info : list)
    {
        auto disabled = false;
        auto selected = false;
        auto editing  = false;
        auto leaf     = false;

        //  是否叶子节点, 是否已勾选, 是否已禁用
        auto pos = info.find_first_of('/');
        if (pos == std::string::npos)
        {
            leaf = true;
            pos = info.find_first_of('@');
            if (pos != std::string::npos) { selected = true; }
            pos = info.find_first_of('!');
            if (pos != std::string::npos) { disabled = true; }
            pos = info.find_first_of('~');
            if (pos != std::string::npos) { editing = true; }

            ASSERT_LOG(info.find('@') == std::string::npos || info.find('@') != std::string::npos && info.find('!') == info.find('~'), info.c_str());
            ASSERT_LOG(info.find('!') == std::string::npos || info.find('!') != std::string::npos && info.find('@') == info.find('~'), info.c_str());
            ASSERT_LOG(info.find('~') == std::string::npos || info.find('~') != std::string::npos && info.find('!') == info.find('@'), info.c_str());

            if (pos != std::string::npos && info.at(pos) == '!') { --pos; }
            if (pos != std::string::npos && info.at(pos) == '@') { --pos; }
            if (pos != std::string::npos && info.at(pos) == '~') { --pos; }
            if (pos != std::string::npos) { ++pos; }
        }

        auto name = info.substr(0, pos);
        if (name != markName)
        {
            if (!result.empty())
            {
                auto children = Parse(result.back().mPath, backup);
                result.back().mChildren = std::move(children);
                backup.clear();
            }

            MenuItem item;
            item.mName = name;
            item.mBuffer = name;
            item.mEditing = editing;
            item.mSelected = selected;
            item.mDisabled = disabled;
            item.mPath = !parent.empty()
                ? parent + '/' + item.mName
                :                item.mName;
            result.push_back(item);

            markName = name;
        }

        if (!leaf)
        {
            backup.push_back(info.substr(pos + 1));
        }
    }

    if (!backup.empty())
    {
        result.back().mChildren = std::move(Parse(result.back().mPath, backup));
    }

    return std::move(result);
}

void UIMenu::BarMenu(const SharePtr<UIObject> & object, const std::vector<std::string> & list)
{
    auto items = MenuItem::Parse("", list);
    ImGui::BeginMenuBar();
    RenderMenu(object, items);
    ImGui::EndMenuBar();
}

void UIMenu::PopMenu(const SharePtr<UIObject> & object, const std::vector<std::string>& list)
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

void UIMenu::RenderMenu(const SharePtr<UIObject> & object, std::vector<MenuItem> & items)
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
                    object->PostEventMessage(UIEventEnum::kMenu, UIEvent::Menu(item.mPath, item.mBuffer.c_str()));
                }
            }
            else
            {
                if (ImGui::MenuItem(item.mName.c_str(), nullptr, item.mSelected, !item.mDisabled))
                {
                    ImGui::CloseCurrentPopup();
                    object->PostEventMessage(UIEventEnum::kMenu, UIEvent::Menu(item.mPath, item.mSelected));
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
