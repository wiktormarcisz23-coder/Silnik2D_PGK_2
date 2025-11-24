cmake_minimum_required(VERSION 3.21)
project(Tiny2DEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Œcie¿ka do SFML 2.6.2 (wa¿ne: do folderu g³ównego!)
set(CMAKE_PREFIX_PATH "C:/DEV/SFML-2.6.2")

find_package(SFML 2.6 REQUIRED COMPONENTS graphics window system)

add_executable(demo src/main.cpp)

target_include_directories(demo PRIVATE include)

target_link_libraries(demo PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

add_custom_command(TARGET demo POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "C:/DEV/SFML-2.6.2/bin"
          "$<TARGET_FILE_DIR:demo>"
)
