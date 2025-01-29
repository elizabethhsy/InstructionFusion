cmake_minimum_required(VERSION 3.10)
project(common VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(tests)

file(GLOB SOURCES "src/*.cxx")
file(GLOB HEADERS
    "include/*.h"
    ${CMAKE_CURRENT_BINARY_DIR}/fusion_core_export.h
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/fusion>
    ${Boost_INCLUDE_DIRS}
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
    BASE_NAME fusion_common
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/fusion_common_export.h
)

find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt::fmt ${Boost_LIBRARIES}
)
