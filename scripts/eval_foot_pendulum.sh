#!/usr/bin/env bash
set -euo pipefail
puffer eval foot_pendulum --load-model-path "${1:-latest}"
