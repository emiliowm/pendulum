#!/usr/bin/env bash
set -euo pipefail

WANDB_PROJECT="${WANDB_PROJECT:-raised-foot-pendulum}"
TIMESTEPS="${TIMESTEPS:-10000000}"
COMMON=(
    --wandb
    --wandb-project "$WANDB_PROJECT"
    --train.total-timesteps "$TIMESTEPS"
)

bash build.sh foot_pendulum

run_exp() {
    local tag="$1"
    shift
    puffer train foot_pendulum --tag "$tag" "${COMMON[@]}" "$@"
}

run_exp m1_fix_default_10m
run_exp m1_fix_catch_stillness_10m \
    --env.catch-bonus-weight 6.0 \
    --env.stillness-sigma 0.75
run_exp m1_fix_smooth_hanging_10m \
    --env.action-rate-penalty-weight 0.12 \
    --env.reset-hanging-pct 70 \
    --env.reset-near-upright-pct 15
run_exp m1_fix_lower_entropy_lr_10m \
    --train.learning-rate 0.0005 \
    --train.ent-coef 0.003
run_exp m1_fix_longer_horizon_10m \
    --env.min-horizon-steps 360 \
    --env.max-horizon-steps 720
