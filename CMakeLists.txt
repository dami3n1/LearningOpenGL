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

# ---- Build executable ----
add_executable(MyApp main.cpp)

set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "Build GLFW as static library")
target_link_libraries(MyApp PRIVATE glfw  OpenGL::GL)