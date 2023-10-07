
#[[
    vcpkg dependencies:
    core: spdlog magic-enum palsigslot freetype mimalloc cereal boost-algorithm boost-container SDL2 sdl2-gfx gl3w glm imgui imgui[docking-experimental] imgui[sdl2-binding] imgui[opengl3-binding] assimp cmakerc
    tests: sfml
]]

cmake_minimum_required(VERSION 3.15)
project(TLib VERSION 0.1.0)

include(CTest)
enable_testing()
add_subdirectory("Tests")
add_subdirectory("Demos")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS TLib/*.cpp TLib/*.hpp TLib/*.h pmp/*.cpp pmp/*.hpp pmp/*.h)

# Show all warnings
add_compile_options(/W4)

# Remove PMP sources
file(GLOB_RECURSE PMP_FILES CONFIGURE_DEPENDS
TLib/thirdparty/pmp/*.cpp TLib/thirdparty/pmp/*.hpp TLib/thirdparty/pmp/*.h)
list(REMOVE_ITEM SOURCE_FILES ${PMP_FILES})

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(TLib "TLib.cpp" ${SOURCE_FILES})
add_library(TLib::TLib ALIAS TLib)
target_compile_features(TLib PUBLIC cxx_std_20)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
include(FetchContent)

message("Triplet:      ${VCPKG_TARGET_TRIPLET}")
message("Build Type:   ${CMAKE_BUILD_TYPE}")
message("Shared Build: ${BUILD_SHARED_LIBS}")
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

target_include_directories(TLib PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:TLib> # <prefix>/TLib
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

find_dependency(PalSigslot CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC Pal::Sigslot)

find_dependency(mimalloc CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC mimalloc-static)

find_dependency(cereal CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC cereal::cereal)

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)  # link Boost Static libraries
find_dependency(Boost REQUIRED COMPONENTS container)
target_link_libraries(TLib PUBLIC Boost::boost Boost::container)

find_dependency(box2d CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC box2d::box2d)

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

find_dependency(assimp CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC assimp::assimp)

find_dependency(freetype CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC freetype)

# Polygon Mesh Processing Library (pmp-library)
# https://github.com/pmp-library/pmp-library
# set(pmp_DIR "TLib/thirdparty/pmp/build")
# find_dependency(pmp CONFIG REQUIRED)
# target_link_libraries(TLib PUBLIC pmp)

##
#### Embedder
enable_testing()
add_executable(Embedder "Embedder/embedder.cpp")
target_compile_features(Embedder PUBLIC cxx_std_20)

find_dependency(CLI11 CONFIG REQUIRED)
target_link_libraries(Embedder PUBLIC CLI11::CLI11)
target_link_libraries(Embedder PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

add_test(EmbedTest1 Embedder -f "Embedder/test/2d.frag" "Embedder/test/2d.vert" -o "embed.hpp" --ow)

# Embedder tests
add_custom_target(EmbedTestAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Embedder/test ${CMAKE_CURRENT_BINARY_DIR}/Embedder/test )
add_dependencies(Embedder EmbedTestAssets)

# Copy Embedder
add_custom_target(CopyEmbedder COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/Embedder.exe ${CMAKE_CURRENT_LIST_DIR})
add_dependencies(CopyEmbedder Embedder)

# Run Embedder
add_custom_target(RunEmbedder WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} COMMAND ${CMAKE_CURRENT_LIST_DIR}/Embedder.exe
-f
"TLib/Embed/Shaders/sdf_text.frag"
"TLib/Embed/Shaders/2d.frag"
"TLib/Embed/Shaders/2d.vert"
-o "TLib/Embed/Embed.hpp" --ow)
add_dependencies(RunEmbedder CopyEmbedder)
add_dependencies(TLib RunEmbedder)

##
#### Make package

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
