TEMPLATE = lib
CONFIG += qt buildlib-qchartsynzoom staticlib qwt

include( ../qchartsynzoom.pri )

TARGET = $$QCHARTSYNZOOM_LIBNAME
DESTDIR = $$QCHARTSYNZOOM_LIBDIR

target.path = $$DESTDIR
INSTALLS += target
