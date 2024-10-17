cmake_minimum_required(VERSION 3.26)

include(cmake/modules.cmake)
include(GitVersion)

project(ext
    VERSION ${GIT_VERSION_ABBREV}
    DESCRIPTION "Helper tools and functions for C++ programs"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)

include(ProjectTesting)
include(packages.cmake)

add_library(ext "")
add_library(ext::ext ALIAS ext)

target_sources(ext PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_link_libraries(ext PRIVATE fmt::fmt)

if(PROJECT_TESTING)
    add_executable(ext.test "")

    target_link_libraries(ext.test PRIVATE
        ext
        fmt::fmt
        GTest::gtest_main
    )

    add_test("Unit Tests" ext.test)
endif()

add_subdirectory(include)
add_subdirectory(src)

include(Sanitizers)
enable_asan()

include(EasyInstall)
easy_install(TARGETS ext)

if(PROJECT_IS_TOP_LEVEL)
    add_custom_target(docker
        docker build --progress=plain --no-cache .
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        USES_TERMINAL
    )
endif()
