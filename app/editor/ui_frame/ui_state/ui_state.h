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
    UIStateWindow();
};

class UIStateLayout : public UIState {
public:
    struct LayoutInfo {
        struct LinkEdge {
            UIStateLayout * mLayout;
            DirectEnum      mEdge;
        };
        std::vector<UIStateLayout *> mLinks[(size_t)DirectEnum::LENGTH]; //  相连的Layout
        std::vector<LinkEdge>        mEdges[(size_t)DirectEnum::LENGTH]; //  相连的Edge
    };

    //  布局信息
    LayoutInfo mLayoutInfo;

    UIStateLayout();
};