project(Darpa_Data_Collection)
set(CMAKE_CXX_FLAGS "-DNDEBUG -O3 -std=c++0x")
cmake_minimum_required(VERSION 2.8)
SET(SRC_LIST main.cpp utils.cpp eeg_receiver.cpp gnuplot.cpp)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries (${PROJECT_NAME} SDL SDL_gfx fltk SDL_ttf SDL_image fftw3 pthread zmq rt)

