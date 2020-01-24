#!/bin/bash

CMAKE_FORMAT_VERSION="0.6.7"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
VENV_NAME=".cmakeformat_venv"


install_cmakeformat () {
    if [[ ! -d "${VENV_NAME}" ]]; then
        python3 -m venv "${VENV_NAME}" || return
    fi
    source "${VENV_NAME}/bin/activate" && \
    # Guarantee that all team members use the same version (of dependent packages)
    pip -qqq install --upgrade --upgrade-strategy eager "cmake_format==${CMAKE_FORMAT_VERSION}" && \
    deactivate && \
    echo "$(cd "${VENV_NAME}" && pwd)/bin/cmake-format"
}


main () {
    cd "${SCRIPT_DIR}"
    # Always use a pinned version of `cmake-format`
    install_cmakeformat
}

main "$@"
