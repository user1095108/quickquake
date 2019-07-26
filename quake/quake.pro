!include(../common.pri)

TEMPLATE = lib

CONFIG -= qt
CONFIG += static

QMAKE_CFLAGS += -Ofast -no-pie -fno-plt -fno-stack-protector

TARGET = quake

DESTDIR = ../lib

DEFINES = 

SOURCES = Quake/WinQuake/mathlib.c
