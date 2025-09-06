# ZeroTier One GUI CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Enable Qt6 support
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Set up Qt6 for this project
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define ZT_SDK to avoid automatic library linking
add_definitions(-DZT_SDK=1)

# Source files for the GUI (standalone mode)
set(GUI_SOURCES
    main.cpp
    qt/QtGUIManager.cpp
    common/BaseGUIManager.cpp
)

set(GUI_HEADERS
    qt/QtGUIManager.hpp
    common/BaseGUIManager.hpp
    common/GUIManager.hpp
)

# Resource files (icons, etc.)
set(GUI_RESOURCES
    resources/icons.qrc
)

# Create the GUI executable
add_executable(zerotier-gui ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_RESOURCES})

# Link Qt6 libraries and dependencies
target_link_libraries(zerotier-gui 
    Qt6::Core 
    Qt6::Widgets
    Qt6::Network
)

# Include directories
target_include_directories(zerotier-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings
    set_target_properties(zerotier-gui PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "zerotier-gui"
    )
    
    # Add Windows resources if they exist
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/zerotier.rc")
        target_sources(zerotier-gui PRIVATE resources/windows/zerotier.rc)
    endif()
    
    # Windows-specific libraries
    target_link_libraries(zerotier-gui 
        ws2_32 
        iphlpapi 
        wininet 
        netapi32
        advapi32
        shell32
        user32
        kernel32
    )
elseif(UNIX)
    # Linux-specific settings
    set_target_properties(zerotier-gui PROPERTIES
        OUTPUT_NAME "zerotier-gui"
    )
    
    # No additional libraries needed for basic Linux build
    
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(zerotier-gui PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "ZeroTier One"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.14.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.14.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.zerotier.one"
        MACOSX_BUNDLE_ICON_FILE "zerotier.icns"
    )
    
    # Add macOS resources if the icon file exists
    set(MACOS_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/zerotier.icns")
    if(EXISTS "${MACOS_ICON_PATH}")
        target_sources(zerotier-gui PRIVATE "${MACOS_ICON_PATH}")
        set_source_files_properties("${MACOS_ICON_PATH}" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
    else()
        # Copy from artwork directory if it exists
        set(ARTWORK_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../artwork/ZeroTierIcon.icns")
        if(EXISTS "${ARTWORK_ICON_PATH}")
            file(COPY "${ARTWORK_ICON_PATH}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/")
            file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/ZeroTierIcon.icns" 
                        "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/zerotier.icns")
            target_sources(zerotier-gui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/zerotier.icns")
            set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/zerotier.icns" PROPERTIES
                MACOSX_PACKAGE_LOCATION "Resources"
            )
        endif()
    endif()
    
elseif(UNIX)
    # Linux-specific settings
    install(TARGETS zerotier-gui DESTINATION bin)
    install(FILES resources/linux/zerotier-one.desktop 
            DESTINATION share/applications)
    install(FILES resources/linux/zerotier.png 
            DESTINATION share/pixmaps)
endif()

# Copy Qt6 libraries on Windows and macOS
if(WIN32)
    # Windows Qt6 deployment
    find_program(QT_DEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(QT_DEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET zerotier-gui POST_BUILD
            COMMAND ${QT_DEPLOYQT_EXECUTABLE} $<TARGET_FILE:zerotier-gui>
            COMMENT "Deploying Qt6 libraries")
    endif()
elseif(APPLE)
    # macOS Qt6 deployment
    find_program(QT_DEPLOYQT_EXECUTABLE macdeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(QT_DEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET zerotier-gui POST_BUILD
            COMMAND ${QT_DEPLOYQT_EXECUTABLE} $<TARGET_BUNDLE_DIR:zerotier-gui>
            COMMENT "Deploying Qt6 libraries")
    endif()
endif()
