#-------------------------------------------------
#
# Project created by QtCreator 2014-06-18T13:02:01
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = DomeProject
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
INCLUDEPATH +=G:/opencv-2.4.9/mybuild/install/include \
              "F:/Program Files (x86)/Intel/OpenCL SDK/4.4/include"
CONFIG(debug, debug|release): {
LIBS += -LG:/OpenCV/build/x64/vc11/lib \
        -lopencv_core248d \
        -lopencv_highgui248d \
        -lopencv_imgproc248d \
        -lopencv_contrib248d \
        -lopencv_calib3d248d \
        #-lopencv_ocl248d \
        -L"F:/Program Files (x86)/Intel/OpenCL SDK/4.4/lib/x64" -lOpenCL
}

CONFIG(release, debug|release): {
LIBS += -LG:/OpenCV/build/x64/vc11/lib \
        -lopencv_core248 \
        -lopencv_imgproc248 \
        -lopencv_highgui248 \
        -lopencv_contrib248 \
        -lopencv_calib3d248 \
        #-lopencv_ocl248 \
        -L"F:/Program Files (x86)/Intel/OpenCL SDK/4.4/lib/x64" -lOpenCL
}

SOURCES += main.cpp \
    domecamera.cpp \
    dome.cpp \
    skeleton.cpp \
    visualhull.cpp

HEADERS += \
    domecamera.h \
    dome.h \
    skeleton.h \
    checkVoxel.cl \
    visualhull.h
