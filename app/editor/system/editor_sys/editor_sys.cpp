#include "editor_sys.h"
#include "project.h"
#include "component/gl_object.h"
#include "../ui_sys/ui_object/ui_object.h"

//  选择文件夹对话框
#include<Shlobj.h>
#include<Commdlg.h>
#pragma comment(lib,"Shell32.lib")

EditorSys::EditorSys(): mProject(nullptr)
{ }

bool EditorSys::OpenProjectDialog(const std::string & url)
{
    char nameBuffer[256] = { 0 };

    //OPENFILENAME ofn = { 0 };
    //ofn.lStructSize     = sizeof(OPENFILENAME);
    //ofn.lpstrFilter     = TEXT("所有文件\0*.*\0*.proj\0\0");
    //ofn.nFilterIndex    = 1;
    //ofn.lpstrFile       = nameBuffer;
    //ofn.nMaxFile        = sizeof(nameBuffer);
    //ofn.lpstrTitle      = "请选择一个文件";
    //ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    //if (GetOpenFileName(&ofn))
    //{
    //    MessageBox(NULL, nameBuffer, TEXT("选择的文件"), 0);
    //}
    return false;
}

bool EditorSys::OpenProject(const std::string & url)
{
    SAFE_DELETE(mProject);
    mProject = new Project();
    return mProject->Load(url);
}

bool EditorSys::SaveProject(const std::string & url)
{
    if (mProject != nullptr)
    {
        mProject->Save(url);
    }
    return true;
}

void EditorSys::FreeProject()
{
    if (mProject != nullptr)
    {
        mProject->Free();
    }
}

bool EditorSys::IsOpenProject() const
{
    return mProject != nullptr;
}

std::string EditorSys::ObjectName(GLObject * object) const
{
    size_t i = 0;
    auto name = SFormat("object_{0}", i++);
    while (object->GetObject(name))
        name = SFormat("object_{0}", i++);
    return std::move(name);
}

bool EditorSys::ObjectName(GLObject * object, const std::string & name) const
{
    if (name.empty()) { return false; }
    return object->GetParent()->GetObject(name) == nullptr;
}
