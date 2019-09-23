#pragma once

#include "../../../include.h"

#ifdef GetObject
#undef GetObject
#endif

class Component;
class CompTransform;

class GLObject {
public:
    GLObject();
    virtual ~GLObject();
    virtual void OnUpdate(float dt);

    void AddObject(GLObject * child, const std::string & name);
    void DelObject(const std::string & name, bool del = true);
    void DelObject(GLObject * child, bool del = true);
    void DelObject(size_t idx, bool del = true);
    void ClearObjects();
    void DelThis();

    GLObject * GetObject(const std::string & name);
    GLObject * GetObject(const size_t idx);
    std::vector<GLObject *> & GetObjects();

    const std::string & GetName() const;

    void SetActive(bool active);
    bool IsActive() const;

    void RootUpdate(float dt);
    void Update(float dt);

    void SetParent(GLObject * parent);
    GLObject * GetParent();

	void ClearComponents();
    void AddComponent(Component * component);
    void DelComponent(Component * component);
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

    CompTransform * GetTransform();

private:
    bool                        _active;
    GLObject *                  _parent;
    std::string                 _name;
	CompTransform *             _transform;
    std::vector<GLObject *>     _children;
    std::vector<Component *>    _components;
};