TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \

#
# NOTE!!!
# for ( works fine for qmake-qt5 only
# for(  works fine for qmake-qt5/qmake-qt4
#

EXTRAS = qwt
for(dir, EXTRAS) {
    exists($$dir) {
        message(3rdparty@qchartsynzoom: "Going to build SUBPROJECT = $$dir")
        DEPENDPATH += $$dir
        SUBDIRS += $$dir
    }
}
