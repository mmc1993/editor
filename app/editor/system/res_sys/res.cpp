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

bool Res::Ref::IsModify()
{
    return _modify;
}

void Res::Ref::SetModify()
{
    _modify = true;
}

// ---
//  Res
// ---
Res::Res(uint id)
    : _id(id)
    , _tag(kNull)
    , _metai(~0L)
{ }

Res::~Res()
{ 
    ASSERT_LOG(_refs.empty(), "");
}

std::any Res::Load()
{
    std::any ret;
    switch (_tag)
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

Res::Ref * Res::AppendRef()
{
    return new Ref(this);
}

void Res::DeleteRef(Ref * ref)
{ 
    auto it = std::remove(_refs.begin(), _refs.end(), ref);
    ASSERT_LOG(it != _refs.end(), "");
    _refs.erase(it);
}

void Res::SetTag(TagEnum tag)
{ 
    _tag = tag;
}

Res::TagEnum Res::GetTag()
{ 
    return _tag;
}

std::string Res::GetPath()
{
    std::string path;
    switch (_tag)
    {
    case Res::kTxt:
    case Res::kImg:
    case Res::kMap:
    case Res::kFont:
        path = _metas;
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
