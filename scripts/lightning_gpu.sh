#!/usr/bin/env bash
set -euo pipefail

TEAMSPACE="${LIGHTNING_TEAMSPACE:-emiliomunguia04/deploy-model-project}"
STUDIO="${LIGHTNING_STUDIO:-deploy-model-devbox}"
MACHINE="${LIGHTNING_MACHINE:-T4}"
LIGHTNING_BIN="${LIGHTNING_BIN:-lightning}"
LOCAL_TMP_LIGHTNING="/private/tmp/lightning-sdk-venv/bin/lightning"

if ! command -v "$LIGHTNING_BIN" >/dev/null 2>&1; then
  if [ -x "$LOCAL_TMP_LIGHTNING" ]; then
    LIGHTNING_BIN="$LOCAL_TMP_LIGHTNING"
  else
    echo "Error: lightning CLI not found. Set LIGHTNING_BIN or install lightning-sdk." >&2
    exit 1
  fi
fi

usage() {
  cat <<EOF
Usage: $(basename "$0") <status|start|stop|ssh>

Environment:
  LIGHTNING_TEAMSPACE  Default: $TEAMSPACE
  LIGHTNING_STUDIO     Default: $STUDIO
  LIGHTNING_MACHINE    Default: $MACHINE
  LIGHTNING_BIN        Default: lightning, with local temp fallback if present
EOF
}

case "${1:-}" in
  status)
    "$LIGHTNING_BIN" studio list --teamspace "$TEAMSPACE"
    ;;
  start)
    "$LIGHTNING_BIN" studio start \
      --name "$STUDIO" \
      --teamspace "$TEAMSPACE" \
      --machine "$MACHINE"
    ;;
  stop)
    "$LIGHTNING_BIN" studio stop \
      --name "$STUDIO" \
      --teamspace "$TEAMSPACE"
    "$LIGHTNING_BIN" studio list --teamspace "$TEAMSPACE"
    ;;
  ssh)
    TERM="${TERM:-xterm}" "$LIGHTNING_BIN" studio ssh \
      --name "$STUDIO" \
      --teamspace "$TEAMSPACE"
    ;;
  *)
    usage
    exit 2
    ;;
esac

