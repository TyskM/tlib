
# TODO: More tests!!!!
# TODO: Add tests for resource copy and move

set(TLib_Tests
        MinimalExample
        FunkyTriangle
        Indices
        FunkyTriangle3D
        Water
        SpriteRenderer
        SFMLTest
        SimpleSpriteTest
        Text
        Model
        #GUI
        Box2D
        NinePatchTest
        Camera2D
        FBOTest
        AStar2D
        Construct
        Scripting
)

enable_testing()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(TLib STATIC IMPORTED)

add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets )

foreach(test IN LISTS TLib_Tests)
    add_executable(${test} "${test}.cpp" "Common.hpp")
    target_include_directories(${test} PUBLIC TLib::TLib)
    target_link_libraries(${test} PUBLIC TLib::TLib)
    add_test(NAME ${test} COMMAND ${test})
    add_dependencies(${test} copy_assets)
endforeach()

# For benchmarks
find_package(SFML CONFIG REQUIRED COMPONENTS system window graphics)
target_link_libraries(SFMLTest PRIVATE sfml-system sfml-graphics sfml-window)


