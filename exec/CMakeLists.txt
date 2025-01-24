cmake_minimum_required(VERSION 3.10)
project(application)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(rules)
add_executable(experiment main.cxx)
find_package(fusion REQUIRED)
find_package(fmt REQUIRED)
target_link_libraries(experiment PRIVATE fmt::fmt fusion::fusion)
