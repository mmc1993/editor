#pragma once

#include "../base.h"

class UIState {
public:
    UIState()
    { }

    virtual ~UIState()
    { }

    std::map<std::string, std::any> mData;
};

class UIStateTree : public UIState {
public:
    UIStateTree();
};

class UIStateImage : public UIState {
public:
    UIStateImage();
};

class UIStateLayout : public UIState {
public:
    struct Join {
        std::vector<UIClass *>           mOut;
        std::pair<UIClass *, DirectEnum> mIn;
        Join() {}
    } mJoin[(size_t)DirectEnum::LENGTH];

    struct StretchFocus {
        UIClass *   mObject;
        DirectEnum  mDirect;
        StretchFocus() : mObject(nullptr) {}
    } mStretchFocus;

    UIStateLayout();
};

class UIStateTextBox : public UIState {
public:
    std::string mBuffer;

    UIStateTextBox();
};

class UIStateComboBox : public UIState {
public:
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
