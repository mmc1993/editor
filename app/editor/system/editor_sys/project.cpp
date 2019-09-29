#include "project.h"
#include "component/gl_object.h"

void Project::New(const std::string & url)
{
    _root.reset(new GLObject());
    _url  = url;
}

bool Project::Load(const std::string & url)
{
    ASSERT_LOG(_root == nullptr, url.c_str());
    std::ifstream is(url);
    ASSERT_LOG(is, url.c_str());
    _root.reset(new GLObject());
    _root->DecodeBinary(is);
    _url  = url;
    is.close();
    return true;
}

void Project::Save(const std::string & url)
{
    ASSERT_LOG(_root != nullptr, url.c_str());
    std::ofstream os(url.empty()? _url : url);
    _root->EncodeBinary(os);
    os.close();
}
