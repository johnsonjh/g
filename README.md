# G

This is ***`G`***, a general purpose programmable text editor.

## Overview

* ***`G`*** is a general purpose programmable text editor with a
 [long history](https://github.com/johnsonjh/g/blob/master/HISTORY.md),
 originally written in the **B** programming language for **GCOS‑3/TSS**,
 running on GE/Honeywell 6000-series mainframe systems.

## Features

* ***`G`*** combines features from many sources. For example, the macro
  language was derived from the **ICL** **George** mainframe editors,
  regular expressions (*among other things*) come from **UNIX** **vi**,
  the screen editor keystrokes are similar to **WordStar**, and the
  mathematical syntax is inspired by the **C** programming language.

* The macro language is *Turing complete*, offering loops, conditionals,
  and arithmetic. Complete ***`G`*** macro programs or individual commands
  can be executed from the command line, from an edit buffer or file, or
  from the *home* area at the top of the screen (*interactively*, so that
  the effects can be seen). In many cases, ***`G`*** macros can do the work
  of traditional *UNIX* text processing tools such as `sed`, `awk`, `cut`,
  and `fmt`.

* ***`G`*** can operate as either a line editor or as a visual screen
  editor, similar to `ex`/`vi` on *UNIX* systems.

* The ***`G`*** language uses a conceptually simple two-file transcription
  (*copy*/*edit*) paradigm which is transparent for the casual screen
  editor user.

* ***`G`*** is designed to be as efficient as possible, to be highly
  portable, and to enable manipulation of large files. ***`G`*** has a very
  fast startup time and needs little memory compared to other applications.

* ***`G`*** provides a more flexible way of viewing files than commands
  like `TYPE`, `pg`, or `more`.

* Although ***`G`*** is not intended to be a fully featured word processor,
  it does have text and paragraph formatting abilities and can easily be
  used to produce simple documents.

## Compilation

### Modern

* ***`G`*** can be built for **DOS**, **Windows**, and **UNIX** systems.
  **AIX**, **Darwin**, **FreeBSD**, **NetBSD**, **OpenBSD**, **Solaris**,
  **illumos**, and **Linux** are regularly tested and supported.

* A **Curses** library is required on **UNIX** systems. **AT&T System V**
  **Curses**, **XPG4**/**XSI** **Extended Curses**, **PdCurses**,
  **PdCursesMod**, **NetBSD** **Curses**, and **NCurses** are known to work.

* A **C** compiler is required. *Oracle* **Developer Studio**, *LLVM*
  **Clang**, *AMD* **AOCC**, *GNU* **GCC**, *IBM* **XLC**/**OpenXL**,
  *Intel* **ICX**/**ICC**, **DJGPP**, and **PCC** are currently known to
  work.

* The included `GNUmakefile` can be used if **GNU** **Make** (version 3.81
  or later) is available. **GNU** **Make** is helpful, but not required to
  build ***`G`***.

### Legacy

* Historically, ***`G`*** was available for many systems, including Amdahl
  UTS/V, AT&T System V, Berkeley (BSD) UNIX, BSDI BSD/OS, Commodore AMIX,
  DG/UX, Honeywell GCOS/TSS, HP‑UX, IBM OS/2, ICL CDOS, ICL DRS/NX,
  JRG/Everex ESIX, Microsoft Xenix, MINIX, MWC Coherent, NCR SVR4 MP‑RAS,
  Novell/SCO UnixWare, Pyramid DC/OSx, Reliant UNIX/SINIX, SCO OpenServer,
  Sequent DYNIX, SunOS 3/4, Tulip SVR3, and DEC VAX/VMS.

* The [`GNUmakefile`](https://github.com/johnsonjh/g/blob/master/src/GNUmakefile)
  contains information relevant to building ***`G`*** on many of these older
  systems.

## Binaries

* [***`G`***  V4.7.2 for DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g472.exe), 32‑bit, DPMI (2022‑03‑12)
* [***`G`***  V4.7.1 for DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g471.exe), 16‑bit, real-mode (1995‑10‑20)
* [***`G`***  V4.7.2 for Win32](https://github.com/johnsonjh/g/raw/master/bin/WIN32/g472.exe), 32‑bit, Win32 (2022‑03‑12)
