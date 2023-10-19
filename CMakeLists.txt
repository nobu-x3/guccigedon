cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Guccigedon VERSION 0.1)
add_executable(Guccigedon
    src/main.cpp
    src/render/renderer.cpp
    src/render/vulkan_builders.cpp)
find_package(Vulkan REQUIRED)
find_package(SDL2)

if(NOT SDL2_FOUND)
    message("Cannot find SDL2")
    execute_process(COMMAND git submodule update --init -- vendor/SDL
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/SDL)
endif()

# this stuff is temp, just wanna speed things up a little
add_subdirectory(vendor/vk-bootstrap)

include_directories(${Vulkan_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} vendor/vk-bootstrap/src)
target_include_directories(Guccigedon PUBLIC)
target_link_libraries(Guccigedon ${SDL2_LIBRARIES} ${Vulkan_LIBRARY} vk-bootstrap::vk-bootstrap)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
