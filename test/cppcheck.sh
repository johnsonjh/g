#!/usr/bin/env sh

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

# shellcheck disable=SC2086
eval ${CPPCHECK:-cppcheck}                                           \
  "--force"                                                          \
  "--inline-suppr"                                                   \
  "--suppress=unknownMacro"                                          \
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

eval ${RM:-rm} "-f" "${tmpfile:?}"  \
  2> "/dev/null"
