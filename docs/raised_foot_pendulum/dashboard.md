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

- `m0_breakout_smoke`: pending GPU/W&B run; Docker daemon and W&B auth now work locally, but no NVIDIA GPU runtime is available.
- `m1_smoke`: pending GPU/W&B run on a Linux NVIDIA Docker host.
- Local CPU metric smoke: non-W&B `pufferlib.pufferl train foot_pendulum --slowly` run populated all custom metric groups in the terminal dashboard at ~65K SPS with 16 agents, horizon 64, total timesteps 8192.
- Dashboard screenshots: pending W&B dashboard creation after online GPU runs emit the metrics.
