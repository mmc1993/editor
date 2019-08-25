#include "ui_state.h"

UIState::UIState()
    : mEnabled(true)
    , mVisible(true)
    , mEnabledKey(false)
    , mEnabledMouse(false)
{ }

UIStateWindow::UIStateWindow()
{
    mIsMove = true;
    mIsSize = false;
    mUIType = UITypeEnum::kWINDOW;
}

UIStateLayout::UIStateLayout()
{
    mUIType = UITypeEnum::kLAYOUT;
}

