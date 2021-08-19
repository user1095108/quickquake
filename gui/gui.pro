!include(../common.pri) {
  error("")
}

CONFIG += qtquickcompiler

TEMPLATE = app

HEADERS = $$files(*.hpp)
SOURCES = $$files(*.cpp)
RESOURCES = gui.qrc

QT += qml quick

CONFIG += precompile_header
PRECOMPILED_HEADER  = "../precompile.hpp"

*-g++* {
  POST_TARGETDEPS += ../lib/libglquake.a
}

LIBS += -L ../lib -lglquake
