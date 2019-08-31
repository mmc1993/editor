#pragma once

#include "../base.h"

class UIState {
public:
    UIState();

    virtual ~UIState()
    { }

    std::map<std::string, std::any> mData;
};

class UIStateLayout : public UIState {
public:
    struct Join {
        std::vector<UIClass *>          mOut;
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