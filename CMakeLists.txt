cmake_minimum_required(VERSION 3.14)

project(
  MyProject
  VERSION 1.0
  LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# --------------------------------------------------
# C++ Standard
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 17)

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

include(FetchContent)

# GLFW options must be set BEFORE MakeAvailable
set(GLFW_BUILD_DOCS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "" FORCE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.7)

FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG v6.0.5)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.3)

# --------------------------------------------------
# Assimp configuration
# --------------------------------------------------

set(ASSIMP_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS
    OFF
    CACHE BOOL "" FORCE)

# Disable everything by default
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)

# Enable only what you likely need
set(ASSIMP_BUILD_OBJ_IMPORTER
    ON
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER
    ON
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER
    ON
    CACHE BOOL "" FORCE)

# --------------------------------------------------
# Fetch dependencies
# --------------------------------------------------

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(imgui)
FetchContent_MakeAvailable(assimp)
FetchContent_MakeAvailable(glm)

# assimp optimization
target_compile_options(assimp PRIVATE -O2) # ultra performance: -O3 and -flto

# --------------------------------------------------
# OpenGL
# --------------------------------------------------

find_package(OpenGL REQUIRED)

# --------------------------------------------------
# GLAD
# --------------------------------------------------

add_library(glad STATIC ext/glad/src/glad.c)

target_include_directories(glad PUBLIC ext/glad/include)

# --------------------------------------------------
# ImGui
# --------------------------------------------------

add_library(
  imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}
                                        ${imgui_SOURCE_DIR}/backends)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# --------------------------------------------------
# Project Sources
# --------------------------------------------------

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(MyApp ${SOURCES})

# --------------------------------------------------
# Main executable
# --------------------------------------------------

target_link_libraries(MyApp PRIVATE glad glfw glm::glm OpenGL::GL imgui assimp)

target_include_directories(MyApp PRIVATE src)

# --------------------------------------------------
# Warnings
# --------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(MyApp PRIVATE -Wall -Wextra -Wpedantic)
endif()
