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
    mListBox = CastPtr<UIObjectLayout>(GetOwner()->GetObject({ "LayoutList" }));
    mTypeBox = CastPtr<UIObjectLayout>(GetOwner()->GetObject({ "LayoutType" }));
    mRefsBox = CastPtr<UIObjectLayout>(GetOwner()->GetObject({ "LayoutRefs" }));
    mSearchBox = CastPtr<UIObjectTextBox>(GetOwner()->GetObject({ "LayoutSearch", "Input" }));

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
    //for (auto i = 0; i != Res::TypeEnum::Length;)
    
}

void UIDelegateExplorer::RefsRefresh()
{

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
