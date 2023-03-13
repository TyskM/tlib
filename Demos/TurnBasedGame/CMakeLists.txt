
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}          /MTd")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}     /MT")

get_filename_component(PARENT_DIR ../../ ABSOLUTE)
set(TLib_DIR ${PARENT_DIR})
find_package(TLib REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.hpp *.h)

add_executable(TurnBasedGame ${SOURCE_FILES})
target_include_directories(TurnBasedGame PUBLIC TLib::TLib)
target_link_libraries(TurnBasedGame PUBLIC TLib::TLib)

find_package(libtcod CONFIG REQUIRED)
target_link_libraries(TurnBasedGame PRIVATE libtcod::libtcod)

add_custom_target(TurnBasedGameAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets )
add_dependencies(TurnBasedGame TurnBasedGameAssets)

set(CPACK_OUTPUT_FILE_PREFIX "")
set(CPACK_GENERATOR "ZIP" CACHE STRING "Generators to support. semi-colon delimited list")
include(CPack)