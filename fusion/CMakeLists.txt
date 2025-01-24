cmake_minimum_required(VERSION 3.10)
project(fusion VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(core)
add_subdirectory(simulation)

INCLUDE(GNUInstallDirs)
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

install(TARGETS core simulation
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


install(DIRECTORY core/include/ DESTINATION include/core)
install(DIRECTORY simulation/include/ DESTINATION include/simulation)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/fusionConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/fusionConfigVersion.cmake
    COMPONENT fusion-dev
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fusion
)
