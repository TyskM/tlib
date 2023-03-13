
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}          /MTd")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}     /MT")

get_filename_component(PARENT_DIR ../../ ABSOLUTE)
set(TLib_DIR ${PARENT_DIR})
find_package(TLib REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.hpp *.h)



add_executable(Asteroids ${SOURCE_FILES})
target_include_directories(Asteroids PUBLIC TLib::TLib)
target_link_libraries(Asteroids PUBLIC TLib::TLib)

add_custom_target(AsteroidsAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets )
add_dependencies(Asteroids AsteroidsAssets)

set(CPACK_OUTPUT_FILE_PREFIX "")
set(CPACK_GENERATOR "ZIP" CACHE STRING "Generators to support. semi-colon delimited list")
include(CPack)