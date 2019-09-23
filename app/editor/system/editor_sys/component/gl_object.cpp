#include "gl_object.h"
#include "component.h"
#include "comp_transform.h"

GLObject::GLObject()
    : _active(true)
    , _parent(nullptr)
{ }

GLObject::~GLObject()
{
    ClearComponents();
    ClearObjects();
}

void GLObject::OnUpdate(float dt)
{ }

void GLObject::EncodeBinary(std::ofstream & os)
{
    auto count = GetComponents().size();
    tools::Serialize(os, count);

    for (auto comp : GetComponents())
    {
        tools::Serialize(os, comp->GetName());
        comp->EncodeBinary(os);
    }
}

void GLObject::DecodeBinary(std::ifstream & is)
{
    size_t count;
    tools::Deserialize(is, count);

    std::string name;
    for (auto i = 0; i != count; ++i)
    {
        tools::Deserialize(is, name);
        auto comp = Component::Create(name);
        comp->DecodeBinary(is);
        AddComponent(comp);
    }
}

void GLObject::AddObject(GLObject * child, const std::string & name)
{
    child->_name = name;
    child->_parent = this;
    _children.push_back(child);
}

void GLObject::DelObject(const std::string & name, bool del)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [name](GLObject * child){return child->_name==name;});
    if (it != _children.end())
    {
        DelObject(std::distance(_children.begin(), it), del);
    }
}

void GLObject::DelObject(GLObject * child, const bool del)
{
    auto it = std::find(_children.begin(), _children.end(), child);
    ASSERT_LOG(it != _children.end(), "Object DelChildIdx");
    DelObject(std::distance(_children.begin(), it), del);
}

void GLObject::DelObject(size_t idx, const bool del)
{
    ASSERT_LOG(idx < _children.size(), "Object DelChild Idx: {0}", idx);
    auto it = std::next(_children.begin(), idx);
    (*it)->_parent = nullptr;
    if (del) { delete *it; }
    _children.erase(it);
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

GLObject * GLObject::GetObject(const std::string & name)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [name](GLObject * child){return child->_name==name;});
    return it != _children.end() ? *it : nullptr;
}

GLObject * GLObject::GetObject(const size_t idx)
{
    ASSERT_LOG(idx < _children.size(), "Object GetChildIdx Idx: {0}", idx);
    return *std::next(_children.begin(), idx);
}

std::vector<GLObject *> & GLObject::GetObjects()
{
    return _children;
}

const std::string & GLObject::GetName() const
{
    return _name;
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
        parent->AddObject(this, _name);
    }
}

GLObject * GLObject::GetParent()
{
    return _parent;
}

void GLObject::ClearComponents()
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
