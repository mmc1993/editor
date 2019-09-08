#include "global.h"
#include "ui/ui_mgr.h"
#include "raw/raw_mgr.h"
#include "atlas/atlas_mgr.h"

void Global::Free()
{
    SAFE_DELETE(mUIMgr);
    SAFE_DELETE(mRawMgr);
    SAFE_DELETE(mAtlasMgr);
}

Global::Global()
    : mUIMgr(nullptr)
    , mRawMgr(nullptr)
    , mAtlasMgr(nullptr)
{ }

Global::~Global()
{
    Free();
}
