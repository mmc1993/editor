#include "gl_object.h"
#include "component.h"
#include "comp_transform.h"

GLObject::GLObject()
    : _active(true)
    , _parent(nullptr)
{ }

GLObject::~GLObject()
{
    ClearObjects();
    ClearComponent();
}

void GLObject::OnUpdate(float dt)
{ }

void GLObject::AddObject(GLObject * child, const std::string & tag)
{
    child->_tag = tag;
    child->_parent = this;
    _children.push_back(child);
}

void GLObject::DelObject(GLObject * child, const bool del)
{
    auto it = std::find(_children.begin(), _children.end(), child);
    ASSERT_LOG(it != _children.end(), "Object DelChildIdx");
    DelObjectByIdx(std::distance(_children.begin(), it), del);
}

void GLObject::DelObjectByIdx(size_t idx, const bool del)
{
    ASSERT_LOG(idx < _children.size(), "Object DelChild Idx: {0}", idx);
    auto it = std::next(_children.begin(), idx);
    (*it)->_parent = nullptr;
    if (del) { delete *it; }
    _children.erase(it);
}

void GLObject::DelObjectByTag(const std::string & tag)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [tag](GLObject * child) { return child->_tag == tag; });
    if (it != _children.end())
    {
        DelObjectByIdx(std::distance(_children.begin(), it), true);
    }
}

void GLObject::ClearObjects()
{
    while (!_children.empty())
    {
        DelObject(_children.back());
    }
}

void GLObject::DelThis()
{
    if (nullptr == GetParent()) { delete this; }
    else { GetParent()->DelObject(this); }
}

GLObject * GLObject::GetObjectByTag(const std::string & tag)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [tag](GLObject * child) { return child->_tag == tag; });
    return it != _children.end() ? *it : nullptr;
}

GLObject * GLObject::GetObjectByIdx(const size_t idx)
{
    ASSERT_LOG(idx < _children.size(), "Object GetChildIdx Idx: {0}", idx);
    return *std::next(_children.begin(), idx);
}

std::vector<GLObject *> & GLObject::GetObjects()
{
    return _children;
}

const std::string & GLObject::GetTag() const
{
    return _tag;
}

void GLObject::SetActive(bool active)
{
    _active = active;
}

bool GLObject::IsActive() const
{
    return _active;
}

void GLObject::Update(float dt)
{ }

void GLObject::RootUpdate(float dt)
{ }

void GLObject::SetParent(GLObject * parent)
{
    if (nullptr != _parent)
    {
        _parent->DelObject(this, false);
    }
    if (nullptr != parent)
    {
        parent->AddObject(this, _tag);
    }
}

GLObject * GLObject::GetParent()
{
    return _parent;
}

void GLObject::ClearComponent()
{
	while (!_components.empty())
	{
        _components.back()->OnDel();
        _components.back()->SetOwner(nullptr);
		delete _components.back();
		_components.pop_back();
	}
}

void GLObject::AddComponent(Component * component)
{
    _components.push_back(component);
    component->SetOwner(this);
    component->OnAdd();
}

void GLObject::DelComponent(Component * component)
{
    auto it = std::find(_components.begin(), _components.end(), component);
    if (it != _components.end()) { (*it)->OnDel(); delete *it; _components.erase(it); }
}

void GLObject::DelComponent(const std::type_info & type)
{
    auto it = std::find_if(_components.begin(), _components.end(),
        [&type](Component * component) { return typeid(*component) == type; });
    if (it != _components.end()) { (*it)->OnDel(); delete *it; _components.erase(it); }
}

std::vector<Component*>& GLObject::GetComponents()
{
    return _components;
}

CompTransform * GLObject::GetTransform()
{
    return _transform;
}
