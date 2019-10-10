#include "gl_object.h"
#include "component.h"
#include "comp_transform.h"

GLObject::GLObject()
    : _status(kActive)
    , _canvas(nullptr)
{
    _transform = std::create_ptr<CompTransform>();
    AddComponent(_transform);
}

GLObject::~GLObject()
{
    ClearObjects();
    ClearComponents();
}

void GLObject::OnUpdate(float dt)
{ }

void GLObject::EncodeBinary(std::ofstream & os)
{
    //  自身
    tools::Serialize(os, _name);
    
    //  组件
    auto count = GetComponents().size();
    tools::Serialize(os, count);
    for (const auto & comp : GetComponents())
    {
        tools::Serialize(os,comp->GetName());
        comp->EncodeBinary(os);
    }

    //  子节点
    count = GetObjects().size();
    tools::Serialize(os, count);
    for (const auto & obj : GetObjects())
    {
        obj->EncodeBinary(os);
    }
}

void GLObject::DecodeBinary(std::ifstream & is)
{
    //  自身
    tools::Deserialize(is, _name);
    //  组件
    size_t count;
    std::string name;
    tools::Deserialize(is, count);
    for (auto i = 0; i != count; ++i)
    {
        tools::Deserialize(is, name);
        auto comp = Component::Create(name);
        comp->DecodeBinary(is);
        AddComponent(comp);
    }
    //  子节点
    tools::Deserialize(is, count);
    for (auto i = 0; i != count; ++i)
    {
        auto object = std::create_ptr<GLObject>();
        object->DecodeBinary(is);
        AddObject(object, object->GetName());
    }
}

void GLObject::AddObject(const SharePtr<GLObject> & object, const std::string & name)
{
    ASSERT_LOG(object->GetParent() == nullptr, name.c_str());
    object->_name = name;
    object->_parent = shared_from_this();
    _children.push_back(object);
}

void GLObject::DelObject(const SharePtr<GLObject> & object)
{
    auto it = std::find(_children.begin(), _children.end(), object);
    ASSERT_LOG(it != _children.end(), "Object DelChildIdx");
    DelObject(std::distance(_children.begin(), it));
}

void GLObject::DelObject(const std::string & name)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [name] (const SharePtr<GLObject> & object) 
        { return object->_name == name; });
    if (it != _children.end())
    {
        DelObject(std::distance(_children.begin(), it));
    }
}

void GLObject::DelObject(size_t idx)
{
    ASSERT_LOG(idx < _children.size(), "Object DelChildIdx: {0}", idx);
    auto it = std::next(_children.begin(), idx);
    (*it)->_parent.reset();
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
    ASSERT_LOG(GetParent() != nullptr, "");
    GetParent()->DelObject(shared_from_this());
}

SharePtr<GLObject> GLObject::GetObject(const std::string & name)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [name] (const SharePtr<GLObject> & object)
        { return object->_name == name; });
    return it != _children.end() ? *it : nullptr;
}

SharePtr<GLObject> GLObject::GetObject(const size_t idx)
{
    ASSERT_LOG(idx < _children.size(), "Object GetChildIdx: {0}", idx);
    return *std::next(_children.begin(), idx);
}

std::vector<SharePtr<GLObject>> & GLObject::GetObjects()
{
    return _children;
}

void GLObject::SetName(const std::string & name)
{
    _name = name;
}

const std::string & GLObject::GetName() const
{
    return _name;
}

UIObjectGLCanvas * GLObject::GetCanvas()
{
    return _canvas;
}

void GLObject::SetActive(bool active)
{
    if (active)
        _status |=  kActive;
    else
        _status &= ~kActive;
}

bool GLObject::IsActive() const
{
    return _status & kActive;
}

void GLObject::Update(float dt)
{
    
}

void GLObject::SetParent(const SharePtr<GLObject> & parent)
{
    if (GetParent() != nullptr)
    {
        GetParent()->DelObject(shared_from_this());
    }
    if (parent != nullptr)
    {
        parent->AddObject(shared_from_this(), _name);
    }
}

SharePtr<GLObject> GLObject::GetParent()
{
    return _parent.expired() ? nullptr : _parent.lock();
}

void GLObject::ClearComponents()
{
	while (!_components.empty())
	{
        _components.back()->OnDel();
        _components.back()->SetOwner(nullptr);
		_components.pop_back();
	}
}

void GLObject::AddComponent(const SharePtr<Component> & component)
{
    _components.push_back(component);
    component->SetOwner(shared_from_this());
    component->OnAdd();
}

void GLObject::DelComponent(const SharePtr<Component> & component)
{
    auto it = std::find(_components.begin(), _components.end(), component);
    if (it != _components.end()) { (*it)->OnDel(); _components.erase(it); }
}

void GLObject::DelComponent(const std::type_info & type)
{
    auto it = std::find_if(_components.begin(), _components.end(),
        [&type](const SharePtr<Component> & component) 
        { return typeid(*component) == type; });
    if (it != _components.end()) { (*it)->OnDel(); _components.erase(it); }
}

std::vector<SharePtr<Component>> & GLObject::GetComponents()
{
    return _components;
}

SharePtr<CompTransform> GLObject::GetTransform()
{
    ASSERT_LOG(_transform != nullptr, "");
    return _transform;
}
