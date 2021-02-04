QT += serialport widgets

include(./common/config/config.pri)
include(./common/fileinfo/fileinfo.pri)

RESOURCES += \
    can_terminal.qrc

FORMS += \
    consoleform.ui \
    filterdialog.ui \
    mainwindow.ui \
    senddialog.ui

DISTFILES += \
    can_terminal.rc

HEADERS += \
    com_func.h \
    consoleform.h \
    filterdialog.h \
    mainwindow.h \
    proto.h \
    senddialog.h

SOURCES += \
    com_func.cpp \
    consoleform.cpp \
    filterdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    proto.cpp \
    senddialog.cpp
windows:RC_FILE=can_terminal.rc
