# vim: syntax=cmake
cmake_minimum_required (VERSION 2.8.8)

project (app-x265)

include_directories(../../../x265/source ../../../x265/build/linux)

link_directories(../../../x265/build/linux)

find_package(Threads REQUIRED)

add_executable(main
  enc.c
  encs.c
  chan.c
  profile.c
  buf.c
  bufs.c
  msg-i.c
  msg-o.c
  signal.c

  main.c
)

target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT} x265)
