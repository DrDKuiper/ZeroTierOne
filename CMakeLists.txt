# CMake build script for libzerotiercore.a

cmake_minimum_required (VERSION 3.5)
project (zerotiercore)

set (PROJ_DIR ${PROJECT_SOURCE_DIR})
set (ZT_DEFS -std=c++11)

file(GLOB core_src_glob ${PROJ_DIR}/node/*.cpp)
add_library(zerotiercore STATIC ${core_src_glob})

target_compile_options(zerotiercore PRIVATE ${ZT_DEFS})

# Adicionar suporte para interface web
option(ENABLE_WEBUI "Enable Web UI interface" ON)

if(ENABLE_WEBUI)
    # Instalar arquivos da Web UI
    install(DIRECTORY ${PROJ_DIR}/webui/
            DESTINATION share/zerotier-one/webui
            FILES_MATCHING PATTERN "*.html" PATTERN "*.css" PATTERN "*.js")
    
    # Definir macro para habilitar Web UI
    target_compile_definitions(zerotiercore PRIVATE ZT_ENABLE_WEBUI=1)
    
    message(STATUS "Web UI habilitada - arquivos serão instalados em share/zerotier-one/webui")
endif()
