#pragma once

#include <TLib/String.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Logging.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>

#include <fstream>
#include <stdexcept>
#include <filesystem>

#include <TLib/Windows.hpp>

namespace fs = std::filesystem;
using Path = fs::path;

struct FileReadError  : public std::runtime_error { using std::runtime_error::runtime_error; };
struct FileWriteError : public std::runtime_error { using std::runtime_error::runtime_error; };

// If the user cancels, path.empty() will be true
static Path openSingleFileDialog(
    const String&         title             = "Open File",
          Path            defaultPath       = Path(), // Will use the .exe directory if empty
    const Vector<String>& filters           = {"*"},  // {"*.jpg","*.png"}
    const String&         filterDescription = "")     // "image files"
{
    Vector<const char*> cfilters;
    cfilters.reserve(filters.size());
    for (auto& s : filters)
    { cfilters.push_back(s.c_str()); }

    if (defaultPath.empty())
    { defaultPath = fs::current_path(); }

    char* openedFilePath = tinyfd_openFileDialog(
        title.c_str(),
        defaultPath.string().c_str(),
        cfilters.size(),
        cfilters.data(),
        filterDescription.empty() ? NULL : filterDescription.c_str(), 0);

    if (!openedFilePath)
    { return Path(); }
    return openedFilePath;
}

// This opens the file explorer and selects the file/folder provided
static bool browseToFile(const Path& path)
{
#ifdef OS_WINDOWS
    String pathStr = path.string();
    String params  = fmt::format("/select, \"{}\"", pathStr);

    auto hret = ShellExecuteA(NULL, "open", "explorer.exe", params.c_str(), NULL, SW_SHOWDEFAULT);
    int32_t ret = static_cast<int32_t>(reinterpret_cast<uintptr_t>(hret));
    if (ret <= 32)
    {
        tlog::error("\nShellExecuteA failed with code: {}\nReason: {}\nCommand: {}", ret, getLastWin32ErrorAsString(), params);
        return false;
    }
    return true;
#else
    tlog::error("browseToFile is only supported on windows. (for now)");
#endif
}

// Reads a file into a string
// Returns empty string on failure
static String readFile(const Path& filePath)
{
    // http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) { return String(); } // return empty string on failure
    return String(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

static Vector<char> readFileBytes(const Path& filePath)
{
    std::ifstream in(filePath, std::ios_base::binary);
    if (!in) throw FileReadError("Failed to read file: " + filePath.string());

    in.seekg(0, in.end);
    size_t length = in.tellg();
    in.seekg(0, in.beg);

    Vector<char> buffer;
    if (length > 0) {
        buffer.resize(length);
        in.read(&buffer[0], length);
    }
    return buffer;
}

static bool writeToFile(const Path& filePath, const String& value)
{
    auto parentPath = filePath.parent_path();
    if (!parentPath.empty())
    { fs::create_directories(parentPath); }

    std::ofstream out(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
    out << value;
    out.close();

    if (out.fail())
    { return false; }

    return true;
}

static bool createDirectories(const Path& dirPath)
{
    return fs::create_directories(dirPath);
}

// Puts each line of a file into a vector of strings
static std::vector<String> fileToStringVector(const Path& path)
{
    std::ifstream file(path);
    if (!file) throw FileReadError("Failed to read file: " + path.string());

    String line;
    std::vector<String> outputVector;

    while(std::getline(file, line))
    { outputVector.push_back(line); }

    return outputVector;
}

static std::vector<String> dirToStringVector(const Path& path, bool includeSubDirs = false)
{
    std::vector<String> vec;

    if (includeSubDirs)
    {
        for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::follow_directory_symlink))
        {
            if (!entry.is_regular_file()) continue;
            const auto str = entry.path().string();
            vec.push_back(str);
        }
        return vec;
    }

    else
    {
        for (const auto& entry : fs::directory_iterator(path, fs::directory_options::follow_directory_symlink))
        {
            if (!entry.is_regular_file()) continue;
            const auto str = entry.path().string();
            vec.push_back(str);
        }
        return vec;
    }
}