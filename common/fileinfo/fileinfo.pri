INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/fileinfo.h

SOURCES += \
    $$PWD/fileinfo.cpp

windows:LIBS += libversion libpsapi
