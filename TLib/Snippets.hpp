#pragma once
// Things to copy/paste. Do not include.

struct WrapperExample
{
    WrapperExample() = default;
   ~WrapperExample() { reset(); }

    void create()
    {

    }

    void reset()
    {
        if (!created()) { return; }
    }

    bool created() { return false; }
};