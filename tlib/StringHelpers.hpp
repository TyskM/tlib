#pragma once

#include <string>
#include <initializer_list>
#include <utility>
#include <algorithm>

namespace strhelp
{
    std::string toLower(const std::string& s)
    {
        auto newstr = s;
        std::transform(newstr.begin(), newstr.end(), newstr.begin(), [](unsigned char c){ return std::tolower(c); });
        return newstr;
    }

    std::string toUpper(const std::string& s)
    {
        auto newstr = s;
        std::transform(newstr.begin(), newstr.end(), newstr.begin(), [](unsigned char c){ return std::toupper(c); });
        return newstr;
    }

    bool endswith(const std::string& str, const std::string& ending)
    {
        if (str.length() >= ending.length())
        { return (0 == str.compare(str.length() - ending.length(), ending.length(), ending)); }
        else
        { return false; }
    }

    bool endswith(const std::string& str, const std::initializer_list<std::string> endings)
    {
        bool r = true;
        for (auto& ending : endings)
        { if (!endswith(str, ending)) { r = false; } }
        return r;
    }

    bool beginswith(const std::string& str, const std::string& beginning)
    {
        if (str.length() >= beginning.length())
        { return (0 == str.compare(0, beginning.length(), beginning)); }
        else
        { return false; }
    }

    bool beginswith(const std::string& str, const std::initializer_list<std::string> beginnings)
    {
        bool r = true;
        for (auto& beginning : beginnings)
        { if (!beginswith(str, beginning)) { r = false; } }
        return r;
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

    std::string floatToStr(float v, size_t precision)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << v;
        return ss.str();
    }
}
