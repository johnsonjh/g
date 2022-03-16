# History

* First implementation of context editor kernel by Roger McCalman and
  Robert Ash, October 1981 - February 1982, at Aberystwyth University,
  originally weitten in 'B' for Honeywell-6000 GCOS-3/TSS
* All further development by Jeremy Hall. Reading, England
* Ported to UNIX (Amdahl UTS/V), August 1984 - May 1985. In the
  process it was translated to 'C' and re-written to include an
  in-core file-system, Regular Expressions, arithmetic, etc.
* Ported to ICL DRS300 (iAPX286) CDOS 4.1, January 1987
* Screen editor implemented April 1987
* Ported to ICL DRS300 UNIX (DRS/NX - SVR2 286), October 1987
* Ported to SUN 3 UNIX BSD 4.2, February 1988 (CE only)
* Ported to DRS300 CDOS 5.2, July 1988
* Ported to Tulip (386) UNIX SVR3, September 1988
* Ported to DRS300 UNIX (DRS/NX - System V R3.0 386), March 1989
* Kernel fully compiled with separate syntax analyser, July 1989
* Last CDOS build, March 1989 and CDOS support removed, 1/9/1989
* Ported to IBM PC MS-DOS 3.3, Colour VGA etc, (ANSI C), January 1990
* Ported to AT&T UNIX System V R4.0 - SPARC (ANSI C), i486, May 1990
* Ported to Sun 4 SunOS (SPARC), October 1991
* Ported to AT&T UNIX System V R4.2 December 1992
* Private malloc removed, linked VSAM finally complete, Xenix support
  removed, special code for early terminal emulators removed, C++ port
  complete. Now compiles cleanly with C++ and ANSI C. All in Jan 1993
* Ported to DEC VAX/VMS 5.0, Feb 1993 (incomplete)
* Ported to Borland Turbo C with PDCurses 2.0 on DOS, Dec 1993
* Ported to Coherent 3.2 (64K I/D, CE only)
* Ported to Pyramid (MIPS 4400), DC/OSx 1.1, Mar 1994
* Support for non-ANSI C comilers removed, now back in a single file
* Ported to Solaris 2.3, Mar 1994
* Ported to Watcom C/C++ 10 on DOS, Aug 1994
* Ported to HP-UX (PA-RISC), Jan 1995
* Ported to IBM AIX (RS/6000), Jan 1995
* Ported to DG/UX 5.4R3.10 AViiON mc88110, July 1995
* Ported to SCO 5, Aug 1995
* Ported to Linux 1.2.8, Sep 1995
* Ported to UnixWare, Sep 1995
* First distributed on the Internet
* DOS version now uses a very small and fast subset of curses, mostly in
  assembler. Memory moves and fill done with dword instructions where
  appropriate. Screen attributes are (unfortunately) now fixed, but screen
  updates are very fast. Sep 1995
* Cleaned up for modern Linux and BSD systems by Jeffrey H. Johnson, mid-2019
* G 4.7.2 and 4.7.3 on GitHub, March 2021
