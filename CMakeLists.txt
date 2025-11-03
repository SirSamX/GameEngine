cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo)

set(CMAKE_CXX_STANDARD 20)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/RaycastResult.h"
    "${CMAKE_SOURCE_DIR}/lib/*.cpp"
    "${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp"
)

add_executable(OpenGLDemo ${SRC_FILES})

target_include_directories(OpenGLDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(OpenGLDemo PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
)

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

add_test(NAME OpenGLDemoTest COMMAND OpenGLDemo)
set_tests_properties(OpenGLDemoTest PROPERTIES TIMEOUT 5)
