#include "global.h"
#include "system/ui_sys/ui_sys.h"
#include "system/raw_sys/raw_sys.h"
#include "system/cfg_sys/cfg_sys.h"
#include "system/atlas_sys/atlas_sys.h"
#include "system/event_sys/event_sys.h"
#include "system/editor_sys/editor_sys.h"

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


