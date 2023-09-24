# CMake version
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# Functions
function(copy_folder target source dest folder)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${source}/${folder} ${dest}/${folder}
    )
endfunction()

# CMake compiler and linker flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
if(NOT CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
    # -march=native is not supported by arm64
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()
if(WIN32)
    set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec True)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build package with shared libraries.")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif(WIN32)

# External libraries
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "[Assimp] If the supplementary tools for Assimp are built in addition to the library.")
set(ASSIMP_BUILD_TESTS        OFF CACHE BOOL "[Assimp] If the test suite for Assimp is built in addition to the library.")
set(ASSIMP_INSTALL            OFF CACHE BOOL "[Assimp] Disable this if you want to use Assimp as a submodule.")
set(ASSIMP_NO_EXPORT          ON  CACHE BOOL "[Assimp] Disable Assimp's export functionality.")
set(ASSIMP_UBSAN              OFF CACHE BOOL "[Assimp] Enable Undefined Behavior sanitizer.")
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "[Assimp] Treat all warnings as errors.")
set(GLFW_BUILD_DOCS           OFF CACHE BOOL "[GLFW] Build the GLFW documentation")
set(GLFW_BUILD_EXAMPLES       OFF CACHE BOOL "[GLFW] Build the GLFW example programs")
set(GLFW_BUILD_TESTS          OFF CACHE BOOL "[GLFW] Build the GLFW test programs")
add_library(glad ./src/external/glad-core-4.6/include/KHR/khrplatform.h
                 ./src/external/glad-core-4.6/include/glad/glad.h
                 ./src/external/glad-core-4.6/src/glad.c)
add_library(imgui ./src/external/imgui-1.88/imconfig.h
                  ./src/external/imgui-1.88/imgui.cpp
                  ./src/external/imgui-1.88/imgui.h
                  ./src/external/imgui-1.88/imgui_demo.cpp
                  ./src/external/imgui-1.88/imgui_draw.cpp
                  ./src/external/imgui-1.88/imgui_internal.h
                  ./src/external/imgui-1.88/imgui_tables.cpp
                  ./src/external/imgui-1.88/imgui_widgets.cpp
                  ./src/external/imgui-1.88/imstb_rectpack.h
                  ./src/external/imgui-1.88/imstb_textedit.h
                  ./src/external/imgui-1.88/imstb_truetype.h
                  ./src/external/imgui-1.88/backends/imgui_impl_opengl3.cpp
                  ./src/external/imgui-1.88/backends/imgui_impl_opengl3.h
                  ./src/external/imgui-1.88/backends/imgui_impl_glfw.cpp
                  ./src/external/imgui-1.88/backends/imgui_impl_glfw.h)
add_library(stb_image ./src/external/stb_image/stb_image.h
                      ./src/external/stb_image/stb_image.cpp)
add_subdirectory(./src/external/glm-0.9.9.8/glm/)
add_subdirectory(./src/external/glfw-3.3.8/)
add_subdirectory(./src/external/assimp-5.3.0)
target_include_directories(glad PUBLIC ./src/external/glad-core-4.6/include/)
target_include_directories(imgui PUBLIC ./src/external/imgui-1.88/
                                        ./src/external/imgui-1.88/backends/
                                        ./src/external/glfw-3.3.8/include/)
target_include_directories(stb_image PUBLIC ./src/external/stb_image/)
if(UNIX)
    target_compile_options(glad      PUBLIC "-fPIC")
    target_compile_options(stb_image PUBLIC "-fPIC")
endif(UNIX)

# Project
project(gl_app
    VERSION 1.0.0
    DESCRIPTION "OpenGL 4.6 application."
)

# Executable target
set(SOURCE_LIST src/main.cpp
                src/context_manager.cpp
                src/input/input_manager.cpp
                src/rendering/camera.cpp
                src/rendering/light_manager.cpp
                src/rendering/material_manager.cpp
                src/rendering/renderer.cpp
                src/resources/mesh.cpp
                src/resources/model.cpp
                src/resources/resource_manager.cpp
                src/resources/shader.cpp
                src/resources/texture.cpp
                src/scene/entity_manager.cpp
                src/scene/entity.cpp)
add_executable(gl_app ${SOURCE_LIST})
target_include_directories(gl_app PUBLIC ./src/
                                         ./src/external/
                                         ./src/external/glad-core-4.6/include/
                                         ./src/external/glm-0.9.9.8/glm/
                                         ./src/external/glfw-3.3.8/include/
                                         ./src/external/imgui-1.88/
                                         ./src/external/assimp-5.3.0/include/)
target_link_libraries(gl_app PUBLIC glad
                                    glm
                                    glfw
                                    imgui
                                    assimp
                                    stb_image)

# Copying folders into build
copy_folder(gl_app ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} assets)