cmake_minimum_required(VERSION 3.13)
project(Test)
include_directories(/Users/nikita/CLionProjects/linux-4.8.6/include)

add_executable(Test kernelspace/solution.c)
set_property(TARGET Test PROPERTY C_STANDARD 11)
##
add_executable(drvTest userspace/main.c)

