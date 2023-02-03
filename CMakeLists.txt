cmake_minimum_required(VERSION 3.0.0)
project(TLib VERSION 0.1.0)

include(CTest)
enable_testing()
add_subdirectory("Tests")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.hpp *.h)

add_library(TLib STATIC "TLib.cpp" ${SOURCE_FILES})
add_library(TLib::TLib ALIAS TLib)
target_compile_features(TLib PUBLIC cxx_std_20)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

target_include_directories(TLib PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:TLib>  # <prefix>/TLib
)

include(CMakeFindDependencyMacro)

#### Dependencies
### Core stuff

find_dependency(EASTL CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC EASTL)

find_dependency(spdlog CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

find_dependency(magic_enum CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC magic_enum::magic_enum)

find_dependency(sol2 CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC sol2)

find_dependency(PalSigslot CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC Pal::Sigslot)

### Graphics stuff

find_dependency(SDL2 CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)

find_dependency(sdl2-gfx CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC SDL2::SDL2_gfx)

find_dependency(gl3w CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC unofficial::gl3w::gl3w)

find_dependency(glm CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC glm::glm)

find_dependency(imgui CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC imgui::imgui)

install(TARGETS TLib EXPORT TLib-targets)
# install package
install(EXPORT TLib-targets
    FILE TLib-targets.cmake
    NAMESPACE TLib::
    DESTINATION share/cmake/TLib
)
# export build tree
export(
    EXPORT TLib-targets
    FILE "${CMAKE_CURRENT_LIST_DIR}/TLib-targets.cmake"
    NAMESPACE TLib::
)
