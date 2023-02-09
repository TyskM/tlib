
# TODO: More tests!!!!
set(TLib_Tests
        FunkyTriangle
        Indices
        FunkyTriangle3D
        Water
        SpriteRenderer
)

enable_testing()
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}          /MTd")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}     /MT")

get_filename_component(PARENT_DIR ../ ABSOLUTE)
set(TLib_DIR ${PARENT_DIR})
find_package(TLib REQUIRED)

foreach(test IN LISTS TLib_Tests)
    add_executable(${test} "${test}.cpp" "Common.hpp")
    target_include_directories(${test} PUBLIC TLib::TLib)
    target_link_libraries(${test} PUBLIC TLib::TLib)
    add_test(NAME ${test} COMMAND ${test} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach()


add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
