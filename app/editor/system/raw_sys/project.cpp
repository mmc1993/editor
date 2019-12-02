#include "project.h"

Project::Project(): _acc(0)
{ }

void Project::New(const std::string & url)
{
    _acc = 0x0;
    _url = url;
    _root = NewObject();
}

void Project::Load(const std::string & url)
{
    ASSERT_LOG(_root == nullptr, url.c_str());
    std::ifstream is(url, std::ios::binary);
    ASSERT_LOG(is, url.c_str());
    tools::Deserialize(is,_acc);
    _root.reset(new GLObject());
    _root->DecodeBinary(is);
    _url = url;
    is.close();

    //  Éú³ÉObject Map
    std::deque<SharePtr<GLObject>> list{ _root };
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
    ASSERT_LOG(_root != nullptr, url.c_str());
    std::ofstream os(url.empty()? _url : url, std::ios::binary);
    tools::Serialize(os, _acc);
    _root->EncodeBinary(os);
    os.close();
}

SharePtr<GLObject> Project::NewObject()
{
    auto object = std::create_ptr<GLObject>(++_acc);
    _objects.insert(std::make_pair(_acc, object));
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
