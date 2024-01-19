
#include <CLI/App.hpp>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace spdlog;
using String = std::string;
namespace fs = std::filesystem;
using Path = fs::path;

static std::shared_ptr<logger> createConsoleLogger(const char* name, const String& pattern = "[%H:%M:%S] [%^%l%$] [%n] [thread %t] %v") noexcept
{
    std::shared_ptr<logger> console = spdlog::stdout_color_mt(name);
    console->set_pattern(pattern);
    return console;
}

std::vector<char> readFileBytes(const Path& filePath)
{
    std::ifstream in(filePath, std::ios_base::binary);

    if (!in)
    { return std::vector<char>(); }

    in.seekg(0, in.end);
    size_t length = in.tellg();
    in.seekg(0, in.beg);

    std::vector<char> buffer;
    if (length > 0) {
        buffer.resize(length);
        in.read(&buffer[0], length);
    }

    return buffer;
}

bool writeToFile(const Path& filePath, const String& value)
{
    if (fs::is_directory(filePath.parent_path()))
    { fs::create_directories(filePath.parent_path()); }
    std::ofstream out(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << value;
    out.close();
    return true;
}

void replace(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty()) { return; }
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

constexpr const char* outputSrc = R"(
#pragma once
#include <unordered_map>
#include <string>
#include <vector>

struct EmbeddedData
{{
private:
    std::vector<char> data;

public:
    EmbeddedData() = default;
    EmbeddedData(std::initializer_list<char> bytes) : data{{bytes}} {{ }}

    // Returns data as bytes
    const std::vector<char> asBytes() const
    {{ return data; }}

    // Converts and returns data as a string
    String asString() const
    {{ return String(data.begin(), data.end()); }}
}};

std::unordered_map<std::string, EmbeddedData> {mapName} =
{{
    {mapVars}
}};

)";

struct EmbeddedData
{
private:
    std::vector<char> data;

public:
    EmbeddedData() = default;
    EmbeddedData(std::initializer_list<char> bytes) : data{bytes} { }

    // Returns data as bytes
    const std::vector<char> asBytes() const
    { return data; }

    // Converts and returns data as a string
    String asString() const
    { return String(data.begin(), data.end()); }
};

int main(int argc, char* argv[])
{
    set_default_logger(createConsoleLogger("Main"));

    #ifdef TLIB_DEBUG
        tlog::info("Arg count: {}", argc);
        for (int i = 0; i < argc; i++)
        { tlog::info("Arg {}: {}", i, argv[i]); }
    #endif

    std::vector<String> embedFiles;
    String              outPath = "";
    String              embedMapName = "myEmbeds";
    bool                overwrite = false;

    CLI::App app("Embed files");

    app.add_option(
        "-f,--file",
        embedFiles,
        "Individual files to embed")
        ->required();

    app.add_option(
        "-o,--out",
        outPath,
        "Output path (including output filename)")
        ->required();

    app.add_option(
        "-n,--mapName",
        embedMapName,
        "Name of the map variable used to access your data (myEmbeds by default)")
        ->default_val(embedMapName);
    
    app.add_flag(
        "--ow",
        overwrite,
        "Overwrite output (You probably want this but it's false by default to be safe)")
        ->default_val(overwrite);

    CLI11_PARSE(app, argc, argv);

    spdlog::info("Working dir: {}", fs::current_path().string());

    std::stringstream mainStream;
    spdlog::info("Got files: ");

    for (size_t i = 0; i < embedFiles.size(); i++)
    {
        const String& f = embedFiles[i];
        spdlog::info(f);

        // Normalize key
        String normalizedPath = fs::path(f).lexically_normal().string();
        replace(normalizedPath, "\\", "/");

        // Get value from file
        std::vector<char> bytes;
        bytes = readFileBytes(f);
        if (bytes.empty())
        { spdlog::error("Failed to read file: {}\n Exiting...", f); return 1; }
        
        std::stringstream byteStream;
        byteStream << "{{ ";
        for (size_t i = 0; i < bytes.size(); i++)
        {
            auto& byte = bytes[i];
            byteStream << "0x" << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned int)byte);

            if (i < bytes.size()-1)
            { byteStream << ", "; }
        }
        byteStream << " }}";

        mainStream << fmt::format("{{ \"{}\", EmbeddedData({}) }}", normalizedPath, byteStream.str());
        
        // Last one, no comma
        if (i < embedFiles.size()-1)
        { mainStream << ",\n    "; }
    }

    String finalSrc = fmt::format(outputSrc, fmt::arg("mapName", embedMapName), fmt::arg("mapVars", mainStream.str()));

    if (fs::exists(outPath) && !overwrite)
    {
        spdlog::error("Output path '{}' already exists. Use -ow flag to overwrite output.", outPath);
        return 1;
    }

    writeToFile(outPath, finalSrc);

    return 0;
}