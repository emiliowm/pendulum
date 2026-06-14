// Passive foot-plate pendulum swing-up task with continuous plate accelerations.

#pragma once

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#define FP_OBS_SIZE 16
#define FP_NUM_ACTIONS 3
#define FP_PI 3.14159265358979323846f
#define FP_WIDTH 960
#define FP_HEIGHT 640
#define FP_SCALE 180.0f
#define FP_RESET_HANGING 0
#define FP_RESET_NEAR_UPRIGHT 1
#define FP_RESET_RANDOM 2

#define FP_SUCCESS_HOLD_REWARD_SCALE 0.02f
typedef struct Log {
    float perf;
    float score;
    float episode_return;
    float episode_length;
    float reward_tip_height;
    float reward_upright;
    float reward_stillness;
    float reward_catch_bonus;
    float reward_plate_smoothness;
    float reward_action_penalty;
    float reward_action_rate_penalty;
    float reward_bounds_penalty;
    float success_duration;
    float max_success_duration;
    float hanging_success_duration;
    float hanging_success_episode;
    float kick_peak_plate_speed;
    float invalid_state;
    float nan_termination;
    float bounds_termination;
    float reset_hanging;
    float reset_near_upright;
    float reset_random;
    float n;
} Log;

typedef struct FootPendulum {
    Log log;
    float* observations;
    float* actions;
    float* rewards;
    float* terminals;
    int num_agents;
    int env_id;
    int episode_idx;
    unsigned int rng;

    float plate_x;
    float plate_z;
    float plate_pitch;
    float plate_vx;
    float plate_vz;
    float plate_pitch_dot;
    float pendulum_theta;
    float pendulum_theta_dot;
    float previous_action[FP_NUM_ACTIONS];
    int tick;
    int horizon_steps;
    int reset_mode;
    int success_steps_current;
    int success_steps_max;
    float episode_return;
    float reward_tip_height;
    float reward_upright;
    float reward_stillness;
    float reward_catch_bonus;
    float reward_plate_smoothness;
    float reward_action_penalty;
    float reward_action_rate_penalty;
    float reward_bounds_penalty;
    float reward_current;
    float sum_reward_tip_height;
    float sum_reward_upright;
    float sum_reward_stillness;
    float sum_reward_catch_bonus;
    float sum_reward_plate_smoothness;
    float sum_reward_action_penalty;
    float sum_reward_action_rate_penalty;
    float sum_reward_bounds_penalty;
    float kick_peak_plate_speed;

    float dt;
    float pendulum_length;
    float gravity;
    float hinge_damping;
    float x_limit;
    float z_min;
    float z_max;
    float pitch_limit;
    float max_plate_velocity;
    float max_plate_accel;
    float max_pitch_rate;
    float max_pitch_accel;
    float max_theta_dot;
    float success_angle_rad;
    float success_angular_velocity;
    int success_steps;
    int min_horizon_steps;
    int max_horizon_steps;
    float height_reward_weight;
    float upright_reward_weight;
    float catch_bonus_weight;
    float stillness_sigma;
    float action_penalty_weight;
    float action_rate_penalty_weight;
    float plate_smoothness_weight;
    float reset_hanging_pct;
    float reset_near_upright_pct;
    float reset_random_pct;
} FootPendulum;

const Color FP_RED = (Color){187, 0, 0, 255};
const Color FP_CYAN = (Color){0, 187, 187, 255};
const Color FP_WHITE = (Color){241, 241, 241, 255};
const Color FP_BACKGROUND = (Color){6, 24, 24, 255};
const Color FP_YELLOW = (Color){245, 197, 66, 255};
const Color FP_GREEN = (Color){85, 220, 130, 255};

static inline float fp_clampf(float x, float lo, float hi) {
    return fminf(fmaxf(x, lo), hi);
}

static inline float fp_randf(FootPendulum* env, float lo, float hi) {
    float t = (float)rand_r(&env->rng) / (float)RAND_MAX;
    return lo + t * (hi - lo);
}

static inline float fp_wrap_pi(float x) {
    while (x > FP_PI) x -= 2.0f * FP_PI;
    while (x < -FP_PI) x += 2.0f * FP_PI;
    return x;
}

static inline float fp_world_angle(FootPendulum* env) {
    return env->plate_pitch + env->pendulum_theta;
}

static inline float fp_world_angular_velocity(FootPendulum* env) {
    return env->plate_pitch_dot + env->pendulum_theta_dot;
}

static inline bool fp_state_isfinite(FootPendulum* env) {
    return isfinite(env->plate_x)
        && isfinite(env->plate_z)
        && isfinite(env->plate_pitch)
        && isfinite(env->plate_vx)
        && isfinite(env->plate_vz)
        && isfinite(env->plate_pitch_dot)
        && isfinite(env->pendulum_theta)
        && isfinite(env->pendulum_theta_dot);
}

static inline bool fp_bounds_violated(FootPendulum* env) {
    return fabsf(env->plate_x) > env->x_limit
        || env->plate_z < env->z_min
        || env->plate_z > env->z_max
        || fabsf(env->plate_pitch) > env->pitch_limit;
}

static inline const char* fp_reset_mode_name(int reset_mode) {
    if (reset_mode == FP_RESET_HANGING) return "hanging";
    if (reset_mode == FP_RESET_NEAR_UPRIGHT) return "near-upright";
    return "random";
}
static inline float fp_plate_speed(FootPendulum* env) {
    return sqrtf(env->plate_vx * env->plate_vx + env->plate_vz * env->plate_vz);
}

static inline float fp_tip_height_norm(FootPendulum* env, float phi) {
    (void)env;
    return fp_clampf(0.5f * (cosf(phi) + 1.0f), 0.0f, 1.0f);
}

static inline float fp_smooth_gate(float value, float limit) {
    float margin = fmaxf(limit * 0.20f, 0.0001f);
    float excess = fmaxf(fabsf(value) - (limit - margin), 0.0f);
    float scaled = excess / margin;
    return scaled * scaled;
}

static inline float fp_pre_bounds_penalty(FootPendulum* env) {
    float penalty = 0.0f;
    penalty += fp_smooth_gate(env->plate_x, env->x_limit);
    penalty += fp_smooth_gate(env->plate_pitch, env->pitch_limit);

    float z_margin = fmaxf((env->z_max - env->z_min) * 0.20f, 0.0001f);
    float z_low = fmaxf(env->z_min + z_margin - env->plate_z, 0.0f) / z_margin;
    float z_high = fmaxf(env->plate_z - (env->z_max - z_margin), 0.0f) / z_margin;
    penalty += z_low * z_low + z_high * z_high;
    return penalty;
}
static inline bool fp_apply_bounds(FootPendulum* env) {
    bool touched = false;
    if (env->plate_x < -env->x_limit) {
        env->plate_x = -env->x_limit;
        if (env->plate_vx < 0.0f) env->plate_vx = 0.0f;
        touched = true;
    } else if (env->plate_x > env->x_limit) {
        env->plate_x = env->x_limit;
        if (env->plate_vx > 0.0f) env->plate_vx = 0.0f;
        touched = true;
    }

    if (env->plate_z < env->z_min) {
        env->plate_z = env->z_min;
        if (env->plate_vz < 0.0f) env->plate_vz = 0.0f;
        touched = true;
    } else if (env->plate_z > env->z_max) {
        env->plate_z = env->z_max;
        if (env->plate_vz > 0.0f) env->plate_vz = 0.0f;
        touched = true;
    }

    if (env->plate_pitch < -env->pitch_limit) {
        env->plate_pitch = -env->pitch_limit;
        if (env->plate_pitch_dot < 0.0f) env->plate_pitch_dot = 0.0f;
        touched = true;
    } else if (env->plate_pitch > env->pitch_limit) {
        env->plate_pitch = env->pitch_limit;
        if (env->plate_pitch_dot > 0.0f) env->plate_pitch_dot = 0.0f;
        touched = true;
    }

    return touched;
}


static inline const char* fp_terminal_reason(bool timeout, bool nan_termination, bool bounds_termination) {
    if (nan_termination) return "nan";
    if (bounds_termination) return "bounds";
    if (timeout) return "timeout";
    return "none";
}

static inline FILE* fp_trace_file(void) {
    static FILE* file = NULL;
    static bool checked = false;
    if (checked) return file;
    checked = true;

    const char* path = getenv("FOOT_PENDULUM_TRACE_PATH");
    if (path == NULL || path[0] == '\0') return NULL;

    file = fopen(path, "w");
    if (file == NULL) return NULL;
    fprintf(file,
        "episode,step,reset_mode,terminal_reason,action_x,action_z,action_pitch,"
        "plate_x,plate_z,plate_pitch,plate_vx,plate_vz,plate_pitch_dot,"
        "pendulum_theta,pendulum_theta_dot,phi,phi_dot,tip_height_norm,"
        "reward,reward_tip_height,reward_upright,reward_stillness,reward_catch_bonus,"
        "reward_plate_smoothness,reward_action_penalty,reward_action_rate_penalty,"
        "reward_bounds_penalty,success_duration,max_success_duration,kick_peak_plate_speed\n");
    return file;
}

static inline void fp_trace_step(FootPendulum* env, float action[FP_NUM_ACTIONS],
        bool timeout, bool nan_termination, bool bounds_termination) {
    if (env->env_id != 0) return;
    FILE* file = fp_trace_file();
    if (file == NULL) return;

    float phi = fp_world_angle(env);
    float phi_dot = fp_world_angular_velocity(env);
    fprintf(file,
        "%d,%d,%s,%s,%.9g,%.9g,%.9g,"
        "%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,"
        "%.9g,%.9g,%.9g,%.9g,%.9g,"
        "%.9g,%.9g,%.9g,%.9g,%.9g,"
        "%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g\n",
        env->episode_idx,
        env->tick,
        fp_reset_mode_name(env->reset_mode),
        fp_terminal_reason(timeout, nan_termination, bounds_termination),
        action[0], action[1], action[2],
        env->plate_x, env->plate_z, env->plate_pitch,
        env->plate_vx, env->plate_vz, env->plate_pitch_dot,
        env->pendulum_theta, env->pendulum_theta_dot,
        phi, phi_dot, fp_tip_height_norm(env, phi),
        env->reward_current,
        env->reward_tip_height,
        env->reward_upright,
        env->reward_stillness,
        env->reward_catch_bonus,
        env->reward_plate_smoothness,
        env->reward_action_penalty,
        env->reward_action_rate_penalty,
        env->reward_bounds_penalty,
        env->success_steps_current * env->dt,
        env->success_steps_max * env->dt,
        env->kick_peak_plate_speed);
    if (timeout || nan_termination || bounds_termination) fflush(file);
}

void compute_observations(FootPendulum* env) {
    float phi = fp_world_angle(env);
    float phi_dot = fp_world_angular_velocity(env);
    float z_span = env->z_max - env->z_min;
    float z_norm = z_span != 0.0f ? (env->plate_z - env->z_min) / z_span : 0.0f;
    float tip_height_norm = fp_tip_height_norm(env, phi);
    float time_remaining = 0.0f;
    if (env->horizon_steps > 0) {
        time_remaining = (float)(env->horizon_steps - env->tick) / (float)env->horizon_steps;
    }

    env->observations[0] = env->plate_x / env->x_limit;
    env->observations[1] = fp_clampf(z_norm, 0.0f, 1.0f);
    env->observations[2] = sinf(env->plate_pitch);
    env->observations[3] = cosf(env->plate_pitch);
    env->observations[4] = env->plate_vx / env->max_plate_velocity;
    env->observations[5] = env->plate_vz / env->max_plate_velocity;
    env->observations[6] = env->plate_pitch_dot / env->max_pitch_rate;
    env->observations[7] = sinf(phi);
    env->observations[8] = cosf(phi);
    env->observations[9] = phi_dot / env->max_theta_dot;
    env->observations[10] = tip_height_norm;
    env->observations[11] = env->pendulum_theta_dot / env->max_theta_dot;
    env->observations[12] = env->previous_action[0];
    env->observations[13] = env->previous_action[1];
    env->observations[14] = env->previous_action[2];
    env->observations[15] = fp_clampf(time_remaining, 0.0f, 1.0f);
}

static inline void fp_reset_component_sums(FootPendulum* env) {
    env->sum_reward_tip_height = 0.0f;
    env->sum_reward_upright = 0.0f;
    env->sum_reward_stillness = 0.0f;
    env->sum_reward_catch_bonus = 0.0f;
    env->sum_reward_plate_smoothness = 0.0f;
    env->sum_reward_action_penalty = 0.0f;
    env->sum_reward_action_rate_penalty = 0.0f;
    env->sum_reward_bounds_penalty = 0.0f;
    env->kick_peak_plate_speed = 0.0f;
}

static inline void fp_accumulate_reward_components(FootPendulum* env) {
    env->sum_reward_tip_height += env->reward_tip_height;
    env->sum_reward_upright += env->reward_upright;
    env->sum_reward_stillness += env->reward_stillness;
    env->sum_reward_catch_bonus += env->reward_catch_bonus;
    env->sum_reward_plate_smoothness += env->reward_plate_smoothness;
    env->sum_reward_action_penalty += env->reward_action_penalty;
    env->sum_reward_action_rate_penalty += env->reward_action_rate_penalty;
    env->sum_reward_bounds_penalty += env->reward_bounds_penalty;
}

static inline float fp_average(float sum, int count) {
    return count > 0 ? sum / (float)count : 0.0f;
}

static inline float fp_reward_tip_height(FootPendulum* env, float phi) {
    return fp_tip_height_norm(env, phi);
}

static inline float fp_reward_stillness(FootPendulum* env, float phi_dot) {
    float sigma = fmaxf(env->stillness_sigma, 0.0001f);
    return expf(-(phi_dot * phi_dot) / (sigma * sigma));
}

static inline float fp_reward_catch_bonus(FootPendulum* env, float upright_clipped, float stillness) {
    float catch_gate = 1.0f / (1.0f + expf(-14.0f * (upright_clipped - 0.93f)));
    float speed = fp_plate_speed(env);
    float plate_calm = expf(-(speed * speed) / 0.25f);
    float pitch_calm = expf(-(env->plate_pitch_dot * env->plate_pitch_dot) / 0.25f);
    float upright_sq = upright_clipped * upright_clipped;
    float hold_bonus = 1.0f + FP_SUCCESS_HOLD_REWARD_SCALE * (float)env->success_steps_current;
    return catch_gate * upright_sq * upright_sq * stillness * stillness * plate_calm * pitch_calm * hold_bonus;
}

static inline float fp_reward_action_penalty(FootPendulum* env, float action[FP_NUM_ACTIONS]) {
    float sum = action[0] * action[0] + action[1] * action[1] + action[2] * action[2];
    return env->action_penalty_weight * sum;
}

static inline float fp_reward_action_rate_penalty(FootPendulum* env, float action[FP_NUM_ACTIONS]) {
    float dx = action[0] - env->previous_action[0];
    float dz = action[1] - env->previous_action[1];
    float dp = action[2] - env->previous_action[2];
    return env->action_rate_penalty_weight * (dx * dx + dz * dz + dp * dp);
}

static inline float fp_reward_plate_smoothness(FootPendulum* env) {
    float speed_cost = env->plate_vx * env->plate_vx
        + env->plate_vz * env->plate_vz
        + 0.25f * env->plate_pitch_dot * env->plate_pitch_dot;
    return env->plate_smoothness_weight * speed_cost;
}

void compute_reward(FootPendulum* env, float action[FP_NUM_ACTIONS], bool bounds_violation) {
    if (!fp_state_isfinite(env)) {
        env->reward_tip_height = 0.0f;
        env->reward_upright = 0.0f;
        env->reward_stillness = 0.0f;
        env->reward_catch_bonus = 0.0f;
        env->reward_action_penalty = fp_reward_action_penalty(env, action);
        env->reward_action_rate_penalty = fp_reward_action_rate_penalty(env, action);
        env->reward_plate_smoothness = 0.0f;
        env->reward_bounds_penalty = 1.0f;
        env->reward_current = -env->reward_action_penalty
            - env->reward_action_rate_penalty
            - env->reward_bounds_penalty;
        return;
    }

    float phi = fp_world_angle(env);
    float phi_dot = fp_world_angular_velocity(env);
    float upright = cosf(phi);
    float upright_clipped = fp_clampf((upright + 1.0f) * 0.5f, 0.0f, 1.0f);
    env->reward_tip_height = fp_reward_tip_height(env, phi);
    env->reward_stillness = fp_reward_stillness(env, phi_dot);
    env->reward_upright = upright_clipped * (0.10f + 0.90f * env->reward_stillness);
    env->reward_catch_bonus = fp_reward_catch_bonus(env, upright_clipped, env->reward_stillness);
    env->reward_action_penalty = fp_reward_action_penalty(env, action);
    env->reward_action_rate_penalty = fp_reward_action_rate_penalty(env, action);
    env->reward_plate_smoothness = fp_reward_plate_smoothness(env);
    env->reward_bounds_penalty = bounds_violation ? 5.0f : fp_pre_bounds_penalty(env);
    env->reward_current = env->height_reward_weight * env->reward_tip_height
        + env->upright_reward_weight * env->reward_upright
        + env->catch_bonus_weight * env->reward_catch_bonus
        - env->reward_action_penalty
        - env->reward_action_rate_penalty
        - env->reward_plate_smoothness
        - env->reward_bounds_penalty;
}

void update_success(FootPendulum* env) {
    if (!fp_state_isfinite(env)) {
        env->success_steps_current = 0;
        return;
    }

    float phi = fp_wrap_pi(fp_world_angle(env));
    float phi_dot = fp_world_angular_velocity(env);
    bool success_now = fabsf(phi) <= env->success_angle_rad
        && fabsf(phi_dot) <= env->success_angular_velocity
        && fabsf(env->plate_vx) <= 0.5f
        && fabsf(env->plate_vz) <= 0.5f;

    if (success_now) env->success_steps_current += 1;
    else env->success_steps_current = 0;
    if (env->success_steps_current > env->success_steps_max) {
        env->success_steps_max = env->success_steps_current;
    }
}

void add_log(FootPendulum* env, bool timeout, bool nan_termination, bool bounds_termination) {
    (void)timeout;
    int steps = env->tick > 0 ? env->tick : 1;
    float max_success_duration = (float)env->success_steps_max * env->dt;
    float success_duration = (float)env->success_steps_current * env->dt;
    float perf_denom = env->success_steps > 0 ? (float)env->success_steps : 1.0f;

    env->log.perf += fp_clampf((float)env->success_steps_max / perf_denom, 0.0f, 1.0f);
    env->log.score += env->episode_return;
    env->log.episode_return += env->episode_return;
    env->log.episode_length += (float)env->tick;
    env->log.reward_tip_height += fp_average(env->sum_reward_tip_height, steps);
    env->log.reward_upright += fp_average(env->sum_reward_upright, steps);
    env->log.reward_stillness += fp_average(env->sum_reward_stillness, steps);
    env->log.reward_catch_bonus += fp_average(env->sum_reward_catch_bonus, steps);
    env->log.reward_plate_smoothness += fp_average(env->sum_reward_plate_smoothness, steps);
    env->log.reward_action_penalty += fp_average(env->sum_reward_action_penalty, steps);
    env->log.reward_action_rate_penalty += fp_average(env->sum_reward_action_rate_penalty, steps);
    env->log.reward_bounds_penalty += fp_average(env->sum_reward_bounds_penalty, steps);
    env->log.success_duration += success_duration;
    env->log.max_success_duration += max_success_duration;
    env->log.kick_peak_plate_speed += env->kick_peak_plate_speed;
    env->log.nan_termination += nan_termination ? 1.0f : 0.0f;
    env->log.bounds_termination += bounds_termination ? 1.0f : 0.0f;
    env->log.reset_hanging += env->reset_mode == FP_RESET_HANGING ? 1.0f : 0.0f;
    env->log.reset_near_upright += env->reset_mode == FP_RESET_NEAR_UPRIGHT ? 1.0f : 0.0f;
    env->log.reset_random += env->reset_mode == FP_RESET_RANDOM ? 1.0f : 0.0f;
    if (env->reset_mode == FP_RESET_HANGING) {
        env->log.hanging_success_duration += max_success_duration;
        env->log.hanging_success_episode += env->success_steps_max >= env->success_steps ? 1.0f : 0.0f;
    }
    env->log.n += 1.0f;
}

void init(FootPendulum* env) {
    env->num_agents = 1;
}

void c_reset(FootPendulum* env) {
    env->plate_x = 0.0f;
    env->plate_z = 1.0f;
    env->plate_pitch = 0.0f;
    env->plate_vx = fp_randf(env, -0.02f, 0.02f);
    env->plate_vz = fp_randf(env, -0.02f, 0.02f);
    env->plate_pitch_dot = fp_randf(env, -0.02f, 0.02f);
    env->episode_idx += 1;

    int span = env->max_horizon_steps - env->min_horizon_steps + 1;
    if (span < 1) span = 1;
    env->horizon_steps = env->min_horizon_steps + (int)(fp_randf(env, 0.0f, (float)span));
    if (env->horizon_steps > env->max_horizon_steps) env->horizon_steps = env->max_horizon_steps;

    float roll = fp_randf(env, 0.0f, 100.0f);
    if (roll < env->reset_hanging_pct) {
        env->reset_mode = FP_RESET_HANGING;
        env->pendulum_theta = fp_wrap_pi(FP_PI + fp_randf(env, -0.15f, 0.15f));
        env->pendulum_theta_dot = fp_randf(env, -0.20f, 0.20f);
    } else if (roll < env->reset_hanging_pct + env->reset_near_upright_pct) {
        env->reset_mode = FP_RESET_NEAR_UPRIGHT;
        env->pendulum_theta = fp_randf(env, -0.12f, 0.12f);
        env->pendulum_theta_dot = fp_randf(env, -0.20f, 0.20f);
    } else {
        env->reset_mode = FP_RESET_RANDOM;
        env->pendulum_theta = fp_randf(env, -FP_PI, FP_PI);
        env->pendulum_theta_dot = fp_randf(env, -2.00f, 2.00f);
    }

    env->previous_action[0] = 0.0f;
    env->previous_action[1] = 0.0f;
    env->previous_action[2] = 0.0f;
    env->tick = 0;
    env->episode_return = 0.0f;
    env->success_steps_current = 0;
    env->success_steps_max = 0;
    env->reward_current = 0.0f;
    fp_reset_component_sums(env);
    compute_observations(env);
}

void c_step(FootPendulum* env) {
    float action[FP_NUM_ACTIONS];
    bool invalid_action = false;
    for (int i = 0; i < FP_NUM_ACTIONS; i++) {
        float a = env->actions[i];
        if (!isfinite(a)) {
            a = 0.0f;
            invalid_action = true;
        }
        action[i] = fp_clampf(a, -1.0f, 1.0f);
        env->actions[i] = action[i];
    }
    if (invalid_action) env->log.invalid_state += 1.0f;

    float plate_ax = action[0] * env->max_plate_accel;
    float plate_az = action[1] * env->max_plate_accel;
    float plate_pitch_accel = action[2] * env->max_pitch_accel;

    env->plate_vx = fp_clampf(env->plate_vx + env->dt * plate_ax,
        -env->max_plate_velocity, env->max_plate_velocity);
    env->plate_vz = fp_clampf(env->plate_vz + env->dt * plate_az,
        -env->max_plate_velocity, env->max_plate_velocity);
    env->plate_pitch_dot = fp_clampf(env->plate_pitch_dot + env->dt * plate_pitch_accel,
        -env->max_pitch_rate, env->max_pitch_rate);
    env->plate_x += env->dt * env->plate_vx;
    env->plate_z += env->dt * env->plate_vz;
    env->plate_pitch += env->dt * env->plate_pitch_dot;

    float phi = fp_world_angle(env);
    float phi_dot = fp_world_angular_velocity(env);
    float length = fmaxf(env->pendulum_length, 0.0001f);
    float phi_ddot = ((env->gravity + plate_az) * sinf(phi) - plate_ax * cosf(phi)) / length;
    float pendulum_theta_ddot = phi_ddot - plate_pitch_accel - env->hinge_damping * env->pendulum_theta_dot;
    (void)phi_dot;
    env->pendulum_theta_dot = fp_clampf(env->pendulum_theta_dot + env->dt * pendulum_theta_ddot,
        -env->max_theta_dot, env->max_theta_dot);
    env->pendulum_theta = fp_wrap_pi(env->pendulum_theta + env->dt * env->pendulum_theta_dot);

    bool bounds_contact = fp_apply_bounds(env);
    env->tick += 1;
    bool nan_termination = !fp_state_isfinite(env);
    bool bounds_termination = false;
    bool timeout = env->tick >= env->horizon_steps;
    bool done = timeout || nan_termination;

    float plate_speed = fp_plate_speed(env);
    if (plate_speed > env->kick_peak_plate_speed) env->kick_peak_plate_speed = plate_speed;

    compute_reward(env, action, bounds_contact || nan_termination);
    update_success(env);
    fp_accumulate_reward_components(env);
    env->rewards[0] = env->reward_current;
    env->episode_return += env->reward_current;
    env->terminals[0] = done ? 1.0f : 0.0f;
    fp_trace_step(env, action, timeout, nan_termination, bounds_termination);

    env->previous_action[0] = action[0];
    env->previous_action[1] = action[1];
    env->previous_action[2] = action[2];

    if (done) {
        if (nan_termination) env->log.invalid_state += 1.0f;
        add_log(env, timeout, nan_termination, bounds_termination);
        c_reset(env);
        return;
    }

    compute_observations(env);
}

static inline Vector2 fp_world_to_screen(float x, float z) {
    return (Vector2){FP_WIDTH * 0.5f + x * FP_SCALE, FP_HEIGHT * 0.82f - z * FP_SCALE};
}

void c_render(FootPendulum* env) {
    if (!IsWindowReady()) {
        InitWindow(FP_WIDTH, FP_HEIGHT, "PufferLib Foot Pendulum");
        SetTargetFPS(60);
    }
    if (IsKeyDown(KEY_ESCAPE)) exit(0);
    if (IsKeyPressed(KEY_TAB)) ToggleFullscreen();
    if (!fp_state_isfinite(env)) return;

    float phi = fp_world_angle(env);
    Vector2 hinge = fp_world_to_screen(env->plate_x, env->plate_z);
    Vector2 tip = fp_world_to_screen(
        env->plate_x + sinf(phi) * env->pendulum_length,
        env->plate_z + cosf(phi) * env->pendulum_length);
    Rectangle plate = {hinge.x, hinge.y, 140.0f, 18.0f};
    Vector2 plate_origin = {70.0f, 9.0f};
    float plate_rotation_deg = -env->plate_pitch * 180.0f / FP_PI;

    BeginDrawing();
    ClearBackground(FP_BACKGROUND);
    DrawLine(0, (int)(FP_HEIGHT * 0.82f), FP_WIDTH, (int)(FP_HEIGHT * 0.82f), FP_CYAN);
    DrawRectanglePro(plate, plate_origin, plate_rotation_deg, FP_CYAN);
    DrawLineEx(hinge, tip, 7.0f, FP_RED);
    DrawCircleV(hinge, 8.0f, FP_WHITE);
    DrawCircleV(tip, 12.0f, FP_YELLOW);
    DrawCircleLines((int)hinge.x, (int)hinge.y, env->pendulum_length * FP_SCALE, FP_GREEN);

    DrawText(TextFormat("mode %s  step %d/%d  return %.2f  reward %.3f",
        fp_reset_mode_name(env->reset_mode), env->tick, env->horizon_steps,
        env->episode_return, env->reward_current), 20, 20, 20, FP_WHITE);
    DrawText(TextFormat("success %.2fs  max %.2fs  perf %.2f",
        env->success_steps_current * env->dt, env->success_steps_max * env->dt,
        fp_clampf((float)env->success_steps_max / fmaxf((float)env->success_steps, 1.0f), 0.0f, 1.0f)),
        20, 48, 20, FP_WHITE);
    DrawText(TextFormat("plate x %.2f z %.2f pitch %.1fdeg  speed %.2f",
        env->plate_x, env->plate_z, env->plate_pitch * 180.0f / FP_PI,
        sqrtf(env->plate_vx * env->plate_vx + env->plate_vz * env->plate_vz)),
        20, 76, 20, FP_WHITE);
    DrawText(TextFormat("theta %.1fdeg phi %.1fdeg phi_dot %.2f",
        env->pendulum_theta * 180.0f / FP_PI, phi * 180.0f / FP_PI,
        fp_world_angular_velocity(env)), 20, 104, 20, FP_WHITE);
    DrawText(TextFormat("actions ax %.2f az %.2f pitch %.2f",
        env->previous_action[0], env->previous_action[1], env->previous_action[2]),
        20, 132, 20, FP_WHITE);
    DrawText("A/D or arrows: x accel   W/S: z accel   Q/E: pitch accel   R: random", 20, FP_HEIGHT - 34, 18, FP_WHITE);
    EndDrawing();
    const char* frame_dir = getenv("FOOT_PENDULUM_FRAME_DIR");
    if (frame_dir != NULL && frame_dir[0] != '\0') {
        static int frame_idx = 0;
        static int frame_limit = -1;
        if (frame_limit < 0) {
            const char* limit_env = getenv("FOOT_PENDULUM_FRAME_LIMIT");
            frame_limit = (limit_env != NULL && limit_env[0] != '\0') ? atoi(limit_env) : 0;
        }
        if (frame_limit == 0 || frame_idx < frame_limit) {
            char frame_path[4096];
            snprintf(frame_path, sizeof(frame_path), "%s/frame_%06d.png", frame_dir, frame_idx);
            Image frame = LoadImageFromScreen();
            ExportImage(frame, frame_path);
            UnloadImage(frame);
            frame_idx += 1;
        }
    }
}

void c_close(FootPendulum* env) {
    (void)env;
    if (IsWindowReady()) CloseWindow();
}
