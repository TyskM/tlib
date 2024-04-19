#pragma once

// Saves 3 seconds of compile time :o
#define SPDLOG_COMPILED_LIB
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "String.hpp"

namespace tlog
{
using namespace spdlog;

static std::shared_ptr<logger> createConsoleLogger(const char* name, const String& pattern = "[%H:%M:%S] [%^%l%$] [%n] [thread %t] %v") noexcept
{
    auto maybeExistingLogger = spdlog::get(name);
    if (maybeExistingLogger)
    { return maybeExistingLogger; }

    std::shared_ptr<logger> console = spdlog::stdout_color_mt(name);
    console->set_pattern(pattern);
    return console;
}

static bool init()
{
    spdlog::set_default_logger(createConsoleLogger("Main"));
    return true;
}
static bool d = init();
// lmao this works
}

