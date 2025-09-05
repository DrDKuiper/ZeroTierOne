# CMake build script for ZeroTier One

cmake_minimum_required (VERSION 3.16)
project (zerotierone VERSION 1.14.0)

# Options
option(BUILD_GUI "Build the GUI application" OFF)

set (PROJ_DIR ${PROJECT_SOURCE_DIR})
set (ZT_DEFS -std=c++11)

# Core library
file(GLOB core_src_glob ${PROJ_DIR}/node/*.cpp)
add_library(zerotiercore STATIC ${core_src_glob})

target_compile_options(zerotiercore PRIVATE ${ZT_DEFS})
target_include_directories(zerotiercore PUBLIC 
    ${PROJ_DIR}/include
    ${PROJ_DIR}/ext
    ${PROJ_DIR}/ext/prometheus-cpp-lite-1.0/core/include
    ${PROJ_DIR}/ext/prometheus-cpp-lite-1.0/simpleapi/include
    ${PROJ_DIR}/node
    ${PROJ_DIR}/osdep
    ${PROJ_DIR}/service
)

# Service/daemon executable
file(GLOB service_src_glob ${PROJ_DIR}/service/*.cpp)
add_executable(zerotier-one ${PROJ_DIR}/one.cpp ${service_src_glob})
target_link_libraries(zerotier-one zerotiercore)
target_compile_options(zerotier-one PRIVATE ${ZT_DEFS})
target_include_directories(zerotier-one PRIVATE 
    ${PROJ_DIR}/include
    ${PROJ_DIR}/ext
    ${PROJ_DIR}/ext/prometheus-cpp-lite-1.0/core/include
    ${PROJ_DIR}/ext/prometheus-cpp-lite-1.0/simpleapi/include
    ${PROJ_DIR}/node
    ${PROJ_DIR}/osdep
    ${PROJ_DIR}/service
)

# Optional GUI
if(BUILD_GUI)
    add_subdirectory(gui)
endif()
