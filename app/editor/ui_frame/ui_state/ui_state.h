#pragma once

#include "../base.h"

class UIState {
public:
    UIState();

    virtual ~UIState()
    { }

    std::map<std::string, std::any> mData;
};

class UIStateWindow : public UIState {
public:
    struct StretchFocus {
        UIClass *   mObject;
        DirectEnum  mDirect;
        StretchFocus() : mObject(nullptr) {}
    } mStretchFocus;

    UIStateWindow();
};

class UIStateLayout : public UIState {
public:
    struct Join {
        std::vector<UIClass *>          mOut;
        std::pair<UIClass *, DirectEnum> mIn;
        Join() {}
    } mJoin[(size_t)DirectEnum::LENGTH];

    UIStateLayout();
};