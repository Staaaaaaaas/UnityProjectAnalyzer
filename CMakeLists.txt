cmake_minimum_required(VERSION 3.26)
project(UnityProjectAnalyzer)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(yaml-cpp)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(
        UnityProjectAnalyzer
        yaml-cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME tool)
