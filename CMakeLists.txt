cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo)

set(CMAKE_CXX_STANDARD 20)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# Automatically gather all cpp files in src and lib/imgui
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/RaycastResult.h"
    "${CMAKE_SOURCE_DIR}/lib/*.cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp"
)

add_executable(OpenGLDemo ${SRC_FILES})

# Include paths
target_include_directories(OpenGLDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${OPENGL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(OpenGLDemo PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
)

# Copy assets after build
add_custom_command(
    TARGET OpenGLDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:OpenGLDemo>/assets
)

add_custom_command(
    TARGET OpenGLDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/shader $<TARGET_FILE_DIR:OpenGLDemo>/shader
)

enable_testing()

# Optional: test executable
add_executable(compilation_test tests/test_project_compiles.cpp)
add_test(NAME CompilationTest COMMAND compilation_test)
