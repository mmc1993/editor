#pragma once

#include "../../../../include.h"
#include "../../component/component.h"
#include "../../../event_sys/event_sys.h"
#include "../../../cfg_sys/cfg_sys.h"
#include "../../../ui_sys/ui_sys.h"
#include "../../../ui_sys/ui_menu.h"
#include "../../../ui_sys/ui_parse/ui_parse.h"
#include "../../../ui_sys/ui_state/ui_state.h"
#include "../../../ui_sys/ui_object/ui_object.h"
#include "../../editor_sys.h"
#include "../../project.h"

class UIEventDelegateMainObjList : public UIEvent::DelegateHandler {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object) override;
};

class UIEventDelegateMainResList : public UIEvent::DelegateHandler {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object) override;
};

class UIEventDelegateMainComList : public UIEvent::DelegateHandler {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object) override;

private:
    void OnRefreshComponent();

private:
    UIObject * _onwer;
    EventSys::Listener _listener;
};

class UIEventDelegateMainStage : public UIEvent::DelegateHandler {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, UIObject * object) override;
};