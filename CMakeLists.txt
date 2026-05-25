cmake_minimum_required(VERSION 3.14)
project(MyProject)

# Enable C++17 (or whatever you need)
set(CMAKE_CXX_STANDARD 17)

# ---- Fetch GLFW ----
include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4   # or latest stable release
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.7   # or latest stable release
)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp
    GIT_TAG        v6.0.5   # or latest stable release
)

# Make the content available
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(imgui)
FetchContent_MakeAvailable(assimp)

file(GLOB IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/*.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

#find openGL
find_package(OpenGL REQUIRED)

#find glm
add_subdirectory(ext/glm-1.0.3)

# Create GLAD static library
add_library(glad STATIC ext/glad/src/glad.c
        ext/stb/stb_image.h)

# Tell CMake where headers are
target_include_directories(glad PUBLIC ext/glad/include)

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(MyApp ${SOURCES})

set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "Build GLFW as static library")
target_link_libraries(MyApp PRIVATE 
glfw 
glad 
glm::glm
OpenGL::GL 
imgui 
assimp
)