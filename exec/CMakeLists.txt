add_executable(experiment main.cxx)
target_include_directories(experiment PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(experiment PRIVATE fusion)
