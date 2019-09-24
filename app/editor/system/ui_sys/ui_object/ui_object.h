#pragma once

#include "../include.h"
#include "../ui_event.h"

class UIObject {
public:
    template <class T = UIState>
    T * GetState() { return (T *)_state; }
    
    UIObject * GetObjects(const std::initializer_list<std::string> & list);
    std::vector<UIObject *>   GetObjects(UITypeEnum type) const;
    std::vector<UIObject *> & GetObjects();
    void AddObject(UIObject * child);
    void DelObject(UIObject * child);
    void DelObject(size_t index);
    void DelThis();
    void ClearObjects();
    UIObject * GetRoot();
    UIObject * GetParent();
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

    //  const 重载版本
    UITypeEnum GetType() const
    {
        return _type;
    }

    template <class T = UIState>
    const T * GetState() const
    {
        return const_cast<UIObject *>(this)->GetState<T>();
    }

    const UIObject * GetRoot() const
    {
        return const_cast<UIObject *>(this)->GetRoot();
    }

    const UIObject * GetParent() const
    {
        return const_cast<UIObject *>(this)->GetParent();
    }

    const std::vector<UIObject *> & GetObjects() const
    {
        return const_cast<UIObject *>(this)->GetObjects();
    }

    const UIObject * GetObjects(const std::initializer_list<std::string> & list) const
    {
        return const_cast<UIObject *>(this)->GetObjects(list);
    }

protected:
    UIObject(UITypeEnum type, UIState * state) 
        : _type(type)
        , _state(state)
        , _parent(nullptr)
        , _delegate(nullptr)
    { }

    void AdjustSize();
    void LockPosition();
    virtual bool OnEnter();
    virtual void OnLeave(bool ret);
    virtual void OnRender(float dt);
    virtual void OnResetLayout();
    virtual void OnApplyLayout();

    //  事件处理
    void DispatchEventK();
    bool DispatchEventK(const UIEvent::Key & param);
    void DispatchEventM();
    bool DispatchEventM(const UIEvent::Mouse & param);
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param);

private:
    bool CallEventMessage(UIEventEnum e, const UIEvent::Event & param);
public:
    bool PostEventMessage(UIEventEnum e, const UIEvent::Event & param);

private:
    UITypeEnum                                  _type;
    UIState *                                   _state;
    UIObject *                                  _parent;
    bool                                        _visible;       //  标记当前节点是否渲染
    std::vector<UIObject *>                     _children;
    std::unique_ptr<UIEvent::DelegateHandler>   _delegate;
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
    virtual void OnRender(float dt) override;
};

class UIClassImageBox : public UIObject {
public:
    UIClassImageBox();

private:
    virtual void OnRender(float dt) override;
};

class UIClassComboBox : public UIObject {
public:
    UIClassComboBox();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual void OnRender(float dt) override;
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