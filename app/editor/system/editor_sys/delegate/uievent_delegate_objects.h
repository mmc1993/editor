#pragma once

#include "../include.h"
#include "../property/property.h"
#include "../../cfg_sys/cfg_sys.h"

class UIEventDelegateObjects : public UIObject::UIEventDelegate {
public:
    virtual bool OnCallEventMessage(UIObject * object, UIEventEnum e, const UIObject::UIEventDetails::Base & param) override;
};