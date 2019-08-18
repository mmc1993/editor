#pragma once

#include "../base.h"

class UIClass {
public:
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

    template <class T>
    T & GetState() { return *(T *)_state; }

    virtual void OnUpdate(float dt) = 0;
    virtual void OnDraw(float dt) = 0;

private:
    UIState *              _state;
    std::vector<UIClass *> _children;
};

class UIClassDDContainer : public UIClass {
public:
    UIClassDDContainer(UIState * state): UIClass(state)
    { }

protected:
    virtual void OnUpdate(float dt) override;
    virtual void OnDraw(float dt) override;
};

class UIClassButton : public UIClass {
public:
    UIClassButton(UIState * state) : UIClass(state)
    { }

protected:
    virtual void OnUpdate(float dt) override;
    virtual void OnDraw(float dt) override;
};