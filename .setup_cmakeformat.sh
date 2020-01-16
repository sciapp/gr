#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
VENV_NAME=".cmakeformat_venv"


install_cmakeformat () {
    if [[ ! -d "${VENV_NAME}" ]]; then
        python3 -m venv "${VENV_NAME}" || return
    fi
    source "${VENV_NAME}/bin/activate" && \
    pip -qqq install -U cmake_format six && \
    deactivate && \
    echo "$(cd "${VENV_NAME}" && pwd)/bin/cmake-format"
}


main () {
    cd "${SCRIPT_DIR}"
    # If `cmake-format` is already installed on the system, use that version
    command -v "cmake-format" && return
    install_cmakeformat
}

main "$@"
