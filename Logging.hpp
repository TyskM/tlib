#pragma once

// Saves 3 seconds of compile time :o
#define SPDLOG_COMPILED_LIB
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "String.hpp"

namespace tlog
{
using namespace spdlog;

static std::shared_ptr<logger> createConsoleLogger(const char* name, String pattern = "[%H:%M:%S] [%^%l%$] [%n] [thread %t] %v")
{
    std::shared_ptr<logger> console = spdlog::stdout_color_mt(name);
    console->set_pattern(pattern);
    return console;
}

static int init() { spdlog::set_default_logger(createConsoleLogger("Main")); return 0; }
static int d = init();
// lmao this works
}

