#include "foot_pendulum.h"

static void load_default_config(FootPendulum* env) {
    env->dt = 0.016666667f;
    env->pendulum_length = 1.0f;
    env->gravity = 9.81f;
    env->hinge_damping = 0.05f;
    env->x_limit = 2.0f;
    env->z_min = 0.35f;
    env->z_max = 1.60f;
    env->pitch_limit = 0.90f;
    env->max_plate_velocity = 4.0f;
    env->max_plate_accel = 20.0f;
    env->max_pitch_rate = 5.0f;
    env->max_pitch_accel = 30.0f;
    env->max_theta_dot = 20.0f;
    env->success_angle_rad = 0.174533f;
    env->success_angular_velocity = 1.0f;
    env->success_steps = 180;
    env->min_horizon_steps = 240;
    env->max_horizon_steps = 480;
    env->height_reward_weight = 0.5f;
    env->upright_reward_weight = 0.5f;
    env->catch_bonus_weight = 3.0f;
    env->stillness_sigma = 1.5f;
    env->action_penalty_weight = 0.01f;
    env->action_rate_penalty_weight = 0.02f;
    env->plate_smoothness_weight = 0.005f;
    env->reset_hanging_pct = 30.0f;
    env->reset_near_upright_pct = 30.0f;
    env->reset_random_pct = 40.0f;
}

static float key_axis(int negative_key_a, int negative_key_b, int positive_key_a, int positive_key_b) {
    float value = 0.0f;
    if (IsKeyDown(negative_key_a) || IsKeyDown(negative_key_b)) value -= 1.0f;
    if (IsKeyDown(positive_key_a) || IsKeyDown(positive_key_b)) value += 1.0f;
    return value;
}

int main(void) {
    float observations[FP_OBS_SIZE] = {0};
    float actions[FP_NUM_ACTIONS] = {0};
    float rewards[1] = {0};
    float terminals[1] = {0};

    FootPendulum env = {
        .observations = observations,
        .actions = actions,
        .rewards = rewards,
        .terminals = terminals,
        .num_agents = 1,
        .rng = 1,
    };

    load_default_config(&env);
    init(&env);
    c_reset(&env);
    c_render(&env);

    bool random_actions = false;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) random_actions = !random_actions;

        if (random_actions) {
            actions[0] = fp_randf(&env, -1.0f, 1.0f);
            actions[1] = fp_randf(&env, -1.0f, 1.0f);
            actions[2] = fp_randf(&env, -1.0f, 1.0f);
        } else {
            actions[0] = key_axis(KEY_LEFT, KEY_A, KEY_RIGHT, KEY_D);
            actions[1] = key_axis(KEY_S, KEY_DOWN, KEY_W, KEY_UP);
            actions[2] = key_axis(KEY_Q, KEY_J, KEY_E, KEY_L);
        }

        c_step(&env);
        c_render(&env);
    }

    c_close(&env);
    return 0;
}
