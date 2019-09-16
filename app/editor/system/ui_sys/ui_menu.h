#pragma once

#include "base.h"

//  两种菜单
//  1. 窗口菜单
//  2. 弹出菜单

//  每个窗口可以有自己的窗口菜单, 在窗口中每帧渲染
//  同一时刻只能有一个弹出菜单, 在所有UI之后渲染

//  菜单格式:
//      @勾选, !禁用, ~编辑
//      Menu0/Menu0[~][@][!]
//      Menu0/Menu1[~][@][!]
//      Menu0/Menu2[~][@][!]
//      Menu1/Menu0[~][@][!]
//      Menu1/Menu1[~][@][!]
//      Menu1/Menu2[~][@][!]
//      Menu2/Menu0~

class UIMenu {
public:
    struct MenuItem {
        std::string           mPath;
        std::string           mName;
        std::string           mBuffer;
        bool                  mEditing;
        bool                  mSelected;
        bool                  mDisabled;
        std::vector<MenuItem> mChildren;

        static std::vector<MenuItem> Parse(const std::string & parent, const std::vector<std::string> & list);
    };

    struct Popup {
        glm::vec2 mMouse;
        UIObject * mObject;
        std::vector<MenuItem> mItems;
    };

public:
    static void BarMenu(UIObject * object, const std::vector<std::string> & list);
    static void PopMenu(UIObject * object, const std::vector<std::string> & list);
    static void RenderPopup();

private:
    static void RenderMenu(UIObject * object, std::vector<MenuItem> & items);
    static int OnResizeBuffer(ImGuiInputTextCallbackData * data);

private:
    static Popup s_popup;
};