#include "event_sys.h"
#include "../../base.h"
#include "../../global.h"

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

void EventSys::Listener::Add(TypeEnum id, const func_t & func)
{
    _listens.push_back(Global::Ref().mEventSys->Add(id, func));
}
