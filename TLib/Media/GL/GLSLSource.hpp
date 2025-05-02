#pragma once
#include <TLib/String.hpp>
#include <TLib/Logging.hpp>

// Deprecated
struct GLSLSource
{
private:
    String src;

public:
    GLSLSource() = default;
    GLSLSource(const String& str) : src{str} { }

    void inject(const String& str)
    {
        size_t versionIndex = src.find("#version");
        if (versionIndex == src.size()) { tlog::error("Missing #version"); return; }
        size_t insertPoint = src.find('\n', versionIndex) + 1;
        src.insert(insertPoint, str + '\n');
    }

    String string() const
    { return src; }
};

struct ShaderSource
{
    String src;

    void inject(const String& str)
    {
        size_t versionIndex = src.find("#version");
        if (versionIndex == src.size()) { tlog::error("Missing #version"); return; }
        size_t insertPoint = src.find('\n', versionIndex) + 1;
        src.insert(insertPoint, str + '\n');
    }

    String string() const
    { return src; }
};
