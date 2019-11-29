#include "res_sys.h"
#include "../event_sys/event_sys.h"

Res * ResSys::GetRes(uint id)
{
    return _resources.at(id);
}

std::vector<Res *> ResSys::GetResByType(Res::TypeEnum type)
{
    return GetResByType({ type });
}

std::vector<Res*> ResSys::GetResByType(std::initializer_list<Res::TypeEnum> types)
{
    std::vector<Res *> result;
    for (auto & pair : _resources)
    {
        auto fn = [&pair] (const Res::TypeEnum type)
        {
            return pair.second->GetType() == type;
        };

        if (auto it = std::find_if(types.begin(), types.end(), fn); it != types.end())
        {
            result.emplace_back(pair.second);
        }
    }
    return std::move(result);
}

void ResSys::DeleteRes(Res * res)
{ 
    ASSERT_LOG(res->GetRefCount() == 0, "");
    _resources.erase(res->GetID());
    delete res;
}

void ResSys::OptDeleteRes(uint id)
{
    OptDeleteRes(_resources.at(id));
}

void ResSys::OptDeleteRes(Res * res)
{
    if (res->GetRefCount() == 0 && (res->GetType() == Res::kTxt ||
                                    res->GetType() == Res::kImg ||   
                                    res->GetType() == Res::kMap ||
                                    res->GetType() == Res::kFont))
    {
        auto value = std::any_cast<std::string>(res->Load());
        auto tuple = std::make_tuple(res->GetID(), value);
        DeleteRes(res);

        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteRes, tuple);
    }
}

void ResSys::OptModifyRes(uint id, const std::string & url)
{
    OptModifyRes(_resources.at(id), url);
}

void ResSys::OptModifyRes(Res * res, const std::string & url)
{
    if (res->GetType() == Res::kTxt ||
        res->GetType() == Res::kImg ||   
        res->GetType() == Res::kMap ||
        res->GetType() == Res::kFont)
    {
        try
        {
            auto oldPath = std::any_cast<std::string>(res->Load());
            std::filesystem::rename(oldPath, url); res->BindMeta(url);
            auto newPath = std::any_cast<std::string>(res->Load());

            Global::Ref().mEventSys->Post(EventSys::TypeEnum::kDeleteRes, std::make_tuple(res, oldPath, newPath));
        }
        catch (const std::exception &)
        {
            //  无视该异常
        }
    }
}

void ResSys::OptSetResType(uint id, uint type)
{
    OptSetResType(_resources.at(id), type);
}

void ResSys::OptSetResType(Res * res, uint type)
{
    if (res->GetRefCount() == 0)
    {
        res->SetType((Res::TypeEnum)type);

        Global::Ref().mEventSys->Post(EventSys::TypeEnum::kSetResType, std::make_tuple(res, (Res::TypeEnum)type));
    }
}
