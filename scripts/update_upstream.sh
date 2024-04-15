#!/bin/bash

if [[ $# -lt 1 ]] || [[ $# -gt 2 ]] || ! [[ -r "$1" ]]; then
  echo 1>&2 "USAGE: $0 NEW_K2PDFOPT_ARCHIVE [UPSTREAM_BRANCH]"
  exit 1
fi

archive="$(realpath "$1")"
shift
if [[ $# -ne 0 ]]; then
    upstream="$1"
    shift
else
    upstream='upstream'
fi

set -xeo pipefail

# Prepare temporary worktree.
git worktree add upstream "${upstream}"
trap 'git worktree remove upstream' EXIT
pushd upstream
# Remove tracked files.
git ls-files -z | xargs -0 --no-run-if-empty rm
# Remove empty trees.
find -mindepth 1 -type d -name '.git*' -prune -o -empty -printf '%P\0' \
    | xargs -0 --no-run-if-empty rmdir -p --ignore-fail-on-non-empty
# Import archive content.
bsdtar xf "${archive}" --strip-components 1
# Add new files.
git add --all .
git commit -m "$(basename "${archive%.*}")"

# vim: sw=4
