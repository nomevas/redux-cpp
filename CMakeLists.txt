cmake_minimum_required(VERSION 3.18)

project(AppStore)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} app_store.cpp)
add_executable(app_store_hierarchy app_store_hierarchy.cpp)