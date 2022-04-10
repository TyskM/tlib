#pragma once

#include <string>

namespace strhelp
{
    bool endswith(const std::string& str, const std::string& ending)
    {
        if (str.length() >= ending.length())
        { return (0 == str.compare(str.length() - ending.length(), ending.length(), ending)); }
        else
        { return false; }
    }

    bool beginswith(const std::string& str, const std::string& beginning)
    {
        if (str.length() >= beginning.length())
        { return (0 == str.compare(0, beginning.length(), beginning)); }
        else
        { return false; }
    }

    std::string whitespace = " \t\n\r\f\v";

    // trims characters in totrim from the right side of the string.
    // by default, totrim will trim any whitespace
    inline void rtrim(std::string& str, const std::string& totrim = whitespace)
    { str.erase(str.find_last_not_of(totrim) + 1); }

    // trims characters in totrim from the left side of the string.
    // by default, totrim will trim any whitespace
    inline void ltrim(std::string& str, const std::string& totrim = whitespace)
    { str.erase(0, str.find_first_not_of(totrim)); }

    // trims characters in totrim from the string.
    // by default, totrim will trim any whitespace
    inline void trim(std::string& str, const std::string& totrim = whitespace)
    { ltrim(str, totrim); rtrim(str, totrim); }

    // Like trim, but returns a copy instead of modifying the original string.
    inline std::string trimmed(const std::string& str, const std::string& totrim = whitespace)
    {
        std::string retStr = str;
        trim(retStr, totrim);
        return retStr;
    }
}
