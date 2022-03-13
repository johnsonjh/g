# G

This is ***G***, a general purpose programmable text editor.

## Overview

* ***G*** is a general purpose programmable text editor with a
 [long history](https://github.com/johnsonjh/g/blob/master/HISTORY.md),
 originally written in **B** for **GCOS-3/TSS** running on Honeywell
 6000-series mainframe systems.

## Features

* ***G*** combines features from many sources. For example, the macro
  language was derived from **ICL** mainframe editors, regular expressions
  (among other things) come from **Vi**, the screen editor keystrokes
  are similar to **WordStar**, and the arithmetic has the syntax of the
  `C` programming language.

* The macro language has loops, conditionals and arithmetic. ***G*** programs
  or individual commands may run from the command line, from an edit file, or 
  from a "home" area at the top of the screen (interactively, so that the
  effects can be seen). In some cases ***G*** can replace commands like `sed`,
  `awk`, and `cut`, especially when used as a pipe filter.

* The editor language uses a conceptually simple two-file transcription
  (*copy*/*edit*) paradigm which is transparent for the casual screen editor user.

* ***G*** is designed to be as efficient as possible, to be highly portable, and
  to enable manipulation of large files. ***G*** has a very fast startup time and
  needs comparatively little memory.

* ***G*** also provides a more flexible way of viewing files than commands like
  `TYPE`, `pg`, or `more`.

* Although ***G*** is not intended to be a fully featured word processor, it does
  have a basic text and paragraph formatting abilities and can easily produce simple
  documents.

## Binaries

* [***G***  V4.7.2 for DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g472.exe) - 32-bit, DPMI (2022-03-12)
* [***G***  V4.7.1 for DOS](https://github.com/johnsonjh/g/raw/master/bin/DOS/g471.exe) - 16-bit, real-mode (1995-10-20)
* [***G***  V4.7.2 for Win32](https://github.com/johnsonjh/g/raw/master/bin/WIN32/g472.exe) - 32-bit, Win32 (2022-03-13)
