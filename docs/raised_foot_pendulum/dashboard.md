# Raised Foot Pendulum W&B Dashboard Contract

Project: `raised-foot-pendulum`.

All custom metric names are exported from `ocean/foot_pendulum/binding.c::my_log` with:

```c
dict_set(out, "metric_name", log->metric_name);
```

## Panels

### Infrastructure / throughput

- Built-in Puffer steps/sec metric from `m0_breakout_smoke` and later `foot_pendulum` runs. Use the exact Puffer-emitted throughput key observed in W&B.
- Built-in wall-clock time metric if emitted by Puffer.
- Built-in total timestep/global-step metric if emitted by Puffer.

### PPO health

Use Puffer-emitted keys when present:

- policy loss
- value loss
- entropy
- KL
- clip fraction

### Task reward components

Custom keys:

- `reward_tip_height`
- `reward_upright`
- `reward_stillness`
- `reward_catch_bonus`
- `reward_plate_smoothness`
- `reward_action_penalty`
- `reward_action_rate_penalty`
- `reward_bounds_penalty`

### Success

Custom keys:

- `success_duration`
- `max_success_duration`
- `hanging_success_duration`
- `hanging_success_episode`
- `perf`
- `score`

### Simulator validity

Custom keys:

- `invalid_state`
- `nan_termination`
- `bounds_termination`

### Reset mix

Custom keys:

- `reset_hanging`
- `reset_near_upright`
- `reset_random`

### Videos

Artifacts:

- evaluation GIF/video artifacts from `puffer eval foot_pendulum --save-frames` when enabled
- screenshots from the raylib standalone renderer
- notes linking visible upright hold to `success_duration` / `max_success_duration`

## Run links and panel notes

- Project: `https://wandb.ai/emiliomunguia-none/raised-foot-pendulum`.
- `studio_preflight`: `https://wandb.ai/emiliomunguia-none/raised-foot-pendulum/runs/pjaha87k`; verified W&B logging from the Lightning Studio.
- `studio_gpu_preflight`: `https://wandb.ai/emiliomunguia-none/raised-foot-pendulum/runs/qlpmd5n3`; verified CUDA backend build, PPO metrics, and custom `env/*` metrics at 8,192 steps on Tesla T4.
- `m0_breakout_1m_confirm`: `https://wandb.ai/emiliomunguia-none/raised-foot-pendulum/runs/crhsl1lu`; verified PufferTank CUDA build and Breakout smoke at 1,048,576 agent steps, ~1.39M SPS.
- `m1_train`: `https://wandb.ai/emiliomunguia-none/raised-foot-pendulum/runs/p9pnocwa`; completed 99,876,864 agent steps at ~1.23M SPS. It did not meet the 3s success target: final `max_success_duration=0.019`, `hanging_success_duration=0.00027`, `bounds_termination=0.99894`.
- W&B report: `https://wandb.ai/emiliomunguia-none/raised-foot-pendulum/reports/Raised-Foot-Pendulum-Milestone-0-Dashboard--VmlldzoxNzIyMDgzNA==?accessToken=zgzq2u8gopxhbm0d9lsx218czvqqtklmx54i9jzh60y88xm39ubtj20yt8yq2c5p`; share link enabled for unauthenticated viewing.
- Dashboard screenshot: `docs/raised_foot_pendulum/wandb_m0_dashboard_final.png`.
- Local CPU metric smoke: non-W&B `pufferlib.pufferl train foot_pendulum --slowly` run populated all custom metric groups in the terminal dashboard at ~65K SPS with 16 agents, horizon 64, total timesteps 8192.
