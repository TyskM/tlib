
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(TLib STATIC IMPORTED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.hpp *.h)

add_executable(Pong ${SOURCE_FILES})
target_include_directories(Pong PUBLIC TLib::TLib)
target_link_libraries(Pong PUBLIC TLib::TLib)

add_custom_target(PongAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets )
add_dependencies(Pong PongAssets)
