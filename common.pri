INCLUDEPATH += $$PWD

MOC_DIR = .moc
OBJECTS_DIR = .obj
UI_DIR = .ui

CONFIG += no_lflags_merge exceptions_off rtti_off stl thread warn_on c++latest
CONFIG -= exceptions rtti

DEFINES +=                 \
  QT_USE_FAST_CONCATENATION\
  QT_USE_FAST_OPERATOR_PLUS\
  QT_NO_CAST_FROM_ASCII    \
  QT_NO_CAST_TO_ASCII      \
  QT_NO_CAST_FROM_BYTEARRAY
win32:debug:CONFIG += console

CONFIG(release, debug|release):QMAKE_RESOURCE_FLAGS += -compress 9

*-g++* {
  QMAKE_CFLAGS = -Wall -Wextra -Wno-missing-field-initializers -Wno-multichar -fno-stack-protector -fno-plt
  QMAKE_CXXFLAGS = -Wall -Wextra -Wno-missing-field-initializers -Wno-multichar -fno-stack-protector -fno-plt

  QMAKE_CFLAGS_RELEASE *= -DNDEBUG -g
  QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG -g

  unix:QMAKE_CXXFLAGS_DEBUG += -fsanitize=address,undefined
  unix:QMAKE_LFLAGS_DEBUG += -fsanitize=address,undefined

  QMAKE_LFLAGS += -fno-stack-protector -fuse-ld=gold
}
