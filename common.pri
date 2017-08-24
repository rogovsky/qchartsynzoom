exists(config.pri) : infile(config.pri, BUILD_LIBRARY, yes) : CONFIG += uselib-qchartsynzoom

TEMPLATE += fakelib
    QCHARTSYNZOOM_LIBNAME = qchartsynzoom
    QCHARTSYNZOOM_LIBDIR  = $${PWD}/lib
TEMPLATE -= fakelib

unix : uselib-qchartsynzoom : !buildlib-qchartsynzoom : QMAKE_RPATHDIR += $$QCHARTSYNZOOM_LIBDIR

#CONFIG *= c++11 c++14
