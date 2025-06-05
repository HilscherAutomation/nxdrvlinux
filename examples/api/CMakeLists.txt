
cmake_minimum_required (VERSION 3.13)
project(cifxapi VERSION 1.0.0)

set(src_dir ${CMAKE_CURRENT_LIST_DIR})

if(LIBRARY_HEADER OR LIBRARY_INC_LIB)
    if (LIBRARY_HEADER)
        set(LIBRARY_REQ_INCLUDE_DIRS ${LIBRARY_HEADER})
    endif (LIBRARY_HEADER)
    if (LIBRARY_INC_LIB)
        set (LIBRARY_INC_LIB "-L${LIBRARY_INC_LIB}")
    endif (LIBRARY_INC_LIB)
    set(LIBRARY_REQ_LIBRARIES "-lpthread -lrt -lcifx ${LIBRARY_INC_LIB}")
else(LIBRARY_HEADER OR LIBRARY_INC_LIB)
    include(FindPkgConfig)
    pkg_check_modules(LIBRARY_REQ REQUIRED cifx)
endif(LIBRARY_HEADER OR LIBRARY_INC_LIB)

add_executable( cifx_api ${src_dir}/cifxlinuxsample.c)
set_target_properties(cifx_api PROPERTIES COMPILE_FLAGS " -Wall -Wextra -Wpedantic")
target_include_directories( cifx_api BEFORE PUBLIC ${src_dir}/ ${LIBRARY_REQ_INCLUDE_DIRS})
target_link_libraries ( cifx_api ${LIBRARY_REQ_LIBRARIES})
install(TARGETS cifx_api DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
