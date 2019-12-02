#include "editor_sys.h"
#include "component/gl_object.h"
#include "../event_sys/event_sys.h"

//  选择文件夹对话框
#include<Shlobj.h>
#include<Commdlg.h>
#pragma comment(lib,"Shell32.lib")

void EditorSys::OptInsertObject(SharePtr<GLObject> object, SharePtr<GLObject> parent)
{
    parent->InsertObject(object, object->GetName());
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kInsertObject, std::make_tuple(object));
}

void EditorSys::OptSelectObject(SharePtr<GLObject> object, bool select, bool multi)
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

void EditorSys::OptSelectObject(uint objectID, bool select, bool multi)
{
    OptSelectObject(GetProject()->GetObject(objectID), select, multi);
}

void EditorSys::OptDeleteObject(SharePtr<GLObject> object)
{
    OptSelectObject(object, false); GetProject()->DeleteObject(object); object->DeleteThis();
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteObject, std::make_tuple(object));
}

void EditorSys::OptDeleteObject(uint objectID)
{
    OptDeleteObject(GetProject()->GetObject(objectID));
}

void EditorSys::OptStateAddObject(SharePtr<GLObject> object, uint state)
{
    OptStateObject(object, object->HasState(~0u) |  state);
}

void EditorSys::OptStateAddObject(uint objectID, uint state)
{
    OptStateAddObject(GetProject()->GetObject(objectID), state);
}

void EditorSys::OptStateSubObject(SharePtr<GLObject> object, uint state)
{
    OptStateObject(object, object->HasState(~0u) & ~state);
}

void EditorSys::OptStateSubObject(uint objectID, uint state)
{
    OptStateSubObject(GetProject()->GetObject(objectID), state);
}

void EditorSys::OptStateObject(SharePtr<GLObject> object, uint state)
{
    auto old = object->HasState(~0u);
    object->AddState(~0u,  false);
    object->AddState(state, true);
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kStateObject, std::make_tuple(object, old, state));
}

void EditorSys::OptStateObject(uint objectID, uint state)
{
    OptStateObject(GetProject()->GetObject(objectID), state);
}

void EditorSys::OptMoveObject(SharePtr<GLObject> object, SharePtr<GLObject> target, int pos)
{
    if (object == target || target->HasParent(object))
    {
        return;
    }

    switch (pos)
    {
    case 0: //  里面
        {
            if (!target->GetObject(object->GetName()))
            {
                object->SetParent(target);
                
                Global::Ref().mEventSys->Post(EventSys::TypeEnum::kMoveObject, 
                    std::make_tuple(object, 
                        object->GetParent(), 
                        object->GetParent()->GetObjects().size() - 1));
            }
        }
        break;
    case 1: //  前面
        {
            auto & objects = target->GetParent()->GetObjects();
            if (object->GetParent() == target->GetParent())
            {
                auto it0 = std::find(objects.begin(), objects.end(), object);
                auto it1 = std::find(objects.begin(), objects.end(), target);
                if (std::distance(it0, it1) == 1) { break; }
            }
            else if (target->GetParent()->GetObject(object->GetName()))
            {
                break;
            }
            object->SetParent(target->GetParent());
            auto iter = std::find(objects.begin(), objects.end(), target);
            std::move_backward(iter, std::prev(objects.end(), 1), objects.end());
            *iter = object;

            Global::Ref().mEventSys->Post(EventSys::TypeEnum::kMoveObject, 
                std::make_tuple(object, object->GetParent(), 
                (uint)std::distance(objects.begin(), iter)));
        }
        break;
    case 2: //  后面
        {
            auto & objects = target->GetParent()->GetObjects();
            if (object->GetParent() == target->GetParent())
            {
                auto it0 = std::find(objects.begin(), objects.end(), object);
                auto it1 = std::find(objects.begin(), objects.end(), target);
                if (std::distance(it1, it0) == 1) { break; }
            }
            else if (target->GetParent()->GetObject(object->GetName()))
            {
                break;
            }
            object->SetParent(target->GetParent());
            auto iter = std::find(objects.begin(), objects.end(), target) + 1;
            std::move_backward(iter, std::prev(objects.end(), 1), objects.end());
            *iter = object;

            Global::Ref().mEventSys->Post(EventSys::TypeEnum::kMoveObject, 
                std::make_tuple(object, object->GetParent(), 
                (uint)std::distance(objects.begin(), iter)));
        }
        break;
    }
}

void EditorSys::OptMoveObject(uint objectID, SharePtr<GLObject> target, int pos)
{
    OptMoveObject(Global::Ref().mEditorSys->GetProject()->GetObject(objectID), target, pos);
}

void EditorSys::OptMoveObject(SharePtr<GLObject> object, uint targetID, int pos)
{
    OptMoveObject(object, Global::Ref().mEditorSys->GetProject()->GetObject(targetID), pos);
}

void EditorSys::OptMoveObject(uint objectID, uint targetID, int pos)
{
    OptMoveObject(Global::Ref().mEditorSys->GetProject()->GetObject(objectID),
                  Global::Ref().mEditorSys->GetProject()->GetObject(targetID), pos);
}

void EditorSys::OptRenameObject(SharePtr<GLObject> object, const std::string & name)
{
    if (ObjectName(object->GetParent(), name))
    {
        auto old = object->SetName(name);
        auto arg = std::make_tuple(object, old);
        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kRenameObject, arg);
    }
}

void EditorSys::OptRenameObject(uint objectID, const std::string & name)
{
    OptRenameObject(GetProject()->GetObject(objectID), name);
}

void EditorSys::OptAppendComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component)
{
    object->AddComponent(component);
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kAppendComponent, std::make_tuple(object, component));
}

void EditorSys::OptAppendComponent(uint objectID, const SharePtr<Component> & component)
{
    OptAppendComponent(GetProject()->GetObject(objectID), component);
}

void EditorSys::OptDeleteComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component)
{
    object->DelComponent(component);
    Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteComponent, std::make_tuple(object, component));
}

void EditorSys::OptDeleteComponent(uint objectID, const SharePtr<Component>& component)
{
    OptDeleteComponent(GetProject()->GetObject(objectID), component);
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

void EditorSys::OptDeleteRes(uint id)
{
    //Global::Ref().mResSys->OptDeleteRes(id);
}

void EditorSys::OptDeleteRes(Res * res)
{
    //Global::Ref().mResSys->OptDeleteRes(res);
}

void EditorSys::OptModifyRes(uint id, const std::string & url)
{ 
    //Global::Ref().mResSys->OptModifyRes(id, url);
}

void EditorSys::OptModifyRes(Res * res, const std::string & url)
{
    //Global::Ref().mResSys->OptModifyRes(res, url);
}

void EditorSys::OptSetResType(uint id, uint type)
{ 
    //Global::Ref().mResSys->OptSetResType(id, type);
}

void EditorSys::OptSetResType(Res * res, uint type)
{
    //Global::Ref().mResSys->OptSetResType(res, type);
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

