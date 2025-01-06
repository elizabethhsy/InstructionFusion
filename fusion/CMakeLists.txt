cmake_minimum_required(VERSION 3.10)
project(fusion VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add_subdirectory(exec)
add_subdirectory(tests)

INCLUDE(GNUInstallDirs)

file(GLOB SOURCES "src/*.cxx")
file(GLOB HEADERS
    "include/*.h"
    ${CMAKE_CURRENT_BINARY_DIR}/fusion_export.h
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
    BASE_NAME fusion
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/fusion_export.h
)

find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt::fmt ${Boost_LIBRARIES}
)

# create config file
include(CMakePackageConfigHelpers)

configure_package_config_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/fusionConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/fusionConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fusion
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/fusionConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS ${PROJECT_NAME}
    EXPORT fusionTargets
    LIBRARY
            COMPONENT fusion-lib
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER
        COMPONENT fusion-dev
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fusion
)

install(EXPORT fusionTargets
    FILE fusionTargets.cmake
    COMPONENT fusion-dev
    NAMESPACE fusion::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fusion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/fusionConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/fusionConfigVersion.cmake
    COMPONENT fusion-dev
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fusion
)
