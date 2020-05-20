cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -std=c++14")

project(rt_hw2)


add_executable(
	exe
	main.cpp
	gtu.cpp
)