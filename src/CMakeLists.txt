# global
cmake_minimum_required(VERSION 3.2)

if(DEFINED ENV{TRAVIS_BUILD_NUMBER})
    project(kkmeans VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
    project(kkmeans VERSION 0.0.1)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message("${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED)
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT madmazoku@gmail.com)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -pthread")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -std=c++11 -pthread")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -std=c++11 -pthread")

include(CPack)

enable_testing()

# local
configure_file(in.version.h version.h)

add_executable(kkmeans "main.cpp")

add_executable(kkmeans_test "test.cpp")

set_target_properties(kkmeans kkmeans_test PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

set_target_properties(kkmeans kkmeans_test PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_STATIC_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

message("linkL ${SDL2_LIBRARY} ${SDL2_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}")

target_link_libraries(kkmeans
    ${SDL2_LIBRARY} ${SDL2_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(kkmeans_test
    ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS kkmeans RUNTIME DESTINATION bin)

add_test(kkmeans kkmeans_test)
