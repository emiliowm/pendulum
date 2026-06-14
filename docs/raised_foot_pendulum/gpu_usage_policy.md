# Lightning GPU Usage Policy

## Current Rule

The Lightning GPU Studio must be stopped unless a training run or GPU build is actively running.

Default state:

```bash
scripts/lightning_gpu.sh status
```

Expected safe result:

```text
deploy-model-devbox  ...  Stopped
```

## Start GPU Only For Active Work

Start the T4 only when immediately beginning setup, a smoke test, or a training run:

```bash
scripts/lightning_gpu.sh start
scripts/lightning_gpu.sh ssh
```

Inside the Studio:

```bash
cd /teamspace/studios/this_studio/pendulum
bash scripts/train_foot_pendulum.sh lightning_gpu
```

## Shutdown Checklist

Run this from the local Mac whenever training stops, before ending a work session, and before sleeping the machine:

```bash
scripts/lightning_gpu.sh stop
```

Then verify:

```bash
scripts/lightning_gpu.sh status
```

Both Studios in `emiliomunguia04/deploy-model-project` should show `Stopped`.

## Operating Policy

- Do not leave `deploy-model-devbox` running overnight.
- Do not start a GPU unless the next command needs GPU compute.
- If a remote shell is open but no training process is running, stop the Studio.
- If W&B is not authenticated, do not start the long training script with `--wandb`; authenticate first or run an explicit non-W&B smoke command.
- Treat `scripts/lightning_gpu.sh stop` as the final step of every GPU session.

## Known Safe Stop Command

The direct command used successfully on 2026-06-14 was:

```bash
/private/tmp/lightning-sdk-venv/bin/lightning studio stop \
  --name deploy-model-devbox \
  --teamspace emiliomunguia04/deploy-model-project
```

