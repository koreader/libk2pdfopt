#!/bin/bash

set -eo pipefail

if [[ $# -lt 1 ]] || [[ $# -gt 2 ]]; then
  echo 1>&2 "USAGE: $0 {LEPTONICA|MUPDF|TESSERACT}_CHECKOUT [UPSTREAM_REF]"
  exit 1
fi

checkout="$1"
shift
if [[ $# -ne 0 ]]; then
    upstream="$1"
    shift
else
    upstream='origin/upstream'
fi

mapfile -t mod_dirs < <(git show "${upstream}:" | grep '_mod/$')
mapfile -t mod_files < <(find "${mod_dirs[@]}" -type f)

for file in "${mod_files[@]}"; do
    # Ignore some red herrings.
    case "${file}" in
        tesseract_mod/allheaders.h) continue;;
    esac
    mapfile -t candidates < <(find "${checkout}" -type f -name "${file##*/}" -printf '%P')
    if [[ 0 -eq ${#candidates[@]} ]]; then
        continue
    fi
    if [[ 1 -ne ${#candidates[@]} ]]; then
        echo "error, multiple candidates in checkout for ${file}: ${candidates[*]}" 1>&2
        exit 2
    fi
    diff --ignore-trailing-space --unified --label "a/${candidates[0]}" "${checkout}/${candidates[0]}" --label "b/${candidates[0]}" <(git show "${upstream}:${file}") || [[ $? -eq 1 ]]
done

# vim: sw=4
