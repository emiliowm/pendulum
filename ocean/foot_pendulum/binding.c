#include "foot_pendulum.h"

#define OBS_SIZE FP_OBS_SIZE
#define NUM_ATNS 3
#define ACT_SIZES {1, 1, 1}
#define OBS_TENSOR_T FloatTensor

#define Env FootPendulum
#include "vecenv.h"

static float fp_read_float(Dict* kwargs, const char* key) {
    return (float)dict_get(kwargs, key)->value;
}

static int fp_read_int(Dict* kwargs, const char* key) {
    return (int)dict_get(kwargs, key)->value;
}

void my_init(Env* env, Dict* kwargs) {
    env->num_agents = 1;
    env->dt = fp_read_float(kwargs, "dt");
    env->pendulum_length = fp_read_float(kwargs, "pendulum_length");
    env->gravity = fp_read_float(kwargs, "gravity");
    env->hinge_damping = fp_read_float(kwargs, "hinge_damping");
    env->x_limit = fp_read_float(kwargs, "x_limit");
    env->z_min = fp_read_float(kwargs, "z_min");
    env->z_max = fp_read_float(kwargs, "z_max");
    env->pitch_limit = fp_read_float(kwargs, "pitch_limit");
    env->max_plate_velocity = fp_read_float(kwargs, "max_plate_velocity");
    env->max_plate_accel = fp_read_float(kwargs, "max_plate_accel");
    env->max_pitch_rate = fp_read_float(kwargs, "max_pitch_rate");
    env->max_pitch_accel = fp_read_float(kwargs, "max_pitch_accel");
    env->max_theta_dot = fp_read_float(kwargs, "max_theta_dot");
    env->success_angle_rad = fp_read_float(kwargs, "success_angle_rad");
    env->success_angular_velocity = fp_read_float(kwargs, "success_angular_velocity");
    env->success_steps = fp_read_int(kwargs, "success_steps");
    env->min_horizon_steps = fp_read_int(kwargs, "min_horizon_steps");
    env->max_horizon_steps = fp_read_int(kwargs, "max_horizon_steps");
    env->height_reward_weight = fp_read_float(kwargs, "height_reward_weight");
    env->upright_reward_weight = fp_read_float(kwargs, "upright_reward_weight");
    env->catch_bonus_weight = fp_read_float(kwargs, "catch_bonus_weight");
    env->stillness_sigma = fp_read_float(kwargs, "stillness_sigma");
    env->action_penalty_weight = fp_read_float(kwargs, "action_penalty_weight");
    env->action_rate_penalty_weight = fp_read_float(kwargs, "action_rate_penalty_weight");
    env->plate_smoothness_weight = fp_read_float(kwargs, "plate_smoothness_weight");
    env->reset_hanging_pct = fp_read_float(kwargs, "reset_hanging_pct");
    env->reset_near_upright_pct = fp_read_float(kwargs, "reset_near_upright_pct");
    env->reset_random_pct = 100.0f - env->reset_hanging_pct - env->reset_near_upright_pct;

    if (env->reset_hanging_pct + env->reset_near_upright_pct > 100.0f) {
        fprintf(stderr,
            "foot_pendulum config error: reset_hanging_pct + reset_near_upright_pct must be <= 100, got %.3f + %.3f\n",
            env->reset_hanging_pct, env->reset_near_upright_pct);
        exit(EXIT_FAILURE);
    }
    if (env->min_horizon_steps <= 0 || env->max_horizon_steps < env->min_horizon_steps) {
        fprintf(stderr,
            "foot_pendulum config error: expected 0 < min_horizon_steps <= max_horizon_steps, got %d <= %d\n",
            env->min_horizon_steps, env->max_horizon_steps);
        exit(EXIT_FAILURE);
    }
    if (env->z_max <= env->z_min) {
        fprintf(stderr,
            "foot_pendulum config error: expected z_max > z_min, got %.3f <= %.3f\n",
            env->z_max, env->z_min);
        exit(EXIT_FAILURE);
    }

    init(env);
}

void my_log(Log* log, Dict* out) {
    dict_set(out, "perf", log->perf);
    dict_set(out, "score", log->score);
    dict_set(out, "episode_return", log->episode_return);
    dict_set(out, "episode_length", log->episode_length);
    dict_set(out, "reward_tip_height", log->reward_tip_height);
    dict_set(out, "reward_upright", log->reward_upright);
    dict_set(out, "reward_stillness", log->reward_stillness);
    dict_set(out, "reward_catch_bonus", log->reward_catch_bonus);
    dict_set(out, "reward_plate_smoothness", log->reward_plate_smoothness);
    dict_set(out, "reward_action_penalty", log->reward_action_penalty);
    dict_set(out, "reward_action_rate_penalty", log->reward_action_rate_penalty);
    dict_set(out, "reward_bounds_penalty", log->reward_bounds_penalty);
    dict_set(out, "success_duration", log->success_duration);
    dict_set(out, "max_success_duration", log->max_success_duration);
    dict_set(out, "hanging_success_duration", log->hanging_success_duration);
    dict_set(out, "hanging_success_episode", log->hanging_success_episode);
    dict_set(out, "kick_peak_plate_speed", log->kick_peak_plate_speed);
    dict_set(out, "invalid_state", log->invalid_state);
    dict_set(out, "nan_termination", log->nan_termination);
    dict_set(out, "bounds_termination", log->bounds_termination);
    dict_set(out, "reset_hanging", log->reset_hanging);
    dict_set(out, "reset_near_upright", log->reset_near_upright);
    dict_set(out, "reset_random", log->reset_random);
    dict_set(out, "n", log->n);
}
