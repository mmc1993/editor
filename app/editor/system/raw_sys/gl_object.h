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
    SharePtr<T> QueryComponent()
    {
        auto iter = std::find_if(mComponents.begin(), mComponents.end(),
           [](const SharePtr<Component> & component)
           {return typeid(*component)==typeid(T); });
        return CastPtr<T>(iter != mComponents.end() ? *iter : nullptr);
    }

    template <class T>
    std::vector<SharePtr<T>> QueryComponentInChildren()
    {
        std::vector<SharePtr<T>> result;
        auto self = QueryComponent<T>();
        if (self != nullptr)
        {
            result.push_back(self);
        }
        for (const auto & v : mChildren)
        {
            auto comps =std::move(v->QueryComponentInChildren<T>());
            result.insert(result.end(), comps.begin(), comps.end());
        }
        return std::move(result);
    }

    //template <class T>
    //std::vector<SharePtr<T>> QueryComponents()
    //{
    //    std::vector<SharePtr<T>> result;
    //    std::copy_if(mComponents.begin(), mComponents.end(), std::back_inserter(result),
    //        [] (const SharePtr<Component> & component)
    //        {return typeid(*component) == typeid(T);});
    //    return std::move(result);
    //}

    //template <class T>
    //std::vector<SharePtr<T>> QueryComponentsInChildrens()
    //{
    //    std::vector<SharePtr<T>> result;
    //    auto comps = QueryComponents<T>();
    //    result.insert(result.end(), comps.begin(), comps.end());
    //    for (const auto & v : mChildren)
    //    {
    //        auto comps = std::move(v->QueryComponentsInChildrens<T>());
    //        result.insert(result.end(), comps.begin(), comps.end());
    //    }
    //    return std::move(result);
    //}

private:
    uint                                mID;
    uint                                mState;
    std::string                         mName;
    GLObject *                          mParent;
    SharePtr<CompTransform>             mTransform;
    std::vector<SharePtr<GLObject>>     mChildren;
    std::vector<SharePtr<Component>>    mComponents;
};
