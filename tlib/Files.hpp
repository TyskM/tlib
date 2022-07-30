#pragma once

#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>

struct FileReadError  : public std::runtime_error { using std::runtime_error::runtime_error; };
struct FileWriteError : public std::runtime_error { using std::runtime_error::runtime_error; };

// Reads a file into a string
std::string readFile(const std::string& filePath)
{
	// http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html
	std::ifstream in(filePath, std::ios::in | std::ios::binary);
	if (!in) throw FileReadError("Failed to read file: " + filePath);
	return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

void writeToFile(const std::string& filePath, const std::string& value)
{
    std::ofstream out(filePath, std::ios::out | std::ios::binary);
    if (!out) throw FileWriteError("Failed to write file: " + filePath);
    out << value;
    out.close();
}

// Puts each line of a file into a vector of strings
std::vector<std::string> fileToStringVector(std::string path)
{
    std::ifstream file(path);
    if (!file) throw FileReadError("Failed to read file: " + path);

    std::string line;
    std::vector<std::string> outputVector;

    while(std::getline(file, line))
    { outputVector.push_back(line); }

    return outputVector;
}