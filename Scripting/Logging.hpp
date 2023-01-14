#pragma once

#include "../Logging.hpp"

// Scripting engine logging sink
static inline std::shared_ptr<tlog::logger> selog = tlog::createConsoleLogger("SE");
