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
    return new Ref(this);
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