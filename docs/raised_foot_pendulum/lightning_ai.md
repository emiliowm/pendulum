# Lightning AI GPU Setup

This project should run on Lightning as a GPU-backed Studio or VM. It does not need to be rewritten to PyTorch Lightning; use Lightning as the remote NVIDIA Linux host and run the existing PufferLib training command.

## Recommended Flow

1. Create or open a Lightning Studio.
2. Start it on a GPU machine. The project target is an RTX 4090/5090/L40S-class GPU; an L4/T4 can work for smoke tests, but serious training should use a faster single GPU.
3. Clone this repo into the Studio.
4. Run the project bootstrap:

   ```bash
   cd pendulum
   bash scripts/lightning_bootstrap.sh
   ```

5. Start training:

   ```bash
   cd pendulum
   bash scripts/train_foot_pendulum.sh lightning_gpu
   ```

   On Lightning Studios, use the active `cloudspace` Python environment. On a non-Studio Linux host, activate `.venv` first if the bootstrap created one.

## What The Bootstrap Checks

- `nvidia-smi` can see the GPU.
- Python can import Torch and `torch.cuda.is_available()` is true.
- System build tools are installed when `apt-get` is available.
- The local PufferLib package is installed in editable mode. Lightning Studios use the active conda environment because extra virtual environments are blocked inside a Studio.
- `bash build.sh foot_pendulum` produces the CUDA extension.

## Secrets

Do not hardcode API keys in this repository. For W&B, add `WANDB_API_KEY` as a Lightning managed secret or run `wandb login` manually in the Studio terminal. The training script already sends runs to the `raised-foot-pendulum` W&B project.

## Cost Controls

Lightning Studios auto-sleep when idle, but GPU compute can still cost money while the Studio or VM is running. Stop the Studio when training is done, and prefer a small GPU for install/build validation before switching to a larger GPU for long runs.
