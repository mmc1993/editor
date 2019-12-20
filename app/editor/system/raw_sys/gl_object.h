#pragma once

#include "serializer.h"

class Component;
class CompTransform;
class UIObjectGLCanvas;

class GLObject 
    : public Serializer
    , public std::enable_shared_from_this<GLObject>
{
public:
    enum StateEnum {
        kActive = 0x1,
        kLocked = 0x2,
    };

public:
    GLObject();
    GLObject(uint id);
    virtual ~GLObject();
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;

    void InsertObject(const SharePtr<GLObject> & object, const std::string & name, uint pos = ~0u);
    void InsertObject(const SharePtr<GLObject> & object, uint pos = ~0u);
    void DeleteObject(const SharePtr<GLObject> & object);
    void DeleteObject(const std::string & name);
    void DeleteObject(size_t idx);
    void ClearObjects();
    void DeleteThis();

    //  确认节点关系, 0 没关系, 1 自己, 2 长辈, 3 晚辈
    int Relation(const SharePtr<GLObject> & target);

    SharePtr<GLObject> GetObject(const std::string & name);
    SharePtr<GLObject> GetObject(const size_t idx);
    std::vector<SharePtr<GLObject>> & GetObjects();

    void Update(UIObjectGLCanvas * canvas, float dt);

    std::string SetName(const std::string & name);
    const std::string & GetName() const;

    void AddState(uint state, bool add);
    uint HasState(uint state);

    bool HasParent(const SharePtr<GLObject> & object);
    void SetParent(const SharePtr<GLObject> & parent);
    void SetParent(GLObject * parent);
    SharePtr<GLObject> GetParent();

    uint GetID() { return mID; }

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
        auto iter = std::find_if(mComponents.begin(), mComponents.end(),
           [](const SharePtr<Component> & component) 
           { return typeid(*component) == typeid(T); });
        return CastPtr<T>(iter != mComponents.end() ? *iter : nullptr);
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
        for (const auto & v : mChildren)
        {
            auto ret = std::move(v->GetComponentsInChildren<T>());
            result.insert(result.end(), ret.begin(), ret.end());
        }
        return std::move(result);
    }

private:
    uint                                mID;
    uint                                _state;
    std::string                         mName;
    GLObject *                          _parent;
    SharePtr<CompTransform>             mTransform;
    std::vector<SharePtr<GLObject>>     mChildren;
    std::vector<SharePtr<Component>>    mComponents;
};
