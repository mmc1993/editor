#pragma once

#include "../include.h"

class Component;
class CompTransform;

class Object {
public:
    Object();
    virtual ~Object();
    virtual void OnUpdate(float dt);

    void AddChild(Object * child, const std::string & tag = "");
    void DelChild(Object * child, bool del = true);
    void DelChildIdx(size_t idx, bool del = true);
    void DelChildTag(const std::string & tag);
    void ClearChildren();
    void DelThis();

	Object * GetChildIdx(size_t idx);
	Object * GetChildTag(const std::string & tag);
    std::vector<Object *> & GetChildren();

    void SetActive(bool active);
    bool IsActive() const;

    void RootUpdate(float dt);
    void Update(float dt);

    void SetParent(Object * parent);
    Object * GetParent();

	void ClearComponent();
    void AddComponent(Component * component);
    void DelComponent(const std::type_info & type);
    std::vector<Component *> & GetComponents();

    template <class T>
    T * GetComponent()
    {
        auto iter = std::find_if(_components.begin(), _components.end(),
           [](Component * component) { return typeid(*component) == typeid(T); });
        return reinterpret_cast<T *>(iter != _components.end() ? *iter : nullptr);
    }

    template <class T>
    std::vector<T *> GetComponentsInChildren()
    {
        std::vector<T *> result{};
        auto self = GetComponent<T>();
        if (self != nullptr)
        {
            result.push_back(self);
        }
        for (auto v : _children)
        {
            auto ret = std::move(v->GetComponentsInChildren<T>());
            result.insert(result.end(), ret.begin(), ret.end());
        }
        return std::move(result);
    }

private:
    std::string                 _tag;
    bool                        _active;
    Object *                    _parent;
	CompTransform *             _transform;
    std::vector<Object *>       _children;
    std::vector<Component *>    _components;
};