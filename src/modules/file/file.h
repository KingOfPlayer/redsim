
#pragma once
#include <cstdio>
#include <cstring>

#define PATH_SIZE 256

struct FilePath{
    char path_length;
    char* path;
};

class FileModule{
public:
    static FilePath SelectFile();
    static FilePath SaveFile();
};