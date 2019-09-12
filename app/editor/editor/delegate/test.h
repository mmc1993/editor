#pragma once

#include "../include.h"

class UIEventDelegateTest : public UIClass::UIEventDelegate {
public:
    virtual bool OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param) override;
};