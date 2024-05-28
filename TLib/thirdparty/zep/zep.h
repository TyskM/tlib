#pragma once

#include "editor.h"
#include "syntax.h"
#include "buffer.h"
#include "tab_window.h"
#include "mode_vim.h"
#include "mode_standard.h"
#include "window.h"
#include "mode.h"
#include "mode_vim.h"
#include "mode_standard.h"
#ifdef ZEP_QT
#include "qt/display_qt.h"
#include "qt/editor_qt.h"
#else
#include "imgui/display_imgui.h"
#include "imgui/editor_imgui.h"
#include "imgui/console_imgui.h"
#endif

