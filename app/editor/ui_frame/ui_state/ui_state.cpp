#include "ui_state.h"

UIState::UIState()
    : mID(~0)
    , mName("root")
    , mEnabled(true)
    , mVisible(true)
    , mEnabledKey(true)
    , mEnabledMouse(true)
{ }

UIStateWindow::UIStateWindow()
{
    mFullScreen = true;
    mIsStretch = false;
    mUIType = UITypeEnum::kWINDOW;
}

UIStateDDContainer::UIStateDDContainer()
{
    mUIType = UITypeEnum::kCONTAINER;
}

