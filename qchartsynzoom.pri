include( common.pri )

INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD
VPATH       += $$PWD

DEFINES += QCHARTZOOM

# ==============================================================================

PUBLIC_HEADERS += \
    qchartsynzoom.h      \
    qaxissynzoomsvc.h    \
    qwheelsynzoomsvc.h   \

PUBLIC_SOURCES += \
    qchartsynzoom.cpp    \
    qaxissynzoomsvc.cpp  \
    qwheelsynzoomsvc.cpp \

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
