#pragma once

#include "base.h"

class AtlasSys;
class RawSys;
class CfgSys;
class UISys;

class Global {
public:
    static Global & Ref()
    {
        static Global instance;
        return instance;
    }

public:
    void Free();

    AtlasSys * mAtlasSys;
    RawSys * mRawSys;
    CfgSys * mCfgSys;
    UISys * mUISys;

private:
    Global();
    ~Global();
};