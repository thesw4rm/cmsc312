cmake_minimum_required(VERSION 2.8)
project(a3 C)

set(CMAKE_C_STANDARD 11)

#add_executable(a3 main.c dll.h dll.c user.c user.h printer.c printer.h constants.h)
add_executable(a3 main.c csema_bad.h csema_good.c constants.h dll.h dll.c )
target_link_libraries(a3 pthread)