
cmake_minimum_required (VERSION 2.8.12)
project(cifx_examples)
set(PROJECT_VERSION, 1.0.0)

include(${CMAKE_CURRENT_LIST_DIR}/tcpserver/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/api/CMakeLists.txt)
