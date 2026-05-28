cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo)

set(CMAKE_CXX_STANDARD 20)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

add_executable(OpenGLDemo
        src/main.cpp
        src/engine/Camera.cpp
        src/engine/DebugWindow.cpp
        src/engine/Input.cpp
        src/engine/Mesh.cpp
        src/engine/MeshFactory.cpp
        src/engine/Model.cpp
        src/engine/Shader.cpp
        src/engine/Texture.cpp
        src/world/Chunk.cpp
        src/world/World.cpp
        lib/stb_image.cpp
        lib/imgui/imgui.cpp
        lib/imgui/imgui_demo.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_impl_glfw.cpp
        lib/imgui/imgui_impl_opengl3.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        src/engine/Time.cpp
        src/engine/Time.h
)

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
    assimp::assimp
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
