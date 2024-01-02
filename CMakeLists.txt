cmake_minimum_required(VERSION 3.24)
project(NetSim)


set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wconversion)

include_directories(
        include
)

set(SOURCE_FILES
        src/package.cpp
        src/storage_types.cpp
        src/nodes.cpp
        src/helpers.cpp
        src/factory.cpp
        src/reports.cpp
        src/simulation.cpp
        )

set(SOURCES_FILES_TESTS
        #test/main_gtest.cpp
        #test/test_Factory.cpp
        #test/test_factory_io.cpp
        #test/test_nodes.cpp
        #test/test_package.cpp
        test/test_reports.cpp
        test/test_simulate.cpp
        #test/test_storage_types.cpp
        )

add_executable(NetSim_packages ${SOURCE_FILES} ${SOURCES_FILES_TESTS} test/main_gtest.cpp)

target_include_directories(NetSim_packages PUBLIC
        googletest-master/googlemock/include
        googletest-master/googletest/include
        mocks
        )

add_subdirectory(googletest-master)

target_link_libraries(NetSim_packages gmock)

target_compile_definitions(NetSim_packages PUBLIC EXERCISE_ID=5)