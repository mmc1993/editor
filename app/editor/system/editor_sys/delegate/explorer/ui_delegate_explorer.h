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
    void TypeRefresh();
    void RefsRefresh();

    //  资源操作
    void ResSetType(const SharePtr<UIObject> & object, const Res::TypeEnum type);
    void ResRename(const SharePtr<UIObject> & object, const std::string & name);
    void ResDelete(const SharePtr<UIObject> & object);
    void RefReference(const SharePtr<UIObject> & object);

    void NewRecord();

private:
    std::map<Res *, SharePtr<UIObject>> mRes2Obj;
    std::map<SharePtr<UIObject>, Res *> mObj2Res;
    //  控件
    SharePtr<UIObjectLayout> mListBox;
    SharePtr<UIObjectLayout> mTypeBox;
    SharePtr<UIObjectLayout> mRefsBox;
    SharePtr<UIObjectTextBox> mSearchBox;
    //  参数
    std::string                     mPreSearch;
    std::function<void(Res::Ref *)> mOptSelect;

    //  上一个被选中的节点
    SharePtr<UIObject> mLastSelect;
};
