#pragma once

#include "base.h"

class EditorSys;
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

    EditorSys * mEditorSys;
    AtlasSys * mAtlasSys;
    RawSys * mRawSys;
    CfgSys * mCfgSys;
    UISys * mUISys;

private:
    Global();
    ~Global();
};