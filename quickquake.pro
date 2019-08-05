TEMPLATE = subdirs

SUBDIRS = glquake\
          gui    \
          gui2

gui.depends = glquake
gui2.depends = glquake
