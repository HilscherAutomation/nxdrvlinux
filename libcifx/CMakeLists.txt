
cmake_minimum_required(VERSION 2.8.12)
# required to set project version
cmake_policy(SET CMP0048 NEW)

FIND_PACKAGE(PkgConfig REQUIRED)

set(DRV_MAJOR 3)
set(DRV_MINOR 0)
set(DRV_PATCH 0)

set(DRV_VERSION ${DRV_MAJOR}.${DRV_MINOR}.${DRV_PATCH})
set(DRVVERSION "\"V${DRV_VERSION}\"")

project("libcifx user space library" VERSION ${DRV_VERSION})

option(DEBUG                 "Build Library debug messages" OFF)
option(DISABLE_LIB_PCIACCESS "Disable link to libpciaccess. If set to 'ON' - only VFIO PCI devices are supported" OFF)
option(TIME                   "Enable device time setting during start-up" OFF)
option(DMA                    "Compile driver with dma support" OFF)
option(NO_MINSLEEP            "Disable minimum sleep time" OFF)
option(VIRTETH                "Enables virtual ethernet interface support" OFF)
option(HWIF                   "Enables the toolkit's Hardware Function Interface (e.g. for SPI)" OFF)
option(PLUGIN                 "Enables support of device plugins and enables hardware function interface (see HWIF)" OFF)
option(SPM_PLUGIN             "Build spm-plugin and enables support of device plugins (see PLUGIN)" OFF)
option(SHARED                 "Build shared libary (default: ON)" ON)
option(VFIO                   "Build libary supporting VFIO interface (default: OFF)" OFF)

if(SHARED)
    add_library( cifx SHARED)
    set_target_properties(cifx PROPERTIES VERSION ${DRV_VERSION} SOVERSION ${DRV_MAJOR})
else(SHARED)
    add_library( cifx STATIC)
endif(SHARED)

if(VIRTETH STREQUAL "ON")
  set(VIRTETH_SEND_RETRIES     "0" CACHE STRING "Number of send retries")
  set(VIRTETH_MAX_ACTIVE_SENDS "8" CACHE STRING "Maximum active sends - depends on mailbox depth")
else(VIRTETH STREQUAL "ON")
  unset(VIRTETH_SEND_RETRIES CACHE)
  unset(VIRTETH_MAX_ACTIVE_SENDS CACHE)
endif(VIRTETH STREQUAL "ON")

if(VFIO STREQUAL "ON")
  set(VFIO_FORCE_LEGACY FALSE CACHE BOOL "Force library supporting only the legacy vfio interface and not cdev interface (CONFIG_VFIO_DEVICE_CDEV) (default: OFF)")
else(VFIO STREQUAL "ON")
  unset(VFIO_FORCE_LEGACY CACHE)
endif(VFIO STREQUAL "ON")

set(src_dir ${CMAKE_CURRENT_LIST_DIR})
set(tk_dir  ${src_dir}/Toolkit)

# check endianess
include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

target_compile_definitions( cifx
    PRIVATE
        VERSION_INFO="\"${DRVVERSION}\""
        $<$<BOOL:${IS_BIG_ENDIAN}>:CIFX_TOOLKIT_BIGENDIAN>

        $<$<BOOL:${DEBUG}>:DEBUG>
        $<$<BOOL:${DISABLE_LIB_PCIACCESS}>:CIFX_NO_PCIACCESS_LIB>
        $<$<BOOL:${DMA}>:CIFX_TOOLKIT_DMA>
        $<$<BOOL:${NO_MINSLEEP}>:NO_MIN_SLEEP>
        $<$<BOOL:${TIME}>:CIFX_TOOLKIT_TIME>
        $<$<BOOL:${VIRTETH}>:CIFXETHERNET>
        $<$<BOOL:${VIRTETH}>:NETX_TAP_SEND_RETRIES=${VIRTETH_SEND_RETRIES}>
        $<$<BOOL:${VIRTETH}>:NETX_TAP_MAX_ACTIVE_SENDS=${VIRTETH_MAX_ACTIVE_SENDS}>
        $<$<BOOL:${HWIF}>:CIFX_DRV_HWIF>
        $<$<OR:$<BOOL:${PLUGIN}>,$<BOOL:${SPM_PLUGIN}>>:CIFX_DRV_HWIF CIFX_PLUGIN_SUPPORT>
        $<$<BOOL:${VFIO}>:VFIO_SUPPORT>
        $<$<NOT:$<BOOL:${VFIO_FORCE_LEGACY}>>:VFIO_CDEV>
)

file(GLOB SOURCES "${src_dir}/*.c" "${tk_dir}/Source/*.c")
if(VIRTETH)
    file(GLOB NETXTAP_SOURCES ${src_dir}/netx_tap/*.c)
    set_source_files_properties(${tk_dir}/Source/cifXFunctions.c PROPERTIES COMPILE_FLAGS -DAPIRENAME)

    if(NOT LIBDNL_INCLUDE_DIRS OR NOT LIBDNL_LIBRARY_DIRS OR NOT LIBDNL_LIBRARIES)
        pkg_check_modules(LIBDNL libnl-3.0)
        set(REQUIRED_PACKAGES "${REQUIRED_PACKAGES} libnl-3.0")
    endif(NOT LIBDNL_INCLUDE_DIRS OR NOT LIBDNL_LIBRARY_DIRS OR NOT LIBDNL_LIBRARIES)

    if(NOT LIBDNL_CLI_INCLUDE_DIRS OR NOT LIBDNL_CLI_LIBRARY_DIRS OR NOT LIBDNL_CLI_LIBRARIES)
        pkg_check_modules(LIBDNL-CLI libnl-cli-3.0)
        set(REQUIRED_PACKAGES "${REQUIRED_PACKAGES} libnl-cli-3.0")
    endif(NOT LIBDNL_CLI_INCLUDE_DIRS OR NOT LIBDNL_CLI_LIBRARY_DIRS OR NOT LIBDNL_CLI_LIBRARIES)
endif(VIRTETH)

target_link_directories( cifx
    PRIVATE
        $<$<NOT:$<BOOL:${DISABLE_LIB_PCIACCESS}>>:${LIBPCIACCESS_LIBRARY_DIRS}>
        $<$<BOOL:${VIRTETH}>:${LIBDNL_LIBRARY_DIRS}>
        $<$<BOOL:${VIRTETH}>:${LIBDNL_CLI_LIBRARY_DIRS}>
)

target_include_directories( cifx
    PRIVATE
        ${src_dir}/
        ${tk_dir}/Source/
        ${tk_dir}/Common/cifXAPI/
        ${tk_dir}/Common/HilscherDefinitions/
        $<$<NOT:$<BOOL:${DISABLE_LIB_PCIACCESS}>>:${LIBPCIACCESS_INCLUDE_DIRS}>
        $<$<BOOL:${VIRTETH}>:${src_dir}/netx_tap/ ${LIBDNL_CLI_INCLUDE_DIRS} ${LIBDNL_INCLUDE_DIRS}>
)

target_link_libraries( cifx
    PRIVATE
        pthread
        rt
        $<$<NOT:$<BOOL:${DISABLE_LIB_PCIACCESS}>>:pciaccess>
        $<$<OR:$<BOOL:${PLUGIN}>,$<BOOL:${SPM_PLUGIN}>>:dl>
        $<$<BOOL:${VIRTETH}>:${LIBDNL_LIBRARIES}>
        $<$<BOOL:${VIRTETH}>:${LIBDNL_CLI_LIBRARIES}>
)

# link and create shared & static library
target_sources( cifx
    PRIVATE
        ${SOURCES}
        ${NETXTAP_SOURCES}
)

# removes warning about snprintf buffer truncation
set_target_properties(cifx PROPERTIES COMPILE_FLAGS -Wformat-truncation=0)

# configure pkgconfig file
set(prefix       ${CMAKE_INSTALL_PREFIX})
set(exec_prefix  ${prefix})
set(libdir       ${exec_prefix}/lib)
set(includedir   ${prefix}/include)
configure_file( ${src_dir}/cifx.pc.in ${CMAKE_CURRENT_BINARY_DIR}/cifx.pc @ONLY)

# install header files
file(GLOB INSTALL_HEADERS
     ${src_dir}/cifxlinux.h
     ${tk_dir}/Source/cifXEndianess.h
     ${tk_dir}/Common/cifXAPI/*.h
     ${tk_dir}/Common/HilscherDefinitions/*.h
     $<$<BOOL:${VIRTETH}>:${src_dir}/netx_tap/netx_tap.h>
)

# install resources
install(TARGETS cifx                              DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)
install(FILES ${INSTALL_HEADERS}                  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/cifx)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cifx.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

# can't add it as subdir as long it's one above
if (SPM_PLUGIN)
    set(CIFX_HEADER ${tk_dir}/Common/cifXAPI/ ${tk_dir}/Common/HilscherDefinitions/ ${tk_dir}/Source/  ${src_dir}/)
    include(${src_dir}/../plugins/netx-spm/CMakeLists.txt)
endif(SPM_PLUGIN)
