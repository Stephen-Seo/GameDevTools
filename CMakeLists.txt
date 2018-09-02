cmake_minimum_required(VERSION 3.0)

project(GameDevTools VERSION 1.4)

set(GameDevTools_SOURCES
    src/GDT/Internal/NetworkIdentifiers.cpp
    src/GDT/GameLoop.cpp
    src/GDT/NetworkConnection.cpp
    src/GDT/SceneNode.cpp
    src/GDT/CollisionDetection.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D NDEBUG")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug', none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

add_library(GameDevTools ${GameDevTools_SOURCES})

set_target_properties(GameDevTools
    PROPERTIES
    VERSION ${GameDevTools_VERSION}
    SOVERSION ${GameDevTools_VERSION}
)

target_compile_features(GameDevTools PUBLIC cxx_std_11)

if(WIN32)
    target_link_libraries(GameDevTools ws2_32)
endif()

find_package(GTest)

if(GTEST_FOUND)
    set(UnitTests_SOURCES
        src/test/Main.cpp
        src/test/TestGameLoop.cpp
        src/test/TestSceneNode.cpp
        src/test/TestCollisionDetection.cpp
    )

    add_executable(UnitTests ${UnitTests_SOURCES})
    target_link_libraries(UnitTests GameDevTools ${GTEST_BOTH_LIBRARIES})

endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(NetworkingTest_SOURCES
        src/networkingTest/Main.cpp
    )

    add_executable(NetworkingTest ${NetworkingTest_SOURCES})
    target_link_libraries(NetworkingTest GameDevTools)
endif()

install(TARGETS GameDevTools
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY src/GDT
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

