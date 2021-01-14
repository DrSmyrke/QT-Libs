QT	+= core network sql serialport
QT	-= gui widgets

TARGET = appName
TEMPLATE = app

DEFINES += SERVER

CONFIG(debug, debug|release):CONFIGURATION=debug
CONFIG(release, debug|release):CONFIGURATION=release

CONFIG += c++11 console
CONFIG -= app_bundle
QMAKE_CXXFLAGS += "-std=c++11"

CONFIG(debug, debug|release) {
    unix: TARGET = $$join(TARGET,,,_debug)
    else: TARGET = $$join(TARGET,,,d)
}

OBJECTS_DIR         = ../build/obj/$${CONFIGURATION}
MOC_DIR             = ../build/$${CONFIGURATION}
RCC_DIR             = ../build/rcc
UI_DIR              = ../build/ui
DESTDIR             = ../bin

SOURCES += main.cpp \
    ../../proto.cpp \
    core.cpp \
    database.cpp \
    global.cpp \
    socketclient.cpp \
    socketserver.cpp

HEADERS += core.h \
    ../../proto.h \
    database.h \
    global.h \
    socketclient.h \
    socketserver.h

exists(./gitversion.pri):include(./gitversion.pri)
exists(./myLibs.pri):include(./myLibs.pri)
exists(index.rc){
    win32|win64{
        RC_FILE=  index.rc
        OTHER_FILES+= index.rc
        DISTFILES += index.rc
    }
}
