
cmake_minimum_required(VERSION 3.13)

set(LIB_MAJOR 1)
set(LIB_MINOR 0)
set(LIB_BUILD 0)
set(LIB_REVISION 0)
set(LIB_VERSION ${LIB_MAJOR}.${LIB_MINOR}.${LIB_REVISION})

project("libcifx SPM plugin" VERSION ${LIB_VERSION})

# shared library
add_library(netx-spm SHARED)

add_definitions(-D_GNU_SOURCE)

include_directories( ${CMAKE_CURRENT_LIST_DIR})

if(CIFX_HEADER)
    include_directories( ${CIFX_HEADER})
else(CIFX_HEADER)
    include(FindPkgConfig)
    pkg_check_modules(LIBCIFX REQUIRED cifx)
    include_directories(${LIBCIFX_INCLUDE_DIRS})
endif(CIFX_HEADER)

file(GLOB SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.c)
target_sources(netx-spm
    PRIVATE
        ${SOURCES}
)

set_target_properties( netx-spm PROPERTIES PREFIX "")

if(NOT PLUGINPATH)
    set(PLUGINPATH "/opt/cifx/plugins/")
endif(NOT PLUGINPATH)

# install resources
install(TARGETS netx-spm DESTINATION ${PLUGINPATH})
if(NOT EXISTS ${PLUGINPATH}/netx-spm/config0)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/config0 DESTINATION ${PLUGINPATH}/netx-spm/)
endif(NOT EXISTS ${PLUGINPATH}/netx-spm/config0)
