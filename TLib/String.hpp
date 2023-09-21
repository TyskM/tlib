#pragma once

#include <string>
#include <sstream>
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <ios>
#include <iomanip>
#include <vector>
#include <boost/algorithm/string/predicate.hpp>
#include <TLib/Containers/Vector.hpp>

using String = std::string;

namespace strhelp
{
    inline Vector<String> split(String src, const String& delimiter)
    {
        Vector<String> ret;
        size_t pos = 0;
        String token;
        while ((pos = src.find(delimiter)) != std::string::npos)
        {
            token = src.substr(0, pos);
            src.erase(0, pos + delimiter.length());
            ret.push_back(src);
        }
        return ret;
    }

    inline String toLower(const String& s)
    {
        String newstr = s;
        std::transform(newstr.begin(), newstr.end(), newstr.begin(), [](unsigned char c){ return std::tolower(c); });
        return newstr;
    }

    inline String toUpper(const String& s)
    {
        String newstr = s;
        std::transform(newstr.begin(), newstr.end(), newstr.begin(), [](unsigned char c){ return std::toupper(c); });
        return newstr;
    }

    inline bool endswith(const String& str, const String& ending)
    {
        if (str.length() >= ending.length())
        { return (0 == str.compare(str.length() - ending.length(), ending.length(), ending)); }
        else
        { return false; }
    }

    inline bool endswith(const String& str, const std::initializer_list<String> endings)
    {
        bool r = true;
        for (auto& ending : endings)
        { if (!endswith(str, ending)) { r = false; } }
        return r;
    }

    inline bool beginswith(const String& str, const String& beginning)
    {
        if (str.length() >= beginning.length())
        { return (0 == str.compare(0, beginning.length(), beginning)); }
        else
        { return false; }
    }

    inline bool beginswith(const String& str, const std::initializer_list<String> beginnings)
    {
        bool r = true;
        for (auto& beginning : beginnings)
        { if (!beginswith(str, beginning)) { r = false; } }
        return r;
    }

    inline bool replaceFirst(String& str, const String& from, const String& to)
    {
        size_t start_pos = str.find(from);
        if (start_pos == String::npos) { return false; }
        str.replace(start_pos, from.length(), to);
        return true;
    }

    inline void replace(String& str, const String& from, const String& to)
    {
        // https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string

        if (from.empty()) { return; }
        size_t start_pos = 0;

        while ((start_pos = str.find(from, start_pos)) != String::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    inline constexpr const char* whitespace = " \t\n\r\f\v";

    // trims characters in totrim from the right side of the string.
    // by default, totrim will trim any whitespace
    inline void rtrim(String& str, const String& totrim = whitespace)
    { str.erase(str.find_last_not_of(totrim) + 1); }

    // trims characters in totrim from the left side of the string.
    // by default, totrim will trim any whitespace
    inline void ltrim(String& str, const String& totrim = whitespace)
    { str.erase(0, str.find_first_not_of(totrim)); }

    // trims characters in totrim from the string.
    // by default, totrim will trim any whitespace
    inline void trim(String& str, const String& totrim = whitespace)
    { ltrim(str, totrim); rtrim(str, totrim); }

    // Like trim, but returns a copy instead of modifying the original string.
    inline String trimmed(const String& str, const String& totrim = whitespace)
    {
        String retStr = str;
        trim(retStr, totrim);
        return retStr;
    }

    inline String floatToStr(float v, size_t precision)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << v;
        return ss.str();
    }

    inline bool compareIgnoreCase(const String& first, const String& second)
    { return boost::iequals(first, second); }
}

using namespace strhelp;
