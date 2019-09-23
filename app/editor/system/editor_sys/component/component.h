#pragma once

#include "gl_object.h"
#include "../../interface/serializer.h"

class UIObject;

class Component: public Interface::Serializer {
public:
    struct Property {
        Interface::Serializer::StringValueTypeEnum mType;
        std::string                                mName;
        void *                                     mMember;

        Property()
            : mType(Interface::Serializer::StringValueTypeEnum::kERR)
            , mMember(nullptr)
        { }

        Property(
            Interface::Serializer::StringValueTypeEnum type, 
            const std::string & name, void * member)
            : mType(type), mName(name), mMember(member)
        { }
    };

public:
    //  创建组件
    static Component * Create(const std::string & name);
    //  创建显示属性
    static std::vector<UIObject *> BuildUIPropertys(Component * component);

public:
	Component()
        : _owner(nullptr)
        , _active(true) { }
	virtual ~Component() { }
    virtual void OnAdd() = 0;
    virtual void OnDel() = 0;
    virtual void OnUpdate(float dt) = 0;

    bool IsActive() const { return _active; }
	void SetActive(bool active) { _active = active; }

    GLObject * GetOwner() { return _owner; }
	void SetOwner(GLObject * owner) { _owner = owner; }

    //  组件名字
    virtual const std::string & GetName() = 0;
    //  Property修改时被调用
    virtual bool OnModifyProperty(
        const std::any &    value, 
        const std::string & title, 
        const std::any &    backup) = 0;
    //  创建Property列表, 用于界面展示修改
    virtual std::vector<Property> CollectProperty() = 0;

private:
    bool _active;
    GLObject *_owner;
};