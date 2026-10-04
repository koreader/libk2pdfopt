#!/bin/sh

ANSI_DIM="$(printf '%b' '\e[2m')"
ANSI_GREEN="$(printf '%b' '\e[32;1m')"
ANSI_RED="$(printf '%b' '\e[31;1m')"
ANSI_RESET="$(printf '%b' '\e[0m\e[1G')"

for prog in "$@"; do
    case "${prog}" in
        env | *=* | */buildcache | */npx) continue ;;
        *) break ;;
    esac
done
prog="${prog##*/}"
for file in "$@"; do :; done
case "${file}" in *=*) file="${file##*=}" ;; esac

synopsis="${prog} ${file}"
timefmt="${ANSI_DIM}${synopsis}: %U user %S system %E total${ANSI_RESET}"

output="$(command time -f "${timefmt}" "$@" 2>&1)"
code=$?

if [ ${code} -eq 0 ]; then
    format="${ANSI_GREEN}"
    extra=''
else
    format="${ANSI_RED}"
    extra=" [${code}]"
fi

printf '::group::%s%s%s%s\n' "${format}" "${synopsis}" "${extra}" "${ANSI_RESET}"
printf '%s\n' "${output}"
printf '::endgroup::\n'

exit ${code}
