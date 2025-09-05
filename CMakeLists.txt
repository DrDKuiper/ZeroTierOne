# CMake build script for ZeroTier One

cmake_minimum_required (VERSION 3.16)
project (zerotierone VERSION 1.14.0)

# Options
option(BUILD_GUI "Build the GUI application" OFF)

# Set C++17 as the standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set (PROJ_DIR ${PROJECT_SOURCE_DIR})
set (ZT_DEFS)

# Core library
file(GLOB core_src_glob ${PROJ_DIR}/node/*.cpp)
add_library(zerotiercore STATIC ${core_src_glob})

target_compile_options(zerotiercore PRIVATE ${ZT_DEFS})

# Add C++17 required definitions
if(APPLE)
    # Ensure proper shared_mutex support on macOS
    target_compile_definitions(zerotiercore PRIVATE _LIBCPP_ENABLE_SHARED_MUTEX)
endif()
target_include_directories(zerotiercore PUBLIC 
    ${PROJ_DIR}/include
    ${PROJ_DIR}/ext
    ${PROJ_DIR}/ext/prometheus-cpp-lite-1.0/simpleapi/include
    ${PROJ_DIR}/ext/prometheus-cpp-lite-1.0/core/include
    ${PROJ_DIR}/node
    ${PROJ_DIR}/osdep
    ${PROJ_DIR}/service
)

# Service/daemon executable
file(GLOB service_src_glob ${PROJ_DIR}/service/*.cpp)

# Platform-specific source files
set(osdep_src "")
if(UNIX)
    file(GLOB osdep_unix_src 
        ${PROJ_DIR}/osdep/OSUtils.cpp
        ${PROJ_DIR}/osdep/Http.cpp
        ${PROJ_DIR}/osdep/Phy.cpp
        ${PROJ_DIR}/osdep/Arp.cpp
        ${PROJ_DIR}/osdep/EthernetTap.cpp
    )
    list(APPEND osdep_src ${osdep_unix_src})

    if(APPLE)
        file(GLOB osdep_bsd_src 
            ${PROJ_DIR}/osdep/MacEthernetTap.cpp
            ${PROJ_DIR}/osdep/MacKextEthernetTap.cpp
            ${PROJ_DIR}/osdep/MacMDNSResponder.cpp
        )
        list(APPEND osdep_src ${osdep_bsd_src})
    else()
        # Linux
        file(GLOB osdep_linux_src 
            ${PROJ_DIR}/osdep/LinuxEthernetTap.cpp
            ${PROJ_DIR}/osdep/LinuxNetLink.cpp
            ${PROJ_DIR}/osdep/ManagedRoute.cpp
        )
        list(APPEND osdep_src ${osdep_linux_src})
    endif()
elseif(WIN32)
    file(GLOB osdep_windows_src 
        ${PROJ_DIR}/osdep/OSUtils.cpp
        ${PROJ_DIR}/osdep/WindowsEthernetTap.cpp
        ${PROJ_DIR}/osdep/WindowsMDNSResponder.cpp
        ${PROJ_DIR}/osdep/Http.cpp
    )
    list(APPEND osdep_src ${osdep_windows_src})
endif()

add_executable(zerotier-one
    ${PROJ_DIR}/one.cpp
    ${service_src_glob}
    ${osdep_src}
    ${PROJ_DIR}/controller/EmbeddedNetworkController.cpp
    ${PROJ_DIR}/controller/CtlUtil.cpp
    ${PROJ_DIR}/controller/DB.cpp
    ${PROJ_DIR}/controller/DBMirrorSet.cpp
    ${PROJ_DIR}/controller/FileDB.cpp
    ${PROJ_DIR}/controller/LFDB.cpp
    ${PROJ_DIR}/controller/PostgreSQL.cpp
    ${PROJ_DIR}/controller/CV1.cpp
    ${PROJ_DIR}/controller/CV2.cpp
)
# Compile the http-parser library
add_library(http_parser STATIC ${CMAKE_CURRENT_SOURCE_DIR}/ext/http-parser/http_parser.c)
target_include_directories(http_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ext/http-parser)

target_link_libraries(zerotier-one PRIVATE zerotiercore http_parser)

target_include_directories(zerotier-one PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/opentelemetry-cpp-1.21.0/api/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/opentelemetry-cpp-1.21.0/sdk/include"
)

if (UNIX AND NOT APPLE)
    # Install rules for Unix-like systems
    install(TARGETS zerotier-one
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
    )
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*"
    )
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ext/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*"
    )
endif()

# Optional GUI
if(BUILD_GUI)
    add_subdirectory(gui)
endif()
