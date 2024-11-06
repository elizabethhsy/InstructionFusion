cmake_minimum_required(VERSION 3.10)
project(fusion)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "src/*.cxx")
add_executable(${PROJECT_NAME} ${SOURCES})

find_package(fmt REQUIRED)
find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt ${Boost_LIBRARIES})

target_include_directories(
    ${PROJECT_NAME} PRIVATE
    include
    ${Boost_INCLUDE_DIRS}
)
