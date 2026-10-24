# Kiküldetési Rendelvény Ellenőrző — kiküldetési rendelvények ellenőrzése.
# Copyright (C) 2026 Vajda Ákos Péter <vajdaakos@alu-fon.hu>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.19)

if(NOT CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain file")
    elseif(EXISTS "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain file")
    endif()
endif()

if(CMAKE_HOST_WIN32 AND NOT VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "vcpkg triplet")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(KrEllenorzo LANGUAGES CXX)

if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    get_filename_component(_vcpkg_root "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
    get_filename_component(_vcpkg_root "${_vcpkg_root}" DIRECTORY)
    get_filename_component(_vcpkg_root "${_vcpkg_root}" DIRECTORY)
endif()

if(WIN32 AND NOT PKG_CONFIG_EXECUTABLE AND DEFINED _vcpkg_root)
    file(GLOB _pkg_config_candidates "${_vcpkg_root}/downloads/tools/msys2/*/mingw64/bin/pkg-config.exe")
    if(_pkg_config_candidates)
        list(GET _pkg_config_candidates 0 PKG_CONFIG_EXECUTABLE)
        set(PKG_CONFIG_EXECUTABLE "${PKG_CONFIG_EXECUTABLE}" CACHE FILEPATH "pkg-config executable")
    endif()
endif()

if(DEFINED _vcpkg_root)
    if(MINGW AND NOT VCPKG_TARGET_TRIPLET MATCHES "mingw")
        message(FATAL_ERROR
            "VCPKG_TARGET_TRIPLET='${VCPKG_TARGET_TRIPLET}' does not match the MinGW compiler. "
            "Clear the CMake configuration or set -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic.")
    endif()
    set(_vcpkg_pkgconfig "${_vcpkg_root}/installed/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
    if(EXISTS "${_vcpkg_pkgconfig}")
        set(ENV{PKG_CONFIG_PATH} "${_vcpkg_pkgconfig}")
    endif()
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)

qt_standard_project_setup()

# Qt 6.11 standard project setup enables AUTOMOC and AUTOUIC only.
set(CMAKE_AUTORCC ON)

qt_add_executable(KrEllenorzo
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    trip.h
    tripmetadata.h
    tripmetadatawidget.h
    tripmetadatawidget.cpp
    columnheaders.h
    triploader.h
    triploader.cpp
    tripstorage.h
    tripstorage.cpp
    tripmanager.h
    tripmanager.cpp
    tripchecker.h
    tripchecker.cpp
    errorreportcreator.h
    errorreportcreator.cpp
    settingsmanager.h
    settingsmanager.cpp
    errorstorage.h
    errorstorage.cpp
    icons.qrc
    version.h
)

if(WIN32)
    target_sources(KrEllenorzo PRIVATE app.rc)
endif()

if(APPLE)
    set_source_files_properties(icons/app.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
    target_sources(KrEllenorzo PRIVATE icons/app.icns)
    set_target_properties(KrEllenorzo PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER hu.alu-fon.KrEllenorzo
        MACOSX_BUNDLE_ICON_FILE app.icns)
endif()

target_link_libraries(KrEllenorzo
    PRIVATE
        Qt::Core
        Qt::Widgets
        PkgConfig::POPPLER_CPP
)

target_include_directories(KrEllenorzo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GNUInstallDirs)

install(TARGETS KrEllenorzo
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES LICENSE.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

qt_generate_deploy_app_script(
    TARGET KrEllenorzo
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
