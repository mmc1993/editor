#include "gl_object.h"
#include "component.h"
#include "comp_transform.h"
#include "../ui_sys/ui_object/ui_object.h"

GLObject::GLObject()
    : mID(~0)
    , mState(kActive)
    , mParent(nullptr)
{
    //  PS:
    //      这个构造函数仅在从文件反序列化时使用.
}

GLObject::GLObject(uint id)
    : mID(id)
    , mState(kActive)
    , mParent(nullptr)
{
    mTransform = std::create_ptr<CompTransform>();
    AppendComponent(mTransform);
}

GLObject::~GLObject()
{
    ClearObjects();
    ClearComponents();
}

void GLObject::EncodeBinary(std::ostream & os, Project * project)
{
    //  自身
    tools::Serialize(os, mID);
    tools::Serialize(os, mName);
    tools::Serialize(os, mState);

    //  组件
    auto count = GetComponents().size();
    tools::Serialize(os, count);
    for (const auto & comp : GetComponents())
    {
        tools::Serialize(os,comp->GetName());
        comp->EncodeBinary(os, project);
    }
    //  子节点
    count = GetObjects().size();
    tools::Serialize(os, count);
    for (const auto & obj : GetObjects())
    {
        obj->EncodeBinary(os, project);
    }
}

void GLObject::DecodeBinary(std::istream & is, Project * project)
{
    //  自身
    tools::Deserialize(is, mID);
    tools::Deserialize(is, mName);
    tools::Deserialize(is, mState);
    //  组件
    size_t count;
    tools::Deserialize(is, count);
    for (auto i = 0; i != count; ++i)
    {
        std::string name;
        tools::Deserialize(is, name);

        auto comp = Component::Create(name);
        comp->DecodeBinary(is, project);
        (void)AppendComponent(comp);
    }
    SetTransform(QueryComponent<CompTransform>());

    //  子节点
    count = 0;
    tools::Deserialize(is, count);
    for (auto i = 0; i != count; ++i)
    {
        auto object = std::create_ptr<GLObject>();
        object->DecodeBinary(is, project);
        InsertObject(object);
    }
}

void GLObject::InsertObject(const SharePtr<GLObject> & object, const std::string & name, uint pos)
{
    ASSERT_LOG(object->GetParent() == nullptr, name.c_str());
    auto insert = std::next(mChildren.begin(), 
             std::min(pos, mChildren.size()));
    mChildren.insert(insert, object);
    object->mParent = this;
    object->mName = name;
}

void GLObject::InsertObject(const SharePtr<GLObject> & object, uint pos)
{
    InsertObject(object, object->GetName(), pos);
}

void GLObject::DeleteObject(const SharePtr<GLObject> & object)
{
    auto it = std::find(mChildren.begin(), mChildren.end(), object);
    ASSERT_LOG(it != mChildren.end(), "Object DeleteObject");
    DeleteObject(std::distance(mChildren.begin(), it));
}

void GLObject::DeleteObject(const std::string & name)
{
    auto it = std::find_if(mChildren.begin(), mChildren.end(),
        [name] (const SharePtr<GLObject> & object) 
        { return object->mName == name; });
    if (it != mChildren.end())
    {
        DeleteObject(std::distance(mChildren.begin(), it));
    }
}

void GLObject::DeleteObject(size_t idx)
{
    ASSERT_LOG(idx < mChildren.size(), "Object DeleteObject: {0}", idx);
    auto it = std::next(mChildren.begin(), idx);
    (*it)->mParent = nullptr;
    mChildren.erase(it);
}

void GLObject::ClearObjects()
{
    while (!mChildren.empty())
    {
        DeleteObject(mChildren.back());
    }
}

void GLObject::DeleteThis()
{
    ASSERT_LOG(GetParent() != nullptr, "");
    GetParent()->DeleteObject(shared_from_this());
}

int GLObject::Relation(const SharePtr<GLObject> & target)
{
    //  自己
    if (this == target.get())
    {
        return 1;
    }
    //  长辈
    for (auto parent  = GetParent(); 
              parent != nullptr;
              parent  = parent->GetParent())
    {
        if (parent == target) { return 2; }
    }
    //  晚辈
    std::deque<SharePtr<GLObject>> list{ 
        GetObjects().begin(),
        GetObjects().end() };
    while (!list.empty())
    {
        auto & front = list.front();
        if (front == target)
        {
            return 3;
        }
        list.insert(list.end(),
            front->GetObjects().begin(),
            front->GetObjects().end());
        list.pop_front();
    }
    return 0;
}

SharePtr<GLObject> GLObject::GetObject(const std::string & name)
{
    auto it = std::find_if(mChildren.begin(), mChildren.end(),
        [name] (const SharePtr<GLObject> & object)
        { return object->mName == name; });
    return it != mChildren.end() ? *it : nullptr;
}

SharePtr<GLObject> GLObject::GetObject(const size_t idx)
{
    ASSERT_LOG(idx < mChildren.size(), "Object GetChildIdx: {0}", idx);
    return *std::next(mChildren.begin(), idx);
}

std::vector<SharePtr<GLObject>> & GLObject::GetObjects()
{
    return mChildren;
}

void GLObject::Update(UIObjectGLCanvas * canvas, float dt)
{
    canvas->GetMatrixStack().Mul(RenderPipline::MatrixStack::TypeEnum::kModel, GetTransform()->GetMatrix());

    for (auto component : mComponents)
    {
        if (component->HasState(Component::StateEnum::kActive))
        {
            component->OnStart (canvas    );
            component->OnUpdate(canvas, dt);
        }
    }

    for (auto object : mChildren)
    {
        if (object->HasState(GLObject::StateEnum::kActive))
        {
            object->Update(canvas, dt);
        }
    }

    for (auto component : mComponents)
    {
        if (component->HasState(Component::StateEnum::kActive))
        {
            component->OnLeave(canvas);
        }
    }

    canvas->GetMatrixStack().Pop(RenderPipline::MatrixStack::TypeEnum::kModel);
}

std::string GLObject::SetName(const std::string & name)
{
    auto old = std::move(mName);
    mName.operator=(name);
    return std::move(old);
}

const std::string & GLObject::GetName() const
{
    return mName;
}

void GLObject::AddState(uint state, bool add)
{
    if (add) mState |=  state;
    else     mState &= ~state;
}

uint GLObject::HasState(uint state)
{
    return mState & state;
}

bool GLObject::HasParent(const SharePtr<GLObject> & object)
{
    auto parent = GetParent();
    for (;parent != nullptr && parent != object; 
          parent = parent->GetParent());
    return parent != nullptr;
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
        parent->InsertObject(shared_from_this(), mName);
    }
}

SharePtr<GLObject> GLObject::GetParent()
{
    return mParent != nullptr? mParent->shared_from_this(): nullptr;
}

void GLObject::SetTransform(const SharePtr<CompTransform> & transform)
{
    ASSERT_LOG(mTransform == nullptr, "");
    mTransform = transform;
}

SharePtr<CompTransform> GLObject::GetTransform()
{
    ASSERT_LOG(mTransform != nullptr, "");
    return mTransform;
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
	while (!mComponents.empty())
	{
        mComponents.back()->OnDelete();
        mComponents.back()->SetOwner(nullptr);
		mComponents.pop_back();
	}
}

void GLObject::AppendComponent(const SharePtr<Component> & component)
{
    ASSERT_LOG(mID != ~0, "");
    mComponents.push_back(component);
    component->SetOwner(this);
    component->OnAppend();
}

void GLObject::DeleteComponent(const SharePtr<Component> & component)
{
    auto it = std::find(mComponents.begin(), mComponents.end(), component);
    if (it != mComponents.end()) {(*it)->OnDelete();mComponents.erase(it);}
}

void GLObject::DeleteComponent(const std::type_info & type)
{
    auto it = std::find_if(mComponents.begin(), mComponents.end(),
        [&](const SharePtr<Component> & component) 
        { return typeid(*component) == type; });
    if (it != mComponents.end()) 
    { 
        (*it)->OnDelete(); mComponents.erase(it);
    }
}

std::vector<SharePtr<Component>> & GLObject::GetComponents()
{
    return mComponents;
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
