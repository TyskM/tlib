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
#include <EASTL/string.h>
#include <codecvt>


using String       = std::string;
using StringStream = std::stringstream;

using WideString       = std::wstring;
using WideStringStream = std::wstringstream;

namespace strhelp
{
    inline WideString toWide(const String& strIn)
    {
        WideString ws;
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        ws = converter.from_bytes(strIn);
        return ws;
    }

    template <typename StrT>
    inline Vector<StrT> split(StrT src, const StrT& delimiter)
    {
        Vector<StrT> ret;
        size_t pos = 0;
        StrT token;
        while ((pos = src.find(delimiter)) != std::string::npos)
        {
            token = src.substr(0, pos);
            src.erase(0, pos + delimiter.length());
            ret.push_back(src);
        }
        return ret;
    }

    template <typename StrT>
    inline StrT toLower(StrT str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](auto c){ return std::tolower(c); });
        return str;
    }

    template <typename StrT>
    inline String toUpper(StrT str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](auto c){ return std::toupper(c); });
        return str;
    }

    template <typename StrT>
    inline bool endswith(const StrT& str, const StrT& ending)
    {
        if (str.length() >= ending.length())
        { return (0 == str.compare(str.length() - ending.length(), ending.length(), ending)); }
        else
        { return false; }
    }

    template <typename StrT>
    inline bool endswith(const StrT& str, const std::initializer_list<StrT> endings)
    {
        bool r = true;
        for (auto& ending : endings)
        { if (!endswith(str, ending)) { r = false; } }
        return r;
    }

    template <typename StrT>
    inline bool beginswith(const StrT& str, const StrT& beginning)
    {
        if (str.length() >= beginning.length())
        { return (0 == str.compare(0, beginning.length(), beginning)); }
        else
        { return false; }
    }

    template <typename StrT>
    inline bool beginswith(const StrT& str, const std::initializer_list<StrT> beginnings)
    {
        bool r = true;
        for (auto& beginning : beginnings)
        { if (!beginswith(str, beginning)) { r = false; } }
        return r;
    }

    template <typename StrT>
    inline bool replaceFirst(StrT& str, const StrT& from, const StrT& to)
    {
        size_t start_pos = str.find(from);
        if (start_pos == StrT::npos) { return false; }
        str.replace(start_pos, from.length(), to);
        return true;
    }

    template <typename StrT>
    inline void replace(StrT& str, const StrT& from, const StrT& to)
    {
        // https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string

        if (from.empty()) { return; }
        size_t start_pos = 0;

        while ((start_pos = str.find(from, start_pos)) != StrT::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    inline constexpr const char* whitespace = " \t\n\r\f\v";

    // trims characters in totrim from the right side of the string.
    // by default, totrim will trim any whitespace
    template <typename StrT>
    inline void rtrim(StrT& str, const StrT& totrim = whitespace)
    { str.erase(str.find_last_not_of(totrim) + 1); }

    // trims characters in totrim from the left side of the string.
    // by default, totrim will trim any whitespace
    template <typename StrT>
    inline void ltrim(StrT& str, const StrT& totrim = whitespace)
    { str.erase(0, str.find_first_not_of(totrim)); }

    // trims characters in totrim from the string.
    // by default, totrim will trim any whitespace
    template <typename StrT>
    inline void trim(StrT& str, const StrT& totrim = whitespace)
    { ltrim(str, totrim); rtrim(str, totrim); }

    // Like trim, but returns a copy instead of modifying the original string.
    template <typename StrT>
    inline StrT trimmed(const StrT& str, const StrT& totrim = whitespace)
    {
        StrT retStr = str;
        trim(retStr, totrim);
        return retStr;
    }

    template <typename StrT>
    inline StrT floatToStr(float v, size_t precision)
    {
        std::basic_stringstream<StrT::value_type, std::char_traits<StrT::value_type>, MiAllocator> ss;
        ss << std::fixed << std::setprecision(precision) << v;
        auto str = ss.str();
        return StrT(str.c_str());
    }

    template <typename StrT>
    inline bool compareIgnoreCase(const String& first, const String& second)
    { return boost::iequals(first, second); }
}

using namespace strhelp;
