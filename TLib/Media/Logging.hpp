#pragma once

#include <TLib/Logging.hpp>

static constexpr bool verboseRendererLogging = false;

static std::shared_ptr<tlog::logger> rendlog = tlog::createConsoleLogger("Renderer");