#pragma once

#include "../../base.h"

class EventSys {
public:
    //  事件枚举
    enum class TypeEnum {
        //  project
        kOpenProject,       //  {}
        kSaveProject,       //  {}
        kFreeProject,       //  {}

        //  gl object
        kSelectObject,    //  { GLOBject 选中的节点, bool 是否选中, bool 是否多选 }
        kDeleteObject,    //  { GLOBject 删除的节点 }
        kInsertObject,    //  { GLObject 新增的节点  }
        kRenameObject,    //  { GLOBject 选中的节点, string 旧名字 }
        kStateObject,     //  { GLOBject 选中的节点, uint 旧状态, uint 新状态 }
        kMoveObject,      //  { GLOBject 选中的节点, GLObject 父节点, uint 位置 }

        //  component
        kDeleteComponent,   //  { GLOBject 选中的节点, Component 选中的组件 }
        kAppendComponent,   //  { GLOBject 选中的节点, Component 新增的组件 }

        //  resource
        kSetResType,        //  { Res 选中的资源, string 路径, Res::TypeEnum 类型 }
        kModifyRes,         //  { Res 选中的资源, string 旧路径, string 新路径 }
        kDeleteRes,         //  { id 选中的资源, string 路径 }
    };

    using func_t = std::function<void(TypeEnum type, const std::any &)>;


    //  监听器
    class Listener {
    public:
        Listener(): _owner(nullptr)
        { }

        ~Listener()
        {
            ASSERT_LOG(_listens.empty() || _owner != nullptr, "");
            for (auto listen : _listens)
            {
                _owner->Del(listen);
            }
        }

        void Add(TypeEnum id, const func_t & func, EventSys * owner = nullptr)
        {
            if (owner != nullptr) { _owner = owner; }
            ASSERT_LOG(_owner != nullptr, "");
            _listens.push_back(_owner->Add(id, func));
        }

    private:
        EventSys * _owner;
        std::vector<size_t> _listens;
    };

    //  事件类型
    struct Event {
        size_t mID;
        func_t mFunc;
        Event(size_t id, const func_t & func): mID(id), mFunc(func)
        { }

        bool operator ==(size_t id) const { return mID == id; }
    };

private:
    static size_t s_coundID;

public:
    EventSys()
    { }

    ~EventSys()
    { }

    size_t Add(TypeEnum type, const func_t & func)
    {
        auto insert = _events.insert(std::make_pair(type, std::vector<Event>()));
        insert.first->second.emplace_back(EventSys::s_coundID, func);
        return EventSys::s_coundID++;
    }

    void Del(size_t id)
    {
        for (auto & events : _events)
        {
            auto it = std::find(events.second.begin(),
                                events.second.end(), id);
            if (it != events.second.end())
            {
                events.second.erase(it);
            }
        }
    }

    void Post(TypeEnum type, std::any param)
    {
        auto it = _events.find(type);
        if (it != _events.end())
        {
            for (auto & e : it->second)
            {
                e.mFunc(type, param);
            }
        }
    }

private:
    std::map<TypeEnum, std::vector<Event>> _events;
};

