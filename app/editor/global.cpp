#include "global.h"
#include "system/ui_sys/ui_sys.h"
#include "system/raw_sys/raw_sys.h"
#include "system/atlas_sys/atlas_sys.h"

void Global::Free()
{
    SAFE_DELETE(mUISys);
    SAFE_DELETE(mRawSys);
    SAFE_DELETE(mAtlasSys);
}

Global::Global()
    : mUISys(nullptr)
    , mRawSys(nullptr)
    , mAtlasSys(nullptr)
{ }

Global::~Global()
{
    Free();
}
