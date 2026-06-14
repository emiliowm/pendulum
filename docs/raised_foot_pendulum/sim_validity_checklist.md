# Milestone 1 Simulator Validity Checklist

- [x] No NaNs or infs in observations/rewards during 50,000-step random-policy validity probe.
- [x] Pendulum has no action input and no hidden torque term; policy actions only set plate x/z/pitch acceleration.
- [x] Reward is low when the pendulum passes vertical at high angular velocity: `fast_vertical_reward=0.55`, `fast_vertical_catch=0.0`, versus `calm_upright_reward=3.181324`.
- [x] Reward is high only when upright and slow; catch bonus now gates on uprightness, stillness, plate calm, and pitch calm.
- [x] Plate bounds prevent teleporting/flying away; bounds and pre-bounds penalties are active.
- [x] A random hanging-start policy does not produce high `success_duration`: 50,000 random steps produced `max_success_duration=0.0`.
- [x] Rendering matches logged state: VNC screenshot and exported frame show non-upright/non-success state with `success 0.00s`.
- [x] Timeout, bounds, and invalid resets are separately logged; trace CSV adds per-step `terminal_reason`.
