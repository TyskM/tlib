#pragma once

#include <TLib/Logging.hpp>
#include <TLib/Pointers.hpp>

// Scripting engine logging sink
static inline SharedPtr<tlog::logger> selog = tlog::createConsoleLogger("AngelScript");
