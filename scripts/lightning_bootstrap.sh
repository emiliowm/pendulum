#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PYTHON_BIN="${PYTHON:-python3}"
VENV_DIR="${VENV_DIR:-$PROJECT_DIR/.venv}"
ENV_NAME="${PUFFER_ENV:-foot_pendulum}"

info() {
  printf '\n==> %s\n' "$*"
}

info "Checking GPU visibility"
if command -v nvidia-smi >/dev/null 2>&1; then
  nvidia-smi
else
  echo "nvidia-smi is not on PATH. Start the Lightning Studio on a GPU machine before training."
fi

info "Installing system build dependencies when apt is available"
if command -v apt-get >/dev/null 2>&1; then
  SUDO=""
  if [ "$(id -u)" -ne 0 ] && command -v sudo >/dev/null 2>&1; then
    SUDO="sudo"
  fi
  $SUDO apt-get update
  $SUDO apt-get install -y \
    build-essential \
    clang \
    ccache \
    curl \
    git \
    libgl1 \
    libgl1-mesa-dev \
    libomp-dev \
    python3-venv \
    tar \
    unzip
else
  echo "apt-get not found; assuming build dependencies are already present."
fi

info "Creating Python virtual environment at $VENV_DIR"
"$PYTHON_BIN" -m venv "$VENV_DIR"
source "$VENV_DIR/bin/activate"
python -m pip install --upgrade pip setuptools wheel

info "Installing project package"
python -m pip install -e "$PROJECT_DIR"

info "Validating CUDA from Python"
python - <<'PY'
import torch

print("torch_version=", torch.__version__)
print("cuda_available=", torch.cuda.is_available())
print("cuda_device_count=", torch.cuda.device_count())
if torch.cuda.is_available():
    print("cuda_device_name=", torch.cuda.get_device_name(0))
else:
    raise SystemExit("CUDA is not available to PyTorch. Switch the Studio to a GPU machine and rerun this script.")
PY

info "Building PufferLib environment: $ENV_NAME"
cd "$PROJECT_DIR"
bash build.sh "$ENV_NAME"

info "Bootstrap complete"
cat <<EOF

To start training:

  cd "$PROJECT_DIR"
  source "$VENV_DIR/bin/activate"
  bash scripts/train_foot_pendulum.sh lightning_gpu

If you use W&B, set WANDB_API_KEY as a Lightning secret or run 'wandb login' in the Studio terminal before training.
EOF

