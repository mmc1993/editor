#pragma once

#include "../../../../include.h"
#include "../../component/component.h"
#include "../../component/comp_transform.h"
#include "../../../event_sys/event_sys.h"
#include "../../../cfg_sys/cfg_sys.h"
#include "../../../ui_sys/ui_sys.h"
#include "../../../ui_sys/ui_menu.h"
#include "../../../ui_sys/ui_parse/ui_parse.h"
#include "../../../ui_sys/ui_object/ui_object.h"
#include "../../property/ui_property.h"
#include "../../editor_sys.h"
#include "../../project.h"

class UIEventDelegateMain : public UIEvent::DelegateHandler {
protected:
    EventSys::Listener _listener;
};

//  对象列表
class UIEventDelegateMainObjList : public UIEventDelegateMain {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object) override;

private:
    void InitObjects(
        const SharePtr<UIObject> & uiobject,
        const SharePtr<GLObject> & globject);
    SharePtr<UIObject> NewObject(uint id, const std::string & name);

    void OnEvent(EventSys::TypeEnum type, const std::any & param);
    void OnEventOpenProject();
    void OnEventFreeProject();
    void OnEventInsertObject(const SharePtr<GLObject> & object);
    void OnEventDeleteObject(const SharePtr<GLObject> & object);
    void OnEventRenameObject(const SharePtr<GLObject> & object, const std::string &name);
    void OnEventSelectObject(const SharePtr<GLObject> & object, bool select, bool multi);
    void OnEventStateObject(const SharePtr<GLObject> & object, uint state0, uint state1);

private:
    std::map<uint, SharePtr<UIObject>> _id2obj;
    std::map<SharePtr<UIObject>, uint> _obj2id;
};

//  资源列表
class UIEventDelegateMainResList : public UIEventDelegateMain {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object) override;
};

//  组件列表
class UIEventDelegateMainComList : public UIEventDelegateMain {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object) override;

private:
    void OnEvent(EventSys::TypeEnum type, const std::any & param);
    void OnEventSelectObject(const SharePtr<GLObject> & object, bool select, bool multi);
    void OnEventAppendComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component);
    void OnEventDeleteComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component);
};

//  舞台
class UIEventDelegateMainStage : public UIEventDelegateMain {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object) override;

private:
    void OnEvent(EventSys::TypeEnum type, const std::any & param);
    void OnEventOpenProject();
    void OnEventFreeProject();
    void OnEventSelectObject(const SharePtr<GLObject> & object, bool select, bool multi);
};

//  全局
class UIEventDelegateMainGlobal : public UIEventDelegateMain {
public:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object) override;
};