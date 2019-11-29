#include "res.h"
#include "../editor_sys/editor_sys.h"
#include "../editor_sys/component/gl_object.h"

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
Res::Res(uint id)
    : _id(id)
    , _type(kNull)
    , _metai(~0L)
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
        ret = _metas;
        break;
    case Res::kObj:
        ret = Global::Ref().mEditorSys->GetProject()->GetObject(_metai);
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
    if (_metai != val)
    {
        _metai = val;
        WakeRefs();
    }
}

void Res::BindMeta(const std::string & val)
{ 
    if (_metas != val)
    {
        _metas = val;
        WakeRefs();
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
            path = _metas;
        }
        break;
    case Res::kObj:
        {
            auto object = Global::Ref().mEditorSys->GetProject()->GetObject(_metai);
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