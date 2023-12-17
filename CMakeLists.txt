cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Guccigedon VERSION 0.1)

include(${CMAKE_CURRENT_SOURCE_DIR}/includes.cmake)
add_executable(Guccigedon
    src/main.cpp
    ${GUCCIGEDON_TRANSLATION_UNITS}
)


add_subdirectory(experiments)
find_package(Vulkan REQUIRED)
#find_package(spirv-reflect)
find_package(SDL2)
find_package(VulkanMemoryAllocator CONFIG)
find_package(glm)
find_package(tinygltf) # I eventually want to just have gltf support
find_package(tinyobjloader) # but for now obj is fine. I need to get some meshes up.
find_package(vk-bootstrap)
find_package(stb)
find_package(KTX-Software)

#if(NOT spirv-reflect_FOUND)
#    message("Cannot find SPIRV-Reflect, using bundled version")
#    execute_process(COMMAND git submodule update --init -- vendor/SPIRV-Reflect
#                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#                    set(SPIRV_REFLECT_EXECUTABLE OFF)
#                    set(SPIRV_REFLECT_STATIC_LIB ON)
#    add_subdirectory(vendor/SPIRV-Reflect)
#    set(SPIRV_REFLECT_INCLUDE_DIR vendor/SPIRV-Reflect)
#    set(SPIRV_REFLECT_LIBRARY spirv-reflect-static)
#endif()

if(NOT SDL2_FOUND)
    message("Cannot find SDL2, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/SDL2
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    add_subdirectory(vendor/SDL2)
    #set(SDL2_INCLUDE_DIRS vendor/SDL2/include)
    #set(SDL2_LIBRARIES SDL2)
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
    set(TINYGLTF_INCLUDE_DIR vendor/tinygltf/tiny_gltf.h)
    set(TINYGLTF_LIBRARY tinygltf)
endif()

if(NOT tinyobjloader_FOUND)
    message("Cannot find tinyobj, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/tinyobj
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(vendor/tinyobj)
    set(TINYOBJLOADER_INCLUDE_DIR vendor/tinyobj/tiny_obj_loader.h)
    set(TINYOBJLOADER_LIBRARY tinyobjloader)
endif()


if(NOT vk-bootstrap_FOUND)
    message("Cannot find vk-bootstrap, using bundled version")
    execute_process(COMMAND git submodule update --init -- vendor/vk-bootstrap
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# this stuff is temp, just wanna speed things up a little
    add_subdirectory(vendor/vk-bootstrap)
    set(VK_BOOTSTRAP_INCLUDE_DIR vendor/vk-bootstrap/src)
    set(VK_BOOTSTRAP_LIBRARY vk-bootstrap::vk-bootstrap)
endif()

if(NOT stb_FOUND)
    message("Cannot find stb, using bundled version")
# stb_image stuff
    execute_process(COMMAND git submodule update --init -- vendor/stb
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(STB_INCLUDE_DIR vendor/stb)
endif()

if(NOT KTX-Software_FOUND)
       message("Cannot find KTX, using bundled version")
       execute_process(COMMAND git submodule update --init -- vendor/KTX-Software
       			       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
       set(KTX_FEATURE_TESTS OFF)
       add_subdirectory(vendor/KTX-Software)
       set(KTX_LIBRARY ktx)
       set(KTX_INCLUDE vendor/KTX-Software/include)
endif()

include_directories(${Vulkan_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    ${TINYGLTF_INCLUDE_DIR}
    ${TINYOBJLOADER_INCLUDE_DIR}
    ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR}
    ${VK_BOOTSTRAP_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${KTX_INCLUDE}
    )

target_include_directories(Guccigedon PUBLIC include)

target_link_libraries(Guccigedon ${SDL2_LIBRARIES}
    SDL2::SDL2
    SDL2::SDL2main
    ${Vulkan_LIBRARY} ${GLM_LIBRARY}
    ${VULKAN_MEMORY_ALLOCATOR_LIBRARY}
    ${TINYGLTF_LIBRARY}
    ${TINYOBJLOADER_LIBRARY}
    ${VK_BOOTSTRAP_LIBRARY}
    ${SPIRV_REFLECT_LIBRARY}
    ${KTX_LIBRARY}
    )

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

### Make logs folder
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/logs")

# Tests
project(Tests VERSION 0.1)
set(GUCCIGEDON_TESTS
	tests/sphere_collider_test.cpp
	tests/aabb_collider_test.cpp
	tests/plane_collider_test.cpp
	tests/physics_engine_test.cpp
)
include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(Tests
    ${GUCCIGEDON_TRANSLATION_UNITS}
    ${GUCCIGEDON_TESTS}
)

target_include_directories(Tests PUBLIC include)
include_directories(${Vulkan_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    ${TINYGLTF_INCLUDE_DIR}
    ${TINYOBJLOADER_INCLUDE_DIR}
    ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR}
    ${VK_BOOTSTRAP_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
    ${SPIRV_REFLECT_INCLUDE_DIR}
    ${KTX_INCLUDE}
    )

target_link_libraries(Tests gtest_main ${SDL2_LIBRARIES}
    SDL2::SDL2
    SDL2::SDL2main
    ${Vulkan_LIBRARY} ${GLM_LIBRARY}
    ${VULKAN_MEMORY_ALLOCATOR_LIBRARY}
    ${TINYGLTF_LIBRARY}
    ${TINYOBJLOADER_LIBRARY}
    ${VK_BOOTSTRAP_LIBRARY}
    ${SPIRV_REFLECT_LIBRARY}
    ${KTX_LIBRARY}
    )
include(GoogleTest)
gtest_discover_tests(Tests)
