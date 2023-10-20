cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Guccigedon VERSION 0.1)
add_executable(Guccigedon
    src/main.cpp
    src/render/renderer.cpp
    src/render/vulkan_builders.cpp
    src/render/vulkan_mesh.cpp)
find_package(Vulkan REQUIRED)
find_package(SDL2)
find_package(VulkanMemoryAllocator CONFIG)
find_package(glm)

if(NOT SDL2_FOUND)
    message("Cannot find SDL2")
    execute_process(COMMAND git submodule update --init -- vendor/SDL
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/SDL)
endif()

if (NOT VulkanMemoryAllocator_DIR)
  message("Cannot find VMA")
  execute_process(COMMAND git submodule update --init -- vendor/VulkanMemoryAllocator
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(vendor/VulkanMemoryAllocator)
endif()

if(NOT glm_FOUND)
    message("Cannot find glm")
    execute_process(COMMAND git submodule update --init -- vendor/glm
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/glm)
endif()

# this stuff is temp, just wanna speed things up a little
add_subdirectory(vendor/vk-bootstrap)

include_directories(${Vulkan_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${GLM_INCLUDE_DIR}vendor/VulkanMemoryAllocator/include vendor/vk-bootstrap/src)
target_include_directories(Guccigedon PUBLIC)
target_link_libraries(Guccigedon ${SDL2_LIBRARIES} ${Vulkan_LIBRARY} ${GLM_LIBRARY} vk-bootstrap::vk-bootstrap VulkanMemoryAllocator)
# set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

find_program(GLSL_VALIDATOR glslangValidator)


file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.glsl"
    )

foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "assets/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
    )
