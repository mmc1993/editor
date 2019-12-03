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
        auto res = new Res(this); res->DecodeBinary(is);
        auto pair = std::make_pair(res->GetID(), res);
        _resources.insert(pair);
    }
    is.close();

    //  加载对象
    is.open(url, std::ios::binary);
    ASSERT_LOG(is,    url.c_str());
    _object.reset(new  GLObject());
    _object->DecodeBinary(is);
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
}

void Project::Save(const std::string & url)
{
    ASSERT_LOG(_object != nullptr, url.c_str());
    const auto & saveURL = url.empty()?_url:url;

    std::ofstream os;
    //  写入资源
    os.open(GetResURL(saveURL), std::ios::binary);
    tools::Serialize(os,    _gid);
    uint num =  _resources.size();
    tools::Serialize(os,     num);
    for (auto & pair : _resources)
    {
        pair.second->EncodeBinary(os);
    }
    os.close();

    //  写入Obj对象
    os.open(saveURL,    std::ios::binary);
    _object->EncodeBinary(os); os.close();
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
    if (res->Type() == Res::kTxt ||
        res->Type() == Res::kImg ||
        res->Type() == Res::kMap ||
        res->Type() == Res::kFont)
    {
        _resources.erase(res->GetID());
        res->WakeRefs();
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

void Project::Retrieve()
{
    std::set<uint>          set0;       //  对象
    std::set<std::string>   set1;       //  文件

    for (auto & pair : _resources)
    {
        switch (pair.second->Type())
        {
        case Res::kNull:
        case Res::kTxt:
        case Res::kImg:
        case Res::kMap:
        case Res::kFont:
            set1.insert(std::any_cast<std::string>(pair.second->Instance()));
            break;
        case Res::kObj:
            set0.insert(std::any_cast<SharePtr<GLObject>>(pair.second->Instance())->GetID());
            break;
        }
    }

    //  对象
    std::deque<SharePtr<GLObject>> list{ _object };
    while (!list.empty())
    {
        auto & front = list.front();
        std::copy(
            front->GetObjects().begin(),
            front->GetObjects().end(),
            std::back_inserter(list));
        if (0 == set0.count(front->GetID()))
        {
            auto res = NewRes();
            res->BindMeta(front);
            res->Type(Res::kObj);
        }
        list.pop_front();
    }

    //  检索本地文件
    tools::ListPath("res", [&] (const std::string & path)
        {
            if (0 == set1.count(path))
            {
                auto res = NewRes();
                res->BindMeta(path);
                res->Type(Res::kNull);
            }
        });
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

Res * Project::NewRes()
{
    auto res = new Res(this, ++_gid);
    _resources.emplace(_gid, res);
    return res;
}
