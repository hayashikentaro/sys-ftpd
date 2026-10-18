#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"
# Same devkitA64 image used for the local nxdumptool build.
image="${SYS_FTPD_BUILD_IMAGE:-devkitpro/devkita64@sha256:1fc388c3a0d34bd2045a6dadcb1020e069d5f876a187fd705de14b4440c00282}"
docker run --rm -v "$PWD:/work" -w /work "$image" make "$@"
