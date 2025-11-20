
cmake_minimum_required(VERSION 3.13)
FIND_PACKAGE(PkgConfig REQUIRED)

set(DRV_MAJOR 3)
set(DRV_MINOR 0)
set(DRV_PATCH 0)

set(DRV_VERSION ${DRV_MAJOR}.${DRV_MINOR}.${DRV_PATCH})
set(DRVVERSION "\"V${DRV_VERSION}\"")

project("libcifx user space library" VERSION ${DRV_VERSION})

# general
option(DEBUG                 "Build Library including debug messages (not printable to log file)" OFF)
option(SHARED                "Build shared libary (default: ON)" ON)
option(DISABLE_LIB_PCIACCESS "Disable link to libpciaccess. If set to 'ON' - only VFIO PCI devices are supported" OFF)
#toolkit
option(TIME                   "Enable device time setting during start-up" OFF)
option(DMA                    "Compile driver with dma support" OFF)
option(NO_MINSLEEP            "Disable minimum sleep time" OFF)
# virteth
option(VIRTETH                                "Enables virtual ethernet interface support" OFF)
set(VIRTETH_SEND_RETRIES     "0" CACHE STRING "Number of send retries (modifying may reduce performance)")
set(VIRTETH_MAX_ACTIVE_SENDS "8" CACHE STRING "Maximum active sends - (parameter depends on mailbox depth - modifying may reduce performance drastically)")
# hw-interface / plugin
option(HWIF                   "Enables the toolkit's Hardware Function Interface (e.g. for SPI)" OFF)
option(PLUGIN                 "Enables support of device plugins and enables hardware function interface (sets HWIF)" OFF)
option(SPM_PLUGIN             "Build spm-plugin and enables support of device plugins (sets as well HWIF and PLUGIN)" OFF)
# vfio
option(VFIO                   "Build libary supporting VFIO interface (default: OFF)" OFF)
option(VFIO_FORCE_LEGACY      "Force library supporting only the legacy vfio interface and not cdev interface (CONFIG_VFIO_DEVICE_CDEV) (sets VFIO)" OFF)
# sbom
option(SBOM                   "Generate SBOM" OFF)

if(SHARED)
    add_library( cifx SHARED)
    set_target_properties(cifx PROPERTIES VERSION ${DRV_VERSION} SOVERSION ${DRV_MAJOR})
else(SHARED)
    add_library( cifx STATIC)
endif(SHARED)

set(src_dir ${CMAKE_CURRENT_LIST_DIR})
set(tk_dir  ${src_dir}/Toolkit)

# check endianess
include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (NOT DISABLE_LIB_PCIACCESS)
    set(SBOM_OPTS ${CMAKE_CURRENT_SOURCE_DIR}/sbom/libpciaccess.cdx.in)
endif()

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

        $<$<OR:$<BOOL:${VFIO}>,$<BOOL:${VFIO_FORCE_LEGACY}>>:VFIO_SUPPORT>
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

    set(SBOM_OPTS "${SBOM_OPTS} ${CMAKE_CURRENT_SOURCE_DIR}/sbom/libnl.cdx.in")
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

# -Wformat-truncation=0 removes warning about snprintf buffer truncation
# -Wpedantic emits warnings:
# - Toolkit Header Hil_ApplicationCmd.h used in netx_tap.c "invalid use of structure with flexible array member"
# - use of dlsym(): "ISO C forbids assignment between function pointer and ‘void *'"
set_target_properties(cifx PROPERTIES COMPILE_FLAGS "-Wformat-truncation=0 -Wall -Wextra")

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

add_custom_target(libcifx_sbom
    ALL
    SOURCES
        $<$<BOOL:${SBOM}>:${CMAKE_CURRENT_BINARY_DIR}/libcifx.cdx.json>
)

set(update_sbom ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/update_sbom)
set(TKIT_VERSION "2.8.0.1")
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/Toolkit/LICENSE.md TKIT_LICENSE_TEXT)
if(NOT "${TKIT_LICENSE_TEXT}" STREQUAL "")
    string(REGEX REPLACE "\ " "\\\\ "  TKIT_LICENSE_TEXT_SAN "${TKIT_LICENSE_TEXT}")
    string(REGEX REPLACE "\n" "\\\\n"  TKIT_LICENSE_TEXT_SAN "${TKIT_LICENSE_TEXT_SAN}")
    string(REPLACE "\"" "\\\""  TKIT_LICENSE_TEXT_SAN "${TKIT_LICENSE_TEXT_SAN}")
endif()
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libcifx.cdx.json
    DEPENDS cifx
    DEPENDS ${update_sbom}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sbom/libcifx.cdx.in
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Toolkit/LICENSE.md
    COMMAND jq '.metadata.component.version = \"${DRV_VERSION}\" | .components[0].version = \"${TKIT_VERSION}\" | .components[0].licenses[0].license.text.content = \"${TKIT_LICENSE_TEXT_SAN}\"' ${CMAKE_CURRENT_SOURCE_DIR}/sbom/libcifx.cdx.in > ${CMAKE_CURRENT_BINARY_DIR}/sbom/libcifx.cdx.json
    COMMAND bash -c '${update_sbom} ${CMAKE_CURRENT_BINARY_DIR}/sbom/libcifx.cdx.json ${CMAKE_CURRENT_BINARY_DIR}/libcifx.cdx.json "${SBOM_OPTS}" '
)
