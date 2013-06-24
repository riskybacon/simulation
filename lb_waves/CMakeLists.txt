cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME lb_waves)

project(${PROJ_NAME})

# Set up C++0x
if(APPLE)
  set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -stdlib=libc++ -g -Wall")
  include_directories(/usr/lib/c++/v1)
endif(APPLE)

# Set up the CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/
)

# Find OpenGL dependencies
include(FindOpenGL)
include(FindGLFW)

# Make sure that OpenGL is found
if(NOT OPENGL_FOUND)
  message(ERROR "Could not find OpenGL")
endif(NOT OPENGL_FOUND)

# Make sure that GLFW is found
if(NOT GLFW_FOUND)
  message(ERROR "Could not find GLFW")
endif(NOT GLFW_FOUND)

# Include the platform specific configuration.
# This will define the following useful variables:
#
# LIBRARIES     The libraries to be linked to the executable
# INCLUDE_PATH  Path to the include files
include(${CMAKE_CURRENT_SOURCE_DIR}/PlatformSpecifics.cmake)

# Set the include directories
include_directories(${INCLUDE_PATH}
  ${OPENGL_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR}
)


# Get the path to the source code and create a define. This is used
# for locating the shaders
add_definitions("-DSOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
add_definitions("-D_DEBUG")

# Tell GLFW to use the OpenGL 3 header files
add_definitions("-DGLFW_INCLUDE_GL3")
add_definitions("-DGLFW_NO_GLU")
add_definitions("-DOPENGL3")
set(SOURCE_FILES
  ca_model_glsl.cpp
  ca_model_normals.cpp
  ca_view_glsl.cpp
  main.cpp
  ocean.cpp
  scene.cpp
  shader.cpp
)

set(HEADER_FILES
  ca_model_glsl.h
  ca_model_normals.h
  ca_view_glsl.h
  ocean.h
  opengl.h
  scene.h
  shader.h
)

# Add a target executable
add_executable(${PROJ_NAME}
  ${HEADER_FILES}
  ${SOURCE_FILES}
)

# Libraries to be linked
target_link_libraries(${PROJ_NAME}
  ${LIBRARIES}
  "-framework IOKit"
)
