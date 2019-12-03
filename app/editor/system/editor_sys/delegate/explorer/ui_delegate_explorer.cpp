#include "ui_delegate_explorer.h"

bool UIDelegateExplorer::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject>& object)
{
    switch (e)
    {
    case UIEventEnum::kMouse:
        return OnEventMouse((const UIEvent::Mouse &)param);
    case UIEventEnum::kMenu:
        return OnEventMenu((const UIEvent::Menu &)param);
    case UIEventEnum::kInit:
        return OnEventInit((const UIEvent::Init &)param);
    }
    return false;
}

bool UIDelegateExplorer::OnEventMouse(const UIEvent::Mouse & param)
{ 
    return true;
}

bool UIDelegateExplorer::OnEventMenu(const UIEvent::Menu & param)
{
    if      (param.mPath == "Rename")
    {
        ResRename(param.mObject, param.mEdit);
    }
    else if (param.mPath == "Delete")
    {
        ResDelete(param.mObject);
    }
    return true;
}

bool UIDelegateExplorer::OnEventInit(const UIEvent::Init & param)
{
    mListLayout = CastPtr<UIObjectLayout>(GetOwner()->GetObject({ "LayoutList" }));
    mTypeLayout = CastPtr<UIObjectLayout>(GetOwner()->GetObject({ "LayoutType" }));
    mRefsLayout = CastPtr<UIObjectLayout>(GetOwner()->GetObject({ "LayoutRefs" }));
    mSearchText = CastPtr<UIObjectTextBox>(GetOwner()->GetObject({ "LayoutSearch", "Input" }));

    auto init = std::any_cast<const InitParam_t &>(param);
    mPreSearch = std::get<0>(init);
    mOptSelect = std::get<1>(init);
    ListRefresh();
    return true;
}

void UIDelegateExplorer::ListClick1(const SharePtr<UIObject> & object)
{ 
    if (mLastSelect != nullptr)
    {
        mLastSelect->GetState()->IsSelect = false;
    }
    mLastSelect = object;
    if (mLastSelect != nullptr)
    {
        mLastSelect->GetState()->IsSelect = true;
        TypeRefresh();
        RefsRefresh();
    }
}

void UIDelegateExplorer::TypeRefresh()
{
    const char * TYPE_LIST[] = { "NULL", "TXT", "IMG", "MAP", "FNT", "OBJ", "VAR", "BLUE_PRINT" };

    mTypeLayout->ClearObjects();
    for (auto i = 0; i != Res::TypeEnum::Length; ++i)
    {
        auto type = mmc::Json::Hash();
        type->Insert(mmc::Json::List(), "__Children");
        type->Insert(mmc::Json::Hash(), "__Property");
        type->Insert(mmc::Json::FromValue("3"),          "__Property", "Type");
        type->Insert(mmc::Json::FromValue(TYPE_LIST[i]), "__Property", "Name");
        type->Insert(mmc::Json::FromValue("false"),      "__Property", "IsCanDragMove");
        type->Insert(mmc::Json::FromValue("false"),      "__Property", "IsCanDragFree");
        mTypeLayout->InsertObject(UIParser::Parse(type));
    }
}

void UIDelegateExplorer::RefsRefresh()
{
    mRefsLayout->ClearObjects();
}

void UIDelegateExplorer::ResSetType(const SharePtr<UIObject>& object, const Res::TypeEnum type)
{ 
    Global::Ref().mEditorSys->OptSetResType(mObj2Res.at(object), type);
}

void UIDelegateExplorer::ResRename(const SharePtr<UIObject> & object, const std::string & name)
{ 
    Global::Ref().mEditorSys->OptRenameRes(mObj2Res.at(object), name);
}

void UIDelegateExplorer::ResDelete(const SharePtr<UIObject>& object)
{ 
    Global::Ref().mEditorSys->OptDeleteRes(mObj2Res.at(object));
}

void UIDelegateExplorer::RefReference(const SharePtr<UIObject>& object)
{
    mOptSelect(mObj2Res.at(object)->AppendRef());
}
