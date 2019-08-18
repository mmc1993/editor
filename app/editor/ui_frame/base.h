#pragma once

#include "../include.h"
#include "imgui.h"

class UIState;
class UIClass;
class UIFrame;

enum class DirectEnum {
    kT, kD, kL, kR, LENGTH
};

static const std::string WEIGET_CONTAINER      = "DDContainer";
static const std::string WEIGET_BUTTON         = "Button";
