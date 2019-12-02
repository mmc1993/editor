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
    ASSERT_LOG(_owner != nullptr, "");
    _owner->DeleteRef(this);
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
    ASSERT_LOG(_refs.empty(), "");
}

std::any Res::Instance()
{
    std::any ret;
    switch (_type)
    {
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFont:
        ret = std::any_cast<std::string>(_meta);
        break;
    case Res::kObj:
        ret = _owner->GetObject(std::any_cast<uint>(_meta));
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

void Res::BindMeta(const uint val)
{
    if (std::any_cast<uint>(_meta) != val)
    {
        _meta = val; WakeRefs();
    }
}

void Res::BindMeta(const std::string & val)
{ 
    if (std::any_cast<std::string>(_meta) != val)
    {
        _meta = val; WakeRefs();
    }
}

void Res::EncodeBinary(std::ofstream & os)
{
    tools::Serialize(os, _id);
    tools::Serialize(os, _type);
    switch (_type)
    {
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFont:
        tools::Serialize(os, std::any_cast<std::string &>(_meta));
        break;
    case Res::kObj:
        tools::Serialize(os, std::any_cast<uint &>(_meta));
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
    case Res::kFont:
        _meta.emplace<std::string>();
        tools::Deserialize(is, std::any_cast<std::string &>(_meta));
        break;
    case Res::kObj:
        _meta.emplace<uint>();
        tools::Deserialize(is, std::any_cast<uint &>(_meta));
        break;
    case Res::kVar:
        break;
    case Res::kBlueprint:
        break;
    }
}

std::string Res::Path()
{
    std::string path;
    switch (_type)
    {
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFont:
        {
            path = std::any_cast<std::string>(_meta);
        }
        break;
    case Res::kObj:
        {
            auto object = _owner->GetObject(std::any_cast<uint>(_meta));
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