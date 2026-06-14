#!/usr/bin/env bash
set -euo pipefail
bash build.sh foot_pendulum
puffer train foot_pendulum --wandb --wandb-project raised-foot-pendulum --tag "${1:-m1_train}" "${@:2}"
