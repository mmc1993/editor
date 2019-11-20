#include "gl_object.h"
#include "component.h"
#include "comp_transform.h"
#include "../../ui_sys/ui_object/ui_object.h"

GLObject::GLObject()
    : _id(~0)
    , _state(kActive)
    , _parent(nullptr)
{
    //  PS:
    //      这个构造函数仅在从文件反序列化时使用.
}

GLObject::GLObject(uint id)
    : _id(id)
    , _state(kActive)
    , _parent(nullptr)
{
    _transform = std::create_ptr<CompTransform>();
    AddComponent(_transform);
}

GLObject::~GLObject()
{
    ClearObjects();
    ClearComponents();
}

void GLObject::EncodeBinary(std::ofstream & os)
{
    //  自身
    tools::Serialize(os, _id);
    tools::Serialize(os, _name);
    tools::Serialize(os, _state);

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
    tools::Deserialize(is, _id);
    tools::Deserialize(is, _name);
    tools::Deserialize(is, _state);
    //  组件
    size_t count;
    tools::Deserialize(is, count);
    for (auto i = 0; i != count; ++i)
    {
        std::string name;
        tools::Deserialize(is, name);

        auto comp = Component::Create(name);
        comp->DecodeBinary(is);
        AddComponent(comp);
    }
    SetTransform(GetComponent<CompTransform>());

    //  子节点
    count = 0;
    tools::Deserialize(is, count);
    for (auto i = 0; i != count; ++i)
    {
        auto object = std::create_ptr<GLObject>();
        object->DecodeBinary(is);
        InsertObject(object);
    }
}

void GLObject::InsertObject(const SharePtr<GLObject> & object, const std::string & name, uint pos)
{
    ASSERT_LOG(object->GetParent() == nullptr, name.c_str());
    auto insert = std::next(_children.begin(), 
             std::min(pos, _children.size()));
    _children.insert(insert, object);
    object->_parent = this;
    object->_name = name;
}

void GLObject::InsertObject(const SharePtr<GLObject> & object, uint pos)
{
    InsertObject(object, object->GetName(), pos);
}

void GLObject::DeleteObject(const SharePtr<GLObject> & object)
{
    auto it = std::find(_children.begin(), _children.end(), object);
    ASSERT_LOG(it != _children.end(), "Object DelChildIdx");
    DeleteObject(std::distance(_children.begin(), it));
}

void GLObject::DeleteObject(const std::string & name)
{
    auto it = std::find_if(_children.begin(), _children.end(),
        [name] (const SharePtr<GLObject> & object) 
        { return object->_name == name; });
    if (it != _children.end())
    {
        DeleteObject(std::distance(_children.begin(), it));
    }
}

void GLObject::DeleteObject(size_t idx)
{
    ASSERT_LOG(idx < _children.size(), "Object DelChildIdx: {0}", idx);
    auto it = std::next(_children.begin(), idx);
    (*it)->_parent = nullptr;
    _children.erase(it);
}

void GLObject::ClearObjects()
{
    while (!_children.empty())
    {
        DeleteObject(_children.back());
    }
}

void GLObject::DeleteThis()
{
    ASSERT_LOG(GetParent() != nullptr, "");
    GetParent()->DeleteObject(shared_from_this());
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

void GLObject::Update(UIObjectGLCanvas * canvas, float dt)
{
    canvas->GetMatrixStack().Mul(interface::MatrixStack::kModel, GetTransform()->GetMatrix());

    for (auto component : _components)
    {
        if (component->HasState(Component::StateEnum::kActive))
        {
            component->OnStart (canvas    );
            component->OnUpdate(canvas, dt);
        }
    }

    for (auto object : _children)
    {
        if (object->HasState(GLObject::StateEnum::kActive))
        {
            object->Update(canvas, dt);
        }
    }

    for (auto component : _components)
    {
        if (component->HasState(Component::StateEnum::kActive))
        {
            component->OnLeave(canvas);
        }
    }

    canvas->GetMatrixStack().Pop(interface::MatrixStack::kModel);
}

std::string GLObject::SetName(const std::string & name)
{
    auto old = std::move(_name);
    _name.operator=(name);
    return std::move(old);
}

const std::string & GLObject::GetName() const
{
    return _name;
}

void GLObject::AddState(uint state, bool add)
{
    if (add) _state |=  state;
    else     _state &= ~state;
}

uint GLObject::HasState(uint state)
{
    return _state & state;
}

void GLObject::SetParent(const SharePtr<GLObject> & parent)
{
    SetParent(parent.get());
}

void GLObject::SetParent(GLObject * parent)
{
    if (GetParent() != nullptr)
    {
        GetParent()->DeleteObject(shared_from_this());
    }
    if (parent != nullptr)
    {
        parent->InsertObject(shared_from_this(), _name);
    }
}

SharePtr<GLObject> GLObject::GetParent()
{
    return _parent != nullptr? _parent->shared_from_this(): nullptr;
}

void GLObject::SetTransform(const SharePtr<CompTransform> & transform)
{
    ASSERT_LOG(_transform == nullptr, "");
    _transform = transform;
}

SharePtr<CompTransform> GLObject::GetTransform()
{
    ASSERT_LOG(_transform != nullptr, "");
    return _transform;
}

glm::mat4 GLObject::GetWorldMatrix()
{
    return GetTransform()->GetMatrixFromRoot();
}

const glm::mat4 & GLObject::GetLocalMatrix()
{
    return GetTransform()->GetMatrix();
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
    ASSERT_LOG(_id != ~0, "");
    _components.push_back(component);
    component->SetOwner(this);
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

glm::vec2 GLObject::WorldToLocal(const glm::vec2 & point)
{
    return glm::inverse(GetWorldMatrix()) * glm::vec4(point, 0, 1);
}

glm::vec2 GLObject::LocalToWorld(const glm::vec2 & point)
{
    return GetWorldMatrix() * glm::vec4(point, 0, 1);
}

glm::vec2 GLObject::ParentToLocal(const glm::vec2 & point)
{
    return glm::inverse(GetLocalMatrix()) * glm::vec4(point, 0, 1);
}

glm::vec2 GLObject::LocalToParent(const glm::vec2 & point)
{
    return GetLocalMatrix() * glm::vec4(point, 0, 1);
}
