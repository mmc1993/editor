#pragma once

#include "gl_object.h"
#include "../../interface/serializer.h"

class UIObject;
class UIObjectGLCanvas;

class Component
    : public interface::Serializer 
    , public std::enable_shared_from_this<Component> {
public:
    enum StateEnum {
        kActive = 1 << 0,   //  激活
        kInsertTrackPoint = 1 << 1,   //  支持插入控制点
        kDeleteTrackPoint = 1 << 2,   //  支持删除控制点
        kModifyTrackPoint = 1 << 3,   //  支持修改控制点
    };

    struct Property {
        interface::Serializer::StringValueTypeEnum mType;
        std::string                                mName;
        void *                                     mMember;

        Property()
            : mType(interface::Serializer::StringValueTypeEnum::kErr)
            , mMember(nullptr)
        { }

        Property(
            interface::Serializer::StringValueTypeEnum type, 
            const std::string & name, void * member)
            : mType(type), mName(name), mMember(member)
        { }
    };

public:
    //  创建组件
    static SharePtr<Component> Create(const std::string & name);

	Component(): _owner(nullptr), _state(kActive) { }
	virtual ~Component() { }
    virtual void OnAdd() = 0;
    virtual void OnDel() = 0;
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) = 0;

    bool IsActive() { return _state & kActive; }
	void SetActive(bool active) { ModifyState(kActive, active); }

    void ModifyState(StateEnum state, bool add)
    {
        if (add) _state |=  state;
        else     _state &= ~state;
    }

    SharePtr<GLObject> GetOwner() { return _owner->shared_from_this(); }
	void SetOwner(GLObject * owner) { _owner = owner; }

    //  组件名字
    virtual const std::string & GetName() = 0;
    //  Property修改时被调用
    virtual bool OnModifyProperty(const std::any & value, 
                                  const std::any & backup,
                                  const std::string & title) = 0;
    std::vector<SharePtr<UIObject>> CreateUIPropertys();

    const std::vector<glm::vec2> & GetTrackPoints();
    void ModifyTrackPoint(const size_t index, const glm::vec2 & point) { if (_state & kModifyTrackPoint) { OnModifyTrackPoint(index, point); } }
    void InsertTrackPoint(const size_t index, const glm::vec2 & point) { if (_state & kInsertTrackPoint) { OnInsertTrackPoint(index, point); } }
    void DeleteTrackPoint(const size_t index, const glm::vec2 & point) { if (_state & kDeleteTrackPoint) { OnDeleteTrackPoint(index, point); } }

protected:
    virtual std::vector<Property> CollectProperty() = 0;
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) {};
    virtual void OnInsertTrackPoint(const size_t index, const glm::vec2 & point) {};
    virtual void OnDeleteTrackPoint(const size_t index, const glm::vec2 & point) {};

protected:
    size_t              _state;
    GLObject *          _owner;
    std::vector<glm::vec2> _trackPoints;
};