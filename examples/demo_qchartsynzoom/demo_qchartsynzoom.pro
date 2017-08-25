TEMPLATE = app
CONFIG += qt qwt


QT  += core gui
greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets

CONFIG( qwt ) : include( ../../find_qwt.pri )
include( ../../qchartsynzoom.pri )


SOURCES += \
    main.cpp \
    mainwindow.cpp \

HEADERS += \
    mainwindow.h \

FORMS   += \
    mainwindow.ui


TARGET  = demo_qwtchartsynzoom
