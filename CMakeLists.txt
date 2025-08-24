cmake_minimum_required(VERSION 4.0)
project(project_name LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

include_directories(${project_name_SOURCE_DIR}/include)

add_executable(main
    src/main.cpp
    # src/Basket.cpp
)

target_compile_features(main PRIVATE cxx_std_17)

target_link_libraries(main PRIVATE SDL3::SDL3)
