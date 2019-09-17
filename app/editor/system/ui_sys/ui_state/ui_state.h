#pragma once

#include "../include.h"

class UIState {
public:
    UIState(): Pointer(nullptr)
    { }

    virtual ~UIState()
    { }

    void Parse(const std::string & key, const std::string & val);

    int Align               = 0;        //  对齐方式
    
    bool IsMulti            = false;    //  多行
    bool IsWindow           = false;    //  窗口
    bool IsButton           = false;    //  按钮
    bool IsShowNav          = false;    //  导航
    bool IsEditBox          = false;    //  编辑
    bool IsCanMove          = false;    //  可移动
    bool IsShowBorder       = true;     //  显示边框
    bool IsCanStretch       = true;     //  可拉伸
    bool IsFullScreen       = false;    //  全屏
    bool IsShowMenuBar      = false;    //  显示菜单
    bool IsShowTitleBar     = false;    //  显示标题
    bool IsShowScrollBar    = false;    //  显示滚动条

    float BorderNumber      = 1;        //  边框宽度

    std::string Name;                   //  名字
    std::string Tips;                   //  说明
    std::string LSkin;                  //  皮肤

    glm::vec2 StretchMin;               //  最小尺寸
    glm::vec4 Move;                     //  方位
    glm::vec4 Move_;                    //  上一帧方位

    std::vector<std::string> MenuBar;   //  菜单栏

    void * Pointer;                     //  用户数据
};

class UIStateLayout : public UIState {
public:
    struct Join {
        std::vector<UIObject *>           mOut;
        std::pair<UIObject *, DirectEnum> mIn;
        Join() {}
    } mJoin[(size_t)DirectEnum::LENGTH];

    struct StretchFocus {
        UIObject *  mObject;
        DirectEnum  mDirect;
        StretchFocus() : mObject(nullptr) {}
    } mStretchFocus;

    UIStateLayout();
};

class UIStateTreeBox : public UIState {
public:
    bool mSelect;

    UIStateTreeBox();
};

class UIStateTextBox : public UIState {
public:
    std::string mBuffer;

    UIStateTextBox();
};

class UIStateImageBox : public UIState {
public:
    UIStateImageBox();
};

class UIStateComboBox : public UIState {
public:
    std::string mSelected;

    UIStateComboBox();
};

class UIStateUICanvas : public UIState {
public:
    UIStateUICanvas();
};

class UIStateGLCanvas : public UIState {
public:
    UIStateGLCanvas();
};
