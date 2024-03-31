
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/String.hpp>
#include <TLib/Logging.hpp>
#include <EASTL/string.h>

UnorderedMap<eastl::string, int> testMap;

int main()
{
    testMap.insert_or_assign("test", 420);
    tlog::info(testMap.at("test"));
    return 0;
}