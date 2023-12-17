cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
project(Tests VERSION 0.1)

set(GUCCIGEDON_TESTS
	sphere_collider_test.cpp
	aabb_collider_test.cpp
	plane_collider_test.cpp
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../includes.cmake)
foreach(TRUNIT ${GUCCIGEDON_TO_TEST})
    set(UNIT ${CMAKE_CURRENT_SOURCE_DIR}/../${TRUNIT})
    list(APPEND GUCCIGEDON_SOURCE ${UNIT})
endforeach(TRUNIT)

# Tests
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
    ${GUCCIGEDON_SOURCE}
    ${GUCCIGEDON_TESTS}
)

include(CMakePrintHelpers)
message("Translation units")
foreach(dir ${GUCCIGEDON_SOURCE})
    cmake_print_variables(dir)
endforeach()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glm glm)
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glm)
set(GLM_LIBRARY glm)

target_include_directories(Tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${GLM_INCLUDE_DIR})
target_link_libraries(Tests gtest_main ${GLM_LIBRARY})
include(GoogleTest)
gtest_discover_tests(Tests)
