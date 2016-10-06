
project(GameDevTools)

set(GameDevTools_SOURCES
    src/GDT/GameLoop.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D NDEBUG")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug', none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

add_library(GameDevTools ${GameDevTools_SOURCES})

find_package(GTest)

if(GTEST_FOUND)
    set(UnitTests_SOURCES
        src/test/Main.cpp
        src/test/TestGameLoop.cpp
    )

    add_executable(UnitTests ${UnitTests_SOURCES})
    target_link_libraries(UnitTests GameDevTools ${GTEST_BOTH_LIBRARIES})

endif()
