cmake_minimum_required(VERSION 3.16)

project(RdpQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic compilation of Qt MOC, RCC, and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Locate Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Project sources
set(SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/RdpManager.h
    src/RdpManager.cpp
)

add_executable(rdpqt ${SOURCES})

target_link_libraries(rdpqt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ─────────────────────────────────────────────────────────
# Installation Rules (needed for CPack and AppImage)
# ─────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS rdpqt
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES packaging/rdpqt.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES packaging/rdpqt.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)

# ─────────────────────────────────────────────────────────
# CPack Configuration
# ─────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME                 "rdpqt")
set(CPACK_PACKAGE_VERSION              "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR               "rdpqt contributors")
set(CPACK_PACKAGE_CONTACT              "https://github.com/amgpulse/rdpqt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  "A modern Qt6 GUI client for FreeRDP (xfreerdp) on Linux")
set(CPACK_PACKAGE_DESCRIPTION
"rdpqt is a lightweight, native Qt6 desktop application that provides a
graphical interface for managing FreeRDP remote desktop connections on Linux.
It supports configuring host, port, username, password, display resolution,
clipboard sharing, and local folder redirection.")
set(CPACK_RESOURCE_FILE_LICENSE        "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_STRIP_FILES                  TRUE)

# ─── DEB ───────────────────────────────────────────────
set(CPACK_DEBIAN_PACKAGE_MAINTAINER    "rdpqt contributors <https://github.com/amgpulse/rdpqt>")
set(CPACK_DEBIAN_PACKAGE_SECTION       "net")
set(CPACK_DEBIAN_PACKAGE_PRIORITY      "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS       "freerdp2-x11 | freerdp3-x11, libqt6widgets6, libqt6gui6, libqt6core6")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE      "https://github.com/amgpulse/rdpqt")

# ─── RPM ───────────────────────────────────────────────
set(CPACK_RPM_PACKAGE_LICENSE          "MIT")
set(CPACK_RPM_PACKAGE_GROUP            "Applications/Internet")
set(CPACK_RPM_PACKAGE_REQUIRES         "freerdp, qt6-qtbase")
set(CPACK_RPM_PACKAGE_URL              "https://github.com/amgpulse/rdpqt")
set(CPACK_RPM_PACKAGE_DESCRIPTION      "${CPACK_PACKAGE_DESCRIPTION}")

include(CPack)
