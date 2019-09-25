#pragma once

#include "../../../include.h"
#include "../../ui_sys/ui_state/ui_state.h"
#include "../../ui_sys/ui_object/ui_object.h"

class UIComponentHeader : public UIObject {
public:
    UIComponentHeader(const std::string & name): UIObject(UITypeEnum::kOther, new UIState())
    {
        GetState()->Name = name;
    }

private:
    virtual bool OnEnter() override;
};

template <class T, class B = T>
class UIPropertyState : public UIState {
public:
    using Handler_t = std::function<bool(const std::any & value, const std::string & title, const std::any & backup)>;

public:
    UIPropertyState(
        T & value, 
        const std::string & title, 
        const Handler_t & handler)
        : mTitle(title)
        , mValue(value)
        , mBackup(value)
        , mHandler(handler)
    { }

    B           mBackup;
    T &         mValue;
    std::string mTitle;
    Handler_t mHandler;
};

template <class T, class B = T>
class UIPropertyObject : public UIObject {
public:
    using Handler_t = typename UIPropertyState<T>::Handler_t;

protected:
    UIPropertyObject(
        T & value,
        const std::string & title,
        const Handler_t & handler)
        : UIObject(UITypeEnum::kOther, new UIPropertyState<T, B>(value, title, handler))
    { }

    const std::string & GetTitle() const
    {
        return GetState<UIPropertyState<T, B>>()->mTitle;
    }

    const T & GetValue() const
    {
        return GetState<UIPropertyState<T, B>>()->mValue;
    }

    T & GetValue()
    {
        return GetState<UIPropertyState<T, B>>()->mValue;
    }

    B & GetBackup()
    {
        return GetState<UIPropertyState<T, B>>()->mBackup;
    }

    bool Modify()
    {
        auto ret = GetState<UIPropertyState<T, B>>()->mHandler(GetBackup(), GetTitle(), GetValue());
        if (ret) { GetValue() = GetBackup(); }
        else { GetBackup() = GetValue(); }
        return ret;
    }

    virtual void OnRender(float dt) override
    {
        auto width = ImGui::GetWindowWidth();
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(1, width * 0.7f);
        ImGui::SetColumnOffset(1, width * 0.3f);

        ImGui::Text(GetTitle().c_str());
        ImGui::NextColumn();
    }
};

// ---
//  属性 int
// ---
class UIPropertyInt : public UIPropertyObject<int> {
public:
    UIPropertyInt(
        int & value, 
        const std::string & title, 
        const Handler_t & handler)
        : UIPropertyObject<int>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 bool
// ---
class UIPropertyBool : public UIPropertyObject<bool> {
public:
    UIPropertyBool(
        bool & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<bool>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 float
// ---
class UIPropertyFloat : public UIPropertyObject<float> {
public:
    UIPropertyFloat(
        float & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<float>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 string
// ---
class UIPropertyString : public UIPropertyObject<std::string> {
public:
    UIPropertyString(
        std::string & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<std::string>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 combo
// ---
class UIPropertyCombo : public UIPropertyObject<
    std::pair<
        size_t, 
        std::vector<std::string>
    >
> {
public:
    UIPropertyCombo(
        std::pair<
            size_t, 
            std::vector<std::string>
        > & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<
            std::pair<
                size_t, 
                std::vector<std::string>
            >
        >(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 vec2
// ---
class UIPropertyVector2 : public UIPropertyObject<glm::vec2> {
public:
    UIPropertyVector2(
        glm::vec2 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec2>(value, title, handler)
    { }
    
    virtual void OnRender(float dt) override;
};

// ---
//  属性 vec3
// ---
class UIPropertyVector3 : public UIPropertyObject<glm::vec3> {
public:
    UIPropertyVector3(
        glm::vec3 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec3>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 vec4
// ---
class UIPropertyVector4 : public UIPropertyObject<glm::vec4> {
public:
    UIPropertyVector4(
        glm::vec4 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec4>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};

// ---
//  属性 color4
// ---
class UIPropertyColor4 : public UIPropertyObject<glm::vec4> {
public:
    UIPropertyColor4(
        glm::vec4 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec4>(value, title, handler)
    { }

    virtual void OnRender(float dt) override;
};