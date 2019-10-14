#include "editor_sys.h"
#include "component/gl_object.h"
#include "../ui_sys/ui_object/ui_object.h"
#include "../event_sys/event_sys.h"

//  选择文件夹对话框
#include<Shlobj.h>
#include<Commdlg.h>
#pragma comment(lib,"Shell32.lib")

void EditorSys::OptInsertObject(const SharePtr<UIObject> & uiObject, const SharePtr<UIObject> & parentUIObject)
{
    auto glObject       = std::any_cast<SharePtr<GLObject>>(uiObject      ->GetState()->UserData);
    auto parentGLObject = std::any_cast<SharePtr<GLObject>>(parentUIObject->GetState()->UserData);

    parentUIObject->AddObject(uiObject                     );
    parentGLObject->AddObject(glObject, glObject->GetName());

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kInsertObject, std::make_tuple(parentUIObject, parentGLObject, uiObject, glObject));
}

void EditorSys::OptSelectObject(const SharePtr<UIObject> & uiObject, bool select, bool multi)
{
    auto iter = std::find(_selected.begin(), _selected.end(), uiObject);
    auto has = iter != _selected.end();
    if (select)
    {
        if (!multi)
        {
            auto count = _selected.size();
            while (0 != count--)
            {
                auto selected = _selected.back();
                if (selected != uiObject)
                {
                    _selected.pop_back();
                    Global::Ref().mEventSys->Post(
                        EventSys::TypeEnum::kSelectObject, 
                        std::make_tuple(selected,
                            std::any_cast<SharePtr<GLObject>>(selected->GetState()->UserData), false, multi));
                }
            }
        }

        if (!has)
        {
            _selected.push_back(uiObject);
            Global::Ref().mEventSys->Post(
                EventSys::TypeEnum::kSelectObject, 
                std::make_tuple(uiObject,
                    std::any_cast<SharePtr<GLObject>>(uiObject->GetState()->UserData), true, multi));
        }
    }
    else
    {
        if (has)
        {
            _selected.erase(iter);
            Global::Ref().mEventSys->Post(
                EventSys::TypeEnum::kSelectObject,
                std::make_tuple(uiObject, 
                    std::any_cast<SharePtr<GLObject>>(uiObject->GetState()->UserData), false, multi));
        }
    }
}

void EditorSys::OptDeleteObject(const SharePtr<UIObject> & uiObject)
{
    //  取消选中
    OptSelectObject(uiObject, false);
    
    auto glObject = std::any_cast<SharePtr<GLObject>>(uiObject->GetState()->UserData);
    uiObject->DelThis();
    glObject->DelThis();

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteObject, std::make_tuple(uiObject, glObject));
}

void EditorSys::OptRenameObject(const SharePtr<UIObject> & uiObject, const std::string & name)
{
    if (ObjectName(uiObject->GetParent(), name))
    {
        auto glObject = std::any_cast<SharePtr<GLObject>>(uiObject->GetState()->UserData);
        uiObject->GetState()->Name = name;
        glObject->SetName(name);

        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kRenameObject, std::make_tuple(uiObject, glObject, name));
    }
}

void EditorSys::OptAppendComponent(const SharePtr<UIObject> & uiObject, const SharePtr<Component> & component)
{
    auto glObject = std::any_cast<SharePtr<GLObject>>(uiObject->GetState()->UserData);
    glObject->AddComponent(component);

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kAppendComponent, std::make_tuple(uiObject, glObject, component));
}

void EditorSys::OptDeleteComponent(const SharePtr<UIObject> & uiObject, const SharePtr<Component> & component)
{
    auto glObject = std::any_cast<SharePtr<GLObject>>(uiObject->GetState()->UserData);
    glObject->DelComponent(component);

    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteComponent, std::make_tuple(uiObject, glObject, component));
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

std::string EditorSys::ObjectName(const SharePtr<GLObject> & object) const
{
    size_t i = 0;
    auto name = SFormat("object_{0}", i++);
    while (object->GetObject(name))
        name = SFormat("object_{0}", i++);
    return std::move(name);
}

std::string EditorSys::ObjectName(const SharePtr<UIObject> & parent) const
{
    return ObjectName(std::any_cast<SharePtr<GLObject>>(parent->GetState()->UserData));
}

bool        EditorSys::ObjectName(const SharePtr<UIObject> & parent, const std::string & name) const
{
    if (name.empty()) 
    {
        return false; 
    }
    return nullptr == std::any_cast<SharePtr<GLObject>>(parent->GetState()->UserData)->GetObject(name);
}

const std::vector<SharePtr<UIObject>> & EditorSys::GetSelectedObjects() const
{
    return _selected;
}

const UniquePtr<Project> & EditorSys::GetProject()
{
    return _project;
}
