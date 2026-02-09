cmake_minimum_required(VERSION 4.2.2)

project(gameserver)

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/src
  ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src
)

file(GLOB USOCKETS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src/eventing/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src/crypto/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src/io_uring/*.c
)

add_library(usockets STATIC ${USOCKETS_SOURCES})

target_include_directories(usockets PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src
)

target_compile_definitions(usockets PRIVATE LIBUS_NO_SSL)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_features(usockets PRIVATE c_std_11)

target_link_libraries(${PROJECT_NAME} PRIVATE usockets z)