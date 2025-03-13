#include "pch.h"
#include "FileSystem.h"

using namespace std;

void CreateFolders(string path)
{
    vector<string> folders = SplitString(path, "/");

    string temp = "";
    for (string folder : folders)
    {
        temp += folder + "/";

        if (!ExistDirectory(temp))
            CreateDirectoryA(temp.c_str(), 0);
    }
}

bool ExistDirectory(string path)
{
    DWORD fileValue = GetFileAttributesA(path.c_str());

    BOOL temp = (fileValue != INVALID_FILE_ATTRIBUTES &&
        (fileValue & FILE_ATTRIBUTE_DIRECTORY));

    return temp == TRUE;
}

bool ExistFile(string path)
{
    DWORD fileValue = GetFileAttributesA(path.c_str());

    return fileValue < 0xffffffff;
}
