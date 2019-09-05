#include "global.h"
#include "ui/ui_mgr.h"
#include "raw/raw_mgr.h"

void Global::Free()
{
    //SAFE_DELETE(mAtlasMgr);
    SAFE_DELETE(mUIMgr);
    SAFE_DELETE(mRawMgr);
}

Global::Global()
    : mUIMgr(nullptr)
    , mRawMgr(nullptr)
{ }

Global::~Global()
{
    Free();
}
