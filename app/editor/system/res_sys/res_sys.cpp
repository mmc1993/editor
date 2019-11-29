#include "res_sys.h"

Res * ResSys::GetRes(uint id)
{
    return _resources.at(id);
}

std::vector<Res *> ResSys::GetResByType(Res::TypeEnum type)
{
    std::vector<Res*> result;
    std::copy_if(_resources.begin(), _resources.end(), 
        std::back_inserter(result), [&type] (const auto & pair)
        {
            return type == pair.second->GetType();
        });
    return std::move(result);
}
