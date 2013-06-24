cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME copy_to_pbo)

project(${PROJ_NAME})

# Set up C++0x
if(APPLE)
  set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x -stdlib=libc++ -g -Wall")
  include_directories(/usr/lib/c++/v1)
endif(APPLE)

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

# Set the include directories
include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/opengl
)

# Get the path to the source code and create a define. This is used
# for locating the shaders
add_definitions("-DSOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

# Platform specific libraries
if(APPLE)
  set(PLATFORM_LIBRARIES "-framework IOKit")
endif(APPLE)

set(COMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../common/opengl)

# Add a target executable
add_executable(${PROJ_NAME}
  main.cpp
  ${COMMON_SOURCE_DIR}/opengl.h
  ${COMMON_SOURCE_DIR}/shader.cpp
  ${COMMON_SOURCE_DIR}/shader.h
  ${COMMON_SOURCE_DIR}/trackball.cpp
  ${COMMON_SOURCE_DIR}/trackball.h
)

# Libraries to be linked
target_link_libraries(${PROJ_NAME}
  ${OPENGL_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)
