cmake_minimum_required(VERSION 3.10)
project(fusion)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})