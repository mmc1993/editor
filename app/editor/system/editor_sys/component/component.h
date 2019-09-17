#pragma once

#include "gl_object.h"
#include "../property/property.h"
#include "../../tools/parser_tool.h"

class Component {
public:
    static Component * Create(const std::string & tag);

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

    //  根据Key, Val, 初始化对应的成员变量
    virtual bool Parse(const std::string & key, const std::string & val) = 0;
    //  创建Property列表, 用于界面展示修改
    virtual std::vector<std::pair<size_t, void *>> CollectPropertys() = 0;
    //  Property修改时被调用
    virtual bool OnModifyProperty(
        const std::any    & value,
        const std::string & title,
        const std::any    & backup) = 0;

private:
    bool _active;
    GLObject *_owner;
};