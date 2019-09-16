#pragma once

#include "../include.h"

class UIEventDelegateTest : public UIObject::UIEventDelegate {
public:
    virtual bool OnCallEventMessage(UIObject * object, UIEventEnum e, const UIObject::UIEventDetails::Base & param) override;
};