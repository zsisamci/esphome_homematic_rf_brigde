#!/usr/bin/env bash
set -e

# install dependecies
 sudo apt update
 sudo apt install -y cmake ninja-build make autoconf autogen automake libtool 

#install wasi sdk
ARCH=$(dpkg --print-architecture)
if [ "$ARCH" = "amd64" ]; then
    wget -q https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-28/wasi-sdk-28.0-x86_64-linux.deb
    sudo dpkg -i wasi-sdk-*-x86_64-linux.deb
elif [ "$ARCH" = "arm64" ]; then
    wget -q 
    https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-28/wasi-sdk-28.0-arm64-linux.deb
    sudo dpkg -i wasi-sdk-*-arm64-linux.deb
else
    exit 1
rm wasi-sdk-*.deb

# add wasi sdk to path
echo "export PATH=/opt/wasi-sdk/bin:\$PATH" >> ~/.bashrc

echo "WASI SDK installed"

