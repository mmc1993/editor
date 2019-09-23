#include "project.h"
#include "component/gl_object.h"

Project::Project(): mRoot(nullptr)
{ }

Project::~Project()
{
    Free();
}

bool Project::Load(const std::string & url)
{
    std::ifstream is(url);
    ASSERT_LOG(is, url.c_str());
    mRoot = new GLObject();
    return true;
}

void Project::Save(const std::string & url)
{
    ASSERT_LOG(mRoot != nullptr, url.c_str());
    std::ofstream os(url.empty()? mURL: url);
    ASSERT_LOG(os,(url.empty() ? mURL : url).c_str());
    mRoot->EncodeBinary(os);
}

void Project::Free()
{
    SAFE_DELETE(mRoot);
}


