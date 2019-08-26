#pragma once

#include "../base.h"

class UIClass {
public:
    template <class T>
    T & GetState()
    {
        return *(T *)_state;
    }

    template <class T>
    const T & GetState() const
    {
        return (const T *)_state;
    }

    std::vector<UIClass *> & GetChildren();
    void AddChild(UIClass * child);
    void DelChild(UIClass * child);
    void ClearChild();

    void Update(float dt);
    void Draw(float dt);

protected:
    UIClass(UIState * state) : _state(state)
    { }

    virtual void OnUpdate(float dt) = 0;
    virtual void OnDraw(float dt) = 0;
    virtual bool OnEnter() = 0;
    virtual void OnLeave() = 0;

private:
    UIState *              _state;
    std::vector<UIClass *> _children;
};

class UIClassWindow : public UIClass {
public:
    UIClassWindow(UIState * state ): UIClass(state)
    { }

protected:
    virtual void OnUpdate(float dt) override;
    virtual void OnDraw(float dt) override;

private:
    virtual bool OnEnter() override;
    virtual void OnLeave() override;
};

class UIClassLayout : public UIClass {
public:
    UIClassLayout(UIState * state): UIClass(state)
    { }

protected:
    virtual void OnUpdate(float dt) override;
    virtual void OnDraw(float dt) override;

private:
    virtual bool OnEnter() override;
    virtual void OnLeave() override;
};
