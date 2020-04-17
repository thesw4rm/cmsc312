cmake_minimum_required(VERSION 2.8)
project(a3 C)
# Found on StackOverflow. Allows older CMakes to use later versions of C

macro(use_c99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "-std=gnu11 ${CMAKE_C_FLAGS}")
    endif ()
  else ()
    set (CMAKE_C_STANDARD 11)
  endif ()
endmacro(use_c99)
use_c99()

#add_executable(a3 main.c dll.h dll.c user.c user.h printer.c printer.h constants.h)
add_executable(a3 main.c csema_bad.h csema_good.c constants.h dll.h dll.c )
target_link_libraries(a3 pthread)