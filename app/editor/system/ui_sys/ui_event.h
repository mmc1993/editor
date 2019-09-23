#pragma once

#include "include.h"

class UIEvent {
public:
    //  返回按下的状态键
    static int CheckStateKey();

    struct Event {
        mutable UIObject * mObject;

        Event(UIObject * object = nullptr) : mObject(object)
        {  }
    };

    //  键盘事件
    struct Key : Event {
        //  支持的热键响应
        static std::vector<int> Hotkeys;

        int mKey;
        int mAct;   //  0, 1, 2 => 按下, 抬起, 单击
        int mState; //  1, 2, 4 => alt, ctrl, shift

        Key(const int act, const int key, UIObject * object = nullptr)
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
        glm::vec2 mMouse;   //  鼠标坐标

        Mouse(const int act, const int key, UIObject * object = nullptr)
            : Event(object)
            , mAct(act)
            , mKey(key)
            , mState(CheckStateKey())
        {
            mMouse.x = ImGui::GetMousePos().x;
            mMouse.y = ImGui::GetMousePos().y;
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
        { }

        Menu(const std::string & path, const std::string & edit)
            : mPath(path)
            , mEdit(edit)
        { }
    };

    //  编辑文本事件
    struct EditText : Event {
        std::string mString;

        EditText(const std::string & string, UIObject * object = nullptr)
            : Event(object)
            , mString(string)
        { }
    };

    //  事件代理
    struct Delegate : Event {
        int mType;      //  0 初始化, 1 销毁

        Delegate(int type) : mType(type)
        {  }
    };

    //  事件代理
    class DelegateHandler {
    public:
        virtual bool OnCallEventMessage(UIEventEnum type, const Event & event, UIObject * object)
        {
            if (type == UIEventEnum::kDELEGATE)
            {
                auto & delegate = (const Delegate &)event;
                if (delegate.mType == 0)
                {
                    _onwer = delegate.mObject;
                }
            }
            return false;
        }

        UIObject * GetOnwer()
        {
            return _onwer;
        }
    private:
        UIObject * _onwer;
    };
};