TEMPLATE = app
CONFIG += qt qwt


QT  += core gui
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    INCLUDEPATH += /usr/include/qwt/
    LIBS += -L/usr/lib/ -lqwt
    unix : QMAKE_RPATHDIR += /usr/lib/
}


include( ../../qchartsynzoom.pri )


SOURCES += \
    main.cpp \
    mainwindow.cpp \

HEADERS += \
    mainwindow.h \

FORMS   += \
    mainwindow.ui


TARGET  = demo2_qwtchartsynzoom
