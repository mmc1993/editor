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
        //  ui object
        kSelectObject,    //  { GLOBject 选中的节点, bool 是否选中, bool 是否多选 }
        kDeleteObject,    //  { GLOBject 删除的节点 }
        kInsertObject,    //  { GLObject 新增的节点  }
        kRenameObject,    //  { GLOBject 选中的节点, string 旧名字 }
        //  component
        kDeleteComponent,   //  { GLOBject 选中的节点, Component 选中的组件 }
        kAppendComponent,   //  { GLOBject 选中的节点, Component 新增的组件 }
    };

    using func_t = std::function<void(TypeEnum type, const std::any &)>;


    //  监听器
    class Listener {
    public:
        Listener();
        ~Listener();
        void Add(TypeEnum id, const func_t & func);

    private:
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

