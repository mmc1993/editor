#pragma once

#include "base.h"

//  两种菜单
//  1. 窗口菜单
//  2. 弹出菜单

//  每个窗口可以有自己的窗口菜单, 在窗口中每帧渲染
//  同一时刻只能有一个弹出菜单, 在所有UI之后渲染

class UIMenu {
public:
    struct MenuItem {
        std::string                 mPath;
        std::string                 mName;
        bool                        mSelected;
        bool                        mDisabled;
        std::vector<std::string>    mChildren;

        static std::vector<MenuItem> Parse(const std::string & parent, const std::vector<std::string> & list);
    };

    struct Popup {
        glm::vec2 mMouse;
        UIClass * mObject;
        std::vector<MenuItem> mItems;
    };

public:
    static void BarMenu(UIClass * object, const std::vector<std::string> & list);
    static void PopMenu(UIClass * object, const std::vector<std::string> & list);
    static void RenderPopup();

private:
    static void RenderMenu(UIClass * object, const std::vector<MenuItem> & items);

private:
    static Popup s_popup;
};