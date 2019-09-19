#include "event_sys.h"
#include "../../include.h"

size_t EventSys::s_coundID = 0;

EventSys::Listener::Listener()
{ }

EventSys::Listener::~Listener()
{
    for (auto listen : _listens)
    {
        Global::Ref().mEventSys->Del(listen);
    }
}

void EventSys::Listener::Add(Type id, const std::function<void(const std::any&)>& func)
{
    _listens.push_back(Global::Ref().mEventSys->Add(id, func));
}
