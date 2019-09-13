#pragma once

#include "../include.h"
#include "../../ui/ui_class/ui_class.h"
#include "../../ui/ui_state/ui_state.h"

template <class T>
class PropertyState : public UIState {
public:
    using Handler_t = std::function<void(const T & value, const std::string & title)>;

public:
    PropertyState(
        T * value, 
        const std::string & title, 
        const Handler_t & handler)
        : mTitle(title)
        , mValue(value)
        , mHandler(handler)
    { }

    T *         mValue;
    std::string mTitle;
    Handler_t mHandler;
};

template <class T>
class PropertyClass : public UIClass {
public:
    //  这里声明纯粹是为了让这个别名暴露在子类中
    using Handler_t = std::function<void(const T & value, const std::string & title)>;

public:
    PropertyClass(
        T * value,
        const std::string & title,
        const Handler_t & handler)
        : UIClass(UITypeEnum::kOTHER, new PropertyState<T>(value, title, handler))
    { }

    const std::string & GetTitle() const
    {
        return GetState<PropertyState<T>>()->mTitle;
    }

    const T & GetValue() const
    {
        return *GetState<PropertyState<T>>()->mValue;
    }

    T & GetValue()
    {
        return *GetState<PropertyState<T>>()->mValue;
    }

    void Modify()
    {
        GetState<PropertyState<T>>()->mHandler(GetValue(), GetTitle());
    }
};

// ---
//  属性 int
// ---
class PropertyInt : public PropertyClass<int> {
public:
    PropertyInt(
        int * value, 
        const std::string & title, 
        const Handler_t & handler)
        : PropertyClass<int>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 bool
// ---
class PropertyBool : public PropertyClass<bool> {
public:
    PropertyBool(
        bool * value,
        const std::string & title,
        const Handler_t & handler)
        : PropertyClass<bool>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 float
// ---
class PropertyFloat : public PropertyClass<float> {
public:
    PropertyFloat(
        float * value,
        const std::string & title,
        const Handler_t & handler)
        : PropertyClass<float>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 string
// ---
class PropertyString : public PropertyClass<std::string> {
public:
    PropertyString(
        std::string * value,
        const std::string & title,
        const Handler_t & handler)
        : PropertyClass<std::string>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;

private:
    static int OnResizeBuffer(ImGuiInputTextCallbackData * data)
    {
        auto value = (std::string *)data->UserData;
        value->resize(data->BufTextLen);
        data->Buf = value->data();
        return value->capacity();
    }
};

// ---
//  属性 combo
// ---
class PropertyCombo : public PropertyClass<std::pair<size_t *, std::vector<std::string>>> {
public:
    PropertyCombo(
        std::pair<size_t *, std::vector<std::string>> * value,
        const std::string & title,
        const Handler_t & handler)
        : PropertyClass<std::pair<size_t *, std::vector<std::string>>>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};