#pragma once

#include "include.h"

class UIEvent {
public:
    //  返回按下的状态键
    static int CheckStateKey();

    struct Event {
        mutable SharePtr<UIObject> mObject;

        Event(const SharePtr<UIObject> & object = nullptr) : mObject(object) { }
    };

    //  键盘事件
    struct Key : Event {
        //  支持的热键响应
        static std::vector<int> Hotkeys;

        int mKey;
        int mAct;   //  0, 1, 2 => 按下, 抬起, 单击
        int mState; //  1, 2, 4 => alt, ctrl, shift

        Key(const int act, const int key, const SharePtr<UIObject> & object = nullptr)
            : Event(object)
            , mKey(key)
            , mAct(act)
            , mState(CheckStateKey())
        { }
    };

    //  鼠标事件
    struct Mouse : Event {
        int mKey;           //  0, 1, 2       => 左键, 右键, 中键
        int mAct;           //  0, 1, 2, 3, 4 => 悬浮, 按下, 抬起, 单击, 双击
        int mState;         //  1, 2, 4       => alt, ctrl, shift
        int mWheel;         //  滚轮滚动
        glm::vec2 mDelta;   //  坐标偏移
        glm::vec2 mMouse;   //  鼠标坐标
        glm::vec2 mFirst;   //  起始坐标

        Mouse(const int act, const int key, const SharePtr<UIObject> & object = nullptr)
            : Event(object)
            , mAct(act)
            , mKey(key)
            , mState(CheckStateKey())
        {
            mMouse.x = ImGui::GetMousePos().x;
            mMouse.y = ImGui::GetMousePos().y;
            mDelta = ImGui::GetIO().MouseDelta;
            mWheel = (int)ImGui::GetIO().MouseWheel;
            mFirst = ImGui::GetIO().MouseDownPoints[mKey];
        }
    };

    //  菜单事件
    struct Menu : Event {
        bool mSelect;
        std::string mEdit;
        std::string mPath;

        Menu(const std::string & path, const bool select)
            : mPath(path)
            , mSelect(select)
        { }

        Menu(const std::string & path, const char * edit)
            : mPath(path)
            , mEdit(edit)
            , mSelect(false)
        { }

        Menu(const std::string & path, const std::string & edit)
            : mPath(path)
            , mEdit(edit)
            , mSelect(false)
        { }
    };

    //  拖放事件
    struct Drag : Event {
        DirectEnum mDirect;
        SharePtr<UIObject> mDragObj;
        SharePtr<UIObject> mFreeObj;
        glm::vec2 mBegWorld;
        glm::vec2 mEndWorld;
        int mAct;               //  0 锁定, 1 拖动, 2 释放

        Drag(int act, const glm::vec2 & beg, 
            const SharePtr<UIObject> & dragObj = nullptr,
            const SharePtr<UIObject> & freeObj = nullptr,
            const DirectEnum direct = DirectEnum::kNone) 
            : mAct(act)
            , mBegWorld(beg)
            , mEndWorld(beg)
            , mDragObj(dragObj)
            , mFreeObj(freeObj)
            , mDirect(direct)
        {
            if (mAct == 1 || mAct == 2)
            {
                mEndWorld = ImGui::GetMousePos();
            }
        }
    };

    //  编辑文本事件
    struct Edit : Event {
        std::string mString;

        Edit(const std::string & string, const SharePtr<UIObject> & object = nullptr)
            : Event(object)
            , mString(string)
        { }
    };

    //  初始化事件
    struct Init : Event {
        std::any mParam;

        Init(const std::any & param) : mParam(param)
        {  }
    };

    //  事件代理
    class DelegateHandler {
    public:
        virtual bool OnCallEventMessage(UIEventEnum type, const Event & event) = 0;

        SharePtr<UIObject> GetOwner()
        {
            ASSERT_LOG(!mOwner.expired(), "");
            return mOwner.lock();
        }

        void SetOwner(const SharePtr<UIObject> & owner)
        {
            mOwner = owner;
        }

        virtual ~DelegateHandler() { }
    private:
        WeakPtr<UIObject> mOwner;
    };
};