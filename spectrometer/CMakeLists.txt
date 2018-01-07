cmake_minimum_required(VERSION 2.8.9)
project (spec)

find_package( OpenCV REQUIRED )
find_package(GSL REQUIRED) 

add_executable(spec spectrometer.cpp)
target_link_libraries( spec ${OpenCV_LIBS} GSL::gsl)
