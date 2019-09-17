#pragma once

#include "base.h"

class EditorSys;
class AtlasSys;
class EventSys;
class RawSys;
class CfgSys;
class UISys;

class Global {
public:
    class EventListener {
    public:
        EventListener();
        ~EventListener();
        void AddListen(size_t id, const std::function<void(const std::any &)> & func);

    private:
        std::vector<size_t> _listens;
    };

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
    AtlasSys * mAtlasSys;
    RawSys * mRawSys;
    CfgSys * mCfgSys;
    UISys * mUISys;

private:
    Global();
    ~Global();
};