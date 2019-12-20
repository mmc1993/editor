#include "project.h"

Project::Project(): _gid(0)
{ }

Project::~Project()
{
    //  删除资源, 因为资源不需要SharePtr, 故此在这里删除
    for (auto & pair : _resources)
    {
        delete pair.second;
    }
}

void Project::New(const std::string & url)
{
    _gid = 0x0;
    _url = url;
    _object = NewObject();
}

void Project::Load(const std::string & url)
{
    ASSERT_LOG(_object == nullptr, url.c_str());
    ASSERT_LOG(_resources.empty(), url.c_str());
    std::ifstream is;

    //  加载资源
    is.open(GetResURL(url), std::ios::binary);
    ASSERT_LOG(is, url.c_str());
    tools::Deserialize(is,_gid);

    //  读入Res对象
    uint num = 0; tools::Deserialize(is, num);
    for (auto i = 0; i != num; ++i)
    {
        auto res    = new Res(this);
        res->DecodeBinary(is, this);
        auto pair = std::make_pair(res->GetID(), res);
        _resources.insert(pair);
    }
    is.close();

    //  加载对象
    is.open(url, std::ios::binary);
    ASSERT_LOG(is,    url.c_str());
    _object.reset(new  GLObject());
    _object->DecodeBinary(is,this);
    is.close();
    _url = url;

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
    Retrieve();
}

void Project::Save(const std::string & url)
{
    ASSERT_LOG(_object != nullptr, url.c_str());
    const auto & saveURL = url.empty()?_url:url;

    std::ofstream os;
    //  写入资源
    os.open(GetResURL(saveURL), std::ios::binary);
    tools::Serialize(os, _gid);
    uint num = _resources.size();
    tools::Serialize(os,  num);
    for (auto & pair : _resources)
    {
        pair.second->EncodeBinary(os, this);
    }
    os.close();

    //  写入Obj对象
    os.open(saveURL, std::ios::binary);
    _object->EncodeBinary(os, this);
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

bool Project::DeleteRes(Res * res)
{ 
    if (res->Type() == Res::kNull ||
        res->Type() == Res::kTxt ||
        res->Type() == Res::kImg ||
        res->Type() == Res::kMap ||
        res->Type() == Res::kFnt)
    {
        std::filesystem::remove(res->Path());
        _resources.erase(res->GetID());
        res->WakeRefs();
        delete  res;
        return true;
    }
    return false;
}

bool Project::RenameRes(Res * res, const std::string & url)
{
    if (res->Type() == Res::kNull ||
        res->Type() == Res::kTxt ||
        res->Type() == Res::kImg ||
        res->Type() == Res::kMap ||
        res->Type() == Res::kFnt)
    {
        try
        {
            std::filesystem::rename(res->Path(), url);
            res->Meta(url);
            return true;
        }
        catch (const std::exception &)
        {
            //  挪动文件位置发生异常, 操作失败, 忽略本次操作
        }
    }
    return false;
}

bool Project::SetResType(Res * res, uint type)
{
    auto ret = res->GetRefCount() == 0 
            && type != Res::kObj 
            && type != Res::kVar 
            && type != Res::kBlueprint 
            && (res->Type() == Res::kNull ||
                res->Type() == Res::kTxt ||
                res->Type() == Res::kImg ||
                res->Type() == Res::kMap ||
                res->Type() == Res::kFnt);
    if (ret) { res->Type((Res::TypeEnum)type); }
    return ret;
}

void Project::Retrieve()
{
    std::set<std::string>   set0;     //  文件
    std::set<uint>          set1;     //  id

    for (auto it = _resources.begin(); it != _resources.end();)
    {
        auto tmp = it;
        switch (it->second->Type())
        {
        case Res::kNull:
        case Res::kTxt:
        case Res::kImg:
        case Res::kMap:
        case Res::kFnt:
            if (tools::IsFileExists(it->second->Meta<std::string>()))
            {
                set0.insert(it->second->Meta<std::string>()); ++it;
            }
            break;
        case Res::kObj:
            if (IsExistObject(it->second->Meta<uint>()))
            {
                set1.insert(it->second->Meta<uint>()); ++it;
            }
            break;
        }
        if (tmp == it)
        {
            delete it->second; it = _resources.erase(it); 
        }
    }

    //  检索本地文件
    tools::ListPath("res", [&] (const std::string & path)
        {
            if (0 == set0.count(path))
            {
                auto res = NewRes();
                res->Meta(path);
                res->Type(Res::kNull);
            }
        });

    //  对象
    std::deque<SharePtr<GLObject>> list{
        _object->GetObjects().begin(),
        _object->GetObjects().end()
    };
    while (!list.empty())
    {
        auto & front = list.front();
        std::copy(
            front->GetObjects().begin(),
            front->GetObjects().end(),
            std::back_inserter(list));
        auto objid = front->GetID();
        if (0 == set1.count(objid))
        {
            auto res = NewRes();
            res->Meta(objid);
            res->Type(Res::kObj);
        }
        list.pop_front();
    }
}

Res * Project::GetRes(uint id)
{
    return _resources.at(id);
}

Res * Project::GetResFromPGID(uint PGID)
{
    auto it = std::find_if(_resources.begin(), _resources.end(),
        [PGID] (const std::pair<uint, Res *> & pair)
        {
            return (pair.second->Type() == Res::TypeEnum::kObj 
                 || pair.second->Type() == Res::TypeEnum::kVar)
                 && pair.second->Meta<uint>() == PGID;
        });
    return it != _resources.end() ? it->second : nullptr;
}

std::vector<Res*> Project::GetResByType(const std::vector<Res::TypeEnum> & types)
{
    std::vector<Res *> result;
    for (auto & pair : _resources)
    {
        if (types.empty())
        {
            result.emplace_back(pair.second);
        }
        else
        {
            auto fn =[&pair](const Res::TypeEnum type){return pair.second->Type()==type;};
            if (auto it = std::find_if(types.begin(), types.end(), fn); it != types.end())
            {
                result.emplace_back(pair.second);
            }
        }
    }
    return std::move(result);
}

Res * Project::NewRes()
{
    auto res = new Res(this, ++_gid);
    _resources.emplace(_gid, res);
    return res;
}
