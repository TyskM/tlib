
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(TLib STATIC IMPORTED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.hpp *.h)

add_executable(Asteroids ${SOURCE_FILES})
target_include_directories(Asteroids PUBLIC TLib::TLib)
target_link_libraries(Asteroids PUBLIC TLib::TLib)

add_custom_target(AsteroidsAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets )
add_dependencies(Asteroids AsteroidsAssets)
