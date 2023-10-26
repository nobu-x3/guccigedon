cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Guccigedon VERSION 0.1)

add_executable(Guccigedon
    src/main.cpp
    src/render/vulkan/renderer.cpp
    src/render/vulkan/vulkan_builders.cpp
    src/render/vulkan/vulkan_mesh.cpp
    src/render/vulkan/vulkan_types.cpp
    src/render/vulkan/vulkan_image.cpp
    src/render/vulkan/device.cpp
    src/render/vulkan/instance.cpp
    src/render/vulkan/scene.cpp
    )

find_package(Vulkan REQUIRED)
find_package(SDL2)
find_package(VulkanMemoryAllocator CONFIG)
find_package(glm)
find_package(tinygltf) # I eventually want to just have gltf support
find_package(tinyobjloader) # but for now obj is fine. I need to get some meshes up.


if(NOT SDL2_FOUND)
    message("Cannot find SDL2, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/SDL
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/SDL)
endif()

if (NOT VulkanMemoryAllocator_DIR)
    message("Cannot find VMA, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/VulkanMemoryAllocator
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/VulkanMemoryAllocator SYSTEM)
    set(VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR vendor/VulkanMemoryAllocator/include)
    set(VULKAN_MEMORY_ALLOCATOR_LIBRARY VulkanMemoryAllocator)
endif()

if(NOT glm_FOUND)
    message("Cannot find glm, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/glm
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/glm)
    set(GLM_INCLUDE_DIR vendor/glm)
    set(GLM_LIBRARY glm)
endif()

if(NOT tinygltf_FOUND)
    message("Cannot find tinygltf, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/tinygltf
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/tinygltf SYSTEM)
    set(TINYGLTF_INCLUDE_DIR vendor/tinygltf/tinygltf.h)
    set(TINYOBJLOADER_LIBRARY tinyobjloader)
endif()

if(NOT tinyobjloader_FOUND)
    message("Cannot find tinyobj, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/tinyobj
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/tinyobj)
    set(TINYOBJLOADER_INCLUDE_DIR vendor/tinyobj/tiny_obj_loader.h)
endif()



# this stuff is temp, just wanna speed things up a little
add_subdirectory(vendor/vk-bootstrap)

# stb_image stuff
execute_process(COMMAND git submodule update --init -- vendor/stb
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${Vulkan_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR} ${TINYGLTF_INCLUDE_DIR}
    ${TINYOBJLOADER_INCLUDE_DIR}
    ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR} vendor/vk-bootstrap/src vendor/stb)

target_include_directories(Guccigedon PUBLIC include)

target_link_libraries(Guccigedon ${SDL2_LIBRARIES}
    ${Vulkan_LIBRARY} ${GLM_LIBRARY}
    ${VULKAN_MEMORY_ALLOCATOR_LIBRARY}
    ${TINYOBJLOADER_LIBRARY}
    vk-bootstrap::vk-bootstrap)

find_program(GLSL_VALIDATOR glslangValidator)

include(CMakePrintHelpers)
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message("Listing dependencies' include dirs:")
foreach(dir ${dirs})
    cmake_print_variables(dir)
endforeach()

### Copy assets to bin

# Copy files
macro(resource_files files dest)
    foreach(file ${files})
        message(STATUS "Copying resource ${file}")
        file(COPY ${file} DESTINATION ${dest})
    endforeach()
endmacro()

# Copy directories
macro(resource_dirs dirs)
    foreach(dir ${dirs})
        # Replace / at the end of the path (copy dir content VS copy dir)
        string(REGEX REPLACE "/+$" "" dirclean "${dir}")
        message(STATUS "Copying resource ${dirclean}")
        file(COPY ${dirclean} DESTINATION ${CMAKE_BINARY_DIR})
    endforeach()
endmacro()
resource_dirs(assets)

### Compile shaders
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
