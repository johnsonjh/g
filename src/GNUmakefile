###############################################################################
# G  --  GNUmakefile
###############################################################################
# vi: filetype=make:tabstop=8:tw=79
###############################################################################

###############################################################################
#
#  Building G for legacy systems:
#
#  * HP-UX 9/10:
#     cc -O -Ae +Oall +Oaggressive +ESlit -D_XPG4 +u4 -o g -n \
#        -Wl,-a,archive g.c -lcur_colr
#
#  * ICL DRS/NX (SPARC/Intel):
#     cc -Xc -O g.c -dn -o g -lcurses
#
#  * UnixWare:
#     cc -Xc -O g.c -dn -o g -lcurses
#
#  * Solaris (Intel):
#     cc -Xc -xO3 -Di386=1 g.c -dn -o g -lcurses
#
#  * Solaris (SPARC):
#     cc -Xc -xO3 -dalign g.c -dn -o g -lcurses
#
#  * IBM AIX 3.25 & 4.12:
#     cc -O3 -qflag=e:e -qcompact -qro -qroconst -o g g.c -lcurses
#
#  * DG/UX AViiON 4.1 (Motorola):
#     cc -Xc -O2 -D_USING_SYSV4_OR_DGUX g.c -dn -o g -lcurses
#
#  * Pyramid DC/OSx:
#     cc -Xc -WM,-mips2 -WM,-O3 -Wf,-Cs g.c -dn -o g -lcurses
#
#  * SCO 5:
#     cc -O -Xc -o g g.c -lcurses
#
#  * DOS (Watcom C 10, 8086):
#     wasm crt0 -bt=DOS -ml -2r_
#     wcl -DDOS=1 -DASM86=0 -0 -7 -ml -zp2 -oaosilm @g.lnk \
#       -fe=g.exe crt0.obj g.c
#
#  * DOS (Watcom C 10, 286):
#     wasm crt0 -bt=DOS -ml -2r_
#     wcl -DDOS=1 -DASM86=0 -2 -7 -fp2 -ml -zp2 -oaosilm @g.lnk \
#       -fe=g.exe crt0.obj g.c
#
#  * DOS (Watcom C 10, 386):
#     wasm crt0 -bt=DOS -ml -2r_
#     wcl -DDOS=1 -4 -7 -fp3 -ml -zp2 -oaosilm @g.lnk \
#       -fe=g.exe crt0.obj g.c
#
#  * Use `-DDEBUG=1` to enable extra debugging checks
#  * Use `-DCOLOUR=0` for older curses implementations
#
#  The above do: inline floating point, 2 byte struct alignment,
#  large model, optimize for small size, and no stack check. g.lnk
#  just has "option eliminate" in to get rid of unreferenced segments.
#
#  crt0.asm is optional; for use with Watcom C only. It's ~2 KiB smaller
#  and much faster than the default (uses stosd instead of stosb to zero
#  the BSS region for example). If it isn't used, -k8192 must be added to
#  set the stack size.
#
#  G also compiles with Microsoft Visual C/C++ and Borland C. Look for "MSVC"
#  in the comments. 16-bit DOS versions need large memory model and 8 KB stack.
#
###############################################################################

###############################################################################
# Utilities

CC      ?= cc
GETCONF ?= getconf
PRINTF  ?= printf
RM      ?= rm -f
SSTRIP  ?= sstrip
STRIP   ?= strip
TEST    ?= test
TR      ?= tr
TRUE    ?= true
UNAME   ?= uname

###############################################################################
# Detect operating system

ifndef OS
    OS=$(shell $(UNAME) -s 2> /dev/null | \
        $(TR) '[:upper:]' '[:lower:]' 2> /dev/null)
endif # OS
ifeq ($(OS),sunos)
    OS=$(shell $(UNAME) -o 2> /dev/null | \
        $(TR) '[:upper:]' '[:lower:]' 2> /dev/null)
    _SUNOS = 1
endif # sunos

###############################################################################
# Verbose

ifdef V
  VERBOSE = set -x
else
  VERBOSE = set +x
endif # !V

###############################################################################
# Debugging

ifdef DEBUG
  CFLAGS  += -Wall -Wextra -DDEBUG=1 -g
else
 OPTFLAGS ?= -Os
  CFLAGS  += $(OPTFLAGS) -D_FORTIFY_SOURCE=2 -Wno-int-to-pointer-cast
endif # DEBUG

###############################################################################
# Link-time optimization and garbage collection

ifdef LTO
  CFLAGS    += -flto
  LDFLAGS   += -flto
endif # LTO

ifdef LGC
  CFLAGS    += -fdata-sections -ffunction-sections
  LDFLAGS   += -Wl,--gc-sections
  ifdef DEBUG
    LDFLAGS += -Wl,--print-gc-sections
  endif # DEBUG
endif # LGC

###############################################################################
# Libraries

ifeq ($(OS),aix)
  CFLAGS    += -I/opt/freeware/include
  LDFLAGS   += -L/opt/freeware/lib
  CURSESLIB ?= -lncurses
else
  CURSESLIB ?= -lcurses
endif # aix
LDFLAGS     += $(CURSESLIB)

###############################################################################
# Architecture

ifeq ($(OS),aix)
  MAIXBITS  ?= $(shell command -p $(GETCONF) KERNEL_BITMODE 2> /dev/null || \
                 $(PRINTF) '%s' "32")
  ifneq (,$(findstring gcc,$(CC))) # gcc (GNU C)
    CFLAGS  += -maix$(MAIXBITS)
    LDFLAGS += -maix$(MAIXBITS) -Wl,-b$(MAIXBITS)
  endif # gcc
  ifneq (,$(findstring clang,$(CC))) # xlclang/ibm-clang (IBM Open XL)
    CFLAGS  += -m$(MAIXBITS)
    LDFLAGS += -m$(MAIXBITS) -Wl,-b$(MAIXBITS)
  endif # clang
  ifneq (,$(findstring gxlc,$(CC))) # gxlc (IBM XL C)
    CFLAGS  += -m$(MAIXBITS)
    LDFLAGS += -m$(MAIXBITS) -Wl,-b$(MAIXBITS)
  endif # gxlc
endif # aix

###############################################################################
# Files

SRC = g.c
OUT = g$(OUTEXT)

###############################################################################
# Compile

.PHONY: all
all: $(OUT)

$(OUT): $(SRC)
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(CC):\t\t%s\n' $<
endif # !V
	@$(VERBOSE); $(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

###############################################################################
# Strip

ifeq ($(OS),aix)
export OBJECT_MODE=32_64
endif # aix
.PHONY: strip
strip: $(OUT)
ifneq ($(V),1)
	-@$(TEST) -f ./$(OUT) && \
		$(PRINTF) '\r\t\t$(STRIP):\t\t%s\n' $(OUT) || $(TRUE)
endif # !V
	-@$(VERBOSE); $(STRIP)     $(OUT) 2> /dev/null || $(TRUE)
	-@$(VERBOSE); $(STRIP)  -s $(OUT) 2> /dev/null || $(TRUE)
	-@$(VERBOSE); $(STRIP)  -R .gnu.build.attributes   \
                                -R .note.gnu.build-id      \
                                -R .note.gnu.property      \
                                -R .got                    \
                                -R .eh_frame               \
                                -R .eh_frame_hdr           \
                                -R .gnu.version            \
                                -R .comment                \
                                -R .SUNW_ctf               \
                                -R .jcr                    \
                                -R .note.netbsd.pax        \
                                   $(OUT) 2> /dev/null || $(TRUE)
	-@$(VERBOSE); $(SSTRIP) -z $(OUT) 2> /dev/null || $(TRUE)

###############################################################################
# Clean

.PHONY: clean distclean
clean distclean:
ifneq ($(V),1)
	-@$(TEST) -f ./$(OUT) && \
		$(PRINTF) '\r\t\t$(RM):\t\t%s\n' $(OUT) || $(TRUE)
endif # !V
	-@$(VERBOSE); $(TEST) -f ./$(OUT) && \
		$(RM) ./$(OUT) || $(TRUE)

###############################################################################
