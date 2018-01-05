cmake_minimum_required(VERSION 2.8.9)
project (spec)

include_directories(/usr/local/Cellar/opencv/3.4.0/include/)
link_directories(/usr/local/Cellar/opencv/3.4.0/lib/)

add_executable(spec spectrometer.cpp)

