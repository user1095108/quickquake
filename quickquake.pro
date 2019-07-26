TEMPLATE = subdirs

SUBDIRS = quake\
          gui

gui.depends = quake
