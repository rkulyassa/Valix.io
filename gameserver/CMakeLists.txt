cmake_minimum_required(VERSION 4.2.2)

project(Valix)

add_executable(${PROJECT_NAME} src/main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)