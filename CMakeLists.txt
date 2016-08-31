project(rogue)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)

# Make targets
add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

# Set up libraries
include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

include_directories(
  ${SDL2_INCLUDE_DIRS}
  ${SDL2IMAGE_INCLUDE_DIRS}
  ${SDL2TTF_INCLUDE_DIRS}
)
target_link_libraries(
  ${PROJECT_NAME}
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
