#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace tlog
{
using namespace spdlog;

std::shared_ptr<logger> createConsoleLogger(const char* name)
{
    std::shared_ptr<logger> console = spdlog::stdout_color_mt(name);
    console->set_pattern("[%H:%M:%S] [%^%l%$] [%n] [thread %t] %v");
    return console;
}

static int init() { spdlog::set_default_logger(createConsoleLogger("Main")); return 0; }
static int d = init();
// lmao this works
}

