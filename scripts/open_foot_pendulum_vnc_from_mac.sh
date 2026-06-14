#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-open}"
REMOTE="${REMOTE:-deploy-model-devbox}"
REMOTE_DIR="${REMOTE_DIR:-/teamspace/studios/this_studio/pendulum}"
LOCAL_PORT="${LOCAL_PORT:-5901}"
REMOTE_PORT="${REMOTE_PORT:-5901}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REMOTE_SCRIPT="${REMOTE_DIR}/scripts/view_foot_pendulum_vnc.sh"
CONTROL_SOCKET="${CONTROL_SOCKET:-${TMPDIR:-/tmp}/foot_pendulum_vnc_${LOCAL_PORT}.sock}"

sync_remote_script() {
    scp -q "${SCRIPT_DIR}/view_foot_pendulum_vnc.sh" "${REMOTE}:${REMOTE_SCRIPT}"
    ssh -o BatchMode=yes "$REMOTE" "chmod +x '${REMOTE_SCRIPT}'"
}

start_tunnel() {
    if ssh -S "$CONTROL_SOCKET" -O check "$REMOTE" >/dev/null 2>&1; then
        return
    fi
    ssh -fN -M -S "$CONTROL_SOCKET" -L "${LOCAL_PORT}:127.0.0.1:${REMOTE_PORT}" "$REMOTE"
}

stop_tunnel() {
    ssh -S "$CONTROL_SOCKET" -O exit "$REMOTE" >/dev/null 2>&1 || true
    rm -f "$CONTROL_SOCKET"
}

case "$MODE" in
    open)
        sync_remote_script
        ssh -o BatchMode=yes "$REMOTE" "bash -lc 'cd ${REMOTE_DIR} && VNC_PORT=${REMOTE_PORT} ${REMOTE_SCRIPT} launch'"
        start_tunnel
        open "vnc://localhost:${LOCAL_PORT}"
        printf 'Viewer URL: vnc://localhost:%s\n' "$LOCAL_PORT"
        printf 'Stop with: %s stop\n' "$0"
        ;;
    stop)
        sync_remote_script
        ssh -o BatchMode=yes "$REMOTE" "bash -lc 'cd ${REMOTE_DIR} && ${REMOTE_SCRIPT} stop'"
        stop_tunnel
        ;;
    status)
        sync_remote_script
        ssh -o BatchMode=yes "$REMOTE" "bash -lc 'cd ${REMOTE_DIR} && ${REMOTE_SCRIPT} status'"
        ;;
    *)
        printf 'Usage: %s [open|stop|status]\n' "$0" >&2
        exit 1
        ;;
esac
