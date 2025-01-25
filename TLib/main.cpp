
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/String.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Containers/Bitset.hpp>
#include <EASTL/string.h>

int main()
{
    Bitset<8, int> b{ {0, true}, {1, false}, {2, true} };
    tlog::info(b.test(0));
    tlog::info(b.test(1));
    tlog::info(b.test(2));

    return 0;
}