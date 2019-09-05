#pragma once

#include "include.h"

class UIFrame;

class Global {
public:
    static Global & Ref()
    {
        static Global instance;
        return instance;
    }

public:
    void Free();

    UIFrame * mUIFrame;

private:
    Global();
    ~Global();
};