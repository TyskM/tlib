
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
)

enable_testing()
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}          /MTd")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}     /MT")

get_filename_component(PARENT_DIR ../ ABSOLUTE)
set(TLib_DIR ${PARENT_DIR})
find_package(TLib REQUIRED)

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


