#pragma once

class EditorSys;
class EventSys;
class RawSys;
class ResSys;
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
    EventSys * mEventSys;
    RawSys * mRawSys;
    ResSys * mResSys;
    CfgSys * mCfgSys;
    UISys * mUISys;

private:
    Global();
    ~Global();
};