#include "editor_sys.h"
#include "component/gl_object.h"
#include "../ui_sys/ui_object/ui_object.h"

EditorSys::EditorSys(): mRootObject(new GLObject())
{ }

std::string EditorSys::GenerateObjectName(GLObject * object) const
{
    size_t i = 0;
    auto name = SFormat("object_{0}", i++);
    while (object->GetObject(name))
        name = SFormat("object_{0}", i++);
    return std::move(name);
}

bool EditorSys::CheckRename(GLObject * object, const std::string & name) const
{
    if (name.empty()) { return false; }
    return object->GetParent()->GetObject(name) == nullptr;
}
