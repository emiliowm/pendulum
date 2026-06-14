#!/usr/bin/env bash
set -euo pipefail
bash build.sh foot_pendulum
for catch_bonus_weight in 2.0 3.0 5.0; do
  for stillness_sigma in 1.0 1.5; do
    puffer train foot_pendulum \
      --wandb \
      --wandb-project raised-foot-pendulum \
      --tag "m1_cb_${catch_bonus_weight}_sigma_${stillness_sigma}" \
      --env.catch_bonus_weight "${catch_bonus_weight}" \
      --env.stillness_sigma "${stillness_sigma}"
  done
done
