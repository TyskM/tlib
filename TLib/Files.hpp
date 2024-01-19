#pragma once

#include <TLib/String.hpp>
#include <TLib/Containers/Vector.hpp>

#include <fstream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;
using Path = fs::path;

struct FileReadError  : public std::runtime_error { using std::runtime_error::runtime_error; };
struct FileWriteError : public std::runtime_error { using std::runtime_error::runtime_error; };

// Reads a file into a string
String readFile(const Path& filePath)
{
    // http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) { return String(); } // return empty string on failure
    return String(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

Vector<char> readFileBytes(const Path& filePath)
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

bool writeToFile(const Path& filePath, const String& value)
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

// Puts each line of a file into a vector of strings
std::vector<String> fileToStringVector(const Path& path)
{
    std::ifstream file(path);
    if (!file) throw FileReadError("Failed to read file: " + path.string());

    String line;
    std::vector<String> outputVector;

    while(std::getline(file, line))
    { outputVector.push_back(line); }

    return outputVector;
}

std::vector<String> dirToStringVector(const Path& path, bool includeSubDirs = false)
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