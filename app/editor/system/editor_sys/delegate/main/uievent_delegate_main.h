#pragma once

#include "../../include.h"
#include "../../property/property.h"
#include "../../../cfg_sys/cfg_sys.h"

class UIEventDelegateMainObjList : public UIClass::UIEventDelegate {
public:
    virtual bool OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param) override;
};

class UIEventDelegateMainResList : public UIClass::UIEventDelegate {
public:
    virtual bool OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param) override;
};

class UIEventDelegateMainComList : public UIClass::UIEventDelegate {
public:
    virtual bool OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param) override;
};

class UIEventDelegateMainStage : public UIClass::UIEventDelegate {
public:
    virtual bool OnCallEventMessage(UIClass * object, UIEventEnum e, const UIClass::UIEventDetails::Base & param) override;
};