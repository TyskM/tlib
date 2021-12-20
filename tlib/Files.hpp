#pragma once

#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>

// Reads a file into a string
std::string readFile(const std::string& filePath)
{
	// http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html
	std::ifstream in(filePath, std::ios::in | std::ios::binary);
	if (!in) throw std::runtime_error("Failed to read file: " + filePath);
	return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

// Puts each line of a file into a vector of strings
std::vector<std::string> fileToStringVector(std::string path)
{
    std::ifstream file(path);
    std::string line;
    std::vector<std::string> outputVector;

    while(std::getline(file, line))
    { outputVector.push_back(line); }

    return outputVector;
}