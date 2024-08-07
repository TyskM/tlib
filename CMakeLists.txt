
#[[
    vcpkg dependencies:
    core: spdlog magic-enum palsigslot freetype mimalloc cereal boost-algorithm boost-container SDL2 sdl2-gfx gl3w glm imgui imgui[docking-experimental] imgui[sdl2-binding] imgui[opengl3-binding] assimp
    tests: sfml
]]

cmake_minimum_required(VERSION 3.15)
project(TLib VERSION 0.1.0)

include(CTest)
enable_testing()
add_subdirectory("Tests")
add_subdirectory("Demos")

# Like file(glob) but appends, not overwrites
macro(GLOB_APPEND out pattern)
    file (GLOB temp LIST_DIRECTORIES false CONFIGURE_DEPENDS ${pattern})
    if(NOT DEFINED ${out})
        set(${out} "")
    endif()
    set(${out} "${${out}};${temp}") # lmao this works
endmacro()

macro(GLOB_RECURSE_APPEND out pattern)
    file (GLOB_RECURSE temp CONFIGURE_DEPENDS ${pattern})
    if(NOT DEFINED ${out})
        set(${out} "")
    endif()
    set(${out} "${${out}};${temp}")
endmacro()

GLOB_APPEND        (SOURCE_FILES "TLib/*.cpp;TLib/*.hpp;TLib/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/Containers/*.hpp")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/ECS/*.hpp")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/Game/*.hpp")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/Media/*.hpp")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/Scripting/*.hpp")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/Types/*.hpp")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/Embed/*.hpp")

# Third party
GLOB_APPEND        (SOURCE_FILES "TLib/thirdparty/*.cpp;TLib/thirdparty/*.hpp;TLib/thirdparty/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/thirdparty/angelscript/*.hpp;TLib/thirdparty/angelscript/*.cpp;TLib/thirdparty/angelscript/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/thirdparty/angelscript-docgen/*.hpp;TLib/thirdparty/angelscript-docgen/*.cpp;TLib/thirdparty/angelscript-docgen/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/thirdparty/cute_headers/*.hpp;TLib/thirdparty/cute_headers/*.cpp;TLib/thirdparty/cute_headers/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/thirdparty/multiarray/*.hpp;TLib/thirdparty/multiarray/*.cpp;TLib/thirdparty/multiarray/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/thirdparty/plf/*.hpp;TLib/thirdparty/plf/*.cpp;TLib/thirdparty/plf/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/thirdparty/rectpack2d/*.hpp;TLib/thirdparty/rectpack2d/*.cpp;TLib/thirdparty/rectpack2d/*.h")
GLOB_RECURSE_APPEND(SOURCE_FILES "TLib/thirdparty/ImGuiColorTextEdit/*.hpp;TLib/thirdparty/ImGuiColorTextEdit/*.cpp;TLib/thirdparty/ImGuiColorTextEdit/*.h")

# AngelScript
if(MSVC AND CMAKE_CL_64)
    enable_language(ASM_MASM)
    set(SOURCE_FILES "${SOURCE_FILES};TLib/thirdparty/angelscript/as_callfunc_x64_msvc_asm.asm")
endif()

message("Found source files: ")
foreach(src IN ITEMS ${SOURCE_FILES})
    message("${src}")
endforeach()

# Remove PMP sources
#file(GLOB_RECURSE PMP_FILES CONFIGURE_DEPENDS
#TLib/thirdparty/pmp/*.cpp TLib/thirdparty/pmp/*.hpp TLib/thirdparty/pmp/*.h)
#list(REMOVE_ITEM SOURCE_FILES ${PMP_FILES})

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(TLib STATIC ${SOURCE_FILES} "TLib/thirdparty/zep/config_app.h")
add_library(TLib::TLib ALIAS TLib)
target_compile_features(TLib PUBLIC cxx_std_20)

add_executable(TLibTest "TLib/main.cpp" "TLib/thirdparty/zep/config_app.h")
target_link_libraries(TLibTest PUBLIC TLib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
include(FetchContent)

message("Triplet:      ${VCPKG_TARGET_TRIPLET}")
message("Build Type:   ${CMAKE_BUILD_TYPE}")
message("Shared Build: ${BUILD_SHARED_LIBS}")
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("ASan:         Enabled")
    add_compile_definitions(_DISABLE_STRING_ANNOTATION=1 _DISABLE_VECTOR_ANNOTATION=1)
    add_compile_options(-fsanitize=address)
else()
    message("ASan:         Disabled")
endif()

target_include_directories(TLib PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:TLib> # <prefix>/TLib
)

# TODO: Disable warning if clang-tidy enabled
# Show all warnings
add_compile_options(/W4)
#add_definitions(-w)

# Enable clang-tidy
# 1. Get LLVM installer exe from here: https://releases.llvm.org/download.html
# 2. Install, restart VS, clear cache and done
#find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
#set(CMAKE_CXX_CLANG_TIDY clang-tidy)
#set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "--extra-arg=/EHa;--header-filter=.*;--checks=clang-analyzer-*,performance-*")
#set_target_properties(TLib PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")

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

find_package(tinyfiledialogs CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC tinyfiledialogs::tinyfiledialogs)

target_compile_definitions(TLib PUBLIC PX_PHYSX_STATIC_LIB)
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)
target_link_libraries(TLib PUBLIC unofficial::omniverse-physx-sdk::sdk)

find_package(Bullet CONFIG REQUIRED)
target_link_libraries(TLib PRIVATE ${BULLET_LIBRARIES})

find_package(flecs CONFIG REQUIRED)
target_link_libraries(TLib PRIVATE $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>)

find_package(unofficial-enet CONFIG REQUIRED)
target_link_libraries(TLib PRIVATE unofficial::enet::enet)

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
set(EmbedderSources "Embedder/embedder.cpp")
add_executable(Embedder ${EmbedderSources})
target_compile_features(Embedder PUBLIC cxx_std_20)
set_target_properties(Embedder PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")

find_dependency(CLI11 CONFIG REQUIRED)
target_link_libraries(Embedder PUBLIC CLI11::CLI11)
target_link_libraries(Embedder PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

enable_testing()
add_test(NAME EmbedderTest COMMAND "Embedder -f Embedder/test/2d.frag Embedder/test/2d.vert -o embed.hpp --ow")

# Embedder tests
add_custom_target(EmbedTestAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Embedder/test ${CMAKE_CURRENT_BINARY_DIR}/Embedder/test )
add_dependencies(Embedder EmbedTestAssets)

# Copy Embedder
add_custom_target(CopyEmbedder
COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/Embedder.exe ${CMAKE_CURRENT_LIST_DIR}
DEPENDS ${EmbedderSources})
add_dependencies(CopyEmbedder Embedder)

set(EMBEDDED_FILES
"TLib/Embed/Shaders/sdf_text.frag"
"TLib/Embed/Shaders/2d.frag"
"TLib/Embed/Shaders/2d.vert"
"TLib/Embed/Shaders/3d.vert"
"TLib/Embed/Shaders/3d.frag"
"TLib/Embed/Shaders/light.vert"
"TLib/Embed/Shaders/empty.frag"
"TLib/Embed/Shaders/3d_primitive.vert"
"TLib/Embed/Shaders/3d_primitive.frag"
"TLib/Embed/Shaders/csm.vert")

# Run Embedder
set(EmbedderCommand ${CMAKE_CURRENT_LIST_DIR}/Embedder.exe -f ${EMBEDDED_FILES} -o "TLib/Embed/Embed.hpp" --ow)
add_custom_target(RunEmbedder WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
COMMAND ${EmbedderCommand}
DEPENDS ${EMBEDDED_FILES})
add_dependencies(RunEmbedder CopyEmbedder)
add_dependencies(TLib RunEmbedder)

# I don't understand the overcomplicated packaging BS
# Link TLib like this:
# add_subdirectory("D:/Resources/Dev/CPP/TLib" "sub/tlib")
# target_link_libraries(main PUBLIC TLib::TLib)
