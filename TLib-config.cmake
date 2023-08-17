
include(CMakeFindDependencyMacro)
find_dependency(EASTL CONFIG REQUIRED)
find_dependency(spdlog CONFIG REQUIRED)
find_dependency(magic_enum CONFIG REQUIRED)
find_dependency(SDL2 CONFIG REQUIRED)
find_dependency(sdl2-gfx CONFIG REQUIRED)
find_dependency(gl3w CONFIG REQUIRED)
find_dependency(glm CONFIG REQUIRED)
find_dependency(imgui CONFIG REQUIRED)
find_dependency(PalSigslot CONFIG REQUIRED)
find_dependency(mimalloc CONFIG REQUIRED)
find_dependency(Boost REQUIRED COMPONENTS container)
find_dependency(cereal CONFIG REQUIRED)
find_dependency(assimp CONFIG REQUIRED)
find_dependency(CMakeRC CONFIG REQUIRED)

#set(pmp_DIR "${TLib_DIR}/TLib/thirdparty/pmp/build")
#find_dependency(pmp CONFIG REQUIRED)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/TLib-targets.cmake")
   include("${CMAKE_CURRENT_LIST_DIR}/TLib-targets.cmake")
endif()

