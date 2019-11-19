#pragma once

#include "../../../include.h"
#include "../../interface/serializer.h"

#ifdef GetObject
#undef GetObject
#endif

class Component;
class CompTransform;
class UIObjectGLCanvas;

class GLObject 
    : public interface::Serializer
    , public std::enable_shared_from_this<GLObject> {
public:
    enum StateEnum {
        kActive = 0x1,
        kLocked = 0x2,
    };

public:
    GLObject();
    GLObject(uint id);
    virtual ~GLObject();
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;

    void AddObject(const SharePtr<GLObject> & object, const std::string & name);
    void DelObject(const SharePtr<GLObject> & object);
    void DelObject(const std::string & name);
    void DelObject(size_t idx);
    void ClearObjects();
    void DelThis();

    SharePtr<GLObject> GetObject(const std::string & name);
    SharePtr<GLObject> GetObject(const size_t idx);
    std::vector<SharePtr<GLObject>> & GetObjects();

    void Update(UIObjectGLCanvas * canvas, float dt);

    void SetName(const std::string & name);
    const std::string & GetName() const;

    void AddState(StateEnum state, bool add);
    bool HasState(StateEnum state);

    void SetParent(GLObject * parent);
    SharePtr<GLObject> GetParent();

    uint GetID() { return _id; }

    void SetTransform(const SharePtr<CompTransform> & transform);
    SharePtr<CompTransform> GetTransform();

	void ClearComponents();
    void AddComponent(const SharePtr<Component> & component);
    void DelComponent(const SharePtr<Component> & component);
    void DelComponent(const std::type_info & type);
    std::vector<SharePtr<Component>> & GetComponents();

    glm::mat4 GetWorldMatrix();
    const glm::mat4 & GetLocalMatrix();
    glm::vec2 WorldToLocal(const glm::vec2 & point);
    glm::vec2 LocalToWorld(const glm::vec2 & point);
    glm::vec2 ParentToLocal(const glm::vec2 & point);
    glm::vec2 LocalToParent(const glm::vec2 & point);

    template <class T>
    SharePtr<T> GetComponent()
    {
        auto iter = std::find_if(_components.begin(), _components.end(),
           [](const SharePtr<Component> & component) 
           { return typeid(*component) == typeid(T); });
        return CastPtr<T>(iter != _components.end() ? *iter : nullptr);
    }

    template <class T>
    std::vector<SharePtr<T>> GetComponentsInChildren()
    {
        std::vector<SharePtr<T>> result;
        auto self = GetComponent<T>();
        if (self != nullptr)
        {
            result.push_back(self);
        }
        for (const auto & v : _children)
        {
            auto ret = std::move(v->GetComponentsInChildren<T>());
            result.insert(result.end(), ret.begin(), ret.end());
        }
        return std::move(result);
    }

private:
    uint                                _id;
    uint                                _state;
    std::string                         _name;
    GLObject *                          _parent;
    SharePtr<CompTransform>             _transform;
    std::vector<SharePtr<GLObject>>     _children;
    std::vector<SharePtr<Component>>    _components;
};
