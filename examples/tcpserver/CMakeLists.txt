
cmake_minimum_required (VERSION 2.8.12)
project(cifxtcpserver)
set(PROJECT_VERSION, 1.0.0)

option(PERMANENT "Enable permanent connection (marshaller server feature - see \"HIL_MARSHALLER_PERMANENT_CONNECTION\")" ON)

set(src_dir ${CMAKE_CURRENT_LIST_DIR})

if(LIBRARY_HEADER OR LIBRARY_INC_LIB)
    if (LIBRARY_HEADER)
        set(LIBRARY_REQ_INCLUDE_DIRS  ${LIBRARY_HEADER})
    endif (LIBRARY_HEADER)
    if (LIBRARY_INC_LIB)
        set (LIBRARY_INC_LIB "-L${LIBRARY_INC_LIB}")
    endif (LIBRARY_INC_LIB)
    set(LIBRARY_REQ_LIBRARIES "-lpthread -lrt -lcifx ${LIBRARY_INC_LIB}")
else(LIBRARY_HEADER OR LIBRARY_INC_LIB)
    include(FindPkgConfig)
    pkg_check_modules(LIBRARY_REQ REQUIRED cifx)
endif(LIBRARY_HEADER OR LIBRARY_INC_LIB)

include_directories(${LIBRARY_REQ_INCLUDE_DIRS} ${src_dir}/Marshaller/cifXAPI/ /usr/local/include/cifx/)

add_executable(cifx_tcpserver
    ${src_dir}/tcp_connector.c
    ${src_dir}/os_specific.c
    ${src_dir}/tcp_server.c
    ${src_dir}/cifx_download_hook.c
    ${src_dir}/Marshaller/CifXTransport.c
    ${src_dir}/Marshaller/HilMarshaller.c
)
target_include_directories(cifx_tcpserver
    PUBLIC
        ${src_dir}/
        ${src_dir}/Marshaller
        ${src_dir}/Marshaller/APIHeader/
)

target_link_libraries (cifx_tcpserver ${LIBRARY_REQ_LIBRARIES})
target_compile_options(cifx_tcpserver PRIVATE -Wformat-overflow=0)

target_compile_definitions(cifx_tcpserver
    PUBLIC
        $<$<BOOL:${PERMANENT}>:HIL_MARSHALLER_PERMANENT_CONNECTION>
)

install(TARGETS cifx_tcpserver DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
