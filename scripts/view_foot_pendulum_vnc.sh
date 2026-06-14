#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-run}"
DISPLAY_NUM="${DISPLAY_NUM:-99}"
DISPLAY=":${DISPLAY_NUM}"
VNC_PORT="${VNC_PORT:-5901}"
GEOMETRY="${GEOMETRY:-1280x900x24}"
PROJECT_DIR="${PROJECT_DIR:-/teamspace/studios/this_studio/pendulum}"
VENV_PATH="${VENV_PATH:-/teamspace/studios/this_studio/venvs/foot_pendulum}"
POLICY_PATH="${POLICY_PATH:-${PROJECT_DIR}/checkpoints/foot_pendulum/p9pnocwa/0000000099876864.bin}"
LOG_DIR="${LOG_DIR:-${PROJECT_DIR}/runs/view_foot_pendulum_vnc}"

export DISPLAY
export LIBGL_ALWAYS_SOFTWARE="${LIBGL_ALWAYS_SOFTWARE:-1}"

mkdir -p "$LOG_DIR"

pid_file() {
    printf '%s/%s.pid' "$LOG_DIR" "$1"
}

is_alive() {
    local file="$1"
    [ -f "$file" ] && kill -0 "$(cat "$file")" 2>/dev/null
}

require_cmds() {
    local missing=""
    for cmd in Xvfb x11vnc fluxbox xdpyinfo; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            missing="${missing} ${cmd}"
        fi
    done
    if [ -n "$missing" ]; then
        printf 'Missing display tools:%s\n' "$missing" >&2
        printf 'Install on Ubuntu with: sudo apt-get install -y xvfb x11vnc fluxbox x11-apps mesa-utils libgl1-mesa-dri libglx-mesa0\n' >&2
        exit 1
    fi
}

start_xvfb() {
    local pidfile
    pidfile="$(pid_file xvfb)"
    if is_alive "$pidfile"; then
        return
    fi
    rm -f "/tmp/.X${DISPLAY_NUM}-lock"
    Xvfb "$DISPLAY" -screen 0 "$GEOMETRY" -ac +extension GLX +render -noreset >"$LOG_DIR/xvfb.log" 2>&1 &
    echo $! > "$pidfile"
    for _ in 1 2 3 4 5; do
        if xdpyinfo -display "$DISPLAY" >/dev/null 2>&1; then
            return
        fi
        sleep 1
    done
    printf 'Xvfb did not become ready. See %s/xvfb.log\n' "$LOG_DIR" >&2
    exit 1
}

start_fluxbox() {
    local pidfile
    pidfile="$(pid_file fluxbox)"
    if is_alive "$pidfile"; then
        return
    fi
    fluxbox >"$LOG_DIR/fluxbox.log" 2>&1 &
    echo $! > "$pidfile"
}

start_x11vnc() {
    local pidfile
    pidfile="$(pid_file x11vnc)"
    if is_alive "$pidfile"; then
        return
    fi
    x11vnc -display "$DISPLAY" -localhost -nopw -forever -shared -noxdamage -rfbport "$VNC_PORT" >"$LOG_DIR/x11vnc.log" 2>&1 &
    echo $! > "$pidfile"
}

start_display() {
    require_cmds
    start_xvfb
    start_fluxbox
    start_x11vnc
}

launch_eval() {
    local pidfile
    pidfile="$(pid_file eval)"
    if is_alive "$pidfile"; then
        printf 'Policy eval already running with pid %s\n' "$(cat "$pidfile")"
        return
    fi
    if [ ! -f "$POLICY_PATH" ]; then
        printf 'Policy checkpoint not found: %s\n' "$POLICY_PATH" >&2
        exit 1
    fi
    if [ ! -f "$VENV_PATH/bin/activate" ]; then
        printf 'Virtualenv not found: %s\n' "$VENV_PATH" >&2
        exit 1
    fi
    (
        cd "$PROJECT_DIR"
        # shellcheck disable=SC1091
        source "$VENV_PATH/bin/activate"
        exec puffer eval foot_pendulum --load-model-path "$POLICY_PATH" --render-mode raylib
    ) >"$LOG_DIR/eval.log" 2>&1 &
    echo $! > "$pidfile"
}

stop_one() {
    local name="$1"
    local pidfile
    pidfile="$(pid_file "$name")"
    if is_alive "$pidfile"; then
        kill "$(cat "$pidfile")" 2>/dev/null || true
    fi
    rm -f "$pidfile"
}

stop_all() {
    stop_one eval
    stop_one x11vnc
    stop_one fluxbox
    stop_one xvfb
}

status_one() {
    local name="$1"
    local pidfile
    pidfile="$(pid_file "$name")"
    if is_alive "$pidfile"; then
        printf '%s: running pid %s\n' "$name" "$(cat "$pidfile")"
    else
        printf '%s: stopped\n' "$name"
    fi
}

print_connect_info() {
    printf 'Remote display: %s\n' "$DISPLAY"
    printf 'Remote VNC: 127.0.0.1:%s\n' "$VNC_PORT"
    printf 'Policy: %s\n' "$POLICY_PATH"
    printf 'Logs: %s\n' "$LOG_DIR"
    printf 'From your Mac: ssh -N -L %s:127.0.0.1:%s deploy-model-devbox\n' "$VNC_PORT" "$VNC_PORT"
    printf 'Then open: vnc://localhost:%s\n' "$VNC_PORT"
}

case "$MODE" in
    run)
        start_display
        print_connect_info
        if [ ! -f "$POLICY_PATH" ]; then
            printf 'Policy checkpoint not found: %s\n' "$POLICY_PATH" >&2
            exit 1
        fi
        cd "$PROJECT_DIR"
        # shellcheck disable=SC1091
        source "$VENV_PATH/bin/activate"
        exec puffer eval foot_pendulum --load-model-path "$POLICY_PATH" --render-mode raylib
        ;;
    launch)
        start_display
        launch_eval
        print_connect_info
        ;;
    start)
        start_display
        print_connect_info
        ;;
    stop)
        stop_all
        ;;
    status)
        status_one xvfb
        status_one fluxbox
        status_one x11vnc
        status_one eval
        ;;
    *)
        printf 'Usage: %s [run|launch|start|stop|status]\n' "$0" >&2
        exit 1
        ;;
esac
