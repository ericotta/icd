#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

source ${ICD_HOME:-$HOME/icd}/tools/install_shared.sh

# or try --configure-args="CXXFLAGS=-O0"
R_MAKEVARS_USER="$HOME/.R/Makevars.quick" \
  MAKEFLAGS="-j16" \
  R CMD INSTALL -d \
    --no-byte-compile \
    --no-clean-on-error \
    --library="$install_dir" \
    --install-tests \
    --no-docs \
    "$(ls -t icd*gz | head -1)"
