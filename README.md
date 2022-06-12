# G

**_`G`_** is a portable, general purpose, programmable
text editor and calculator, with macro facility.

## Table of Contents

<!-- toc -->

- [Overview](#overview)
- [Features](#features)
- [Compilation](#compilation)
  - [Modern](#modern)
  - [Legacy](#legacy)
- [Roadmap](#roadmap)
- [Binaries](#binaries)

<!-- tocstop -->

## Overview

- **_`G`_** is a general purpose programmable text editor with a
  [long history](https://github.com/johnsonjh/g/blob/master/HISTORY.md),
  originally written in the **B** programming language for **GCOS‑3/TSS**,
  running on GE/Honeywell 6000-series mainframe systems.

## Features

- **_`G`_** combines features from many sources. For example, the macro
  language was derived from the **ICL** **George** mainframe editors,
  regular expressions (_among other things_) come from **UNIX** **vi**,
  the screen editor keystrokes are similar to **WordStar**, and the
  mathematical syntax is inspired by the **C** programming language.

- The macro language is _Turing complete_, offering loops, conditionals,
  and arithmetic. Complete **_`G`_** macro programs or individual commands
  can be executed from the command line, from an edit buffer or file, or
  from the _home_ area at the top of the screen (_interactively_, so that
  the effects can be seen). In many cases, **_`G`_** macros can do the work
  of traditional _UNIX_ text processing tools such as `sed`, `awk`, `cut`,
  and `fmt`.

- **_`G`_** can operate as either a line editor or as a visual screen
  editor, similar to `ex`/`vi` on _UNIX_ systems.

- The **_`G`_** language uses a conceptually simple two-file transcription
  (_copy_/_edit_) paradigm, which is transparent for the casual screen
  editor user.

- **_`G`_** is designed to be as efficient as possible, to be highly
  portable, and to enable manipulation of large files. **_`G`_** has a very
  fast startup time and utilizes little memory compared to other applications -
  approximately **130 KiB** for the standard build, **112 KiB** for the *Tiny*
  build, and **32 KiB** for the *line-mode* editor (*measured using the*
  **Massif** *heap profiler on an AMD64 system running GNU/Linux*).

- **_`G`_** provides a more flexible way of viewing files than commands
  like `TYPE`, `pg`, or `more`.

- Although **_`G`_** is not intended to be a fully featured word processor,
  it does have text and paragraph formatting abilities and can easily be
  used to produce simple documents.

## Compilation

### Modern

- **_`G`_** can be built for **DOS**, **Windows**, **OS/2**, **Haiku**, and
  many **UNIX** systems. **AIX**, **Darwin**, **FreeBSD**, **NetBSD**,
  **OpenBSD**, **Solaris**, **illumos**, and **Linux** are regularly tested
  and fully supported.

- A **Curses** library is required on **UNIX** systems. **AT&T System V**
  **Curses**, **XPG4**/**XSI** **Extended Curses**, **PdCurses**,
  **PdCursesMod**, **NetBSD** **Curses**, and **NCurses** are known to work.

- A **C** compiler is required. _Oracle_ **Developer Studio**, _LLVM_
  **Clang**, _AMD_ **AOCC**, _GNU_ **GCC**, _IBM_ **XLC**, _IBM_ **OpenXL**,
  _Intel_ **ICX**, _Intel_ **ICC**, **DJGPP**, **Microsoft C**, **MSVC**,
  **Watcom C/C++**, **OpenWatcom V2**, **Borland C**, and **PCC** are
  currently known to work.

- The included `GNUmakefile` can be used if **GNU** **Make** (version 3.81
  or later) is available. **GNU** **Make** is helpful, but is not required
  to build **_`G`_**.

#### Building

- Most users of *UNIX*-like operating systems can build **_`G`_** by simply
  invoking **GNU** **Make** (usually available as `gmake` or `make`) without
  any additional options or configuration. The conventional `make` targets
  are available, e.g. `clean`, `strip`, etc.
- The following options affecting compilation may be set in the shell
  environment or on the `make` command-line, for example, `make OPTION=1`:
  * `CC`: Overrides default C compiler selection, e.g. `CC=clang`
  * `LTO=1`: Enables link-time optimization
  * `LGC=1`: Enables link-time garbage collection (reducing binary size)
  * `OPTFLAGS`: Overrides default optimization flags, e.g. `OPTFLAGS=-Oz`
  * `V=1`: Enables verbose compilation messages
  * `NOSSP=1`: Disables `FORTIFY_SOURCE` and compiler stack protections
  * `STATIC=1`: Attempt statically linking all required libraries
  * `DUMA=1`: Enables support for the *DUMA* memory debugging library
  * `DSTATIC=1`: Attempt statically linking *DUMA* library (*needs GNU ld*)
  * `DEBUG=1`: Enables debugging code (for development or troubleshooting)
  * `LINE_G=1`: Builds only the line-mode editor (no curses library needed)
  * `TINY_G=1`: Disables features for lower memory use (default for PC DOS)
  * `FULL_G=1`: Overrides `TINY_G` default (enables a fully-featured build)
  * `CURSESLIB`: Overrides curses library selection, e.g. `-lcurses -ltinfo`
  * `COLOUR=0`: Disables colours (required for some older curses libraries)
- Some less common options may be undocumented, and some option combinations
  are not valid. For some practical examples and complete details, review the
  [GNUmakefile](/src/GNUmakefile) and [test script](/test/build.sh).

### Legacy

- Historically, **_`G`_** was available for many systems, including Amdahl
  UTS/V, AT&T System V, Berkeley (BSD) UNIX, BSDI BSD/OS, Commodore Amiga
  UNIX (AMIX), DG/UX, Honeywell GCOS/TSS, HP‑UX, IBM OS/2, ICL CDOS, ICL
  DRS/NX, JRG/Everex ESIX, Microsoft Xenix, MINIX, MWC Coherent, NCR SVR4
  MP‑RAS, Novell/SCO UnixWare, Pyramid DC/OSx, Reliant UNIX/SINIX, SCO
  OpenServer, Sequent DYNIX, SunOS 3/4, Tulip SVR3, and DEC VAX/VMS.

- The [`GNUmakefile`](https://github.com/johnsonjh/g/blob/master/src/GNUmakefile)
  contains information relevant to building **_`G`_** on many of these older
  systems.

## Roadmap

Future plans for **_`G`_**:

- Overhauled documentation
- Improved online help viewer
- Proper Windows console support
- VGA-mode DOS port
- Dynamic resizing
- UTF-8/multibyte support
- Binaries via GitHub and GitLab releases

## Binaries

- **_`G`_** for IBM PC DOS (MS-DOS, PC-DOS, DR-DOS, PTS-DOS, FreeDOS, etc.)
  - [**_`G`_** V4.7.4β for PC DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g474_x86.exe), 16-bit 8086, Real mode, Tiny build, G-Curses
  - [**_`G`_** V4.7.4β for PC DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g474_2bg.exe), 16-bit 8086, Real mode, Full build, G-Curses
  - [**_`G`_** V4.7.3 for PC DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g473_386.exe), 16-bit 80386, Real mode, Fast-Curses
  - [**_`G`_** V4.7.3 for PC DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g473_pro.exe), 32-bit 80386, Protected mode, DPMI, G-Curses
  - [**_`G`_** V4.7.3 for PC DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g473_cws.exe), 32-bit 80386, Protected mode, DPMI, PdCursesMod
- **_`G`_** for IBM OS/2
  - [**_`G`_** V4.7.4β for OS/2](https://github.com/johnsonjh/g/raw/master/bin/OS2/g474_216.exe), 16-bit 80286, Protected mode, OS/2 console, PdCursesMod
  - [**_`G`_** V4.7.4β for OS/2](https://github.com/johnsonjh/g/raw/master/bin/OS2/g474_232.exe), 32-bit 80386, Protected mode, OS/2 console, PdCursesMod
- **_`G`_** for Microsoft Windows
  - [**_`G`_** V4.7.3 for Windows](https://github.com/johnsonjh/g/raw/master/bin/WIN32/g473_w32.exe), 32-bit i686, Win32 GUI, PdCursesMod
