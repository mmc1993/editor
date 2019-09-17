#include "global.h"
#include "system/ui_sys/ui_sys.h"
#include "system/raw_sys/raw_sys.h"
#include "system/cfg_sys/cfg_sys.h"
#include "system/atlas_sys/atlas_sys.h"
#include "system/event_sys/event_sys.h"
#include "system/editor_sys/editor_sys.h"

// ---
//  Global::EventListener
// ---
Global::EventListener::EventListener()
{ }

Global::EventListener::~EventListener()
{ 
    for (auto listen : _listens)
    {
        Global::Ref().mEventSys->Del(listen);
    }
}

void Global::EventListener::AddListen(size_t id, const std::function<void(const std::any&)> & func)
{
    _listens.push_back(Global::Ref().mEventSys->Add(id, func));
}

// ---
//  Global
// ---
void Global::Free()
{
    SAFE_DELETE(mUISys);
    SAFE_DELETE(mRawSys);
    SAFE_DELETE(mCfgSys);
    SAFE_DELETE(mAtlasSys);
    SAFE_DELETE(mEventSys);
    SAFE_DELETE(mEditorSys);
}

Global::Global()
    : mUISys(nullptr)
    , mRawSys(nullptr)
    , mCfgSys(nullptr)
    , mAtlasSys(nullptr)
    , mEventSys(nullptr)
    , mEditorSys(nullptr)
{ }

Global::~Global()
{
    Free();
}


