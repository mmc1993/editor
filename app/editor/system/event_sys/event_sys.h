#pragma once

#include "../../base.h"

class EventSys {
public:
    using func_t = std::function<void(const std::any &)>;

    //  事件枚举
    enum class Type {
        kSELECT_GLOBJECT,   //  选中对象
    };

    //  监听器
    class Listener {
    public:
        Listener();
        ~Listener();
        void Add(Type id, const std::function<void(const std::any &)> & func);

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

    size_t Add(Type type, const func_t & func)
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

    void Post(Type type, std::any param)
    {
        auto it = _events.find(type);
        if (it != _events.end())
        {
            for (auto & e : it->second)
            {
                e.mFunc(param);
            }
        }
    }

private:
    std::map<Type, std::vector<Event>> _events;
};

