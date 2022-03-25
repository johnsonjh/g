###############################################################################
# G  --  GNUmakefile
###############################################################################
# vi: filetype=make:tabstop=8:tw=79
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

CAT     ?= cat
CC      ?= cc
CHMOD   ?= chmod
ENV     ?= env
GETCONF ?= getconf
MKTEMP  ?= mktemp
LN      ?= ln
PRINTF  ?= printf
RM      ?= rm -f
SCRIPT  ?= script
SSTRIP  ?= sstrip
STRIP   ?= strip
TEST    ?= test
TR      ?= tr
TRUE    ?= true
UNAME   ?= uname
WATWCL  ?= wcl
WCL386  ?= wcl386

###############################################################################
# OpenWatcom configuration

WATCOM   ?= /opt/watcom
WATBIN   ?= binl64
EXTENDER ?= DOS32A

###############################################################################
# DJGPP configuration

DJGPP_GHOME    ?= /opt/djgpp
DJGPP_GARCH    ?= i586-pc-msdosdjgpp-
DJGPP_CCOMP    ?= gcc
DJGPP_ECOFF    ?= exe2coff
ifdef DEBUG
   DJGPP_STRIP ?= size
else
   DJGPP_STRIP ?= strip
endif # DEBUG

###############################################################################
# CWSDPMI configuration

CWSDPMI_STUB ?= /opt/cwspdmi/cwsdstub.exe

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
# Tiny build

ifdef TINY
  CFLAGS   += -DTINY_G=1
endif # TINY
ifdef TINY_G
  ifndef TINY
    CFLAGS += -DTINY_G=1
  endif # !TINY
endif # TINY_G

###############################################################################
# Verbose

ifdef V
  VERBOSE = set -x
else
  VERBOSE = set +x
endif # !V

###############################################################################
# Stack protection disable

ifdef NOSSP
  ifeq ($(_SUNOS),1)
    STACK_FLAGS ?= -U_FORTIFY_SOURCE
  else
    STACK_FLAGS ?= -fno-stack-protector -U_FORTIFY_SOURCE
  endif # _SUNOS
else
  STACK_FLAGS   ?= -D_FORTIFY_SOURCE=1
endif # NOSSP

###############################################################################
# Debugging

ifdef DEBUG
  CFLAGS     += -Wall -Wextra -DDEBUG=1 -g
else
  ifeq ($(_SUNOS),1)
    OPTFLAGS ?= -O
  endif # _SUNOS
  OPTFLAGS   ?= -Os
  CFLAGS     += $(OPTFLAGS) $(STACK_FLAGS) -Wno-int-to-pointer-cast
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
# Architectures

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
  STRARGS  = -X$(MAIXBITS)
endif # aix

###############################################################################
# Files

SRC  = g.c
OUT ?= g

###############################################################################
# Standard Compilation

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
   WATDBG = -d3 -DDEBUG=1
else
   WATDBG = -d0 -s -zq -oabls
endif # DEBUG

###############################################################################
# OpenWatcom Compilation: 16-bit DOS, 8086 (Real mode)

.PHONY: $(OUT)86 owc16
.NOTPARALLEL: $(OUT)86 owc16 $(OUT)86.exe
$(OUT)86 owc16 $(OUT)86.exe: g.lnk
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WATWCL):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "g86.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)" INCLUDE="$(WATCOM)/h"   \
           PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WATWCL) -k32767     \
             -l=DOS -bcl=DOS -DDOS=1 -DASM86=0 -0 -ml -fpi -zm @g.lnk  \
               $(WATDBG) -zp2 -fe=$(OUT)86.exe $(SRC) &&               \
                 $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# OpenWatcom Compilation: 16-bit DOS, 80386 (Real mode)

.PHONY: $(OUT)386r owc386r
.NOTPARALLEL: $(OUT)386r owc386r $(OUT)386r.exe
$(OUT)386r owc386r $(OUT)386r.exe: g.lnk
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WATWCL):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "g386r.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)" INCLUDE="$(WATCOM)/h"   \
           PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WATWCL) -k32767     \
             -l=DOS -bcl=DOS -DDOS=1 -DASM86=1 -3 -ml -fpi -zm @g.lnk  \
               $(WATDBG) -zp2 -fe=$(OUT)386r.exe $(SRC) &&             \
                 $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# OpenWatcom Compilation: 32-bit DOS, 386 (Protected mode)

.PHONY: $(OUT)386p owc386p
.NOTPARALLEL: $(OUT)386p owc386p $(OUT)386p.exe
$(OUT)386p owc386p $(OUT)386p.exe: g.lnk
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WCL386):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "./g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "./g386p.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)" INCLUDE="$(WATCOM)/h"        \
         PATH="$(WATCOM)/$(WATBIN):$(WATCOM)/binw:$${PATH:?}" $(WCL386)     \
          -l=$(EXTENDER) -bt=DOS -DDOS=1 -DASM86=0 -3r -mf -fpi -zm @g.lnk  \
           -k32767 $(WATDBG) -zp2 -DWCL386=1 -fe=$(OUT)386p.exe $(SRC) &&   \
            $(RM) -f "./g.o" 2> /dev/null

###############################################################################
# OpenWatcom Compilation: 16-bit OS/2, 286 (Protected mode)

.PHONY: $(OUT)286p owc286p
.NOTPARALLEL: $(OUT)286p owc286p $(OUT)286p.exe
$(OUT)286p owc286p $(OUT)286p.exe: gos2.lnk
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WATWCL):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "./g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "./g286p.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)"                                \
         INCLUDE="$(WATCOM)/h:$(WATCOM)/h/os21x:$${WATCOM_PDC_INCLUDE:?}"     \
          PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WATWCL)                     \
           -l=OS2 -bcl=OS2 -UDOS -DUNIX=1 -2 -ml -fpi -zm @gos2.lnk           \
            -k32767 $(WATDBG) -D_TIMESPEC_DEFINED -za -zp2 -fe=$(OUT)286p.exe \
             $(SRC) "$${WATCOM_PDC_LIBRARY:?}" && $(RM) -f "./g.o"            \
              2> /dev/null

###############################################################################
# OpenWatcom Compilation: 32-bit OS/2, 386 (Protected mode)

.PHONY: $(OUT)os2p owcos2p
.NOTPARALLEL: $(OUT)os2p owcos2p $(OUT)os2p.exe
$(OUT)os2p owcos2p $(OUT)os2p.exe: gos2.lnk
ifneq ($(V),1)
	-@$(PRINTF) '\r\t\t$(WCL386):\t\t%s\n' "$(SRC)"
endif # !V
	@$(VERBOSE); $(RM) -f "./g.o" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(RM) -f "./gos2p.exe" 2> /dev/null || $(TRUE)
	@$(VERBOSE); $(ENV) WATCOM="$(WATCOM)"                                \
         INCLUDE="$(WATCOM)/h:$(WATCOM)/h/os2:$${WATCOM_PDC_INCLUDE:?}"       \
          PATH="$(WATCOM)/$(WATBIN):$${PATH:?}" $(WCL386)                     \
           -l=OS2V2 -bcl=OS2V2 -UDOS -DUNIX=1 -3r -mf -fpi -zm @gos2.lnk      \
            -k32767 $(WATDBG) -D_TIMESPEC_DEFINED -zp2 -fe=$(OUT)os2p.exe     \
             $(SRC) "$${WATCOM_PDC_LIBRARY:?}" && $(RM) -f "./g.o"            \
              2> /dev/null

###############################################################################
# DJGPP Compilation: 32-bit DOS, 386 (Protected mode)

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
         -march=i386 -Wno-attributes $(CFLAGS) $(SRC) -o gt.exe $(LDFLAGS) && \
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
	-@$(VERBOSE); $(STRIP) $(STRARGS) -R .gnu.build.attributes   \
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
