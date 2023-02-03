
include(CMakeFindDependencyMacro)
find_dependency(EASTL CONFIG REQUIRED)
find_dependency(spdlog CONFIG REQUIRED)
find_dependency(magic_enum CONFIG REQUIRED)
find_dependency(sol2 CONFIG REQUIRED)
find_dependency(SDL2 CONFIG REQUIRED)
find_dependency(sdl2-gfx CONFIG REQUIRED)
find_dependency(gl3w CONFIG REQUIRED)
find_dependency(glm CONFIG REQUIRED)
find_dependency(imgui CONFIG REQUIRED)
find_dependency(PalSigslot CONFIG REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/TLib-targets.cmake")