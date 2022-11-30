#pragma once

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

/// <summary>
/// This class uses std::filesystem so C++17 is REQUIRED
/// 
/// This class writes stuff to a file.
/// </summary>
class Logger
{
	// Relative to working dir
	// output is outputFilePath + outputFileName + _outputFileCreationDate + outputFileExtension
	const std::string outputFilePath;
	const std::string outputFileDir;
	const std::string outputFileExtension;
	const std::string outputPath;

public:
	// If this is true, the log() function also writes to the console.
	bool logToConsole = false;

	/// <summary>
	/// Logs text to a file at the specified location.
	/// </summary>
	/// <param name="outputPath">Path of the output file without file extension.</param>
	/// <param name="outputExtension">The output files extension eg. ".txt" ".log" etc. It doesn't really matter.</param>
	Logger(const std::string& outputFilePathv, const std::string& outputFileExtensionv) :
		outputFilePath(outputFilePathv),
		outputFileDir(trimFilename(outputFilePathv)),
		outputFileExtension(outputFileExtensionv),
		outputPath(outputFilePath + '_' + getCurrentDateTime() + outputFileExtension)
	{
		std::filesystem::create_directory(outputFileDir);
		std::cout << "Created log at: " << outputPath << '\n';
	}

	static std::string getCurrentDateTimeCustom(const std::string& format)
	{
		//auto t  = std::time(nullptr);
		//auto tm = *localtime_s(&t, *time);

		struct tm newtime;
		time_t now = time(0);
		localtime_s(&newtime, &now);

		std::ostringstream oss;
		oss << std::put_time(&newtime, format.data());
		return oss.str();
	}

	std::string trimFilename(const std::string& filePath)
	{
		std::filesystem::path front(filePath);
		return front.remove_filename().string();
	}

	static std::string getCurrentDateTime() { return getCurrentDateTimeCustom("%Y-%m-%d-%H-%M-%S"); }
	static std::string getCurrentDate()	    { return getCurrentDateTimeCustom("%Y-%m-%d"); };
	static std::string getCurrentTime()	    { return getCurrentDateTimeCustom("%H-%M-%S"); };

	void log(const std::string& text) const
	{
		auto output = getCurrentDateTime() + "\t" + text + "\n";

		if (logToConsole) std::cout << output;

		std::ofstream fs(outputPath.c_str(), std::ios_base::out | std::ios_base::app);
		fs << output;
		fs.close();
	}

	void logAndError(const std::string& text) const
	{
		log(text);
		throw std::runtime_error(text);
	}

	// Error type should take a string as a parameter, like std::runtime_error
	template <typename ErrorType>
	void logAndError(const std::string& text) const
	{
		log(text);
		throw ErrorType(text);
	}
};