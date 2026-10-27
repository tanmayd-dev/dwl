#!/bin/sh
# Build-check for local/main: compile the working tree without a config.h.
#
# The tracked configuration is config.local.h; config.h is a gitignored,
# per-machine override. This script copies the tracked files (with uncommitted
# working-tree edits, but never the gitignored config.h) into a temporary
# directory and builds there, so it proves that a clean checkout is
# reproducible. Nothing in the repository is modified or installed.
#
# Usage: scripts/build-check.sh
# Exit:  0 on a successful build, non-zero otherwise.

set -eu

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
builddir=$(mktemp -d "${TMPDIR:-/tmp}/dwl-build-check.XXXXXX")
trap 'rm -rf "$builddir"' EXIT INT TERM

# tar reads the files from the working tree, so local edits are tested while
# git ls-files keeps generated/ignored files (config.h, binaries, protocol
# headers) out of the check. --others includes not-yet-committed sources such
# as a new config.local.h; .gitignore still excludes config.h.
git -C "$repo" ls-files -z --cached --others --exclude-standard |
	tar -C "$repo" --null -T - -cf - | tar -C "$builddir" -xf -

if [ ! -f "$builddir/config.local.h" ]; then
	echo "build-check: config.local.h is missing from the tracked files" >&2
	exit 1
fi
if [ -e "$builddir/config.h" ]; then
	echo "build-check: config.h leaked into the clean tree" >&2
	exit 1
fi

echo "build-check: building tracked local/main in $builddir (no config.h)"
make -C "$builddir"

if [ ! -f "$builddir/config.h" ]; then
	echo "build-check: build did not generate config.h from config.local.h" >&2
	exit 1
fi
if ! cmp -s "$builddir/config.h" "$builddir/config.local.h"; then
	echo "build-check: generated config.h differs from config.local.h" >&2
	exit 1
fi

echo "build-check: OK (config.h generated from config.local.h)"
