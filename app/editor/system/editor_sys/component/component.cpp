#include "component.h"
#include "comp_transform.h"

static const std::map<std::string, Component *(*)()> ComponentMap = {
    std::make_pair("Transform", &std::create<Component, CompTransform>)
};

Component * Component::Create(const std::string & name)
{
    auto it = ComponentMap.find(name);
    ASSERT_LOG(it != ComponentMap.end(), name.c_str());
    return it->second();
}
