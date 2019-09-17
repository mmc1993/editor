#include "component.h"
#include "comp_transform.h"

static const std::map<std::string, Component *(*)()> ComponentMap = {
    std::make_pair("Transform", &std::create<Component, CompTransform>)
};

Component * Component::Create(const std::string & tag)
{
    auto it = ComponentMap.find(tag);
    ASSERT_LOG(it != ComponentMap.end(), tag.c_str());
    return it->second();
}
