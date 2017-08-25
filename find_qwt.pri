# ==============================================================================

QT *= svg

exists(useqwt.cfg) : infile(useqwt.cfg, USE_QWT, local) : CONFIG *= qwt_local
exists(useqwt.cfg) : infile(useqwt.cfg, USE_QWT, systm) : CONFIG *= qwt_systm

!qwt_local : !qwt_systm {
    message("Neither 'qwt_local' nor 'qwt_systm' found in 'useqwt.cfg' see CONFIG = '$${CONFIG}'.")

    USE_QWT = $${USE_QWT}
    !isEmpty( USE_QWT ) {
        CONFIG *= qwt_$$USE_QWT
        message("USE_QWT found in parameters,  see CONFIG = '$${CONFIG}'.")
    }

    USE_QWT = $$(USE_QWT)
    !isEmpty( USE_QWT ) {
        CONFIG *= qwt_$$USE_QWT
        message("USE_QWT found in environment, see CONFIG = '$${CONFIG}'.")
    }
}

!qwt_local : !qwt_systm {
    message("Neither 'qwt_local' nor 'qwt_systm' found in parameters/environment, see CONFIG = '$${CONFIG}'.")
    USE_QWT = qwt_systm
    CONFIG *= $${USE_QWT}
    message("USE_QWT = '$${USE_QWT}' will be used as default.")
}

# ==============================================================================

qwt_local : !qwt_systm {
    message("Local  copy of Qwt will be used.")

    QWT_ROOT = $${PWD}/3rdparty/qwt
        INCLUDEPATH += $${QWT_ROOT}/src/
        LIBS += -L$${QWT_ROOT}/lib -lqwt
    QMAKE_RPATHDIR += $${QWT_ROOT}/lib

    CONFIG *= qwt
    CONFIG -= qwt
}

qwt_systm : !qwt_local {
    message("System copy of Qwt will be used.")

    QWT_ROOT = /usr
    greaterThan(QT_MAJOR_VERSION, 4) {
        INCLUDEPATH += $${QWT_ROOT}/include/qwt
        LIBS += -L$${QWT_ROOT}/lib -lqwt
    } else {
        INCLUDEPATH += $${QWT_ROOT}/include/qwt-qt4
        LIBS += -L$${QWT_ROOT}/lib -lqwt-qt4
    }
    QMAKE_RPATHDIR += $${QWT_ROOT}/lib

    CONFIG *= qwt
    CONFIG -= qwt
}

!qwt_local : !qwt_systm {
    error("Shit happens...")
}

qwt_local : qwt_systm {
    error("Shit happens...")
}

# ==============================================================================
