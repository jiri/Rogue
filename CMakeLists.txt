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
  src/Font.cpp
)

# Set up libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED STATIC)
find_package(glm REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

find_package(Freetype REQUIRED)

include_directories(
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
)
target_link_libraries(
  ${PROJECT_NAME}

  # OpenGL
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}

  # Freetype
  ${FREETYPE_LIBRARIES}
)

# Set up SOIL
add_library(soil STATIC
  SOIL/src/image_helper.c
  SOIL/src/stb_image_aug.c
  SOIL/src/image_DXT.c
  SOIL/src/SOIL.c
)

if (APPLE)
  target_link_libraries(soil
    "-framework CoreFoundation"
  )
endif ()

include_directories(SOIL/src/)
target_link_libraries(${PROJECT_NAME} soil)

# Copy resources

# XXX: Temporary symlink for development purposes
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMENT "Linking resources..."
  COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
)
