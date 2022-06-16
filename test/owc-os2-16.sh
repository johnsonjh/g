# 16-bit OS/2 PdCurses(Mod) cross-compilation helper script for OpenWatcom 2
##############################################################################
#
# Copyright (c) 2019-2022 Jeffrey H. Johnson <trnsz@pobox.com>
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
export INCLUDE="${WATCOM:?}/h:${WATCOM:?}/h/os21x"
export PATH="${WATCOM:?}/binl64:${PATH:?}"
export WCLFLAGS="-c -l=OS2 -bcl=OS2 -ml -fpi -zm -d0 -za \
                 -wx -zq -i=.. -s -oneatx -DNDEBUG"
set -eux

wcl ${WCLFLAGS:?} -fo=addch.obj     ../pdcurses/addch.c
wcl ${WCLFLAGS:?} -fo=addchstr.obj  ../pdcurses/addchstr.c
wcl ${WCLFLAGS:?} -fo=addstr.obj    ../pdcurses/addstr.c
wcl ${WCLFLAGS:?} -fo=attr.obj      ../pdcurses/attr.c
wcl ${WCLFLAGS:?} -fo=beep.obj      ../pdcurses/beep.c
wcl ${WCLFLAGS:?} -fo=bkgd.obj      ../pdcurses/bkgd.c
wcl ${WCLFLAGS:?} -fo=border.obj    ../pdcurses/border.c
wcl ${WCLFLAGS:?} -fo=clear.obj     ../pdcurses/clear.c
wcl ${WCLFLAGS:?} -fo=color.obj     ../pdcurses/color.c
wcl ${WCLFLAGS:?} -fo=delch.obj     ../pdcurses/delch.c
wcl ${WCLFLAGS:?} -fo=deleteln.obj  ../pdcurses/deleteln.c
wcl ${WCLFLAGS:?} -fo=getch.obj     ../pdcurses/getch.c
wcl ${WCLFLAGS:?} -fo=getstr.obj    ../pdcurses/getstr.c
wcl ${WCLFLAGS:?} -fo=getyx.obj     ../pdcurses/getyx.c
wcl ${WCLFLAGS:?} -fo=inch.obj      ../pdcurses/inch.c
wcl ${WCLFLAGS:?} -fo=inchstr.obj   ../pdcurses/inchstr.c
wcl ${WCLFLAGS:?} -fo=initscr.obj   ../pdcurses/initscr.c
wcl ${WCLFLAGS:?} -fo=inopts.obj    ../pdcurses/inopts.c
wcl ${WCLFLAGS:?} -fo=insch.obj     ../pdcurses/insch.c
wcl ${WCLFLAGS:?} -fo=insstr.obj    ../pdcurses/insstr.c
wcl ${WCLFLAGS:?} -fo=instr.obj     ../pdcurses/instr.c
wcl ${WCLFLAGS:?} -fo=kernel.obj    ../pdcurses/kernel.c
wcl ${WCLFLAGS:?} -fo=keyname.obj   ../pdcurses/keyname.c
wcl ${WCLFLAGS:?} -fo=mouse.obj     ../pdcurses/mouse.c
wcl ${WCLFLAGS:?} -fo=move.obj      ../pdcurses/move.c
wcl ${WCLFLAGS:?} -fo=outopts.obj   ../pdcurses/outopts.c
wcl ${WCLFLAGS:?} -fo=overlay.obj   ../pdcurses/overlay.c
wcl ${WCLFLAGS:?} -fo=pad.obj       ../pdcurses/pad.c
wcl ${WCLFLAGS:?} -fo=panel.obj     ../pdcurses/panel.c
wcl ${WCLFLAGS:?} -fo=printw.obj    ../pdcurses/printw.c
wcl ${WCLFLAGS:?} -fo=refresh.obj   ../pdcurses/refresh.c
wcl ${WCLFLAGS:?} -fo=scanw.obj     ../pdcurses/scanw.c
wcl ${WCLFLAGS:?} -fo=scr_dump.obj  ../pdcurses/scr_dump.c
wcl ${WCLFLAGS:?} -fo=scroll.obj    ../pdcurses/scroll.c
wcl ${WCLFLAGS:?} -fo=slk.obj       ../pdcurses/slk.c
wcl ${WCLFLAGS:?} -fo=termattr.obj  ../pdcurses/termattr.c
wcl ${WCLFLAGS:?} -fo=touch.obj     ../pdcurses/touch.c
wcl ${WCLFLAGS:?} -fo=util.obj      ../pdcurses/util.c
wcl ${WCLFLAGS:?} -fo=window.obj    ../pdcurses/window.c
wcl ${WCLFLAGS:?} -fo=debug.obj     ../pdcurses/debug.c
## XXX(jhj) - We don't use the clipboard - requires minor tweaking of getch
#wcl ${WCLFLAGS:?} -fo=pdcclip.obj   pdcclip.c
wcl ${WCLFLAGS:?} -fo=pdcdisp.obj   pdcdisp.c
wcl ${WCLFLAGS:?} -fo=pdcgetsc.obj  pdcgetsc.c
wcl ${WCLFLAGS:?} -fo=pdckbd.obj    pdckbd.c
wcl ${WCLFLAGS:?} -fo=pdcscrn.obj   pdcscrn.c
wcl ${WCLFLAGS:?} -fo=pdcsetsc.obj  pdcsetsc.c
wcl ${WCLFLAGS:?} -fo=pdcutil.obj   pdcutil.c
wlib -q -n -b -c -t pdcurses.lib *.obj
env cp -f pdcurses.lib panel.lib
