#!/bin/sh

SCRIPT_DIR="$(cd "$(dirname "$0")" >/dev/null 2>&1 && pwd)"

"${SCRIPT_DIR}/../Applications/grm-plots.app/Contents/Macos/grm-plots" "$@"
