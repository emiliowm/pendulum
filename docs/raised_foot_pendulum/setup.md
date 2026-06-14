# Raised Foot Pendulum Setup

## Project baseline

- Repository shape: PufferLib root with custom Ocean environment under `ocean/foot_pendulum`.
- PufferLib branch: `4.0`.
- PufferLib commit SHA: `9836f0d2e78889c1aaf189c04d161b6fc61a9386`.
- Implementation branch: `m0-m1-foot-pendulum`.
- PufferTank branch: `4.0`.
- PufferTank commit SHA checked locally: `4e4ad27bd15a56743325f3e4968b6d9120e0661a`.
- W&B project name: `raised-foot-pendulum`.

## Local editing machine

- Host: `Darwin Emilios-MacBook-Air.local 25.1.0`.
- Architecture: `arm64`.
- Docker CLI: `Docker version 29.1.3, build f52814d`.
- Docker daemon: reachable through Docker Desktop; `docker run --rm hello-world` completed successfully on 2026-06-13.
- NVIDIA GPU: unavailable on this host; `nvidia-smi` is not installed.
- Python: `python3 3.12.1`; `python` is not installed on PATH.
- Local Python packages installed for CPU smoke tests: `numpy`, `pybind11`, `torch 2.12.0`, `rich`, `rich_argparse`, `gpytorch`, `scikit-learn`, `wandb`.
- Torch/CUDA on local host: Torch imports after install; CUDA is unavailable on this Apple Silicon host and Docker Desktop does not expose an NVIDIA GPU runtime.
- Local build dependency installed during setup: Homebrew `libomp 22.1.7`.
- Rendering availability: local macOS display is available for raylib standalone builds when raylib/OpenMP dependencies are present. Remote `ssh -X` rendering has not been configured.

## Required GPU environment

Milestone training is now staged on a Lightning AI Studio:

- Studio: `deploy-model-devbox` in teamspace `emiliomunguia04/deploy-model-project`.
- OS/image: Ubuntu Linux on AWS, x86_64.
- GPU model: NVIDIA Tesla T4, 15360 MiB VRAM.
- NVIDIA driver version: `580.159.03`.
- CUDA compiler: `/usr/local/cuda/bin/nvcc`, CUDA `13.0`.
- Docker: installed; `docker info` passes.
- Docker GPU runtime: `docker run --rm --gpus all nvidia/cuda:12.4.1-base-ubuntu22.04 nvidia-smi` passes.
- Python environment: `/teamspace/studios/this_studio/venvs/foot_pendulum`.
- Project checkout: `/teamspace/studios/this_studio/pendulum`.
- Rendering: remote `ssh -X` rendering has not been configured.

## External service status

- GitHub repository remote: `git@github.com:emiliowm/pendulum.git`.
- GitHub branch: `m0-m1-foot-pendulum` pushed.
- W&B project target: `raised-foot-pendulum`.
- W&B authentication is configured locally and on the Lightning Studio for user `emiliomunguia`.
- W&B project URL: `https://wandb.ai/emiliomunguia-none/raised-foot-pendulum`.

## PufferTank verification commands

Run on the GPU host:

```bash
git clone --branch 4.0 https://github.com/PufferAI/PufferTank puffertank
cd puffertank
./docker.sh test
```

If the prebuilt image is unavailable:

```bash
./docker.sh build -d puffertank.dockerfile
./docker.sh test
```

Inside the container:

```bash
cd /puffertank/pufferlib
python - <<'PY'
import torch
print('torch_cuda_available=', torch.cuda.is_available())
print('torch_cuda_device_count=', torch.cuda.device_count())
PY
bash build.sh breakout
```

Milestone 0 PufferTank shell validation passed on the Lightning T4 host. `./docker.sh test` entered the `pufferai/puffertank:4.0` container, `/puffertank/pufferlib` was the working directory, `nvidia-smi` reported `Tesla T4`, and `torch.cuda.is_available()` returned `True` with one CUDA device.
