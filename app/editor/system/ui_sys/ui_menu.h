#pragma once

#include "include.h"

//  两种菜单
//  1. 窗口菜单
//  2. 弹出菜单

//  每个窗口可以有自己的窗口菜单, 在窗口中每帧渲染
//  同一时刻只能有一个弹出菜单, 在所有UI之后渲染

//  菜单格式:
//      @勾选, !禁用, ~编辑
//      Menu0\\Menu0[~][@][!]
//      Menu0\\Menu1[~][@][!]
//      Menu0\\Menu2[~][@][!]
//      Menu1\\Menu0[~][@][!]
//      Menu1\\Menu1[~][@][!]
//      Menu1\\Menu2[~][@][!]
//      Menu2\\Menu0~

class UIMenu {
public:
    struct MenuItem {
        std::string           mPath;        //  菜单项路径
        std::string           mName;        //  菜单项名字
        std::string           mBuffer;      //  菜单项缓冲
        bool                  mEditing;     //  菜单项是否可编辑
        bool                  mSelected;    //  菜单项是否已选中
        bool                  mDisabled;    //  菜单项是否已禁用
        std::vector<MenuItem> mChildren;    //  菜单项子项

        static std::vector<MenuItem> Parse(const std::string & parent, const std::vector<std::string> & list);
    };

    struct Popup {
        glm::vec2 mMouse;
        SharePtr<UIObject> mObject;
        std::vector<MenuItem> mItems;
    };

public:
    static void BarMenu(SharePtr<UIObject> object, const std::vector<std::string> & list);
    static void PopMenu(SharePtr<UIObject> object, const std::vector<std::string> & list);
    static void RenderPopup();

private:
    static void RenderMenu(SharePtr<UIObject> object, std::vector<MenuItem> & items);

private:
    static Popup s_popup;
};