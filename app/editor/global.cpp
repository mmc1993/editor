#include "global.h"
#include "ui_frame/ui_frame.h"

void Global::Free()
{
    SAFE_DELETE(mUIFrame);
}

Global::Global()
    : mUIFrame(nullptr)
{ }

Global::~Global()
{
    Free();
}
