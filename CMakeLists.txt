cmake_minimum_required(VERSION 3.16)

project(StarSummary2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)

#Now using PkgConfig find libindi
pkg_check_modules(CFITS   REQUIRED IMPORTED_TARGET cfitsio)
pkg_check_modules(CCFITS REQUIRED IMPORTED_TARGET CCfits)
if (${CCFITS_FOUND})
    message(STATUS "CCFits found")
    message(STATUS "CCFITS Inc is <${CCFITS_INCLUDE_DIRS}> ${CCFITS_INCLUDE_DIRS}")
    message(STATUS "CCFITS Lib is <${CCFITS_LIBRARIES_DIR}> ${CCFITS_LIBRARIES}")

else()
    message(FATAL "CCFits not installed or configured")
endif()


find_library(SEP_LIB sep PATHS /usr/local/lib)
if (SEP_LIB)
    message(STATUS "Sep library has been found")
    message(STATUS "Sep Lib is <${SEP_LIB}> ${SEP_LIB}")
    set(SEP_INCLUDE_DIR "/usr/local/include/")
    message(STATUS "Sep Inc Dir  is <${SEP_INCLUDE_DIR}> ")

else()
    message(FATAL "Sep Lib not installed,configured or could not be found")
endif()

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        starextractor.cpp
        starextractor.h
        mainwindow.h
        mainwindow.ui
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(StarSummary2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        starextractor.h starextractor.cpp)
endif()
# --- Include directories ---
target_include_directories(StarSummary2 PRIVATE
  ${CCFITS_INCLUDE_DIRS}
  ${CFITS_INCLUDE_DIRS}
  ${SEP_INCLUDE_DIR}

)

target_link_libraries(StarSummary2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PkgConfig::CFITS PkgConfig::CCFITS ${SEP_LIB} )


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.StarSummary2)
endif()
set_target_properties(StarSummary2 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS StarSummary2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(StarSummary2)
endif()
