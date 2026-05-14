#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

install_dependencies() {
    if ! command -v apt-get >/dev/null 2>&1; then
        echo "Error: apt-get was not found. Install g++ manually."
        exit 1
    fi

    SUDO=""
    if [ "$(id -u)" -ne 0 ]; then
        if ! command -v sudo >/dev/null 2>&1; then
            echo "Error: sudo was not found. Install g++ manually."
            exit 1
        fi
        SUDO="sudo"
    fi

    echo "Installing dependency: g++"
    $SUDO apt-get update
    $SUDO apt-get install -y g++
}

if ! command -v g++ >/dev/null 2>&1; then
    install_dependencies
fi

EXE=fft_task
trap 'rm -f "$EXE"' EXIT

mkdir -p output

g++ -std=c++17 -O2 -Wall -Wextra -Iinclude \
    cpp/main.cpp cpp/FastFourierTransform.cpp \
    -o "$EXE"

"./$EXE" "$@"

echo
echo "Done."
echo "Report: output/fft_report.txt"
echo "CSV: output/fft_errors.csv"
