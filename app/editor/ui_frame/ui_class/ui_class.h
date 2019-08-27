#pragma once

#include "../base.h"

class UIClass {
public:
    template <class T>
    T & GetState()
    {
        return *(T *)_state;
    }

    UITypeEnum GetType() const;

    std::vector<UIClass *>   GetChildren(UITypeEnum type);
    std::vector<UIClass *> & GetChildren();
    void AddChild(UIClass * child);
    void DelChild(UIClass * child);
    void ClearChild();
    UIClass * GetParent();

    void Update(float dt);
    void Render(float dt);

    virtual void ApplyLayout();
    virtual void ResetLayout();

protected:
    UIClass(UITypeEnum type, UIState * state) 
        : _type(type)
        , _state(state)
        , _parent(nullptr)
    { }

    virtual void OnUpdate(float dt) = 0;
    virtual void OnRender(float dt) = 0;
    virtual bool OnEnter() = 0;
    virtual void OnLeave() = 0;

private:
    UITypeEnum             _type;
    UIState *              _state;
    UIClass *              _parent;
    std::vector<UIClass *> _children;
};

class UIClassWindow : public UIClass {
public:
    UIClassWindow(UIState * state ): UIClass(UITypeEnum::kWINDOW, state)
    { }

    virtual void ApplyLayout() override;
    virtual void ResetLayout() override;

protected:
    virtual void OnUpdate(float dt) override;
    virtual void OnRender(float dt) override;

private:
    virtual bool OnEnter() override;
    virtual void OnLeave() override;

};

class UIClassLayout : public UIClass {
public:
    UIClassLayout(UIState * state): UIClass(UITypeEnum::kLAYOUT, state)
    { }

    virtual void ApplyLayout() override;
    virtual void ResetLayout() override;

protected:
    virtual void OnUpdate(float dt) override;
    virtual void OnRender(float dt) override;

private:
    virtual bool OnEnter() override;
    virtual void OnLeave() override;
};
