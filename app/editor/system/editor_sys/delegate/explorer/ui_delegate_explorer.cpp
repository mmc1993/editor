#include "ui_delegate_explorer.h"

bool UIDelegateExplorer::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject>& object)
{
    auto ret = UIDelegateBase::OnCallEventMessage(e, param, object);
    switch (e)
    {
    case UIEventEnum::kMouse:
        return OnEventMouse((const UIEvent::Mouse &)param);
    case UIEventEnum::kMenu:
        return OnEventMenu((const UIEvent::Menu &)param);
    case UIEventEnum::kInit:
        return OnEventInit((const UIEvent::Init &)param);
    }
    return ret;
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
    mProject = Global::Ref().mEditorSys->GetProject().get();
    mProject->Retrieve();

    auto init = std::any_cast<const InitParam_t &>(param.mParam);
    mPreSearch = std::get<0>(init);
    mOptSelect = std::get<1>(init);
    NewSearch(std::upper(mPreSearch));
    return true;
}

void UIDelegateExplorer::ListRefresh()
{
    ListClick1(nullptr);
}

void UIDelegateExplorer::ListClick1(const SharePtr<UIObject> & object)
{ 
    if (mLastSelect != nullptr)
    {
        mLastSelect->GetState()->IsSelect = false;
    }
    mLastSelect = object;
    mTypeLayout->ClearObjects();
    if (mLastSelect != nullptr)
    {
        mLastSelect->GetState()->IsSelect = true;
        for (auto i = 0; i != Res::TypeEnum::Length; ++i)
        {
            auto type = mmc::Json::Hash();
            auto name = Res::TypeString((Res::TypeEnum)i);
            type->Insert(mmc::Json::List(), "__Children");
            type->Insert(mmc::Json::Hash(), "__Property");
            type->Insert(mmc::Json::FromValue("3"), "__Property", "Type");
            type->Insert(mmc::Json::FromValue(name), "__Property", "Name");
            type->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragMove");
            type->Insert(mmc::Json::FromValue("false"), "__Property", "IsCanDragFree");
            mTypeLayout->InsertObject(UIParser::Parse(type));
        }
    }
}

void UIDelegateExplorer::ListClick2(const SharePtr<UIObject>& object)
{
    if (mOptSelect)
    {
        mOptSelect(mObj2Res.at(object)->AppendRef());
        Global::Ref().mUISys->FreeWindow(GetOwner());
    }
}

void UIDelegateExplorer::ListRClick(const SharePtr<UIObject>& object)
{
    auto res = mObj2Res.at(object);
    std::vector<std::string> list;
    list.emplace_back("Delete");
    if (res->Type() == Res::TypeEnum::kImg ||
        res->Type() == Res::TypeEnum::kTxt ||
        res->Type() == Res::TypeEnum::kFnt ||
        res->Type() == Res::TypeEnum::kMap)
    {
        list.emplace_back(SFormat("Rename/{0}~", mObj2Res.at(object)->Path()));
    }
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

void UIDelegateExplorer::NewRecord(Res * res)
{ 

}

void UIDelegateExplorer::NewSearch(const std::string & search)
{
    SearchStat searchStat;
    auto txt = tools::ReplaceEx(search, "\\s+", " ");
    auto pos = txt.find_first_of('|');
    if (pos == std::string::npos)
    {
        searchStat.mWords = tools::Split(txt, " ");
    }
    else
    {
        searchStat.mTypes = tools::Split(txt.substr(0, pos), " ");
        searchStat.mWords = tools::Split(txt.substr(   pos), " ");
    }

    auto ret = mSearchStat.mTypes.size() != searchStat.mTypes.size()
            || mSearchStat.mWords.size() != searchStat.mWords.size();
    for (auto i = 0; !ret && i != mSearchStat.mTypes.size(); ++i)
    {
        if (mSearchStat.mTypes.at(i) != searchStat.mTypes.at(i)) { ret = true; }
    }
    for (auto i = 0; !ret && i != mSearchStat.mWords.size(); ++i)
    {
        if (mSearchStat.mWords.at(i) != searchStat.mWords.at(i)) { ret = true; }
    }
    if (ret) { NewSearch(searchStat); }
}

void UIDelegateExplorer::NewSearch(const SearchStat & search)
{ 
    mSearchStat = search;
    mSearchItems.clear();
    //  ∆•≈‰¿‡–Õ
    std::vector<Res::TypeEnum> types;
    for (const auto & type : mSearchStat.mTypes)
    {
        for (auto i = 0; i != Res::TypeEnum::Length; ++i)
        {
            if (Res::TypeString((Res::TypeEnum)i) == type)
            {
                types.emplace_back((Res::TypeEnum)i);
            }
        }
    }
    //  ∆•≈‰πÿº¸◊÷
    for (auto res : mProject->GetResByType(types))
    {
        SearchItem item(res, res->Type());
        for (const auto & word:mSearchStat.mWords)
        {
            item.mWords.emplace_back(res->Path().find(word));
        }
        if (std::none_ofv(item.mWords.begin(), item.mWords.end(), std::string::npos))
        {
            mSearchItems.emplace_back(item);
        }
    }
    //  ≈≈–ÚΩ·π˚
    std::sort(mSearchItems.begin(), mSearchItems.end(), 
        [] (const SearchItem & a, const SearchItem & b)
        {
            if (a.mType != b.mType)
            {
                return a.mType < b.mType;
            }
            for (auto i = 0; i != a.mWords.size(); ++i)
            {
                if (a.mWords.at(i) < b.mWords.at(i))
                {
                    return true;
                }
            }
            return false;
        });
    for (auto & item : mSearchItems)
    {
        std::cout << "Path: " << item.mRes->Path() << std::endl;
    }
    ListRefresh();
}
