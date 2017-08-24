TEMPLATE = subdirs
CONFIG  += ordered

exists(3rdparty) : SUBDIRS += 3rdparty

include(common.pri)
uselib-qchartsynzoom : SUBDIRS += buildlib
SUBDIRS += examples
