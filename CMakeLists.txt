cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB SOURCES "src/*.cpp" "lib/*.cpp")

add_executable(OpenGLDemo ${SOURCES})

# Include-Pfade
target_include_directories(OpenGLDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
    ${OPENGL_INCLUDE_DIR}
)

# Link-Bibliotheken
target_link_libraries(OpenGLDemo PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
)
