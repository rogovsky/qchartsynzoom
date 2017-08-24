include( common.pri )

INCLUDEPATH += $$PWD/sources
DEPENDPATH  += $$PWD/sources
VPATH       += $$PWD

DEFINES += QCHARTZOOM

# ==============================================================================

PUBLIC_HEADERS += \
    sources/qchartsynzoom.h      \
    sources/qaxissynzoomsvc.h    \
    sources/qwheelsynzoomsvc.h   \

PUBLIC_SOURCES += \
    sources/qchartsynzoom.cpp    \
    sources/qaxissynzoomsvc.cpp  \
    sources/qwheelsynzoomsvc.cpp \

# ==============================================================================

PRIVATE_HEADERS += \

PRIVATE_SOURCES += \

# ==============================================================================

uselib-qchartsynzoom : !buildlib-qchartsynzoom {
    LIBS += -L$$QCHARTSYNZOOM_LIBDIR -l$$QCHARTSYNZOOM_LIBNAME
} else {

    HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \

    SOURCES += $$PUBLIC_SOURCES $$PRIVATE_SOURCES \
}

# ==============================================================================

OBJECTS_DIR = dir.obj
MOC_DIR     = dir.moc
RCC_DIR     = dir.rcc
UI_DUR      = dir.ui

# ==============================================================================
