#!/usr/bin/env sh
# shellcheck disable=SC2015
# vi: filetype=sh:tabstop=4:tw=79:expandtab
##############################################################################

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

set -eu

##############################################################################

export OPTFLAGS="-Oz" ;                                                      \
export FPTFLAGS="-Os"

##############################################################################

# shellcheck disable=SC3040
(set -o pipefail > /dev/null 2>&1) &&                                        \
    set -o pipefail

##############################################################################

MYOS="$(uname -o 2> /dev/null ||                                             \
            uname -s 2> /dev/null ||                                         \
                printf '%s\n'                                                \
                    'UNIX')"

##############################################################################

SHELL="$(command -v sh 2> /dev/null ||                                       \
            printf '%s\n'                                                    \
                '/bin/sh')"

##############################################################################

GMAKE="$(command -v gmake 2> /dev/null ||                                    \
            command -v make 2> /dev/null ||                                  \
                printf '%s\n'                                                \
                    'make')";                                                \
MAKE="${GMAKE:?} --no-print-directory"

##############################################################################

COLS="$(command -p tput cols 2> /dev/null ||                                 \
            printf '%s\n'                                                    \
                '80')";                                                      \
COLS="$(( COLS - 1 ))"

##############################################################################

test "${COLS:?}" -lt 40 &&                                                   \
    { printf '%s\n'                                                          \
        "Error: Screen too narrow (<40 columns)";                            \
      exit 1; }

##############################################################################

test "${COLS:?}" -lt 76 &&                                                   \
    { printf '%s\n'                                                          \
        "# Warning: Screen is narrow (<77 columns)"; }

##############################################################################

TWIDTH="$(( COLS - 06 ))" ; GWIDTH="$(( COLS - 10 ))"

##############################################################################

FMTOUT="fmt -c -t -s -w                                                      \
        ${TWIDTH:?} -g ${GWIDTH:?} |                                         \
        sed -E 's/(^[^#+])/   \1/'"

##############################################################################

export SSTRIP="true"

##############################################################################

MYPDCURSES="${HOME:?}/src/PDCursesMod"

##############################################################################

DJGPP_ARCH="i586-pc-msdosdjgpp" ;                                            \
DJGPP_HOME="/opt/djgpp" ;                                                    \
DJGPP_CCOMP="${DJGPP_HOME:?}/bin/${DJGPP_ARCH:?}-gcc" ;                      \
DJGPP_STRIP="${DJGPP_HOME:?}/bin/${DJGPP_ARCH:?}-strip" ;                    \
DJGPP_PDCBUILD="${MAKE:?} \"clean\" && CFLAGS=\"-flto -Os -fdata-sections    \
    -ffunction-sections -march=i386\" LDFLAGS=\"-flto -Wl,--gc-sections\"    \
      STRIP=\"${DJGPP_STRIP:?}\" CC=\"${DJGPP_CCOMP:?}\" ${MAKE:?}           \
        -f \"Makefile\""

##############################################################################

TREELOC="${HOME:?}/src/official/personal/g/test"

##############################################################################

test -d "${TREELOC:?}" ||                                                    \
  {                                                                          \
    printf 'ERROR: %s\n' "${TREELOC:?} not found!" ;                         \
    exit 1 ;                                                                 \
  }

##############################################################################

OWC_OS2_16_BUILD="${SHELL:?} -xc \"${TREELOC:?}/owc-os2-16.sh\"" ;           \
OWC_OS2_32_BUILD="${SHELL:?} -xc \"${TREELOC:?}/owc-os2-32.sh\""

##############################################################################

IA16GCC_HOME="${HOME:?}/src/build-ia16/prefix" ;                             \
IA16GCC_CCOMP="${IA16GCC_HOME:?}/bin/ia16-elf-gcc"

##############################################################################

blline()
  { printf '%s\n' ""; }

##############################################################################

hrline()
  { printf '%*s\n'                                                           \
        "${COLUMNS:-${COLS:?}}" '' |                                         \
    tr ' ' '#'; }; blline

##############################################################################

test -f "./g.c" ||                                                           \
    cd "src" 2> /dev/null ||                                                 \
        cd "../src" 2> /dev/null &&                                          \
test -f "./g.c" ||                                                           \
    { printf '%s\n'                                                          \
        "Error: Couldn't find G source code.";                               \
      exit 1; }

##############################################################################

REDACT="sed -e \"s#${HOME:?}#…#g\""

##############################################################################

unset V > /dev/null 2>&1 || true ;                                           \
unset VERBOSE > /dev/null 2>&1 || true ;                                     \
unset MAKEFLAGS > /dev/null 2>&1 || true

##############################################################################

{ hrline; printf '%s\n' "## ${MYOS:?}: GCC, Native:"; blline;                \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="gcc -ftrivial-auto-var-init=zero"                                    \
    LTO=1 LGC=1 V=1                                                          \
    ${MAKE:?} 2>&1 ;                                                         \
${MAKE:?} "strip" > /dev/null 2>&1 ;                                         \
test -f "g" && du -k "g" |                                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g"); }' ||                                          \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## ${MYOS:?}: GCC, Tiny, Native:"; blline;          \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="gcc -ftrivial-auto-var-init=zero"                                    \
    TINY_G=1 LTO=1 LGC=1 V=1                                                 \
    ${MAKE:?} 2>&1 ;                                                         \
${MAKE:?} "strip" > /dev/null 2>&1 ;                                         \
test -f "g" && du -k "g" |                                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g"); }' ||                                          \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## ${MYOS:?}: GCC, Line-mode, Native:"; blline;     \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="gcc -ftrivial-auto-var-init=zero"                                    \
    LINE_G=1 LTO=1 LGC=1 V=1                                                 \
    ${MAKE:?} 2>&1 ;                                                         \
${MAKE:?} "strip" > /dev/null 2>&1 ;                                         \
test -f "g" && du -k "g" |                                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g"); }' ||                                          \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## ${MYOS:?}: Clang, Native:"; blline;              \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="clang"                                                               \
    LTO=1 LGC=1 V=1                                                          \
    ${MAKE:?} 2>&1 ;                                                         \
${MAKE:?} "strip" > /dev/null 2>&1 ;                                         \
test -f "g" && du -k "g" |                                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g"); }' ||                                          \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## ${MYOS:?}: PCC, Native:"; blline;                \
${MAKE:?} "clean" > /dev/null &&                                             \
    PATH="/opt/pcc/bin:${PATH:-}"                                            \
    CC="/opt/pcc/bin/pcc"                                                    \
    OPTFLAGS="${FPTFLAGS:?}"                                                 \
    NOSSP=1 LTO=1 LGC=1 V=1                                                  \
    ${MAKE:?} 2>&1 |                                                         \
        grep -v -e 'warning: cannot inline but '                             \
                -e 'warning: unsupported attribute ' ;                       \
${MAKE:?} "strip" > /dev/null 2>&1 ;                                         \
test -f "g" && du -k "g" |                                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g"); }' ||                                          \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## ${MYOS:?}: Oracle Studio, Native:"; blline;      \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="suncc"                                                               \
    PATH="/opt/oracle/developerstudio-latest/bin:${PATH:?}"                  \
    NOSSP=1 _SUNOS=1 V=1                                                     \
    ${MAKE:?} 2>&1 ;                                                         \
${MAKE:?} "strip" > /dev/null 2>&1 ;                                         \
test -f "g" && du -k "g" |                                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g"); }' ||                                          \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, 8086:"; blline;                 \
${MAKE:?} "clean" > /dev/null &&                                             \
    V=1 PKGCFG="false"                                                       \
    ${MAKE:?} "g86" 2>&1 |                                                   \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g86.exe" && du -k "g86.exe" |                                       \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g86.exe"); }' ||                                    \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, Full, 8086:"; blline;           \
${MAKE:?} "clean" > /dev/null &&                                             \
    FULL_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} "g86" 2>&1 |                                                   \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g86.exe" && du -k "g86.exe" |                                       \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g86.exe"); }' ||                                    \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, Line, 8086:"; blline;           \
${MAKE:?} "clean" > /dev/null &&                                             \
    LINE_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} "g86" 2>&1 |                                                   \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g86.exe" && du -k "g86.exe" |                                       \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g86.exe"); }' ||                                    \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, 386, Real mode:"; blline;       \
${MAKE:?} "clean" > /dev/null &&                                             \
    V=1 PKGCFG="false"                                                       \
    ${MAKE:?} "g386r" 2>&1 |                                                 \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g386r.exe" && du -k "g386r.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386r.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, 386, Full, Real mode:"; blline; \
${MAKE:?} "clean" > /dev/null &&                                             \
    FULL_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} "g386r" 2>&1 |                                                 \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g386r.exe" && du -k "g386r.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386r.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, 386, Line, Real mode:"; blline; \
${MAKE:?} "clean" > /dev/null &&                                             \
    LINE_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} "g386r" 2>&1 |                                                 \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g386r.exe" && du -k "g386r.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386r.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline


##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, 386, Protected:"; blline;       \
${MAKE:?} "clean" > /dev/null &&                                             \
    V=1 PKGCFG="false"                                                       \
    ${MAKE:?} "g386p" 2>&1 |                                                 \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g386p.exe" && du -k "g386p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, 386, Full, Protected:"; blline; \
${MAKE:?} "clean" > /dev/null &&                                             \
    FULL_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} "g386p" 2>&1 |                                                 \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g386p.exe" && du -k "g386p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: OpenWatcom, 386, Line, Protected:"; blline; \
${MAKE:?} "clean" > /dev/null &&                                             \
    LINE_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} "g386p" 2>&1 |                                                 \
        sed 's/ PATH=.\+ w/ w/' |                                            \
            grep -v '^+ rm ' | eval "${REDACT:?}" ;                          \
test -f "g386p.exe" && du -k "g386p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: DJGPP, Curses, Full, Protected:"; blline;   \
PDC_BUILD_LOG="$(mktemp)" &&                                                 \
    printf '%s' "+ Building PdCursesMod: " ;                                 \
( cd "${MYPDCURSES:?}/dos" 2>&1 &&                                           \
    eval "${DJGPP_PDCBUILD:?}" 2>&1 )                                        \
      2>&1 1> "${PDC_BUILD_LOG:?}" ||                                        \
    {     printf '%s\n' "Error!";                                            \
        cat "${PDC_BUILD_LOG:?}"                                             \
            2> /dev/null || true;                                            \
        rm -f "${PDC_BUILD_LOG:?}"                                           \
            2> /dev/null || true;                                            \
        exit 1; } ;                                                          \
rm -f "${PDC_BUILD_LOG:?}" 2> /dev/null                                      \
    || true ;                                                                \
printf '%s\n' "OK." ;                                                        \
${MAKE:?} clean > /dev/null &&                                               \
    OPTFLAGS="${FPTFLAGS:?}"                                                 \
    CFLAGS="-I${MYPDCURSES:?}"                                               \
    CURSESLIB="${MYPDCURSES:?}/dos/pdcurses.a"                               \
    FULL_G=1 LTO=1 LGC=1 V=1 PKGCFG="false"                                  \
    ${MAKE:?} "g386" 2>&1 |                                                  \
        grep -v '^+ rm ' |                                                   \
        grep -v '^+ chmod ' | eval "${REDACT:?}" ;                           \
test -f "g386.exe" && du -k "g386.exe" |                                     \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386.exe"); }' ||                                   \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: DJGPP, Curses, Tiny, Protected:"; blline;   \
PDC_BUILD_LOG="$(mktemp)" &&                                                 \
    printf '%s' "+ Building PdCursesMod: " ;                                 \
( cd "${MYPDCURSES:?}/dos" 2>&1 &&                                           \
    eval "${DJGPP_PDCBUILD:?}" 2>&1 )                                        \
      2>&1 1> "${PDC_BUILD_LOG:?}" ||                                        \
    {     printf '%s\n' "Error!";                                            \
        cat "${PDC_BUILD_LOG:?}"                                             \
            2> /dev/null || true;                                            \
        rm -f "${PDC_BUILD_LOG:?}"                                           \
            2> /dev/null || true;                                            \
        exit 1; } ;                                                          \
rm -f "${PDC_BUILD_LOG:?}" 2> /dev/null                                      \
    || true ;                                                                \
printf '%s\n' "OK." ;                                                        \
${MAKE:?} clean > /dev/null &&                                               \
    OPTFLAGS="${FPTFLAGS:?}"                                                 \
    CFLAGS="-I${MYPDCURSES:?}"                                               \
    CURSESLIB="${MYPDCURSES:?}/dos/pdcurses.a"                               \
    TINY_G=1 LTO=1 LGC=1 V=1 PKGCFG="false"                                  \
    ${MAKE:?} "g386" 2>&1 |                                                  \
        grep -v '^+ rm ' |                                                   \
        grep -v '^+ chmod ' | eval "${REDACT:?}" ;                           \
test -f "g386.exe" && du -k "g386.exe" |                                     \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386.exe"); }' ||                                   \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n' "## DOS: DJGPP, Line, Protected:"; blline;           \
${MAKE:?} clean > /dev/null &&                                               \
    OPTFLAGS="${FPTFLAGS:?}"                                                 \
    LINE_G=1 LTO=1 LGC=1 V=1 PKGCFG="false"                                  \
    ${MAKE:?} "g386" 2>&1 |                                                  \
        grep -v '^+ rm ' |                                                   \
        grep -v '^+ chmod ' | eval "${REDACT:?}" ;                           \
test -f "g386.exe" && du -k "g386.exe" |                                     \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g386.exe"); }' ||                                   \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

# NOTE: Not working yet!
{ hrline; printf '%s\n' "## DOS: IA16-GCC, 8086, Tiny:"; blline;             \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="${IA16GCC_CCOMP:?}"                                                  \
    CFLAGS="-march=i8086 -mregparmcall                                       \
            -mcmodel=medium -DDOS=1 -UUNIX"                                  \
    OPTFLAGS="${FPTFLAGS:?}"                                                 \
    CURSESLIB=""                                                             \
    LDFLAGS="-li86"                                                          \
    OUT="g.exe"                                                              \
    V=1 PKGCFG="false"                                                       \
    ${MAKE:?} 2>&1 | eval "${REDACT:?}" ;                                    \
test -f "g.exe" && du -k "g.exe" |                                           \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g.exe"); }' ||                                      \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

# NOTE: Not working yet!
{ hrline; printf '%s\n' "## DOS: IA16-GCC, 8086, Line:"; blline;             \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="${IA16GCC_CCOMP:?}"                                                  \
    CFLAGS="-march=i8086 -mregparmcall                                       \
            -mcmodel=medium -DDOS=1 -UUNIX"                                  \
    OPTFLAGS="-O1"                                                           \
    CURSESLIB=""                                                             \
    LDFLAGS="-li86"                                                          \
    OUT="g.exe"                                                              \
    LINE_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} 2>&1 | eval "${REDACT:?}" ;                                    \
test -f "g.exe" && du -k "g.exe" |                                           \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g.exe"); }' ||                                      \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

# NOTE: Not working yet!
{ hrline; printf '%s\n' "## DOS: IA16-GCC, 8086, Tiny, Curses:"; blline;     \
${MAKE:?} "clean" > /dev/null &&                                             \
    CC="${IA16GCC_CCOMP:?}"                                                  \
    CFLAGS="-march=i8086 -mregparmcall                                       \
            -mcmodel=medium -UDOS -DUNIX=1"                                  \
    OPTFLAGS="${FPTFLAGS:?}"                                                 \
    CURSESLIB="-lpdcurses -li86"                                             \
    OUT="g.exe"                                                              \
    TINY_G=1 V=1 PKGCFG="false"                                              \
    ${MAKE:?} 2>&1 | eval "${REDACT:?}" ;                                    \
test -f "g.exe" && du -k "g.exe" |                                           \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g.exe"); }' ||                                      \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n'                                                      \
    "## OS/2: OpenWatcom, 286, Full, Curses, Protected:"; blline;            \
PDC_BUILD_LOG="$(mktemp)" &&                                                 \
    printf '%s' "+ Building PdCursesMod: " ;                                 \
( cd "${MYPDCURSES:?}/os2" 2>&1 &&                                           \
    eval "${OWC_OS2_16_BUILD:?}" 2>&1 )                                      \
      2>&1 1> "${PDC_BUILD_LOG:?}" ||                                        \
    {     printf '%s\n' "Error!";                                            \
        cat "${PDC_BUILD_LOG:?}"                                             \
            2> /dev/null || true;                                            \
        rm -f "${PDC_BUILD_LOG:?}"                                           \
            2> /dev/null || true;                                            \
        exit 1; } ;                                                          \
rm -f "${PDC_BUILD_LOG:?}" 2> /dev/null                                      \
    || true ;                                                                \
printf '%s\n' "OK." ;                                                        \
${MAKE:?} "clean" > /dev/null &&                                             \
    FULL_G=1 V=1 PKGCFG="false"                                              \
    WATCOM_PDC_INCLUDE="${MYPDCURSES:?}"                                     \
    WATCOM_PDC_LIBRARY="${MYPDCURSES:?}/os2/pdcurses.lib"                    \
   ${MAKE:?} "g286p" 2>&1 |                                                  \
       sed 's/ PATH=.\+ w/ w/' |                                             \
           grep -v '^+ rm ' | eval "${REDACT:?}" ;                           \
test -f "g286p.exe" && du -k "g286p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g286p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n'                                                      \
    "## OS/2: OpenWatcom, 286, Tiny, Curses, Protected:"; blline;            \
PDC_BUILD_LOG="$(mktemp)" &&                                                 \
    printf '%s' "+ Building PdCursesMod: " ;                                 \
( cd "${MYPDCURSES:?}/os2" 2>&1 &&                                           \
    eval "${OWC_OS2_16_BUILD:?}" 2>&1 )                                      \
      2>&1 1> "${PDC_BUILD_LOG:?}" ||                                        \
    {     printf '%s\n' "Error!";                                            \
        cat "${PDC_BUILD_LOG:?}"                                             \
            2> /dev/null || true;                                            \
        rm -f "${PDC_BUILD_LOG:?}"                                           \
            2> /dev/null || true;                                            \
        exit 1; } ;                                                          \
rm -f "${PDC_BUILD_LOG:?}" 2> /dev/null                                      \
    || true ;                                                                \
printf '%s\n' "OK." ;                                                        \
${MAKE:?} "clean" > /dev/null &&                                             \
    TINY_G=1 V=1 PKGCFG="false"                                              \
    WATCOM_PDC_INCLUDE="${MYPDCURSES:?}"                                     \
    WATCOM_PDC_LIBRARY="${MYPDCURSES:?}/os2/pdcurses.lib"                    \
   ${MAKE:?} "g286p" 2>&1 |                                                  \
       sed 's/ PATH=.\+ w/ w/' |                                             \
           grep -v '^+ rm ' | eval "${REDACT:?}" ;                           \
test -f "g286p.exe" && du -k "g286p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g286p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf '%s\n'                                                      \
    "## OS/2: OpenWatcom, 286, Line, Protected:"; blline;                    \
${MAKE:?} "clean" > /dev/null &&                                             \
    LINE_G=1 V=1 PKGCFG="false"                                              \
   ${MAKE:?} "g286p" 2>&1 |                                                  \
       sed 's/ PATH=.\+ w/ w/' |                                             \
           grep -v '^+ rm ' | eval "${REDACT:?}" ;                           \
test -f "g286p.exe" && du -k "g286p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file g286p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf                                                             \
    '%s\n' "## OS/2: OpenWatcom, 386, Full, Curses, Protected:"; blline;     \
PDC_BUILD_LOG="$(mktemp)" &&                                                 \
    printf '%s' "+ Building PdCursesMod: " ;                                 \
( cd "${MYPDCURSES:?}/os2" 2>&1 &&                                           \
    eval "${OWC_OS2_32_BUILD:?}" 2>&1 )                                      \
      2>&1 1> "${PDC_BUILD_LOG:?}" ||                                        \
    {     printf '%s\n' "Error!";                                            \
        cat "${PDC_BUILD_LOG:?}"                                             \
            2> /dev/null || true;                                            \
        rm -f "${PDC_BUILD_LOG:?}"                                           \
            2> /dev/null || true;                                            \
        exit 1; } ;                                                          \
rm -f "${PDC_BUILD_LOG:?}" 2> /dev/null                                      \
    || true ;                                                                \
printf '%s\n' "OK." ;                                                        \
${MAKE:?} "clean" > /dev/null &&                                             \
    FULL_G=1 V=1 PKGCFG="false"                                              \
    WATCOM_PDC_INCLUDE="${MYPDCURSES:?}"                                     \
    WATCOM_PDC_LIBRARY="${MYPDCURSES:?}/os2/pdcurses.lib"                    \
   ${MAKE:?} "gos2p" 2>&1 |                                                  \
       sed 's/ PATH=.\+ w/ w/' |                                             \
           grep -v '^+ rm ' | eval "${REDACT:?}" ;                           \
test -f "gos2p.exe" && du -k "gos2p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file gos2p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf                                                             \
    '%s\n' "## OS/2: OpenWatcom, 386, Tiny, Curses, Protected:"; blline;     \
PDC_BUILD_LOG="$(mktemp)" &&                                                 \
    printf '%s' "+ Building PdCursesMod: " ;                                 \
( cd "${MYPDCURSES:?}/os2" 2>&1 &&                                           \
    eval "${OWC_OS2_32_BUILD:?}" 2>&1 )                                      \
      2>&1 1> "${PDC_BUILD_LOG:?}" ||                                        \
    {     printf '%s\n' "Error!";                                            \
        cat "${PDC_BUILD_LOG:?}"                                             \
            2> /dev/null || true;                                            \
        rm -f "${PDC_BUILD_LOG:?}"                                           \
            2> /dev/null || true;                                            \
        exit 1; } ;                                                          \
rm -f "${PDC_BUILD_LOG:?}" 2> /dev/null                                      \
    || true ;                                                                \
printf '%s\n' "OK." ;                                                        \
${MAKE:?} "clean" > /dev/null &&                                             \
    TINY_G=1 V=1 PKGCFG="false"                                              \
    WATCOM_PDC_INCLUDE="${MYPDCURSES:?}"                                     \
    WATCOM_PDC_LIBRARY="${MYPDCURSES:?}/os2/pdcurses.lib"                    \
   ${MAKE:?} "gos2p" 2>&1 |                                                  \
       sed 's/ PATH=.\+ w/ w/' |                                             \
           grep -v '^+ rm ' | eval "${REDACT:?}" ;                           \
test -f "gos2p.exe" && du -k "gos2p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file gos2p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

{ hrline; printf                                                             \
    '%s\n' "## OS/2: OpenWatcom, 386, Line, Protected:"; blline;             \
${MAKE:?} "clean" > /dev/null &&                                             \
    LINE_G=1 V=1 PKGCFG="false"                                              \
   ${MAKE:?} "gos2p" 2>&1 |                                                  \
       sed 's/ PATH=.\+ w/ w/' |                                             \
           grep -v '^+ rm ' | eval "${REDACT:?}" ;                           \
test -f "gos2p.exe" && du -k "gos2p.exe" |                                   \
    awk '{ printf("+ OK! ["$1 "KiB] ");                                      \
            system("file gos2p.exe"); }' ||                                  \
                { printf '%s\n' "Error!"; exit 1; }
} | eval "${FMTOUT:?}"; blline

##############################################################################

${MAKE:?} "clean" > /dev/null 2>&1                                           \
    || true                                                                  \

##############################################################################
