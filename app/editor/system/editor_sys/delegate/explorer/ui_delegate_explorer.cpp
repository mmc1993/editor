#include "ui_delegate_explorer.h"

const glm::vec4 UIDelegateExplorer::sTypeColors[Res::Length]
{
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
    glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
    glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
};

bool UIDelegateExplorer::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject>& object)
{
    auto ret = UIDelegateBase::OnCallEventMessage(e, param, object);
    switch (e)
    {
    case UIEventEnum::kMouse:
        return OnEventMouse((const UIEvent::Mouse &)param);
    case UIEventEnum::kEdit:
        return OnEventEdit((const UIEvent::Edit &)param);
    case UIEventEnum::kMenu:
        return OnEventMenu((const UIEvent::Menu &)param);
    case UIEventEnum::kInit:
        return OnEventInit((const UIEvent::Init &)param);
    }
    return ret;
}

bool UIDelegateExplorer::OnEventMouse(const UIEvent::Mouse & param)
{
    if (param.mAct == 3)
    {
        if (param.mKey == 0)
        {
            if (mObj2Res.end() != mObj2Res.find(param.mObject))
            {
                //  单击条目
                ListClick1(param.mObject);
            }
            else if (param.mObject->GetParent() == mTypeLayout)
            {
                //  单击类型
                ResSetType(param.mObject);
            }
        }
        else if (param.mKey == 1)
        {
            if (mObj2Res.end() != mObj2Res.find(param.mObject))
            {
                //  右键条目
                ListClick1(param.mObject);
                ListRClick(param.mObject);
            }
        }
    }
    else if (param.mAct == 4 && param.mKey == 0)
    {
        if (mObj2Res.end() != mObj2Res.find(param.mObject))
        {
            //  双击条目
            ListClick2(param.mObject);
        }
    }
    return true;
}

bool UIDelegateExplorer::OnEventEdit(const UIEvent::Edit & param)
{
    NewSearch(std::upper(param.mString));
    return true;
}

bool UIDelegateExplorer::OnEventMenu(const UIEvent::Menu & param)
{
    if      (tools::IsEqualSkipSpace(param.mPath, "Rename"))
    {
        Global::Ref().mEditorSys->OptRenameRes(mObj2Res.at(param.mObject), param.mEdit);
    }
    else if (tools::IsEqualSkipSpace(param.mPath, "Delete"))
    {
        Global::Ref().mEditorSys->OptDeleteRes(mObj2Res.at(param.mObject));
    }
    return true;
}

bool UIDelegateExplorer::OnEventInit(const UIEvent::Init & param)
{
    //  注册监听
    _listener.Add(EventSys::TypeEnum::kSetResType, std::bind(&UIDelegateExplorer::OnEvent, this, std::placeholders::_1, std::placeholders::_2), Global::Ref().mEventSys);
    _listener.Add(EventSys::TypeEnum::kRenameRes, std::bind(&UIDelegateExplorer::OnEvent, this, std::placeholders::_1, std::placeholders::_2), Global::Ref().mEventSys);
    _listener.Add(EventSys::TypeEnum::kDeleteRes, std::bind(&UIDelegateExplorer::OnEvent, this, std::placeholders::_1, std::placeholders::_2), Global::Ref().mEventSys);

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
    mSearchText->SetText(mPreSearch);
    mLimitType = mSearchStat.mTypes;        //  限制搜索类型
    return true;
}

void UIDelegateExplorer::OnEvent(EventSys::TypeEnum type, const std::any & param)
{ 
    switch (type)
    {
    case EventSys::TypeEnum::kSetResType:
        {
            auto & args = std::any_cast<const std::tuple<Res *, uint> &>(param);
            auto & arg0 = std::get<0>(args);
            auto & arg1 = std::get<1>(args);
            ListClick1(mRes2Obj.at(arg0));

            const auto & [path, color] = GenItemPathAndColor(arg0);
            mRes2Obj.at(arg0)->GetState()->Color = color;
            mRes2Obj.at(arg0)->GetState()->Name = path;
        }
        break;
    case EventSys::TypeEnum::kRenameRes:
        {
            auto & args = std::any_cast<const std::tuple<Res *, std::string, std::string> &>(param);
            auto & arg0 = std::get<0>(args);
            auto & arg1 = std::get<1>(args);
            auto & arg2 = std::get<2>(args);

            const auto & [path, color] = GenItemPathAndColor(arg0);
            mRes2Obj.at(arg0)->GetState()->Color = color;
            mRes2Obj.at(arg0)->GetState()->Name = path;
        }
        break;
    case EventSys::TypeEnum::kDeleteRes:
        {
            auto & args = std::any_cast<const std::tuple<Res *, uint, std::string> &>(param);
            auto & arg0 = std::get<0>(args);
            auto & arg1 = std::get<1>(args);
            auto & arg2 = std::get<2>(args);
            ListClick1(nullptr);

            mRes2Obj.at(arg0)->GetParent()->DeleteThis();
        }
        break;
    }
}

void UIDelegateExplorer::ListRefresh()
{
    ListClick1(nullptr);
    mRes2Obj.clear();
    mObj2Res.clear();
    mListLayout->ClearObjects();
    for (auto & item : mSearchItems)
    {
        NewRecord(item);
    }
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
        auto resource = mObj2Res.at(mLastSelect);
        for (auto i = 0; i != Res::TypeEnum::Length; ++i)
        {
            auto color = resource->Type() == (Res::TypeEnum)i ? "1 0 0 1" : "1 1 1 1";
            auto type = mmc::Json::Hash();
            auto name = Res::TypeString((Res::TypeEnum)i);
            type->Insert(mmc::Json::List(), "__Children");
            type->Insert(mmc::Json::Hash(), "__Property");
            type->Insert(mmc::Json::FromValue("3"), "__Property", "Type");
            type->Insert(mmc::Json::FromValue(name), "__Property", "Name");
            type->Insert(mmc::Json::FromValue(color), "__Property", "Color");
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
    if (res->Type() == Res::TypeEnum::kNull ||
        res->Type() == Res::TypeEnum::kImg ||
        res->Type() == Res::TypeEnum::kTxt ||
        res->Type() == Res::TypeEnum::kFnt ||
        res->Type() == Res::TypeEnum::kMap)
    {
        std::vector<std::string> list;
        list.emplace_back("Delete"  );
        list.emplace_back(SFormat("Rename\\{0}~", mObj2Res.at(object)->Path()));
        UIMenu::PopMenu(object, list);
    }
}

void UIDelegateExplorer::ResSetType(const SharePtr<UIObject> & object)
{
    ASSERT_LOG(mLastSelect != nullptr, "");
    auto iter = std::find(
        object->GetParent()->GetObjects().begin(), 
        object->GetParent()->GetObjects().end(), object);
    auto idx = std::distance(object->GetParent()->GetObjects().begin(), iter);
    Global::Ref().mEditorSys->OptSetResType(mObj2Res.at(mLastSelect), idx);
}

void UIDelegateExplorer::NewRecord(const SearchItem & item)
{
    const auto & color = SFormat("{0} {1} {2} {3}", 
                         sTypeColors[item.mType].r,
                         sTypeColors[item.mType].g,
                         sTypeColors[item.mType].b,
                         sTypeColors[item.mType].a);
    const auto & type  = item.mRes->TypeString();
    const auto & path  = item.mRes->Path();

    auto layout = mmc::Json::Hash();
    layout->Insert(mmc::Json::List(), "__Children");
    layout->Insert(mmc::Json::Hash(), "__Property");
    layout->Insert(mmc::Json::FromValue("0"), "__Property", "Type");
    layout->Insert(mmc::Json::FromValue("0"), "__Property", "Name");
    layout->Insert(mmc::Json::FromValue("true"), "__Property", "IsSameline");

    //  颜色, 类型, 路径
    auto text = type + "> " + path + " ";
    auto line = mmc::Json::Hash();
    line->Insert(mmc::Json::List(), "__Children");
    line->Insert(mmc::Json::Hash(), "__Property");
    line->Insert(mmc::Json::FromValue("2"), "__Property", "Type");
    line->Insert(mmc::Json::FromValue(text), "__Property", "Name");
    line->Insert(mmc::Json::FromValue(color), "__Property", "Color");
    layout->Insert(line, "__Children", 0);

    //  颜色, 关键字
    if (!item.mWords.empty())
    {
        text.clear();
        for (auto i = 0; i != item.mWords.size(); ++i)
        {
            if (item.mWords.at(i) != std::string::npos)
            {
                text.append(mSearchStat.mWords.at(i));
                text.append(" ");
            }
        }
        if (!text.empty())
        {
            const auto & color = "0.5, 0.5, 0.5, 1";
            auto word = mmc::Json::Hash();
            word->Insert(mmc::Json::List(), "__Children");
            word->Insert(mmc::Json::Hash(), "__Property");
            word->Insert(mmc::Json::FromValue("3"), "__Property", "Type");
            word->Insert(mmc::Json::FromValue(text), "__Property", "Name");
            word->Insert(mmc::Json::FromValue(color), "__Property", "Color");
            layout->Insert(word, "__Children", 1);
        }
    }

    mListLayout->InsertObject(UIParser::Parse(layout));
    mRes2Obj.emplace(item.mRes, mListLayout->GetObjects().back()->GetObjects().at(0));
    mObj2Res.emplace(mListLayout->GetObjects().back()->GetObjects().at(0), item.mRes);
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
        searchStat.mWords = tools::Split(txt.substr(pos +1), " ");
    }

    //  限制搜索类型
    if (!mLimitType.empty()) { searchStat.mTypes = mLimitType; }

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

std::tuple<std::string, glm::vec4> UIDelegateExplorer::GenItemPathAndColor(Res * res)
{
    std::tuple<std::string, glm::vec4> result;
    std::get<0>(result).append(res->TypeString());
    std::get<0>(result).append("> "             );
    std::get<0>(result).append(res->Path()      );
    std::get<1>(result) = sTypeColors[res->Type()];
    return std::move(result);
}

void UIDelegateExplorer::NewSearch(const SearchStat & search)
{ 
    mSearchStat = search;
    mSearchItems.clear();
    //  匹配类型
    std::vector<Res::TypeEnum> types;
    for (const auto & type : mSearchStat.mTypes)
    {
        auto i = 0;
        for (; i != Res::TypeEnum::Length &&
            Res::TypeString(i) != type; ++i);
        if (i != Res::TypeEnum::Length)
        {
            types.emplace_back((Res::TypeEnum)i);
        }
    }
    //  匹配关键字
    for (auto res : mProject->GetResByType(types))
    {
        SearchItem item(res, res->Type());
        if (mSearchStat.mWords.empty())
        {
            mSearchItems.emplace_back(item);
        }
        else
        {
            const auto & path = std::upper(res->Path());
            for (const auto & word : mSearchStat.mWords)
            {
                item.mWords.emplace_back(path.find(word));
            }
            if (!std::all_ofv(item.mWords.begin(),
                              item.mWords.end(), 
                              std::string::npos))
            {
                mSearchItems.emplace_back(item);
            }
        }
    }
    //  排序结果
    std::sort(mSearchItems.begin(), mSearchItems.end(), 
        [] (const SearchItem & a, const SearchItem & b)
        {
            if (a.mType != b.mType)
            {
                return a.mType < b.mType;
            }
            int n = 0;
            for (auto i = 0; i != a.mWords.size(); ++i)
            {
                if (a.mWords.at(i) != std::string::npos) { ++n; }
                if (b.mWords.at(i) != std::string::npos) { --n; }
            }
            return n > 0;
        });
    ListRefresh();
}
