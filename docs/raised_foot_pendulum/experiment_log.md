# Raised Foot Pendulum Experiment Log

| run_id | git_sha | env | tag | total_timesteps | wall_clock_minutes | observed_steps_per_second | checkpoint_path | eval_result | notes |
| --- | --- | --- | --- | ---: | ---: | ---: | --- | --- | --- |
| BLOCKED_NO_GPU_WANDB_RUN | 9836f0d2e78889c1aaf189c04d161b6fc61a9386 | breakout | m0_breakout_smoke | 1000000 |  |  |  | Not run | Local Docker daemon and W&B auth are now working. This still requires a Linux NVIDIA Docker host; local Docker Desktop reports no `--gpus all` runtime. GitHub auth/remote setup is also not configured in this checkout. Replace this row with the W&B run URL and observed Puffer throughput metric after the GPU smoke run. |
| LOCAL_CPU_TRAIN_SMOKE | 9836f0d2e78889c1aaf189c04d161b6fc61a9386 | foot_pendulum | local_cpu_metric_smoke | 8192 | 0.03 | 65000 | local ignored logs/checkpoints | Passed non-W&B CPU trainer smoke | Ran with `--slowly`, `OMP_NUM_THREADS=1`, 16 agents, horizon 64. Custom metrics appeared in Puffer dashboard output: reward components, success metrics, simulator validity, reset mix, and PPO losses. |
