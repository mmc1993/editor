#pragma once

#include "../include.h"
#include "../ui_event.h"
#include "../ui_state/ui_state.h"

#ifdef GetObject
#undef GetObject
#endif

class UIObject: public std::enable_shared_from_this<UIObject> {
public:
    template <class T = UIState>
    T * GetState() { return (T *)_state.get(); }
    
    SharePtr<UIObject> GetObject(const std::initializer_list<std::string> & list);
    std::vector<SharePtr<UIObject>>   GetObjects(UITypeEnum type) const;
    std::vector<SharePtr<UIObject>> & GetObjects();
    void AddObject(const SharePtr<UIObject> & object);
    void DelObject(const SharePtr<UIObject> & object);
    void DelObject(size_t index);
    void DelThis();
    void ClearObjects();
    SharePtr<UIObject> GetParent();
    SharePtr<UIObject> GetRoot();
    bool IsVisible() const;

    void ResetLayout();
    void ApplyLayout();
    void Render(float dt, bool visible = true);

    glm::vec4 CalcStretech(DirectEnum direct, const glm::vec2 & offset) const;
    glm::vec2 ToWorldCoord(const glm::vec2 & coord = glm::vec2(0)) const;
    glm::vec4 ToLocalCoord(const glm::vec4 & coord) const;
    glm::vec2 ToLocalCoord(const glm::vec2 & coord) const;
    glm::vec4 ToWorldRect() const;
    glm::vec2 ToLocalCoordFromImGUI() const;

    //  绑定事件委托, 事件将被传递到委托中
    void BindDelegate(UIEvent::DelegateHandler * delegate);

    //  TODO 干掉
    //  const 重载版本
    UITypeEnum GetType() const { return _type; }
    const auto GetRoot() const { return const_cast<UIObject *>(this)->GetRoot(); }
    template <class T = UIState>
    const T * GetState() const { return const_cast<UIObject *>(this)->GetState<T>(); }
    const auto GetParent() const { return const_cast<UIObject *>(this)->GetParent(); }
    const auto & GetObjects() const { return const_cast<UIObject *>(this)->GetObjects(); }
    const auto GetObject(const std::initializer_list<std::string> & list) const { return const_cast<UIObject *>(this)->GetObject(list); }

protected:
    UIObject(UITypeEnum type, UIState * state) 
        : _type(type)
        , _state(state)
    { }

    virtual ~UIObject()
    { }

    void RenderDrag();
    void UpdateSize();
    void UpdateMove();
    virtual bool OnEnter();
    virtual void OnLeave(bool ret);
    virtual void OnRender(float dt);
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
    WeakPtr<UIObject>                   _parent;
    UniquePtr<UIState>                  _state;
    std::vector<SharePtr<UIObject>>     _children;
    UniquePtr<UIEvent::DelegateHandler> _delegate;
};

class UIClassLayout : public UIObject {
public:
    UIClassLayout();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual void OnResetLayout() override;
    virtual void OnApplyLayout() override;
    virtual void OnRender(float dt) override;

    bool IsCanStretch(DirectEnum edge);
    bool IsCanStretch(DirectEnum edge, const glm::vec2 & offset);
    void HandleStretch();
};

class UIClassTreeBox : public UIObject {
public:
    UIClassTreeBox();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
};

class UIClassTextBox : public UIObject {
public:
    UIClassTextBox();

private:
    virtual bool OnEnter() override;
};

class UIClassImageBox : public UIObject {
public:
    UIClassImageBox();

private:
    virtual bool OnEnter() override;
};

class UIClassComboBox : public UIObject {
public:
    UIClassComboBox();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param) override;
};

class UIClassUICanvas : public UIObject {
public:
    UIClassUICanvas();

private:
    virtual void OnRender(float dt) override;
};

class UIClassGLCanvas : public UIObject {
public:
    UIClassGLCanvas();

private:
    virtual void OnRender(float dt) override;
};