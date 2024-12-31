cmake_minimum_required(VERSION 3.10)
project(fusion)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "src/*.cxx")
add_library(${PROJECT_NAME} ${SOURCES})

find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt::fmt ${Boost_LIBRARIES}
)

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    include
    ${Boost_INCLUDE_DIRS}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(exec)
add_subdirectory(tests)
