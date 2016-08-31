project(rogue)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)

# Make targets
include_directories(inc)
add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/Shader.cpp
)

# Set up libraries
find_package(OpenGL REQUIRED)
find_package(glew REQUIRED STATIC)
find_package(glm REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# Deprecated
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

include_directories(
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}

  # Deprecated
  ${SDL2_INCLUDE_DIRS}
  ${SDL2IMAGE_INCLUDE_DIRS}
  ${SDL2TTF_INCLUDE_DIRS}
)
target_link_libraries(
  ${PROJECT_NAME}

  # OpenGL
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}

  # Deprecated
  ${SDL2_LIBRARIES}
  ${SDL2IMAGE_LIBRARIES}
  ${SDL2TTF_LIBRARIES}
)

# Copy resources
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMENT "Copying resources..."
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/res
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
)
