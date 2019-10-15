#pragma once

#include "../include.h"
#include "../ui_event.h"
#include "../ui_state/ui_state.h"
#include "../../raw_sys/raw.h"

#ifdef GetObject
#undef GetObject
#endif

class UIObject: public std::enable_shared_from_this<UIObject> {
public:
    template <class T = UIState>
    T * GetState() { return (T *)_state.get(); }
    
    SharePtr<UIObject> GetObject(const std::initializer_list<std::string> & list);
    std::vector<SharePtr<UIObject>>   GetObjects(UITypeEnum type);
    std::vector<SharePtr<UIObject>> & GetObjects();
    void AddObject(const SharePtr<UIObject> & object);
    void DelObject(const SharePtr<UIObject> & object);
    void DelObject(size_t index);
    void DelThis();
    void ClearObjects();
    SharePtr<UIObject> GetParent();
    SharePtr<UIObject> GetRoot();
    UITypeEnum GetType();
    bool IsVisible();

    void ResetLayout();
    void ApplyLayout();
    void Render(float dt, bool visible = true);

    glm::vec4 CalcStretech(DirectEnum direct, const glm::vec2 & offset);
    glm::vec2 ToWorldCoord(const glm::vec2 & coord = glm::vec2(0));
    glm::vec4 ToLocalCoord(const glm::vec4 & coord);
    glm::vec2 ToLocalCoord(const glm::vec2 & coord);
    glm::vec2 ToLocalCoordFromImGUI();
    glm::vec4 ToWorldRect();

    //  绑定事件委托, 事件将被传递到委托中
    void BindDelegate(UIEvent::DelegateHandler * delegate);

protected:
    UIObject(UITypeEnum type, UIState * state) 
        : _type(type)
        , _state(state)
        , _parent(nullptr)
    { }

    virtual ~UIObject()
    { }

    void RenderDrag();
    void UpdateSize();
    void UpdateMove();
    virtual bool OnEnter();
    virtual void OnLeave(bool ret);
    virtual void OnResetLayout();
    virtual void OnApplyLayout();

    //  事件处理
    void DispatchEventKey();
    SharePtr<UIObject> DispatchEventKey(const UIEvent::Key & param);
    void DispatchEventDrag();
    SharePtr<UIObject> DispatchEventDrag(const UIEvent::Drag & param);
    void DispatchEventMouse();
    SharePtr<UIObject> DispatchEventMouse(const UIEvent::Mouse & param);
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param);

private:
    SharePtr<UIObject> CallEventMessage(UIEventEnum e, const UIEvent::Event & param);
public:
    SharePtr<UIObject> PostEventMessage(UIEventEnum e, const UIEvent::Event & param);

private:
    bool                                _visible;       //  标记当前节点是否渲染
    UITypeEnum                          _type;
    UIObject *                          _parent;
    UniquePtr<UIState>                  _state;
    std::vector<SharePtr<UIObject>>     _children;
    UniquePtr<UIEvent::DelegateHandler> _delegate;
};

class UIObjectLayout : public UIObject {
public:
    UIObjectLayout();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual void OnResetLayout() override;
    virtual void OnApplyLayout() override;

    bool IsCanStretch(DirectEnum edge);
    bool IsCanStretch(DirectEnum edge, const glm::vec2 & offset);
    void HandleStretch();
};

class UIObjectTreeBox : public UIObject {
public:
    UIObjectTreeBox();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
};

class UIObjectTextBox : public UIObject {
public:
    UIObjectTextBox();

private:
    virtual bool OnEnter() override;
};

class UIObjectImageBox : public UIObject {
public:
    UIObjectImageBox();

private:
    virtual bool OnEnter() override;
};

class UIObjectComboBox : public UIObject {
public:
    UIObjectComboBox();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param) override;
};

class UIObjectUICanvas : public UIObject {
public:
    UIObjectUICanvas();
};

class UIObjectGLCanvas : public UIObject {
public:
    UIObjectGLCanvas();
    interface::MatrixStack & GetMatrixStack();
    void BindRoot(const SharePtr<GLObject> & root);
    void Post(const SharePtr<interface::PostCommand>   & cmd);
    void Post(const SharePtr<interface::FowardCommand> & cmd);
    void Post(const SharePtr<GLProgram> & program, const glm::mat4 & transform);

    //  交互操作
    void OptSelected(const SharePtr<GLObject> & object, bool selected);
    void OptSelectedClear();

private:
    void HandleFowardCommands();
    void HandlePostCommands();
    void CollCommands();
    void CallCommands();

    void DrawOutlineObjects();
    void DrawTrackingPoints();

    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual void OnApplyLayout() override;
    virtual void OnResetLayout() override;
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param) override;

    void OnEventKey(const UIEvent::Key & param);
    void OnEventMenu(const UIEvent::Menu & param);
    void OnEventMouse(const UIEvent::Mouse & param);
};