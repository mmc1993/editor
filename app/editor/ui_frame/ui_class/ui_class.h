#pragma once

#include "../base.h"

class UIClass {
public:
    template <class T>
    T * GetState()
    {
        return (T *)_state;
    }

    UITypeEnum GetType() const;

    std::vector<UIClass *>   GetChildren(UITypeEnum type);
    std::vector<UIClass *> & GetChildren();
    void AddChild(UIClass * child);
    void DelChild(UIClass * child);
    void ClearChild();
    UIClass * GetRoot();
    UIClass * GetParent();

    void Update(float dt);
    void Render(float dt);

    void ResetLayout();
    void ApplyLayout();

    glm::vec2 ToWorldCoord(const glm::vec2 & coord = glm::vec2(0));
    glm::vec4 ToLocalCoord(const glm::vec4 & coord);
    glm::vec2 ToLocalCoord(const glm::vec2 & coord);

protected:
    UIClass(UITypeEnum type, UIState * state) 
        : _type(type)
        , _state(state)
        , _parent(nullptr)
    { }

    virtual void OnResetLayout() = 0;
    virtual void OnApplyLayout() = 0;
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

protected:
    virtual void OnResetLayout() override;
    virtual void OnApplyLayout() override;
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

protected:
    virtual void OnResetLayout() override;
    virtual void OnApplyLayout() override;
    virtual void OnUpdate(float dt) override;
    virtual void OnRender(float dt) override;

private:
    virtual bool OnEnter() override;
    virtual void OnLeave() override;

    glm::vec4 StretchU(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move);
    glm::vec4 StretchD(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move);
    glm::vec4 StretchL(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move);
    glm::vec4 StretchR(const std::pair<UIClass *, DirectEnum> &edge, const glm::vec4 & move);
};
