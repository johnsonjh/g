###############################################################################
# G  --  GNUmakefile
###############################################################################
# vi: filetype=make:tabstop=8:tw=79
###############################################################################

###############################################################################
#
# Copyright (c) 2019-2022 Jeffrey H. Johnson <trnsz@pobox.com>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered "AS-IS",
# without any warranty.
#
###############################################################################

###############################################################################
#
#  Building G with GNU Make 3.81:
#
#  * The following options affecting compilation may be set in the shell
#    environment or on the make command-line, for example, `make OPTION=1`:
#
#    * `CC`: Overrides default C compiler selection, e.g. `CC=clang`
#    * `LTO=1`: Enables link-time optimization
#    * `LGC=1`: Enables link-time garbage collection (reducing binary size)
#    * `OPTFLAGS`: Overrides default optimization flags, e.g. `OPTFLAGS=-Oz`
#    * `V=1`: Enables verbose compilation messages
#    * `NOSSP=1`: Disables `FORTIFY_SOURCE` and compiler stack protections
#    * `STATIC=1`: Attempt statically linking all required libraries
#    * `DUMA=1`: Enables support for the *DUMA* memory debugging library
#    * `DSTATIC=1`: Attempt statically linking *DUMA* library (needs GNU ld)
#    * `DEBUG=1`: Enables debugging code (for development or troubleshooting)
#    * `LINE_G=1`: Builds only the line-mode editor (no curses library needed)
#    * `TINY_G=1`: Disables features for lower memory use (default for PC DOS)
#    * `FULL_G=1`: Overrides `TINY_G` default (enables a fully-featured build)
#    * `CURSESLIB`: Overrides curses library selection, e.g. `-lcurses -ltinfo`
#    * `NOCURSES`: Disables linking curses (often used for cross compilations)
#    * `COLOUR=0`: Disables colours (required for some older curses libraries)
#
###############################################################################

###############################################################################
#
#  Building G for legacy systems:
#
#  * Commodore Amiga UNIX (AMIX):
#     cc -O g.c -o g -lcurses
#
#  * HP-UX 9/10:
#     cc -O -Ae +Oall +Oaggressive +ESlit -D_XPG4 +u4 -o g -n \
#        -Wl,-a,archive g.c -lcur_colr
#
#  * ICL DRS/NX (SPARC/Intel):
#     cc -Xc -O g.c -dn -o g -lcurses
#
#  * Novell/SCO UnixWare:
#     cc -Xc -O g.c -dn -o g -lcurses
#
#  * Solaris (Intel) Legacy:
#     cc -Xc -xO3 -Di386=1 g.c -dn -o g -lcurses
#
#  * Solaris (SPARC) Legacy:
#     cc -Xc -xO3 -dalign g.c -dn -o g -lcurses
#
#  * IBM AIX Legacy (3.25 & 4.12):
#     cc -O3 -qflag=e:e -qcompact -qro -qroconst -o g g.c -lcurses
#
#  * DG/UX AViiON 4.1 (Motorola):
#     cc -Xc -O2 -D_USING_SYSV4_OR_DGUX g.c -dn -o g -lcurses
#
#  * Pyramid DC/OSx:
#     cc -Xc -WM,-mips2 -WM,-O3 -Wf,-Cs g.c -dn -o g -lcurses
#
#  * SCO OpenServer 5:
#     cc -O -Xc -o g g.c -lcurses
#
#  * DOS (Borland C 5.02, 8086):
#     bcc -1- -2- -3- -DDOS=1 -UUNIX -Os -ml -a2 -d g.c
#
#  * DOS (Microsoft C/C++ 7.00b/8.00a, MSVC 1.52c, 8086):
#     cl /Gs /Gy -DDOS=1 -UUNIX /AL /Os /G0 /c G.C
#     link /STACK:32767 G.OBJ,G.EXE,,,,
#
#   Definitions for compiling G:
#
#   * `-DDEBUG=1`: Enables debugging code (for development or troubleshooting)
#   * `-DLINE_G=1`: Builds only the line-mode editor (no curses library needed)
#   * `-DCOLOUR=0`: Disables colours (required for some older curses libraries)
#   * `-DTINY_G=1`: Disables features for lower memory use (default on for DOS)
#   * `-DFULL_G=1`: Overrides `TINY_G` default (enables a fully-featured build)
#
###############################################################################

###############################################################################
# Utilities

CAT             ?= cat
CC              ?= cc
CHMOD           ?= chmod
ENV             ?= env
GETCONF         ?= getconf
MKTEMP          ?= mktemp
LN              ?= ln
PKGCFG          ?= pkg-config
PRINTF          ?= printf
RM              ?= rm -f
SCRIPT          ?= script
SSTRIP          ?= sstrip
STRIP           ?= strip
TEST            ?= test
TR              ?= tr
TRUE            ?= true
UNAME           ?= uname
WATWCL          ?= wcl
WCL386          ?= wcl386

###############################################################################
# OpenWatcom configuration

WATCOM          ?= /opt/watcom
WATBIN          ?= binl64
EXTENDER        ?= DOS32A

###############################################################################
# DJGPP configuration

DJGPP_GHOME     ?= /opt/djgpp
DJGPP_GARCH     ?= i586-pc-msdosdjgpp-
DJGPP_CCOMP     ?= gcc
DJGPP_ECOFF     ?= exe2coff
ifdef DEBUG
   DJGPP_STRIP  ?= size
else # DEBUG
   DJGPP_STRIP  ?= strip
endif # !DEBUG

###############################################################################
# CWSDPMI configuration

CWSDPMI_STUB    ?= /opt/cwspdmi/cwsdstub.exe

###############################################################################
# Detect operating system

ifndef OS
    OS=$(shell $(UNAME) -s 2> /dev/null | \
        $(TR) '[:upper:]' '[:lower:]' 2> /dev/null)
endif # OS
ifeq ($(OS),sunos)
    OS=$(shell $(UNAME) -o 2> /dev/null | \
        $(TR) '[:upper:]' '[:lower:]' 2> /dev/null)
    _SUNOS       = 1
endif # sunos

###############################################################################
# Static build

ifdef STATIC
  STATIC         = 1
  LDFLAGS       += -static
  STFLAGS       := --static
endif # STATIC

###############################################################################
# pkg-config configuration

ifndef NOCURSES
  ifndef LINE_G
    ifndef CURSESLIB
      CFLAGS    += $(shell $(PKGCFG) ncurses --cflags $(STFLAGS) 2> /dev/null \
                         || $(PKGCFG) curses --cflags $(STFLAGS) 2> /dev/null)
      CURSESLIB += $(shell $(PKGCFG) ncurses --libs $(STFLAGS) 2> /dev/null   \
                         || $(PKGCFG) curses --libs $(STFLAGS) 2> /dev/null)
    endif #!CURSESLIB
  endif # !LINE_G
endif # !NOCURSES

###############################################################################
# DUMA build

ifdef DUMA
  DUMA           = 1
  NOSSP          = 1
  CFLAGS        += -DDUMA=1
  WGFLAGS       += -DDUMA=1
  ifdef DSTATIC
    DUMALIB     += -l:libduma.a
  else # DSTATIC
    DUMALIB     += -lduma
  endif # !DSTATIC
endif # DUMA

###############################################################################
# Full build

ifdef FULL_G
  CFLAGS        += -DFULL_G=1 -DTINY_G=0
  WGFLAGS       += -DFULL_G=1 -DTINY_G=0
endif # FULL_G

###############################################################################
# Line-mode build

ifdef LINE_G
  NOCURSES       = 1
  TINY           =
  CFLAGS        += -DLINE_G=1
  WGFLAGS       += -DLINE_G=1
endif # LINE_G

###############################################################################
# Tiny build

ifdef TINY
  CFLAGS        += -DTINY_G=1
  WGFLAGS       += -DTINY_G=1
endif # TINY
ifdef TINY_G
  ifndef TINY
    CFLAGS      += -DTINY_G=1
    WGFLAGS     += -DTINY_G=1
  endif # !TINY
endif # TINY_G

###############################################################################
# Disable colour

ifeq "$(COLOUR)" "0"
  CFLAGS        += -DCOLOUR=0
  WGFLAGS       += -DCOLOUR=0
endif # !COLOUR

###############################################################################
# Verbose mode

ifdef V
  VERBOSE        = set -x
else # V
  VERBOSE        = set +x
endif # !V

###############################################################################
# Stack protection

ifdef DEBUG
  NOSSP          = 1
endif # DEBUG

ifdef NOSSP
  ifeq ($(_SUNOS),1)
    STACK_FLAGS ?= -U_FORTIFY_SOURCE
  else # _SUNOS
    STACK_FLAGS ?= -fno-stack-protector -U_FORTIFY_SOURCE
  endif # !_SUNOS
else # NOSSP
  STACK_FLAGS   ?= -D_FORTIFY_SOURCE=2
endif # !NOSSP

###############################################################################
# Debugging

ifdef DEBUG
  CFLAGS        += -Wall -Wextra $(STACK_FLAGS) -DDEBUG=1 -g
else # DEBUG
  ifeq ($(_SUNOS),1)
    OPTFLAGS    ?= -O
  endif # _SUNOS
  OPTFLAGS      ?= -Os
  CFLAGS        += $(OPTFLAGS) $(STACK_FLAGS) -Wno-int-to-pointer-cast
endif # !DEBUG

###############################################################################
# Link-time optimization and garbage collection

ifdef LTO
  CFLAGS        += -flto=auto
  LDFLAGS       += -flto=auto
endif # LTO

ifdef LGC
  CFLAGS        += -fdata-sections -ffunction-sections
  LDFLAGS       += -Wl,--gc-sections
  ifdef DEBUG
    LDFLAGS     += -Wl,--print-gc-sections
  endif # DEBUG
endif # LGC

###############################################################################
# Libraries

ifeq ($(OS),aix)
  CFLAGS        += -I/opt/freeware/include
  LDFLAGS       += -L/opt/freeware/lib
  ifndef LINE_G
    ifeq ($(CURSESLIB),)
      CURSESLIB := -lncurses
    endif # !CURSESLIB
  endif # !LINE_G
else # aix
  ifndef LINE_G
    ifeq ($(CURSESLIB),)
      CURSESLIB ?= -lcurses
    endif # !CURSESLIB
  endif # !LINE_G
endif # !aix
ifndef NOCURSES
  ifndef LINE_G
    LDFLAGS     += $(CURSESLIB)
  endif # !LINE_G
endif # !NOCURSES

###############################################################################
# Architectures

ifeq ($(OS),aix)
  MAIXBITS      ?= $(shell command -p $(GETCONF) KERNEL_BITMODE 2> /dev/null  \
                    || $(PRINTF) '%s' "32")
  ifneq (,$(findstring gcc,$(CC))) # gcc (GNU C)
    CFLAGS      += -maix$(MAIXBITS)
    LDFLAGS     += -maix$(MAIXBITS) -Wl,-b$(MAIXBITS)
  endif # gcc
  ifneq (,$(findstring clang,$(CC))) # xlclang/ibm-clang (IBM Open XL)
    CFLAGS      += -m$(MAIXBITS)
    LDFLAGS     += -m$(MAIXBITS) -Wl,-b$(MAIXBITS)
  endif # clang
  ifneq (,$(findstring gxlc,$(CC))) # gxlc (IBM XL C)
    CFLAGS      += -m$(MAIXBITS)
    LDFLAGS     += -m$(MAIXBITS) -Wl,-b$(MAIXBITS)
  endif # gxlc
  STRARGS        = -X$(MAIXBITS)
endif # aix

###############################################################################
# Files

SRC              = g.c
OUT             ?= g

###############################################################################
# Standard Compilation

ifdef DUMA
  LDFLAGS       += $(DUMALIB)
endif # DUMA

.PHONY: all
.NOTPARALLEL: all g
$(OUT) all:

$(OUT): $(SRC)
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(CC):\t\t%s\n' $<
endif # !V
	@$(VERBOSE); $(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

###############################################################################
# OpenWatcom Debugging

ifdef DEBUG
   WATDBG        = -d3 -DDEBUG=1
else # DEBUG
   WATDBG        = -d0 -s -zq -oabls
endif # !DEBUG

###############################################################################
# OpenWatcom Compilation: 16-bit DOS, 8086 (Real mode)

.PHONY: $(OUT)86 owc16
.NOTPARALLEL: $(OUT)86 owc16 $(OUT)86.exe
$(OUT)86 owc16 $(OUT)86.exe: g.lkr
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WATWCL):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "g86.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)" INCLUDE="$(WATCOM)/h"          \
           PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WATWCL) -k32767            \
             -bcl=DOS -DDOS=1 -DASM86=0 -0 -ml -fpi @g.lkr $(WGFLAGS)         \
               $(WATDBG) -zp2 -j -s -fe=$(OUT)86.exe $(SRC) &&                \
                 $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# OpenWatcom Compilation: 16-bit DOS, 80386 (Real mode)

.PHONY: $(OUT)386r owc386r
.NOTPARALLEL: $(OUT)386r owc386r $(OUT)386r.exe
$(OUT)386r owc386r $(OUT)386r.exe: g.lkr
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WATWCL):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "g386r.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)" INCLUDE="$(WATCOM)/h"          \
           PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WATWCL) -k32767            \
             -bcl=DOS -DDOS=1 -DASM86=1 -3 -ml -fpi @g.lkr $(WGFLAGS)         \
               $(WATDBG) -zp2 -j -s -fe=$(OUT)386r.exe $(SRC) &&              \
                 $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# OpenWatcom Compilation: 32-bit DOS, 386 (Protected mode)

.PHONY: $(OUT)386p owc386p
.NOTPARALLEL: $(OUT)386p owc386p $(OUT)386p.exe
$(OUT)386p owc386p $(OUT)386p.exe: g.lkr
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WCL386):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "./g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "./g386p.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)" INCLUDE="$(WATCOM)/h"          \
         PATH="$(WATCOM)/$(WATBIN):$(WATCOM)/binw:$${PATH:?}" $(WCL386) -s    \
          -l=$(EXTENDER) -bt=DOS -DDOS=1 -DASM86=0 -3r -mf -j -fpi @g.lkr     \
           -k32767 $(WATDBG) -zp2 -DWCL386=1 -fe=$(OUT)386p.exe $(WGFLAGS)    \
            $(SRC) && $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# OpenWatcom Compilation: 16-bit OS/2, 286 (Protected mode)

.PHONY: $(OUT)286p owc286p
.NOTPARALLEL: $(OUT)286p owc286p $(OUT)286p.exe
$(OUT)286p owc286p $(OUT)286p.exe: gos2.lkr
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WATWCL):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "./g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "./g286p.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)"                                \
         INCLUDE="$(WATCOM)/h:$(WATCOM)/h/os21x:$${WATCOM_PDC_INCLUDE:-.}"    \
          PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WATWCL)                     \
           -bcl=OS2 -UDOS -DUNIX=1 -2 -ml -fpi @gos2.lkr -k32767 -s           \
            $(WATDBG) -D_TIMESPEC_DEFINED -za -zp2 -j -fe=$(OUT)286p.exe      \
             $(WGFLAGS) $(SRC) $${WATCOM_PDC_LIBRARY:-} &&                    \
              $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# OpenWatcom Compilation: 32-bit OS/2, 386 (Protected mode)

.PHONY: $(OUT)os2p owcos2p
.NOTPARALLEL: $(OUT)os2p owcos2p $(OUT)os2p.exe
$(OUT)os2p owcos2p $(OUT)os2p.exe: gos2.lkr
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WCL386):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "./g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "./gos2p.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)"                                \
         INCLUDE="$(WATCOM)/h:$(WATCOM)/h/os2:$${WATCOM_PDC_INCLUDE:-.}"      \
          PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WCL386)                     \
           -bcl=OS2V2 -UDOS -DUNIX=1 -3r -mf -fpi @gos2.lkr -k32767 -s        \
            $(WATDBG) -D_TIMESPEC_DEFINED -zp2 -j -fe=$(OUT)os2p.exe          \
             $(WGFLAGS) $(SRC) $${WATCOM_PDC_LIBRARY:-} &&                    \
              $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# DJGPP Compilation: 32-bit DOS, 386 (Protected mode)

CFLAGZ  := $(patsubst %-flto=auto,%-flto,$(CFLAGS))
LDFLAGZ := $(patsubst %-flto=auto,%-flto,$(LDFLAGS))

.PHONY: $(OUT)386 djgpp
$(OUT)386 djgpp $(OUT)386.exe:                                                \
                              $(CWSDPMI_STUB)                                 \
                              $(DJGPP_GHOME)/bin/$(DJGPP_GARCH)$(DJGPP_CCOMP) \
                              $(DJGPP_GHOME)/bin/$(DJGPP_GARCH)$(DJGPP_STRIP)
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(DJGPP_CCOMP):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "gt" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "gt.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "g.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "g386.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); "$(DJGPP_GHOME)/bin/$(DJGPP_GARCH)$(DJGPP_CCOMP)"        \
         -march=i386 -Wno-attributes $(CFLAGZ) $(SRC) -o gt.exe $(LDFLAGZ) && \
          $(RM) -f "./gt" &&                                                  \
           "$(DJGPP_GHOME)/bin/$(DJGPP_GARCH)$(DJGPP_STRIP)" -s "./gt.exe" && \
             $(DJGPP_ECOFF) "./gt.exe" && $(RM) -f "./gt.exe" &&              \
              $(CAT) "$(CWSDPMI_STUB)" "./gt" > "./$(OUT)386.exe" &&          \
               $(RM) -f "./gt" 2> /dev/null && $(CHMOD) a+x "./$(OUT)386.exe"

###############################################################################
# Strip

.PHONY: strip
strip: $(OUT)
ifneq ($(V),1)
	-@$(TEST) -f ./$(OUT) && \
          $(PRINTF) '\r\t\t$(STRIP):\t\t%s\n' $(OUT) || $(TRUE)
endif # !V
	-@$(VERBOSE); $(STRIP) $(STRARGS)    $(OUT) 2> /dev/null || $(TRUE)
	-@$(VERBOSE); $(STRIP) $(STRARGS) -s $(OUT) 2> /dev/null || $(TRUE)
	-@$(VERBOSE); $(STRIP) $(STRARGS) -R .gnu.build.attributes            \
                                          -R .note.gnu.build-id               \
                                          -R .note.gnu.property               \
                                          -R .eh_frame                        \
                                          -R .eh_frame_hdr                    \
                                          -R .got                             \
                                          -R .SUNW_ctf                        \
                                          -R .jcr                             \
                                          -R .note.netbsd.pax                 \
                                   $(OUT) 2> /dev/null || $(TRUE)
	-@$(VERBOSE); $(SSTRIP) -z $(OUT) 2> /dev/null || $(TRUE)

###############################################################################
# Clean

.PHONY: clean distclean
clean distclean:
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(RM):\t\t%s\n' "(output)" || $(TRUE)
endif # !V
	-@$(VERBOSE); $(RM) -f "./gt"             2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./gt.o"           2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./gt.exe"         2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT)"         2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT).o"       2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT).exe"     2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT).err"     2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT)86.exe"   2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT)386.exe"  2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT)286p.exe" 2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT)os2p.exe" 2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT)386p.exe" 2>/dev/null || $(TRUE)
	-@$(VERBOSE); $(RM) -f "./$(OUT)386r.exe" 2>/dev/null || $(TRUE)

###############################################################################
# Tests

# WARNING: GNU and Linux-isms abound; must be adaptated to your local system!

.PHONY: test check
test check: g.c ../test/build.sh
	@$(VERBOSE); export BUILD_LOG="$$($(MKTEMP))" &&                      \
          $(TEST) -f "$${BUILD_LOG:?}" &&                                     \
            $(SCRIPT) -q -e -f -c "command -p $(ENV)                          \
              PATH=\"$$(command -p $(ENV) $(GETCONF) PATH):$${PATH:?}\"       \
                \"sh\" -c \"../test/build.sh\" 2>&1" "$${BUILD_LOG:?}" && {   \
                  $(PRINTF) '%s\n' '## Tests completed successfully';         \
                      $(RM) "$${BUILD_LOG:?}"; } || {                         \
                  $(PRINTF) '\n%s\n'                                          \
                    "## TEST ERROR - Build log saved to \"$${BUILD_LOG:?}\""; \
                  exit 1; }

###############################################################################
