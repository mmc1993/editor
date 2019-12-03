#pragma once

#include "../../../include.h"
#include "../editor_sys.h"
#include "../../ui_sys/ui_sys.h"
#include "../../raw_sys/raw_sys.h"
#include "../../cfg_sys/cfg_sys.h"
#include "../../event_sys/event_sys.h"

class UIDelegateBase : public UIEvent::DelegateHandler {
protected:
    EventSys::Listener _listener;
};
