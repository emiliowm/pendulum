# Milestone 1 Simulator Validity Checklist

- [ ] No NaNs or infs in observations/rewards.
- [ ] Pendulum has no action input and no hidden torque term.
- [ ] Reward is low when the pendulum passes vertical at high angular velocity.
- [ ] Reward is high only when upright and slow.
- [ ] Plate bounds prevent teleporting/flying away.
- [ ] A random policy does not produce high `success_duration`.
- [ ] Rendering matches logged state: upright in video means high `success_duration`.
- [ ] Timeout, bounds, and invalid resets are separately logged.
