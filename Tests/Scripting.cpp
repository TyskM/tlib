
#include <TLib/Scripting/AngelScript/AngelScript.hpp>
#include <TLib/thirdparty/zep/zep.h>

int main()
{

    ScriptingEngine se;
    se.create();
    se.generateDocs();
    se.generateStubs();

    return 0;
}