#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

install_dependencies() {
    if ! command -v apt-get >/dev/null 2>&1; then
        echo "Error: apt-get was not found. Install g++, python3 and matplotlib manually."
        exit 1
    fi

    SUDO=""
    if [ "$(id -u)" -ne 0 ]; then
        if ! command -v sudo >/dev/null 2>&1; then
            echo "Error: sudo was not found. Install g++, python3 and matplotlib manually."
            exit 1
        fi
        SUDO="sudo"
    fi

    echo "Installing dependencies: g++, python3, python3-matplotlib"
    $SUDO apt-get update
    $SUDO apt-get install -y g++ python3 python3-matplotlib
}

need_install=0

if ! command -v g++ >/dev/null 2>&1; then
    need_install=1
fi

if ! command -v python3 >/dev/null 2>&1; then
    need_install=1
elif ! python3 -c "import matplotlib" >/dev/null 2>&1; then
    need_install=1
fi

if [ "$need_install" -eq 1 ]; then
    install_dependencies
fi

EXE=viterbi_task
trap 'rm -f "$EXE"' EXIT

mkdir -p output

g++ -std=c++17 -O2 -Wall -Wextra -Iinclude \
    cpp/main.cpp cpp/ConvolutionalCode.cpp cpp/BinarySymmetricChannel.cpp \
    -o "$EXE"

"./$EXE" "$@"

python3 python/plot_ber.py output/viterbi_ber.csv output/viterbi_ber.png

echo
echo "Done."
echo "CSV:  output/viterbi_ber.csv"
echo "Plot: output/viterbi_ber.png"
