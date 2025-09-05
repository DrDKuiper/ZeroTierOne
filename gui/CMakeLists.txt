# ZeroTier One GUI CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Enable Qt6 support
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Set up Qt6 for this project
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files for the GUI
set(GUI_SOURCES
    main.cpp
    qt/QtGUIManager.cpp
    common/BaseGUIManager.cpp
)

set(GUI_HEADERS
    qt/QtGUIManager.hpp
    common/GUIManager.hpp
)

# Resource files (icons, etc.)
set(GUI_RESOURCES
    resources/icons.qrc
)

# Create the GUI executable
add_executable(zerotier-gui ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_RESOURCES})

# Link Qt6 libraries
target_link_libraries(zerotier-gui 
    Qt6::Core 
    Qt6::Widgets
    ${ZEROTIER_ONE_LIBRARIES}
)

# Include directories
target_include_directories(zerotier-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/node
    ${CMAKE_SOURCE_DIR}/service
    ${CMAKE_SOURCE_DIR}/osdep
)

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings
    set_target_properties(zerotier-gui PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "ZeroTier One"
    )
    
    # Add Windows resources
    target_sources(zerotier-gui PRIVATE resources/windows/zerotier.rc)
    
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
    
    # Add macOS resources
    target_sources(zerotier-gui PRIVATE resources/macos/zerotier.icns)
    set_source_files_properties(resources/macos/zerotier.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    
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
