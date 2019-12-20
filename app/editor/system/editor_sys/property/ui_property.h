#pragma once

#include "../../ui_sys/ui_object/ui_object.h"
#include "../../raw_sys/res.h"

class UIComponentHeader : public UIObject {
public:
    UIComponentHeader(const std::string & name): UIObject(UITypeEnum::kOther, new UIState())
    {
        GetState()->Name = name;
    }

private:
    virtual bool OnEnter() override;
};

template <class T>
class UIPropertyState : public UIState {
public:
    using Handler_t = std::function<bool(
        const std::any & oldValue,
        const std::any & newValue,
        const std::string & title)>;

public:
    UIPropertyState(
        T & value, 
        const std::string & title, 
        const Handler_t & handler)
        : mTitle(title)
        , mValue(value)
        , mOldValue(value)
        , mNewValue(value)
        , mHandler(handler)
    { }

    T           mOldValue;
    T           mNewValue;
    T &         mValue;
    std::string mTitle;
    Handler_t mHandler;
};

template <class T>
class UIPropertyObject : public UIObject {
public:
    using Handler_t = typename UIPropertyState<T>::Handler_t;

protected:
    UIPropertyObject(
        T & value,
        const std::string & title,
        const Handler_t & handler)
        : UIObject(UITypeEnum::kOther, new UIPropertyState<T>(value, title, handler))
    { }

    const std::string & GetTitle() { return GetState<UIPropertyState<T>>()->mTitle; }
    T & GetNewValue() { return GetState<UIPropertyState<T>>()->mNewValue; }
    T & GetOldValue() { return GetState<UIPropertyState<T>>()->mOldValue; }
    T & GetValue() { return GetState<UIPropertyState<T>>()->mValue; }

    void Modify()
    {
        if (auto ret = GetState<UIPropertyState<T>>()->mHandler(
            GetOldValue(), GetNewValue(), GetTitle()); ret)
        {
            GetValue() = GetNewValue();
            GetOldValue() = GetValue();
        }
        else
        {
            GetNewValue() = GetValue();
            GetOldValue() = GetValue();
        }
    }

    virtual bool OnEnter() override
    {
        if (GetOldValue() != GetValue())
        {
            GetOldValue() = GetValue();
            GetNewValue() = GetValue();
        }

        auto width = ImGui::GetWindowWidth();
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(1, width * 0.6f);
        ImGui::SetColumnOffset(1, width * 0.3f);

        ImGui::Text(GetTitle().c_str());
        ImGui::NextColumn();
        return true;
    }
};

// ---
//  属性 UIPropertyInt
// ---
class UIPropertyInt : public UIPropertyObject<int> {
public:
    UIPropertyInt(
        int & value, 
        const std::string & title, 
        const Handler_t & handler)
        : UIPropertyObject<int>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyBool
// ---
class UIPropertyBool : public UIPropertyObject<bool> {
public:
    UIPropertyBool(
        bool & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<bool>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyFlag
// ---
class UIPropertyFlag : public UIPropertyObject<uint> {
public:
    UIPropertyFlag(
        uint & value,
        const std::string & title,
        const Handler_t & handler, const std::any & params)
        : UIPropertyObject<uint>(value, title, handler), mMask(std::any_cast<uint>(params))
    { }

    virtual bool OnEnter() override;
private:
    uint mMask;
};

// ---
//  属性 UIPropertyFloat
// ---
class UIPropertyFloat : public UIPropertyObject<float> {
public:
    UIPropertyFloat(
        float & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<float>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyString
// ---
class UIPropertyString : public UIPropertyObject<std::string> {
public:
    UIPropertyString(
        std::string & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<std::string>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyCombo
// ---
class UIPropertyCombo : public UIPropertyObject<size_t> {
public:
    UIPropertyCombo(
        size_t & value,
        const std::string & title,
        const Handler_t & handler,
        const std::vector<std::string> & list)
        : UIPropertyObject<size_t>(value, title, handler), _list(list)
    { }

    virtual bool OnEnter() override;

private:
    const std::vector<std::string> & _list;
};

// ---
//  属性 UIPropertyVector2
// ---
class UIPropertyVector2 : public UIPropertyObject<glm::vec2> {
public:
    UIPropertyVector2(
        glm::vec2 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec2>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyVector3
// ---
class UIPropertyVector3 : public UIPropertyObject<glm::vec3> {
public:
    UIPropertyVector3(
        glm::vec3 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec3>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyVector4
// ---
class UIPropertyVector4 : public UIPropertyObject<glm::vec4> {
public:
    UIPropertyVector4(
        glm::vec4 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec4>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyColor4
// ---
class UIPropertyColor4 : public UIPropertyObject<glm::vec4> {
public:
    UIPropertyColor4(
        glm::vec4 & value,
        const std::string & title,
        const Handler_t & handler)
        : UIPropertyObject<glm::vec4>(value, title, handler)
    { }

    virtual bool OnEnter() override;
};

// ---
//  属性 UIPropertyAsset
// ---
class UIPropertyAsset: public UIPropertyObject<Res::Ref> {
public:
    UIPropertyAsset(
        Res::Ref & value,
        const std::string & title,
        const Handler_t & handler, const std::any & params)
        : UIPropertyObject<Res::Ref>(value, title, handler)
    {
        for (auto type : std::any_cast<std::vector<uint>>(params))
        {
            mSearch.append(Res::TypeString(type));
            mSearch.append(" ");
        }
        mSearch.append("|");
    }

    virtual bool OnEnter() override;

private:
    std::string                     mSearch;
};