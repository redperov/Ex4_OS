cmake_minimum_required(VERSION 3.7)
project(Ex4)

set(CMAKE_C_STANDARD 90)

set(SOURCE_FILES ex42.c ex41.c)
add_executable(Server ex42.c)
add_executable(Client ex41.c)
target_link_libraries(Server -lpthread)