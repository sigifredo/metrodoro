find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia Svg)

# Evitar la suspensión mientras suena usa D-Bus (solo Linux).
if(UNIX AND NOT APPLE)
    find_package(Qt6 REQUIRED COMPONENTS DBus)
endif()

qt_standard_project_setup(REQUIRES 6.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
