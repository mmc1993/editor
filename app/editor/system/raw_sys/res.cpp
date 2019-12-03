#include "res.h"
#include "project.h"

// ---
//  Res::Ref
// ---
Res::Ref::Ref(Res * owner)
    : _owner(owner)
    , _modify(true)
{ }

Res::Ref::~Ref()
{
    if (_owner != nullptr)
    {
        _owner->DeleteRef(this);
    }
}

Res::Ref * Res::Ref::Clone()
{
    ASSERT_LOG(_owner != nullptr, "");
    return _owner->AppendRef();
}

bool Res::Ref::Modify()
{
    return _modify;
}

bool Res::Ref::Modify(bool modify)
{
    _modify = true;
    return _modify;
}

void Res::Ref::EncodeBinary(Project * project, std::ofstream & os)
{
    tools::Serialize(os, _owner->GetID());
}

void Res::Ref::DecodeBinary(Project * project, std::ifstream & is)
{
    uint id = 0;
    tools::Deserialize(is, id);
    project->GetRes(id)->AppendRef(this);
}

// ---
//  Res
// ---
Res::Res(Project * owner, uint id)
    : _id(id)
    , _type(kNull)
    , _owner(owner)
{ }

Res::~Res()
{
    for (auto & ref : _refs)
    {
        ref->_owner = nullptr;
    }
}

std::any Res::Instance()
{
    std::any ret;
    switch (_type)
    {
    case Res::kNull:
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFnt:
        ret = std::any_cast<std::string>(_meta);
        break;
    case Res::kObj:
        ret = std::any_cast<SharePtr<GLObject>>(_meta);
        break;
    case Res::kVar:
        ASSERT_LOG(false, "");
        break;
    case Res::kBlueprint:
        ASSERT_LOG(false, "");
        break;
    }
    return std::move(ret);
}

uint Res::GetRefCount()
{
    return _refs.size();
}

uint Res::GetID()
{
    return _id;
}

Res::Ref * Res::AppendRef()
{
    return _refs.emplace_back(new Ref(this));
}

Res::Ref * Res::AppendRef(Ref * ref)
{
    ref->_owner = this;
    return _refs.emplace_back(ref);
}

void Res::DeleteRef(Ref * ref)
{ 
    auto it = std::remove(_refs.begin(), _refs.end(), ref);
    ASSERT_LOG(it != _refs.end(), ""); 
    _refs.erase(it);
    delete ref;
}

Res::TypeEnum Res::Type()
{ 
    return _type;
}

Res::TypeEnum Res::Type(TypeEnum type)
{ 
    _type = type;
    return _type;
}

void Res::EncodeBinary(std::ofstream & os)
{
    tools::Serialize(os, _id);
    tools::Serialize(os, _type);
    switch (_type)
    {
    case Res::kNull:
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFnt:
        tools::Serialize(os, std::any_cast<std::string &>(_meta));
        break;
    case Res::kObj:
        tools::Serialize(os, std::any_cast<SharePtr<GLObject>>(_meta)->GetID());
        break;
    case Res::kVar:
        break;
    case Res::kBlueprint:
        break;
    }
}

void Res::DecodeBinary(std::ifstream & is)
{
    tools::Deserialize(is, _id);
    tools::Deserialize(is, _type);
    switch (_type)
    {
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFnt:
        _meta.emplace<std::string>();
        tools::Deserialize(is, std::any_cast<std::string &>(_meta));
        break;
    case Res::kObj:
        {
            uint id = 0;
            tools::Deserialize(is, id);
            _meta = _owner->GetObject(id);
        }
        break;
    case Res::kVar:
        {
            uint id = 0;
            tools::Deserialize(is, id);
            //  读取变量
        }
        break;
    case Res::kBlueprint:
        { 
            uint id = 0;
            tools::Deserialize(is, id);
            //  读取蓝图
        }
        break;
    }
}

std::string Res::Path()
{
    std::string path;
    switch (_type)
    {
    case Res::kNull:
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFnt:
        {
            path = std::any_cast<std::string>(_meta);
        }
        break;
    case Res::kObj:
        {
            auto object = std::any_cast<SharePtr<GLObject>>(_meta);
            path.append(object->GetName());

            for (   object = object->GetParent(); 
                    object->GetParent() != nullptr; 
                    object = object->GetParent())
            {
                path.insert(0, "/");
                path.insert(0, object->GetName());
            }
        }
        break;
    case Res::kVar:
        ASSERT_LOG(false, "");
        break;
    case Res::kBlueprint:
        ASSERT_LOG(false, "");
        break;
    }
    return std::move(path);
}

void Res::WakeRefs()
{ 
    for (auto & ref : _refs)
    {
        ref->Modify(true);
    }
}