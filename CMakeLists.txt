cmake_minimum_required(VERSION 3.2)
project(OsgVideo3D)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package( Qt5Widgets)
find_package( OpenSceneGraph REQUIRED osgDB osgUtil osgManipulator osgViewer osgGA osgFX osgQt osgText)
find_package( OpenCV REQUIRED )
find_package( OpenGL REQUIRED )

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})

# For apple, include and link opengl related libraries
IF(APPLE)
    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    MARK_AS_ADVANCED (COCOA_LIBRARY
            OpenGL_LIBRARY)
    SET(EXTRA_LIBS ${COCOA_LIBRARY} ${OpenGL_LIBRARY})
ENDIF (APPLE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cpp opencv_imagestream.cpp opencv_imagestream.h qt_mainwindow.cpp qt_mainwindow.h virtual_camera.cpp virtual_camera.h)

add_executable(OsgVideo3D ${SOURCE_FILES})
target_link_libraries(OsgVideo3D ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)