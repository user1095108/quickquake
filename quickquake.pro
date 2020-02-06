TEMPLATE = subdirs

SUBDIRS = glquake\
          gui

gui.depends = glquake
