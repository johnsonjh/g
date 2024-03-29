#!/usr/bin/env sh
# shellcheck disable=SC2086
# vi: filetype=sh:tabstop=8:tw=79:expandtab
###############################################################################

###############################################################################
#
# Copyright (c) 2019-2023 Jeffrey H. Johnson <trnsz@pobox.com>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered "AS-IS",
# without any warranty.
#
###############################################################################

${TEST:-test} "-f" "g.c" ||
  {
    ${PRINTF:-printf} '%s\n'  \
      "Error: \"g.c\" not found."
    exit 1
  }

tmpfile="$( ${PRINTF:-printf} '%s'             \
              'mkstemp(/tmp/cpptmp.XXXXXX)' |  \
                ${M4:-m4} 2> "/dev/null"       \
          )"

${TEST:-test} "-z" "${tmpfile:-}" &&
  {
    ${PRINTF:-printf} '%s\n'  \
      "Error: mkstemp() failed."
    exit 1
  }

eval trap                                                \
  "eval ${RM:-rm} -f \"${tmpfile:?}\" 2> \"/dev/null\""  \
    SIGINT                                               \
    SIGQUIT                                              \
    SIGABRT                                              \
    SIGTERM                                              \
    EXIT                                                 \
      2> "/dev/null"

${TEST:-test} -z "${HTMLOUT:-}" ||
  {
    XMLARGS="--xml --xml-version=2"
    export XMLARGS
  }

eval ${CPPCHECK:-cppcheck} ${XMLARGS:-}                              \
  "--force"                                                          \
  "--inconclusive"                                                   \
  "--inline-suppr"                                                   \
  "--max-ctu-depth=16"                                               \
  "--suppress=unknownMacro"                                          \
  "--suppress=syntaxError:/usr/include/stdlib.h"                     \
  "--enable=warning,portability,performance,unusedFunction"          \
  "$( ${CC:-cc} "-E" "-Wp,-v" "-" < "/dev/null" 2>&1 |               \
      ${GREP:-grep} '^ \/' 2> "/dev/null" |                          \
      ${TR:-tr} "-d" '\"' |                                          \
      ${SED:-sed} "-e" 's/^ /-I\"/' "-e" 's/$/\"/' 2> "/dev/null" |  \
      ${TR:-tr} '\n' ' '                                             \
    )"                                                               \
  "-D__CPPCHECK__=1"                                                 \
  "--std=c99"                                                        \
  "g.c"                                                              \
    2> "${tmpfile:?}" &&                                             \
      ${TEST:-test} -f "${tmpfile:?}" ;                              \
        ${PRINTF:-printf} '\n%s\n' "## Start of Cppcheck output" ;   \
          ${CAT:-cat} "${tmpfile:?}" 2> "/dev/null" ;                \
            ${PRINTF:-printf} '%s\n' "## End of Cppcheck output"

${TEST:-test} -z "${HTMLOUT:-}" ||
  {
    ${MKDIR:-mkdir} "-p" "../cppcheck.out"  \
      2> "/dev/null"
    ${TEST:-test} -f "${tmpfile:?}" &&
      ${TEST:-test} -d "../cppcheck.out" &&
        {
          ${RM:-rm} -f "../cppcheck.out"/*  \
            2> "/dev/null"
          ${CPPCHECK_HTMLREPORT:-cppcheck-htmlreport}  \
            --source-dir="."                           \
            --title="G"                                \
            --file="${tmpfile:?}"                      \
            --report-dir="../cppcheck.out"
        }
    }

eval ${RM:-rm} "-f" "${tmpfile:?}"  \
  2> "/dev/null"
