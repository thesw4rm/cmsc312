cmake_minimum_required(VERSION 2.8)
project(a2_312)
 
#Bring the headers, such as Student.h into the project
include_directories(include)
 
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
 
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.h *.c")
 
add_executable(prog ${SOURCES})
target_link_libraries(prog pthread)
