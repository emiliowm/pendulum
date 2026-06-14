#!/usr/bin/env bash
set -euo pipefail

POLICY_PATH="${1:-latest}"
OUT_DIR="${OUT_DIR:-runs/foot_pendulum_inspect}"
SAVE_FRAMES="${SAVE_FRAMES:-300}"
FPS="${FPS:-30}"
GIF_PATH="${GIF_PATH:-${OUT_DIR}/foot_pendulum_eval.gif}"
TRACE_PATH="${TRACE_PATH:-${OUT_DIR}/foot_pendulum_trace.csv}"
FRAME_DIR="${FRAME_DIR:-${OUT_DIR}/frames}"

mkdir -p "$OUT_DIR" "$FRAME_DIR"

if [ "$(uname -s)" = "Linux" ]; then
    export DISPLAY="${DISPLAY:-:99}"
    export LIBGL_ALWAYS_SOFTWARE="${LIBGL_ALWAYS_SOFTWARE:-1}"
    if ! xdpyinfo -display "$DISPLAY" >/dev/null 2>&1 && [ -x scripts/view_foot_pendulum_vnc.sh ]; then
        scripts/view_foot_pendulum_vnc.sh start >/dev/null
    fi
fi

bash build.sh foot_pendulum
puffer eval foot_pendulum \
    --load-model-path "$POLICY_PATH" \
    --save-frames "$SAVE_FRAMES" \
    --frame-dir "$FRAME_DIR" \
    --gif-path "$GIF_PATH" \
    --fps "$FPS" \
    --trace-path "$TRACE_PATH"

printf 'GIF: %s\n' "$GIF_PATH"
printf 'Trace: %s\n' "$TRACE_PATH"
printf 'Frames: %s\n' "$FRAME_DIR"
