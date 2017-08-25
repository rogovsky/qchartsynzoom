TEMPLATE = lib
CONFIG += qt buildlib-qchartsynzoom staticlib qwt


QT  += core gui
greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets

ONFIG( qwt ) : include( ../find_qwt.pri )
include( ../qchartsynzoom.pri )


TARGET = $$QCHARTSYNZOOM_LIBNAME
DESTDIR = $$QCHARTSYNZOOM_LIBDIR

target.path = $$DESTDIR
INSTALLS += target
