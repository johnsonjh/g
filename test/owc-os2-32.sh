# 32-bit OS/2 PdCurses(Mod) cross-compilation helper script for OpenWatcom 2
##############################################################################
#
# Copyright (c) 2019-2023 Jeffrey H. Johnson
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered "AS-IS",
# without any warranty.
#
##############################################################################

test -f "../curses.h" ||
  {
    printf 'ERROR: %s\n' "Not in a PDCurses port directory?"
    exit 1
  }

env rm -f ./*.err         2> /dev/null || true
env rm -f ./*.obj         2> /dev/null || true
env rm -f ./*.lib         2> /dev/null || true
env rm -f ./pdcurses.lbc  2> /dev/null || true
env rm -f ./*.a           2> /dev/null || true

export WATCOM="/opt/watcom"
export INCLUDE="${WATCOM:?}/h:${WATCOM:?}/h/os2"
export PATH="${WATCOM:?}/binl64:${PATH:?}"
export WCLFLAGS="-c -l=OS2V2 -bcl=OS2V2 -mf -fpi -d0 -zl \
                 -wx -zq -i=.. -s -oneatx -DNDEBUG"
set -eux

wcl386 ${WCLFLAGS:?} -fo=addch.obj     ../pdcurses/addch.c
wcl386 ${WCLFLAGS:?} -fo=addchstr.obj  ../pdcurses/addchstr.c
wcl386 ${WCLFLAGS:?} -fo=addstr.obj    ../pdcurses/addstr.c
wcl386 ${WCLFLAGS:?} -fo=attr.obj      ../pdcurses/attr.c
wcl386 ${WCLFLAGS:?} -fo=beep.obj      ../pdcurses/beep.c
wcl386 ${WCLFLAGS:?} -fo=bkgd.obj      ../pdcurses/bkgd.c
wcl386 ${WCLFLAGS:?} -fo=border.obj    ../pdcurses/border.c
wcl386 ${WCLFLAGS:?} -fo=clear.obj     ../pdcurses/clear.c
wcl386 ${WCLFLAGS:?} -fo=color.obj     ../pdcurses/color.c
wcl386 ${WCLFLAGS:?} -fo=delch.obj     ../pdcurses/delch.c
wcl386 ${WCLFLAGS:?} -fo=deleteln.obj  ../pdcurses/deleteln.c
wcl386 ${WCLFLAGS:?} -fo=getch.obj     ../pdcurses/getch.c
wcl386 ${WCLFLAGS:?} -fo=getstr.obj    ../pdcurses/getstr.c
wcl386 ${WCLFLAGS:?} -fo=getyx.obj     ../pdcurses/getyx.c
wcl386 ${WCLFLAGS:?} -fo=inch.obj      ../pdcurses/inch.c
wcl386 ${WCLFLAGS:?} -fo=inchstr.obj   ../pdcurses/inchstr.c
wcl386 ${WCLFLAGS:?} -fo=initscr.obj   ../pdcurses/initscr.c
wcl386 ${WCLFLAGS:?} -fo=inopts.obj    ../pdcurses/inopts.c
wcl386 ${WCLFLAGS:?} -fo=insch.obj     ../pdcurses/insch.c
wcl386 ${WCLFLAGS:?} -fo=insstr.obj    ../pdcurses/insstr.c
wcl386 ${WCLFLAGS:?} -fo=instr.obj     ../pdcurses/instr.c
wcl386 ${WCLFLAGS:?} -fo=kernel.obj    ../pdcurses/kernel.c
wcl386 ${WCLFLAGS:?} -fo=keyname.obj   ../pdcurses/keyname.c
wcl386 ${WCLFLAGS:?} -fo=mouse.obj     ../pdcurses/mouse.c
wcl386 ${WCLFLAGS:?} -fo=move.obj      ../pdcurses/move.c
wcl386 ${WCLFLAGS:?} -fo=outopts.obj   ../pdcurses/outopts.c
wcl386 ${WCLFLAGS:?} -fo=overlay.obj   ../pdcurses/overlay.c
wcl386 ${WCLFLAGS:?} -fo=pad.obj       ../pdcurses/pad.c
wcl386 ${WCLFLAGS:?} -fo=panel.obj     ../pdcurses/panel.c
wcl386 ${WCLFLAGS:?} -fo=printw.obj    ../pdcurses/printw.c
wcl386 ${WCLFLAGS:?} -fo=refresh.obj   ../pdcurses/refresh.c
wcl386 ${WCLFLAGS:?} -fo=scanw.obj     ../pdcurses/scanw.c
wcl386 ${WCLFLAGS:?} -fo=scr_dump.obj  ../pdcurses/scr_dump.c
wcl386 ${WCLFLAGS:?} -fo=scroll.obj    ../pdcurses/scroll.c
wcl386 ${WCLFLAGS:?} -fo=slk.obj       ../pdcurses/slk.c
wcl386 ${WCLFLAGS:?} -fo=termattr.obj  ../pdcurses/termattr.c
wcl386 ${WCLFLAGS:?} -fo=touch.obj     ../pdcurses/touch.c
wcl386 ${WCLFLAGS:?} -fo=util.obj      ../pdcurses/util.c
wcl386 ${WCLFLAGS:?} -fo=window.obj    ../pdcurses/window.c
wcl386 ${WCLFLAGS:?} -fo=debug.obj     ../pdcurses/debug.c
## XXX(jhj) - We don't use the clipboard - requires minor tweaking of getch
#wcl386 ${WCLFLAGS:?} -fo=pdcclip.obj   pdcclip.c
wcl386 ${WCLFLAGS:?} -fo=pdcdisp.obj   pdcdisp.c
wcl386 ${WCLFLAGS:?} -fo=pdcgetsc.obj  pdcgetsc.c
wcl386 ${WCLFLAGS:?} -fo=pdckbd.obj    pdckbd.c
wcl386 ${WCLFLAGS:?} -fo=pdcscrn.obj   pdcscrn.c
wcl386 ${WCLFLAGS:?} -fo=pdcsetsc.obj  pdcsetsc.c
wcl386 ${WCLFLAGS:?} -fo=pdcutil.obj   pdcutil.c
wlib -q -n -b -c -t pdcurses.lib *.obj
env cp -f pdcurses.lib panel.lib
