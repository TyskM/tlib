
#include <TLib/Files.hpp>

struct IResource
{
    virtual bool loadFromFile(const Path& path) = 0;
};