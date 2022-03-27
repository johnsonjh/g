/* ***************************************************************************
 *  G  --  g.c
 * ***************************************************************************
 * vi: filetype=c:tabstop=2:tw=79:expandtab
 * 
 *  Copyright (c) 1981-1982 Robert Ash
 *  Copyright (c) 1981-1982 Roger McCalman
 *  Copyright (c) 1984-1995 Jeremy Hall
 *  Copyright (c) 2019-2022 Jeffrey H. Johnson <trnsz@pobox.com>
 *
 *  See LICENSE for complete redistribution details.
 *
 * ***************************************************************************
 */

#undef VERSION_STRING
#define VERSION_STRING     "G 4.7.4-dev (2022-03-27)"

#ifdef DOS
# define UNIX              0
# ifdef __BORLANDC__
extern unsigned _stacklen = 32767;
extern unsigned _stklen = 32767;
# endif  /* ifdef __BORLANDC__ */
#else  /* ifdef DOS */
# define UNIX              1
# define DOS               0
# ifndef _DARWIN_C_SOURCE
#  define _DARWIN_C_SOURCE 1
# endif  /* ifndef _DARWIN_C_SOURCE */
# ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE    1
# endif  /* ifndef _XOPEN_SOURCE */
# ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE    1
# endif  /* ifndef _POSIX_SOURCE */
# ifdef __NetBSD__
#  define _NETBSD_SOURCE   1
# endif  /* ifdef __NetBSD__ */
#endif  /* ifdef DOS */

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# undef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif  /* if defined(__FreeBSD__) || defined(__OpenBSD__) */

#undef NEVER

#ifndef TINY_G
# if DOS
#  define TINY_G  1  /* small buffers etc for real mode DOS */
# else  /* if DOS */
#  define TINY_G  0
# endif  /* if DOS */
#endif  /* ifndef TINY_G */

#ifndef FULL_G
#define FULL_G ( TINY_G == 0 )
#endif  /* ifndef FULL_G */

#if DOS
# ifndef ASM86
#  ifdef __WATCOMC__
#   define ASM86  1
#  else  /* ifdef __WATCOMC__ */
#   define ASM86  0
#  endif  /* ifdef __WATCOMC__ */
# endif  /* ifndef ASM86 */
#else  /* if DOS */
# define near  /* Empty */
# define ASM86    0
#endif  /* if DOS */

#if ASM86
# define B_SIZE  2
# define B_COLS h_inc
#else  /* if ASM86 */
# define B_SIZE  1
# define B_COLS COLS
#endif  /* if ASM86 */

#ifdef _AIX
# pragma alloca
# undef _ALL_SOURCE
# define _ALL_SOURCE
#endif  /* ifdef _AIX */

#if ( ( ( defined(__SVR4) || defined(__svr4__) ) && \
        ( defined(sun) || defined(__sun) || defined(__sun__) ) ) || \
          defined(__illumos__) || defined(__solaris__) )
# define _ISO_CTYPE_ISO_H 1
# define _CTYPE_H         1
#endif  /* ( ( ( defined(__SVR4) || defined(__svr4__) ) &&
               ( defined(sun) || defined(__sun) || defined(__sun__) ) ) ||
                 defined(__illumos__) || defined(__solaris__) ) */

#if defined(__GNUC__) && defined(__ia16__) && defined(_DOS)
# define IA16_GCC_DOS 1
#endif  /* if defined(__GNUC__) && defined(__ia16__) && defined(_DOS) */

#if defined(IA16_GCC_DOS)
# define OMIT_MMAP   1
# define OMIT_SYSTEM 1
# define OMIT_POPEN  1
# define OMIT_SIGNAL 1
# define DOS_CONSOLE 1
# define near
#endif  /* if defined(IA16_GCC_DOS) */

#if defined(__WATCOMC__) && defined(__OS2V2__)
# define __OS2__     1
#endif  /* if defined(__WATCOMC__) && defined(__OS2V2__) */

#if defined(__WATCOMC__) && defined(__OS2__)
# define OMIT_MMAP   1
# define OMIT_POPEN  1
# define OMIT_SIGNAL 1
# define DOS_CONSOLE 1
# define near
#endif  /* if defined(__WATCOMC__) && defined(__OS2__) */

#ifdef __MINGW32__
# define DOS_CONSOLE 1
#endif  /* ifdef __MINGW32__ */

#ifdef __DJGPP__
# define OMIT_MMAP   1
#endif  /* ifdef __DJGPP__ */

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>

#if !defined(OMIT_SIGNAL)
#include <signal.h>
#endif  /* if !defined(OMIT_SIGNAL) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if ( DOS && defined(__BORLANDC__) ) || ( DOS && defined(_MSC_VER) )
# include <io.h>
#else
# include <unistd.h>
#endif  /* if ( DOS && defined(__BORLANDC__) ) || ( DOS && defined(_MSC_VER) ) */

#if DOS
# if defined(__BORLANDC__)
#  include <dos.h>
# endif  /* if defined(__BORLANDC__) */
# include <direct.h>
#endif  /* if DOS */

#if UNIX
# ifndef caddr_t
#  define caddr_t void *
# endif  /* ifndef caddr_t */
# if !defined(OMIT_MMAP)
#  include <sys/mman.h>
# endif  /* if !defined(OMIT_MMAP) */
#endif  /* if UNIX */

#undef open
#undef lines
#undef rows
#undef cols

/*
 *  Manifests for the George Editor.
 */

#define NUMERIC   \
     '0':         \
case '1':         \
case '2':         \
case '3':         \
case '4':         \
case '5':         \
case '6':         \
case '7':         \
case '8':         \
case '9'

#define PNUMERIC  \
     '1':         \
case '2':         \
case '3':         \
case '4':         \
case '5':         \
case '6':         \
case '7':         \
case '8':         \
case '9'

#define XNUMERIC  \
 NUMERIC:         \
case 'A':         \
case 'B':         \
case 'C':         \
case 'D':         \
case 'E':         \
case 'F'

#define EOS     0x00
#define SPACE   0x20
#define BSLASH  0x5C
#define SLASH   0x2F
#define GRAVE   0x60
#define SQUOTE  0x27
#define DQUOTE  0x22
#define TAB     0x09
#define ESC     0x1B
#define DEL     0x7F
#define LFEED   0x0A

#define CNTRL_MASK   037U
#define BYTE_MASK   0377U
#define ASCII_MASK  0177U
#define ODD_MASK      01U

#define delim(c)    ( g_map[c] &   01U )
#define comsep(c)   ( g_map[c] &   02U )
#define isdigit(c)  ( g_map[c] &  010U )
#define isupper(c)  ( g_map[c] &  020U )
#define se_b4key(c) ( g_map[c] &  040U )
#define se_jkey(c)  ( g_map[c] & 0100U )
#define se_b1key(c) ( g_map[c] & 0200U )
#define wordch(c)   ( g_map[c] & ( 04U | 010U | 020U ) )

#if DOS
# define path_sep(c) ( c == SLASH || c == BSLASH )
#else  /* if DOS */
# define path_sep(c) ( c == SLASH )
#endif  /* if DOS */

/* upper case is done by g_map */

#define tolower(c) ( isupper(c) ? c + ( 'a' - 'A' ) : c )

/* Standard one treats Formfeed etc as space too */

#define isspace(c) ( ( c ) == SPACE )

/* "punctuation" includes control characters here */

#define punctch(c) ( !( wordch(c) || isspace(c) ) )

/* determine if char is valid ASCII */

#define isascii(c) ( !( ( c ) & ~ASCII_MASK ) )

/* convert char to ASCII 7 bit */

#define toascii(c) ( ( c ) & ASCII_MASK )

/* control character literals */

#define CNTRL(c)   ( c & CNTRL_MASK )
#define iscntrl(c) ( c < SPACE  || c >= DEL )
#define isprint(c) ( c >= SPACE && c  < DEL )

/* case conversion */

#define u_star(p) u_map[*( p )]

#define TAB_WIDTH  8

#define YES        1
#define NO         0

#define G_OK       0
#define G_FAIL     1

#define byte   unsigned char
#define ushort unsigned short
#define word   unsigned int
#define real   double

#define repeat for (;;)
#define elif   else if

#define private static

#define csc  const  *const
#define cssc const **const

#define skip_space( s )        \
  while ( isspace( *( s ) ) )  \
     ++ ( s )

#define nullstr(s)    ( *( s ) == EOS )
#define heap(t)       ( t *)getvec(sizeof ( t ) )
#define getbuf(len)   (char *)getvec(len)
#define equal(s1, s2) ( strcmp(s1, s2) == 0 )

#if DOS
# define equal1(s, c) ( *(word *)s == c )
#else  /* if DOS */
# define equal1(s, c) ( s[0] == c && s[1] == EOS )
#endif  /* if DOS */

/* length of temp strings */

#define STR_LEN 2048
typedef char string[STR_LEN];

/* history, del/ins record stacks */

typedef int *stack;

#define plural(n) ( n > 1 ? let_s : empty )

/* path/file names */

#if DOS
# if !defined(_MAX_PATH) && defined(STR_LEN)
#  define _MAX_PATH STR_LEN
# endif  /* if !defined(_MAX_PATH) && defined(STR_LEN) */
typedef char FNAME[_MAX_PATH];
#else  /* if DOS */
typedef char FNAME[STR_LEN];
#endif  /* if DOS */

/* Length of system provided date string */

#define DATE_LEN 33

#if ASM86

/* smaller or faster versions of standard functions */

extern void  space_fill(  void *start,    short len);
extern byte *mspace_fill( void *start,    short len);
extern void  bzero(       void *start,    short len);
extern void  wfill(       void *start,    short val,         int len);
extern void  cwmovelr(    void *dst,      const void *src, short len);
extern void  movelr(      void *dst,      const void *src, short len);
extern char *mmovelr(     void *dst,      const void *src, short len);
extern byte *cwmmovelr(   void *dst,      const void *src, short len);
extern void  tmovelr(     void *dst,      const void *src, short len);
extern void  movelrz(     void *dst,      const void *src, short len);
extern void  wmovelr(     void *dst,      const void *src, short len);
extern void  bmovelr(     void *dst,      const void *src, short len);
extern void  wmoverl(     void *dst,      const void *src, short len);
extern void  bmoverl(     void *dst,      const void *src, short len);
extern void  moverl(      void *dst,      const void *src, short len);
extern void  cmovelr(     void *dst,      const void *src);
extern char *mcmovelr(    void *dst,      const void *src);
extern void  zmovelr(     void *dst,      const void *src);
extern char *mzmovelr(    void *dst,      const void *src);
extern char *mmovelr4(    void *dst,      const void *src);
extern void  movelr5(     void *dst,      const void *src);
extern char *mmovelr5(    void *dst,      const void *src);
extern int   ecmp(        const void *s1, const void *s2,  ushort n);
extern int   size(        const void *s);
extern char *get_eos(     const char *s);
extern char *tab_fill(    void *start);
extern void  bios_wait(   void   );

# pragma aux bios_wait =                                   \
  "xor ah,ah"                                              \
  "int 16h" modify[ah];

# pragma aux get_eos =                                     \
  "xor al,al"                                              \
  "or cx,-1"                                               \
  "repne scasb"                                            \
  "dec di" parm[es di] value[es di] modify[cx al];

# pragma aux wfill =                                       \
  "rep stosw" parm[es di][ax][cx];

# pragma aux movelr =                                      \
  "rep movsb" parm[es di][ds si][cx];

# pragma aux mmovelr =                                     \
  "rep movsb" parm[es di][ds si][cx] value[es di];

# pragma aux cwmovelr =                                    \
  "shr cx,1"                                               \
  "rep movsw"                                              \
  "jnc nocp"                                               \
  "movsb"                                                  \
  "nocp:" parm[es di][ds si][cx];

# pragma aux cwmmovelr =                                   \
  "shr cx,1"                                               \
  "rep movsw"                                              \
  "jnc nocp"                                               \
  "movsb"                                                  \
  "nocp:" parm[es di][ds si][cx] value[es di];

# pragma aux movelrz =                                     \
  "rep movsb"                                              \
  "xor al,al"                                              \
  "stosb" parm[es di][ds si][cx] modify[al];

# pragma aux tmovelr =                                     \
  "cmp cx,0"                                               \
  "jbe nocp"                                               \
  "rep movsb"                                              \
  "nocp:" parm[es di][ds si][cx];

# pragma aux zmovelr =                                     \
  "nc: lodsb"                                              \
  "stosb"                                                  \
  "test al,al"                                             \
  "jnz nc" parm[es di][ds si] modify[al];

# pragma aux mzmovelr =                                    \
  "nc: lodsb"                                              \
  "stosb"                                                  \
  "test al,al"                                             \
  "jnz nc"                                                 \
  "dec di" parm[es di][ds si] modify[al] value[es di];

# pragma aux cmovelr =                                     \
  "or cx,-1"                                               \
  "xor al,al"                                              \
  "repne scasb"                                            \
  "dec di"                                                 \
  "nc: lodsb"                                              \
  "stosb"                                                  \
  "test al,al"                                             \
  "jnz nc" parm[es di][ds si] modify[cx al];

# pragma aux mcmovelr =                                    \
  "or cx,-1"                                               \
  "xor al,al"                                              \
  "repne scasb"                                            \
  "dec di"                                                 \
  "nc: lodsb"                                              \
  "stosb"                                                  \
  "test al,al"                                             \
  "jnz nc"                                                 \
  "dec di" parm[es di][ds si] modify[cx al] value[es di];

# pragma aux bmovelr =                                     \
  "rep movsd" parm[es di][ds si][cx];

# pragma aux mmovelr4 =                                    \
  "movsd" parm[es di][ds si] value[es di];

# pragma aux movelr5 =                                     \
  "movsd"                                                  \
  "movsb" parm[es di][ds si];

# pragma aux mmovelr5 =                                    \
  "movsd"                                                  \
  "movsb" parm[es di][ds si] value[es di];

# pragma aux bzero =                                       \
  "xor eax,eax"                                            \
  "rep stosd" parm[es di][cx] modify[ax];

# pragma aux bmoverl =                                     \
  "mov dx,cx"                                              \
  "shl dx,2"                                               \
  "add di,dx"                                              \
  "add si,dx"                                              \
  "sub di,4"                                               \
  "sub si,4"                                               \
  "std"                                                    \
  "rep movsd"                                              \
  "cld" parm[es di][ds si][cx] modify[dx];

# pragma aux tab_fill =                                    \
  "mov eax,20202020h"                                      \
  "stosd"                                                  \
  "stosd" parm[es di] modify[ax] value[es di];

# pragma aux wmovelr =                                     \
  "rep movsw" parm[es di][ds si][cx];

# pragma aux moverl =                                      \
  "add di,cx"                                              \
  "add si,cx"                                              \
  "dec di"                                                 \
  "dec si"                                                 \
  "std"                                                    \
  "rep movsb"                                              \
  "cld" parm[es di][ds si][cx];

# pragma aux wmoverl =                                     \
  "add di,cx"                                              \
  "add di,cx"                                              \
  "add si,cx"                                              \
  "add si,cx"                                              \
  "sub di,2"                                               \
  "sub si,2"                                               \
  "std"                                                    \
  "rep movsw"                                              \
  "cld" parm[es di][ds si][cx];

# pragma aux space_fill =                                  \
  "mov al,32"                                              \
  "rep stosb" parm[es di][cx] modify[al];

# pragma aux mspace_fill =                                 \
  "mov al,32"                                              \
  "rep stosb" parm[es di][cx] modify[al] value[es di];

# pragma aux ecmp =                                        \
  "xor ax,ax"                                              \
  "or cx,cx"                                               \
  "repe cmpsb"                                             \
  "jne neq"                                                \
  "inc ax"                                                 \
  "neq:" parm[es di][ds si][cx] value[ax];

# pragma aux size =                                        \
  "xor al,al"                                              \
  "or cx,-1"                                               \
  "repnz scasb"                                            \
  "not cx" parm[es di] value[cx] modify[al];

# define save_jbuf(d, s) wmovelr(d, s, 13)

#else  /* if ASM86 */

# if DOS
#  ifndef __DJGPP__
#   if defined(__BORLANDC__) || defined(_MSC_VER)
#    include <bios.h>
#   else
#    include <i86.h>
#   endif  /* if defined(__BORLANDC__) || defined(_MSC_VER) */
#  endif  /* ifndef __DJGPP__ */
# endif  /* if DOS */

private
void
wfill(void *s, const short v, int len)
{
  ushort *p = s;

  while (len--)
    {
      *p++ = v;
    }
}

# define movelr(a, b, n)                           \
    memcpy( (void *)( a ),                         \
            (const void *)( b ), ( n ) )

# define moverl(a, b, n)                           \
    memmove( (void *)( a ),                        \
             (const void *)( b ), ( n ) )

# define movelr5(a, b)                             \
    memcpy( (void *)( a ),                         \
            (const void *)( b ), 5 )

# define mmovelr(a, b, n)                          \
    ( memcpy( (void *)( a ),                       \
              (const void *)( b ), n),             \
              (void *)( (char *)( a ) + n ) )

# define cwmovelr(a, b, n)                         \
    memcpy( (void *)( a ),                         \
            (const void *)( b ), ( n ) )

# define cwmmovelr(a, b, n)                        \
    ( memcpy( (void *)( a ),                       \
              (const void *)( b ), n),             \
              (void *)( (char *)( a ) + n ) )

# define mmovelr4(a, b)                            \
    ( memcpy( (void *)( a ),                       \
              (const void *)( b ), 4),             \
              (void *)( (char *)( a ) + 4 ) )

# define mmovelr5(a, b)                            \
    ( memcpy( (void *)( a ),                       \
              (const void *)( b ), 5),             \
              (void *)( (char *)( a ) + 5 ) )

# define tmovelr(a, b, n)                          \
    if (n > 0)                                     \
      memcpy( (void *)( a ),                       \
              (const void *)( b ), n )

# define movelrz(a, b, n)                          \
    memcpy( (void *)( a ),                         \
            (const void *)( b ), n );              \
    a[n] = EOS;

# define zmovelr(a, b)                             \
    strcpy( (void *)( a ),                         \
            (const void *)( b ) )

# define mzmovelr(a, b)                            \
    ( strcpy( (void *)a,                           \
              (const void *)b), get_eos(a) )

# define cmovelr(a, b)                             \
    strcat( (void *)( a ),                         \
            (const void *)( b ) )

# define mcmovelr(a, b)                            \
    ( strcat( (void *)a,                           \
              (const void *)b),                    \
      get_eos(a) )

# define wmovelr(a, b, n)                          \
    memcpy( (void *)( a ),                         \
            (const void *)( b ),                   \
            ( n ) << 1 )

# define wmoverl(a, b, n)                          \
    memmove( (void *)( a ),                        \
             (const void *)( b ),                  \
             ( n ) << 1 )

# ifndef bzero
#  define bzero(s, n)                              \
     (void)memset( (char *)( s ),                  \
                   EOS, ( n ) << 1 )
# endif  /* ifndef bzero */

# define space_fill(s, n)                          \
    (void)memset( (char *)( s ),                   \
                  SPACE, n )

# define mspace_fill(s, n)                         \
    ( (void)memset( (char *)s,                     \
                    SPACE, n), s + n )

# define tab_fill(s)                               \
    ( space_fill(s, TAB_WIDTH),                    \
      s + TAB_WIDTH )

# define ecmp(s1, s2, n)                           \
    ( memcmp(s1, s2, n) == 0 )

# define size(s)                                   \
    ( strlen(s) + 1 )

# define get_eos(s)                                \
    strchr(s, EOS)

# define bmovelr                                   \
    wmovelr

# define bmoverl                                   \
    wmoverl

# define save_jbuf(d, s)                           \
    movelr( d, s, sizeof ( jmp_buf ) )

#endif  /* if ASM86 */

/*
 *  FCB for internal files.
 */

typedef struct _pp
{
  byte *base;     /* Memory address of page       */
  word rec;       /* Record number of last record */
  ushort linked,  /* Shared page index            */
    end_pos;      /* One past last byte           */
} PAGE_PTR;

typedef struct _unit
{
  PAGE_PTR *list;        /* page table              */
  byte *rec_start;       /* address for next r/w op */
  struct _unit *link_u;  /* linked file descriptor  */
  word rec_num,          /* current GE record       */
    eof_rec,             /* location of EOF         */
    page,                /* index of current page   */
    eof_page,            /* last active page        */
    list_end;            /* last page in table      */
  char read;             /* read/write              */
} UNIT;

/* Report the current record pointer of the file */

#define vstell(fp) ( fp->rec_num )

/* Report the current size of the file (for read files only) */

#define vssizeof(fp) ( fp->eof_rec )

/* Reset file pointers */

#define vsrewind(fp) \
  ( fp->rec_num = fp->page = 0, fp->rec_start = fp->list->base )

/* Determine if a file is primary and therefore linked */

#define isprimary(fp) ( fp->link_u != NULL )

/* vsam page size & allocation unit */

#if TINY_G
# define PPP         200              /* Entries per page */
# define BLOCK_SIZE 4096              /* R/W block size   */
#else  /* if TINY_G */
# define PPP         360
# define BLOCK_SIZE 8192
#endif  /* if TINY_G */

#define PPB 8                         /* Pages per allocation unit */
#define PP_SIZE  sizeof ( PAGE_PTR )  /*   should be 10/12 bytes   */
#ifndef PAGE_SIZE
# define PAGE_SIZE ( PP_SIZE * PPP )  /* Data page size */
#endif  /* ifndef PAGE_SIZE */
#define PBLOCK         ( PPP * PPB )  /* Sizeof allocation unit */

/* Max length of text line */

#if TINY_G
# define E_BUFF_LEN 1022
#else  /* if TINY_G */
# define E_BUFF_LEN ( PAGE_SIZE - 3 )
#endif  /* if TINY_G */
#define E_BUFF_SIZE ( E_BUFF_LEN + 2 )
typedef char LINE[E_BUFF_SIZE];

/* Macro list element */

#define MAC_NAME_LEN 64

typedef struct _ml
{
  const struct _ml *next;
  const char *text;
  char name[MAC_NAME_LEN + 1];
  char par_sub;
  byte nargs;
} MACRO;

/* Entire save area, holds variables and files */

typedef struct
{
  char *in_rec, *out_rec;
  int in_rec_len, out_rec_len, in_rec_num, g_rec, \
      g_eof, i_eor, i_col, e_col, o_rec;
} SAVE_AREA;

/* General purpose file list element */

typedef struct _fl
{
  struct _fl *next, *prev;
  UNIT *old_u;     /* Previous file in list  */
  SAVE_AREA save;  /* Previous kernel state  */
  FNAME name;      /* File name              */
  char disp;       /* Access mode letter     */
  byte trans;      /* File is transient file */
} FILE_LIST;

/* A variable for the calculator */

typedef struct _tk
{
  union
  {
    real r;
    long i;
  } opval,           /* operand and temporary values                 */
    litval;          /* literal values                               */
  struct _tk *next,  /* when compiled or on free list                */
    *snext;          /* when stacked                                 */
  const char *errp;  /* position of start of token for error reports */
  word id;           /* operator, var name or LITERAL                */
  char fp;           /* int or real operand                          */
  char l_fp;         /* int or real literal                          */
  char group;        /* Basic type selected in outer loop            */
}  /* Note one wasted byte */
TOKEN;

/* G_err manifests */

#define SYN_EXPR         0
#define NO_DOT           1
#define Y_LENGTHS        2
#define B_BOFFILE        3
#define EMPTY_USE_MERGE  4
#define NO_BACK          5
#define DIV_CHECK        6
#define I_REPEAT         7
#define XIT_S_M_T        8
#define I_OPT            9
#define DIG_OR_END      10
#define S_STR_LEN       11
#define END_OF_FILE     12
#define END_OF_LINE     13
#define STR_N_F         14
#define B_BOFLINE       15
#define BAD_NUM         16
#define N_T_S           17
#define TYPE_ERR        18
#define VERIFY_FAIL     19
#define W_IN_MERGE      20
#define INT_OPT         21
#define I_COMMAND       22
#define NO_MAC_NAM      23
#define RE_BACKREF      24
#define NO_SUBS_CH      25
#define NO_COUNT        26
#define NO_M_COMM       27
#define M_NAME_NF       28
#define ARG_NUM_INV     29
#define SM_SE           30
#define SYS_COM_FAIL    31
#define COMM_TOO_LONG   32
#define SAV_ON_STACK    33
#define NO_SMUT         34
#define M_DELIM         35
#define NO_ENV          36
#define UP_DELIM        37
#define BREAK_KEY       38
#define FILE_ERROR      39
#define NO_PREV_RE      40
#define MISSING_BRA     41
#define RE_TOO_MANY_BRA 42
#define RE_END_RANGE    43
#define RE_CLOSE_CURLY  44
#define RE_1_GT_2       45
#define EOF_INSERT      46
#define LINE_TOO_LONG   47
#define RE_CCL_IMB      48
#define RE_TOO_LONG     49
#define NO_RE_READ      50
#define NO_TEMP_FILE    51
#define TRANS_IN_USE    52
#define RHS_TOO_LONG    53
#define ILL_RHS_STR     54
#define CLOSE_SAVE      55
#define CLOSE_MERGE     56
#define HEX_INV         57

/* Standard files */

#define vdu    stderr
#define kbd_fd 0
#define vdu_fd 2

/* screen action on return from CE */

#define SE_DISP 1
#define SE_WAIT 2

/* terminating condition codes */

#define NO_OPT          0
#define OP_EOF          1
#define RECS            2
#define MRECS           3
#define OR_END          4
#define R_TIMES         5
#define R_END           6
#define L_LON           7
#define L_LOFF          8
#define OP_CALC    0x0100
#define STR_IGNORE 0x0200
#define STR_NEGATE 0x0400
#define OP_WHILE   0x1000
#define OP_UNTIL   0x2000
#define LOOP_MASK  0xF000

/* string options for find_string */

#define STR_END  \
     'B':        \
case 'C':        \
case 'G':        \
case 'F':        \
case 'S':        \
case 'R':        \
case 'r'

/* Drive options */

#define D_LINE_USER 0
#define D_SE_HOME   1
#define D_USE_FILE  2
#define D_SE_AUTO   3

/* calculator options */

#define C_ENDP   0   /* calculator is to evaluate an endpoint */
#define C_SIDE   3   /* Used for side effects only            */
#define C_REPEAT 4   /* Interactive mode, display result      */

/* length of portions */

#define L_LEN 76

/* Possible GE delimiters */

#define DELIM   \
      '/':      \
case  ':':      \
case  '=':      \
case  '?':      \
case  '$':      \
case  '%':      \
case  '&':      \
case  '+':      \
case  '>':      \
case  '<':      \
case  '[':      \
case  ']':      \
case '\'':      \
case '\"':      \
case  '`':      \
case DEL

/* Structures for parsed options/endpoints */

typedef struct _option
{
  word q;    /* the primary option (#,R,C,S etc) */
  int v;     /* from #, N, or {}                 */
  TOKEN *e;  /* compiled numeric expressions     */
  string s;  /* search string                    */
} OPTION;

typedef struct _verb
{
  struct _verb *next, *cpar;  /* end of loop/cond clause               */
  const char *errp;           /* pointer to start of command for g_err */
  OPTION o1;                  /* inter-record component                */
  OPTION o2;                  /* intra-record component                */
  char comm;                  /* the command itself (T,P etc)          */
  char dot;                   /* line component flag                   */
} VERB;

/* record nested command lists to implement co-routine SE/CE */

typedef struct _verb_list
{
  struct _verb_list *next, *prev;
  VERB *prog;
  int save_depth;
} VERB_LIST;

#if DOS

/*
 *  Mini Curses for G, no stdscr, write direct to screen
 */

# include <bios.h>

# define bios_byte(offs) ( *(   (byte *)offs ) )
# define bios_word(offs) ( *( (ushort *)offs ) )

typedef ushort chtype;       /* 8-bit attr + 8-bit char */

private
word near LINES, near COLS;  /* terminal width/height */

# define ACS_HLINE    0xC4
# define ACS_VLINE    0xB3
# define ACS_ULCORNER 0xDA
# define ACS_LLCORNER 0xC0
# define ACS_URCORNER 0xBF
# define ACS_LRCORNER 0xD9

/*
 *  Function and Keypad Key Definitions.
 */

# define ERR             0     /* general error flag            */

# define KEY_DOWN   0x5000     /* Down arrow key                */
# define KEY_UP     0x4800     /* Up arrow key                  */
# define KEY_LEFT   0x4B00     /* Left arrow key                */
# define KEY_RIGHT  0x4D00     /* Right arrow key               */
# define KEY_HOME   0x4700     /* home key                      */
# define KEY_NPAGE  0x5100     /* next page                     */
# define KEY_PPAGE  0x4900     /* previous page                 */
# define KEY_BTAB   0x0F00     /* Back tab key                  */
# define KEY_IC     0x5200     /* insert char or enter ins mode */
# define KEY_END    0x4F00     /* end key                       */
# define KEY_DC     0x5300     /* delete character              */
# define KEY_SLEFT  0x7300     /* shifted left arrow key        */
# define KEY_SRIGHT 0x7400     /* shifted right arrow           */
# define KEY_F0     0x3A00     /* 10 function keys              */

# define KEY_F(n) ( ( 0x3A + ( n ) ) << 8 )

private
void clrtoeol(void);

private
void deleteln(void);

private
ushort curs_getc(void);

private
void insertln(void);

private
void initscr(void);

private
void curs_chins(void);

private
void napms(const unsigned long);

# define init_pair(a, b, c)  /* Empty */
# define endwin()            bios_gotoxy( (byte)( LINES - 2 ), 0 )
# define raw()               /* Empty */
# define noraw()             /* Empty */
# define attrset(a)          /* Empty */
# define refresh()           /* Empty */
# define move(y, x)          ( curs_row = ( y ), curs_col = ( x ) )
# define rgetc()             curs_getc()
# define getch()             curs_getc()
# define insch(c)            curs_chins()
# define keypad(w, flag)     /* Empty */
# define nonl()              /* Empty */
# define noecho()            /* Empty */
# define kbd_check(c)        (c = _bios_keybrd(_KEYBRD_READY) ? rgetc() : ERR)
# define erase()             berase(v_base, LINES * B_COLS)

#else  /* if DOS */

/* specials for UNIX curses.h */

# define PERFORMANCE 1
# define CURS_PERFORMANCE
# define NCC         8  /* kludge for termio.h (_XOPEN_SOURCE on SVR4.2)  */
# ifndef L_ctermid
#  define L_ctermid  1  /* so curses defines SYSV and not index and bcopy */
# endif  /* ifndef L_ctermid */

# ifdef _HPUX_SOURCE
#  include <curses_colr/curses.h>
# else  /* ifdef _HPUX_SOURCE  */
#  include <curses.h>
# endif  /* ifdef _HPUX_SOURCE */

/* specials for AIX */

# ifndef ACS_HLINE
#  define ACS_HLINE      '-'
#  define ACS_VLINE      '|'
#  define ACS_ULCORNER   '+'
#  define ACS_URCORNER   '+'
#  define ACS_LLCORNER   '+'
#  define ACS_LRCORNER   '+'
#  define wtimeout(w, t) ( ( w )->_nodelay = ( t ) )

private
int
rgetc(void)
{
  (void)refresh();
  return getch();
}

# else  /* ifndef ACS_HLINE  */
#  define rgetc() getch()
# endif  /* ifndef ACS_HLINE */

# ifndef COLOUR
#  ifdef COLOR_PAIR
#   define COLOUR 1
#  else  /* ifdef COLOR_PAIR  */
#   define COLOUR 0
#  endif  /* ifdef COLOR_PAIR */
# endif  /* ifndef COLOUR */

#endif  /* if DOS */

/* Manifests for the Screen Editor */

/* Action codes */

#define NEXT_LINE  1   /* Move window down one record       */
#define PREV_LINE  2   /* Move window up one record         */
#define NEXT_PAGE  3   /* Move window down one 20 line page */
#define PREV_PAGE  4   /* Move window up one page           */
#define MOVE_TOF   5   /* Move to start of file             */
#define MOVE_EOF   6   /* Move to end of file               */
#define MOVE_ABS   7   /* Move direct to absolute line      */
#define SE_ENTER   8   /* Do T.#0 and fill buffer           */
#define SE_LEAVE   9   /* Leave S.E and write ALL text back */
#define PEEK_LINE 10   /* Push line direct from file to stk */

/* Screen and command buffer definition */

#define STATUS_LINE   0
#define COMMAND_LINE  1
#define TEMPLATE_LINE 2
#define FIRST_LINE    3
#define MATCH_LINE    6

/* Screen attributes */

#if DOS

/* colour & mono */

# define found_col  0x4F00    /* matched text           */
# define cntrl_col  0x1F00    /* control characters     */
# define eof_col    0x0F00    /* EOF marker             */
# define scale_col  0x0E00    /* the scale line         */
# define status_col 0x0B00    /* the status line        */
# define norm_col   0x0A00    /* normal text            */
# define query_col  0x0C00    /* query                  */
# define marg_col   0x0D00    /* margins                */
# define found_ctrl 0x4900    /* matched binary         */
# define norm_space 0x0A20    /* normal text space char */

#else  /* if DOS */

/* colour */

# define FOUND_COL  COLOR_PAIR(1)  /* matched text       */
# define CNTRL_COL  COLOR_PAIR(2)  /* control characters */
# define EOF_COL    COLOR_PAIR(3)  /* EOF marker         */
# define SCALE_COL  COLOR_PAIR(4)  /* the scale line     */
# define STATUS_COL COLOR_PAIR(5)  /* the status line    */
# define NORM_COL   COLOR_PAIR(6)  /* normal text        */
# define QUERY_COL  COLOR_PAIR(7)  /* query              */
# define MARG_COL   COLOR_PAIR(8)  /* margins            */
# define FOUND_CTRL COLOR_PAIR(9)  /* matched binary     */

/* monochrome */

# ifndef A_NORMAL
#  define A_NORMAL 0
# endif  /* ifndef A_NORMAL */

# define M_FOUND_COL  A_REVERSE    /* matched text       */
# define M_CNTRL_COL  A_REVERSE    /* control characters */
# define M_EOF_COL    A_BOLD       /* EOF marker         */
# define M_SCALE_COL  A_NORMAL     /* the scale line     */
# define M_STATUS_COL A_NORMAL     /* the status line    */
# define M_NORM_COL   A_NORMAL     /* normal text        */
# define M_QUERY_COL  A_BOLD       /* query              */
# define M_MARG_COL   A_BOLD       /* margins            */
# define M_FOUND_CTRL A_REVERSE    /* matched binary     */

# if COLOUR

private
chtype found_col = FOUND_COL,      /* matched text       */
cntrl_col        = CNTRL_COL,      /* control characters */
eof_col          = EOF_COL,        /* EOF marker         */
scale_col        = SCALE_COL,      /* the scale line     */
status_col       = STATUS_COL,     /* the status line    */
norm_col         = NORM_COL,       /* normal text        */
query_col        = QUERY_COL,      /* query              */
marg_col         = MARG_COL,       /* margins            */
found_ctrl       = FOUND_CTRL;     /* matched binary     */

# else  /* if COLOUR */

private
chtype found_col = M_FOUND_COL,    /* matched text       */
  cntrl_col      = M_CNTRL_COL,    /* control characters */
  eof_col        = M_EOF_COL,      /* EOF marker         */
  scale_col      = M_SCALE_COL,    /* the scale line     */
  status_col     = M_STATUS_COL,   /* the status line    */
  norm_col       = M_NORM_COL,     /* normal text        */
  query_col      = M_QUERY_COL,    /* query              */
  marg_col       = M_MARG_COL,     /* margins            */
  found_ctrl     = M_FOUND_CTRL;   /* matched binary     */

# endif  /* if COLOUR */

#endif  /* if DOS */

/* directions for deletes etc */

#define LEFT  1
#define RIGHT 2

/* current screen line */

#define CURSOR_ROW ( ( row == COMMAND_LINE ? text_row : row ) - FIRST_LINE )

/* line number of start of screen */

#define START_OF_PAGE ( o_rec + 1 )

/* current file position */

#define FILE_LINE  ( START_OF_PAGE + row - FIRST_LINE )
#define HFILE_LINE ( START_OF_PAGE + CURSOR_ROW )
#define FILE_COL   ( col + offset )

/* address of cursor in screen buffer */

#define BUF(c) ( &s_buf[row][c] )

#ifndef kbd_check
# ifdef __DGUX__
#  define kbd_check(c) \
    ( nodelay(stdscr, 1),  c = rgetc(), nodelay(stdscr, 0) )
# else  /* ifdef __DGUX__ */
#  define kbd_check(c) \
    ( wtimeout(stdscr, 0), c = rgetc(), wtimeout(stdscr, -1) )
# endif  /* ifdef __DGUX__ */
#endif  /* ifndef kbd_check */

/* peek at length of object at top of a stack */

#define pop_length(s) ( *(short *)( s + 1 ) )

/* keyboard sequence actions */

typedef enum
{
  A_C_UP,
  A_FILE_MOVE,
  A_C_DOWN,
  A_C_LEFT,
  A_EXP_MODE,
  A_DEL_C,
  A_C_HOME,
  A_B_TAB,
  A_PAGE_SHIFT,
  A_C_STAY,
  A_DEL_REST,
  A_C_EOL,
  A_C_TOS,
  A_C_SOL,
  A_C_BOS,
  A_JUSTIFY,
  A_RWX_FILE,
  A_EXIT_EDITOR,
  A_W_LEFT,
  A_C_RIGHT,
  A_W_RIGHT,
  A_H_TAB,
  A_SEARCH,
  A_CHARACTER,
  A_C_RETURN,
  A_OPEN_LINE,
  A_DEL_LINE,
  A_REST_LINE,
  A_HELP,
  A_HIST,
  A_REPEAT,
  A_MISC_CE,
  A_YANK,
  A_COMMAND,
  A_FINDC,
  A_BLOCK,
  A_REDRAW
} ACTION;

/* user query types */

typedef enum
{
  Q_EDIT,
  Q_RAW,
  Q_YORN,
  Q_BLOCK
} Q_MODE;

/*
 *  Cursor to end of line.
 */

#define c_eol() set_col(eor[row])

/*
 *  Cursor right one character.
 */

#define c_right() set_col(FILE_COL + 1)

/*
 *  Cursor to start of line.
 */

#define c_sol() col = offset = 0

/*
 *  Sync screen & file
 */

#define se_sync() file_move(-START_OF_PAGE)

#if UNIX  /* for smooth scrolling */

# define se_insertln()         \
  {                            \
    (void)idlok(stdscr, YES);  \
    (void)insertln();          \
    ++idlpending;              \
  }

# define se_deleteln()         \
  {                            \
    (void)idlok(stdscr, YES);  \
    (void)deleteln();          \
    ++idlpending;              \
  }

#else  /* if UNIX */

# define se_insertln() insertln()
# define se_deleteln() deleteln()

#endif  /* if UNIX */

private
int George(const VERB *);

private
void G_compile(VERB **, const char *);

private
int Disk_to_mem(char csc, UNIT *const, const int);

private
void c_comm_u(void);

private
void Xit(VERB csc);

private
void term(void);

private
void message(char csc);

private
void inform(char csc);

private
void se_execute(const ACTION, const int);

private
void alter_end(int, const int);

private
void Quit(void);

private
void Exit(void);

private
void Drive(const int);

#if defined(__WATCOMC__) && !defined(__OS2__)
# pragma aux main     aborts;
# pragma aux Quit     aborts;
# pragma aux Exit     aborts;
# pragma aux g_err    aborts;
# pragma aux g_intr   aborts;
# pragma aux se_error aborts;
# pragma aux _exit    aborts;
#endif  /* if defined(__WATCOMC__) && !defined(__OS2__) */

#define FBSTR       \
  private           \
  const byte near

#define FSTR        \
  private           \
  const char near

#define FSTR_LIST   \
  private           \
  char  csc  near

/* system specific strings */

#if DOS || DOS_CONSOLE
FSTR shell_bin[] = "COMMAND",     near tty_file[] = "CON",
near se_pcom[]   = "snLPT1,te,x", near shell_var[] = "COMSPEC",
near se_lcom[]   = ".tss:DIR /W:"
# ifdef __MINGW32__
,write_only[] = "w"
# endif  /* ifdef __MINGW32__ */
;
# if defined(__MINGW32__) || defined(DOS_CONSOLE)
FSTR_LIST save_dirs[] = {                    
  ".", "~", "/usr/preserve", "/tmp", NULL
};                                                        
# endif  /* if defined(__MINGW32__) || defined(DOS_CONSOLE) */
#else  /* if DOS || DOS_CONSOLE */
FSTR shell_var[] = "SHELL", shell_bin[] = "sh",
  se_pcom[]      = "S!\177lp -c '-tG print: %s' 1>/dev/null 2>&1\177,TE,X",
  se_lcom[]      = ".tss/ls -C/", write_only[] = "w", tty_file[] = "/dev/tty";
FSTR_LIST save_dirs[] = {
  ".", "~", "/usr/preserve", "/tmp", NULL
};
#endif  /* if DOS || DOS_CONSOLE */

/* standard file names */

FSTR si_file[] = "stdin", near so_file[] = "stdout", near t_fname[] = "*TMP*",
near no_file[] = "*NEW*";

/* strings used more than once */

FSTR se_find[]  = "Find:", near let_s[] = "s",
near se_ep[]    = "End point:", near se_fin1[] = "(unchanged) ",
near se_fcom1[] = "TR\177%s\177", near se_hit[] = "\r\n[Enter to Continue] ",
near let_col[]  = ":", near empty[] = "", near endsent[] = ".!?;",
near ft_out[]   = "Output", near bra_start[] = "({[<",
near bra_end[]  = ")}]>", near hextrans[] = "0123456789ABCDEF",
near ft_merge[] = "Merge", near esc_symb[] = "VANTBFR0",
near c_rom[]    = "IXCMZVLDWixcmzvldw", near line_pos[] = "Line %d.%d",
near ft_in[]    = "Input", near pt_list[] = "\nListing from %s file %s",
near ps_name[]  = "%s file: %s", near m_real[] = "%.14g",
near eof_mess[] = "************ EOF ************";

/* format of details line */

FSTR f_wc[] = "Lines   Words   Punct.  Cntrl.  Sent.   L.O.C   Chars.\n\
%-8ld%-8ld%-8ld%-8ld%-8ld%-8ld%-ld";

/* arithmetic operator priority */

FBSTR opprio[] = {
  13, 13, 13, 12, 12, 10, 10, 8, 7, 6, 2, 2, 1, 0
},
near esc_char[] = "\013\007\n\t\b\f\r";

#if FULL_G
FSTR \
    user_template[] = "  "
                      "0....+....1....+....2....+....3....+....4....+....5..."
                      ".+....6....+....7....+\n";
#endif  /* if FULL_G */

/* Errors */

FSTR g_mess[] =
  "Syntax error in expression\0\
This command does not accept a dot part\0\
Y verb expects two equal length strings\0\
Cannot go back before beginning of file\0\
Use/Merge file empty\0\
Prior position not valid\0\
Divide by zero\0\
Illegal endpoint\0\
The X verb is followed by S(save) or M(merge) or T(transient)\0\
Verb does not support this option\0\
Number or { expr } expected\0\
Option string too long\0\
No more in your file\0\
End of line encountered\0\
String not found\0\
Start of line encountered\0\
Bad number\0\
Nothing to split\0\
Operation undefined for floating point operands\0\
Verify failure\0\
Merge file open\0\
Interactive/multi-record options not allowed\0\
Verb not recognised\0\
No macro name supplied\0\
\"\\digit\" out of range in RE\0\
Bad macro definition - subs_ch and no_of_args missing/invalid\0\
Bad macro definition - no_of_arguments is a single digit\0\
Bad macro definition - no body supplied\0\
Macro name not in table\0\
Bad macro body - argument number invalid\0\
Cannot edit Save/Merge files in screen mode\0\
Command execution failure\0\
Command too long\0\
Save file open\0\
No Save/Merg/Use file open\0\
Illegal or missing delimiter\0\
Environment variable not found\0\
Unpaired delimiter in single line multirecord insert/display\0\
BREAK key pressed - command discarded\0\
Can\'t access file\0\
No saved RE\0\
Parenthesis imbalance\0\
Too many \\( in RE\0\
More than two numbers given in \\{ \\}\0\
} expected after \\ in RE\0\
First number exceeds second in \\{ \\}\0\
End of edit file encountered during a multiline Insert verb\0\
Record(s) too long - truncated by G\0\
[ ] imbalance in RE\0\
RE overflow\0\
Cannot re-read from a pipe\0\
No transient file saved\0\
Transient file in use\0\
RE substitute too long\0\
Illegal RE substitute\0\
Last file operation was Save - use XS first and then XM\0\
Last file operation was Merge - use XM first and then XS\0\
Hex digits missing or invalid";

/* calculator char display */

FSTR asc_tab[][4]
  = {
  "NUL", "SOH",  "STX", "ETX",  "EOT", "ENQ", "ACK", "BEL",   "BS",  "HT",
  "LF",   "VT",   "FF",  "CR",   "SO",  "SI", "DLE", "DC1",  "DC2", "DC3",
  "DC4", "NAK",  "SYN", "ETB",  "CAN",  "EM", "SUB", "ESC",   "FS",  "GS",
  "RS",   "US",  "DEL"
  };

/*
 *  Help text.
 */

FSTR_LIST hw_mess[] = {
  "Screen Editor Key Strokes.\n\n\
\tSINGLE KEY COMMANDS\n\
^A\tWord move left\t\t\t^N\tOpen blank line\n\
^B\tWord delete left\t\t^Pc\tEnter control character \'c\'\n\
^C\tPage down\t\t\t^R\tPage up\n\
^D\tCursor right\t\t\t^S\tCursor left\n\
^E\tCursor up\t\t\t^T\tWord delete right\n\
^F\tWord move right\t\t\t^U\tRestore deleted/saved lines\n\
^G\tDelete character\t\t^V/Ins\tOverwrite/insert mode\n\
^H\tDelete character left\t\t^W\tScroll up one line\n\
^I\tHorizontal tab\t\t\t^X\tCursor down\n\
^J\tSave current line\t\t^Y\tDelete current line\n\
^L\tRepeat search/replace\t\t^Z\tScroll down one line\n\
^M\tSplit line",
  "\tQUICK COMMANDS\n\
^Q DEL\tErase to start of line\t\t^QA\tSearch and replace\n\
^QB\tJustify and move on\t\t^QC\tMove to end of file\n\
^QD/End\tCursor to end of line\t\t^QE\tCursor to top of screen\n\
^QF\tSearch for string\t\t^QGc\tFind character\n\
^QH/Home  Cursor Home\t\t\t^QI\tMove to line number\n\
^QJ/F1\tDisplay help text\t\t^QK/F9\tRecall home commands\n\
^QQ\tRepeat command\t\t\t^QR\tMove to top of file\n\
^QS\tCursor to start of line\t\t^QV\tMatch brackets/strings\n\
^QW\tFast repeat scroll up\t\t^QX\tCursor to bottom of screen\n\
^QY\tErase to end of line\t\t^QZ\tFast repeat scroll down",
  "\tBLOCK AND SAVE COMMANDS\n\
^KA\tAppend to file\t\t\t^KB/F2\tMark start of block\n\
^KC/F4\tCopy from marked block\t\t^KD\tContext editor\n\
^KE\tSpawn shell\t\t\t^KF\tList files\n\
^KH\tHome and move line to top\t^KK/F3\tMark end of block\n\
^KL\tChange directory\t\t^KO\tRe-read oldfile\n\
^KP\tPrint file\t\t\t^KQ/F7/ESC  Abandon edit\n\
^KR/F4\tRead from file\t\t\t^KS/F10\tSave file\n\
^KU\tUse (execute) edit file\t\t^KW/F3\tWrite to file\n\
^KX/F8\tSave file and exit\t\t^K?\tDisplay file statistics\n\
\n\tJUSTIFICATION\n\
^OB\tJustify paragraph\t\t^OC\tCentre current line\n\
^OL\tSet left margin\t\t\t^OR\tSet right margin\n\
^OS\tSet line spacing\t\t^OJ\tToggle right adjust\n\
^OW\tToggle wordwrap",
  NULL
};

#if FULL_G

FSTR_LIST h_mess[]
  = {
  "Help gives brief information on the following topics:-\n\n\
\tha\tArithmetic\n\
\thd\tG Delimiters\n\
\the\tList the available End-points\n\
\thl\tLine-Editor key strokes\n\
\thm\tThe predefined Macros\n\
\thr\tRegular Expression syntax\n\
\ths\tShorthand forms\n\
\thv\tContext-Editor Verbs\n\
\thw\tScreen-Editor key strokes",
  NULL
  };

FSTR_LIST hl_mess[]
  = {
  "Line-editor control characters - type these under the line.\n\
An elipsis indicates that a sequence is permitted.\n\n\
- ...\t\tDelete character above the hyphen\n\
% ...\t\tReplace character above with a space\n\
!text\t\tReplace remainder of the line with <text>\n\
!\t\tDelete remainder of line\n\
^text\t\tInsert <text>\n\
^\t\tSplit the line at this point\n\
<escape>\tIgnore any special meaning of the next character\n\
<space> ...\tHas no effect except to move the cursor along the line\n\
other ...\tReplace character in the line above with this one",
  NULL
  };

FSTR_LIST hs_mess[] = {
  "The following abbreviations are possible.\n\
Strings here are terminated by <return>, therefore any\n\
character including delimiters may be included freely.\n\n\
Command\t\t\tExpansion\tAction\n\
/<string><return>\tTR/string/\tSearch forwards for RE\n\
^<string><return>\tTR/^string/\tSearch constrained to SOL\n\
?<string><return>\tT#0 TR/string/\tSearch starting from TOF\n\
#digits\t\t\tT#digits\tMove to absolute line\n\
#\t\t\tT#0\t\tMove to start of file\n\
[+/-]digits\t\tT[+/-]digits\tMove relative\n\
[+/-]\t\t\tT[+/-]1\t\tMove up/down one line\n\
@\t\t\tT#{@}\t\tMove to original position\n\
digits\t\t\tT#digits\tMove to absolute line",
  NULL
};

FSTR_LIST hv_mess[] = {
  "G Verbs  (See also HE and HD).\n\n\
A(B)[R]/s1/s2/\tAfter (before) s1 insert s2 [RE]\n\
C/c#/text\tCreate macro, subs char c, num args #, macro text\n\
D<I opts>\tDisplay data (supplied as I verb except for ! and A/B)\n\
DP\t\tPause and wait for return key\n\
E\t\tEnds edit, saving file\n\
F\t\tForget previous command line\n\
G\t\tInvoke the G editor for another file\n\
H\t\tDisplay help\n\
I[X]/s/\t\tInserts [hex] string before pointer\n\
I[AB]\t\t  ..\ttext After or Before current line\n\
I[DFS]\t\t  ..\tDate, Filename of old file, Scale\n\
I!\t\t  ..\tfrom system command\n\
I[C] { expr }\t  ..\tresult of expr as number [byte]\n\
J[0]\t\tJoin to next [this] record\n\
JP\t\tJoin (format) paragraph",
  "Kendpt\t\tKill lines to endpoint\n\
Lendpt\t\tDisplay lines to endpoint\n\
L[ON|OFF]\tSwitch on/off information\n\
L[DSMHXV]\tFiles, Stats, Macros, History, line in heX, Version\n\
Mfilename\tUse filename as the old file\n\
Nformat\t\tSet format for output of numbers\n\
Ofilename\tChange old and new file\n\
Pendpt\t\tMove pointer to endpoint in old file\n\
Q\t\tAbandon edit, files are unchanged\n\
R[R]/s1/s2/\tReplace s1 with s2 [RE]\n\
S[NA]filename\tRedirect the output to (Append, New) filename\n\
SQ\t\tAbandon current save, leaving file untouched\n\
Tendpt\t\tTranscribe text from old file to new file\n\
Ufilename\tTake edit commands from <filename>",
  "Vendpt\t\tVerify endpt, but take no action\n\
W\t\tDisplay a window eight lines either side of current line\n\
X[MSUT]\t\tExit current Merge, Save, Use file, (T - to temp)\n\
X|\"command\"\tFilter text through command\n\
Y/s1/s2/\tCopy char and if in s1, replace with char in s2\n\
.macro/args/...\tCall <macro> with argument list\n\
!<command>\tRun a system command\n\
= | *\t\t(t1) re-execute last command\n\
$\t\tDisplay the Scale\n\
$n\t\tSet the portion of the line to be displayed\n\
{ expr }\tEvaluate the numerical expression\n\
:\t\tInvoke the Screen Editor\n\
;\t\tConditional execution in ()",
  NULL
};

FSTR_LIST he_mess[]
  = {
  "Endpoints for repetition and general verbs - see also HD.\n\
Column endpoints are prefixed with a dot (.).\n\
Line and column endpoints may be combined: line-ep.col-ep.\n\
Any number may be replaced with { expr }.\n\
Any string may be preceded with one or more of:-\n\
N - negate sense, X - string in hex, I - ignore case\n\n\
\tE\t\tTo End of file\n\
\t/.../\t\tLine/col starting with string\n\
\tS/.../\t\tAs above, but ignoring leading Spaces\n\
\tR/.../\t\tLine/col matching Regular Expression\n\
\tC/.../\t\tLine Containing string\n\
\tF/.../\t\tLine Finishing with string\n\
\t[-]S\t\tTo next (last) non-space column\n\
\t[+/-]number\t<number> lines/cols forwards or backwards\n\
\t*digits\t\tExactly <number> times\n\
\tOdigits\t\tUntil new-file line/col <number>\n\
\t#digits\t\tUntil absolute (old file) line/col <number>\n\
\tW|U<endpoint>\tRepeat while/until (verify) endpoint",
  NULL
  };

FSTR_LIST hd_mess[] = {
  "Delimiters for search strings.\n\n\
\t/ : = ? $ % & + ` \' \" [ ] < >\n\n\
Shell commands may only be delimited with \' \" or `\n\
The delimiter chosen must not appear within the string.",
  NULL
};

FSTR_LIST hr_mess[] = {
  "Regular Expressions are like wild cards.\n\n\
char or \\char matches itself except ...\n\
.\tmatches any character\n\
*\tmatches zero or more occurrences of previous RE\n\
\\{m\\}\t\tmatches exactly m occurrences of previous RE\n\
\\{m,\\}\t\t  ..\tat least m\t..\t..\t..\n\
\\{m,n\\}\t\t  ..\tm to n\t\t..\t..\t..\n\
\\abtnfrv\t  ..\tone of BEL, BS, HT, LF, FF, CR, VT\n\
\\xHH\t\t  ..\tcharacter represented by the two hex digits\n\
\\< and \\>\tmatch start or end of a word\n\
\\( and \\)\tmark segment for future use\n\
\\n\tmatches segment previously marked by n'th \\( .. \\)\n\
[ ]\tdelimits a character class, ] must be first char inside\n\
-\tin character class, indicates range of chars unless first char\n\
^\tconstrains match to start of line. In class RE, causes negation\n\
$\t\t..\t    end of line",
  "In the right hand side of substitute.\n\n\
char or \\char inserts itself except ...\n\
\\<abtnfrv>\tinserts one of BEL, BS, HT, LF, FF, CR, VT\n\
\\xHH\tinserts the character represented by the two hex digits\n\
&\t  ..\tentire matched left side\n\
\\n\t  ..\tthe segment marked by \\( .. \\), from LHS\n\
% (~)\tif alone, uses previous replacement (RE)\n\
\\U\tfold next char to upper case\n\
\\L\t\t..\t  lower case\n\
\\u\tstart folding to upper case\n\
\\l\t\t..\t lower case\n\
\\e\tend all case folding",
  NULL
};

FSTR_LIST hm_mess[] = {
  "Predefined Macros.\n\n\
.c/str1/str2/\t\tChange str1 to str2 from here on (simple)\n\
.r/RE1/RE2/\t\t\t..\t..\t..\t (RE)\n\
.tpri/lines/cols/ff/\tPaginated, line-numbered listings\n\
.clean\t\t\tStrip parity and control chars\n\
.lis/string/\t\tList lines containing string (ignore case)\n\
.grep/RE/\t\t    ..     matching RE\n\
.u\t\t\tConvert line to upper case\n\
.l\t\t\t\t..\tlower case\n\
.hex\t\t\t\t..\thex\n\
.tss/command/\t\tExecute system command\n\
.num/variable/\t\tRead a number into variable from the text\n\
.ph\t\t\tDelete from start of file to here\n\
.main\t\t\tInsert \'C\' main() template",
  NULL
};

FSTR_LIST ha_mess[]
  = {
  "The syntax of numerical expressions is the same as for C.\n\
There are 26 variables a - z, initialised to zero and static.\n\
The following constants are set:-\n\n\
\t#\tcurrent line (.# col) number in the OLD file\n\
\t&\tcurrent line (.& col) number in the NEW file\n\
\t@\tline (.@ col) number when the command started\n\
\t$\tnumber of lines (.$ cols) in the file (line)\n\
\t*\tvalue of the next character in the old file\n\
\t.\tresult of the last expression\n\n\
Usage: the macro .num/x/ reads a number from the text into a variable.\n\
I[C]{expr} inserts expr (byte) into text, D[C]{expr} on the screen.\n\
N<printf format> defines output style for I and D. {expr} may be used\n\
instead of a number in any endpoint, for example: T#{$-30} - move to\n\
30 lines before EOF, (t.1)w{* >= '0' && * <= '9'} - move past digits\n\
in text, (v{# > 20} i/x/; i/y/) etc. Use {expr} alone for side-effects\n\
such as initialising variables.  To use as a desk calculator, omit the\n\
closing }.  An empty line or closing } ends this mode.",
  NULL
  };

FSTR_LIST *const near help_tab[]
  = {
  hw_mess, ha_mess, he_mess, hr_mess, hl_mess,
  hv_mess, hd_mess, hm_mess, hs_mess, h_mess
  };

#endif  /* if FULL_G */

/* Case conversion translate table, also strips parity */

FSTR u_map[] = {
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,
 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D,
 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54,
 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,

 /* upper half of table strips parity and maps case */

 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,
 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D,
 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54,
 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F
};

/* 1 if delim, 2 if comsep, 4 if alpha, 8 if digit, 0 otherwise */

FBSTR g_map[] = {
 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x01, 0x00, 0x01, 0x01, 0x01,
 0x01, 0x02, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x08, 0x08, 0x08, 0x08,
 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x01, 0x02, 0x01, 0x01, 0x01, 0x21, 0x00,
 0x90, 0x50, 0x50, 0x10, 0x30, 0x30, 0x10, 0x10, 0x10, 0x50, 0x10, 0x70, 0x10,
 0x10, 0x90, 0x30, 0x10, 0xD0, 0x50, 0x10, 0x90, 0x10, 0xD0, 0x10, 0x10, 0x10,
 0x01, 0x00, 0x01, 0x00, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x80, 0x00, 0x00, 0x01, 0x02, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02,
 0x02, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

/*
 *  Predefined macros
 */

/* Change string1 to string2 from here on (simple) */

FSTR cha[] = "(tc\177%1\177 (r\177%1\177%2\177).e t)e @";

/* Change string1 to string2 from here on (RE's) */

FSTR rcha[] = "(tr\177%1\177 ((rr\177\177%2\177).e t tr//)e) @";

/* Paginated, line-numbered listings */

FSTR tpri[]
  =
    "{t=$/%1}((v%3 ix/0C0A/;) i/      / if(i/ /).o{%2/2-8}id(i/ /).o{%2-12}\
i/Page / n1 i{++p}i/ of / i{t*%1==$?t:t+1}(i////)*2 n10(i{#}i/  / t)%1)e\
(v%3 ix/0C/)";

/* Strip parity/control characters */

FSTR clr[] = "((ic{*&127}p.1).e t)e @((v{*<32|*>126}p.1;t.1).e t)e @";

/* List all lines containing string */

FSTR lis[] = "(tci\177%1\177 l0 t)e @";

/* Get regular expression and print (grep) */

FSTR grep[] = "(tr\177%1\177(l0 t tr//)e) @";

/* Convert line to upper case */

FSTR up[] = "rr/.*/\\u&/";

/* Convert line to lower case */

FSTR lo[] = "rr/.*/\\l&/";

/* Convert line to hex */

FSTR hex[] = "n02X(i{*}p.1).e";

/* Read in and move past number in text */

FSTR num[] = "{%1=0,..=1}t.s(v{* ==43}t.1;v{* ==45}{..=-1}t.1;) \
t.s({%1=%1*10+(*-48)}t.1)w{*>47&&*<58}{%1*=..}";

/* Delete up to the current line */

FSTR ph[] = "{..=#}# p{..}";

/* Write out a C main() prototype */

FSTR cmain[] = "i///#include <stdio.h>//#include <stdlib.h>\
////int//main( int argc, char ** argv )//    {////    }/////@";

/* Execute system command */

FSTR tss[] = "!%1";

private
const MACRO near mac_tab[] = { { mac_tab + 1,  cha,    "c",      '%',   2 },
                               { mac_tab + 2,  rcha,   "r",      '%',   2 },
                               { mac_tab + 3,  tpri,   "tpri",   '%',   3 },
                               { mac_tab + 4,  clr,    "clea",   '%',   0 },
                               { mac_tab + 5,  lis,    "lis",    '%',   1 },
                               { mac_tab + 6,  grep,   "grep",   '%',   1 },
                               { mac_tab + 7,  up,     "u",      '%',   0 },
                               { mac_tab + 8,  lo,     "l",      '%',   0 },
                               { mac_tab + 9,  hex,    "hex",    '%',   0 },
                               { mac_tab + 10, num,    "num",    '%',   1 },
                               { mac_tab + 11, ph,     "ph",     '%',   0 },
                               { mac_tab + 12, cmain,  "main",   '%',   0 },
                               { NULL,         tss,    "tss",    '%',   1 } };

private
int near l_numbers = NO,         /* Switch on or off line-numbered listings */
#if FULL_G
near show_template = NO,         /* display a template */
#endif  /* if FULL_G */
near lon           = NO;         /* listing flag */

private
char near n_format[8] = "%ld";   /* Format for printing numbers */

private
const char *near g_init = NULL;  /* Holds G initial command */

/* Globals for Context Editor (saved by save_all) */

private
int near g_rec = -1,             /* Current G record number (input file) */
near g_eof     = NO,             /* End of file flag (input file)        */
near i_eor     =  0,             /* Input file end of record             */
near i_col     =  0,             /* Input file current column            */
near e_col     =  0,             /* Output side of GE                    */
near o_rec     = -1,             /* Output file record number            */
near s_g_rec   =  0,             /* Saved G rec no for @ end cond        */
near s_g_col   =  0,             /* Saved G col (e_col) for @            */
near buff_sec  =  0,             /* Section number that ptr is in        */
near running   =  1;             /* Currently not parsing                */

private
word near depth = 0;  /* George nesting level */

/* used by save_all() to copy file offsets etc */

private
SAVE_AREA near g_save;

/* Editor buffers */

private
char *near i_buff, *near e_buff;

private
const char *near cmd_buf;

/* primary filenames */

private
const char *near in_fname = si_file, *near out_fname = so_file;

/* file descriptors for stdin/stdout (may be changed after redirection) */

private
int fd_in_terminal = 0, fd_out_terminal = 1;

/* do not allow overwrite of old file */

private
int ro_mode = NO;

/* 'N' for new/normal file, 'A' for append, 'F' for filter */

private
char near i_mode = 'N', near o_mode = 'N';

private
VERB *near g_free_list = NULL;

private
VERB_LIST near com_stack = {
  NULL, NULL, NULL, 0
};

private
VERB_LIST *near com_stack_ptr = &com_stack, *near par_stack_ptr = &com_stack;

private
stack *near hist_top = NULL;

private
FILE_LIST *near c_list = NULL,  /* list of command files open    */
*near f_list = NULL;            /* list of save/merge files open */

private
const MACRO *near mac_list = mac_tab;  /* list of macros defined */

private
int near infile_recs;  /* saved number of old file recs for EQ */

/* Environments for error trap returns */

private
jmp_buf near set_err, near save_err, near se_ret, near se_err;

/* justification parameters */

private
int near l_margin = 0, near r_margin = 68, near line_spacing = 0;

private
int near adjust = YES  /*, overhang = NO */;

private
int near left_right, near t_margin, near wordwrap = NO;

/* status line EOF column */

private
int near se_save_eof;

/* warn of truncated records on input */

private
int near trunc_recs = NO;

/* inserted lines stack */

private
stack *near in_stack = NULL;

private
int near in_count = 0;

/* deleted lines stack */

private
stack *near del_stack = NULL;

/* se only history stack ptr */

private
stack *near hist_ptr = NULL;

/* idlok needs clearing */

#if UNIX
private
int idlpending = NO;
#endif  /* if UNIX */

/* these depend on LINES & COLS in curses.h */

private
int near last_line, near last_col, near text_lines, near h_inc;

/* cursor tracking */

private
int near row, near col, near offset, near text_row, near text_col,
near text_offset, near last_offset;

/* currently in a QQ loop */

private
int near qq_loop = NO;

/* expand mode */

private
int near expand = YES;

/* in full screen mode */

private
word near fscreen = NO;

/* redisplay needed after error in CE */

private
word near redisplay = NO;

/* screen soft tab columns default to every 4 */

private
int near screen_tabs = 4;

/* main screen buffer */

private
char **near s_buf = NULL;

/* the "end of record" index */

private
short *near eor, *near s_eor;

private
FILE_LIST *near f_free_list = NULL;

/* status of trans_u */

private
int near trans_open_count = 0;

/*
 *  Portable virtual storage file system.
 */

private
PAGE_PTR *near page_list = NULL, *near page_list_end = NULL;

/* initial page and free page list */

private
byte *near v_free_list = NULL;

/* fixed primary units */

private
UNIT near primary_out;  /* allow forward use */

private
UNIT near primary_in = {
  NULL, NULL, &primary_out, 0, 0, 0, 0, PPP - 1, NO
};

private
UNIT near primary_out = {
  NULL, NULL, &primary_in, 0, 0, 0, 0, PPP - 1, NO
};

private
UNIT *near in_u = &primary_in,  *near out_u  = \
                  &primary_out, *near comm_u = NULL,
                  *near trans_u = NULL;

/* detect altered primary files */

private
int near prim_changed = NO;

/* no tab processing etc */

private
int near bin_mode = NO;

/*
 *  Find opposite page and clear both links
 */

#define clear_link(f, p) p->linked = f->list[p->linked - 1].linked = 0

/*
 *  Red tape control.
 */

#if DOS
# define put_len(p)                   \
  p[0] = (byte)len;                   \
  if (len > 254)                      \
    {                                 \
      p -= 2;                         \
      p[0] = (byte)255;               \
      *(short *)( p + 1 ) = len;      \
      len += 2;                       \
    }

# define get_len(p)                   \
  if ( ( len = *p++ ) > 254 )         \
    {                                 \
      len = *(short *)p;              \
      p += 2;                         \
    }
#else  /* if DOS */
# define put_len(p)                   \
  p[0] = (byte)len;                   \
  if (len > 254)                      \
    {                                 \
      p -= 2;                         \
      p[0] = (byte)255;               \
      p[1] = (byte)( len >> 8 );      \
      len += 2;                       \
    }

# define get_len(p)                   \
  if ( ( len = *p++ ) > 254 )         \
    {                                 \
      len = ( word ) * p++ << 8;      \
      len |= *p++;                    \
    }
#endif  /* if DOS */

#define skip_rec(p)                   \
  {                                   \
    get_len(p);                       \
    p += len;                         \
  }

/*
 *  Output text in line mode
 */

private
void
new_line(void)
{
  (void)!write(vdu_fd, pt_list, 1);
}

private
void
putstr(char csc s)
{
  (void)!write( vdu_fd, s, strlen(s) );
}

private
void
say(char csc s)
{
  putstr(s);
  new_line();
}

/*
 *  Wait for user to hit return
 */

private
void
wait_user(void)
{
  putstr(se_hit);
#if DOS
# if ASM86
  bios_wait();
# else  /* if ASM86  */
  (void)_bios_keybrd(_KEYBRD_READ);
# endif  /* if ASM86 */
#else  /* if DOS */
  {
    char buf[8];
    (void)!read(kbd_fd, buf, 8);
  }
#endif  /* if DOS */
}

/* malloc wrappers */

private
void *
getvec(const size_t len)
{
  void *const p = malloc(len + 1);

  if (p == NULL)
    {
      term();
      say("\rOut of core\r");
      _exit(1);
    }

  return p;
}

private
void
rlsevec(void *const ptr)
{
  free(ptr);
}

/*
 *  Expand tabs in buffer to every 8, except for leading whitespace.
 */

private
int
tabex(byte *b2, const byte *b1, int len)
{
  const byte *p, *const start = b2;
  int i, gap, col, new_col;
  const int maxlen = E_BUFF_LEN;

  /* delete trailing whitespace on input */

  while ( len && ( b1[len - 1] == SPACE || b1[len - 1] == TAB ) )
    {
      --len;
    }

  /* skip existing tabs */

  while (len && *b1 == TAB)
    {
      *b2++ = *b1++, --len;
    }

  /* compress any further spaces */

  while ( len >= TAB_WIDTH && ( *b1 == SPACE || *b1 == TAB ) )
    {
      for (i = 0; i < TAB_WIDTH && b1[i] == SPACE; ++i)
        {
          ;
        }

      if (i < TAB_WIDTH)
        {
          if (b1[i] != TAB)
            {
              break;
            }

          ++i;
        }

      b1 += i;
      len -= i;
      *b2++ = TAB;
    }
  if (len == 0)
    {
      return 0;
    }

  col = ( b2 - start ) * TAB_WIDTH;

  while ( ( p = (byte *)memchr(b1, TAB, len) ) != NULL)
    {

      /* move the inter-tab data */

      col += ( gap = p - b1 );
      if (col > maxlen)
        {
          (void)movelr( b2, b1, maxlen - ( col - gap ) );
          col = maxlen;
          len = 0;
          break;
        }

      b2 = (byte *)mmovelr(b2, b1, gap);
      b1 = p + 1;
      len -= gap;

      /* expand the tab */

      new_col = ( col / TAB_WIDTH + 1 ) * TAB_WIDTH;
      if (new_col > maxlen)
        {
          space_fill(b2, maxlen - col);
          col = maxlen;
          len = 0;
          break;
        }

      gap = new_col - col;
      b2 = mspace_fill(b2, gap);
      --len;
      col = new_col;
    }

  if (len + col > maxlen)
    {
      len = maxlen - col;
      ++trunc_recs;
    }

  (void)cwmovelr(b2, b1, len);

  return len + ( b2 - start );
}

/*
 *  Expand initial tabs only in buffer to every 8.
 */

private
int
ltabex(char *b2, const byte *b1, int len)
{
  char csc start = b2;

  if (!bin_mode)
    {
      while (len && *b1 == TAB)
        {
          ++b1;
          b2 = tab_fill(b2);
          --len;
        }
    }

  (void)cwmovelr(b2, b1, len);

  return len + ( b2 - start );
}

/*
 *  Transfer page to the free list.
 */

private
void
free_page(PAGE_PTR *const cp)
{
  *(byte **)cp->base = v_free_list;
  v_free_list = cp->base;
}

/*
 *  Clear all data pages owned by a file on close or reload.
 */

private
void
clear_pages(UNIT csc fp)
{
  PAGE_PTR *pp;

  for (pp = fp->list; pp->base != NULL; ++pp)
    {
      if (!pp->linked)
        {
          free_page(pp);
        }
    }
}

/*
 *  Get a new page, possible reusing an old one.
 */

private
byte *
new_page(void)
{
  byte *ad = v_free_list;

  if (ad != NULL)
    {
      v_free_list = *(byte **)ad;
      return ad;
    }

  if (page_list >= page_list_end)
    {
      if ( ( page_list = (PAGE_PTR *)malloc(PBLOCK * PP_SIZE) ) == NULL)
        {
          static int warned = 0;
          if (!warned++)
            {
              inform("WARNING: Low on memory!");
            }

          page_list = page_list_end;
          return (byte *)getvec(PAGE_SIZE);
        }

      page_list_end = page_list + PBLOCK;
    }

  ad = (byte *)page_list;
  page_list += PPP;
  return ad;
}

private
byte *
get_page(void)
{
  PAGE_PTR *pp;

  if (v_free_list != NULL)
    {
      byte *const ad = v_free_list;
      v_free_list = *(byte **)ad;
      return ad;
    }

  for (pp = in_u->list + in_u->eof_page; pp->base != NULL; ++pp)
    {
      if (!pp->linked)
        {
          free_page(pp);
          pp->base = NULL;
        }
    }

  return new_page();
}

/*
 *  Allocate a page table.
 */

private
PAGE_PTR *
get_pt(const int len)
{
  PAGE_PTR *const ad = page_list;

  if (len > page_list_end - ad)
    {
      return (PAGE_PTR *)getvec(len * PP_SIZE);
    }

  page_list += len;
  return ad;
}

/*
 *  Return a page table back to free list of data pages.
 */

private
void
free_pt(UNIT *const fp)
{
  PAGE_PTR *pp = fp->list, *const last = pp + fp->list_end;

  for (; pp < last; pp += PPP)
    {
      *(byte **)pp = v_free_list;
      v_free_list  = (byte *)pp;
    }
}

/*
 *  Alter page table sizes.
 */

private
void
extend_list(UNIT *const fp)
{
  const int old_end = fp->list_end + 1;
  PAGE_PTR *const new_pt = get_pt(old_end + PPP);

  (void)bmovelr(new_pt, fp->list, old_end * PP_SIZE >> B_SIZE);
  bzero( new_pt + old_end, PP_SIZE * ( PPP >> B_SIZE ) );
  free_pt(fp);
  fp->list = new_pt;
  fp->list_end += PPP;
}

/*
 *  Scan along a page to a particular record number.
 */

private
byte *
find_rec(UNIT csc fp, const int dest)
{
  byte *p;
  word len, rec;
  PAGE_PTR csc cp = fp->list + fp->page;

  if (dest == cp->rec)
    {
      return cp->base + cp->end_pos;
    }

  if (fp->page > 0)
    {
      rec = cp[-1].rec;
    }
  else
    {
      rec = 0;
    }

  p = cp->base;
  while (rec++ < dest)
    {
      skip_rec(p);
    }

  return p;
}

/*
 *  Disconnect a page and copy initial data.
 */

private
void
unlink_page(UNIT *const fp, PAGE_PTR *const cp, const word offset)
{
  byte csc link_base = cp->base;

  fp->rec_start = ( cp->base = get_page() ) + offset;
  clear_link(fp->link_u, cp);
  (void)cwmovelr(cp->base, link_base, offset);
}

/*
 *  Set up a normal link for vscopy
 */

private
void
set_link(PAGE_PTR *const cp1, PAGE_PTR *const cp2)
{
  if (cp1->linked)
    {
      unlink_page(in_u, cp1, cp1->end_pos);
    }

  if (cp2->linked)
    {
      clear_link(in_u, cp2);
      cp2->base = NULL;
    }

  if (cp2->base != NULL)
    {
      free_page(cp2);
    }

  *cp2 = *cp1;
  cp1->linked = cp2 - out_u->list + 1;
  cp2->linked = cp1 - in_u->list + 1;
}

/*
 *  Set final positions for a page, check link OK
 */

private
void
page_end(UNIT *const fp)
{
  PAGE_PTR *const cp = fp->list + fp->page;
  const ushort offset = fp->rec_start - cp->base;

  if (offset != cp->end_pos && cp->linked && offset != 0)
    {
      unlink_page(fp, cp, offset);
    }

  cp->rec = fp->rec_num;
  cp->end_pos = offset;
}

#if UNIX

/*
 *  Clear pages ahead of vsload for Insert type reads.
 */

private
void
vsunlink(UNIT *const fp)
{
  PAGE_PTR *cp = fp->list + fp->page;

  if (cp->linked)
    {
      unlink_page( fp, cp, (int)( fp->rec_start - cp->base ) );
    }

  for (++cp; cp->base != NULL; ++cp)
    {
      if (cp->linked)
        {
          clear_link(fp->link_u, cp);
          cp->base = get_page();
        }
    }
}

#endif  /* if UNIX */

/*
 *  Open a file by creating a new FCB and the page table.
 *  Assume open for write, as a close destroys all data.
 */

private
UNIT *
vsopen(void)
{
  UNIT *const fp = heap(UNIT);

  fp->list = (PAGE_PTR *)get_page();
  bzero( fp->list, PP_SIZE * ( PPP >> B_SIZE ) );
  fp->rec_start = fp->list->base = get_page();
  fp->rec_num = fp->page = fp->read = 0;
  fp->link_u = NULL;
  fp->list_end = PPP - 1;

  return fp;
}

/*
 *  Close a file, release all data (forever).
 */

private
void
vsclose(UNIT *const fp)
{

  /* Return all blocks, FCB etc back to pool */

  clear_pages(fp);
  free_pt(fp);
  rlsevec(fp);
}

/*
 *  Change the disposition of a file.
 */

private
void
vsreopen(UNIT *const fp)
{
  if ( ( fp->read = !fp->read ) == YES )
    {

      /*
       * Set eof here.  File cannot end up in read
       * mode without eof set as this is the only route
       */

      page_end(fp);
      fp->eof_rec = fp->rec_num;
      fp->eof_page = fp->page + 1;
    }

  vsrewind(fp);
}

/*
 *  Write a record to an open file.
 */

private
void
vsputrec(UNIT *const fp, const char *record, word len)
{
  int pg, i;
  PAGE_PTR *cp;
  ushort offset;
  byte buf[E_BUFF_SIZE + 3], *const start = buf + 3, *p = start;

  if (bin_mode)
    {
      (void)movelr(p, record, len);
    }
  else
    {  /* compress initial space */
      while (len >= TAB_WIDTH && *record == SPACE)
        {
          for (i = 0; i < TAB_WIDTH && record[i] == SPACE; ++i)
            {
              ;
            }

          if (i < TAB_WIDTH)
            {
              break;
            }

          record += i;
          len -= i;
          *p++ = TAB;
        }
      (void)cwmovelr(p, record, len);
      len += p - start;
    }

  p = buf + 2;
  put_len(p);

  pg = fp->page;
  cp = fp->list + pg;
  offset = fp->rec_start - cp->base;

  /* will record fit in this page ? */

  if (++len > PAGE_SIZE - offset)
    {
      page_end(fp);
      if ( ( fp->page = ++pg ) >= fp->list_end )
        {
          extend_list(fp);
        }

      cp = fp->list + pg;
      if (cp->base == NULL)
        {
          cp->base = get_page();
        }

      fp->rec_start = cp->base;
      offset = 0;
    }

  ++fp->rec_num;

  if (cp->linked)
    {
      if ( offset + len <= cp->end_pos && ecmp(fp->rec_start, p, len) )
        {
          fp->rec_start += len;
          return;
        }

      unlink_page(fp, cp, offset);
      ++prim_changed;
    }
  else
    {
      UNIT *const lu = fp->link_u;
      if (offset == 0 && lu == in_u && pg < lu->list_end)
        {
          PAGE_PTR *lp = lu->list + pg;
          if (lp->base != NULL && ecmp(lp->base, p, len) && !lp->linked)
            {
              if (cp->base != NULL)
                {
                  free_page(cp);
                }

              lp->linked = pg + 1;
              *cp = *lp;
              fp->rec_start = cp->base + len;
              return;
            }
        }
    }

  /* normal write */

  fp->rec_start = cwmmovelr(fp->rec_start, p, len);
}

private
void
vsload(UNIT *const fp, char csc record, word len)
{
  byte buf[E_BUFF_SIZE + 3], *p = buf + 2;
  PAGE_PTR *cp = fp->list + fp->page;
  const ushort offset = fp->rec_start - cp->base;

#if DOS
  if (record[len - 1] == '\r')
    {
      --len;
    }

#endif  /* if DOS */

  if (len > E_BUFF_LEN)
    {
      len = E_BUFF_LEN;
      ++trunc_recs;
    }

  if (bin_mode)
    {
      (void)movelr(buf + 3, record, len);
    }
  else
    {
      len = tabex(buf + 3, (const byte *)record, len);
    }

  put_len(p);

  /* will record fit in this page ? */

  if (++len > PAGE_SIZE - offset)
    {
      cp->rec = fp->rec_num;
      cp->end_pos = offset;
      if (++fp->page >= fp->list_end)
        {
          extend_list(fp);
        }

      cp = fp->list + fp->page;
      if (cp->base == NULL)
        {
          cp->base = (byte *)new_page();
        }

      fp->rec_start = cp->base;
    }

  /* normal write */

  fp->rec_start = cwmmovelr(fp->rec_start, p, len);

  ++fp->rec_num;
}

/*
 *  Read a record from an open file.
 */

private
int
vsgetrec(UNIT *const fp, byte cssc rec)
{
  word len;
  byte *cp = fp->rec_start;

  if (fp->rec_num >= fp->eof_rec)
    {
      return EOF;
    }

  if (++fp->rec_num > fp->list[fp->page].rec)
    {
      cp = fp->list[++fp->page].base;
    }

  get_len(cp);

  *rec = cp;

  fp->rec_start = cp + len;

  return len;
}

/*
 *  Copy two files fast.
 */

private
int
block_copy(UNIT *const fp2, UNIT *const fp1, const word dst, const int one)
{
  byte *ep1, *irs1, *rs2 = fp2->rec_start;
  PAGE_PTR *cp1 = fp1->list + fp1->page, *cp2 = fp2->list + fp2->page;
  register byte *rs1 = fp1->rec_start;
  word rec1 = fp1->rec_num, rec2 = fp2->rec_num, offset2 = rs2 - cp2->base,
    irec1, rem2, len;
  UNIT *const lu = fp2->link_u;

  if (cp2->linked)
    {
      unlink_page(fp2, cp2, offset2);
      rs2 = fp2->rec_start;
    }

  rem2 = PAGE_SIZE - offset2;
  ep1  = cp1->base + cp1->end_pos;
  while (cp1->rec < dst || ( ep1 = find_rec(fp1, dst) ) - rs1 > rem2)
    {
      irec1 = rec1;
      irs1 = rs1;

      /* find end of segment */

      if (ep1 - rs1 > rem2)
        {
          byte csc last = rs1 + rem2;
          while (rs1 <= last)
            {
              skip_rec(rs1);
              ++rec1;
            }
          rs1 -= len + 1;
          if (len > 254)
            {
              rs1 -= 2;
            }

          --rec1;
        }
      else
        {
          rs1  = ep1;
          rec1 = cp1->rec;
        }

      /* move segment */

      len = rs1 - irs1;
      (void)cwmovelr(rs2, irs1, len);

      /* next new file page */

      cp2->rec = rec2 += ( rec1 - irec1 );
      cp2->end_pos = (ushort)( offset2 + len );
      if (++fp2->page >= fp2->list_end)
        {
          extend_list(fp2);
        }

      cp2 = fp2->list + fp2->page;

      /* next old file page */

      if (rec1 >= cp1->rec)
        {
          ++fp1->page;
          if (one)  /* carry on linked */
            {
              return NO;
            }

          rs1 = ( ++cp1 )->base;
        }

      /* check status of this page now we */

      if (cp2->linked)  /* know its going to be written to */
        {
          clear_link(lu, cp2);
          cp2->base = get_page();
        }

      elif(cp2->base == NULL)
          cp2->base = get_page();

      offset2 = 0;
      rs2 = cp2->base;
      rem2 = PAGE_SIZE;
      ep1 = cp1->base + cp1->end_pos;
    }

  fp2->rec_start = cwmmovelr( rs2, rs1, (int)( ep1 - rs1 ) );
  fp1->rec_start = ep1;
  fp1->rec_num   = dst;
  fp2->rec_num   = rec2 + ( dst - rec1 );

  return YES;
}

/*
 *  Copy two files
 */

private
void
vscopy(UNIT *const fp2, UNIT *const fp1, const word end_rec)
{
  PAGE_PTR *cp1 = fp1->list + fp1->page, *cp2 = fp2->list + fp2->page;
  const int rec_offset = fp2->rec_num - fp1->rec_num,
    offset1 = fp1->rec_start - cp1->base,
    slow = ( end_rec <= cp1->rec && !cp2->linked ) || fp1->link_u != fp2;
  int pages;

  if (offset1 != fp2->rec_start - cp2->base || slow)
    {
      if ( block_copy(fp2, fp1, end_rec, !slow) )
        {
          return;
        }

      cp2 = fp2->list + fp2->page;
      if ( ( ++cp1 )->base != cp2->base )
        {
          set_link(cp1, cp2);
        }
    }

  if (cp1->rec < end_rec)
    {

      /* move remainder of current page */

      if (cp1->base != cp2->base)
        {
          if (cp2->linked)
            {
              unlink_page(fp2, cp2, offset1);
            }

          (void)cwmovelr(
                  fp2->rec_start, fp1->rec_start, cp1->end_pos - offset1);
          cp2->end_pos = cp1->end_pos;
        }

      if (end_rec == fp1->eof_rec)
        {
          pages = fp1->eof_page - fp1->page - 1;
        }
      else
        {
          PAGE_PTR *p = cp1;
          while (p->rec < end_rec)
            {
              ++p;
            }
          pages = p - cp1;
        }

      while (fp2->list_end <= fp2->page + pages)
        {
          extend_list(fp2);
          cp2 = fp2->list + fp2->page;
        }

      /* for each page */

      while (pages--)
        {
          cp2->rec = cp1->rec + rec_offset;
          ++cp1, ++cp2;
          if (cp1->base != cp2->base)
            {
              set_link(cp1, cp2);
            }
        }
      fp1->page = cp1 - fp1->list;
      fp2->page = cp2 - fp2->list;
    }

  elif(cp1->base != cp2->base)
      set_link(cp1, cp2);

  fp2->rec_start = fp1->rec_start = find_rec(fp1, end_rec);
  fp2->rec_num = ( fp1->rec_num = end_rec ) + rec_offset;
}

/*
 *  Seek to the given offset, the origin is record zero.
 */

private
void
vsseek(UNIT *const fp, const word offset)
{
  const PAGE_PTR *p;

  for (p = fp->list; p->rec < offset; ++p)
    {
      ;
    }

  fp->page = p - fp->list;
  fp->rec_num = offset;
  fp->rec_start = find_rec(fp, offset);
}

/*
 *  Save/Restore state of the editor.
 */

private
void
save_all(SAVE_AREA *const save)
{
  PAGE_PTR *const cp = out_u->list + out_u->page;

  if (cp->rec < out_u->rec_num)
    {
      cp->rec = out_u->rec_num;
      cp->end_pos = (short)( out_u->rec_start - cp->base );
    }

  save->in_rec_num = vstell(in_u);
  save->g_rec = g_rec;
  save->g_eof = g_eof;
  save->i_eor = i_eor;
  save->i_col = i_col;
  save->e_col = e_col;
  save->o_rec = o_rec;
  if (i_eor > 0)
    {
      if (save->in_rec_len < i_eor)
        {
          if (save->in_rec_len > 0)
            {
              rlsevec(save->in_rec);
            }

          save->in_rec = getbuf(save->in_rec_len = i_eor);
        }

      (void)movelr(save->in_rec, i_buff, i_eor);
    }

  if (e_col > 0)
    {
      if (save->out_rec_len < e_col)
        {
          if (save->out_rec_len > 0)
            {
              rlsevec(save->out_rec);
            }

          save->out_rec = getbuf(save->out_rec_len = e_col);
        }

      (void)movelr(save->out_rec, e_buff, e_col);
    }
}

private
void
rest_in_vars(SAVE_AREA csc save)
{
  g_rec = save->g_rec;
  g_eof = save->g_eof;
  i_eor = save->i_eor;
  i_col = save->i_col;
  tmovelr(i_buff, save->in_rec, i_eor);
}

private
void
rest_out_vars(SAVE_AREA csc save)
{
  e_col = save->e_col;
  o_rec = save->o_rec;
  tmovelr(e_buff, save->out_rec, e_col);
}

private
void
rest_all(SAVE_AREA csc save)
{
  rest_in_vars(save);
  if (!in_u->read)
    {
      vsreopen(in_u);
    }

  vsseek(in_u, save->in_rec_num);
  rest_out_vars(save);
  if (out_u->read)
    {
      vsreopen(out_u);
    }

  vsseek(out_u, o_rec + 1);
}

/*
 *  Fill the input buffer.
 */

private
int
fill_buff(void)
{
  const byte *p;
  const int len = vsgetrec(in_u, &p);

  i_col = 0;
  if (len == EOF)
    {
      g_eof = YES;
      i_eor = 0;
      return NO;
    }

  ++g_rec;
  i_eor = ltabex(i_buff, p, len);
  return YES;
}

/*
 *  Initialize the edit buffers.
 */

private
void
init_line(void)
{
  e_col = 0;
  g_eof = NO;
  g_rec = o_rec = -1;
  (void)fill_buff();
  save_all(&g_save);
}

/*
 *  Initialise the two primary files.
 */

private
void
vsprimary(void)
{
  PAGE_PTR *pp;
  const PAGE_PTR *last;
  int pg;

  bzero(in_u->list, ( in_u->list_end + 1 ) * PP_SIZE >> B_SIZE);
  out_u->rec_start = in_u->list->base = in_u->rec_start;
  if (in_fname != no_file)
    {
      if ( ( infile_recs = Disk_to_mem(in_fname, in_u, i_mode) ) == EOF)
        {
          in_fname = no_file;
        }
    }

  vsreopen(in_u);

  for (pg = 0, pp = in_u->list, last = pp + in_u->eof_page; pp < last; ++pp)
    {
      pp->linked = ++pg;
    }

  pg = out_u->list_end = in_u->list_end;
  out_u->list = get_pt(++pg);
  (void)bmovelr(out_u->list, in_u->list, pg * PP_SIZE >> B_SIZE);

  init_line();
}

/*
 *  Restart edit with new primary files.
 */

private
void
vsreload(void)
{
  PAGE_PTR *cp;

  for (cp = in_u->list + 1; cp->base != NULL; ++cp)
    {
      free_page(cp);
    }

  vsrewind(in_u);
  in_u->read = NO;

  clear_pages(out_u);
  free_pt(out_u);
  out_u->rec_num = out_u->page = 0;

  vsprimary();
}

#ifdef DEBUG
# define IMMEDOK
# include <assert.h>

/* check integrity of primary files */

private
void
ckpage(UNIT csc fp, PAGE_PTR csc cp)
{
  word len, rec, ep, end_rec, this_page = cp - fp->list;
  const byte *p = cp->base;

  assert(p != NULL);
  if (fp->read)
    {
      assert(this_page < fp->eof_page);
    }
  else
    {
      assert(this_page <= fp->page);
    }

  if (fp->read || this_page < fp->page)
    {
      end_rec = cp->rec;
      ep = cp->end_pos;
      if (ep == 0)
        {
          assert(end_rec + fp->eof_rec + fp->rec_num + this_page == 0);
          assert(fp->read && fp->rec_start == p && fp->eof_page == 1);
        }
    }
  else
    {
      end_rec = fp->rec_num;
      ep = fp->rec_start - p;
    }

  assert(ep <= PAGE_SIZE);
  if (this_page != 0)
    {
      rec = cp[-1].rec;
      assert(end_rec > rec);
    }
  else
    {
      rec = 0;
    }

  while (rec++ < end_rec)
    {
      skip_rec(p);
    }
  assert(rec == end_rec + 1);
  assert(p == cp->base + ep);
}

private
void
cklinked(UNIT csc fp, PAGE_PTR csc cp)
{
  UNIT csc lu = fp->link_u;
  const PAGE_PTR *lp = lu->list, *const last = lp + lu->list_end;
  byte csc data = cp->base;
  int linked_pages = 0;

  do
    {
      if (lp->base == data)
        {
          ++linked_pages;
        }
    }
  while ( ++lp < last );
  assert(linked_pages == 1);
  lp = lu->list + cp->linked - 1;
  assert(lp->base == data);
  assert(lp->linked == cp - fp->list + 1);
  if (lp->end_pos == 0)
    {
      assert(lu->rec_start == data);
      assert(lp->rec + lu->rec_num + lu->page == 0);
      if (lu->read)
        {
          assert(lu->eof_page == 1);
        }
    }

  if (lp->end_pos != cp->end_pos)  /* just after vsreopen */
    {
      assert(cp->end_pos == 0 || lp->end_pos == 0);
    }
}

private
void
ckunit(UNIT csc fp)
{
  const PAGE_PTR *cp = fp->list + fp->page, *last = cp + 1;
  const word offset = fp->rec_start - cp->base;

  assert(PAGE_SIZE < BLOCK_SIZE);
# if TINY_G
  assert(sizeof ( PAGE_PTR ) == 10);
# else  /* if TINY_G */
  assert(sizeof ( PAGE_PTR ) == 12);
# endif  /* if TINY_G */
  if (fp->read)
    {
      assert(fp->page < fp->eof_page);
      assert(fp->eof_page <= fp->list_end);
      assert(fp->rec_num <= cp->rec);
      assert(offset <= cp->end_pos);
      last = fp->list + fp->eof_page;
    }
  else
    {
      assert(fp->page < fp->list_end);
      assert(offset <= PAGE_SIZE);
    }

  if (offset == 0)
    {
      assert(fp->rec_num + fp->page == 0);
    }

  for (cp = fp->list; cp < last; ++cp)
    {
      ckpage(fp, cp);
      if (cp->linked)  /* check other page pointer */
        {
          cklinked(fp, cp);
        }
    }

  while (cp->base != NULL)
    {
      ++cp;
    }
  for (last = fp->list + fp->list_end; cp <= last; ++cp)
    {
      assert(cp->base == NULL);
    }
}

#endif  /* ifdef DEBUG */

/*
 *  Make character printable.
 */

private
char
toprint(int c)
{
  c = toascii(c);
  if ( iscntrl(c) )
    {
      c = CNTRL(c) + 0x40;
    }

  return (char)c;
}

/*
 *  Format G error messages.
 */

private
void
err_print(char *const err_text, const int code, const char *ptr)
{
  char *p, c;
  const char *mess = g_mess, *last;
  int i = code;

  while (i--)
    {
      mess = get_eos(mess) + 1;
    }

  if (code == FILE_ERROR && errno != 0)
    {
      (void)sprintf( err_text, "%s - %s.", mess, strerror(errno) );
      errno = 0;
    }
  else
    {
      (void)sprintf(err_text, "%s.", mess);
    }

  if (ptr == NULL && com_stack_ptr->prog != NULL)
    {
      ptr = com_stack_ptr->prog->errp;
    }

  if ( ptr != NULL && !nullstr(ptr) )
    {
      p = mcmovelr(err_text, "\nCulprit: ");
      last = p + L_LEN / 2;
      while (p < last && ( c = *ptr++ ) != EOS)
        {
          if (c == DEL)
            {
              c = SLASH;
            }

          *p++ = toprint(c);
        }
      *p = EOS;
    }
}

/*
 *  GE error handler.
 */

private
void
g_err(const int code, char csc ptr)
{
  VERB opt;
  char buf[256];

  err_print(buf, code, ptr);

  while (c_list != NULL)
    {
      (void)sprintf(
        get_eos(buf),
        "\nCalled from edit file %s (line %d)",
        c_list->name,
        vstell(comm_u) );
      c_comm_u();
    }

  inform(buf);

  if (fscreen)
    {
      while (f_list != NULL)
        {
          opt.o1.q = ( f_list->disp == 'M' ) ? 'M' : 'Q';
          Xit(&opt);
        }
    }

  if (running)
    {
      rest_all(&g_save);
    }

  longjmp(set_err, YES);
}

/*
 *  Translate characters.
 */

private
byte
xlat(const int c, char csc s1, byte csc s2)
{
  char csc p = strchr(s1, c);

  if (p == NULL)
    {
      return (byte)c;
    }

  return s2[p - s1];
}

/*
 *  Return the value for the two digit hex number at P.
 */

private
byte
xtoc(char csc p)
{
  const char *p1, *p2;

  if ( ( p1 = (const char *)memchr(hextrans, u_star(p), 16) ) == NULL
      || ( p2 = (const char *)memchr(hextrans, u_star(p + 1), 16) ) == NULL)
    {
      g_err(HEX_INV, NULL);
    }

  return (byte)( ( p1 - hextrans ) << 4 | ( p2 - hextrans ) );
}

/*
 *  String Handling.
 */

#define CBRA     01
#define CKET     02
#define CDOL     03
#define CCEOF    05
#define SWORD    06
#define EWORD    07

#define CCHR    010
#define CDOT    014
#define CCL     020
#define CBACK   024

#define STAR    01U
#define RNGE    03U
#define CLOSURE 03U

#define NBRA      9

#define CCL_SIZE 32 /* allow 8 bit characters, use 16 for 7 bit */

#if TINY_G
# define RE_BUFF_SIZE 512
#else  /* if TINY_G */
# define RE_BUFF_SIZE 1024
#endif  /* if TINY_G */

#define PLACE(c)   ( ep[c >> 3] |= bittab[c & 07] )
#define ISTHERE(c) ( ep[c >> 3]  & bittab[c & 07] )

private
const char *near braslist[NBRA], *near braelist[NBRA], *near loc1,
*near loc2 = NULL;

private
int near circf, near low, near range, near l2rec;

private
char near nbra;

private
byte expbuf[RE_BUFF_SIZE + CCL_SIZE + 2];  /* for compiled RE */

private
word near cflags;

private
const byte near bittab[] = {
  1, 2, 4, 8, 16, 32, 64, 128
};

/* character translations for rhs */

#define C_UPPER       1
#define C_LOWER       2
#define C_NEXT_UPPER  4
#define C_NEXT_LOWER  8

/*
 *  Place and retrieve length for RNGE, now allows >255 to be given.
 */

private
byte *
putrnge(byte *ep, const word len)
{
  *ep++ = (byte)len;
  if (len > 254)
    {
      ep[-1] = (byte)255;
      *ep++  = (byte)( len >> 8 );
      *ep++  = (byte)len;
    }

  return ep;
}

private
const byte *
getrnge(const byte *ep)
{
  word len;

  if ( ( len = *ep++ ) > 254 )
    {
      len = ( word ) * ep++ << 8;
      len |= *ep++;
    }

  low = len;
  if ( ( len = *ep++ ) > 254 )
    {
      len = ( word ) * ep++ << 8;
      len |= *ep++;
    }

  range = len - low;
  return ep;
}

/*
 *  Compile regular expression into a dfa.
 */

private
void
compile(char csc instring, const int len)
{
  byte *ep = expbuf, *lastep = NULL;
  const char *sp = instring;
  word c, lc;
  byte csc endbuf = ep + RE_BUFF_SIZE;
  char csc endsp  = sp + len;
  char bracket[NBRA], *bracketp = bracket;
  int closed, neg, cflg, i;

  circf = closed = nbra = 0;

  if (*sp == '^')
    {
      ++circf, ++sp;
    }

  while (sp < endsp)
    {
      if (ep >= endbuf)
        {
          g_err(RE_TOO_LONG, instring);
        }

      c = *sp++;
      if ( c != '*' && ( c != '\\' || *sp != '{' ) )
        {
          lastep = ep;
        }

      switch (c)
        {
        case '.':
          *ep++ = CDOT;
          continue;

        case '*':
          if (lastep == NULL || *lastep & CLOSURE)
            {
              goto defchar;
            }

          *lastep |= STAR;
          continue;

        case '$':
          if (sp < endsp)
            {
              goto defchar;
            }

          *ep++ = CDOL;
          break;

        case '[':
          *ep++ = CCL;
          bzero(ep, CCL_SIZE >> B_SIZE);
          lc = neg = 0;
          if ( ( c = *sp++ ) == '^' )
            {
              ++neg;
              c = *sp++;
            }

          do
            {
              if (c == EOS)
                {
                  g_err(RE_CCL_IMB, instring);
                }

              if (c == '-' && lc != 0)
                {
                  if ( ( c = *sp++ ) == ']' )
                    {
                      PLACE('-');
                      break;
                    }

                  while (lc < c)
                    {
                      PLACE(lc);
                      lc++;
                    }
                }

              lc = c;
              PLACE(c);
            }
          while ( ( c = *sp++ ) != ']' );

          if (neg)
            {
              for (i = 0; i < CCL_SIZE; ++i)
                {
                  ep[i] ^= BYTE_MASK;
                }

              ep[0] &= 0376;  /* don't match EOS */
            }

          ep += CCL_SIZE;
          continue;

        case BSLASH:
          switch (c = *sp++)
            {
            case '(':
              if (nbra >= NBRA)
                {
                  g_err(RE_TOO_MANY_BRA, instring);
                }

              *ep++ = CBRA;
              *bracketp++ = *ep++ = nbra++;
              continue;

            case ')':
              if (bracketp <= bracket)
                {
                  g_err(MISSING_BRA, instring);
                }

              *ep++ = CKET;
              *ep++ = *--bracketp;
              ++closed;
              continue;

            case '<':
              *ep++ = SWORD;
              continue;

            case '>':
              *ep++ = EWORD;
              continue;

            case '{':
              if (lastep == NULL || *lastep & CLOSURE)
                {
                  goto defchar;
                }

              *lastep |= RNGE;
              cflg = 0;
nlim:
              c = *sp++;
              i = 0;
              do
                {
                  if (c < '0' || c > '9')
                    {
                      g_err(BAD_NUM, instring);
                    }

                  i = i * 10 + c - '0';
                }
              while ( ( ( c = *sp++ ) != '\\' ) && ( c != ',' ) );
              ep = putrnge(ep, i);
              if (c == ',')
                {
                  if (cflg++)
                    {
                      g_err(RE_END_RANGE, instring);
                    }

                  if ( ( c = *sp++ ) == BSLASH )
                    {
                      ep = putrnge(ep, E_BUFF_SIZE);
                    }
                  else
                    {
                      --sp;
                      goto nlim;  /* get 2'nd number */
                    }
                }

              if (*sp++ != '}')
                {
                  g_err(RE_CLOSE_CURLY, instring);
                }

              if (!cflg)  /* one number */
                {
                  ep = putrnge(ep, i);
                }
              else
                {
                  (void)getrnge(lastep + 2);
                  if (range < low)
                    {
                      g_err(RE_1_GT_2, instring);
                    }
                }

              continue;

            case PNUMERIC:
              if ( ( c -= '1' ) >= closed )
                {
                  g_err(RE_BACKREF, instring);
                }

              *ep++ = CBACK;
              *ep++ = (char)c;
              continue;

            case 'x':
              c = xtoc(sp);
              sp += 2;
              break;

            default:
              c = xlat(u_map[c], esc_symb, esc_char);
            }
          /* fallthrough */
          /* Drop through to default to use \ to turn off special chars */

defchar:
        /* fallthrough */
        default:
          lastep = ep;
          *ep++  = CCHR;
          *ep++  = (char)c;
          /* fallthrough */
        }
    }

  *ep++ = CCEOF;
  if (bracketp != bracket)
    {
      g_err(MISSING_BRA, instring);
    }
}

private
int
advance(const char *lp, const byte *ep)
{
  const char *curlp, *bbeg;
  const byte *nep;
  int   ct;
  word  c;

  repeat switch (*ep++)
    {
    case CCHR:
      if (*ep++ == *lp++)
        {
          continue;
        }

      return NO;

    case CDOT:
      if (*lp++)
        {
          continue;
        }

      return NO;

    case CDOL:
      if (*lp)
        {
          return NO;
        }
      /* fallthrough */

    case CCEOF:
      loc2 = lp;
      l2rec = g_rec;
      return YES;

    case CCL:
      c = ( byte ) * lp++;
      if ( ISTHERE(c) )
        {
          ep += CCL_SIZE;
          continue;
        }
      /* fallthrough */

      return NO;

    case CBRA:
      braslist[*ep++] = lp;
      continue;

    case CKET:
      braelist[*ep++] = lp;
      continue;

    case CBACK:
      bbeg = braslist[*ep];
      ct = braelist[*ep++] - bbeg;
      if ( ecmp(bbeg, lp, ct) )
        {
          lp += ct;
          continue;
        }

      return NO;

    case SWORD:
      if ( ( lp == loc1 || !wordch(lp[-1]) ) && wordch(lp[0]) )
        {
          continue;
        }

      return NO;

    case EWORD:
      if ( lp > loc1 && wordch(lp[-1]) && !wordch(lp[0]) )
        {
          continue;
        }

      return NO;

    case CBACK | RNGE:
      bbeg = braslist[*ep];
      ct = braelist[*ep++] - bbeg;
      ep = getrnge(ep);
      curlp = lp;
      while ( low-- && ecmp(bbeg, lp, ct) )
        {
          lp += ct;
        }
      while ( range-- && ecmp(bbeg, lp, ct) )
        {
          lp += ct;
        }
      goto cstar;

    case CBACK | STAR:
      bbeg = braslist[*ep];
      ct = braelist[*ep++] - bbeg;
      curlp = lp;
      while ( ecmp(bbeg, lp, ct) )
        {
          lp += ct;
        }

cstar:
      while (lp >= curlp)
        {
          if ( advance(lp, ep) )
            {
              return YES;
            }

          lp -= ct;
        }
      return NO;

    case CCHR | RNGE:
      c = *ep++;
      ep = getrnge(ep);
      while (low--)
        {
          if (*lp++ != c)
            {
              return NO;
            }
        }
      curlp = lp;
      while (range--)
        {
          if (*lp++ != c)
            {
              break;
            }
        }
      if (range < 0)
        {
          ++lp;
        }

      goto star;

    case CDOT | RNGE:
      ep = getrnge(ep);
      while (low--)
        {
          if (*lp++ == EOS)
            {
              return NO;
            }
        }
      curlp = lp;
      while (range--)
        {
          if (*lp++ == EOS)
            {
              break;
            }
        }
      if (range < 0)
        {
          ++lp;
        }

      goto star;

    case CCL | RNGE:
      nep = getrnge(ep + CCL_SIZE);
      while (low--)
        {
          c = ( byte ) * lp++;
          if ( !( ISTHERE(c) ) )
            {
              return NO;
            }
        }
      curlp = lp;
      while (range--)
        {
          c = ( byte ) * lp++;
          if ( !( ISTHERE(c) ) )
            {
              break;
            }
        }
      if (range < 0)
        {
          ++lp;
        }

      ep = nep;
      goto star;

    case CCHR | STAR:
      curlp = lp;
      while (*lp++ == *ep)
        {
          ;
        }
      ++ep;
      goto star;

    case CCL | STAR:
      curlp = lp;
      do
        {
          c = ( byte ) * lp++;
        }
      while ( ISTHERE(c) );
      ep += CCL_SIZE;
      goto star;

    case CDOT | STAR:
      curlp = lp;
      while (*lp++)
        {
          ;
        }

star:
      while (--lp >= curlp)
        {
          if ( advance(lp, ep) )
            {
              return YES;
            }
        }
      return NO;
    }
}

private
int
step(const char *lp)
{
  const byte *ep = expbuf;

  if (circf)
    {
      return i_col == 0 && advance(lp, ep);
    }

  /* fast check for first character */

  if (*ep == CCHR)
    {
      const int c = ep[1];
      ep += 2;
      while ( ( lp = strchr(lp, c) ) != NULL)
        {
          if ( advance(++lp, ep) )
            {
              loc1 = lp - 1;
              return YES;
            }
        }
      return NO;
    }

  /* regular algorithm */

  do
    {
      if ( advance(lp, ep) )
        {
          loc1 = lp;
          return YES;
        }
    }
  while ( *lp++ );

  return NO;
}

private
char
xcase(const int c)
{
  const word f = cflags;

  cflags = f & ~( C_NEXT_UPPER | C_NEXT_LOWER );
  return (char)( ( f & C_NEXT_UPPER || \
                   f == C_UPPER ) ? u_map[c] : tolower(c) );
}

/*
 *  Substitute previous matched RE
 */

private
char *
place(char *sp, const char *p1, const char *p2, char csc end_buff)
{
  while (p1 < p2)
    {
      *sp++ = cflags ? xcase(*p1++) : *p1++;
      if (sp >= end_buff)
        {
          g_err(RHS_TOO_LONG, NULL);
        }
    }
  return sp;
}

private
int
re_sub(const char *ep, char *const str)
{
  static char *s_save, *e_save;
  static int s_save_len = -1, e_save_len = -1;
  char c, *sp = str;
  char csc end_buff = str + E_BUFF_LEN;
  int len;

  if ( equal1(ep, '~') )
    {
      if (e_save_len < 0)
        {
          g_err(NO_PREV_RE, NULL);
        }
    }
  else
    {
      if ( ( len = size(ep) ) > e_save_len )
        {
          if (e_save_len > 0)
            {
              rlsevec(e_save);
            }

          if ( ( e_save_len = len ) > 0 )
            {
              e_save = getbuf(len);
            }
        }

      tmovelr(e_save, ep, len);
    }

  ep = e_save;

  if ( equal1(ep, '%') )
    {
      if (s_save_len < 0)
        {
          g_err(NO_PREV_RE, NULL);
        }

      tmovelr(sp, s_save, s_save_len);
      return s_save_len;
    }

  cflags = 0;
  while ( ( c = *ep++ ) != EOS )
    {
      if (c == '&')
        {
          sp = place(sp, loc1, loc2, end_buff);
          continue;
        }

      if (c == '\\')
        {
          switch (c = *ep++)
            {
            case EOS:
              g_err(ILL_RHS_STR, NULL);
              /* fallthrough */

            case 'U':
              cflags = cflags & ~C_NEXT_LOWER | C_NEXT_UPPER;
              continue;

            case 'u':
              cflags = cflags & ~C_LOWER | C_UPPER;
              continue;

            case 'L':
              cflags = cflags & ~C_NEXT_UPPER | C_NEXT_LOWER;
              continue;

            case 'l':
              cflags = cflags & ~C_UPPER | C_LOWER;
              continue;

            case 'e':
              cflags = 0;
              continue;

            case PNUMERIC:
              if ( ( c -= '1' ) >= nbra )
                {
                  g_err(RE_BACKREF, str);
                }

              sp = place(sp, braslist[c], braelist[c], end_buff);
              continue;

            case 'x':
              c = xtoc(ep);
              ep += 2;
              break;

            default:
              c = xlat(u_map[c], esc_symb, esc_char);
            }
        }

      *sp++ = cflags ? xcase(c) : c;
      if (sp >= end_buff)
        {
          g_err(RHS_TOO_LONG, str);
        }
    }

  if ( ( len = (int)( sp - str ) ) > s_save_len)
    {
      if (s_save_len > 0)
        {
          rlsevec(s_save);
        }

      if ( ( s_save_len = len ) > 0 )
        {
          s_save = getbuf(len);
        }
    }

  tmovelr(s_save, str, len);
  return len;
}

/*
 *  Leave editor without updating files.
 */

private
void
Quit(void)
{
  term();
  if (lon)
#if UNIX
    {
      say("Edit abandoned.");
    }

#else  /* if UNIX  */
    {
      putstr("Edit abandoned.");
    }
#endif  /* if UNIX */
  _exit(0);
}

/*
 *  Get a command line, from initial commands, file or the vdu
 */

private
int
get_com(char *str, char csc c_prompt)
{
  const byte *p;
  int len;

  if (c_list != NULL)
    {
      if ( ( len = vsgetrec(comm_u, &p) ) == EOF )
        {
          c_comm_u();
          return YES;
        }

      if (len >= STR_LEN)
        {
          g_err(COMM_TOO_LONG, NULL);
        }

      (void)movelrz(str, p, len);
      if (lon)
        {
          say(str);
        }

      return NO;
    }

  if (g_init != NULL)
    {
      (void)zmovelr(str, g_init);
      g_init = NULL;
      return NO;
    }

  lon = YES;
  putstr(c_prompt);
  if ( ( len = read(kbd_fd, str, STR_LEN) ) <= 0 )
    {
      Quit();
      abort();
    }

  str[len - 1] = EOS;

  return NO;
}

/*
 *  Get delimited substring.
 */

private
int
gdss(char *const buf, int *const buf_len, char cssc ptr)
{
  char ch_delim;
  char csc str = *ptr, *p;
  int len;

  if ( !( delim(ch_delim = str[-1]) ) )
    {
      g_err(M_DELIM, str - 1);
    }

  if ( ( p = strchr(str, ch_delim) ) == NULL )
    {
      p = get_eos(str);
    }

  if ( ( len = p - str ) > STR_LEN )
    {
      g_err(S_STR_LEN, str);
    }

  (void)movelrz(buf, str, len);
  *buf_len = len;
  if (*p == EOS)
    {
      *ptr = p;
      return YES;
    }

  *ptr = p + 1;
  return NO;
}

/*
 *  Actions on lines.
 */

private
void
altr_line(int col, const int comm)
{
  if (col + i_col > i_eor)
    {
      if (!depth)
        {
          g_err(END_OF_LINE, NULL);
        }

      col = i_eor - i_col;
    }

  if (col + i_col < 0)
    {
      g_err(B_BOFLINE, NULL);
    }

  if (comm == 'T')
    {
      if (col + e_col > E_BUFF_LEN)
        {
          g_err(LINE_TOO_LONG, NULL);
        }

      (void)movelr(e_buff + e_col, i_buff + i_col, col);
      e_col += col;
    }

  i_col += col;
}

/*
 *  Execute a t.e
 */

private
void
get_end(void)
{
  const int i_len = i_eor - i_col;

  if (e_col + i_len > E_BUFF_LEN)
    {
      g_err(LINE_TOO_LONG, NULL);
    }

  (void)movelr(e_buff + e_col, i_buff + i_col, i_len);
  e_col += i_len;
  i_col = i_eor;
}

/*
 *  Write the output side of GE to the current output unit.
 */

private
void
out_buff(void)
{
  vsputrec(out_u, e_buff, e_col);
  e_col = 0;
  ++o_rec;
}

/*
 *  Flush the output buffer.
 */

private
void
flush_buff(void)
{
  if (e_col + i_col)  /* Some stuff in the buffers */
    {
      get_end();
      out_buff();
    }
  else
    {
      vsputrec(out_u, i_buff, i_eor);  /* Flush i_buff as no change */
      ++o_rec;
    }
}

/*
 *  Make a line printable for display.
 */

private
void
printable(char *p, int len)
{
  while (len--)
    {
      *p = toprint(*p);
      ++p;
    }
}

/*
 *  Display a line, possibly with a preceding line-number.
 */

private
void
n_print(char csc line, int len, const int arrowed)
{
  string buf;
  char *p;
  int rem = L_LEN;

  *buf = EOS;
  if (l_numbers)
    {
      rem -= sprintf(buf, n_format, (long)g_rec);
    }

  if (len > rem)
    {
      len = rem;
    }

  p = mcmovelr(buf, arrowed ? ">>" : "  ");
  (void)movelrz(p, line, len);
  printable(p, len);
  say(buf);
}

/*
 *  Add next line to the buffer
 */

private
int
add_line(const int comm)
{
  int rc;

  if (!g_eof && comm == 'T')
    {
      flush_buff();
    }

  rc = fill_buff();
  if (comm == 'L')
    {
      n_print(i_buff, i_eor, NO);
    }

  return rc;
}

/*
 *  GE string matching code.
 */

private
int
contains(const char *ptr, char csc str, const int len, char csc last)
{
  const int c = *str;

  while ( ptr <= last &&
           ( ptr = (const char *)memchr( ptr, c, ( last - ptr ) + 1 ) )
             != NULL )
    {
      if ( ecmp(ptr, str, len) )
        {
          loc1 = ptr;
          loc2 = ptr + len;
          l2rec = g_rec;
          return YES;
        }

      ++ptr;
    }

  return NO;
}

private
int
ncontains(const char *ptr, char csc str, const int len, char csc i_end)
{
  while ( i_end - ptr >= len && ecmp(ptr, str, len) )
    {
      ++ptr;
    }
  return YES;
}

private
int
nstep(const char *p1)
{
  byte csc p2 = expbuf;
  int rc = YES;

  if (circf)
    {
      rc = i_col != 0 || !advance(p1, p2);
    }
  else
    {
      while ( advance(p1++, p2) )
        {
          ;
        }
    }

  return rc;
}

private
int
string_search(char *const ign_buff, char csc str, const int len,
              const int comm, const word ignore, const word negate,
              const word type)
{
  char *b_ptr, *i_end;
  int r_len;
  const int regex = u_map[type] == 'R',
    stay = ( type == 'r' || type == 'G' || comm == 'V' );

  if (regex)
    {
      if (len)
        {
          compile(str, len);
        }

      elif( nullstr(expbuf) )
          g_err(NO_PREV_RE, str);
    }

  repeat
  {
    r_len = i_eor;
    if (ignore)
      {  /* convert this record to uppercase */
        char *p1 = i_buff, *p2 = ign_buff;
        int i;
        for (i = 0; i < r_len; ++i)
          {
            *p2++ = u_star(p1++);
          }

        b_ptr = ign_buff;
      }
    else
      {
        b_ptr = i_buff;
      }

    i_end = b_ptr + r_len;
    *i_end = EOS;

    /* set up start position */

    switch (type)
      {
      case 'r':
      case 'G':
      case 'H':
        b_ptr += i_col;
        break;

      /*
       *  TS// matches rec with blanks only
       *  T// matches truly empty rec
       */

      case 'S':
        while ( b_ptr < i_end && isspace(*b_ptr) )
          {
            ++b_ptr;
          }
        r_len = i_end - b_ptr;
        /* fallthrough */

      case 'B':
        if (len == 0)
          {
            if (negate + r_len == 0 || negate && r_len > 0)
              {
                return YES;
              }
          }

        break;

      case 'F':
        if (len < r_len)
          {
            b_ptr = i_end - len  /* search starts at end */;
          }
      }
    loc1 = b_ptr;

    /* too short for match ? */

    if (regex || i_end - loc1 >= len)
      {
        switch (type)
          {
          case 'R':
            if (step(b_ptr) != negate)
              {
                return YES;
              }

            break;

          case 'r':
            if (comm == 'V')
              {
                return advance(b_ptr, expbuf) != negate;
              }

            if ( !negate && step(b_ptr) || negate && nstep(b_ptr) )
              {
                altr_line( (int)( loc1 - b_ptr ), comm );
                return YES;
              }

            return NO;

          case 'F':
          case 'S':
          case 'B':
          case 'H':
            if (len && ecmp(b_ptr, str, len) != negate)
              {
                if (!negate)
                  {
                    loc2 = b_ptr + len;
                    l2rec = g_rec;
                  }

                return YES;
              }

            break;

          case 'G':
            if ( !negate && contains(b_ptr, str, len, i_end - len)
                || negate && ncontains(b_ptr, str, len, i_end) )
              {
                altr_line( (int)( loc1 - b_ptr ), comm );
                return YES;
              }

            return NO;

          case 'C':
            if (contains(b_ptr, str, len, i_end - len) != negate)
              {
                return YES;
              }
          }
      }

    elif(negate)
        return YES;

    /* come here when negative logic finds a match */

    if (stay)
      {
        return NO;
      }

    if ( !add_line(comm) )
      {
        return negate;
      }
  }  /* of repeat */
}

private
int
findstr(char csc str, int len, word type, const int comm)
{
  const word ignore = type & STR_IGNORE, negate = ( type & STR_NEGATE ) != 0;
  LINE buf;

  type &= BYTE_MASK;

  if (type == 'G' && comm == 'V')
    {
      type = 'H';
    }

  loc2 = NULL;

  if ( string_search(buf, str, len, comm, ignore, negate, type) )
    {
      if (ignore && loc2 != NULL)
        {
          len = loc2 - loc1;
          loc1 = i_buff + ( loc1 - buf );
          loc2 = loc1 + len;
        }

      return NO;
    }

  return YES;
}

/*
 *  Expand ~ into home directory if followed by '/'
 *  Expand $VAR
 *  Expand % into old-file name
 *  All may be escaped with '\' which is passed through
 */

private
const char *
prep_name(const char *fname)
{
  static FNAME name;
  const char *v;
  char *p = name, *t, c;

  while ( ( c = *fname++ ) != EOS )
    {
      switch (c)
        {
#if UNIX
        case '\\':
          *p++ = c;
          *p++ = *fname++;
          continue;

#endif  /* if UNIX */
        case '%':
          v = in_fname;
          break;

        case '~':
          if ( path_sep(*fname) )
            {
              if ( ( v = getenv("HOME") ) == NULL )
                {
                  v = empty;
                }

              break;
            }
          /* fallthrough */

        default:
          *p++ = c;
          continue;

        case '$':
          for (t = p; wordch(*fname); ++fname)
            {
              *t++ = *fname;
            }

          *t = EOS;
          if ( ( v = getenv(p) ) == NULL )
            {
              g_err(NO_ENV, p);
            }
        }
      p = mzmovelr(p, v);
    }

  *p = EOS;

  return name;
}

#if UNIX

/*
 *  Read a non-regular file (sadly, duplicated in DOS version of d2m).
 */

private
int
serial_read(UNIT *const vs_u, const int fd)
{
  int len = 0;
  char buf[BLOCK_SIZE + E_BUFF_SIZE], *p;

  while ( ( len = read(fd, p = buf + len, BLOCK_SIZE) ) > 0 )
    {
      const char *start = buf, *const last = p + len;
      while ( ( len = last - p ) > 0 )
        {
          if ( ( p = (char *)memchr(p, LFEED, len) ) == NULL )
            {
              if (len > E_BUFF_LEN)
                {
                  len = E_BUFF_LEN + 1;
                }

              (void)movelr(buf, start, len);
              break;
            }

          vsload( vs_u, start, (int)( p - start ) );
          start = ++p;
        }
    }
  if (p > buf)
    {
      vsload( vs_u, buf, (int)( p - buf ) );
    }

  (void)close(fd);

  return vstell(vs_u);
}

/*
 *  Read and write to/from a process
 */

#if !defined(OMIT_POPEN)
private
int
Mem_to_proc(UNIT *const vs_u, char csc comm)
{
  FILE *fp;
  const byte *p;
  int len, rc = -1;

  if ( ( fp = popen(prep_name(comm), write_only) ) != NULL )
    {
      while ( ( len = vsgetrec(vs_u, &p) ) != EOF )
        {
          if ( fwrite(p, 1, len, fp) != len || \
               fwrite(pt_list, 1, 1, fp) != 1 )
            {
              break;
            }
        }
      rc = pclose(fp);
    }

  return rc != 0 ? EOF : (int)vstell(vs_u);
}

private
int
Proc_to_mem(UNIT *const vs_u, char csc comm)
{
  FILE *fp;
  int rc = -1;

  if ( ( fp = popen(prep_name(comm), "r") ) != NULL )
    {
      trunc_recs = 0;

      (void)serial_read( vs_u, fileno(fp) );

      rc = pclose(fp);
    }

  return rc != 0 ? EOF : (int)vstell(vs_u);
}
#endif  /* if !defined(OMIT_POPEN) */

#endif  /* if UNIX */

private
int
Disk_to_mem(char csc fname, UNIT *const vs_u, const int mode)
{
  int fd;

#if DOS || defined(OMIT_MMAP)
  int len = 0;
  char buf[BLOCK_SIZE + E_BUFF_SIZE], *p;
#else  /* if DOS || defined(OMIT_MMAP) */

# if UNIX
  off_t f_len;
  caddr_t f_p;
# endif  /* if UNIX */

# if !defined(OMIT_POPEN)
  if (*fname == '!')
    {
      return Proc_to_mem(vs_u, fname + 1);
    }
# endif  /* if !defined(OMIT_POPEN) */

#endif  /* if DOS */

  if (mode == 'F')
    {
      fd = fd_in_terminal;
    }

  elif( ( fd = open(prep_name(fname), O_RDONLY) ) == -1 )
      return EOF;

  trunc_recs = 0;

#if DOS || defined(OMIT_MMAP)
# if !defined(__DJGPP__) && !defined(IA16_GCC_DOS)
  setmode(fd, O_BINARY);
# endif  /* if !defined(__DJGPP__) && !defined(IA16_GCC_DOS) */

  while ( ( len = read(fd, p = buf + len, BLOCK_SIZE) ) > 0 )
    {
      const char *start = buf, *const last = p + len;
      while ( ( len = last - p ) > 0 )
        {
          if ( ( p = (char *)memchr(p, LFEED, len) ) == NULL )
            {
              if (len > E_BUFF_LEN)
                {
                  len = E_BUFF_LEN + 1;
                }

              (void)cwmovelr(buf, start, len);
              break;
            }

          vsload( vs_u, start, (int)( p - start ) );
          start = ++p;
        }
    }
  if (p > buf)
    {
      vsload( vs_u, buf, (int)( p - buf ) );
    }

  (void)close(fd);

  return vstell(vs_u);

#else  /* if DOS || defined(OMIT_MMAP) */

# if UNIX || !defined(OMIT_MMAP)
  f_len = lseek(fd, (off_t)0, SEEK_END);
  if ( ( f_p = mmap(NULL, f_len, PROT_READ, MAP_PRIVATE, fd, (off_t)0) )
      != (caddr_t)-1)
    {
      const char *start = f_p, *p, *const last = start + f_len;
      int len;
      (void)close(fd);
      while ( ( len = last - start ) > 0 )
        {
          if ( ( p = memchr(start, LFEED, len) ) == NULL )
            {
              p = last;
            }

          vsload( vs_u, start, (int)( p - start ) );
          start = p + 1;
        }
      (void)munmap(f_p, f_len);
      return vstell(vs_u);
    }

  (void)lseek(fd, 0, SEEK_SET);
# endif  /* if UNIX || !defined(OMIT_MMAP) */

  return serial_read(vs_u, fd);

#endif  /* if DOS || defined(OMIT_MMAP) */
}

private
int
Mem_to_disk(UNIT *const vs_u, char csc fname, const int mode)
{
  const byte *rec;
  int len, fd, fmode;
  byte buf[BLOCK_SIZE + E_BUFF_SIZE], *p = buf, *const last = p + BLOCK_SIZE;

#if UNIX && !defined(OMIT_POPEN)
  if (*fname == '!')
    {
      return Mem_to_proc(vs_u, fname + 1);
    }

#endif  /* if UNIX && !defined(OMIT_POPEN) */
  if (mode == 'F')
    {
      fd = fd_out_terminal;
    }
  else
    {
      if ( ro_mode && equal(fname, in_fname) )
        {
          errno = EACCES;
          return EOF;
        }

      if (mode == 'N')
        {
          fmode = O_WRONLY | O_CREAT | O_TRUNC;
        }
      else
        {
          fmode = O_WRONLY | O_CREAT | O_APPEND;
        }

      if ( ( fd = open(prep_name(fname), fmode, 0644) ) == -1)
        {
          return EOF;
        }
    }

#if DOS && !defined(IA16_GCC_DOS)
  setmode(fd, O_BINARY);
#endif  /* if DOS && !defined(IA16_GCC_DOS) */

  if ( isatty(fd) )
    {
      term();
    }

  while ( ( len = vsgetrec(vs_u, &rec) ) != EOF )
    {
      p = cwmmovelr(p, rec, len);
#if DOS
      *p++ = '\r';
#endif  /* if DOS */
      *p++ = '\n';
      if (p > last)
        {
          if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
            {
              return EOF;
            }

          len = p - last;
          p = cwmmovelr(buf, last, len);
        }
    }
  if ( ( len = p - buf ) > 0 )
    {
      if (write(fd, buf, len) != len)
        {
          return EOF;
        }
    }

  (void)close(fd);

  return vstell(vs_u);
}

/*
 *  Variable length record stack.
 *  Objects on the stack have a structure :-
 *       stack next    --  address of next stacked line (int *)
 *       word  length  --  number of bytes in text field
 *       char  text[]  --  text, length bytes long
 *  These are stored in a single chunk to save malloc red-tape.
 */

private
void
push_line(stack **const line_stack, char csc text, const int len)
{
  stack *const p = (stack *)getvec(sizeof ( stack ) + sizeof ( short ) + len);
  short *const start = (short *)( p + 1 );

  *start = (short)len;
  (void)movelr(start + 1, text, len);
  *p = (stack)( *line_stack );
  *line_stack = p;

  if (p == in_stack)
    {
      ++in_count;
    }
}

private
void
pop_line(stack **const line_stack, char *const line)
{
  stack *p, *const top = *line_stack;
  short *start;

  if (top == NULL)
    {
      return;
    }

  p = (stack *)( *top );
  start = (short *)( top + 1 );
  (void)movelr(line, start + 1, *start);
  rlsevec(top);
  *line_stack = p;

  if (p == in_stack)
    {
      --in_count;
    }
}

/*
 *  Write the screen buffer to the output file.
 */

private
void
buf_to_file(const int com)
{
  int i;
  const int last = ( com == NEXT_LINE ) ? FIRST_LINE : last_line;

  for (i = FIRST_LINE; i <= last && eor[i] != EOF; ++i)
    {
      vsputrec(out_u, s_buf[i], eor[i]);  /* out_buf */
    }

  o_rec += i - FIRST_LINE;

  /* in case buf_to_file gets called again (from SIGUSR2) */

#if UNIX
  eor[FIRST_LINE] = EOF;
#endif  /* if UNIX */

  switch (com)
    {
    case MOVE_TOF:
    case MOVE_EOF:
    case PREV_PAGE:
    case PREV_LINE:
    case MOVE_ABS:
    case SE_LEAVE:
      while (in_stack != NULL)
        {
          e_col = pop_length(in_stack);
          pop_line(&in_stack, e_buff);
          out_buff();
        }
      (void)fill_buff();
    }
}

/*
 *  Swap roles of input and output files (as in T#0 etc).
 */

private
int
wrapround(void)
{
  UNIT *temp;
  const int last_o_rec = o_rec + 1;

  if (f_list != NULL)
    {
      g_err(f_list->disp == 'M' ? W_IN_MERGE : SAV_ON_STACK, NULL);
    }

  if (!g_eof)
    {
      alter_end(EOF, 'T');
    }

  if (e_col)
    {
      out_buff()  /* in case user added line to eof */;
    }

  vsreopen(temp = out_u);
  vsreopen(out_u = in_u);
  in_u = temp;

  init_line();

#ifdef DEBUG
  ckunit(in_u);
  ckunit(out_u);
#endif  /* ifdef DEBUG */

  return last_o_rec;
}

#if UNIX

/*
 *  These functions attempt to save the data "somewhere"
 */

private
void
save_work(char *const mess)
{
  int recs, i;
  FNAME filename;

  if (fscreen)
    {
      buf_to_file(SE_LEAVE);
    }

  lon = 0;
  (void)wrapround();

  if (vssizeof(in_u) == 0)
    {
      (void)cmovelr(mess, "No data to save.\n");
      return;
    }

  for (i = 0; save_dirs[i] != NULL; ++i)
    {
      (void)zmovelr(mzmovelr(filename, save_dirs[i]), "/g.hup");
      if ( ( recs = Mem_to_disk(in_u, filename, 'N' ) ) != EOF )
        {
          (void)sprintf(
            get_eos(mess),
            "%d line%s saved to \"%s\"\n",
            recs,
            plural(recs),
            filename);
          return;
        }

      vsrewind(in_u);
    }

  (void)cmovelr(mess, "Unable to save data.\n");
}

#endif  /* if UNIX */

/* Break key handler */

private
void
g_intr(int sig)
{
#if UNIX
  const char *reason;
  FILE *fp;
  string mess;
#endif  /* if UNIX */

#if !defined(OMIT_SIGNAL)
  (void)signal(sig, g_intr);
#endif  /* if !defined(OMIT_SIGNAL) */

#ifndef __MINGW32__
# if DOS || defined(OMIT_SIGNAL)
  g_err(BREAK_KEY, empty);
# else  /* if DOS || defined(OMIT_SIGNAL) */
  switch (sig)
    {
    case SIGQUIT:
    case SIGINT:
      g_err(BREAK_KEY, empty);
      /* fallthrough */

    case SIGUSR2:
      if (fscreen)
        {
          buf_to_file(SE_LEAVE);
        }

      Exit();
      abort();
      /* fallthrough */

    case SIGUSR1:
      Quit();
      abort();
      /* fallthrough */

    case SIGTERM:
      reason = "user termination";
      break;

    case SIGPIPE:
      errno = EPIPE;
      g_err(FILE_ERROR, NULL);
      /* fallthrough */

    case SIGHUP:
      reason = "comms failure";
    }
  (void)sprintf(mess, "g ( %s ==> %s ) %s.\n\n", in_fname, out_fname, reason);
  save_work(mess);
#if !defined(OMIT_POPEN)
  if ( ( fp = popen("exec mail $LOGNAME", write_only) ) != NULL )
    {
      (void)fprintf(fp, "\n%s", mess);
      (void)pclose(fp);
    }
#endif  /* if !defined(OMIT_POPEN) */

  term();
  _exit(1);
# endif  /* if DOS */
#endif  /* ifndef __MINGW32__ */
}

/*
 *  Arithmetic.
 */

/* portable multiple character constants */

#define COMB(a, b) ( ( a << 8 ) | b )

#define SHL    COMB('<', '<')
#define SHR    COMB('>', '>')
#define GEQ    COMB('=', '>')
#define LEQ    COMB('=', '<')
#define AND    COMB('&', '&')
#define OR     COMB('|', '|')
#define EQ     COMB('=', 'E')
#define NEQ    COMB('=', '!')
#define PLUSAB COMB('+', '=')
#define MINAB  COMB('-', '=')
#define MULAB  COMB('*', '=')
#define DIVAB  COMB('/', '=')
#define MODAB  COMB('%', '=')
#define ORAB   COMB('|', '=')
#define ANDAB  COMB('&', '=')
#define XORAB  COMB('^', '=')
#define SHLAB  COMB('<', '=')
#define SHRAB  COMB('>', '=')
#define PPLUS  COMB('+', '+')
#define MMINUS COMB('-', '-')

#define UNOP1  \
       '!':    \
case   '~':    \
case PPLUS:    \
case MMINUS

#define UNOP2  \
     '-':      \
case '+'

#define BINOP  \
        '/':   \
case    '%':   \
case    '|':   \
case    '^':   \
case    '=':   \
case    '<':   \
case    '>':   \
case    SHL:   \
case    SHR:   \
case    GEQ:   \
case    LEQ:   \
case    AND:   \
case     OR:   \
case     EQ:   \
case    NEQ:   \
case PLUSAB:   \
case  MINAB:   \
case  MULAB:   \
case  DIVAB:   \
case  MODAB:   \
case   ORAB:   \
case  ANDAB:   \
case  XORAB:   \
case  SHLAB:   \
case  SHRAB

/* compound variable names */

#define DOT_AT   COMB('.', '@')
#define DOT_HASH COMB('.', '#')
#define DOT_DOL  COMB('.', '$')
#define DOT_AND  COMB('.', '&')
#define DOT_DOT  COMB('.', '.')

#define LAST_RES    26   /* Last result (.)           */
#define M_TEMP      27   /* Temporary for macros (..) */

#define LITERAL   0177   /* Un-named constant */

#define SHIFT_MASK 077   /* Largest shift-by value */

#define MAXROM  39999L   /* Maximum size of roman literal */
#define NROM         5

/* Free list to save malloc calls/fragmentation */

private
TOKEN *c_free_list = NULL;

/* Possible values for the group field */

#define OPERAND   0  /* literal or variable */
#define OPEN_PAR  1
#define CLOSE_PAR 2
#define PREFIX    3
#define POSTFIX   4
#define DYADIC    5
#define COMMA     6
#define END_EXP   7  /* EOS or Close Curly  */
#define QUERY     8
#define COLON     9

#define last_result ( &locals[LAST_RES] )

private
TOKEN locals[M_TEMP + 1];

private
char last_was_op;

/* Manage operator/operand stacks */

#define push(t, s)  \
  t->snext = s;     \
  s = t

#define pop(s) s = s->snext;

private
word
priority(const word op)
{
  if ( ( op >> 8 ) == 0 )
    {
      return xlat(op, "*/%+-<>&^|?:=,", opprio);
    }

  switch (op)
    {
    case GEQ:
    case LEQ:
      return 10;

    case EQ:
    case NEQ:
      return 8;

    case AND:
      return 4;

    case OR:
      return 3;

    case SHL:
    case SHR:
      return 11;
    }
  return 1;  /* = etc */
}

private
void
free_expr(TOKEN *const ptr)
{
  TOKEN *p = ptr;

  while (p->next != NULL)
    {
      p = p->next;
    }

  p->next = c_free_list;
  c_free_list = ptr;
}

/*
 *  Execute the operator on top of the pending operators stack
 *  on the top two operands of the data stack.
 */

private
void
execute(TOKEN *const rhs, TOKEN csc action)
{
  TOKEN *dest, *lhs;
  char fp;
  const word op = action->id;

  if (rhs == NULL || rhs->snext == NULL)
    {
      g_err(SYN_EXPR, action->errp);
    }

  lhs = rhs->snext;

  if (op == '=')
    {
      fp = lhs->fp = rhs->fp;
    }
  else
    {  /* promote operands */
      fp = lhs->fp | rhs->fp;
      if (fp != rhs->fp)
        {
          rhs->fp = YES;
          rhs->opval.r = (real)rhs->opval.i;
        }

      if (fp != lhs->fp)
        {
          lhs->fp = YES;
          lhs->opval.r = (real)lhs->opval.i;
        }
    }

  if ( ( op & BYTE_MASK ) == '=' )
    {
      if (lhs->id == LITERAL)
        {
          g_err(SYN_EXPR, lhs->errp);
        }

      dest = &locals[lhs->id];
      dest->fp = fp;
    }

  if (fp)
    {
      const real rrval = rhs->opval.r, lrval = lhs->opval.r;
      switch (op)
        {
        default:
          g_err(TYPE_ERR, lhs->errp);
          /* fallthrough */

        case '=':
          lhs->opval.r = dest->opval.r = rrval;
          return;

        case '+':
          lhs->opval.r += rrval;
          return;

        case '-':
          lhs->opval.r -= rrval;
          return;

        case '*':
          lhs->opval.r *= rrval;
          return;

        case '/':
          lhs->opval.r /= rrval;
          return;

        case PLUSAB:
          dest->opval.r = lhs->opval.r += rrval;
          return;

        case MINAB:
          dest->opval.r = lhs->opval.r -= rrval;
          return;

        case MULAB:
          dest->opval.r = lhs->opval.r *= rrval;
          return;

        case DIVAB:
          dest->opval.r = lhs->opval.r /= rrval;
          return;

        case '>':
          lhs->opval.i = lrval > rrval;
          break;

        case '<':
          lhs->opval.i = lrval < rrval;
          break;

        case GEQ:
          lhs->opval.i = lrval >= rrval;
          break;

        case LEQ:
          lhs->opval.i = lrval <= rrval;
          break;

        case AND:
          lhs->opval.i = lrval && rrval;
          break;

        case OR:
          lhs->opval.i = lrval || rrval;
          break;

        case EQ:
          lhs->opval.i = lrval == rrval;
          break;

        case NEQ:
          lhs->opval.i = lrval != rrval;
          break;
        }
      lhs->fp = NO;
    }
  else
    {
      const long rival = rhs->opval.i, lival = lhs->opval.i;
      switch (op)
        {
        case '=':
          lhs->opval.i = dest->opval.i = rival;
          break;

        case '+':
          lhs->opval.i += rival;
          break;

        case '-':
          lhs->opval.i -= rival;
          break;

        case '*':
          lhs->opval.i *= rival;
          break;

        case '/':
          if (rival == 0)
            {
              g_err(DIV_CHECK, lhs->errp);
            }

          lhs->opval.i /= rival;
          break;

        case '%':
          if (rival == 0)
            {
              g_err(DIV_CHECK, lhs->errp);
            }

          lhs->opval.i %= rival;
          break;

        case '&':
          lhs->opval.i &= rival;
          break;

        case '|':
          lhs->opval.i |= rival;
          break;

        case '^':
          lhs->opval.i ^= rival;
          break;

        case SHL:
          lhs->opval.i <<= (int)( rival & SHIFT_MASK );
          break;

        case SHR:
          lhs->opval.i >>= (int)( rival & SHIFT_MASK );
          break;

        case '>':
          lhs->opval.i = lival > rival;
          break;

        case '<':
          lhs->opval.i = lival < rival;
          break;

        case GEQ:
          lhs->opval.i = lival >= rival;
          break;

        case LEQ:
          lhs->opval.i = lival <= rival;
          break;

        case AND:
          lhs->opval.i = lival && rival;
          break;

        case OR:
          lhs->opval.i = lival || rival;
          break;

        case EQ:
          lhs->opval.i = lival == rival;
          break;

        case NEQ:
          lhs->opval.i = lival != rival;
          break;

        case PLUSAB:
          dest->opval.i = lhs->opval.i += rival;
          break;

        case MINAB:
          dest->opval.i = lhs->opval.i -= rival;
          break;

        case MULAB:
          dest->opval.i = lhs->opval.i *= rival;
          break;

        case DIVAB:
          if (rival == 0)
            {
              g_err(DIV_CHECK, lhs->errp);
            }

          dest->opval.i = lhs->opval.i /= rival;
          break;

        case MODAB:
          if (rival == 0)
            {
              g_err(DIV_CHECK, lhs->errp);
            }

          dest->opval.i = lhs->opval.i %= rival;
          break;

        case ORAB:
          dest->opval.i = lhs->opval.i |= rival;
          break;

        case ANDAB:
          dest->opval.i = lhs->opval.i &= rival;
          break;

        case XORAB:
          dest->opval.i = lhs->opval.i ^= rival;
          break;

        case SHLAB:
          dest->opval.i = lhs->opval.i <<= (int)( rival & SHIFT_MASK );
          break;

        case SHRAB:
          dest->opval.i = lhs->opval.i >>= (int)( rival & SHIFT_MASK );
        }
    }
}

private
void
mon_exec(TOKEN *const dp, const int op, const int prefix)
{
  TOKEN *dest;
  int fp = dp->fp;
  real rval = dp->opval.r;
  const long ival = dp->opval.i;

  switch (op)
    {
    case '-':
      if (fp)
        {
          dp->opval.r = -rval;
        }
      else
        {
          dp->opval.i = -ival;
        }

      break;

    case '~':
      if (fp)
        {
          g_err(TYPE_ERR, dp->errp);
        }

      dp->opval.i = ~ival;
      break;

    case '!':
      if (fp)
        {
          dp->opval.i = !rval;
          dp->fp = NO;
        }
      else
        {
          dp->opval.i = !ival;
        }

      break;

    case PPLUS:
      if (dp->id == LITERAL)
        {
          g_err(SYN_EXPR, dp->errp);
        }

      dest = &locals[dp->id];
      if (fp)
        {
          dp->opval.r = prefix ? ++dest->opval.r : dest->opval.r++;
        }
      else
        {
          dp->opval.i = prefix ? ++dest->opval.i : dest->opval.i++;
        }

      break;

    case MMINUS:
      if (dp->id == LITERAL)
        {
          g_err(SYN_EXPR, dp->errp);
        }

      dest = &locals[dp->id];
      if (fp)
        {
          dp->opval.r = prefix ? --dest->opval.r : dest->opval.r--;
        }
      else
        {
          dp->opval.i = prefix ? --dest->opval.i : dest->opval.i--;
        }
    }
}

/*
 *  Convert a Roman numeral string to an integer.
 */

private
long
rtoi(char cssc ptr)
{
  long n = 0, last = 0, val;
  int pow10 = 0, dec, contig;
  const char *p, *const s = *ptr;

  for (p = s; memchr(c_rom, *p, 18) != NULL; ++p)
    {
      ;
    }

  *ptr = p;

  while (--p >= s)
    {
      switch ( u_star(p) )
        {
        default:
          g_err(BAD_NUM, s);
          /* fallthrough */

        case 'I':
          val = pow10 = 1;
          break;

        case 'V':
          val = 5;
          break;

        case 'X':
          val = 10, ++pow10;
          break;

        case 'L':
          val = 50;
          break;

        case 'C':
          val = 100, ++pow10;
          break;

        case 'D':
          val = 500;
          break;

        case 'M':
          val = 1000, ++pow10;
          break;

        case 'W':
          val = 5000;
          break;

        case 'Z':
          val = 10000, ++pow10;
        }
      if (val > last)
        {
          n += val;
          dec = 0;
          contig = 1;
          last = val;
        }

      elif(val == last)
        {
          if (!pow10 || ++contig > 3)
            {
              g_err(BAD_NUM, s);
            }

          n += val;
        }
      else
        {
          if (!pow10 || contig != 1 || dec)
            {
              g_err(BAD_NUM, s);
            }

          dec = 1;
          contig = 0;
          n -= val;
        }
    }

  return n;
}

/*
 *  Get an operator string, if its a dop, combine it into an int
 */

private
word
get_op(char cssc ptr)
{
  word c1, c2;

  if ( ( c1 = *( *ptr )++ ) != EOS )
    {
      c2 = *( *ptr )++;
    }

  switch (c1)
    {
    case '}':
      c1 = EOS;
      break;

    case '>':
    case '<':
      if (c2 == '=')
        {
          return COMB(c2, c1);
        }

      if (c2 == c1)
        {
          if ( ( c1 = **ptr ) == '=' )
            {
              ++( *ptr );
              return COMB(c2, c1);
            }

          return COMB(c2, c2);
        }

      break;

    case '!':
    case '=':
      if (c2 == '=')
        {
          if (c1 == '=')
            {
              c1 = 'E';
            }

          return COMB(c2, c1);
        }

      break;

    case '&':
      if (c2 == '&' || c2 == '=')
        {
          return COMB(c1, c2);
        }

      break;

    case '|':
      if (c2 == '|' || c2 == '=')
        {
          return COMB(c1, c2);
        }

      break;

    case '+':
      if (c2 == '+' || c2 == '=')
        {
          return COMB(c1, c2);
        }

      break;

    case '-':
      if (c2 == '-' || c2 == '=')
        {
          return COMB(c1, c2);
        }

      break;

    case '*':
    case '/':
    case '%':
    case '^':
      if (c2 == '=')
        {
          return COMB(c1, c2);
        }

      break;

    case '.':
      switch (c2)
        {
        case '@':
        case '#':
        case '$':
        case '&':
        case '.':
          return COMB(c1, c2);
        }
    }

  --( *ptr );
  return c1;
}

/* get_tok */

private
TOKEN *
lex(char cssc ptr)
{
  const char *p, *e = *ptr;
  int ch;
  static TOKEN tok;
  char base;

  skip_space(e);
  tok.errp = e;
  if ( ( ch = get_op(&e) ) == EOS )
    {
      *ptr = e;
      return NULL;
    }

  tok.fp = tok.l_fp = NO;
  tok.group = OPERAND;
  tok.id = ch;
  p = e;

  switch (ch)
    {
    case '(':
      tok.group = OPEN_PAR;
      ++last_was_op;
      break;

    case ')':
      tok.group = CLOSE_PAR;
      last_was_op = NO;
      break;

    case '.':
      if ( !( isdigit(*p) ) )
        {
          tok.id = LAST_RES;
          break;
        }
      /* fallthrough */

    case NUMERIC:
      tok.id = LITERAL;
      if (ch == '0')
        {
          base = u_star(p++);
          if (base == 'R')
            {
              e = p;
              tok.litval.i = rtoi(&e);
              goto have_num;
            }

          if (base == 'B')
            {
              tok.litval.i = strtol(p, (char **)&e, 2);
              goto have_num;
            }

          --p;
        }

      tok.litval.i = strtol(--p, (char **)&e, 0);
      ch = u_star(e);
      if (ch == 'E' || ch == '.')
        {
#if TINY_G && !(UNIX)
          if ( ( bios_word(0x410) & 0x02 ) == 0 )  /* no 87 fitted */
            {
              g_err(BAD_NUM, p);
            }

#endif  /* if TINY_G && !(UNIX) */
          tok.litval.r = strtod(p, (char **)&e);
          tok.l_fp = YES;
        }

have_num:
      if (e == p)
        {
          g_err(BAD_NUM, p);
        }

      p = e;
      break;

    case SQUOTE:
      tok.id = LITERAL;
      if ( ( ch = *p++ ) == BSLASH )
        {
          ch = xlat(u_star(p++), esc_symb, esc_char);
        }

      if (*p++ != SQUOTE)
        {
          g_err(SYN_EXPR, p);
        }

      tok.litval.i = ch;
      break;

    case UNOP2:
      if (!last_was_op)
        {
          goto binops;
        }
      /* fallthrough */

    case UNOP1:
      tok.group = last_was_op ? PREFIX : POSTFIX;
      break;

    case '*':
      /* fallthrough */
    case '&':
      if (last_was_op)
        {
          break;
        }
      /* fallthrough */

    case BINOP:
binops:
      tok.group = DYADIC;
      ++last_was_op;
      tok.opval.i = priority(ch);
      break;

    case ',':
    case ';':
      tok.group = COMMA;
      ++last_was_op;
      break;

    case '?':
      tok.group = QUERY;
      ++last_was_op;
      break;

    case ':':
      tok.group = COLON;
      ++last_was_op;
      break;

    case '#':  /* hash/number/sharp sign */
    case '@':
    case '$':
    case DOT_HASH:
    case DOT_AT:
    case DOT_DOL:
    case DOT_AND:
      break;

    case DOT_DOT:
      tok.id = M_TEMP;
      break;

    default:
      ch = u_map[ch];
      if ( !( isupper(ch) ) )
        {
          g_err(SYN_EXPR, tok.errp);
        }

      tok.id = ch - 'A';
    }  /* big switch */

  if (tok.group == OPERAND)
    {
      last_was_op = NO;
    }

  *ptr = p;
  return &tok;
}

private
int
Expr_compile(TOKEN **start, char cssc ptr)
{
  TOKEN *tok, *temp;

  *start = NULL;
  ++last_was_op;  /* ie. it's ok to have a unop here */

  while ( ( tok = lex(ptr) ) != NULL )
    {
      if (c_free_list != NULL)
        {
          temp = c_free_list;
          c_free_list = temp->next;
        }
      else
        {
          temp = heap(TOKEN);
        }

      *temp = *tok;
      temp->next = NULL;
      *start = temp;
      start = &temp->next;
    }

  return ( *ptr )[-1] != '}';
}

/* retrieve value at run-time */

private
void
get_value(TOKEN *const tok)
{
  const TOKEN *tp;
  long ival;

  switch (tok->id)
    {
    default:
      tp = &locals[tok->id];
      tok->fp = tp->fp;
      tok->opval = tp->opval;
      return;

    case LITERAL:
      tok->fp = tok->l_fp;
      tok->opval = tok->litval;
      return;

    case '*':
      if (g_eof || i_col >= i_eor)
        {
          ival = 0;
        }
      else
        {
          ival = (long)( i_buff[i_col] & BYTE_MASK );
        }

      break;

    case '&':
      ival = o_rec + 1;
      break;

    case '#':  /* hash/number/sharp sign */
      ival = g_rec;
      break;

    case '@':
      ival = s_g_rec > 0 ? s_g_rec : 0;
      break;

    case '$':
      ival = vssizeof(in_u);
      break;

    case DOT_HASH:
      ival = i_col;
      break;

    case DOT_AT:
      ival = s_g_col > 0 ? s_g_col : 0;
      break;

    case DOT_DOL:
      ival = i_eor;
      break;

    case DOT_AND:
      ival = e_col;
    }

  tok->opval.i = ival;
}

/*
 *  Clear the operator stack by execution
 */

private
TOKEN *
clear_stack(TOKEN *ds, TOKEN *os)
{
  while (os != NULL)
    {
      execute(ds, os);
      pop(ds);
      pop(os);
    }
  if (ds == NULL)
    {
      g_err(SYN_EXPR, NULL);
    }

  return ds;
}

/*
 *  Balance parenthesis etc
 */

private
TOKEN *
match(TOKEN *tok, const int endp)
{
  for (tok = tok->next; tok != NULL && tok->group != endp; tok = tok->next)
    {
      switch (tok->group)
        {
        case OPEN_PAR:
          tok = match(tok, CLOSE_PAR);
          continue;

        case QUERY:
          tok = match(tok, COLON);
          continue;

        case COLON:
        case CLOSE_PAR:
          if (endp != END_EXP)
            {
              g_err(SYN_EXPR, tok->errp);
            }

          return tok;
        }
    }

  return tok;
}

private
TOKEN *
parse(TOKEN *const retval, TOKEN *tok, const int endp)
{
  TOKEN *temp, *dstack = NULL, *ostack = NULL;
  int query_value, prefix = NO;

  for (; tok != NULL; tok = tok->next)
    {
      switch (tok->group)
        {
        case OPEN_PAR:
          tok = parse(temp = tok, tok->next, CLOSE_PAR);
          push(temp, dstack);
          if (prefix)
            {
              mon_exec(dstack, prefix, YES);
              prefix = NO;
            }

          if (tok == NULL)
            {
              break;
            }

          continue;

        case OPERAND:
          push(tok, dstack);
          get_value(dstack);
          if (prefix)
            {
              mon_exec(dstack, prefix, YES);
              prefix = NO;
            }

          continue;

        case PREFIX:
          prefix = tok->id;
          continue;

        case POSTFIX:
          mon_exec(dstack, tok->id, NO);
          continue;

        case DYADIC:
          /* left to right association */
          while (ostack != NULL && tok->opval.i <= ostack->opval.i)
            {
              execute(dstack, ostack);
              pop(dstack);
              pop(ostack);
            }
          push(tok, ostack);
          continue;

        case COMMA:
          (void)clear_stack(dstack, ostack);
          dstack = ostack = NULL;
          continue;

        case QUERY:
          dstack = clear_stack(dstack, ostack);
          if (dstack->fp)
            {
              query_value = dstack->opval.r != 0.0;
            }
          else
            {
              query_value = dstack->opval.i != 0;
            }

          dstack = ostack = NULL;
          if (query_value)
            {
              tok = parse(temp = tok, tok->next, COLON);
              push(temp, dstack);
              tok = match(tok, END_EXP);
            }
          else
            {
              tok = match(tok, COLON);
            }

          if (tok == NULL)
            {
              break;
            }

          continue;

        case COLON:
        case CLOSE_PAR:
          if (endp != tok->group)
            {
              g_err(SYN_EXPR, tok->errp);
            }
        }  /* switch */
      break;
    }  /* loop */

  dstack = clear_stack(dstack, ostack);

  retval->fp = dstack->fp;
  retval->opval = dstack->opval;

  return tok;
}

/*
 *  Convert an integer to Roman numerals.
 */

private
char *
itor(long n)
{
  static char rnum[12];
  long k, rem[NROM], prev[NROM];
  char csc ones = c_rom, *const fives = c_rom + 5;
  char *cp;
  int i;

  for (i = 0; i < NROM; ++i)
    {
      rem[i] = n % 10;
      n /= 10;
      prev[i] = -1;
    }

  for (i = 0; i < NROM; ++i)
    {
      if (rem[i] == 9)
        {
          if (prev[i] == -1)
            {
              prev[i + 1] = i;
            }
          else
            {
              prev[i + 1] = prev[i];
              prev[i] = -1;
            }

          rem[i] = 0;
        }
    }

  cp = rnum;
  for (i = NROM - 1; i >= 0; --i)
    {
      if (rem[i] == 4)
        {
          if (prev[i] == -1)
            {
              *cp++ = ones[i];
            }
          else
            {
              *cp++ = ones[prev[i]];
            }

          *cp++ = fives[i];
        }
      else
        {
          k = rem[i] % 5;
          if (rem[i] >= 5)
            {
              *cp++ = fives[i];
            }

          while (k--)
            {
              *cp++ = ones[i];
            }
          if (prev[i] != -1)
            {
              *cp++ = ones[prev[i]];
              *cp++ = ones[i];
            }
        }
    }

  *cp = EOS;
  return rnum;
}

private
const char *
itoc(const long n)
{
  static char str[] = "\'c\'";

  if ( iscntrl(n) )
    {
      return asc_tab[n == DEL ? 32 : n];
    }

  str[1] = (char)n;
  return (const char *)str;
}

private
char *
itob(unsigned long n)
{
  static char bstr[33];
  char *p = &bstr[31];

  do
    {
      *p-- = (char)( ( n & ODD_MASK ) + '0' );
      n >>= 1;
    }
  while ( n != 0 );
  if ( ( &bstr[32] - p ) & ODD_MASK )
    {
      return p + 1;
    }

  *p = '0';
  return p;
}

private
char *
itoh(const long n)
{
  static char hbuf[9] = "0";

  return sprintf(hbuf + 1, "%lX", n) & ODD_MASK ? hbuf : hbuf + 1;
}

private
void
print_val(void)
{
  char oline[200], rstr[50], astr[20], *p;
  int len;

  putstr("==>  ");
  if (last_result->fp)
    {
      (void)sprintf(oline, m_real, last_result->opval.r);
    }
  else
    {
      const long val = last_result->opval.i;
      p = mzmovelr(rstr, ",  Roman: ");
      if (val == 0)
        {
          (void)zmovelr(p, "nihil");
        }

      elif(val > 0 && val <= MAXROM)
          (void)zmovelr( p, itor(val) );

      elif(val < 0 && val >= -MAXROM)
          (void)zmovelr( mcmovelr(p, "negativus "), itor(-val) );
      else
        {
          (void)zmovelr(p, "numerus negatus");
        }

      if ( isascii(val) )
        {
          (void)zmovelr( mzmovelr(astr, ",  Ascii: "), itoc(val) );
        }
      else
        {
          astr[0] = EOS;
        }

      len = sprintf(
        oline,
        "Bin: %s,  Oct: %lo,  Dec: %ld,  Hex: %s%s%s",
        itob( (unsigned long)val ),
        val,
        val,
        itoh(val),
        astr,
        rstr);
      if (len > 74)
        {
          len = sprintf(
            oline,
            "Oct: %lo,  Dec: %ld,  Hex: %s%s%s",
            val,
            val,
            itoh(val),
            astr,
            rstr);
        }

      if (len > 74)
        {
          (void)sprintf(
                  oline, "Oct: %lo,  Dec: %ld,  Hex: %s",
                  val,
                  val,
                  itoh(val) );
        }
    }

  say(oline);
}

private
void
Calc(TOKEN *expr)
{
  string cline;
  const char *p;
  TOKEN *ptr = expr;
  jmp_buf c_save_err;

  save_jbuf(c_save_err, set_err);

  if (lon)
    {
      term();
    }

  do
    {
      if (setjmp(set_err) == 0)
        {
          if (ptr != NULL)
            {
              (void)parse(last_result, ptr, END_EXP);
              if (lon)
                {
                  print_val();
                }
            }
        }

      if (ptr != NULL && ptr != expr)
        {
          free_expr(ptr);
        }

      if ( get_com(cline, "{ ") || nullstr(cline) )
        {
          break;
        }

      p = cline;
    }
  while ( Expr_compile(&ptr, &p) );

  save_jbuf(set_err, c_save_err);
}

private
TOKEN *
Evaluate(TOKEN csc expr, const int e_type)
{
  (void)parse(last_result, (TOKEN *)expr, END_EXP);
  if (e_type == C_ENDP && last_result->fp)
    {
      last_result->opval.i = (long)last_result->opval.r;
    }

  return last_result;
}

/*
 *  Print details of files read or written.
 */

private
void
print_size(const int recs)
{
  if (recs <= 0)
    {
      say(" - empty.");
    }
  else
    {
      (void)fprintf( vdu, " - %d line%s.\n", recs, plural(recs) );
    }
}

private
void
print_i_size(char csc ftype, char csc file, UNIT csc fp)
{
  (void)fprintf(vdu, ps_name, ftype, equal1(file, '-') ? si_file : file);
  print_size( vssizeof(fp) );
}

private
void
print_o_size(char csc ftype, char csc file, UNIT csc fp)
{
  (void)fprintf(vdu, ps_name, ftype, equal1(file, '-') ? so_file : file);
  print_size( vstell(fp) );
}

/*
 *  Drop down one level of the command file stack.
 */

private
void
c_comm_u(void)
{
  FILE_LIST *const cptr = c_list->next;
  UNIT *const old_c = comm_u;

  comm_u       = c_list->old_u;
  c_list->next = f_free_list;
  f_free_list  = c_list;
  c_list       = cptr;
  if (f_free_list->trans)
    {
      rlsevec(old_c);
      --trans_open_count;
    }
  else
    {
      vsclose(old_c);
    }
}

/*
 *  Drop file from stack.
 */

private
void
pop_flist(UNIT **const vs_u, const int fflag)
{
  FILE_LIST *const t = f_list->next;
  UNIT *const old_u  = *vs_u;

  *vs_u        = f_list->old_u;
  f_list->next = f_free_list;
  f_free_list  = f_list;
  f_list = t;
  if (fflag)
    {
      if (f_free_list->trans && old_u->read)
        {
          rlsevec(old_u);
        }
      else
        {
          vsclose(old_u);
        }
    }
}

/*
 *  Allocate new file list node
 */

private
FILE_LIST *
get_flist(void)
{
  FILE_LIST *p;

  if (f_free_list != NULL)
    {
      p = f_free_list;
      f_free_list = p->next;
    }
  else
    {
      p = heap(FILE_LIST);
      p->save.in_rec_len = p->save.out_rec_len = 0;
    }

  return p;
}

/*
 *  Open an alternative input file.
 */

private
void
new_input_file(VERB csc opts, FILE_LIST **const f_stack, UNIT **const vs_u)
{
  FILE_LIST *list, *fptr;
  UNIT *temp_u;
  const char *fn;
  const int temp_file = ( opts->o1.v == 0 );

  if (temp_file)
    {
      if (trans_u == NULL)
        {
          g_err(NO_TEMP_FILE, opts->errp);
        }

      if (vssizeof(trans_u) == 0)
        {
          g_err(EMPTY_USE_MERGE, opts->errp);
        }

      ++trans_open_count;
      temp_u = heap(UNIT);
      *temp_u = *trans_u;
      fn = t_fname;
    }
  else
    {
      int recs;
      temp_u = vsopen();
      if ( ( recs = Disk_to_mem(fn = opts->o1.s, temp_u, 'N') ) <= 0 )
        {
          vsclose(temp_u);
          g_err(recs == 0 ? EMPTY_USE_MERGE : FILE_ERROR, opts->errp);
        }

      vsreopen(temp_u);
    }

  fptr = *f_stack;
  *f_stack = list = get_flist();
  if (fptr != NULL)
    {
      fptr->prev = list;
    }

  list->next  = fptr;
  (void)zmovelr(list->name, fn);
  list->trans = (byte)temp_file;
  list->old_u = *vs_u;  /* Save old unit number */
  *vs_u = temp_u;
}

/*
 *  eXit verb.  Close current Merge, Save or Temp file and pop
 *  the next one from the stack.
 */

private
void
Xit(VERB csc opts)
{
  int mode = opts->o1.q, recs, fflag = YES;
  const int verbose = lon && !fscreen;

#if UNIX
  int c_out, save_fd_in;
#endif  /* if UNIX */

  if (mode == 'U')
    {
      if (c_list == NULL)
        {
          g_err(NO_SMUT, opts->errp);
        }

      c_comm_u();
      return;
    }

  if (f_list == NULL)
    {
      g_err(NO_SMUT, opts->errp);
    }

  if (mode == NO_OPT)
    {
      mode = f_list->disp;
    }

  switch (mode)
    {
    default:
      g_err(XIT_S_M_T, opts->errp);
      /* fallthrough */

    case 'T':
      f_list->trans = YES;
      (void)zmovelr(f_list->name, t_fname);
      /* fallthrough */

    case 'Q':
      /* fallthrough */
    case 'A':
      /* fallthrough */
    case 'S':
      /* fallthrough */
    case 'N':
      /* fallthrough */
#if UNIX
    case '!':
      /* fallthrough */
    case '|':
#endif  /* if UNIX */
      if (f_list->disp == 'M')
        {
          g_err(CLOSE_MERGE, opts->errp);
        }

      if (mode == 'Q')
        {
          break;
        }

      if (e_col)
        {
          out_buff()  /* flush buffer if not empty */;
        }

      if (verbose)
        {
          print_o_size(ft_out, f_list->name, out_u);
        }

      vsreopen(out_u);
#if UNIX && !defined(OMIT_POPEN)
      if (mode == '|')
        {
          c_out = dup(1), save_fd_in = fd_in_terminal;
          (void)close(1);
          fd_in_terminal = open(t_fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
          (void)unlink(t_fname);
          recs = Mem_to_proc(out_u, opts->o1.s);
          f_list->trans = NO;
          break;
        }

#endif  /* if UNIX && !defined(OMIT_POPEN) */
      recs = vssizeof(out_u);
      if (f_list->trans)
        {
          if (trans_open_count > 0)
            {
              g_err(TRANS_IN_USE, opts->errp);
            }

          if (mode == 'A' && trans_u != NULL)
            {
              const int trans_recs = vssizeof(trans_u);
              vsreopen(trans_u);
              vsseek(trans_u, trans_recs);
              vscopy(trans_u, out_u, recs);
              vsreopen(trans_u);
              f_list->trans = NO;
            }
          else
            {
              if (trans_u != NULL)
                {
                  vsclose(trans_u);
                }

              trans_u = out_u;
              fflag = NO;
            }
        }

      elif(Mem_to_disk(out_u, f_list->name, f_list->disp) == EOF)
        g_err(FILE_ERROR, f_list->name);
      break;

    case 'M':
      if (f_list->disp != 'M')
        {
          g_err(CLOSE_SAVE, opts->errp);
        }

      if (f_list->trans)
        {
          --trans_open_count;
        }
    }

  rest_in_vars(&f_list->save);
  if (mode == 'M')
    {
      pop_flist(&in_u, fflag);
    }
  else
    {
      vsseek(in_u, f_list->save.in_rec_num);
      rest_out_vars(&f_list->save);
      pop_flist(&out_u, fflag);
    }

  save_all(&g_save);  /* user cannot Forget X */
#if UNIX
  if (mode == '|')
    {
      if (recs != EOF)
        {
          vsunlink(out_u);
          o_rec += Disk_to_mem(t_fname, out_u, 'F');
          alter_end(g_rec + recs, 'P');
          if (verbose)
            {
              print_o_size("Pipe", "-", out_u);
            }
        }

      (void)dup2(c_out, 1);
      (void)close(c_out);
      fd_in_terminal = save_fd_in;
      if (recs == EOF)
        {
          g_err(SYS_COM_FAIL, opts->errp);
        }
    }

#endif  /* if UNIX */
}

/*
 *  Save verb.  Push current output file.
 */

private
void
Save(VERB csc opts)
{
  FILE_LIST *sptr;
  const int mode = opts->o1.q, temp_file = opts->o1.v == 0;

  switch (mode)
    {
    default:
      g_err(I_OPT, opts->errp);
      /* fallthrough */

    case 'Q':
      Xit(opts);
      break;

    case 'A':
      /* fallthrough */
    case 'N':
      /* fallthrough */
    case '!':
      sptr = get_flist();
      if ( ( sptr->next = f_list ) != NULL )
        {
          f_list->prev = sptr;
        }

      sptr->disp = (char)mode;
      sptr->trans = (byte)temp_file;
      (void)zmovelr(sptr->name, temp_file ? t_fname : opts->o1.s);
      save_all(&sptr->save);
      sptr->old_u = out_u;
      out_u = vsopen();
      i_col = e_col = 0  /* save deals with whole records */;
      o_rec = -1;
      f_list = sptr;
      save_all(&g_save);
    }
}

/*
 *  Merge verb.  Push current input file. If no filename given,
 *  use the transient file.
 */

private
void
Merge(VERB csc opts)
{
  new_input_file(opts, &f_list, &in_u);
  if (lon && !fscreen)
    {
      print_i_size(ft_merge, f_list->name, in_u);
    }

  f_list->disp = 'M';
  save_all(&f_list->save);
  save_all(&g_save);
  g_eof = NO;
  g_rec = -1;
  (void)fill_buff();
}

/*
 *  Use verb.  Push the current command file.  If no filename
 *  is given, execute commands from the transient file.
 */

private
void
Use(VERB csc opts)
{
  new_input_file(opts, &c_list, &comm_u);
  lon = NO;  /* turn off listing */
  Drive(D_USE_FILE);
}

/*
 *  Get Hex string.
 */

private
int
strhex(char *s, word len)
{
  const char *sp = s;

  if (len & ODD_MASK)
    {
      (void)moverl(s + 1, s, ++len);
      *s = '0';
    }

  while (*sp)
    {
      *s++ = xtoc(sp);
      sp += 2;
    }
  *s = EOS;
  return len >> 1;
}

/*
 *  Expand abbreviations.
 */

private
const char *
short_num(char *t, const char *p)
{
  char def = '0';
  char c = p[-1];

  if ( isdigit(c) )
    {
      c = '#';
      --p;
    }

  switch (c)
    {
    case '+':
    case '-':
      ++def;
      /* fallthrough */

    case '#':
      *t++ = 'T';
      *t++ = c;
      if ( isdigit(*p) )
        {
          while ( isdigit(*p) )
            {
              *t++ = *p++;
            }
        }
      else
        {
          *t++ = def;
        }

      *t = EOS;
      break;

    case '@':
      (void)strcpy(t, "T#{@}");
    }

  return p;
}

private
void
short_str(char *t, const char *p)
{
  switch (p[-1])
    {
    case '/':
      if ( nullstr(p) )
        {
          *t++ = 'T';
          *t++ = SPACE;
        }

      break;

    case '?':
      t = mmovelr4(t, "T#0 ");
      break;

    case '^':
      --p;
    }

  (void)sprintf(t, se_fcom1, p);
}

/*
 *  Numeric endpoints.
 */

private
const char *
num_ep(OPTION *const o, const char *p, char csc erp)
{
  if ( isdigit(*p) )
    {
      o->v = 0;
      while ( isdigit(*p) )
        {
          o->v = o->v * 10 + *p++ - '0';
        }
    }

  elif(*p++ == '{')
    {
      if ( Expr_compile(&o->e, &p) )
        {
          g_err(INT_OPT, erp);
        }

      o->q |= OP_CALC;
    }
  else
    {
      g_err(DIG_OR_END, erp);
    }

  return p;
}

/*
 *  String endpoints.
 */

private
const char *
string_ep(OPTION *const o, const char *p, char csc erp)
{
  int len, hex = NO;
  char *sp, c;

  repeat
  {
    switch ( c = u_star(p++) )
      {
      default:
        g_err(M_DELIM, erp);
        /* fallthrough */

      case 'X':
        ++hex;
        continue;

      case 'I':
        o->q |= STR_IGNORE;
        continue;

      case 'N':
        o->q |= STR_NEGATE;
        continue;

      case 'R':
      case 'C':
      case 'F':
      case 'S':
        if ( ( o->q & BYTE_MASK ) == 'G' && c == 'R' )
          {
            c = 'r';
          }

        o->q = ( o->q & ( BYTE_MASK << 8 ) ) | c;
        continue;

      case DELIM:
        if ( gdss(o->s, &len, &p) )
          {
            g_err(M_DELIM, erp);
          }

        if (hex)
          {
            o->v = strhex(o->s, len);
          }
        else
          {
            o->v = len;
          }

        if (o->q & STR_IGNORE)
          {
            for (sp = o->s; len--; ++sp)
              {
                *sp = u_star(sp);
              }
          }
      }
    break;
  }
  return p;
}

/*
 *  General endpoints.
 */

private
const char *
ep(const char *p, OPTION *const o, const int d, char csc erp)
{
  int c2 = o->q;
  const int neg = ( c2 == '-' );

  if (neg || c2 == '+')
    {
      o->q = c2 = u_star(p++);
    }

  switch (c2)
    {
    default:
      g_err(I_REPEAT, erp);
      /* fallthrough */

    case '{':
    case NUMERIC:
      o->q = neg ? MRECS : RECS;
      return num_ep(o, --p, erp);

    case '#':
      o->q = R_END;
      return num_ep(o, p, erp);

    case '*':
      o->q = R_TIMES;
      if (isdigit(*p) || *p == '{')
        {
          p = num_ep(o, p, erp);
        }
      else
        {
          o->v = -1;  /* loop until failure */
        }

      break;

    case 'O':
      o->q = OR_END;
      return num_ep(o, p, erp);

    case 'S':
      if (d)
        {
          o->v = neg;
          break;
        }
      /* fallthrough */

    case 'I':
      /* fallthrough */
    case 'X':
      /* fallthrough */
    case 'F':
      /* fallthrough */
    case 'N':
      /* fallthrough */
    case 'C':
      /* fallthrough */
    case 'R':
      /* fallthrough */
    case DELIM:
      o->q = d ? 'G' : 'B';
      return string_ep(o, --p, erp);

    case 'E':
      o->q = OP_EOF;
    }

  return p;
}

/*
 *  Extract file name or command in quotes.
 */

private
int
get_name(char cssc ptr, char *name)
{
  int i = 0;
  const char *p = *ptr;

  if (*p == '!')
#if UNIX
    {
      *name++ = *p++;
    }

#else  /* if UNIX  */
    {
      g_err(I_OPT, p);
    }
#endif  /* if UNIX */

  switch (*p)
    {
    case GRAVE:
      /* fallthrough */
    case SQUOTE:
      /* fallthrough */
    case DQUOTE:
      /* fallthrough */
    case DEL:
      ++p;
      (void)gdss(name, &i, &p);
      break;

    default:
      for (; !comsep(*p); ++p)
        {
          if (i > STR_LEN)
            {
              continue;
            }

          name[i++] = *p;
        }

      name[i] = EOS;
    }

  *ptr = p;
  return i;
}

/*
 *  Parse endpoints for PT etc and loops.
 */

private
const char *
parse_G(const char *p, VERB *const opts)
{
  int o_while = 0, c2 = opts->o1.q, c3;
  char csc erp = opts->errp;
  const int c1 = opts->comm;

  if (c2 == NO_OPT)
    {
      opts->o1.q = ( c1 == ')' ? R_TIMES : RECS );
      opts->o1.v = 1;
      return p - 1;
    }

  switch (c1)
    {
    case 'L':
      switch (c2)
        {
        case 'O':
          if ( ( c3 = u_star(p) ) == 'N' )
            {
              opts->o1.q = L_LON;
              return p + 1;
            }

          if (c3 == 'F')
            {
              if (u_star(++p) == 'F')
                {
                  opts->o1.q = L_LOFF;
                  return p + 1;
                }
            }

          g_err(I_OPT, erp);
          /* fallthrough */

        case 'H':
          /* fallthrough */
        case 'M':
          /* fallthrough */
        case 'X':
          /* fallthrough */
        case 'D':
          /* fallthrough */
        case 'S':
          return p;
        }
      /* fallthrough */

    case 'K':
      /* fallthrough */
    case 'J':
      if (c2 == '.')
        {
          g_err(NO_DOT, erp);
        }

      if (c1 == 'J' && c2 == 'P')
        {
          return p;
        }

      goto standard;

    case 'T':
      /* fallthrough */
    case 'P':
      /* fallthrough */
    case 'V':
      /* fallthrough */
    case ')':
standard:
      if (c2 == 'W' || c2 == 'U')
        {
          o_while = c2 == 'W' ? OP_WHILE : OP_UNTIL;
          c2 = opts->o1.q = u_star(p++);
        }

      if (c2 == '.')
        {
          opts->o1.q = NO_OPT;
          --p;
        }
      else
        {
          p = ep(p, &opts->o1, NO, erp);
        }

      if (*p == '.')
        {
          if (opts->comm == ')' && c2 != '.')
            {
              g_err(NO_DOT, erp);
            }

          opts->dot = YES;
          ++p;
          if ( comsep(*p) )
            {
              g_err(I_OPT, erp);
            }

          opts->o2.q = u_star(p);
          p = ep(++p, &opts->o2, YES, erp);
        }

      if (opts->o2.q == R_TIMES)
        {
          opts->o1 = opts->o2;
          opts->dot = NO;
        }

      opts->o1.q |= o_while;
    }

  return p;
}

/*
 *  Parse Insert verb options.
 */

private
const char *
parse_I(const char *p, VERB *const opts)
{
  const char *dp, *const erp = opts->errp, c2 = (char)opts->o1.q;

  switch (c2)
    {
#if FULL_G
    case 'A':
    case 'B':
      if (opts->comm == 'D')
        {
#endif  /* if FULL_G */
    default:
      g_err(I_OPT, erp);
#if FULL_G
    }
  if (*p == '$')
    {
      opts->o2.q = YES;
      ++p;
    }

  break;

#endif  /* if FULL_G */
    case 'X':
      p = ep(p, &opts->o1, NO, opts->errp);
      opts->o1.q = 'X';
      break;

#if UNIX
    case '!':
      if (get_name(&p, opts->o1.s) == 0)
        {
          g_err(I_OPT, erp);
        }

      break;

#endif  /* if UNIX */
    case 'P':
      if (opts->comm != 'D')
        {
          g_err(I_OPT, erp);
        }

    case 'D':
    case 'F':
#if FULL_G
    case 'S':
#endif  /* if FULL_G */
      break;

    case 'C':
      if (*p++ != '{')
        {
          g_err(I_OPT, erp);
        }
      /* fallthrough */

    case '{':
      if ( Expr_compile(&opts->o1.e, &p) )
        {
          g_err(INT_OPT, erp);
        }

      break;

    case DELIM:
      dp = --p;
      do
        {
          ++p;
          while (*p != EOS && *p++ != c2)
            {
              ;
            }
        }
      while ( *p == c2 );
      opts->o1.v = (int)( p - dp );
      (void)movelrz(opts->o1.s, dp, opts->o1.v);
    }

  return p;
}

private
const MACRO *
get_macro(char csc name)
{
  const MACRO *macptr;

  if ( nullstr(name) )
    {
      g_err(NO_MAC_NAM, NULL);
    }

  for (macptr = mac_list; macptr != NULL; macptr = macptr->next)
    {
      if ( equal(name, macptr->name) )
        {
          break;
        }
    }

  return macptr;
}

/*
 *  Define a new macro.
 */

private
void
Create(const char *s)
{
  MACRO *m;
  int arg_count, i;
  char csc erp = s;
  char name[MAC_NAME_LEN + 1], subs[3], subs_ch;

  /* get macro name from command */

  skip_space(s);
  for (i = 0; wordch(*s); ++s)
    {
      if (i >= MAC_NAME_LEN)
        {
          continue;
        }

      name[i++] = *s;
    }

  name[i] = EOS;

  /* get subs char and count */

  ++s;
  if (gdss(subs, &i, &s) || i != 2)
    {
      g_err(NO_SUBS_CH, erp);
    }

  subs_ch = subs[0];
  if ( !( isdigit(subs[1]) ) )
    {
      g_err(NO_COUNT, erp);
    }

  arg_count = subs[1] - '0';

  /* rest of it is the command */

  skip_space(s);
  if ( nullstr(s) )
    {
      g_err(NO_M_COMM, erp);
    }

  /* add macro to front of the list */

  m = heap(MACRO);
  m->next = mac_list;
  (void)zmovelr(m->name, name);
  m->par_sub = subs_ch;
  m->nargs   = (byte)arg_count;
  m->text    = (const char *)getvec( size(s) );
  (void)zmovelr( (char *)m->text, s );

  /*
   * Note predefined macros are in read/only core, so
   * you cannot just simply replace existing ptr.
   */

  if (c_list == NULL && get_macro(name) != NULL)
    {
      (void)sprintf(name, "Warning: macro \"%s\" redefined", m->name);
      inform(name);
    }

  mac_list = m;
}

/*
 *  Expand macro calls.
 */

private
const char *
dotcallmac(char *const text, const char *p)
{
  int arg_count, i, arg_len[10];
  char subs_ch, ch, *t_ptr;
  const char *c_ptr, *args[10], *const erp = p - 1;
  const MACRO *macptr;

  skip_space(p);
  for (i = 0; wordch(*p); ++p)
    {
      if (i >= MAC_NAME_LEN)
        {
          continue;
        }

      text[i++] = *p;
    }

  text[i] = EOS;

  bzero(args, 10 * sizeof ( char * ) >> B_SIZE);
  if ( ( macptr = get_macro(text) ) == NULL )
    {
      g_err(M_NAME_NF, erp);
    }

  arg_count = macptr->nargs;
  subs_ch = macptr->par_sub;
  c_ptr = macptr->text;

  if (delim(ch = *p) != arg_count > 0)
    {
      g_err(ARG_NUM_INV, erp);
    }

  if (arg_count > 0)
    {

      /* set up pointers to the strings */

      for (i = 1; i <= arg_count; ++i)
        {
          if ( ( p = strchr(args[i] = ++p, ch) ) == NULL )
            {
              g_err(M_DELIM, erp);
            }

          arg_len[i] = p - args[i];
        }

      ++p;

      /* insert in the text */

      t_ptr = text;
      while ( ( ch = *c_ptr++ ) != EOS )
        {
          if (ch == subs_ch)
            {
              i = *c_ptr - '0';
              if (i < 1 || i > 9 || args[i] == NULL)
                {
                  g_err(ARG_NUM_INV, erp);
                }

              t_ptr = mmovelr(t_ptr, args[i], arg_len[i]);
              ++c_ptr;
            }
          else
            {
              *t_ptr++ = ch;
            }

          if (t_ptr - text >= STR_LEN)
            {
              g_err(COMM_TOO_LONG, erp);
            }
        }
      *t_ptr = EOS;
    }
  else
    {
      (void)zmovelr(text, c_ptr);
    }

  return p;
}

/*
 *  Process a verb.
 */

private
int
endpoint(VERB **const prog, char cssc ptr)
{
  VERB *opts;
  int c1, c2;
  char com;
  const char *erp, *p = *ptr;
  VERB_LIST *v;
  string exp;

  while (*p && ( isspace(*p) || *p == ',' ) )
    {
      ++p;
    }

  /* end of command line */

  if (*p == EOS)
    {
      return NO;
    }

  erp = p;

  /* expand # @ / etc */

  switch ( c1 = u_star(p++) )
    {
    case '#':
    case '+':
    case '-':
    case '@':
    case NUMERIC:
      p = short_num(exp, p);
      goto comp;

    case '/':
    case '?':
    case '^':
      short_str(exp, p);
      p = get_eos(p);
      goto comp;

    case '.':
      p = dotcallmac(exp, p);

comp:
      G_compile(prog, exp);
      for (opts = *prog; opts != NULL; opts = opts->next)
        {
          opts->errp = erp;
        }

      *ptr = p;
      return YES;

    case 'C':
      Create(p);
      return NO;
    }

  /* get space for normal verb */

  if (g_free_list != NULL)
    {
      opts = g_free_list;
      g_free_list = opts->next;
    }
  else
    {
      opts = heap(VERB);
    }

  opts->next = NULL;
  *prog = opts;

  opts->errp = erp;
  opts->comm = (char)c1;
  opts->o1.q = c2 = ( comsep(*p) ? NO_OPT : u_star(p) );
  opts->o2.q = NO_OPT;
  opts->dot = NO;
  opts->o1.e = opts->o2.e = NULL;

  switch (c1)
    {
    default:
      g_err(I_COMMAND, erp);
      /* fallthrough */

    case '(':
    case ';':
      v = par_stack_ptr->next;
      if (v == NULL)
        {
          v = par_stack_ptr->next = heap(VERB_LIST);
          v->prev = par_stack_ptr;
          v->next = NULL;
        }

      v->prog = opts;
      par_stack_ptr = v;
      break;

    case 'Y':
    case 'A':
    case 'B':
    case 'R':
      ++p;
      opts->o1.q = 'G';
      if (c2 == 'R')
        {
          if (c1 == 'Y')
            {
              g_err(I_OPT, erp);
            }

          ++p;
          opts->o1.q = 'r';
        }

      if ( gdss(opts->o1.s, &opts->o1.v, &p)
          || gdss(opts->o2.s, &opts->o2.v, &p) )
        {
          g_err(M_DELIM, erp);
        }

      if (c1 == 'Y' && opts->o1.v != opts->o2.v)
        {
          g_err(Y_LENGTHS, erp);
        }

      break;

    case 'X':
      if (c2 != NO_OPT)
        {
          ++p;
        }

#if UNIX
      if (c2 == '|')
        {
          (void)get_name(&p, opts->o1.s);
        }

#endif  /* if UNIX */
      break;

#if FULL_G
    case 'W':
#endif  /* if FULL_G */
    case 'F':
      if (c2 != NO_OPT)
        {
          g_err(I_OPT, erp);
        }

    case 'Q':
    case 'E':
    case ':':
      while ( wordch(*p) )
        {
          ++p;
        }
      break;

    case 'D':
    case 'I':
      p = parse_I(p + 1, opts);
      break;

    case 'S':
      switch (c2)
        {
        default:
          g_err(I_OPT, erp);
          /* fallthrough */

        case 'A':
          /* fallthrough */

        case 'N':
          ++p;
          /* fallthrough */

#if UNIX
          /* fallthrough */
        case '!':
#endif  /* if UNIX */
          opts->o1.v = get_name(&p, opts->o1.s);
          break;

        case 'Q':
          ++p;
          break;

        case NO_OPT:
          opts->o1.q = 'N';
          opts->o1.v = 0;
        }
      break;

    case 'O':
      /* fallthrough */
    case 'M':
      /* fallthrough */
    case 'U':
      opts->o1.v = get_name(&p, opts->o1.s);
      break;

    case 'G':
      --p;
      opts->comm = '!';
      goto copy_com;

    case '!':
      if (c2 == NO_OPT)
        {
          if ( ( p = getenv(shell_var) ) == NULL )
            {
              p = shell_bin;
            }
        }

copy_com:
      (void)zmovelr(opts->o1.s, p);
      p = get_eos(p);
      break;

    case 'N':
    case 'J':
      opts->o1.v = 0;
      c2 = *p;
      while (isdigit(c2) || c2 == '-' || c2 == '.')
        {
          opts->o1.s[opts->o1.v++] = (char)c2;
          c2 = *++p;
        }
      if (opts->o1.v > 4)
        {
          g_err(BAD_NUM, erp);
        }

      if ( comsep(c2) )
        {
          c2 = 'd';
        }
      else
        {
          ++p;
        }

      opts->o2.v = c2;
      break;

    case ')':
      do
        {
          if (par_stack_ptr == com_stack_ptr)
            {
              g_err(MISSING_BRA, erp);
            }

          com = par_stack_ptr->prog->comm;
          par_stack_ptr->prog->cpar = opts;
          par_stack_ptr = par_stack_ptr->prev;
        }
      while ( com != '(' );
      /* fallthrough */

    case 'P':
      /* fallthrough */
    case 'T':
      /* fallthrough */
    case 'V':
      /* fallthrough */
    case 'L':
      /* fallthrough */
    case 'K':
      p = parse_G(p + 1, opts);
      break;

    case '{':
      if ( Expr_compile(&opts->o1.e, &p) )
        {
          if (depth)
            {
              g_err(INT_OPT, erp);
            }

          opts->o1.q = C_REPEAT;
        }
      else
        {
          opts->o1.q = C_SIDE;
        }

      break;

    case 'H':
#if FULL_G
    {
      FSTR h_list[] = "WAERLVDMS";
      char csc h_op = strchr(h_list, c2);
      if (c2 == NO_OPT || c2 == 'E' && !comsep(p[1]) || h_op == NULL)
        {
          opts->o1.q = 9;
        }
      else
        {
          opts->o1.q = h_op - h_list;
        }
    }
#endif  /* if FULL_G */
      p = get_eos(p);
    }

  *ptr = p;

  return YES;
}

/*
 *  Compile entire command line into linked list.
 */

private
void
G_compile(VERB **start, const char *ptr)
{
  VERB *v;

  while ( endpoint(start, &ptr) )
    {
      for (v = *start; v->next != NULL; v = v->next)
        {
          ;
        }

      start = &v->next;
    }
}

/*
 *  Initialise screen mode.
 */

private
void
init_screen(void)
{
  char **p, **last;
  int lines;

  (void)initscr();
  nonl();
  noecho();
  keypad(stdscr, YES);

#if defined(IMMEDOK) && !defined(__WATCOMC__)
  (void)immedok(stdscr, YES);
#endif  /* if defined(IMMEDOK) && !defined(__WATCOMC__) */

#if DOS
  lines = LINES;
#else  /* if DOS */
# if COLOUR
  if (start_color() == OK)
    {
      (void)init_pair(1, COLOR_WHITE,   COLOR_RED);   /* matched text       */
      (void)init_pair(2, COLOR_WHITE,   COLOR_BLUE);  /* control characters */
      (void)init_pair(3, COLOR_WHITE,   COLOR_BLACK); /* EOF marker         */
      (void)init_pair(4, COLOR_YELLOW,  COLOR_BLACK); /* the scale line     */
      (void)init_pair(5, COLOR_CYAN,    COLOR_BLACK); /* the status line    */
      (void)init_pair(6, COLOR_GREEN,   COLOR_BLACK); /* normal text        */
      (void)init_pair(7, COLOR_RED,     COLOR_BLACK); /* query              */
      (void)init_pair(8, COLOR_MAGENTA, COLOR_BLACK); /* margins            */
      (void)init_pair(9, COLOR_BLUE,    COLOR_RED);   /* matched binary     */
    }
  else
    {
      found_col  = M_FOUND_COL;   /* matched text       */
      cntrl_col  = M_CNTRL_COL;   /* control characters */
      eof_col    = M_EOF_COL;     /* EOF marker         */
      scale_col  = M_SCALE_COL;   /* the scale line     */
      status_col = M_STATUS_COL;  /* the status line    */
      norm_col   = M_NORM_COL;    /* normal text        */
      query_col  = M_QUERY_COL;   /* query              */
      marg_col   = M_MARG_COL;    /* margins            */
      found_ctrl = M_FOUND_CTRL;  /* matched binary     */
    }

# endif  /* if COLOUR */

  /* define constants that depend on window size */

  last_col = COLS - 1;
  h_inc = COLS / 2;
  last_line = ( lines = LINES ) - 1;
  text_lines = lines - FIRST_LINE;
#endif  /* if DOS */

  /* get space for all vectors and divide them up */

  p = s_buf = (char **)getvec( ( sizeof ( char * ) + 4 ) * lines );
  last = p++ + lines;
  while (p < last)
    {
      *p++ = getbuf(L_LEN);
    }
  eor = (short *)p;
  eor[TEMPLATE_LINE] = 0;
  s_eor = eor + lines;
  wfill(s_eor, L_LEN, lines);
}

/*
 *  Check buffer line size and expand if needed.
 */

private
int
set_eor(const int line, const int nsize)
{
  const int asize = s_eor[line];

  if (nsize > E_BUFF_LEN)
    {
      message("Line too long");
      qq_loop = NO;
      return YES;
    }

  if (nsize > asize)
    {
      char *const temp = s_buf[line];
      s_buf[line] = getbuf( s_eor[line] = (short)( nsize + L_LEN ) );
      (void)movelr(s_buf[line], temp, asize);
      rlsevec(temp);
    }

  eor[line] = (short)nsize;

  return NO;
}

/*
 *  Get the next record in the old_file, either from the stack, or
 *  if the stack is empty, from the file itself.
 */

private
int
next_line(const int line)
{
  int len;
  const byte *p;

  if (in_stack != NULL)
    {
      (void)set_eor( line, pop_length(in_stack) );
      pop_line(&in_stack, s_buf[line]);
    }
  else
    {
      ++g_rec;
      if ( ( len = vsgetrec(in_u, &p) ) == EOF )
        {
          g_eof = YES;
          eor[line] = EOF;
          return NO;
        }

      len = ltabex(e_buff, p, len);
      (void)set_eor(line, len);
      (void)movelr(s_buf[line], e_buff, len);
    }

  return YES;
}

/*
 *  Read the input file into the screen buffer.
 */

private
void
file_to_buf(const int com)
{
  int line = FIRST_LINE;

  switch (com)
    {
    case PREV_PAGE:
    case MOVE_TOF:
    case MOVE_EOF:
    case PREV_LINE:
    case MOVE_ABS:
    case SE_ENTER:
      if (!g_eof)
        {
          (void)set_eor(line, i_eor);
          (void)movelr(s_buf[line++], i_buff, i_eor);
        }
    }

  do
    {
      if (line > last_line)
        {
          return;
        }
    }
  while ( next_line(line++) );
  --line;

  wfill(&eor[line], EOF, LINES - line);
}

/*
 *  Move to absolute position in file.
 */

private
void
move_to(int line)
{
  if (line <= g_rec || g_rec <= o_rec && o_rec >= line)
    {
      (void)wrapround();
    }

  if (vssizeof(in_u) > 0)
    {
      if (line < 0)
        {
          line = 0;
        }

      alter_end(line, 'T');
    }
}

#if DOS

/*
 *  Mini Curses for G code.
 */

private
short curs_row, curs_col;

# if ( TINY_G && !defined(WCL386) ) \
    || ( DOS && defined(_MSC_VER) )
private
chtype *near v_base = (chtype *)0xb8000000;
#  define call_bios int86
# else
private
chtype *v_base = (chtype *)0xb8000;
#  define call_bios int386
# endif  /* if ( TINY_G && !defined(WCL386) )
              || ( DOS && defined(_MSC_VER) ) */

private
chtype *near h_base, *near t_base;

private
void
initscr(void)
{
  byte rows = bios_byte(0x484);
  const word cols = COLS = bios_word(0x44A);

  if (rows != 24)
    {
      if (rows == 0 || bios_word(0x44C) <= 4096)
        {
          rows = 24;
        }
    }

  LINES = rows + 1;

  if (bios_word(0x463) == 0x3B4)  /* or bios_byte( 0x449 ) == 7 */
    {
# if TINY_G
      v_base = (chtype *)0xb0000000;
# else  /* if TINY_G  */
      v_base = (chtype *)0xb0000;
# endif  /* if TINY_G */
    }

  last_line = rows;
  text_lines = rows - ( FIRST_LINE + 1 );
  h_inc = cols >> 1;
  last_col = cols - 1;
  t_base = ( h_base = v_base + cols ) + cols;
}

private
void
clrtoeol(void)
{
  wfill(v_base + curs_row * COLS + curs_col, norm_space, COLS - curs_col);
}

# if ASM86

extern ushort bios_getc(void);
#  pragma aux bios_getc =                      \
  "xor ah,ah"                                  \
  "int 16h"                                    \
  "test al,al"                                 \
  "jz scanc"                                   \
  "xor ah,ah"                                  \
  "scanc:" value[ax];

extern void bios_gotoxy(byte row, byte col);
#  pragma aux bios_gotoxy =                    \
  "mov ah,2"                                   \
  "xor bh,bh"                                  \
  "int 10h" parm[dh][dl] modify exact[ah bh];

extern void scr_fill(void *start, short len);
extern void scale_fill(void *start, short len);
extern void berase(void *start, short len);

#  pragma aux scr_fill =                       \
  "mov eax,0A200A20h"                          \
  "rep stosd" parm[es di][cx] modify[ax];

#  pragma aux scale_fill =                     \
  "mov eax,0E2E0E2Eh"                          \
  "rep stosd" parm[es di][cx] modify[ax];

#  pragma aux berase =                         \
  "mov eax,07200720h"                          \
  "rep stosd" parm[es di][cx] modify[ax];

# else  /* if ASM86 */

private
void
bios_gotoxy(const byte row, const byte col)
{
  union REGS regs;  /* need _REGS for MSVC? */

  regs.h.ah = 0x02;
  regs.h.bh =    0;
  regs.h.dh =  row;
  regs.h.dl =  col;
  call_bios(0x10, &regs, &regs);
}

#  define scr_fill(start, len)   wfill(start, 0x0A20, len)
#  define scale_fill(start, len) wfill(start, 0x0E2E, len)
#  define berase(start, len)     wfill(start, 0x0720, len)

# endif  /* if ASM86 */

private
ushort
curs_getc(void)
{
  bios_gotoxy( (byte)curs_row, (byte)curs_col );

# if ASM86
  return bios_getc();

# else  /* if ASM86 */
  {
    const ushort c = _bios_keybrd(_KEYBRD_READ);
    if (c & 0xFF)
      {
        return c & 0xFF;
      }

    return c;
  }
# endif  /* if ASM86 */
}

private
void
deleteln(void)
{
  chtype *const curs_y = v_base + curs_row * COLS;

  (void)bmovelr(curs_y, curs_y + COLS, ( last_line - curs_row ) * B_COLS);
}

private
void
insertln(void)
{
  chtype *const curs_y = v_base + curs_row * COLS;

  (void)bmoverl(curs_y + COLS, curs_y, ( last_line - curs_row ) * B_COLS);
  scr_fill(curs_y, B_COLS);
}

private
void
curs_chins(void)
{
  chtype *const p = v_base + curs_row * COLS + curs_col;

  (void)wmoverl(p + 1, p, last_col - curs_col);
}

private
void
napms(const unsigned long msec)
{
  const clock_t goal = clock() + ( msec * CLOCKS_PER_SEC ) / 1000;

  while ( goal > clock() )
    {
      ;
    }
}

private
void
put_byte(const chtype c)
{
  ( v_base + curs_row * COLS )[curs_col++] = c | cntrl_col;
}

private
void
mv_put_byte(const int y, const int x, const chtype c)
{
  v_base[y * COLS + x] = c | cntrl_col;
}

# if ASM86

/* scale line writes */

extern void put_scale(chtype *start, const char *s, ushort n);
#  pragma aux put_scale =                                     \
  "mov ah,0eh"                                                \
  "nextc: lodsb"                                              \
  "stosw"                                                     \
  "dec dx"                                                    \
  "jnz nextc" parm[es di][ds si][dx] modify[ax];

/* status line writes */

extern void put_status(chtype *start, const char *s, ushort n);
#  pragma aux put_status =                                    \
  "mov ah,0bh"                                                \
  "nextc: lodsb"                                              \
  "stosw"                                                     \
  "dec dx"                                                    \
  "jnz nextc" parm[es di][ds si][dx] modify[ax];

/* write text (fixed attribute given) and clear remainder of line */

extern void put_seq(chtype *start, const char *s, ushort n, ushort attr,
                    ushort c);
#  pragma aux put_seq =                                       \
  "sub cx,bx"                                                 \
  "nextc: lodsb"                                              \
  "stosw"                                                     \
  "dec bx"                                                    \
  "jnz nextc"                                                 \
  "mov ax,0a20h"                                              \
  "rep stosw" parm[es di][ds si][bx][ax][cx];

/* write matched text (variable attributes), clear remainder */

extern void put_matched(chtype *start, const char *s, ushort n);
#  pragma aux put_matched =                                   \
  "mov ah,4fh"                                                \
  "nextw: lodsb"                                              \
  "cmp al,20h"                                                \
  "jb ctrlc"                                                  \
  "cmp al,7Fh"                                                \
  "jae ctrlc"                                                 \
  "stosw"                                                     \
  "dec bx"                                                    \
  "jnz nextw"                                                 \
  "jmp endw"                                                  \
  "ctrlc: and al,7Fh"                                         \
  "cmp al,20h"                                                \
  "jl stctl"                                                  \
  "cmp al,7Fh"                                                \
  "jl print"                                                  \
  "stctl: and al,1fh"                                         \
  "add al,40h"                                                \
  "print: mov ah,49h"                                         \
  "stosw"                                                     \
  "mov ah,4fh"                                                \
  "dec bx"                                                    \
  "jnz nextw"                                                 \
  "endw:" parm[es di][ds si][bx] modify[ax];

/* write normal text (variable attributes), clear remainder */

extern chtype *put_text(chtype *start, const char *s, ushort n, ushort c);
#  pragma aux put_text =                                      \
  "mov ah,0ah"                                                \
  "test dx,dx"                                                \
  "jle endw"                                                  \
  "cmp dx,cx"                                                 \
  "jb partr"                                                  \
  "mov dx,cx"                                                 \
  "partr: sub cx,dx"                                          \
  "nextw: lodsb"                                              \
  "cmp al,20h"                                                \
  "jb ctrlc"                                                  \
  "cmp al,7Fh"                                                \
  "jae ctrlc"                                                 \
  "stosw"                                                     \
  "dec dx"                                                    \
  "jnz nextw"                                                 \
  "jmp endw"                                                  \
  "ctrlc: and al,7Fh"                                         \
  "cmp al,20h"                                                \
  "jl still"                                                  \
  "cmp al,7Fh"                                                \
  "jl print"                                                  \
  "still: and al,1fh"                                         \
  "add al,40h"                                                \
  "print: mov ah,1fh"                                         \
  "stosw"                                                     \
  "mov ah,0ah"                                                \
  "dec dx"                                                    \
  "jnz nextw"                                                 \
  "endw: mov eax,0A200A20h"                                   \
  "shr cx,1"                                                  \
  "rep stosd"                                                 \
  "jnc nocp"                                                  \
  "stosw"                                                     \
  "nocp:" value[es di] parm[es di][ds si][dx][cx] modify[ax];

# endif  /* if ASM86 */

#endif  /* if DOS */

#if ASM86 == 0

# if DOS

private
void
put_seq(const char *s, int n, chtype attr)
{
  chtype *const curs_y = v_base + curs_row * COLS;

  while (n--)
    {
      curs_y[curs_col++] = *s++ | attr;
    }
}

private
void
cput_seq(const char *s, int n, chtype attr_print, chtype attr_cntrl)
{
  chtype *const curs_y = v_base + curs_row * COLS;

  while (n--)
    {
      const chtype c = *s++;
      if ( isprint(c) )
        {
          curs_y[curs_col++] = c | attr_print;
        }
      else
        {
          curs_y[curs_col++] = toprint(c) | attr_cntrl;
        }
    }
}

# else  /* if DOS */

/* place char on screen */

#  define put_byte(c) addch( (chtype)( c ) )
#  define mv_put_byte(y, x, c) ( move(y, x), put_byte(c) )

private
void
put_seq(const char *s, int n, chtype attr)
{
  (void)attrset(attr);
  while (n--)
    {
      (void)put_byte(*s++);
    }
}

private
void
cput_seq(const char *s, int n, chtype attr_print, chtype attr_cntrl)
{
  (void)attrset(attr_print);
  while (n--)
    {
      const chtype c = *s++;
      if ( isprint(c) )
        {
          (void)put_byte(c);
        }
      else
        {
          (void)attrset(attr_cntrl);
          (void)put_byte( toprint(c) );
          (void)attrset(attr_print);
        }
    }
}

# endif  /* if DOS */

private
void
put_text(const char *s, int len, int rem)
{
  if (len > rem)
    {
      len = rem;
    }

  if (len > 0)
    {
      cput_seq(s, len, norm_col, cntrl_col);
    }

  if (len < rem)
    {
      (void)clrtoeol();
    }
}

# define put_status movelr

#endif  /* if ASM86 == 0 */

/*
 *  Convert a column number into a screen offset and col.
 */

private
void
set_col(int new_col)
{
  if (new_col <= 0)
    {
      c_sol();
      return;
    }

  if (new_col > E_BUFF_LEN)
    {
      new_col = E_BUFF_LEN;
    }

  col = new_col - offset;
  if (col < 0 || col > last_col)
    {
      offset = new_col - new_col % h_inc - h_inc;
      if (offset < 0)
        {
          offset = 0;
        }

      col = new_col - offset;
    }
}

/*
 *  Display the ********* EOF ********* marker.
 */

private
void
disp_eof(const int r)
{
#if ASM86
  put_seq(v_base + r * COLS, eof_mess, 29, eof_col, COLS);
#else  /* if ASM86 */
  (void)move(r, 0);
  put_seq(eof_mess, 29, eof_col);
  attrset(norm_col);
  (void)clrtoeol();
#endif  /* if ASM86 */
}

/*
 *  Display the template line.
 */

private
void
disp_template(void)
{
  int i, len;
  char buf[4];

#if ASM86
  chtype *p = t_base;
  chtype csc last = p + COLS;

  scale_fill(p, B_COLS);
  for (i = 5; i < COLS; i += 10)
    {
      p[i] = '+' | scale_col;
    }

  for (i = offset; p < last; i += 10, p += 10)
    {
      len = sprintf(buf, "%d", i / 10);
      put_scale(p, buf, len);
    }

#else  /* if ASM86 */
  string scale;
  char *p = scale;
  char csc last = p + COLS;

  (void)memset(p, '.', COLS);
  for (i = 5; i < COLS; i += 10)
    {
      p[i] = '+';
    }

  for (i = offset; p < last; i += 10, p += 10)
    {
      len = sprintf(buf, "%d", i / 10);
      (void)movelr(p, buf, len);
    }

  (void)move(TEMPLATE_LINE, 0);
  put_seq(scale, COLS, scale_col);
  attrset(norm_col);
#endif  /* if ASM86 */
}

/*
 *  Display remainder of line.
 */

private
void
disp_rest(void)
{
  const ushort f_col = FILE_COL;

#if ASM86
  chtype *const start = v_base + row * COLS + col;
  put_text(start, BUF(f_col), eor[row] - f_col, COLS - col);
#else  /* if ASM86 */
  (void)move(row, col);
  put_text(BUF(f_col), eor[row] - f_col, COLS - col);
#endif  /* if ASM86 */
}

/*
 *  Display one line on the screen.
 */

private
void
disp_line(void)
{
#if ASM86
  chtype *const start = v_base + row * COLS;
  put_text(start, BUF(offset), eor[row] - offset, COLS);
#else  /* if ASM86 */
  (void)move(row, 0);
  put_text(BUF(offset), eor[row] - offset, COLS);
#endif  /* if ASM86 */
}

private
void
disp_matched_text(const int m_row, const int m_col, int m_len)
{
  if (m_len > COLS - m_col)
    {
      m_len = COLS - m_col;
    }

  if (m_len > 0)
    {
#if ASM86
      chtype *const start = v_base + m_row * COLS + m_col;
      put_matched(start, s_buf[m_row] + m_col + offset, m_len);
#else  /* if ASM86 */
      (void)move(m_row, m_col);
      cput_seq(s_buf[m_row] + m_col + offset, m_len, found_col, found_ctrl);
#endif  /* if ASM86 */
    }
}

/*
 *  Display the text in the screen buffer.
 */

private
void
disp_row(const int r)
{
  if (eor[r] == EOF)
    {
      if (r == FIRST_LINE || eor[r - 1] != EOF)
        {
          disp_eof(r);
        }

#if DOS
      else
        {
          scr_fill(v_base + r * COLS, B_COLS);
        }
#endif  /* if DOS */
    }
  else
    {
      const int c_row = row;
      row = r;
      disp_line();
      row = c_row;
    }
}

private
void
disp_home(void)
{
#if ASM86
  put_text(
    h_base,
    s_buf[COMMAND_LINE] + offset,
    eor[COMMAND_LINE] - offset,
    COLS);
#else  /* if ASM86 */
  (void)move(COMMAND_LINE, 0);
  put_text(s_buf[COMMAND_LINE] + offset, eor[COMMAND_LINE] - offset, COLS);
#endif  /* if ASM86 */
}

/*
 *  Display entire text area
 */

private
void
disp_text(void)
{
  int i;

#if ASM86
  chtype *start = v_base + FIRST_LINE * COLS;
  for (i = FIRST_LINE; i < LINES && eor[i] != EOF; ++i)
    {
      start = put_text(start, s_buf[i] + offset, eor[i] - offset, COLS);
    }

#else  /* if ASM86 */
  for (i = FIRST_LINE; i < LINES && eor[i] != EOF; ++i)
    {
      (void)move(i, 0);
      put_text(s_buf[i] + offset, eor[i] - offset, COLS);
    }

#endif  /* if ASM86 */
  if (i < LINES)
    {
      disp_eof(i);
#if ASM86
      scr_fill(start + COLS, ( LINES - i + 1 ) * B_COLS);
#else  /* if ASM86 */
      while (++i < LINES)
        {
          (void)move(i, 0);
          (void)clrtoeol();
        }
#endif  /* if ASM86 */
    }
}

/*
 *  Delete a segment of a line.
 *  From the current position (col + offset) to (but not including)
 *  the end_col.
 */

private
void
del_seg(const int end_col)
{
  const int first_col = FILE_COL, gap = end_col - first_col;
  char *const p = BUF(first_col);

  /* Adjust buffer */

  (void)movelr(p, p + gap, eor[row] - end_col);
  eor[row] -= gap;

  /* Adjust screen */

  disp_rest();
}

/*
 *  Update cursor position, eof on status line.
 */

private
void
status(void)
{
  static int pos_len = 0, eor_start = E_BUFF_SIZE, eof_start = 0;
  int len, eol, new_eor;
  const int fcol = FILE_COL, sop = START_OF_PAGE;
  char buf[L_LEN], *p = buf;

  if (row == COMMAND_LINE)
    {
      p += sprintf(buf, "%d", sop);
      eol = eor[FIRST_LINE];
    }
  else
    {
      p += sprintf(buf, line_pos + 5, FILE_LINE, fcol);
      if ( ( eol = eor[row] ) == EOF )
        {
          p = mmovelr5(p, " (EOF");
        }

      elif(eol > fcol)
          p += sprintf(p, " (%02X", s_buf[row][fcol] & BYTE_MASK);

      else
        {
          p = mmovelr5(p, " (EOR");
        }
      *p++ = ')';
    }

  len = p - buf;
  if (len < pos_len)
    {
      space_fill(p, pos_len - len);
      len = pos_len;
    }

  pos_len = len;
#if ASM86
  put_status(v_base + 5, buf, len);
#else  /* if ASM86 */
  (void)move(STATUS_LINE, 5);
  put_seq(buf, len, status_col);
#endif  /* if ASM86 */

  for (len = last_line; eor[len] == EOF; --len)
    {
      ;
    }

  len = ( vssizeof(in_u) - vstell(in_u) ) + sop + in_count
        + ( len - TEMPLATE_LINE );
  if (len != se_save_eof)
    {
      len = sprintf(buf, " EOF %d", se_save_eof = len);
      eof_start = COLS - len;
#if ASM86
      put_status(v_base + eof_start, buf, len);
#else  /* if ASM86  */
      (void)move(STATUS_LINE, eof_start);
      put_seq(buf, len, status_col);
#endif  /* if ASM86 */
    }

  new_eor = eof_start;
  if (eol != EOF)
    {
      new_eor -= ( len = sprintf(buf, "EOR %d", eol) );
#if ASM86
      put_status(v_base + new_eor, buf, len);
#else  /* if ASM86  */
      (void)move(STATUS_LINE, new_eor);
      put_seq(buf, len, status_col);
#endif  /* if ASM86 */
    }

  len = new_eor - eor_start;
#if DOS
  if (len > 0)
    {
      wfill(v_base + eor_start, norm_space, len);
    }

  eor_start = new_eor;
  v_base[h_inc] = ( expand ? 'I' : 'O' ) | status_col;
#else  /* if DOS */
  if (len > 0)
    {
      (void)move(STATUS_LINE, eor_start);
      while (len--)
        {
          (void)put_byte(SPACE);
        }
    }

  eor_start = new_eor;
  (void)mv_put_byte(STATUS_LINE, h_inc, expand ? 'I' : 'O');
  (void)attrset(norm_col);
#endif  /* if DOS */
}

/*
 *  Set up the screen.
 */

private
void
init(void)
{
  int len;

  /* Setup fixed parts of status line */

#if ASM86
  chtype *const mid = v_base + h_inc + 4;
  const char *p, *last;
  scr_fill(v_base, B_COLS);
  put_status(v_base, "LINE", 4);
#else  /* if ASM86  */
  string buf;
  char *const mid = buf + h_inc + 4;
  const char *p, *last;
  space_fill(buf, COLS);
  (void)movelr(buf, "LINE", 4);
#endif  /* if ASM86 */
  se_save_eof = -1;

  /* extract basenames */

  p = last = get_eos(in_fname);
  while ( p > in_fname && !path_sep(p[-1]) )
    {
      --p;
    }
  if (p == last)
    {
      p = out_fname;
    }

  if ( ( len = last - p ) > 15)
    {
      len = 15;
    }

  (void)put_status(mid - 8 - len, p, len);

  p = last = get_eos(out_fname);

  while ( p > out_fname && !path_sep(p[-1]) )
    {
      --p;
    }

  if (p == last)
    {
      p = out_fname;
    }

  if ( ( len = last - p ) > 15 )
    {
      len = 15;
    }

  (void)put_status(mid, p, len);

  (void)put_status(mid - 6, ">GI>", 4);

#if ASM86 == 0
  (void)move(STATUS_LINE, 0);
  put_seq(buf, COLS, status_col);
#endif  /* if ASM86 == 0 */

  ++fscreen;
  status();
  disp_home();
  disp_template();
  disp_text();
  (void)move(row, col);
  redisplay = NO;
}

/*
 *  Restart editing on a line.  Analagous to Wrapround.
 */

private
void
linewrap(void)
{
  if (e_col + i_col)
    {
      char *const t = e_buff;

      /* deal with T.#0 */

      get_end();
      e_buff = i_buff;
      i_buff = t;
      i_eor  = e_col;
      e_col  = i_col = 0;
      loc2   = NULL;
    }
}

/*
 *  Execute context editor commands.
 */

private
int
home_command(const int disp, const int s_start)
{
  const int sop = START_OF_PAGE;
  int rc = 0;

  buf_to_file(SE_LEAVE);
  (void)move_to(s_start);

  lon = disp == D_SE_HOME;

  noraw();
  
  if ( setjmp(set_err) )
    {
      (void)move_to(sop);
      rc = 1;
    }
  else
    {
      Drive(disp);
    }

  raw();

  save_jbuf(set_err, save_err);

  if (f_list != NULL)
    {
      term();
      g_err(SM_SE, cmd_buf);
    }

  lon = NO;

  if (!g_eof)
    {
      linewrap();
    }

  file_to_buf(SE_ENTER);

  if (redisplay)
    {
      if (redisplay == SE_WAIT)
        {
          wait_user();
        }

      init();
    }
  else
    {
      int m_row, m_col = -1;
      if (loc2 != NULL)
        {
          m_row = l2rec - START_OF_PAGE + FIRST_LINE;
          if (m_row >= FIRST_LINE && \
              m_row <= last_line && eor[m_row] != EOF)
            {
              const int c_col = col, c_offset = offset;
              set_col( (int)( loc1 - i_buff ) );
              if (offset != c_offset)
                {
                  disp_template();
                  last_offset = offset;  /* stop exec drawing it again */
                }

              m_col = col;
              if (row == COMMAND_LINE)
                {
                  col = c_col;
                }
            }
        }

      disp_text();
      if (m_col >= 0)
        {
          disp_matched_text( m_row, m_col, (int)( loc2 - loc1 ) );
        }

      loc2 = NULL;
    }

  return rc;
}

/*
 *  Execute a home command without disturbing anything the user has typed in.
 *  s_rec:  if true ends command with T#sop else ends with T-cursor_row
 *  f_rec:  is the line the command commences on (+FILE_LINE)
 */

private
int
run_command(char csc com, const int f_rec, const int s_rec)
{
  string cline;

  if (s_rec)
    {
      (void)sprintf(cline, "%s,T#%d", com, START_OF_PAGE);
    }
  else
    {
      (void)sprintf(cline, "%s,T%d", com, -CURSOR_ROW);
    }

  cmd_buf = cline;

  return home_command(D_SE_AUTO, HFILE_LINE + f_rec);
}

/*
 *  Decode key sequences.
 */

private
int
get_key2(int *const value)
{
  int c = getch();

  if (c <= 0xFF)
    {
      if ( ( c = u_map[c] ) < SPACE )
        {
          c += 0x40;
        }
    }

  *value = c;
  return c;
}

/*
 *  Wordstar "Quick" keys (start with ^Q)
 */

private
ACTION
hand_quick(int *const value)
{
  switch ( get_key2(value) )
    {
    case DEL:  /* ^Q DEL  Erase to start of line */
    case KEY_DC:
      *value = LEFT;
      return A_DEL_REST;

    case 'A':  /* ^QA  Search for string and replace */
      /* fallthrough */
    case 'F':  /* ^QF  Search for string */
      /* fallthrough */
    case 'I':  /* ^QI  Move to line number */
      return A_SEARCH;

    case 'B':  /* ^QB  Justify and move on */
      *value = 'Q';
      return A_JUSTIFY;

    case 'C':  /* ^QC  Move to end of file */
      *value = MOVE_EOF;
      return A_FILE_MOVE;

    case 'D':  /* ^QD  Cursor to end of line */
      return A_C_EOL;

    case 'E':  /* ^QE  Cursor to top of screen */
      return A_C_TOS;

    case 'G':  /* ^QGc Find character */
#if DOS
      *value = getch();
#else  /* if DOS */
      if ( ( *value = getch() ) == KEY_ENTER )
        {
          *value = CNTRL('M');
        }

#endif  /* if DOS */
      return A_FINDC;

    case 'H':  /* ^QH  Extension: Cursor Home */
      *value = 'Q';
      return A_C_HOME;

    case 'J':  /* ^QJ  Display help text */
      return A_HELP;

    case 'K':  /* ^QK  Retrieve last history event */
      return A_HIST;

    case 'Q':  /* ^QQ  Repeat next command */
      *value = 0;
      return A_REPEAT;

    case 'V':  /* ^QV  Match brackets & strings */
      *value = -1;
      return A_FINDC;

    case 'W':  /* ^QW  Fast repeat scroll up */
      *value = PREV_LINE;
      return A_REPEAT;

    case 'Z':  /* ^QZ  Fast repeat scroll down */
      *value = NEXT_LINE;
      return A_REPEAT;

    case 'R':  /* ^QR  Move to top of file */
      *value = MOVE_TOF;
      return A_FILE_MOVE;

    case 'S':  /* ^QS  Cursor to start of line */
      return A_C_SOL;

    case 'X':  /* ^QX  Cursor to bottom of screen */
      return A_C_BOS;

    case 'Y':  /* ^QY  Erase to end of line */
      *value = RIGHT;
      return A_DEL_REST;
    }

  return A_C_STAY;
}

/*
 *  Wordstar file & block keys (start with ^K)
 */

private
ACTION
hand_block(int *const value)
{
  const int c = get_key2(value);

  if ( se_b1key(c) )
    {
      return A_RWX_FILE;
    }

  if (strchr("CBK", c) != NULL)
    {
      return A_BLOCK;
    }

  if (c == 'H')
    {
      return A_C_HOME;
    }

  if (strchr("QXSD", c) != NULL)
    {
      return A_EXIT_EDITOR;
    }

  if ( se_b4key(c) )
    {
      return A_MISC_CE;
    }

  if (c == 'Z')
    {
      return A_REDRAW;
    }

  return A_C_STAY;
}

private
ACTION
get_seq(int *const value)
{
  const int c = rgetc();

  /* Single WordStar keys */

  switch (c)
    {
    case CNTRL('A'):  /* ^A  Word move left */
      *value = NO;
      return A_W_LEFT;

    case CNTRL('B'):  /* ^B  Word delete left */
      *value = YES;
      return A_W_LEFT;

    case CNTRL('C'):  /* ^C  Page down */
      /* fallthrough */
    case KEY_NPAGE:
      /* fallthrough */
#ifdef KEY_NEXT
    case KEY_NEXT:
      /* fallthrough */
#endif  /* ifdef KEY_NEXT */
      *value = NEXT_PAGE;
      return A_FILE_MOVE;

    case CNTRL('D'):  /* ^D  Cursor right */
      /* fallthrough */
    case KEY_RIGHT:
      return A_C_RIGHT;

    case CNTRL('E'):  /* ^E  Cursor up */
      /* fallthrough */
    case KEY_UP:
      return A_C_UP;

    case CNTRL('F'):  /* ^F  Word move right */
      *value = NO;
      return A_W_RIGHT;

    case CNTRL('G'):  /* ^G  Char delete right */
      /* fallthrough */
    case KEY_DC:
      /* fallthrough */
    case DEL:
      *value = RIGHT;
      return A_DEL_C;

    case CNTRL('H'):  /* ^H  BS as char delete left */
      /* fallthrough */
#ifdef KEY_BACKSPACE
    case KEY_BACKSPACE:
      /* fallthrough */
#endif  /* ifdef KEY_BACKSPACE */
      *value = LEFT;
      return A_DEL_C;

    case CNTRL('I'):  /* ^I  Horizontal tab */
      return A_H_TAB;

    case CNTRL('J'):  /* ^J  Save line on delete stack */
      return A_YANK;

    case CNTRL('L'):  /* ^L  Repeat search / replace */
      *value = 'L';
      return A_SEARCH;

    case CNTRL('P'):  /* ^Pc Enter control character */
      *value = CNTRL( getch() );
      return A_CHARACTER;

    case CNTRL('M'):  /* ^M  RETURN (split line) */
      /* fallthrough */
#ifdef KEY_ENTER
    case KEY_ENTER:
      /* fallthrough */
#endif  /* ifdef KEY_ENTER */
      return A_C_RETURN;

    case CNTRL('N'):  /* ^N  Open blank line */
      /* fallthrough */
#ifdef KEY_IL
    case KEY_IL:
      /* fallthrough */
#endif  /* ifdef KEY_IL */
      return A_OPEN_LINE;

    case CNTRL('R'):  /* ^R  Page up */
      /* fallthrough */
    case KEY_PPAGE:
      /* fallthrough */
#ifdef KEY_PREVIOUS
    case KEY_PREVIOUS:
      /* fallthrough */
#endif  /* ifdef KEY_PREVIOUS */
      *value = PREV_PAGE;
      return A_FILE_MOVE;

    case CNTRL('S'):  /* ^S  Cursor left */
      /* fallthrough */
    case KEY_LEFT:
      return A_C_LEFT;

    case CNTRL('T'):  /* ^T  Word delete right */
      *value = YES;
      return A_W_RIGHT;

    case CNTRL('U'):  /* ^U  Restore line */
      return A_REST_LINE;

    case CNTRL('V'):  /* ^V  Toggle expand mode */
      /* fallthrough */
    case KEY_IC:
      return A_EXP_MODE;

    case CNTRL('W'):  /* ^W  Scroll up one line */
      *value = PREV_LINE;
      return A_FILE_MOVE;

    case CNTRL('X'):  /* ^X  Cursor down */
      /* fallthrough */
    case KEY_DOWN:
      return A_C_DOWN;

    case CNTRL('Y'):  /* ^Y  Line delete */
      /* fallthrough */
#ifdef KEY_DL
    case KEY_DL:
#endif  /* ifdef KEY_DL */
      return A_DEL_LINE;

    case CNTRL('Z'):  /* ^Z  Scroll down one line */
      *value = NEXT_LINE;
      return A_FILE_MOVE;

#ifdef KEY_END
    case KEY_END:  /* Cursor to end of line */
      /* fallthrough */
#endif  /* ifdef KEY_END */
#ifdef KEY_C1
    case KEY_C1:
      /* fallthrough */
#endif  /* ifdef KEY_C1 */
#ifdef KEY_LL
    case KEY_LL:
      /* fallthrough */
#endif  /* ifdef KEY_LL */
      return A_C_EOL;

#ifdef KEY_BTAB
    case KEY_BTAB:  /* Back tab */
      return A_B_TAB;

#endif  /* ifdef KEY_BTAB */
    case KEY_F(1):
      return A_HELP;

    case KEY_F(2):  /* ^KB  Start block */
      *value = 'B';
      return A_BLOCK;

    case KEY_F(3):  /* ^KK  End Block */
      *value = 'K';
      return A_BLOCK;

    case KEY_F(4):  /* ^KC  Copy block */
      *value = 'C';
      return A_BLOCK;

    case KEY_F(5):  /* ^KR  Read block */
      *value = 'R';
      return A_RWX_FILE;

    case KEY_F(6):  /* ^KW  Write block */
      *value = 'W';
      return A_RWX_FILE;

    case KEY_F(7):  /* ^KQ  Exit GE without saving file */
      /* fallthrough */
#if DOS
    case ESC:
#endif  /* if DOS */
      *value = 'Q';
      return A_EXIT_EDITOR;

    case KEY_F(8):  /* ^KX  Exit GE saving file */
      *value = 'X';
      return A_EXIT_EDITOR;

    case KEY_F(9):  /* ^QK  Retreive last history event */
      return A_HIST;

    case KEY_F(10):  /* ^KS  Checkpoint save, stay in SE */
      *value = 'S';
      return A_EXIT_EDITOR;

    case KEY_HOME:  /* Home Key */
      *value = 'Q';
      return A_C_HOME;

#ifdef KEY_SLEFT
    case KEY_SLEFT:  /* Page left */
      *value = LEFT;
      return A_PAGE_SHIFT;

    case KEY_SRIGHT:  /* Page Right */
      *value = RIGHT;
      return A_PAGE_SHIFT;

#endif  /* ifdef KEY_SLEFT */

    /* Wordstar "Quick" keys */

    case CNTRL('Q'):
      return hand_quick(value);

    /* Wordstar Justification keys */

    case CNTRL('O'):
      if ( se_jkey( get_key2(value) ) )
        {
          return A_JUSTIFY;
        }

      return A_C_STAY;

    /* Wordstar file & block keys */

    case CNTRL('K'):
      return hand_block(value);

    default:
      if ( isprint(c) )
        {
          *value = c;  /* Ordinary printing character */
          return A_CHARACTER;
        }

      return A_C_STAY;
    }
}

/*
 *  Scroll the text upwards on the screen and in the buffer.
 */

private
void
scroll_up(const int line)
{
  char *const p = s_buf[line];
  const short e = s_eor[line];
  const int dist = last_line - line;

  (void)move(line, 0);
  se_deleteln();
  (void)bmovelr(
          &s_buf[line], &s_buf[line + 1],
          dist * ( sizeof ( char * ) >> B_SIZE ) );
  (void)wmovelr(&s_eor[line], &s_eor[line + 1], dist);
  (void)wmovelr(&eor[line], &eor[line + 1], dist);
  s_buf[last_line] = p;
  s_eor[last_line] = e;
}

/*
 *  Scroll the text downwards on the screen and in the buffer.
 */

private
void
scroll_down(const int line)
{
  char *const p = s_buf[last_line];
  const short e = s_eor[last_line];
  const int dist = last_line - line;

  se_insertln();
  (void)bmoverl(
          &s_buf[line + 1], &s_buf[line],
          dist * ( sizeof ( char * ) >> B_SIZE ) );
  (void)wmoverl(&s_eor[line + 1], &s_eor[line], dist);
  (void)wmoverl(&eor[line + 1], &eor[line], dist);
  s_buf[line] = p;
  s_eor[line] = e;
}

/*
 *  Window movements.
 *  Should be 20 lines for NEXT & PREV PAGE.
 */

private
void
file_move(int value)
{
  const int sop = START_OF_PAGE;
  int dest;

  if (value <= 0)
    {
      dest = -value;
      value = MOVE_ABS;
    }

  /* Optimizations */

  switch (value)
    {
    case NEXT_LINE:
      if (eor[FIRST_LINE] == EOF)
        {
          return;
        }

      break;

    case NEXT_PAGE:
      if (eor[last_line] == EOF)
        {
          return;
        }

      break;

    case MOVE_EOF:
      if (eor[last_line] == EOF)
        {
          for (row = last_line; row > FIRST_LINE && eor[row] == EOF; --row)
            {
              ;
            }

          c_eol();
          return;
        }

      break;

    case MOVE_TOF:
      row = FIRST_LINE;
      c_sol();
      /* fallthrough */ 

    case PREV_LINE:
      /* fallthrough */
    case PREV_PAGE:
      if (sop == 0)
        {
          return;
        }
    }

  buf_to_file(value);

  /* Call CE to reposition in file */

  switch (value)
    {
    case NEXT_LINE:
      scroll_up(FIRST_LINE);
      (void)next_line(last_line);
      disp_row(last_line);
      return;

    case PREV_LINE:
      (void)move_to(sop - 1);
      (void)move(FIRST_LINE, 0);
      se_insertln();
      break;

    case PREV_PAGE:
      (void)move_to(sop - text_lines);  /* Text lines - 2 in future */
      break;

    case MOVE_TOF:
      (void)wrapround();
      break;

    case MOVE_EOF:
      (void)move_to(vssizeof(in_u) - ( LINES - MATCH_LINE ) );
      break;

    case MOVE_ABS:
      (void)move_to(dest);
    }

  file_to_buf(value);

  /* final cursor positioning */

  if (value == MOVE_EOF)
    {
      for (row = last_line; eor[row - 1] == EOF; --row)
        {
          ;
        }

      c_sol();
    }

  /* Redisplay text */

  if (value == PREV_LINE)
    {
      disp_row(FIRST_LINE);
    }
  else
    {
      disp_text();
    }
}

/*
 *  Cursor to home position (start of command line).
 */

private
void
c_home(const int value)
{
  if (row == COMMAND_LINE)
    {
      row = text_row;
      set_col(text_col + text_offset);
    }
  else
    {
      text_row = row, text_col = col, text_offset = offset;
      if (value == 'H')
        {
          file_move(-FILE_LINE);
        }

      col = eor[row = COMMAND_LINE], offset = 0;
      while (col > 0 && s_buf[COMMAND_LINE][col - 1] == SPACE)
        {
          --col;
        }
      eor[COMMAND_LINE] = (short)col;
      set_col(col);
    }
}

/*
 *  Cursor up one line.
 *  If the cursor is on the command line, scroll the screen up.
 */

private
void
c_up(void)
{
  if (row <= FIRST_LINE)
    {
      if (START_OF_PAGE > 0)
        {
          file_move(PREV_LINE);
        }
    }
  else
    {
      --row;
    }
}

/*
 *  Cursor down one line.
 */

private
void
c_down(void)
{
  if (row == COMMAND_LINE)
    {
      text_row = FIRST_LINE, text_col = col;
      c_home('Q');  /* flip to text area and save cmdline coords */
    }

  elif(row == last_line)
    {
      if (eor[MATCH_LINE] != EOF)
        {
          file_move(NEXT_LINE);
        }
    }
  else
    {
      ++row;
    }
}

/*
 *  Delete all of line.
 */

private
void
del_line(void)
{
  if (eor[row] == EOF)
    {
      return;
    }

  scroll_up(row);
  (void)next_line(last_line);
  disp_row(last_line);
}

/*
 *  Join the next line to the end of the current one.
 */

private
int
se_join(const int del)
{
  int len, nrow = row + 1;
  const int j_col = FILE_COL, o_eor = eor[row];
  char *bs, *nbs;

  if (row == COMMAND_LINE)
    {
      return YES;
    }

  if (row == last_line)
    {
      (void)next_line(TEMPLATE_LINE);
      nrow = TEMPLATE_LINE;
    }

  if ( ( len = eor[nrow] ) == EOF )
    {
      return YES;
    }

  nbs = bs = s_buf[nrow];

  if (del)
    {
      while ( len && isspace(*nbs) )
        {
          --len, ++nbs;
        }
    }

  if (j_col == 0 && bs == nbs)
    {
      (void)del_line();
      return NO;
    }

  if ( set_eor(row, j_col + len) )
    {
      if (row == last_line)
        {
          push_line(&in_stack, s_buf[TEMPLATE_LINE], len);
        }

      return YES;
    }

  bs = BUF(0);
  if (j_col > o_eor)
    {
      space_fill(bs + o_eor, j_col - o_eor);
    }

  (void)movelr(bs + j_col, nbs, len);

  disp_rest();
  refresh();

  if (nrow != TEMPLATE_LINE)
    {
      ++row;
      (void)del_line();
      --row;
    }

  return NO;
}

/*
 *  Check for start of word.
 */

private
int
is_sow(char csc p)
{
  const char c = *p;

  if ( p == BUF(0) )
    {
      return wordch(c) || punctch(c);
    }

  return wordch(c) && !wordch(p[-1]) || punctch(c) && !punctch(p[-1]);
}

/*
 *  Find something forwards in text.
 */

private
int
find_forwards(char csc target, const int del)
{
  const int j_col = FILE_COL;
  const char *p = BUF(j_col + 1), *e;

  while (eor[row] != EOF)
    {  /* look on this line */
      for (e = BUF(eor[row]); p < e; ++p)
        {
          if (target != NULL)
            {
              if (strchr(target, *p) != NULL)
                {
                  break;
                }
            }

          elif( is_sow(p) )
              break;
        }

      if (p < e)  /* found */
        {
          char csc s = BUF(0);
          if (del)
            {
              del_seg( (int)( p - s ) );
            }
          else
            {
              set_col( (int)( p - s ) );
            }

          return *p;
        }

      if (del)
        {
          if (se_join(YES) || j_col < eor[row])
            {
              break;
            }

          p = BUF(j_col);
        }
      else
        {  /* move to next line */
          if (row == COMMAND_LINE || row == last_line && target != NULL)
            {
              break;
            }

          c_down();
          c_sol();
          p = BUF(0);
        }
    }

  return EOF;
}

/*
 *  Find something backwards in the text.
 */

private
int
find_backwards(char csc target, const int del)
{
  char *p, *e;
  int end_col, j_col;

  repeat
  {
    j_col = --col + offset;
    if (j_col >= eor[row])
      {
        set_col(eor[row] - 1);
      }

    end_col = j_col + 1;
    if (eor[row] > 0)
      {  /* look on this line */
        for (e = BUF(0), p = e + j_col; p >= e; --p)
          {
            if (target != NULL)
              {
                if (strchr(target, *p) != NULL)
                  {
                    break;
                  }
              }

            elif( is_sow(p) )
                break;
          }

        if (p >= e)  /* found */
          {
            set_col( (int)( p - e ) );
            if (del)
              {
                del_seg(end_col);
              }

            return *p;
          }
      }

    if (row == COMMAND_LINE || row == FIRST_LINE && target != NULL)
      {
        break;
      }

    if (del)  /* move to previous line */
      {
        c_sol();
        del_seg(end_col);
      }

    if (FILE_LINE <= 0)
      {
        break;
      }

    c_up();
    c_eol();
    if (del)
      {
        (void)se_join(NO);
      }
  }

  c_sol();
  return EOF;
}

/*
 *  Find character sequences.
 */

private
void
find_char(const int value)
{
  char target, find_str[3];
  int forwards = YES, count, found;

  if ( value == CNTRL('M') )
    {
      if (row != COMMAND_LINE)
        {
          while (eor[row] > 0)
            {
              c_down();
            }
        }

      return;
    }

  if (value > 0)
    {
      find_str[0] = (char)value;
      find_str[1] = EOS;
      if (find_forwards(find_str, NO) == EOF)
        {
          message("Character not found on screen");
        }

      return;
    }

  target = find_str[0] = *BUF(col + offset);

  switch (target)
    {
    case '\'':
      /* fallthrough */
    case '"':
      /* fallthrough */
    case '`':
      /* fallthrough */
    case '/':
      find_str[1] = EOS;
      while (find_forwards(find_str, NO) != EOF)
        {
          if (FILE_COL > 0 && *BUF(FILE_COL - 1) != BSLASH)
            {
              break;
            }
        }
      return;

    default:
      (void)find_forwards(endsent, NO);
      return;

    case '(':
      /* fallthrough */
    case '{':
      /* fallthrough */
    case '[':
      /* fallthrough */
    case '<':
      find_str[1] = xlat(target, bra_start, (const byte *)bra_end);
      break;

    case ')':
      /* fallthrough */
    case '}':
      /* fallthrough */
    case ']':
      /* fallthrough */
    case '>':
      find_str[1] = xlat(target, bra_end, (const byte *)bra_start);
      forwards = NO;
    }

  find_str[2] = EOS;
  count = 1;
  while (count > 0)
    {
      if (forwards)
        {
          found = find_forwards(find_str, NO);
        }
      else
        {
          found = find_backwards(find_str, NO);
        }

      if (found == EOF)
        {
          break;
        }

      if (found == find_str[1])
        {
          if (--count == 0)
            {
              break;
            }
        }
      else
        {
          ++count;
        }
    }
}

#if UNIX
private
int std_saved = NO, save_fd_0 = -1, save_fd_2 = -1, null_fd = -1;

/*
 *  Save and redirect the standard files
 */

private
void
save_std(void)
{
  if (null_fd == -1)
    {
      null_fd = open("/dev/null", O_RDWR);
    }

  if ( ( save_fd_0 = dup(0) ) != -1 )
    {
      (void)dup2(null_fd, 0);
    }

  if ( ( save_fd_2 = dup(2) ) != -1 )
    {
      (void)dup2(null_fd, 2);
    }

  ++std_saved;
}

private
void
restore_std(void)
{
  if (std_saved)
    {
      (void)dup2(save_fd_0, 0);
      (void)close(save_fd_0);
      (void)dup2(save_fd_2, 2);
      (void)close(save_fd_2);
      std_saved = NO;
    }
}

#endif  /* if UNIX */

/*
 *  Clear down the screen and return to normal attributes
 */

private
void
term(void)
{
  if (!fscreen)
    {
      return;
    }

  (void)erase();
  refresh();
  fscreen = NO;
  (void)endwin();
  lon = redisplay = SE_WAIT;
}

/*
 *  Print help text screens.
 */

private
void
#if TINY_G
Help(void)
{
  char csc *const text = hw_mess;
#else  /* if TINY_G */
Help(VERB csc opts)
{
  char csc *const text = help_tab[opts->o1.q];

#endif  /* if TINY_G */
  int line = 0;

  term();
  do
    {
      new_line();
      say(text[line++]);
      wait_user();
      new_line();
    }
  while ( text[line] != NULL );
  redisplay = SE_DISP;
}

/*
 *  Clear the screen, display message in centre, redraw the screen.
 */

private
void
wmessage(const char *text)
{
  const int max_len = COLS - 8;
  int i, ymax, xmax, lines, cols, begy, begx, longest = 26;
  const char *p, *start;

  for (lines = 5, start = p = text; *p; ++lines, start = p + 1)
    {
      if ( ( p = strchr(start, LFEED) ) == NULL )
        {
          p = get_eos(start);
        }

      if (p - start > longest)
        {
          longest = p - start;
        }
    }

  if (longest > max_len)
    {
      longest = max_len;
    }

  cols = longest + 8;
  begx = ( COLS  - cols  ) / 2;
  begy = ( LINES - lines ) / 2;

  attrset(cntrl_col);

  ymax = lines - 1, xmax = cols - 1;

  for (i = 1; i <= xmax - 1; ++i)
    {
      (void)mv_put_byte(begy, i + begx, ACS_HLINE);
      (void)mv_put_byte(ymax + begy, i + begx, ACS_HLINE);
    }

  for (i = 1; i <= ymax - 1; ++i)
    {
      (void)mv_put_byte(i + begy, begx, ACS_VLINE);
      (void)mv_put_byte(i + begy, xmax + begx, ACS_VLINE);
    }

  (void)mv_put_byte(begy, begx, ACS_ULCORNER);
  (void)mv_put_byte(begy, xmax + begx, ACS_URCORNER);
  (void)mv_put_byte(ymax + begy, begx, ACS_LLCORNER);
  (void)mv_put_byte(ymax + begy, xmax + begx, ACS_LRCORNER);

  for (i = 1; i < lines - 1; ++i)
    {
      int j;
      (void)move(begy + i, begx + 1);
      for (j = 0; j < cols - 2; ++j)
        {
          (void)put_byte(SPACE);
        }
    }

  for (i = 2, start = p = text; *p; ++i, ++start)
    {
      if ( ( p = strchr(start, LFEED) ) == NULL )
        {
          p = get_eos(start);
        }

      (void)move(i + begy, 4 + begx);
      while (start < p)
        {
          (void)put_byte(*start++);
        }
    }

  (void)move(i + 1 + begy, 4 + begx);
  for (p = se_hit + 2; *p; ++p)
    {
      (void)put_byte(*p);
    }

  (void)rgetc();

  attrset(norm_col);
}

private
void
message(char csc text)
{
  wmessage(text);
  disp_text();
}

/*
 *  Inform user in whatever mode.
 */

private
void
inform(char csc mess)
{
  if (fscreen)
    {
      raw();
#if UNIX
      restore_std();
#endif  /* if UNIX */
      wmessage(mess);
    }
  else
    {
      new_line();
      say(mess);
      new_line();
    }
}

/*
 *  Error handler for screen editor.
 */

private
void
se_error(const int code)
{
  string buf;

  err_print(buf, code, "");
  message(buf);
  disp_home();
  longjmp(se_err, YES);
}

/*
 *  Shift screen right one increment.
 */

private
int
shift_right(void)
{
  if (offset + h_inc + last_col > E_BUFF_LEN)
    {
      return NO;
    }

  offset += h_inc;
  col -= h_inc;
  return YES;
}

/*
 *  Shift screen left one increment.
 */

private
int
shift_left(void)
{
  if (offset - h_inc < 0)
    {
      return NO;
    }

  offset -= h_inc;
  col += h_inc;
  return YES;
}

/*
 *  Position EOF on the screen.
 */

private
void
set_eof(void)
{
  int i;

  if (row == COMMAND_LINE)
    {
      return;
    }

  for (i = row - 1; eor[i] == EOF; --i)
    {
      eor[i] = 0;
    }

  (void)move(i + 1, 0);
  (void)clrtoeol();
  if (row < last_line)
    {
      disp_eof(row + 1);
    }

  (void)move(row, col);
}

/*
 *  Page shift left or right.
 */

private
void
page_shift(const int value)
{
  const int c_col = col;

  if (value == RIGHT)
    {
      col += h_inc;
      if ( !( shift_right() ) )
        {
          col = c_col;
        }
    }
  else
    {
      col -= h_inc;
      if ( !( shift_left() ) )
        {
          col = c_col;
        }
    }
}

/*
 *  Line segment operations.
 */

private
void
block(const int value)
{
  static int save_b_len = 0, start_col, end_col, save_len = EOF,
    start_rec = EOF, end_rec = EOF, block_mode = NO;
  static char *save_buf;
  const int b_col = FILE_COL, c_eor = eor[row], sop = START_OF_PAGE,
    c_line = sop + ( row - FIRST_LINE );
  char *a, com[32];
  int start, len, end_row;

  if (value != 'C')
    {
      if (row == COMMAND_LINE || c_eor == EOF && eor[row - 1] == EOF)
        {
          return;
        }

      disp_home();
      disp_text();
    }

  switch (value)
    {
    case 'B':
      start_rec = c_line;
      start_col = b_col;
      if (end_rec != c_line && save_len != EOF && !block_mode)
        {
          end_rec = EOF;
        }

      if (end_rec == EOF)
        {
          return;
        }

      break;

    case 'K':
      end_rec = c_line;
      end_col = b_col;
      if (start_rec == EOF)
        {
          return;
        }

      break;

    case 'C':
      if (save_len == EOF)
        {
          message("No saved block");
          return;
        }

      start_rec = end_rec = EOF;
      if (block_mode)
        {
          if (row != COMMAND_LINE)
            {
              (void)sprintf(com, "T#%d, m te x", c_line);
              (void)run_command(com, -c_line, YES);
            }

          return;
        }

      if ( set_eor(row, ( c_eor > b_col ? c_eor : b_col ) + save_len) )
        {
          return;
        }

      a = BUF(b_col);
      if (b_col >= c_eor)
        {
          if (c_eor == EOF)
            {
              set_eof();
            }

          space_fill(BUF(c_eor), b_col - c_eor);
        }
      else
        {
          (void)moverl(a + save_len, a, c_eor - b_col);
        }

      (void)movelr(a, save_buf, save_len);
      disp_rest();
      return;
    }

  /* deal with block rather than segment definition */

  if (start_rec != end_rec)
    {
      if (end_rec < start_rec)
        {
          len = end_rec;
          end_rec = start_rec;
          start_rec = len;
        }

      save_len = end_rec - start_rec;
      block_mode = YES;

      (void)sprintf(com, "T#%d S T#%d X", start_rec, end_rec);
      (void)run_command(com, -c_line, YES);

      if (start_rec < sop)
        {
          start = FIRST_LINE;
        }
      else
        {
          start = start_rec - sop + FIRST_LINE;
        }

      if (end_rec > sop + text_lines)
        {
          end_row = LINES;
        }
      else
        {
          end_row = end_rec - sop + FIRST_LINE;
        }

      for (; start < end_row; ++start)
        {
          len = eor[start] - offset;
          if (len > 0)
            {
              disp_matched_text(start, 0, len);
            }
          else
#if DOS
            {
              v_base[start * COLS] = SPACE | found_col;
            }

#else  /* if DOS */
            {
              (void)attrset(found_col);
              (void)mv_put_byte(start, 0, SPACE);
            }
#endif  /* if DOS */
        }

      return;
    }

  block_mode = NO;

  /* swap if defined in wrong order */

  if (end_col < start_col)
    {
      len = end_col;
      end_col = start_col;
      start_col = len;
    }

  save_len = end_col - start_col;

  if (save_b_len < save_len)
    {
      rlsevec(save_buf);
      save_buf = getbuf(save_b_len = save_len);
    }

  if (end_col > c_eor)
    {
      (void)set_eor(row, end_col);
      eor[row] = (short)c_eor;
      space_fill(BUF(c_eor), end_col - c_eor);
    }

  (void)movelr(save_buf, BUF(start_col), save_len);

  len = start_col - offset;
  if (len > 0)
    {
      start = len;
      len = save_len;
    }
  else
    {
      len += save_len;
      start = 0;
    }

  disp_matched_text(row, start, len);
}

/*
 *  Centre the current line within the margins.
 */

private
void
centre(void)
{
  char *sol, *ls, *le;
  int dif, l_cen, m_cen, f_char, len;

  if (eor[row] == EOF)
    {
      return;
    }

  /* find start and end of line */

  ls = sol = BUF(0);
  le = ls + eor[row];

  while ( ls < le && isspace(*ls) )
    {
      ++ls;
    }
  while ( le > sol && isspace(le[-1]) )
    {
      --le;
    }
  if (ls >= le)  /* line empty or all spaces */
    {
      return;
    }

  /* find centre's of margins and line */

  m_cen = l_margin + ( r_margin - l_margin ) / 2;
  l_cen = ( ls - sol ) + ( le - ls ) / 2;

  /* distance to shift line */

  dif = m_cen - l_cen;
  f_char = ls - sol;

  /* check limits at each end of line */

  if (dif > 0)
    {
      if (E_BUFF_LEN - dif < le - sol)
        {
          dif = E_BUFF_LEN - ( le - sol );
        }
    }

  elif(f_char + dif < 0)
      dif = -f_char;

  if ( dif == 0 || set_eor(row, ( le - sol ) + dif) )
    {
      return;
    }

  len = le - ls;

  /* align centres */

  if (dif > 0)
    {
      ls = BUF(f_char);
      (void)moverl(ls + dif, ls, len);
      space_fill(ls, dif);
    }
  else
    {
      (void)movelr(ls + dif, ls, len);
    }

  /* redisplay the current line */

  disp_line();
}

/*
 *  Display the margins.
 */

private
void
disp_margins(void)
{
  int m;

  disp_template();
#if DOS == 0
  (void)attrset(marg_col);
#endif  /* if DOS == 0 */
  m = l_margin - offset;
  if (m >= 0 && m < COLS)
    {
#if DOS
      t_base[m] = '<' | marg_col;
#else  /* if DOS */
      (void)mv_put_byte(TEMPLATE_LINE, m, '<');
#endif  /* if DOS */
    }

  m = r_margin - offset;
  if (m >= 0 && m < COLS)
    {
#if DOS
      t_base[m] = '>' | marg_col;
#else  /* if DOS */
      (void)mv_put_byte(TEMPLATE_LINE, m, '>');
#endif  /* if DOS */
    }
}

/*
 *  Cursor left one character.
 */

private
void
c_left(void)
{
  if (FILE_COL > 0)
    {
      set_col(FILE_COL - 1);
    }

  elif(row != COMMAND_LINE && FILE_LINE > 0)
    {
      c_up();
      c_eol();
    }
}

/*
 *  Open a blank line.
 */

private
void
open_line(void)
{
  if (row == COMMAND_LINE)
    {
      return;
    }

  if (eor[row] != EOF)
    {
      if (row == last_line)
        {
          push_line(&in_stack, BUF(0), eor[row]);
          (void)move(row, 0);
          (void)clrtoeol();
        }
      else
        {
          if (eor[last_line] != EOF)
            {
              push_line(&in_stack, s_buf[last_line], eor[last_line]);
            }

          scroll_down(row);
        }
    }
  else
    {
      set_eof();
    }

  eor[row] = 0;
}

/*
 *  Split the current line at the cursor.
 */

private
void
split_line(const int margin)
{
  const int n_row = row + 1, s_col = FILE_COL;
  int len = eor[row] - s_col;
  char csc text = BUF(s_col);

  if (len < 0)
    {
      len = 0;
    }

  if (eor[row] == EOF)
    {
      set_eof();
      eor[row] = 0;
    }

  elif(s_col == 0)
      open_line();

  else
    {
      (void)clrtoeol();
      refresh();
      if (row == last_line)
        {
          push_line(&in_stack, text, len);
        }
      else
        {
          if (eor[last_line] != EOF)
            {
              push_line(&in_stack, s_buf[last_line], eor[last_line]);
            }

          (void)move(n_row, 0);
          scroll_down(n_row);
          (void)set_eor(n_row, len + margin);
          (void)movelr(mspace_fill(s_buf[n_row], margin), text, len);
          disp_row(n_row);
        }

      if (len > 0)
        {
          eor[row] = (short)s_col;
        }
    }

  c_down();
  set_col(margin);
}

/*
 *  Perform crude wordwrap.
 */

private
void
wrap_text(void)
{
  int ws = FILE_COL, we;
  char *const bs = BUF(0);

  while ( ws > 0 && !( isspace(bs[ws]) ) )
    {
      --ws;
    }
  we = ws;
  while ( ws < eor[row] && isspace(bs[ws]) )
    {
      ++ws;
    }
  while ( we > 0 && isspace(bs[we]) )
    {
      --we;
    }
  set_col(++we);
  (void)move(row, we);
  (void)movelr(bs + we, bs + ws, eor[row] - ws);
  eor[row] -= ws - we;
  split_line(l_margin);
}

/*
 *  A simple character.
 */

private
void
character(const int value)
{
  const int b_col = FILE_COL;
  int c_eor = eor[row];

#if DOS
  chtype *cb;
#endif  /* if DOS */

  /* past the end of the line, pad with spaces to here */

  if (b_col >= c_eor)
    {
      if ( set_eor(row, b_col + 1) )
        {
          return;
        }

      if (c_eor == EOF)
        {
          set_eof();
          c_eor = 0;
        }

      space_fill(BUF(c_eor), b_col - c_eor);
    }

  elif(expand)
    {
      char *bs;

      if ( set_eor(row, c_eor + 1) )
        {
          return;
        }

      /* adjust buffer */

      bs = BUF(b_col);
      (void)moverl(bs + 1, bs, c_eor - b_col);
      (void)insch(SPACE);
    }

  *BUF(b_col) = (char)value;

  if (wordwrap && row != COMMAND_LINE && b_col > r_margin)
    {
      wrap_text();
      set_col(eor[row]);
      return;
    }

  /* adjust screen */

#if DOS
  cb = v_base + row * COLS + col;
  if ( isprint(value) )
    {
      *cb = value | norm_col;
    }
  else
    {
      *cb = toprint(value) | cntrl_col;
    }

#else  /* if DOS */
  if ( isprint(value) )
    {
      (void)put_byte( (chtype)value );
    }
  else
    {
      (void)attrset(cntrl_col);
      (void)put_byte( toprint(value) );
      (void)attrset(norm_col);
    }

#endif  /* if DOS */

  c_right();
}

/*
 *  Save line on delete stack.
 */

private
void
yank(void)
{
  const int c_eor = eor[row];

  if (c_eor != EOF)
    {
      push_line(&del_stack, BUF(0), c_eor);
    }
}

/*
 *  Delete character (left or right)
 */

private
void
del_c(const int value)
{
  int b_col;

  if (value == LEFT)
    {
      const int c_line = FILE_LINE, c_col = col;
      c_left();
      if (c_line > FILE_LINE)
        {
          (void)se_join(NO);
          return;
        }

      if (c_col == col)
        {
          return;
        }
    }

  if ( ( b_col = FILE_COL ) >= eor[row] )
    {
      if (value == RIGHT)
        {
          (void)se_join(NO);
        }

      return;
    }

  if (expand)
    {
      del_seg(b_col + 1);
    }
  else
    {
      *BUF(b_col) = SPACE;
#if DOS
      v_base[row * COLS + col] = norm_space;
#else  /* if DOS */
      (void)mv_put_byte(row, col, SPACE);
#endif  /* if DOS */
    }
}

/*
 *  Restore deleted line.
 */

private
void
rest_line(void)
{
  if (del_stack == NULL)
    {
      return;
    }

  open_line();
  (void)set_eor( row, pop_length(del_stack) );
  pop_line( &del_stack, BUF(0) );
  disp_line();
}

/*
 *  Delete remainder of line.
 */

private
void
del_rest(const int value)
{
  int b_col = FILE_COL;
  const int c_eor = eor[row];

  if (value == RIGHT)
    {
      if (b_col >= c_eor)
        {
          return;
        }

      if (b_col == 0)
        {
          yank();
        }

      del_seg(c_eor);
    }
  else
    {
      if (b_col >= c_eor)
        {
          yank();
          b_col = c_eor;
        }

      c_sol();
      if (b_col > 0)
        {
          del_seg(b_col);
        }
    }
}

/*
 *  Horizontal tab.
 */

private
void
h_tab(void)
{
  const int c_col = col;

  col = ( col / screen_tabs + 1 ) * screen_tabs;
  if (col >= COLS && !shift_right() || FILE_COL > E_BUFF_LEN)
    {
      col = c_col;
    }
}

/*
 *  Back tab.
 */

private
void
b_tab(void)
{
  const int c_col = col;

  col -= col % screen_tabs ? col % screen_tabs : screen_tabs;
  if ( col < 0 && !( shift_left() ) )
    {
      col = c_col;
    }
}

/*
 *  Handle the return key.
 */

private
void
c_return(void)
{
  if (row == COMMAND_LINE)
    {
      const int t_len = eor[COMMAND_LINE];
      char *p = BUF(0);
      char csc last = BUF(t_len);
      int len;
      while ( p < last && isspace(*p) )
        {
          ++p;
        }
      if ( ( len = last - p ) >= STR_LEN )
        {
          se_error(COMM_TOO_LONG);
        }

      set_col(0);
      if (len == 0)
        {
          c_down();
          return;
        }

      (void)set_eor(COMMAND_LINE, t_len + 1);
      p[len] = EOS;
      cmd_buf = p;
      (void)home_command(D_SE_HOME, START_OF_PAGE);
      eor[COMMAND_LINE] = 0;
      disp_line();
      hist_ptr = hist_top;
    }
  else
    {
      split_line(0);
    }
}

private
void
recall_comm(void)
{
  if (hist_ptr == NULL)
    {
      hist_ptr = hist_top;
    }

  if (hist_ptr != NULL)
    {
      short csc start = (short *)( hist_ptr + 1 );
      short len = *start;
      hist_ptr = (stack *)( *hist_ptr );
      row = COMMAND_LINE;
      (void)set_eor(COMMAND_LINE, len);
      (void)movelr(s_buf[COMMAND_LINE], start + 1, len);
      disp_line();
      set_col(len);
    }
}

/*
 *  Read a string (such as a file name or a search pattern).
 *  Limit movement to the portion between the end of the prompt and the end
 *  of the line.  Also do not permit movement outside the line.  If the query
 *  type is a YES/NO, break on first character. Called only by Query().
 */

private
int
read_string(Q_MODE qtype)
{
  ACTION fun;
  const int c_col = col;
  int value;

  repeat
  {
    (void)move(COMMAND_LINE, col);
    fun = get_seq(&value);

    switch (fun)
      {
      case A_C_SOL:      /* Move to end of prompt */
        col = c_col;

      default:           /* Ignore all unknown keys */
        continue;

      case A_REST_LINE:  /* Discard this operation */
      case A_EXIT_EDITOR:
        return EOF;

      case A_C_RETURN:   /* End of input */
        col = eor[COMMAND_LINE];
        return G_FAIL;

      case A_DEL_LINE:   /* Delete from end of prompt */
        (void)move(COMMAND_LINE, col = c_col);
        del_rest(RIGHT);
        continue;

      case A_DEL_REST:   /* Delete rest of line */
        if (value == RIGHT)
          {
            del_rest(RIGHT);
          }

        continue;

      case A_B_TAB:
        b_tab();
        if (col < c_col)
          {
            col = c_col;
          }

        continue;

      case A_DEL_C:      /* Check for start of line */
        if (value == LEFT && col <= c_col)
          {
            continue;
          }
        /* fallthrough */

      case A_C_LEFT:
        if (col <= c_col)
          {
            continue;
          }

      case A_YANK:       /* unchanged commands */
      case A_BLOCK:
      case A_EXP_MODE:
      case A_C_EOL:
        break;

      case A_H_TAB:
        if ( ( col / screen_tabs + 1 ) * screen_tabs > last_col )
          {
            continue;
          }

        break;

      case A_CHARACTER:
        if (expand && eor[COMMAND_LINE] >= last_col)
          {
            continue;
          }
        /* fallthrough */

      case A_C_RIGHT:
        if (col >= last_col)
          {
            continue;
          }
      }

    se_execute(fun, value);

    if (qtype == Q_YORN)
      {
        return G_FAIL;
      }
  }
}

/*
 *  Prompt and input a string from the user - on the home command line.
 */

private
int
query(char csc prompt, char *const buf, Q_MODE qtype)
{
  const int c_offset = offset, c_row = row, c_col = col;
  const short c_eor = eor[COMMAND_LINE], c_len = s_eor[COMMAND_LINE];
  char *const c_buf = s_buf[COMMAND_LINE];
  int rc, p_len = strlen(prompt);
  string t_buf;

#if ASM86
  put_seq(v_base + COLS, prompt, p_len, query_col, COLS);
#else  /* if ASM86  */
  (void)move(COMMAND_LINE, 0);
  put_seq(prompt, p_len, query_col);
  attrset(norm_col);
  (void)clrtoeol();
#endif  /* if ASM86 */

  s_buf[COMMAND_LINE] = t_buf;
  s_eor[COMMAND_LINE] = E_BUFF_LEN;

  row = COMMAND_LINE, offset = 0;

  eor[COMMAND_LINE] = col = ++p_len;

  rc = read_string(qtype);

  eor[COMMAND_LINE] = c_eor;
  s_eor[COMMAND_LINE] = c_len;
  s_buf[COMMAND_LINE] = c_buf;

  if (rc != EOF)
    {
      if (qtype == Q_RAW)
        {  /* escape the SLASH'es */
          char *ip = t_buf + p_len, *op = buf, c;
          const char *last = t_buf + col;
          while (ip < last)
            {
              c = *ip++;
              if (c == SLASH)
                {
                  *op++ = BSLASH;
                }

              *op++ = c;
            }
          *op = EOS;
        }
      else
        {  /* Delete leading and trailing spaces */
          while ( p_len < col && isspace(t_buf[p_len]) )
            {
              ++p_len;
            }
          while ( col > p_len && isspace(t_buf[col - 1]) )
            {
              --col;
            }
          col -= p_len;
          (void)movelrz(buf, t_buf + p_len, col);
          if (qtype == Q_YORN)
            {
              if (u_star(buf) == 'Y')
                {
                  rc = G_OK;
                }

              elif(u_star(buf) == 'Q')
                  rc = EOF;
            }
        }
    }

  row = c_row, col = c_col, offset = c_offset;

  return rc;
}

/*
 *  Justify a paragraph.
 */

private
void
se_justify(const int value)
{
  string buf;
  const int j_col = FILE_COL;

  switch (value)
    {

    /* justify paragraph */

    case 'B':
      /* fallthrough */
    case 'Q':
      (void)sprintf(buf, "T.%d JP", col > eor[row] ? eor[row] : col);
      (void)run_command(buf, 0, value == 'B');
      break;

    /* set left margin */

    case 'L':
      if (j_col >= r_margin)
        {
          message("Left margin > right margin.");
        }
      else
        {
          l_margin = j_col;
        }

      break;

    /* set right margin */

    case 'R':
      if (j_col <= l_margin)
        {
          message("Right margin < left margin.");
        }
      else
        {
          r_margin = j_col;
        }

      break;

    /* toggle wordwrap */

    case 'W':
      wordwrap = !wordwrap;
      break;

    /* set line spacing */

    case 'S':
      if (query("New line spacing (0-9):", buf, Q_EDIT) == EOF)
        {
          return;
        }

      line_spacing = isdigit(*buf) ? *buf - '0' : 0;
      break;

    /* centre justify current line */

    case 'C':
      centre();
      break;

    /* toggle right adjust */

    case 'J':
      adjust = !adjust;
    }

  disp_margins();

  disp_home();
}

/*
 *  Search for strings, and possibly replace.
 *  Note:  Use RE syntax here because it avoids the GE options
 *  such as C, S etc.
 */

private
void
search(const int value)
{
  string buf, rhs, find_com, rep_com;
  int rc, rel, neg, iter = NO, nextl = 0, line, sop;
  const char *p;

  switch (value)
    {
    case 'I':  /* Find line Number */
      if (query("Line:", buf, Q_EDIT) == EOF)
        {
          return;
        }

      if (row == COMMAND_LINE)
        {
          c_home('Q');
        }

      sop = START_OF_PAGE;
      p = buf;
      rel = neg = NO;
      if (*p == '+' || *p == '-')
        {
          ++rel;
          if (*p == '-')
            {
              ++neg;
            }

          ++p;
        }

      line = atoi(p);
      if (neg)
        {
          line = -line;
        }

      if (rel)
        {
          line += FILE_LINE;
        }

      if (line < 0)
        {
          line = 0;
        }

      elif( line > vssizeof(in_u) )
          line = vssizeof(in_u);

      if (line >= sop && line < sop + text_lines)
        {
          row = line - sop + FIRST_LINE;
          return;
        }

      if (line < MATCH_LINE - FIRST_LINE)
        {
          row = line + FIRST_LINE;
          line = 0;
        }
      else
        {
          row = MATCH_LINE;
          line -= MATCH_LINE - FIRST_LINE;
        }

      file_move(-line);
      return;

    case 'F':  /* Search only - from current position */
      if (query(se_find, buf, Q_RAW) == EOF)
        {
          return;
        }

      (void)sprintf(find_com, se_fcom1, buf);
      break;

    case 'A':  /* Search and Replace */
      if (query(se_find, buf, Q_RAW) == EOF)
        {
          return;
        }

      if (query("Replacement:", rhs, Q_RAW) == EOF)
        {
          return;
        }

      (void)sprintf(find_com, se_fcom1, buf);
      (void)sprintf(rep_com, "RR\177%s\177%s\177", buf, rhs);
      ++iter;
      break;

    case 'L':  /* Repeat last search/replace */

      /*
       * Simply re-execute the main command
       * after moving on to the next line
       */

      if (eor[row] != EOF)
        {
          nextl = 1;
        }

      (void)movelr5(find_com, "TR//");
    }

  if (row == COMMAND_LINE)
    {
      c_home('Q');
    }

  row = MATCH_LINE;

  while (run_command(find_com, nextl, NO) == 0 && iter)
    {
      status();
      if ( ( rc = query("Replace (y/n) ?", buf, Q_YORN) ) == EOF )
        {
          break;
        }

      if (rc == G_OK)
        {
          (void)run_command(rep_com, 0, NO);
        }

      nextl = 1;
    }

  if (eor[row] == EOF)
    {
      c_sol();
    }
}

/*
 *  Leave the editor or return to the context editor.
 */

private
void
exit_editor(const int value)
{
  string com;
  const int fl = HFILE_LINE;

  se_sync();

  switch (value)
    {
    case 'Q':  /* Exit, discarding everything */
      if ( prim_changed
          && query("File changed, abandon edit (y/n) ?", com, Q_YORN) )
        {
          return;
        }

      Quit();
      abort();
      /* fallthrough */

    case 'X':  /* Exit, saving file */
      (void)sprintf(
        com,
        "Save %sfile and exit (y/n) ?",
        prim_changed ? empty : se_fin1);
      if ( query(com, com, Q_YORN) )
        {
          return;
        }

      cmd_buf = "e";
      (void)home_command(D_SE_HOME, START_OF_PAGE);
      break;

    case 'D':  /* Return to context editor */
      cmd_buf = let_col;
      (void)home_command(D_SE_HOME, fl);
      /* fallthrough */

    case 'S':  /* Save the file and return */
      (void)sprintf(
              com, "Save %sfile (y/n) ?", prim_changed ? empty : se_fin1);
      if ( query(com, com, Q_YORN) )
        {
          return;
        }

      (void)sprintf(com, "SN\177%s\177,TE,X", out_fname);
      (void)run_command(com, -fl, YES);
      prim_changed = NO;
    }
}

/*
 *  Read/Write/Execute files.
 */

private
void
rwx_file(const int value)
{
  int sop = YES;
  char optc;
  string com, nlines;
  FNAME fname;

  switch (value)
    {
    case 'R':  /* Read in entire file */
      if (query("Merge file:", fname, Q_EDIT) == EOF)
        {
          return;
        }

      (void)sprintf(com, "M\177%s\177,TE,X", fname);
      break;

    case '!':  /* Read from command */
      /* fallthrough */
    case '|':  /* Filter through command */
#if UNIX
      if (query("Command:", fname, Q_EDIT) == EOF)
        {
          return;
        }

      if (value == '!')
        {
          (void)sprintf(com, "I!\177%s\177", fname);
        }
      else
        {
          if (query(se_ep, nlines, Q_EDIT) == EOF)
            {
              return;
            }

          (void)sprintf(com, "S,T%s,X|\177%s\177", nlines, fname);
        }

      *fname = '!';
      break;
#else  /* if UNIX */
      return;

#endif  /* if UNIX */

    case 'A':  /* Append a block to a file */
      if (query("Append to file:", fname, Q_EDIT) == EOF)
        {
          return;
        }

      optc = 'A';
      goto make_com;

    case 'W':  /* Write a block to a file */
      if (query("Save file:", fname, Q_EDIT) == EOF)
        {
          return;
        }

      optc = 'N';
make_com:
      if (query(se_ep, nlines, Q_EDIT) == EOF)
        {
          return;
        }

      if ( nullstr(nlines) )
        {
          if ( nullstr(fname) )
            {
              return;
            }

          (void)sprintf(com, "S%c\177%s\177,M,TE,X,X", optc, fname);
        }
      elif( nullstr(fname) )
          (void)sprintf(com, "S%c,T%s,X", optc, nlines);
      else
        {
          (void)sprintf(com, "S%c\177%s\177,T%s,X", optc, fname, nlines);
        }
      break;

    case 'U':  /* Execute (use) a command file */
      if (query("Use file:", fname, Q_EDIT) == EOF)
        {
          return;
        }

      (void)sprintf(com, "U\177%s\177", fname);
      sop = NO;
      break;

    case 'O':  /* Re-read oldfile */
      if (query("Old file:", fname, Q_EDIT) == EOF)
        {
          return;
        }

      (void)sprintf(com, "O\177%s\177", fname);
      if (row == COMMAND_LINE)
        {
          text_row = FIRST_LINE;
        }
      else
        {
          row = FIRST_LINE;
        }

      sop = NO;
    }

#if UNIX
  if (*fname == '!')
    {
      save_std();  /* discard stderr & stdin */
    }

  (void)run_command(com, 0, sop);

  restore_std();
#else  /* if UNIX */
  (void)run_command(com, 0, sop);
#endif  /* if UNIX */
}

/*
 *  Count lines, word and chars in old file.
 */

private
void
word_count(void)
{
  long rc, wc, pc, cc, sc, lc, nc;
  const byte *p, *last;
  byte c;
  string res;
  UNIT t_u;
  int len, inword;

  rc = wc = pc = cc = lc = sc = nc = 0;

  t_u = *in_u;
  vsrewind( ( &t_u ) );
  while ( ( len = vsgetrec(&t_u, &p) ) != EOF )
    {
      cc += len;
      ++rc;
      inword = 0;
      for (last = p + len; p < last; ++p)
        {
          c = *p;
          if ( !( wordch(c) ) )
            {
              inword = 0;
            }

          elif(!inword)
              wc += ( inword = 1 );
          if ( punctch(c) )
            {
              ++pc;
              switch (c)
                {
                case ';':
                  ++lc;
                  break;

                case '.':
                case '!':
                case '?':
                  ++sc;
                }
            }

          if (iscntrl(c) && c != TAB)
            {
              ++nc;
            }
        }
    }

  (void)sprintf(res, f_wc, rc, wc, pc, nc, sc, lc, cc + rc);
  inform(res);
}

/*
 *  Misc system operations.
 */

private
void
misc_ops(const int value)
{
  const char *com;
  int start = 0;
  FNAME buf;

  switch (value)
    {
    case 'E':  /* Spawn Shell */
      com = ".tss//";
      break;

    case 'P':  /* Print the file */
      (void)sprintf(buf, se_pcom, in_fname);
      com = buf;
      start = -HFILE_LINE;
      break;

    case 'L':  /* Change directory */
      if (query("New directory:", buf, Q_EDIT) == EOF)
        {
          return;
        }

      if ( chdir(prep_name(buf) ) == -1 )
        {
          se_error(FILE_ERROR);
        }

      return;

    case '?':  /* Display File Statistics */
      se_sync();
      word_count();
      disp_text();
      return;

    case 'F':  /* List files */
      com = se_lcom;
    }

  (void)run_command(com, start, YES);
}

/*
 *  Execute general repeating commands.
 */

private
void
se_loop(int value)
{
  ACTION fun;
  int speed = 2;  /* 200ms default */
  int c, c_orec, c_row, c_col;
  stack *c_in_s, *c_del_s;

  if (value)
    {
      fun = A_FILE_MOVE;  /* ^QW or ^QZ */
    }

  elif( ( fun = get_seq(&value) ) == A_REPEAT )
      return;

  ++qq_loop;

  while (qq_loop)
    {
      kbd_check(c);
      if (c != ERR)
        {
          if ( isdigit(c) )
            {
              if ( ( speed = c - '0' ) > 5 )
                {
                  speed <<= 2;
                }
            }
          else
            {
              break;
            }
        }

      c_orec = o_rec, c_row = row, c_col = col;
      c_in_s = in_stack, c_del_s = del_stack;
      se_execute(fun, value);
      if (c_orec == o_rec && c_row == row && c_col == col \
           && c_in_s == in_stack && c_del_s == del_stack)
        {
          break;
        }

#if UNIX
      (void)refresh();
#else  /* if UNIX */
      bios_gotoxy( (byte)curs_row, (byte)curs_col );
#endif  /* if UNIX */
      if (speed > 0)
        {
          (void)napms(speed * 100);
        }
    }

  qq_loop = NO;
}

/*
 *  Execute an action on the screen.
 */

private
void
se_execute(const ACTION act, const int value)
{
  last_offset = offset;

  switch (act)
    {
    case A_C_UP:
      c_up();
      break;

    case A_FILE_MOVE:
      file_move(value);
      break;

    case A_C_DOWN:
      c_down();
      break;

    case A_C_LEFT:
      c_left();
      break;

    case A_EXP_MODE:
      expand = !expand;
      break;

    case A_DEL_C:
      del_c(value);
      break;

    case A_C_HOME:
      c_home(value);
      break;

    case A_B_TAB:
      b_tab();
      break;

    case A_PAGE_SHIFT:
      page_shift(value);
      break;

    case A_DEL_REST:
      del_rest(value);
      break;

    case A_C_EOL:
      c_eol();
      break;

    case A_C_TOS:
      row = FIRST_LINE;
      break;

    case A_C_SOL:
      c_sol();
      break;

    case A_C_BOS:
      row = last_line;
      break;

    case A_JUSTIFY:
      se_justify(value);
      break;

    case A_HELP:
#if TINY_G
      Help();
#else  /* if TINY_G */
      {
        VERB opts;
        opts.o1.q = 0;
        Help(&opts);
      }
#endif  /* if TINY_G */
      init();
      break;

    case A_RWX_FILE:
      rwx_file(value);
      goto d_home;

    case A_EXIT_EDITOR:
      exit_editor(value);
      goto d_home;

    case A_MISC_CE:
      misc_ops(value);
      goto d_home;

    case A_SEARCH:
      search(value);
d_home:
      disp_home();
      break;

    case A_W_LEFT:
      (void)find_backwards(NULL, value);
      break;

    case A_C_RIGHT:
      c_right();
      break;

    case A_W_RIGHT:
      (void)find_forwards(NULL, value);
      break;

    case A_H_TAB:
      h_tab();
      break;

    case A_CHARACTER:
      character(value);
      break;

    case A_C_RETURN:
      c_return();
      break;

    case A_OPEN_LINE:
      open_line();
      break;

    case A_DEL_LINE:
      yank();
      if (row == COMMAND_LINE)
        {
          (void)move(COMMAND_LINE, 0);
          (void)clrtoeol();
          eor[COMMAND_LINE] = 0;
          set_col(0);
        }
      else
        {
          (void)del_line();
        }

      break;

    case A_REST_LINE:
      rest_line();
      break;

    case A_REDRAW:
      term();
      init();
      break;

    case A_YANK:
      yank();
      break;

    case A_HIST:
      recall_comm();
      break;

    case A_BLOCK:
      block(value);
      break;

    case A_COMMAND:
      (void)run_command(let_col, 0, NO);
      break;

    case A_REPEAT:
      se_loop(value);
      break;

    case A_FINDC:
      find_char(value);

    default:
      break;
    }

  status();
  if (offset != last_offset)
    {
      disp_home();
      disp_template();
      disp_text();
    }

  (void)move(row, col);
}

/*
 *  Screen editor driver.
 */

private
void
Screen_ed(void)
{
  int value;

  if (setjmp(se_ret) == YES)
    {
      term();
      return;
    }

  if (s_buf == NULL)
    {
      init_screen();
    }

  linewrap();
  file_to_buf(SE_ENTER);

  /* initialize tracking variables */

  eor[row = COMMAND_LINE] = lon = col = text_col = text_offset = offset = 0;
  text_row = FIRST_LINE;
  init();

  if ( setjmp(se_err) )
    {
      (void)move(row, col);
    }

  raw();

  if (trunc_recs)
    {
      trunc_recs = 0;
      se_error(LINE_TOO_LONG);
    }

  repeat
  {
    const ACTION fun = get_seq(&value);
#if UNIX
    if (idlpending)
      {
        (void)idlok(stdscr, NO);
        idlpending = NO;
      }

#endif  /* if UNIX */
    se_execute(fun, value);
  }
}

/*
 *  Context Editor.
 */

/* Free a compiled G command list. */

private
void
free_prog(VERB_LIST *const ptr)
{
  VERB *p = ptr->prog, *last;

  do
    {
      if (p->o1.e != NULL)
        {
          free_expr(p->o1.e);
        }

      elif(p->o2.e != NULL) free_expr(p->o2.e);
          last = p;
      p = p->next;
    }
  while ( p != NULL );

  last->next = g_free_list;
  g_free_list = ptr->prog;

  ptr->prog = NULL;
}

/*
 *  Drop down G command list stack after interrupt etc
 */

private
void
pop_com_stack(VERB_LIST *new_ptr)
{
  while (com_stack_ptr != new_ptr)
    {
      if (com_stack_ptr->prog != NULL)
        {
          free_prog(com_stack_ptr);
        }

      com_stack_ptr = com_stack_ptr->prev;
    }

  depth = new_ptr->save_depth;
}

/*
 *  Set up the current line to be printed
 */

private
int
make_line(char *const buf)
{
  int i_len = i_eor - i_col;

  if (e_col + i_len > E_BUFF_LEN)
    {
      i_len = E_BUFF_LEN - e_col;
    }

  (void)movelr(mmovelr(buf, e_buff, e_col), i_buff + i_col, i_len);
  return e_col + i_len;
}

/*
 *  Called from List to display current line in hex.
 */

private
void
hex_print(void)
{
  int len, seg_len;
  char *p, *rec_end, *first, *last;
  LINE buf;

  new_line();

  if ( ( len = make_line(buf) ) == 0 )
    {
      say("LX: empty line.\n");
      return;
    }

  for (first = buf, rec_end = buf + len; first < rec_end; first += 16)
    {
      (void)fprintf( vdu, "%5d  |  ", (int)( first - buf ) );
      if ( ( last = first + 16 ) > rec_end )
        {
          last = rec_end;
        }

      for (p = first; p < last; ++p)
        {
          (void)fprintf(vdu, "%02X ", *(byte *)p);
        }

      for (p = first; p < last; ++p)
        {
          if ( iscntrl(*p) )
            {
              *p = '.';
            }
        }

      seg_len = last - first;
      (void)fprintf(vdu,
              "%*c|  %.*s", 1 + ( 16 - seg_len ) * 3, SPACE, seg_len, first);
      new_line();
    }

  new_line();
}

/*
 *  Find a space character in the edit buffer.
 */

private
void
get_space(const int minus, const int comm)
{
  int col = i_col;

  if (minus)
    {  /* scan backwards from current pos */
      while ( col > 0 && isspace(i_buff[col - 1]) )
        {
          --col;
        }
      if (col < i_col && comm == 'T')
        {
          linewrap();
        }
    }
  else
    {
      while ( col < i_eor && isspace(i_buff[col]) )
        {
          ++col;
        }
    }

  altr_line(col - i_col, comm);
}

/*
 *  Alter position in file
 */

private
void
alter_end(int last, const int comm)
{
  const int eof_rec = vssizeof(in_u);

  if (last == EOF || last > eof_rec)
    {
      last = eof_rec;
    }

  if (last == g_rec)
    {
      return;
    }

  switch (comm)
    {
    case 'T':
      if (g_eof)
        {
          if (depth)
            {
              return;
            }

          g_err(END_OF_FILE, NULL);
        }

      flush_buff();
      if (last - g_rec > 1)
        {
          o_rec += last - vstell(in_u);
          vscopy(out_u, in_u, last);
          g_rec = last - 1;
        }

      (void)fill_buff();
      break;

    case 'K':
      if (last < g_rec)
        {
          g_err(NO_BACK, NULL);
        }
      /* fallthrough */

    case 'P':
      if (g_eof)
        {
          if (last > g_rec)
            {
              if (depth)
                {
                  return;
                }

              g_err(END_OF_FILE, NULL);
            }

          g_eof = NO;
        }

      vsseek(in_u, last);
      g_rec = last - 1;
      (void)fill_buff();
      break;

    case 'L':
      if (last < g_rec)
        {
          g_err(NO_BACK, NULL);
        }

      if (g_eof)
        {
          return;
        }

      while ( g_rec < last - 1 && fill_buff() )
        {
          n_print(i_buff, i_eor, NO);
        }
    }
}

/*
 *  Display the line number, the edit buffers, and the prompt
 *  on the screen (possibly in hex). Indicate portions etc.
 */

private
void
printline(char *ptr)
{
  int len, sec;
  LINE buf;

  if (fscreen)
    {
      return;
    }

  if (buff_sec * L_LEN > e_col + i_eor - i_col)
    {
      buff_sec = e_col / L_LEN;
    }

  sec = buff_sec * L_LEN;

  if (g_eof || g_rec < 0)
    {
      ptr += sprintf(
        ptr,
        g_rec > 0
                        ? "End of file encountered following line %d.\nEOF"
                        : "Empty file.\nEOF",
        g_rec);
    }
  else
    {
      ptr += sprintf(ptr, line_pos, g_rec, e_col);
    }

  if (f_list != NULL)
    {
      char csc file = f_list->name;
      const int std = equal1(file, '-');
      if (f_list->disp == 'M')
        {
          ptr += sprintf(ptr, ", merged from %s", std ? si_file : file);
        }
      else
        {
          ptr += sprintf(
            ptr,
            ", to be %s to %s",
            f_list->disp == 'A' ? "appended" : "saved",
            std ? so_file : file);
        }
    }

  *ptr++ = '\n';

#if FULL_G
  if (show_template && sec == 0)
    {
      ptr = mzmovelr(ptr, user_template);
    }

#endif  /* if FULL_G */

  if (g_eof)
    {
      (void)strcpy(ptr, "*\r");
    }
  else
    {
      char *p, c;
      *ptr++ = (char)( buff_sec + '0' );
      if ( ( len = make_line(buf) - sec ) > L_LEN )
        {
          len = L_LEN;
        }

      printable(buf + sec, len);
      (void)movelr(p = mmovelr(ptr + 1, buf + sec, len), "\n*\r", 4);
      if (e_col < sec)
        {
          c = '<';
        }

      elif(e_col > sec + L_LEN)
          c = '>';
      else
        {
          c = SPACE;
          (void)sprintf(p, "\n*%*c|\r", e_col - sec + 1, SPACE);
        }
      *ptr = c;
    }
}

/*
 *  History
 */

private
void
hist_write(char csc comm)
{
  int len;

  if ( hist_top == NULL || ( len = *(short *)( hist_top + 1 ) )
      != strlen(comm) || !ecmp(comm, (short *)( hist_top + 1 ) + 1, len) )
    {
      push_line( &hist_top, comm, strlen(comm) );
    }
}

private
int
hist_recall(char *const comm)
{
  const short *start;

  if (hist_top == NULL)
    {
      return 0;
    }

  start = (short *)( hist_top + 1 );
  (void)movelr(comm, start + 1, *start);
  return *start;
}

/*
 *  Insert a string behind the pointer.
 */

private
void
do_ins(char csc str, const int len)
{
  if ( (unsigned long)e_col + len > E_BUFF_LEN )
    {
      g_err(LINE_TOO_LONG, NULL);
    }

  (void)movelr(e_buff + e_col, str, len);  /* append string */
  e_col += len;
}

private
void
ins(const word disp, const char *str, int len)
{
  int l;
  const word scr = disp & 01;
  const char *split;

  if (len < 0)
    {
      len = strlen(str);
    }

  if (len > 0)
    {
      while ( ( split = (const char *)memchr(str, LFEED, len) ) != NULL )
        {
          l = split - str;
          if (scr)
            {
              (void)!write(vdu_fd, str, l++);
              new_line();
            }
          else
            {
              do_ins(str, l++);
              out_buff();
            }

          len -= l;
          str += l;
        }
      if (scr)
        {
          (void)!write(vdu_fd, str, len);
        }
      else
        {
          do_ins(str, len);
        }
    }

  if (disp > 1)
    {
      if (scr)
        {
          new_line();
        }
      else
        {
          out_buff();
        }
    }
}

/*
 *  Insert a block of text read from the terminal.
 */

#if FULL_G

private
void
block_insert(const word ch, const word header)
{
  static char prompt[3] = "x?";

  *prompt = (char)ch;

  if (!depth && header)
    {
      putstr(user_template);
    }

  if (ch == 'A')
    {
      (void)add_line('T');
    }
  else
    {
      linewrap();
    }

  repeat
  {
    if ( get_com(e_buff, prompt) )
      {
        g_err(EOF_INSERT, NULL);
      }

    if ( ( e_col = strlen(e_buff) ) == 0)
      {
        return;
      }

    out_buff();
  }
}

#endif  /* if FULL_G */

/*
 *  After, Before, and Replace.  These now accept the R
 *  qualifier to indicate Regular Expression matching.
 */

private
void
ABR(VERB csc opts)
{
  int len1 = opts->o1.v, len2 = opts->o2.v;
  LINE buf;

  if ( findstr(opts->o1.s, len1, opts->o1.q, 'T') )
    {
      if (depth)
        {
          get_end();
          return;
        }

      g_err(STR_N_F, opts->o1.s);
    }

  if (opts->o1.q == 'r')
    {
      len1 = (int)( loc2 - loc1 );
      len2 = re_sub(opts->o2.s, buf);
    }

  if (opts->comm == 'R')
    {
      altr_line(len1, 'P');
    }

  elif(opts->comm == 'A')
      altr_line(len1, 'T');

  ins(0, opts->o1.q == 'r' ? buf : opts->o2.s, len2);
  loc2 = NULL;
}

/*
 *  Verify is similar to PT but no actions are ever taken
 *  and returns FAIL if the endpoint criterion is met.
 */

private
int
Verify(VERB csc opts)
{
  int val;
  word opt = opts->o1.q & ~LOOP_MASK;

  if (opt != NO_OPT)
    {
      if (opt & OP_CALC)
        {
          val = (int)Evaluate(opts->o1.e, C_ENDP)->opval.i;
        }
      else
        {
          val = opts->o1.v;
        }

      switch (opt & BYTE_MASK)
        {
        case OP_EOF:
          return !g_eof;

        case RECS:
        case MRECS:
          return val == 0;

        case R_END:
          return g_rec != val;

        case OR_END:
          return o_rec != val;

        case STR_END:
          return findstr(opts->o1.s, val, opt, 'V');

        default:
          g_err(I_OPT, opts->errp);
        }
    }

  if (opts->dot)
    {
      if ( ( opt = opts->o2.q ) & OP_CALC )
        {
          val = (int)Evaluate(opts->o2.e, C_ENDP)->opval.i;
        }
      else
        {
          val = opts->o2.v;
        }

      switch (opt & BYTE_MASK)
        {
        case 'S':
          if (opts->o2.v)
            {
              return !( i_col > 0 || isspace(i_buff[i_col - 1]) );
            }

          return !( i_col < i_eor || isspace(i_buff[i_col]) );

        case OP_EOF:
          return i_col < i_eor;

        case 'G':
        case 'r':
          return findstr(opts->o2.s, val, opt, 'V');

        case RECS:
        case MRECS:
          return val == 0;

        case R_END:
          return i_col != val;

        case OR_END:
          return e_col != val;

        default:
          g_err(I_OPT, opts->errp);
        }  /* case */
    }

  return G_OK;
}

/*
 *  Position and Transcribe, also handles List and Kill because
 *  they have similar syntax.
 */

private
int
PT(VERB csc opts)
{
  int val;
  word opt = opts->o1.q;
  const char comm = opts->comm, *const opt2 = opts->o2.s;

  if (opt & LOOP_MASK)
    {
      const int endc = ( opt & OP_UNTIL ) == 0;
      while (Verify(opts) != endc && !g_eof)
        {
          (void)add_line(comm);
        }
      return G_OK;
    }

  if (opt != NO_OPT)
    {
      if (opt & OP_CALC)
        {
          val = (int)Evaluate(opts->o1.e, C_ENDP)->opval.i;
        }
      else
        {
          val = opts->o1.v;
        }

      switch (opt & BYTE_MASK)
        {
        default:
          g_err(I_OPT, opts->errp);
          /* fallthrough */

        case R_TIMES:
          if (val >= 0)
            {
              goto do_recs;
            }
          /* fallthrough */

        case OP_EOF:
          alter_end(EOF, comm);
          break;

        case MRECS:
          val = -val;
          /* fallthrough */

        case RECS:
do_recs:
          if (val < 0 && comm != 'P')
            {
              val += wrapround();
            }
          else
            {
              val += g_rec;
            }

          if (val < 0)
            {
              g_err(B_BOFFILE, opts->errp);
            }

          alter_end(val, comm);
          break;

        case STR_END:
          if ( findstr(opts->o1.s, val, opt, comm) )
            {
              if (comm == 'L')
                {
                  return G_OK;
                }

              if (depth)
                {
                  return G_FAIL;
                }

              g_err(END_OF_FILE, opts->errp);
            }

          break;

        case OR_END:
          if (o_rec >= val)
            {
              break;
            }

          val = g_rec + ( val - o_rec );
          /* fallthrough */

        case R_END:
          if (val < 0)
            {
              g_err(B_BOFFILE, opts->errp);
            }

          if ( ( val <= g_rec || g_rec == -1 ) && comm != 'P' )
            {
              (void)wrapround();
            }

          if (g_rec >= 0)
            {
              alter_end(val, comm);
            }
        }
    }

  if (opts->dot)
    {
      if ( ( opt = opts->o2.q ) & OP_CALC )
        {
          val = (int)Evaluate(opts->o2.e, C_ENDP)->opval.i;
        }
      else
        {
          val = opts->o2.v;
        }

      switch (opt & BYTE_MASK)
        {
        default:
          g_err(I_OPT, opts->errp);
          /* fallthrough */

        case 'S':
          get_space(val, comm);
          break;

        case 'G':
        case 'r':
          if ( findstr(opt2, val, opt, comm) )
            {
              if (depth)
                {
                  altr_line(i_eor - i_col, comm);
                  return G_FAIL;
                }

              g_err(STR_N_F, opts->errp);
            }

          break;

        case R_TIMES:
          if (val >= 0)
            {
              goto r_end;
            }
          /* fallthrough */

        case OP_EOF:
          altr_line(i_eor - i_col, comm);
          break;

        case OR_END:
          if ( (unsigned long)e_col >= val )
            {
              break;
            }

          val = i_col + ( val - e_col ) + 1;
          /* fallthrough */

        case R_END:
          if (val <= i_col && comm == 'T')
            {
              val -= e_col;
            }
          else
            {
              val -= i_col;
            }

          goto r_end;

        case MRECS:
          val = -val;
          /* fallthrough */

        case RECS:
r_end:
          if (val < 0 && comm == 'T')
            {
              val += e_col;
              linewrap();
            }

          altr_line(val, comm);
        }  /* case */
    }

  return G_OK;
}

/*
 *  Return YES if current line should terminate the justification.
 */

private
int
end_line(void)
{
  const char *p = i_buff, *const last = p + i_eor;

  if ( *p == '.' || iscntrl(*p) )
    {
      return YES;
    }

  while (p < last)
    {
      if ( isspace(*p) )
        {
          ++p;
        }
      else
        {
          return NO;
        }
    }
  return YES;
}

/*
 *  Adjust right margin by inserting spaces in gaps (preferably after punct).
 */

private
void
pad(char csc c_type)
{
  char *const eb = e_buff, *const lm = eb + l_margin,
  *const rm = eb + t_margin, *ec = eb + e_col,
    *p = left_right ? lm : ec;

  while ( ec <= rm && ( left_right ? ++p < ec : --p > lm ) )
    {
      if ( isspace(*p)
          && ( c_type == NULL ? !isspace(p[-1])
                           : strchr(c_type, p[-1]) != NULL ) )
        {
          (void)moverl(p + 1, p, ec++ - p);
        }
    }

  e_col = ec - eb;
}

/*
 *  Compress an individual line.
 *  Returns YES at end of input line, NO when output line full.
 */

private
int
pack(void)
{
  char *const eb = e_buff, *const ib = i_buff;
  const char *ip = ib + i_col,  /* current input column        */
    *last = ib + i_eor,         /* end of current input buffer */
    *i_start,                   /* start of cur. input word    */
    *limit;                     /* pointer to right margin     */
  char *e_start,                /* start of cur. output word   */
    *ep = eb + e_col;           /* current output column       */

  /* delete trailing spaces */

  while ( last > ib && isspace(last[-1]) )
    {
      --last;
    }

  limit = eb + t_margin;

  do
    {

      /* skip leading spaces */

      while ( ip < last && isspace(*ip) )
        {
          ++ip;
        }

      if (ip >= last)
        {
          return NO;
        }

      /* record start of word */

      i_start = ip, e_start = ep;

      /* copy a word */

      while ( ip < last && ep <= limit && !( isspace(*ip) ) )
        {
          if ( iscntrl(*ip) )
            {
              ++limit, ++t_margin;
            }

          *ep++ = *ip++;
        }

      /* add space between words */

      if (ep <= limit)
        {
          *ep++ = SPACE;
        }

      if (ip >= last)  /* end of input text */
        {
          e_col = ep - eb;
          return NO;
        }
    }
  while ( ep <= limit );

  /* output line full */

  if (e_start == eb + l_margin && ep[-1] != '-')
    {  /* word too long for margins, ignore limit */
      while ( ip < last && !( isspace(*ip) ) )
        {
          *ep++ = *ip++;
        }
    }

  elif(!isspace(*ip) && ep[-1] != '-')  /* end of word */
    {  /* in middle of word - back up to start */
      while ( ip < last && iscntrl(*ip) )
        {
          *ep++ = *ip++;
        }
      if (!isspace(*ip) && ip < last)
        {
          while (--ip >= i_start)
            {
              if ( iscntrl(*ip) )
                {
                  --t_margin;
                }
            }
          ep = e_start;
          ip = i_start;
        }
    }

  e_col = ep - eb;
  i_col = ip - ib;

  /* delete any trailing blanks */

  while ( e_col > 0 && isspace(eb[e_col - 1]) )
    {
      --e_col;
    }

  return YES;
}

/*
 *  Justify a paragraph.
 */

private
void
justify(void)
{
  char *const eb = e_buff;
  int i;

  /* skip blank lines etc */

  while ( end_line() )
    {
      if ( !(add_line('T') ) )
        {
          return;
        }
    }

  if (i_col > 0)
    {
      char csc ib = i_buff;
      while ( i_col < i_eor && !isspace(ib[i_col]) )
        {
          eb[e_col++] = ib[i_col++];
        }
      if (i_col < i_eor)
        {
          eb[e_col++] = ib[i_col++];
        }
    }

  if (e_col < l_margin)
    {
      space_fill(eb + e_col, l_margin - e_col);
      e_col = l_margin;
    }

  repeat
  {
    t_margin = r_margin;
    if ( pack() )  /* output line full */
      {
        if (e_col <= l_margin)
          {
            break;
          }

        pad(endsent);
        if (adjust)
          {
            pad(",:");
            while (e_col <= t_margin)
              {
                const int s_ec = e_col;
                pad(NULL);
                if (s_ec == e_col)
                  {
                    break;
                  }
              }
          }

        left_right = !left_right;
        out_buff();
        for (i = 0; i < line_spacing; ++i)
          {
            e_col = 0;
            out_buff();
          }

        space_fill(eb, e_col = l_margin);
      }
    else  /* input line exhausted */
      {
        if ( !( fill_buff() ) || end_line() )
          {
            break;
          }
      }
  }

  if (e_col <= l_margin)
    {
      e_col = 0;
    }
  else
    {
      pad(endsent);
      out_buff();
    }
}

/*
 *  Join to current (duplicate) or next line, or justify.
 */

private
int
Join(VERB csc opts)
{
  const char *sp, *ep;
  char *const eb = e_buff, *ec;
  const int next = ( opts->o1.v == 0 );

  if (opts->o1.q == 'P')
    {
      justify();
      return G_OK;
    }

  get_end();
  for (ec = eb + e_col; ec > eb && isspace(ec[-1]); --ec)
    {
      ;
    }

  if (next)
    {
      if ( !( fill_buff() ) )
        {
          return G_FAIL;
        }

      i_col = i_eor;
      sp = i_buff;
      ep = sp + i_eor;
      while ( ep > sp && isspace(ep[-1]) )
        {
          --ep;
        }
    }
  else
    {
      sp = eb;
      ep = ec;
    }

  while ( sp < ep && isspace(*sp) )
    {
      ++sp;
    }

  if (ep > sp)
    {
      const int len = ep - sp;
      *ec++ = SPACE;
      e_col = ( ec - eb ) + len;
      if ( (unsigned long)e_col > E_BUFF_LEN )
        {
          g_err(LINE_TOO_LONG, NULL);
        }

      (void)movelr(ec, sp, len);
    }

  return G_OK;
}

/*
 *  Insert verb.
 */

private
void
Insert(VERB csc opts)
{
  char   ch_delim;
  const  char *s1p = opts->o1.s;
  static char prompt[3] = " x";
  int    len;
  time_t tm;
  const  TOKEN *res;
  const  word disp = ( opts->comm == 'D' );
  string ss1;

  if (disp)
    {
      term();
    }

  switch (opts->o1.q)
    {
#if FULL_G
    case 'A':
    case 'B':
      if (depth | fscreen)
        {
          g_err(INT_OPT, opts->errp);
        }

      block_insert(opts->o1.q, opts->o2.q);
      return;

#endif  /* if FULL_G */
    case 'C':
      *ss1 = (char)Evaluate(opts->o1.e, C_ENDP)->opval.i;
      ins(disp, ss1, 1);
      return;

    case '{':
      res = Evaluate(opts->o1.e, C_SIDE);
      if (res->fp)
        {
          len = sprintf(ss1, m_real, res->opval.r);
        }
      else
        {
          len = sprintf(ss1, n_format, res->opval.i);
        }

      ins(disp, ss1, len);
      return;

    case 'F':
      ins(disp, in_fname, -1);
      return;

    case 'D':
      (void)time(&tm);
      (void)movelrz(ss1, ctime(&tm), DATE_LEN);
      ins(disp, ss1, -1);
      return;

    case 'X':
      ins(disp, s1p, opts->o1.v);
      return;

#if FULL_G
    case 'S':
      ins(disp + 2, user_template + 2, L_LEN);
      return;

#endif  /* if FULL_G */
    case 'P':
      wait_user();
      return;

#if UNIX && !defined(OMIT_POPEN)
    case '!':
      vsunlink(out_u);
      if ( ( len = Proc_to_mem(out_u, s1p) ) == EOF )
        {
          g_err(SYS_COM_FAIL, opts->errp);
        }

      o_rec += len;
      return;

#endif  /* if UNIX && !defined(OMIT_POPEN) */
    }

  ch_delim = *s1p++;
  if ( gdss(ss1, &len, &s1p) )
    {
      if (depth | fscreen)
        {
          g_err(INT_OPT, opts->errp);
        }

      ins(disp + 2, ss1, len);
      prompt[1] = ch_delim;
      repeat
      {
        if ( get_com(ss1, prompt) )
          {
            g_err(EOF_INSERT, opts->errp);
          }

        len = strlen(ss1);
        if (ss1[len - 1] == ch_delim)
          {
            ins(disp, ss1, len - 1);
            return;
          }

        ins(disp + 2, ss1, len);
      }
    }
  else
    {
      while (*s1p++ == ch_delim)
        {
          ins(disp + 2, ss1, len);
          if ( gdss(ss1, &len, &s1p) )
            {
              g_err(UP_DELIM, opts->o1.s);
            }
        }
      ins(disp, ss1, len);
    }
}

private
void
Exit(void)
{
  VERB  xit_opts;
  const FILE_LIST *fptr;

  while (f_list != NULL)
    {
      for (fptr = f_list; fptr != NULL; fptr = fptr->next)
        {
          if (fptr->disp != 'M')
            {
              g_err(SAV_ON_STACK, NULL);
            }
        }

      if (!g_eof)
        {
          alter_end(EOF, 'T');
        }

      if (e_col)
        {
          out_buff()  /* in case user added line at eof */;
        }

      xit_opts.o1.q = 'M';
      Xit(&xit_opts);
    }

  if (!g_eof)
    {
      alter_end(EOF, 'T');
    }

  if (e_col)
    {
      out_buff();
    }

  vsreopen(out_u);
  if (Mem_to_disk(out_u, out_fname, o_mode) == EOF)
    {
      vsreopen(out_u);
      g_err(FILE_ERROR, out_fname);
    }

  term();
  if (lon)
    {
      (void)fprintf(vdu, ps_name, ft_in, in_fname);
      print_size(infile_recs);
      (void)fprintf(vdu, ps_name, ft_out, out_fname);
      print_size( vstell(out_u) );
#if UNIX
      say("Edit Finished.");
#else  /* if UNIX */
      putstr("Edit Finished.");
#endif  /* if UNIX */
    }

  _exit(0);
}

/*
 *  The List verb.  This accepts LON, LOFF to switch listing
 *  on and off, LD to print details of all the open files,
 *  LM list details of currently defined macros, macros and
 *  finaly the normal GE L verb for which most of work is done
 *  by PT and the syntax is the same.
 */

/*
 *  Print out details of all internal files.
 */

private
void
Details(void)
{
  const FILE_LIST *p = f_list;
  const UNIT *u, *prim_in = NULL, *prim_out = NULL, *old_in = in_u,
    *old_out = out_u;

  while (p != NULL)
    {
      if (p->disp == 'M')
        {
          if ( isprimary(old_in) )
            {
              prim_in = old_in;
            }

          print_i_size(ft_merge, p->name, old_in);
          old_in = p->old_u;
          if ( isprimary(old_in) )
            {
              prim_in = old_in;
            }
        }
      else
        {
          if ( isprimary(old_out) )
            {
              prim_out = old_out;
            }

          print_o_size("Save", p->name, old_out);
          old_out = p->old_u;
          if ( isprimary(old_out) )
            {
              prim_out = old_out;
            }
        }

      p = p->next;
    }

  print_i_size(ft_in, in_fname, prim_in != NULL ? prim_in : in_u);
  print_o_size(ft_out, out_fname, prim_out != NULL ? prim_out : out_u);

  if (trans_u != NULL)
    {
      print_i_size("Transient", t_fname, trans_u);
    }

  for (u = comm_u, p = c_list; p != NULL; p = p->next)
    {
      print_i_size("Command", p->name, u);
      u = p->old_u;
    }
}

/*
 *  Set line number mode, define number format.
 */

private
void
Numbers(VERB csc opts)
{
  if (opts->o1.q == NO_OPT)
    {
      l_numbers = !l_numbers;
    }
  else
    {
      ++l_numbers;
      (void)sprintf(
              n_format,
              "%%%.*sl%c",
              opts->o1.v,
              opts->o1.s,
              opts->o2.v);
    }
}

#if FULL_G

private
void
Window(void)
{
  UNIT  t_in = *in_u, t_out = *out_u;
  int   len, last, start;
  const int c_g_rec = g_rec, c_eof = vssizeof(in_u);
  const byte *rec;
  LINE  buf;

  term();
  new_line();

  last = vstell(out_u);
  if ( ( start = last - 8 ) < 0 )
    {
      start = 0;
    }

  if (start == 0)
    {
      say("********* TOF *********");
    }

  t_out.eof_rec = last;
  vsseek(&t_out, start);
  g_rec = start - 1;

  while (start++ < last)
    {
      ++g_rec;
      len = vsgetrec(&t_out, &rec);
      n_print(buf, ltabex(buf, rec, len), NO);
    }

  ++g_rec;
  if (g_eof)
    {
      n_print(eof_mess, 29, YES);
    }
  else
    {
      len = make_line(buf);
      n_print(buf, len, YES);

      if ( ( last = start + 8 ) > c_eof )
        {
          last = c_eof;
        }

      while (start++ < last)
        {
          ++g_rec;
          len = vsgetrec(&t_in, &rec);
          n_print(buf, ltabex(buf, rec, len), NO);
        }

      if (last == c_eof)
        {
          say(eof_mess);
        }
    }

  new_line();
  g_rec = c_g_rec;
}

#endif  /* if FULL_G */

private
void
List(VERB csc opts)
{
  const MACRO *macptr;
  SAVE_AREA l_save;
  stack *p;
  LINE  buf;
  int   len;

  if (opts->o1.q != L_LOFF)
    {
      term();
    }

  switch (opts->o1.q)
    {
    case L_LON:
      ++lon;
      break;

    case L_LOFF:
      lon = NO;
      break;

    case 'D':  /* list details of files */
      Details();
      break;

    case 'H':
      for ( p = hist_top; p != NULL; p = (stack *)( *p ) )
        {
          short csc start = (short *)( p + 1 );
          (void)fprintf( vdu, "%.*s\n", *start, (char *)( start + 1 ) );
        }

      break;

    case 'M':
      for (macptr = mac_list; macptr != NULL; macptr = macptr->next)
        {
          (void)fprintf(
            vdu,
            "%s/%c%d/ %s\n",
            macptr->name,
            macptr->par_sub,
            macptr->nargs,
            macptr->text);
        }

      break;

    case 'X':
      hex_print();
      break;

    case 'S':
      word_count();
      break;

    default:
      if (g_eof)  /* can't list nothing */
        {
          return;
        }

      len = make_line(buf);
      if (opts->o1.q == RECS && opts->o1.v == 0)
        {
          buf[len] = EOS;
          say(buf);
          break  /* we've listed it */;
        }

      if (lon)
        {
          if (f_list != NULL && f_list->disp == 'M')
            {
              (void)fprintf(vdu, pt_list, ft_merge, f_list->name);
            }
          else
            {
              (void)fprintf(vdu, pt_list, ft_in, in_fname);
            }

          (void)fprintf(vdu, ", starting at line %d\n", g_rec);
        }

      n_print(buf, len, YES);

      l_save.in_rec_len = l_save.out_rec_len = E_BUFF_SIZE;
      l_save.in_rec = buf, l_save.out_rec = e_buff;
      save_all(&l_save);
      (void)PT(opts);
      rest_all(&l_save);

      if (lon)
        {
          if (g_eof)
            {
              if (f_list != NULL && f_list->disp == 'M')
                {
                  (void)fprintf(vdu, "End of merge file %s", f_list->name);
                }
              else
                {
                  (void)fprintf(vdu, "End of input file %s", in_fname);
                }

              (void)fprintf(vdu, ", last line = %d\n", g_rec - 1);
            }

          new_line();
        }
    }
}

/*
 *  Translate the character under the cursor.
 */

private
void
Xlate(VERB csc opts)
{
  char c;

  if (i_col >= i_eor)
    {
      if (depth)
        {
          return;
        }

      g_err(END_OF_LINE, NULL);
    }

  c = xlat(i_buff[i_col++], opts->o1.s, (const byte *)opts->o2.s);

  ins(0, &c, 1);
}

/*
 *  Change File Names.
 */

private
void
Oldfile(VERB csc opts)
{
  const  char *nf = opts->o1.s;
  static char *old_new;

  if (f_list != NULL)
    {
      g_err(f_list->disp == 'M' ? W_IN_MERGE : SAV_ON_STACK, NULL);
    }

  if (opts->o1.v == 0)  /* no filename given */
    {
      if (i_mode == 'F')
        {
          g_err(NO_RE_READ, opts->errp);
        }

      if ( ( nf = in_fname ) == no_file )
        {
          nf = in_fname = out_fname;
        }
    }
  else
    {
      rlsevec(old_new);
      out_fname = in_fname = old_new = getbuf( size(nf) );
      (void)zmovelr(old_new, nf);
    }

  i_mode = o_mode = 'N';

  vsreload();

  if (fscreen)
    {
      redisplay = SE_DISP;
    }

  prim_changed = NO;
}

/*
 *  Process a normal verb during execution.
 */

private
int
verb(VERB csc v)
{
  int rc;

  switch (v->comm)
    {
    case 'K':
      linewrap();  /* delete entire lines */
      /* fallthrough */

    case 'P':
      /* fallthrough */
    case 'T':
      return PT(v);

    case 'E':
      Exit();
      abort();
      /* fallthrough */

    case 'Q':
      Quit();
      abort();
      /* fallthrough */

#if FULL_G
    case 'W':
      Window();
      break;
#endif  /* if FULL_G */

    case 'I':
    case 'D':
      Insert(v);
      break;

    case 'A':
    case 'B':
    case 'R':
      ABR(v);
      break;

    case 'S':
      Save(v);
      break;

    case 'M':
      Merge(v);
      break;

    case 'U':
      Use(v);
      break;

    case 'X':
      Xit(v);
      break;

    case 'V':
      if ( ( rc = Verify(v) ) != G_OK && !depth )
        {
          g_err(VERIFY_FAIL, v->errp);
        }

      return rc;

    case 'O':
      Oldfile(v);
      break;

    case 'Y':
      Xlate(v);
      break;

    case 'N':
      Numbers(v);
      break;

    case '{':
      if (v->o1.q == C_REPEAT)
        {
          Calc(v->o1.e);
        }
      else
        {
          (void)Evaluate(v->o1.e, v->o1.q);
        }

      break;

#if !defined(OMIT_SYSTEM)
    case '!':
      term();
      (void)!system( prep_name(v->o1.s) );
      break;
#endif  /* if !defined(OMIT_SYSTEM) */

    case ':':
      if (fscreen)
        {
          save_jbuf(set_err, save_err);
          longjmp(se_ret, YES);
        }

      if (f_list != NULL)
        {
          g_err(SM_SE, cmd_buf);
        }

      Screen_ed();
      pop_com_stack(&com_stack);
      break;

    case 'J':
      return Join(v);

    case 'L':
      List(v);
      break;

    case 'F':
      rest_all(&g_save);
      break;

    case 'H':
#if TINY_G
      Help();
#else  /* if TINY_G */
      Help(v);
#endif  /* if TINY_G */
    }

  return G_OK;
}

/*
 *  Process iteration during execution.
 */

private
int
while_loop(VERB csc vptr, VERB csc prog)
{
  const int endc = ( vptr->o1.q & OP_UNTIL ) == 0;

  while (Verify(vptr) != endc)
    {
      if ( George(prog) )
        {
          return G_FAIL;
        }
    }
  return G_OK;
}

private
int
rec_loop(VERB csc vptr, VERB csc prog)
{
  int   val;
  const word opt = vptr->o1.q;

  if (opt & OP_CALC)
    {
      val = (int)Evaluate(vptr->o1.e, C_ENDP)->opval.i;
    }
  else
    {
      val = vptr->o1.v;
    }

  switch (opt & BYTE_MASK)
    {
    default:
      g_err(I_REPEAT, vptr->errp);
      /* fallthrough */

    case MRECS:
      val = -val;
      /* fallthrough */

    case RECS:
      val += g_rec;
      /* fallthrough */

    case R_END:
      if (val < 0)
        {
          g_err(NO_BACK, vptr->errp);
        }

      while (g_rec < val && !g_eof)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case OR_END:
      while (o_rec < val)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case R_TIMES:
      while (val--)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case OP_EOF:
      while (!g_eof)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case STR_END:
      while (findstr(vptr->o1.s, val, opt, 'V') && !g_eof)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
    }

  return G_OK;
}

private
int
line_loop(VERB csc vptr, VERB csc prog)
{
  int  val;
  const word opt = vptr->o2.q;

  if (opt & OP_CALC)
    {
      val = (int)Evaluate(vptr->o2.e, C_ENDP)->opval.i;
    }
  else
    {
      val = vptr->o2.v;
    }

  switch (opt & BYTE_MASK)
    {
    default:
      g_err(I_REPEAT, vptr->errp);
      /* fallthrough */

    case MRECS:
      val = -val;
      /* fallthrough */

    case RECS:
      val += i_col;
      /* fallthrough */

    case R_END:
      if (val < i_col)
        {
          g_err(NO_BACK, vptr->errp);
        }

      while (i_col < val && i_col < i_eor && !g_eof)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case OR_END:
      while (e_col <= val && !g_eof)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case OP_EOF:
      while (i_col < i_eor && !g_eof)
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case 'S':
      if (val)
        {
          g_err(NO_BACK, vptr->errp);
        }

      while ( i_col < i_eor && !g_eof && isspace(i_buff[i_col]) )
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
      break;

    case 'G':
    case 'r':
      while ( i_col < i_eor && !g_eof && findstr(vptr->o2.s, val, opt, 'V') )
        {
          if ( George(prog) )
            {
              return G_FAIL;
            }
        }
    }

  return G_OK;
}

/*
 *  Execute compiled code, also called recursively to deal with loop clauses.
 */

private
int
George(const VERB *v)
{
  int   rc = G_OK;
  const VERB *prog;

  while (v != NULL)
    {
      switch (v->comm)
        {

        /* deal with repetition first */

        case '(':
          prog = v->next;
          v = v->cpar;

          /* indicate in repetition */

          ++depth;
          if (v->o1.q & LOOP_MASK)
            {
              rc = while_loop(v, prog);
            }

          elif(v->dot)
              rc = line_loop(v, prog);
          else
            {
              rc = rec_loop(v, prog);
            }
          --depth;
          break;

        case ';':
          if (depth)
            {
              v = v->cpar;
            }

          return rc;

        case ')':
          return rc;

        default:
          rc = verb(v);
        }

      /* skip to conditional on failure */

      if (rc != G_OK && depth)
        {
          do
            {
              v = v->next;
              if (v->comm == ')')
                {
                  return rc;
                }

              if (v->comm == '(')
                {
                  v = v->cpar;
                }
            }
          while ( v->comm != ';' );
          rc = G_OK;
        }

      /* otherwise move on to next verb */

      v = v->next;
    }

  return G_OK;
}

/*
 *  The line editor.
 */

#if FULL_G

private
void
Line_ed(char *ptr)
{
  int len, n;
  char ch, *h_eor, *h_ptr;

  /* execute a t.e */

  get_end();
  h_eor = e_buff + e_col;
  h_ptr = e_buff + buff_sec * L_LEN;

  repeat
  {
    switch (ch = *ptr++)
      {
      case SPACE:
        if (++h_ptr > h_eor)
          {
            *h_eor++ = SPACE;
          }

        /* bump pointer */

        continue;

      case '-':  /* look for more - together */
        n = 1;
        while (*ptr++ == '-')
          {
            ++n;
          }

        --ptr;

        if (h_ptr > h_eor)
          {
            continue;
          }

        if (h_ptr + n > h_eor)
          {
            n = h_eor - h_ptr;
          }

        /* adjust pointer */

        h_eor -= n;

        /* adjust buffer */

        (void)movelr( h_ptr, h_ptr + n, (int)( h_eor - h_ptr ) );
        continue;

      case '^':  /* insert text or split line */
        if (*ptr == EOS)  /* split */
          {
            if (h_ptr >= h_eor)
              {
                g_err(N_T_S, NULL);
              }

            e_col = h_ptr - e_buff;
            h_eor -= e_col;
            out_buff();
            (void)movelr( e_buff, h_ptr, (int)( h_eor - e_buff ) );
            buff_sec = 0;
            break;
          }

        len = strlen(ptr);
        if (h_ptr > h_eor)
          {
            h_eor = mmovelr(h_ptr, ptr, len);
          }
        else
          {
            (void)moverl( h_ptr + len, h_ptr, (int)( h_eor - h_ptr ) );
            (void)movelr(h_ptr, ptr, len);
            h_eor += len;
          }

        break;

      case ESC:
        if ( ( ch = *ptr++ ) != EOS )
          {
            goto put_it_in;
          }

      case EOS:
        break;

      case '%':
        ch = SPACE;
        /* fallthrough */

      default:
put_it_in:  /* replace chars on the line */
        *h_ptr++ = ch;
        if (h_ptr > h_eor)
          {
            h_eor++;
          }

        continue;

      case '!':  /* replace */
        h_eor = mzmovelr(h_ptr, ptr);
      }

    break;
  }

  /* end of command line so wrap up to return to george */

  e_col = h_eor - e_buff;
}

#endif  /* if FULL_G */

/*
 *  Main driver, may be called recursively by U verb & screen editor
 */

private
void
Drive(const int level)
{
  char ch, *s;
  int save, rc = 0, len, done = NO;
  string comm;

  if (level == 0 && ( rc = setjmp(set_err) ) == NO)
    {
      /* startup */
#if !defined(__MINGW32__) && !defined(OMIT_SIGNAL)
      (void)signal(SIGINT, g_intr);
# if UNIX
      (void)signal(SIGTERM, g_intr);
      (void)signal(SIGUSR1, g_intr);
      (void)signal(SIGUSR2, g_intr);

      if (signal(SIGHUP, g_intr) == SIG_IGN)
        {
          (void)signal(SIGHUP, SIG_IGN);
        }

      (void)signal(SIGPIPE, g_intr);
      (void)signal(SIGQUIT, g_intr);
# endif  /* if UNIX */
#endif  /* if !defined(__MINGW32__) && !defined(OMIT_SIGNAL) */
      save_jbuf(save_err, set_err);
      if (g_init == NULL)
        {
          Screen_ed();
          pop_com_stack(&com_stack);
        }
    }

  elif(rc != 0)  /* failure */
      pop_com_stack(&com_stack);

  else
    {  /* new instance */
      VERB_LIST *p = com_stack_ptr->next;
      if (p == NULL)
        {
          p = com_stack_ptr->next = heap(VERB_LIST);
          p->prev = com_stack_ptr;
          p->next = NULL;
        }

      com_stack_ptr->save_depth = depth;
      com_stack_ptr = p;
      p->prog = NULL;
      depth = 0;
    }

  repeat
  {
    running = NO;

    if (com_stack_ptr->prog != NULL)
      {
        free_prog(com_stack_ptr);
      }

    if (level == D_SE_HOME || level == D_SE_AUTO)
      {  /* execute whats in cmd_buf */
        if (done++)
          {
            break;
          }

        (void)zmovelr(comm, cmd_buf);
      }
    else
      {
        printline(comm);
        if ( get_com(comm, comm) )
          {
            break;
          }
      }

    s_g_rec = g_rec;
    s_g_col = e_col;

    switch (*comm)
      {
      case '*':
        (void)add_line('T');
        /* fallthrough */

      case '=':
        if ( ( len = hist_recall(comm) ) > 0 )
          {
            comm[len] = EOS;
            if (!fscreen && lon)
              {
                say(comm);
              }
          }

        break;

      case SPACE:
        if (level != D_LINE_USER)
          {
            for (s = comm; isspace(*s); ++s)
              {
                ;
              }

            (void)zmovelr(comm, s);
          }
      }

    if ( ( ch = u_star(comm) ) == ';' )
      {
        continue;
      }

    if (level == D_LINE_USER)
      {
        switch (ch)
          {
          case '$':
            if ( isdigit(comm[1]) )
              {
                buff_sec = comm[1] - '0';
              }

#if FULL_G
            else
              {
                show_template = !show_template;
              }
            continue;

          case SPACE:
            if (comm[1] != SPACE)
              {
                g_err(I_COMMAND, comm);
              }

            save_all(&g_save);
            Line_ed(comm + 2);

#else  /* if FULL_G */
          case SPACE:
#endif  /* if FULL_G */
            continue;

          case EOS:
            /* null string defined as t1 */
            (void)add_line('T');
            continue;
          }
      }

    elif(ch == '$' || ch == EOS)
        continue;

    save = level < D_USE_FILE && ch != 'H' && ch != 'F';
    if (save)
      {
        hist_write(comm);
      }

    par_stack_ptr = com_stack_ptr;
    G_compile(&com_stack_ptr->prog, comm);
    ++running;
    if (com_stack_ptr->prog != NULL)
      {
        if (par_stack_ptr != com_stack_ptr)
          {
            char csc erp = par_stack_ptr->prog->errp;
            par_stack_ptr = com_stack_ptr;
            g_err(MISSING_BRA, erp);
          }

        if (save)
          {
            save_all(&g_save);
          }

        (void)George(com_stack_ptr->prog);
      }
  }

  pop_com_stack(com_stack_ptr->prev);
}

/* G */

#if DOS
void
#else  /* if DOS */
int
#endif  /* if DOS */
main(int i, char csc * argv)
{
  const char *p, *files[2];
  PAGE_PTR *cp;
  char cvstring[255];

  (void)cvstring;
  i = 0;  /* scan .argtype */
  while ( ( p = *++argv ) != NULL )
    {
      if (p[0] == '-' && p[1] != EOS)
        {
          switch ( u_star(++p) )
            {
            case 'B':  /* binary */
              ++bin_mode;
              continue;

            case 'R':  /* read only */
              ++ro_mode;
              continue;

            case 'S':  /* soft tab width on screen */
              if (*++p == EOS && argv[1])
                {
                  p = *++argv;
                }

              if ( ( screen_tabs = atoi(p) ) <= 0 )
                {
                  screen_tabs = TAB_WIDTH;
                }

              continue;

            case 'V':  /* display version */
#if defined(NCURSES_VERSION) || defined(PDCURSES)
              sprintf( cvstring,
                       "%s using %s",
                       VERSION_STRING,
                       curses_version() );
# if DOS
              putstr(
# else  /* if DOS  */
              say(
# endif  /* if DOS */
                cvstring);
#else  /* if defined(NCURSES_VERSION) || defined(PDCURSES) */
# if DOS
              putstr(
# else  /* if DOS  */
              say(
# endif  /* if DOS */
                VERSION_STRING);
#endif  /* if defined(NCURSES_VERSION) || defined(PDCURSES) */
              _exit(0);

            case 'C':  /* initial command */
              ++p;
              /* fallthrough */

            default:
              if (*p != EOS)
                {
                  g_init = p;
                }
              else
                {
                  g_init = argv[1] ? *++argv : ";";
                }

              continue;
            }
        }

      if (i > 1)
        {
          say("Usage: g [ -rscbv ] [ file [ newfile ] ]");
          _exit(1);
        }

      files[i++] = p;
    }

  switch (i)
    {
    case 0:  /* stdin to stdout */
      i_mode = o_mode = 'F';
      break;

    case 1:  /* only one file */
      in_fname = out_fname = files[0];
      break;

    case 2:  /* both files supplied */
      in_fname = files[0];
      out_fname = files[1];
    }

  if ( equal1(in_fname, '-') )
    {
      i_mode = 'F';
    }

  if ( equal1(out_fname, '-') )
    {
      o_mode = 'F';
    }

  i = isatty(0);
  if (!i)
    {
      fd_in_terminal = dup(0);
      (void)close(0);
      if (open(tty_file, O_RDONLY) == -1)
        {
          g_err(FILE_ERROR, tty_file);
        }
    }

  if (i_mode == 'F')
    {
      in_fname = i ? no_file : si_file;
    }

  if (o_mode == 'F')
    {
      out_fname = so_file;
      fd_out_terminal = dup(1);
      (void)close(1);
      if (open(tty_file, O_WRONLY) == -1)
        {
          g_err(FILE_ERROR, tty_file);
        }
    }

  /* get initial buffers */

  in_u->list = cp = (PAGE_PTR *)getvec(PAGE_SIZE * 2 + E_BUFF_SIZE * 2);
  in_u->rec_start = (byte *)( cp + PPP );
  i_buff = (char *)( cp + PPP * 2 );
  e_buff = i_buff + E_BUFF_SIZE;

  /* load input file */

  vsprimary();

  /* load command file if from stdin */

  if (!i && i_mode != 'F' && Disk_to_mem(empty, trans_u = vsopen(), 'F') > 0)
    {
      vsreopen(trans_u);
      g_init = "u";
    }

  Drive(D_LINE_USER);

#if !(DOS)
  return 0;
#endif  /* if !(DOS) */
}
