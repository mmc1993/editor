#pragma once

#include "../include.h"
#include "imgui.h"

class UIState;
class UIClass;
class UIFrame;

enum class DirectEnum {
    kT, kD, kL, kR, LENGTH
};

enum class UITypeEnum {
    kTREE,              //  树形容器 可拖动(n) 可拉伸(n)
    kWINDOW,            //  悬浮窗口 可拖动(y) 可拉伸(y)
    kDIALOG,            //  悬浮窗口 可拖动(n) 可拉伸(n)
    kCONTAINER,         //  内嵌窗口 可拖动(n) 可拉伸(y)
};

static const std::string WEIGET_CONTAINER      = "DDContainer";
static const std::string WEIGET_BUTTON         = "Button";
