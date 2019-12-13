#include "res.h"
#include "project.h"

const std::string Res::sTypeString[TypeEnum::Length] = {
    "NULL", "TXT", "IMG", "MAP", "FNT", "OBJ", "VAR", "BLUEPRINT"
};

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
    : mID(id)
    , mType(kNull)
    , mOwner(owner)
{ }

Res::~Res()
{
    for (auto & ref : mRefs)
    {
        ref->_owner = nullptr;
    }
}

template <>
SharePtr<GLObject> Res::Instance()
{
    ASSERT_LOG(Type() == Res::kObj, "");
    return mOwner->GetObject(std::any_cast<uint>(mMeta));
}

uint Res::GetRefCount()
{
    return mRefs.size();
}

uint Res::GetID()
{
    return mID;
}

void Res::WakeRefs()
{ 
    for (auto & ref : mRefs)
    {
        ref->Modify(true);
    }
}

Res::Ref * Res::AppendRef()
{
    return mRefs.emplace_back(new Ref(this));
}

Res::Ref * Res::AppendRef(Ref * ref)
{
    ref->_owner = this;
    return mRefs.emplace_back(ref);
}

void Res::DeleteRef(Ref * ref)
{ 
    auto it = std::remove(mRefs.begin(), mRefs.end(), ref);
    ASSERT_LOG(it != mRefs.end(), ""); 
    mRefs.erase(it);
    delete ref;
}

Res::TypeEnum Res::Type()
{ 
    return mType;
}

Res::TypeEnum Res::Type(TypeEnum type)
{ 
    mType = type;
    return mType;
}

const std::string & Res::TypeString()
{
    return TypeString(Type());
}

const std::string & Res::TypeString(uint type)
{
    return sTypeString[type];
}

void Res::EncodeBinary(std::ostream & os, Project * project)
{
    tools::Serialize(os, mID);
    tools::Serialize(os, mType);
    switch (mType)
    {
    case Res::kNull:
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFnt:
        tools::Serialize(os, std::any_cast<std::string &>(mMeta));
        break;
    case Res::kObj:
        tools::Serialize(os, std::any_cast<uint>(mMeta));
        break;
    case Res::kVar:
        tools::Serialize(os, std::any_cast<uint>(mMeta));
        break;
    case Res::kBlueprint:
        tools::Serialize(os, std::any_cast<uint>(mMeta));
        break;
    }
}

void Res::DecodeBinary(std::istream & is, Project * project)
{
    tools::Deserialize(is, mID);
    tools::Deserialize(is, mType);
    switch (mType)
    {
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFnt:
        {
            mMeta.emplace<std::string>();
            tools::Deserialize(is, std::any_cast<std::string &>(mMeta));
        }
        break;
    case Res::kObj:
        {
            mMeta.emplace<uint>();
            tools::Deserialize(is, std::any_cast<uint &>(mMeta));
        }
        break;
    case Res::kVar:
        {
            mMeta.emplace<uint>();
            tools::Deserialize(is, std::any_cast<uint &>(mMeta));
        }
        break;
    case Res::kBlueprint:
        {
            mMeta.emplace<uint>();
            tools::Deserialize(is, std::any_cast<uint &>(mMeta));
        }
        break;
    }
}

std::string Res::Path()
{
    std::string path;
    switch (mType)
    {
    case Res::kNull:
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFnt:
        {
            path = std::any_cast<std::string>(mMeta);
        }
        break;
    case Res::kObj:
        {
            auto object = Instance<GLObject>();
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

