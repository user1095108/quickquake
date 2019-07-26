!include(../common.pri) {
  error("")
}

TEMPLATE = app

CONFIG += qt
QT += qml quick

CONFIG += precompile_header
PRECOMPILED_HEADER  = "../precompile.hpp"

*-g++* {
  POST_TARGETDEPS += ../lib/libquake.a
}

HEADERS = $$files(*.hpp)
SOURCES = $$files(*.cpp)

LIBS += -L ../lib -lquake
