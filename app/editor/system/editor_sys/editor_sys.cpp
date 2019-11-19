#include "editor_sys.h"
#include "component/gl_object.h"
#include "../event_sys/event_sys.h"

//  选择文件夹对话框
#include<Shlobj.h>
#include<Commdlg.h>
#pragma comment(lib,"Shell32.lib")

void EditorSys::OptInsertObject(const SharePtr<GLObject> & object, const SharePtr<GLObject> & parent)
{
    parent->AddObject(object, object->GetName());
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kInsertObject, std::make_tuple(object));
}

void EditorSys::OptSelectObject(const SharePtr<GLObject> & object, bool select, bool multi)
{
    auto iter = std::find(_selected.begin(), _selected.end(), object);
    auto has = iter != _selected.end();
    if (select)
    {
        if (!multi)
        {
            while (!_selected.empty() && (_selected.back() != object || _selected.back() != _selected.front()))
            {
                if (_selected.back() == object)
                    OptSelectObject(_selected.front(), false, false);
                else
                    OptSelectObject(_selected.back(), false, false);
            }
        }

        if (!has && object)
        {
            _selected.push_back(object);
            Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSelectObject, std::make_tuple(object, true, multi));
        }
    }
    else
    {
        if (has)
        {
            _selected.erase(iter);
            Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSelectObject, std::make_tuple(object, false, multi));
        }
    }
}

void EditorSys::OptSelectObject(uint id, bool select, bool multi)
{
    OptSelectObject(GetProject()->GetObject(id), select, multi);
}

void EditorSys::OptDeleteObject(const SharePtr<GLObject> & object)
{
    auto temp = object;
    OptSelectObject(temp,  false);
    GetProject()->DelObject(temp);
    temp->DelThis();
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteObject, std::make_tuple(temp));
}

void EditorSys::OptDeleteObject(uint id)
{
    OptDeleteObject(GetProject()->GetObject(id));
}

void EditorSys::OptRenameObject(const SharePtr<GLObject> & object, const std::string & name)
{
    if (ObjectName(object->GetParent(), name))
    {
        auto old = object->SetName(name);
        auto arg = std::make_tuple(object, old);
        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kRenameObject, arg);
    }
}

void EditorSys::OptRenameObject(uint id, const std::string & name)
{
    OptRenameObject(GetProject()->GetObject(id), name);
}

void EditorSys::OptAppendComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component)
{
    object->AddComponent(component);
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kAppendComponent, std::make_tuple(object, component));
}

void EditorSys::OptAppendComponent(uint id, const SharePtr<Component>& component)
{
    OptAppendComponent(GetProject()->GetObject(id), component);
}

void EditorSys::OptDeleteComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component)
{
    object->DelComponent(component);
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteComponent, std::make_tuple(object, component));
}

void EditorSys::OptDeleteComponent(uint id, const SharePtr<Component>& component)
{
    OptDeleteComponent(GetProject()->GetObject(id), component);
}

void EditorSys::OptNewProject(const std::string & url)
{
    OptFreeProject();
    _project.reset(new Project());
    _project->New(url);
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kOpenProject, nullptr);
}

void EditorSys::OptOpenProject(const std::string & url)
{
    OptFreeProject();
    _project.reset(new Project());
    _project->Load(url);
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kOpenProject, nullptr);
}

void EditorSys::OptSaveProject(const std::string & url)
{
    if (_project != nullptr)
    {
        _project->Save(url);
        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSaveProject, nullptr);
    }
}

void EditorSys::OptFreeProject()
{
    if (_project != nullptr)
    {
        while (!_selected.empty())
        {
            EditorSys::OptSelectObject(_selected.back(), false);
        }
        _project.reset();
        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kFreeProject, nullptr);
    }
}

void EditorSys::OpenDialogNewProject(const std::string & url)
{
}

void EditorSys::OpenDialogOpenProject(const std::string & url)
{
}

bool EditorSys::IsOpenProject()
{
    ASSERT_LOG(
        (_project == nullptr) || 
        (_project != nullptr && _project->IsOpen()), "");
    return _project != nullptr;
}

const UniquePtr<Project> & EditorSys::GetProject()
{
    return _project;
}

std::string EditorSys::ObjectName(const SharePtr<GLObject> & object)
{
    size_t i = 0;
    auto name = SFormat("object_{0}", i++);
    while (object->GetObject(name))
        name = SFormat("object_{0}", i++);
    return std::move(name);
}

std::string EditorSys::ObjectName(uint id)
{
    return ObjectName(GetProject()->GetObject(id));
}

bool EditorSys::ObjectName(const SharePtr<GLObject> & parent, const std::string & name)
{
    if (name.empty()) 
    {
        return false; 
    }
    return nullptr == parent->GetObject(name);
}

bool EditorSys::ObjectName(uint id, const std::string & name)
{
    return ObjectName(GetProject()->GetObject(id), name);
}

const std::vector<SharePtr<GLObject>> & EditorSys::GetSelectedObjects()
{
    return _selected;
}

