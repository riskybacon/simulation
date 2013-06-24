cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME transform_feedback)

project(${PROJ_NAME})

# Set up C++0x
set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -stdlib=libc++ -g -Wall")
include_directories(/usr/lib/c++/v1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
 ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/
)

include(FindOpenGL)
include(FindGLFW)
include(FindGLM)

# Make sure that OpenGL is found
if(NOT OPENGL_FOUND)
  message(ERROR "Could not find OpenGL")
endif(NOT OPENGL_FOUND)

# Make sure that GLFW is found
if(NOT GLFW_FOUND)
  message(ERROR "Could not find GLFW")
endif(NOT GLFW_FOUND)

# Use OpenGL 3 core context
add_definitions("-DGLFW_INCLUDE_GL3 -DGLFW_NO_GLU -DOPENGL3")

set(OGL_DEBUG 1 CACHE BOOL "Enable OpenGL debugging code?")

if(OGL_DEBUG)
  add_definitions("-D_DEBUG")
endif(OGL_DEBUG)

# Set the include directories
include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
)

# Get the path to the source code and create a define. This is used
# for locating the shaders
add_definitions("-DSOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

# Platform specific libraries
if(APPLE)
  set(PLATFORM_LIBRARIES "-framework IOKit")
endif(APPLE)

# Add a target executable
add_executable(${PROJ_NAME}
  main.cpp
  shader.cpp
  opengl.h
  shader.h
)

# Libraries to be linked
target_link_libraries(${PROJ_NAME}
  ${OPENGL_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${PLATFORM_LIBRARIES}
  "-framework IOKit"
)
