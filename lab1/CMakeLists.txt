cmake_minimum_required(VERSION 3.15)
project(Lab1_AbstractMachines)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем подпапки
add_subdirectory(post-machine)
add_subdirectory(turing-machine)
add_subdirectory(cli)