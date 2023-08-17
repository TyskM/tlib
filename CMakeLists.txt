cmake_minimum_required(VERSION 3.0.0)
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

add_library(TLib STATIC "TLib.cpp" ${SOURCE_FILES})
add_library(TLib::TLib ALIAS TLib)
target_compile_features(TLib PUBLIC cxx_std_20)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
include(FetchContent)

set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}          /MTd")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}     /MT")

message("Build Type: ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

target_include_directories(TLib PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:TLib> # <prefix>/TLib
)

### Setup CMRC for embedding files
find_package(CMakeRC)
# Add embeded files here
file(GLOB_RECURSE EMBED_FILES CONFIGURE_DEPENDS TLib/Embed/Shaders/*.vert TLib/Embed/Shaders/*.frag) # Shader files
cmrc_add_resource_library(embed ${EMBED_FILES})
target_link_libraries(TLib PRIVATE embed)
target_include_directories(TLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/_cmrc/include>
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

# Polygon Mesh Processing Library (pmp-library)
# https://github.com/pmp-library/pmp-library
# set(pmp_DIR "TLib/thirdparty/pmp/build")
# find_dependency(pmp CONFIG REQUIRED)
# target_link_libraries(TLib PUBLIC pmp)

##
#### Make package

install(TARGETS TLib embed cmrc-base EXPORT TLib-targets)
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
