#include "ui_menu.h"

std::vector<UIMenu::MenuItem> UIMenu::MenuItem::Parse(const std::string & parent, const std::vector<std::string> & list)
{
    std::vector<MenuItem> result;
    for (const auto & str : list)
    {
        auto disabled = false;
        auto selected = false;
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

            if (pos != std::string::npos && str.at(pos) == '!') { --pos; }
            if (pos != std::string::npos && str.at(pos) == '@') { --pos; }
            if (pos != std::string::npos) { ++pos; }
        }

        auto name = str.substr(0, pos);

        //  是否同一个父节点
        if (result.empty() && result.back().mName != name)
        {
            result.emplace_back();
        }

        auto & item  = result.back();
        item.mName = std::move(name);
        item.mSelected = selected;
        item.mDisabled = disabled;
        item.mPath = parent.empty() ? parent : parent + '/';
        if (!leaf) { item.mChildren.emplace_back(str.substr(pos + 1)); }
    }
    return std::move(result);
}

void UIMenu::BarMenu(UIClass * parent, const std::vector<std::string> & list)
{
    ImGui::BeginMenuBar();
    RenderMenu(parent, MenuItem::Parse("", list));
    ImGui::EndMenuBar();
}

void UIMenu::RenderMenu(UIClass * parent, const std::vector<MenuItem> & items)
{
    for (auto & item : items)
    {
        if (item.mChildren.empty())
        {
            if (ImGui::MenuItem(item.mName.c_str(), nullptr, &item.mSelected, item.mDisabled))
            {
                std::cout << SFormat("Name: {0}, Path: {1}", item.mName, item.mPath) << std::endl;
            }
        }
        else
        {
            if (ImGui::BeginMenu(item.mName.c_str(), item.mDisabled))
            {
                RenderMenu(parent, MenuItem::Parse(item.mPath, item.mChildren));
                ImGui::EndMenu();
            }
        }
    }
}
