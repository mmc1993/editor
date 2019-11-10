#pragma once

#include "gl_object.h"
#include "../../interface/serializer.h"

class UIObject;
class UIObjectGLCanvas;

class Component
    : public interface::Serializer 
    , public std::enable_shared_from_this<Component> {
public:
    enum class StateEnum {
        kActive = 1 << 0,             //  激活
        kUpdate = 1 << 1,             //  刷新
        kInsertTrackPoint = 1 << 2,   //  支持插入控制点
        kDeleteTrackPoint = 1 << 3,   //  支持删除控制点
        kModifyTrackPoint = 1 << 4,   //  支持修改控制点
    };

    struct Property {
        interface::Serializer::StringValueTypeEnum mType;
        std::string                                mName;
        void *                                     mMember;
        std::any                                   mExtend;

        Property()
            : mType(interface::Serializer::StringValueTypeEnum::kErr)
            , mMember(nullptr)
        { }

        Property(
            interface::Serializer::StringValueTypeEnum type, 
            const std::string & name, void * member,
            const std::any extend = std::any())
            : mType(type)
            , mName(name)
            , mMember(member)
            , mExtend(extend)
        { }
    };

public:
    //  创建组件
    static SharePtr<Component> Create(const std::string & name);

	Component()
        : _owner(nullptr)
        , _state((uint)StateEnum::kActive | 
                 (uint)StateEnum::kUpdate) { }
	virtual ~Component() { }
    virtual void OnAdd();
    virtual void OnDel();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) = 0;

    void AddState(StateEnum state, bool add)
    {
        if (add) _state |=  (uint)state;
        else     _state &= ~(uint)state;
    }

    bool HasState(StateEnum state)
    {
        return _state & (uint)state;
    }

    SharePtr<GLObject> GetOwner() { return _owner->shared_from_this(); }
	void SetOwner(GLObject * owner) { _owner = owner; }

    //  组件名字
    virtual const std::string & GetName() = 0;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
    //  Property修改时被调用
    virtual bool OnModifyProperty(const std::any & oldValue, 
                                  const std::any & newValue,
                                  const std::string & title) = 0;
    std::vector<SharePtr<UIObject>> CreateUIPropertys();

    const std::vector<glm::vec2> & GetTrackPoints();
    void ModifyTrackPoint(const size_t index, const glm::vec2 & point) { if (_state & (uint)StateEnum::kModifyTrackPoint) { OnModifyTrackPoint(index, point); } }
    void InsertTrackPoint(const size_t index, const glm::vec2 & point) { if (_state & (uint)StateEnum::kInsertTrackPoint) { OnInsertTrackPoint(index, point); } }
    void DeleteTrackPoint(const size_t index, const glm::vec2 & point) { if (_state & (uint)StateEnum::kDeleteTrackPoint) { OnDeleteTrackPoint(index, point); } }

protected:
    virtual std::vector<Property> CollectProperty();
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) {};
    virtual void OnInsertTrackPoint(const size_t index, const glm::vec2 & point) {};
    virtual void OnDeleteTrackPoint(const size_t index, const glm::vec2 & point) {};

protected:
    size_t              _state;
    GLObject *          _owner;
    std::vector<glm::vec2> _trackPoints;
};