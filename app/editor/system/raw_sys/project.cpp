#include "project.h"

Project::Project(): _gid(0)
{ }

void Project::New(const std::string & url)
{
    _gid = 0x0;
    _url = url;
    _object = NewObject();
}

void Project::Load(const std::string & url)
{
    ASSERT_LOG(_object == nullptr, url.c_str());
    std::ifstream is(url, std::ios::binary);
    ASSERT_LOG(is, url.c_str());
    tools::Deserialize(is,_gid);
    _object.reset(new GLObject());
    _object->DecodeBinary(is);
    _url = url;
    is.close();

    //  生成Object Map
    std::deque<SharePtr<GLObject>> list{ _object };
    while (!list.empty())
    {
        auto & front = list.front();
        std::copy(
            front->GetObjects().begin(),
            front->GetObjects().end(),
            std::back_inserter(list));
        _objects.insert(std::make_pair(front->GetID(), front));
        list.pop_front();
    }
}

void Project::Save(const std::string & url)
{
    ASSERT_LOG(_object != nullptr, url.c_str());
    std::ofstream os(url.empty()? _url : url, std::ios::binary);
    tools::Serialize(os, _gid);
    _object->EncodeBinary(os);
    os.close();
}

SharePtr<GLObject> Project::NewObject()
{
    auto object = std::create_ptr<GLObject>(++_gid);
    _objects.insert(std::make_pair(_gid, object));
    return object;
}

void Project::DeleteObject(const uint & id)
{
    _objects.erase(id);
}

void Project::DeleteObject(const SharePtr<GLObject> & object)
{
    DeleteObject(object->GetID());
}

bool Project::DeleteRes(uint id)
{
    return DeleteRes(_resources.at(id));
}

bool Project::DeleteRes(Res * res)
{ 
    ASSERT_LOG(res->GetRefCount() == 0, "");
    if (res->GetRefCount() == 0 && (res->Type() == Res::kTxt ||
                                    res->Type() == Res::kImg ||   
                                    res->Type() == Res::kMap ||
                                    res->Type() == Res::kFont))
    {
        auto value = std::any_cast<std::string>(res->Instance());
        auto tuple = std::make_tuple(res->GetID(), value);
        _resources.erase(res->GetID());
        delete  res;
        return true;
    }
    return false;
}

bool Project::ModifyRes(uint id, const std::string & url)
{
    return ModifyRes(_resources.at(id), url);
}

bool Project::ModifyRes(Res * res, const std::string & url)
{
    if (res->Type() == Res::kTxt ||
        res->Type() == Res::kImg ||
        res->Type() == Res::kMap ||
        res->Type() == Res::kFont)
    {
        try
        {
            auto oldPath = std::any_cast<std::string>(res->Instance());
            std::filesystem::rename(oldPath, url);  res->BindMeta(url);
            return true;
        }
        catch (const std::exception &)
        {
            //  挪动文件位置发生异常, 操作失败, 忽略本次操作
        }
    }
    return false;
}

bool Project::SetResType(uint id, uint type)
{
    return SetResType(_resources.at(id), type);
}

bool Project::SetResType(Res * res, uint type)
{
    if (res->GetRefCount() == 0)
    {
        res->Type((Res::TypeEnum)type);
    }
    return res->GetRefCount() == 0;
}

Res * Project::GetRes(uint id)
{
    return _resources.at(id);
}

std::vector<Res*> Project::GetResByType(const Res::TypeEnum & type)
{
    return GetResByType({ type });
}

std::vector<Res*> Project::GetResByType(const std::initializer_list<Res::TypeEnum> & types)
{
    std::vector<Res *> result;
    for (auto & pair : _resources)
    {
        auto fn =[&pair](const Res::TypeEnum type){return pair.second->Type()==type;};
        if (auto it = std::find_if(types.begin(), types.end(), fn); it != types.end())
        {
            result.emplace_back(pair.second);
        }
    }
    return std::move(result);
}
