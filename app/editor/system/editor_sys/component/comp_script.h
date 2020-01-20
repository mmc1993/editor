#pragma once

#include "../../raw_sys/component.h"

class CompScript : public Component {
public:


private:

protected:
    virtual void OnStart() = 0;
    virtual void OnLeave() = 0;
};