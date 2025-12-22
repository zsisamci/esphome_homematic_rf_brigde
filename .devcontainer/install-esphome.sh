#!/usr/bin/env bash
set -e

# install uv
curl -LsSf https://astral.sh/uv/install.sh | sh

#install esphome (using python 3.12)
uv tool install --python 3.12 esphome

echo "esphome installed"

