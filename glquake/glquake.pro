!include(../common.pri)

TEMPLATE = lib

CONFIG -= qt
CONFIG += static

QMAKE_CFLAGS_RELEASE = -Ofast -fno-plt -fno-stack-protector

TARGET = glquake

DESTDIR = ../lib

DEFINES = GLQUAKE

INCLUDEPATH += $$PWD/Quake/WinQuake

SOURCES = Quake/WinQuake/cd_null.c       \
          Quake/WinQuake/chase.c         \
          \
          Quake/WinQuake/cl_demo.c       \
          Quake/WinQuake/cl_input.c      \
          Quake/WinQuake/cl_main.c       \
          Quake/WinQuake/cl_parse.c      \
          Quake/WinQuake/cl_tent.c       \
          \
          Quake/WinQuake/common.c        \
          Quake/WinQuake/cmd.c           \
          Quake/WinQuake/console.c       \
          Quake/WinQuake/crc.c           \
          Quake/WinQuake/cvar.c          \
          \
#         Quake/WinQuake/gl_draw.c       \
          Quake/WinQuake/gl_mesh.c       \
          Quake/WinQuake/gl_model.c      \
          Quake/WinQuake/gl_refrag.c     \
          Quake/WinQuake/gl_rlight.c     \
          Quake/WinQuake/gl_rmain.c      \
#         Quake/WinQuake/gl_rmisc.c      \
#         Quake/WinQuake/gl_rsurf.c      \
#         Quake/WinQuake/gl_screen.c     \
          Quake/WinQuake/gl_test.c       \
#         Quake/WinQuake/gl_warp.c       \
          \
          Quake/WinQuake/r_part.c        \
          \
          Quake/WinQuake/host.c          \
          Quake/WinQuake/host_cmd.c      \
          Quake/WinQuake/in_null.c       \
          Quake/WinQuake/keys.c          \
          Quake/WinQuake/mathlib.c       \
          Quake/WinQuake/menu.c          \
          \
          Quake/WinQuake/net_none.c      \
          Quake/WinQuake/net_main.c      \
          Quake/WinQuake/net_loop.c      \
          Quake/WinQuake/net_vcr.c       \
          \
          Quake/WinQuake/pr_cmds.c       \
          Quake/WinQuake/pr_edict.c      \
          Quake/WinQuake/pr_exec.c       \
          \
          Quake/WinQuake/snd_null.c      \
          \
          Quake/WinQuake/sv_main.c       \
          Quake/WinQuake/sv_phys.c       \
          Quake/WinQuake/sv_move.c       \
          Quake/WinQuake/sv_user.c       \
          \
          \
          Quake/WinQuake/sbar.c          \
          Quake/WinQuake/zone.c          \
          Quake/WinQuake/view.c          \
          Quake/WinQuake/wad.c           \
          Quake/WinQuake/world.c         \
          \
          gl_draw.c                      \
          gl_rmisc.c                     \
          gl_rsurf.c                     \
          gl_screen.c                    \
          gl_warp.c                      \
          sys_qt.c
