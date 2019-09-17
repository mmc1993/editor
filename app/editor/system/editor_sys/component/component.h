#pragma once

#include "gl_object.h"

class Component {
public:
    static Component * Create(const std::string & tag);

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

private:
    bool _active;
    GLObject *_owner;
};