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

Milestone completion requires a Linux NVIDIA Docker host. The target profile remains:

- Provider/OS image: Linux NVIDIA Docker host, not yet provisioned in this workspace.
- GPU model: RTX 4090/5090/L40S class; no A100/H100 required for milestones 0-1.
- NVIDIA driver version: pending GPU host.
- Docker version: pending GPU host.
- Rendering: local display or `ssh -X` from the GPU host.

## External service status

- GitHub repository target: `humanoid-foot-pendulum`.
- GitHub creation attempt failed because neither the MCP GitHub tool nor `gh` is authenticated in this session; this checkout still points `origin` at upstream `https://github.com/PufferAI/PufferLib`.
- W&B project target: `raised-foot-pendulum`.
- W&B authentication is configured in `/Users/emiliowagnermunguia/.netrc` for user `emiliomunguia`; project/dashboard creation still needs an online training run with metrics.

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

Milestone 0 remains blocked until `torch_cuda_available=True` and `bash build.sh breakout` exits successfully on the GPU host.
