TEMPLATE = subdirs

SUBDIRS = glquake\
          gui    \
          gui2   \
          gui3

gui.depends = glquake
gui2.depends = glquake
gui3.depends = glquake
