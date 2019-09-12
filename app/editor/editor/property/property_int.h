#pragma once

#include "../include.h"
#include "../../ui/ui_class/ui_class.h"
#include "../../ui/ui_state/ui_state.h"

template <class T>
class PropertyState : public UIState {
public:
    PropertyState(T * value): mValue(value)
    { }

    T * mValue;
};

class PropertyClass : public UIClass {
public:
    PropertyClass(const std::string & title, UIState * state)
        : UIClass(UITypeEnum::kOTHER, state), mTitle(title)
    { }

    std::string mTitle;
};

// ---
//   Ù–‘ int
// ---
class PropertyInt : public PropertyClass {
public:
    PropertyInt(const std::string & title, int * value)
        : PropertyClass(title, new PropertyState<int>(value))
    { }

    virtual void OnRender(float dt) override;
};

