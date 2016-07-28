cmake_minimum_required(VERSION 3.2)
project(StringAlgos)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Wno-char-subscripts -Wno-unused-result -g -fsanitize=thread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Wno-char-subscripts -Wno-unused-result -g -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -g")

add_definitions(-D_LINUX)
#add_definitions(-DNDEBUG)
#add_definitions(-D_GLIBCXX_DEBUG)

IF (NOT DEFINED ENV{DEPS_PATH})
    set(ENV{DEPS_PATH} "/home/nemchenko/work/builds/")
ENDIF ()

include_directories("$ENV{DEPS_PATH}/hyperscan/src")

file(GLOB_RECURSE SOURCE_FILES source/*.cpp test/*.cpp)
file(GLOB_RECURSE HEADER_FILES include/*.h  benchmark/*.h resources/*)

IF ("$ENV{BENCHMARK}" STREQUAL "y")
    add_definitions(-DBENCHMARK)
ENDIF ()

include_directories(include benchmark $ENV{PSC_DIR})

IF ("$ENV{GTEST}" STREQUAL "y")
    MESSAGE("!Gtests're enabled!")

    find_package(GTest REQUIRED)

    include_directories(${GTEST_INCLUDE_DIRS})
    add_definitions(-D_GTEST)
ENDIF ()

#find_package(Boost COMPONENTS system REQUIRED)

link_directories("$ENV{DEPS_PATH}/hyperscan/lib")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")
#set(CMAKE_EXE_LINKER_FLAGS  "-static")

IF ("$ENV{GTEST}" STREQUAL "y")
    target_link_libraries(${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} pthread)
ELSE ()
    target_link_libraries(${PROJECT_NAME} pthread)
ENDIF ()
target_link_libraries(${PROJECT_NAME} hs hs_runtime pthread)
