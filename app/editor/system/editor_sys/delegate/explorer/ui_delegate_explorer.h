#pragma once

#include "../ui_delegate_base.h"

//  资源列表格式:
//      Type|Path [> Word0 Word1]

//  搜索关键字格式:
//      Type0 Type1|Word0 Word1

class UIDelegateExplorer : public UIDelegateBase {
public:
    using SelectFunc_t  = std::function<void (Res::Ref *)>;
    using InitParam_t   = std::tuple<std::string, SelectFunc_t>;

    //  搜索信息
    struct SearchStat {
        std::vector<std::string> mTypes;
        std::vector<std::string> mWords;
    };

    //  搜索结果
    struct SearchItem {
        SearchItem(Res * res = nullptr, uint type = 0)
            : mRes(res), mType(type) { }
        Res * mRes;
        uint mType;
        std::vector<uint> mWords;
    };

private:
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject> & object) override;
    bool OnEventMouse(const UIEvent::Mouse & param);
    bool OnEventMenu(const UIEvent::Menu & param);
    bool OnEventInit(const UIEvent::Init & param);

    //  界面操作
    void ListRefresh();
    void ListClick1(const SharePtr<UIObject> & object);
    void ListClick2(const SharePtr<UIObject> & object);
    void ListRClick(const SharePtr<UIObject> & object);

    //  资源操作
    void ResSetType(const SharePtr<UIObject> & object, const Res::TypeEnum type);
    void ResRename(const SharePtr<UIObject> & object, const std::string & name);
    void ResDelete(const SharePtr<UIObject> & object);

    void NewRecord(Res * res);
    void NewSearch(const std::string & search);
    void NewSearch(const SearchStat  & search);

private:
    //  控件
    SharePtr<UIObjectLayout> mListLayout;
    SharePtr<UIObjectLayout> mTypeLayout;
    SharePtr<UIObjectLayout> mRefsLayout;
    SharePtr<UIObjectTextBox> mSearchText;
    //  参数
    std::string                     mPreSearch;
    std::vector<std::string>        mLimitType;
    std::function<void(Res::Ref *)> mOptSelect;
    //  内部状态
    std::vector<SearchItem> mSearchItems;       //  搜索结果
    SearchStat              mSearchStat;        //  搜索状态
    SharePtr<UIObject>      mLastSelect;        //  最后选中
    Project *               mProject;           //  当前项目

    std::map<Res *, SharePtr<UIObject>> mRes2Obj;
    std::map<SharePtr<UIObject>, Res *> mObj2Res;
};
