cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo)

set(CMAKE_CXX_STANDARD 20)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(OpenGLDemo
    src/main.cpp
    src/Shader.cpp
    src/Chunk.cpp
    src/World.cpp
    lib/stb_image.cpp
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_impl_glfw.cpp
    lib/imgui/imgui_impl_opengl3.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
)

# Include-Pfade
target_include_directories(OpenGLDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${OPENGL_INCLUDE_DIR}
)

# Link-Bibliotheken
target_link_libraries(OpenGLDemo PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
)

enable_testing()

add_executable(compilation_test tests/test_project_compiles.cpp)

add_test(NAME CompilationTest COMMAND compilation_test)
