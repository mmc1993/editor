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

protected:
    UIClass(UIState * state) : _state(state)
    { }

    template <class T>
    T & GetState() { return (T *)_state; }

    virtual void OnDraw() = 0;

private:
    UIState *              _state;
    std::vector<UIClass *> _children;
};

class UIClassPanel : public UIClass {
public:
    UIClassPanel(UIState * state): UIClass(state)
    { }

protected:
    virtual void OnDraw() override
    { }
};

class UIClassButton : public UIClass {
public:
    UIClassButton(UIState * state) : UIClass(state)
    { }

protected:
    virtual void OnDraw() override
    { }
};