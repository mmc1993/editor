#pragma once

#include "../ui_delegate_base.h"

//  资源列表格式:
//      Type|Path [> Word0 Word1]

//  搜索关键字格式:
//      Type0 Type1|Word0 Word1

class UIDelegateExplorer : public UIDelegateBase {
public:
    static const glm::vec4 sTypeColors[Res::Length];
    using SelectFn_t = std::function<void (Res::Ref)>;
    using InitArgs_t = std::tuple<std::string, SelectFn_t>;

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
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param) override;
    bool OnEventMouse(const UIEvent::Mouse & param);
    bool OnEventEdit(const UIEvent::Edit & param);
    bool OnEventMenu(const UIEvent::Menu & param);
    bool OnEventInit(const UIEvent::Init & param);

    void OnEvent(EventSys::TypeEnum type, const std::any & param);

    //  界面操作
    void ListRefresh();
    void ListClick1(const SharePtr<UIObject> & object);
    void ListClick2(const SharePtr<UIObject> & object);
    void ListRClick(const SharePtr<UIObject> & object);
    void ResSetType(const SharePtr<UIObject> & object);

    void NewRecord(const SearchItem & item);
    void NewSearch(const SearchStat & search);
    void NewSearch(const std::string & search);

    //  生成条目字符串/颜色
    std::tuple<std::string, glm::vec4> GenItemPathAndColor(Res * res);

private:
    //  控件
    SharePtr<UIObjectLayout> mListLayout;
    SharePtr<UIObjectLayout> mTypeLayout;
    SharePtr<UIObjectLayout> mRefsLayout;
    SharePtr<UIObjectTextBox> mSearchText;
    //  参数
    SelectFn_t               mOptSelect;
    std::string              mPreSearch;
    std::vector<std::string> mLimitType;
    //  内部状态
    std::vector<SearchItem> mSearchItems;       //  搜索结果
    SearchStat              mSearchStat;        //  搜索状态
    SharePtr<UIObject>      mLastSelect;        //  最后选中
    Project *               mProject;           //  当前项目

    std::map<Res *, SharePtr<UIObject>> mRes2Obj;
    std::map<SharePtr<UIObject>, Res *> mObj2Res;
};
