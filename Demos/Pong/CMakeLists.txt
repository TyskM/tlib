
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(TLib STATIC IMPORTED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.hpp *.h)

add_executable(Pong ${SOURCE_FILES})
target_include_directories(Pong PUBLIC TLib::TLib)
target_link_libraries(Pong PUBLIC TLib::TLib)

# Requirement for RmlUi
find_package(SDL2_image CONFIG REQUIRED)
target_link_libraries(Pong PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
find_package(RmlUi CONFIG REQUIRED)
target_link_libraries(Pong PRIVATE RmlUi::RmlUi)

add_custom_target(PongAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets )
add_dependencies(Pong PongAssets)
