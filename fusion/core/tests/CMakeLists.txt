add_executable(fusion_tests test.cxx)
find_package(Catch2 3 REQUIRED)
target_link_libraries(fusion_tests PRIVATE Catch2::Catch2WithMain core)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(fusion_tests)
