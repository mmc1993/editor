#include "editor_sys.h"
#include "project.h"
#include "component/gl_object.h"
#include "../ui_sys/ui_state/ui_state.h"
#include "../ui_sys/ui_object/ui_object.h"
#include "../event_sys/event_sys.h"

//  选择文件夹对话框
#include<Shlobj.h>
#include<Commdlg.h>
#pragma comment(lib,"Shell32.lib")

EditorSys::EditorSys(): _project(nullptr)
{ }

void EditorSys::OptSelectObject(UIObject * uiObject, bool select, bool multi)
{
    auto iter = std::find(_selected.begin(), _selected.end(), uiObject);
    auto has = iter != _selected.end();
    if (select)
    {
        if (!multi)
        {
            while (!_selected.empty())
            {
                if (_selected.back() != uiObject)
                {
                    auto selected = _selected.back();
                    _selected.pop_back();

                    Global::Ref().mEventSys->Post(
                        EventSys::TypeEnum::kSelectObject, std::make_tuple(selected,
                            (GLObject *)uiObject->GetState()->Pointer, false, multi));
                }
            }
        }
        if (!has)
        {
            _selected.push_back(uiObject);
            Global::Ref().mEventSys->Post(
                EventSys::TypeEnum::kSelectObject, std::make_tuple(uiObject, 
                    (GLObject *)uiObject->GetState()->Pointer, true, multi));
        }
    }
    else
    {
        if (has)
        {
            _selected.erase(iter);
            Global::Ref().mEventSys->Post(
                EventSys::TypeEnum::kSelectObject, std::make_tuple(uiObject,
                    (GLObject *)uiObject->GetState()->Pointer, false, multi));
        }
    }
}

void EditorSys::OptDeleteObject(UIObject * uiObject)
{
    //  取消选中
    OptSelectObject(uiObject, false);
    
    auto glObject = (GLObject *)uiObject->GetState()->Pointer;
    uiObject->DelThis();
    glObject->DelThis();

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteObject, std::make_tuple(uiObject, glObject));
}

void EditorSys::OptInsertObject(UIObject * uiObject, UIObject * insUIObject)
{
    auto glObject = (GLObject *)uiObject->GetState()->Pointer;
    auto insGLObject = (GLObject *)insUIObject->GetState()->Pointer;

    insUIObject->AddObject(uiObject);
    insGLObject->AddObject(glObject, glObject->GetName());

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kInsertObject, std::make_tuple(insUIObject, insGLObject, uiObject, glObject));
}

void EditorSys::OptRenameObject(UIObject * uiObject, const std::string & name)
{
    auto glObject = (GLObject *)uiObject->GetState()->Pointer;
    if (ObjectName(uiObject->GetParent(), name))
    {
        uiObject->GetState()->Name = name;
        glObject->SetName(name);

        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kRenameObject, std::make_tuple(uiObject, glObject, name));
    }
}

void EditorSys::OptAppendComponent(UIObject * uiObject, Component * component)
{
    auto glObject = (GLObject *)uiObject->GetState()->Pointer;
    glObject->AddComponent(component);

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kAppendComponent, std::make_tuple(uiObject, glObject, component));
}

void EditorSys::OptDeleteComponent(UIObject * uiObject, Component * component)
{
    auto glObject = (GLObject *)uiObject->GetState()->Pointer;
    glObject->DelComponent(component);

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteComponent, std::make_tuple(uiObject, glObject, component));
}

void EditorSys::OptNewProject(const std::string & url)
{
    OptFreeProject();
    SAFE_DELETE(_project);
    _project = new Project();
    _project->New(url);

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kOpenProject, nullptr);
}

bool EditorSys::OptOpenProject(const std::string & url)
{
    OptFreeProject();
    SAFE_DELETE(_project);
    _project = new Project();
    auto ret = _project->Load(url);
    if (ret)
    {
        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kOpenProject, nullptr);
    }
    return ret;
}

bool EditorSys::OptSaveProject(const std::string & url)
{
    if (_project != nullptr)
    {
        _project->Save(url);
        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSaveProject, nullptr);
    }
    return true;
}

void EditorSys::OptFreeProject()
{
    if (_project != nullptr)
    {
        while (!_selected.empty())
        {
            EditorSys::OptSelectObject(_selected.back(), false);
        }
        SAFE_DELETE(_project);
        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kFreeProject, nullptr);
    }
}

void EditorSys::OpenDialogNewProject(const std::string & url)
{
}

bool EditorSys::OpenDialogOpenProject(const std::string & url)
{
    return false;
}

bool EditorSys::IsOpenProject() const
{
    ASSERT_LOG(
        (_project == nullptr) || 
        (_project != nullptr && _project->IsOpen()), "");
    return _project != nullptr;
}

std::string EditorSys::ObjectName(GLObject * object) const
{
    size_t i = 0;
    auto name = SFormat("object_{0}", i++);
    while (object->GetObject(name))
        name = SFormat("object_{0}", i++);
    return std::move(name);
}

std::string EditorSys::ObjectName(UIObject * parent) const
{
    return ObjectName((GLObject *)parent->GetState()->Pointer);
}

bool EditorSys::ObjectName(UIObject * parent, const std::string & name) const
{
    if (name.empty()) { return false; }
    auto object = parent->GetState()->Pointer;
    return ((GLObject *)object)->GetObject(name) == nullptr;
}

const std::vector<UIObject*> & EditorSys::GetSelectedObjects() const
{
    return _selected;
}

Project * EditorSys::GetProject()
{
    return _project;
}
