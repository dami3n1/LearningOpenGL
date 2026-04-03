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

# Make the content available
FetchContent_MakeAvailable(glfw)

#find openGL
find_package(OpenGL REQUIRED)

# Create GLAD static library
add_library(glad STATIC ext/glad/src/glad.c)

# Tell CMake where headers are
target_include_directories(glad PUBLIC ext/glad/include)

# ---- Build executable ----
add_executable(MyApp main.cpp)

set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "Build GLFW as static library")
target_link_libraries(MyApp PRIVATE glfw glad OpenGL::GL)