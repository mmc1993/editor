#include "component.h"

static const std::map<std::string, Component *(*)()> ComponentMap = {
    //std::make_pair("Transform", &std::create<Component, )
};

Component * Component::Create(const std::string & tag)
{
    auto it = ComponentMap.find(tag);
    ASSERT_LOG(it != ComponentMap.end(), tag.c_str());
    return it->second();
}
