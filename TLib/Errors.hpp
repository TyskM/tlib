#pragma once

#include <iostream>

// It's std::runtime_error but it also writes to the console.
// Crazy.
struct RuntimeError : public std::runtime_error
{
	explicit RuntimeError(const std::string& message) : std::runtime_error(message.c_str()) { printError(); }
	explicit RuntimeError(const char* message) : std::runtime_error(message) { printError(); }
	explicit RuntimeError() : std::runtime_error("") { }

	void printError()
	{ std::cout << what() << '\n'; }
};