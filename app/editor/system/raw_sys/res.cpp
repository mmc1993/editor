#include "res.h"
#include "project.h"
#include "raw_sys.h"

const std::string Res::sTypeString[TypeEnum::Length] = {
    "NULL", "TXT", "IMG", "MAP", "FNT", "OBJ", "VAR", "BLUEPRINT"
};

void Res::Ref::EncodeBinary(std::ostream & os, Project * project)
{ 
    uint id = Vaild() ? mOwner->GetID() : ~0;
    tools::Serialize(os, id);
}

void Res::Ref::DecodeBinary(std::istream & is, Project * project)
{ 
    uint id;
    tools::Deserialize(is, id);
    if (id != ~0) { mOwner.reset(project->GetRes(id)->AppendPtr()); }
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
    for (auto & ptr : mPtrs)
    {
        ptr->Invalid();
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

            for (object = object->GetParent(); 
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


uint Res::GetRefCount()
{
    return mPtrs.size();
}

uint Res::GetID()
{
    return mID;
}

void Res::WakeRefs()
{ 
    for (auto & ptr : mPtrs)
    {
        ptr->Modify(true);
    }
}

Res::Ptr * Res::AppendPtr()
{
    return mPtrs.emplace_back(new Ptr(this));
}

Res::Ref Res::AppendRef()
{
    return Ref(AppendPtr());
}

void Res::DeletePtr(Ptr * ptr)
{
    auto it = std::remove(mPtrs.begin(), mPtrs.end(), ptr);
    ASSERT_LOG(it != mPtrs.end(), ""); 
    mPtrs.erase(it);
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
    case Res::kNull:
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

SharePtr<GLObject> Res::InstanceObj()
{
    ASSERT_LOG(Type() == Res::kObj, "");
    return mOwner->GetObject(std::any_cast<uint>(mMeta));
}

SharePtr<GLTexture> Res::InstanceTex()
{
    ASSERT_LOG(Type() == Res::kImg, "");
    return Global::Ref().mRawSys->Get<GLTexture>(Path());
}


