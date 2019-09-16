#include "object.h"
#include "component.h"

Object::Object()
    : _tag(~0)
    , _active(true)
    , _parent(nullptr)
{ }

Object::~Object()
{
    ClearChildren();
    ClearComponent();
}

void Object::OnUpdate(float dt)
{ }

void Object::AddChild(Object * child, size_t tag)
{
    child->_tag = tag;
    child->_parent = this;
    _children.push_back(child);
}

void Object::DelChild(Object * child, bool del)
{
    auto it = std::find(_children.begin(), _children.end(), child);
    ASSERT_LOG(it != _children.end(), "Object DelChildIdx");
    DelChildIdx(std::distance(_children.begin(), it), del);
}

void Object::DelChildIdx(size_t idx, bool del)
{
    ASSERT_LOG(idx < _children.size(), "Object DelChild Idx: {0}", idx);
    auto it = std::next(_children.begin(), idx);
    (*it)->_parent = nullptr;
    if (del) { delete *it; }
    _children.erase(it);
}

void Object::DelChildTag(size_t tag, bool del)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [tag](Object * child) { return child->_tag == tag; });
    if (it != _children.end())
    {
        DelChildIdx(std::distance(_children.begin(), it), true);
    }
}

void Object::DelThis()
{
	if (nullptr == GetParent()) { delete this; }
	else { GetParent()->DelChild(this); }
}

void Object::ClearChildren()
{
    while (!_children.empty())
    {
        DelChild(_children.back());
    }
}

Object * Object::GetChildTag(size_t tag)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [tag](Object * child) { return child->_tag == tag; });
    return it != _children.end() ? *it : nullptr;
}

Object * Object::GetChildIdx(size_t idx)
{
    ASSERT_LOG(idx < _children.size(), "Object GetChildIdx Idx: {0}", idx);
    return *std::next(_children.begin(), idx);
}

std::vector<Object *> & Object::GetChildren()
{
    return _children;
}

void Object::ClearComponent()
{
	while (!_components.empty())
	{
        _components.back()->OnDel();
        _components.back()->SetOwner(nullptr);
		delete _components.back();
		_components.pop_back();
	}
}

void Object::AddComponent(Component * component)
{
    _components.push_back(component);
    component->SetOwner(this);
    component->OnAdd();
}

void Object::DelComponent(const std::type_info & type)
{
    auto it = std::find_if(_components.begin(), _components.end(),
        [&type](Component * component) { return typeid(*component) == type; });
    if (it != _components.end()) 
    {
        _components.erase(it);
        (*it)->OnDel();
        delete *it;
    }
}

std::vector<Component*>& Object::GetComponents()
{
    return _components;
}

void Object::SetActive(bool active)
{
    _active = active;
}

bool Object::IsActive() const
{
    return _active;
}

void Object::Update(float dt)
{ }

void Object::RootUpdate(float dt)
{ }

void Object::SetParent(Object * parent)
{
    if (nullptr != _parent)
    {
        _parent->DelChild(this, false);
    }
    if (nullptr != parent)
    {
        parent->AddChild(this, _tag);
    }
}

Object * Object::GetParent()
{
    return _parent;
}
