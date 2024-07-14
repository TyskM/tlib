
#include <CLI/App.hpp>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <fmt/args.h>

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
    set_default_logger(createConsoleLogger("Embedder"));
    spdlog::info("Working dir: {}", fs::current_path().string());

    #ifdef TLIB_DEBUG
        tlog::info("Arg count: {}", argc);
        for (int i = 0; i < argc; i++)
        { tlog::info("Arg {}: {}", i, argv[i]); }
    #endif

    std::vector<String> embedFiles;
    String              outPath       = "";
    String              embedMapName  = "myEmbeds";
    String              timestampPath = "{outPathDir}/time.stamp";
    bool                overwrite     = false;

    // Parse Args
    {
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
    
        app.add_option(
            "-t,--timeStamp",
            timestampPath,
            "Path to timestamp")
            ->default_val(timestampPath);

        app.add_flag(
            "--ow",
            overwrite,
            "Overwrite output (You probably want this but it's false by default to be safe)")
            ->default_val(overwrite);

        CLI11_PARSE(app, argc, argv);
    }

    // Format parsed args
    {
        using FormatArgs = fmt::dynamic_format_arg_store<fmt::format_context>;
        FormatArgs args;
        args.push_back(fmt::arg("outPath", outPath));
        args.push_back(fmt::arg("outPathDir", Path(outPath).remove_filename().string()));

        timestampPath = fmt::vformat(timestampPath, args);
    }

    // Normalize file paths
    {
        for (auto& filePath : embedFiles)
        {
            filePath = fs::path(filePath).lexically_normal().string();
            replace(filePath, "\\", "/");
        }
    }

    // Get or create embed timestamp
    fs::file_time_type lastEmbedTimeStamp;
    {
        bool timestampExists = fs::exists(timestampPath);
        if (!timestampExists)
        {
            spdlog::info("No embed timestamp found, creating one...");
            if (!writeToFile(timestampPath, ""))
            { spdlog::error("Failed to write timestamp '{}'", timestampPath); }
        }
        lastEmbedTimeStamp = fs::last_write_time(timestampPath);

        // Set new timestamp
        fs::last_write_time(timestampPath, std::chrono::file_clock::now());
    }

    // Check if files are dirty
    bool filesAreDirty = false;
    {
        for (auto& file : embedFiles)
        {
            auto ts = fs::last_write_time(file);
            filesAreDirty = (ts > lastEmbedTimeStamp);
            if (filesAreDirty)
            {
                spdlog::info("Embedder source file '{}' is dirty, reembedding all source files...", file);
                break;
            }
        }
    }

    // If no files have been changed, do nothing. Happy common case :)
    if (!filesAreDirty)
    {
        spdlog::info("Files are all clean, doing nothing :)");
        return 0;
    }

    // Embed provided source files
    {
        std::stringstream mainStream;
        spdlog::info("Got files: ");

        for (size_t i = 0; i < embedFiles.size(); i++)
        {
            const String& embedFile = embedFiles[i];
            spdlog::info(embedFile);

            // Get value from file
            std::vector<char> bytes;
            bytes = readFileBytes(embedFile);
            if (bytes.empty())
            { spdlog::error("Failed to read file: {}\n Exiting...", embedFile); return 1; }
            
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

            mainStream << fmt::format("{{ \"{}\", EmbeddedData({}) }}", embedFile, byteStream.str());
            
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
}