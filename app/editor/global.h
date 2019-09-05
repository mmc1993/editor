#pragma once

#include "include.h"

class AtlasMgr;
class UIMgr;
class RawMgr;

class Global {
public:
    static Global & Ref()
    {
        static Global instance;
        return instance;
    }

public:
    void Free();

    AtlasMgr * mAtlasMgr;
    RawMgr * mRawMgr;
    UIMgr * mUIMgr;

private:
    Global();
    ~Global();
};