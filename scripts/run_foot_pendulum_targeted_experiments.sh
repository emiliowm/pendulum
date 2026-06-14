#!/usr/bin/env bash
set -euo pipefail

WANDB_PROJECT="${WANDB_PROJECT:-raised-foot-pendulum}"
TIMESTEPS="${TIMESTEPS:-10000000}"
EXP_PREFIX="${EXP_PREFIX:-m1_clamp}"
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

run_exp "${EXP_PREFIX}_default_10m"
run_exp "${EXP_PREFIX}_catch_stillness_10m" \
    --env.catch-bonus-weight 6.0 \
    --env.stillness-sigma 0.75
run_exp "${EXP_PREFIX}_smooth_hanging_10m" \
    --env.action-rate-penalty-weight 0.16 \
    --env.reset-hanging-pct 70 \
    --env.reset-near-upright-pct 15
run_exp "${EXP_PREFIX}_lower_entropy_lr_10m" \
    --train.learning-rate 0.0005 \
    --train.ent-coef 0.003
run_exp "${EXP_PREFIX}_longer_horizon_10m" \
    --env.min-horizon-steps 360 \
    --env.max-horizon-steps 720
