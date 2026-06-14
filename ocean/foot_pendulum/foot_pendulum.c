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
static void init_default_env(FootPendulum* env, float observations[FP_OBS_SIZE],
        float actions[FP_NUM_ACTIONS], float rewards[1], float terminals[1]) {
    memset(env, 0, sizeof(*env));
    env->observations = observations;
    env->actions = actions;
    env->rewards = rewards;
    env->terminals = terminals;
    env->num_agents = 1;
    env->env_id = 0;
    env->rng = 1;
    load_default_config(env);
    init(env);
}

static int run_validity_check(void) {
    float observations[FP_OBS_SIZE] = {0};
    float actions[FP_NUM_ACTIONS] = {0};
    float rewards[1] = {0};
    float terminals[1] = {0};
    FootPendulum env;
    init_default_env(&env, observations, actions, rewards, terminals);

    c_reset(&env);
    env.plate_x = 0.0f;
    env.plate_z = 1.0f;
    env.plate_pitch = 0.0f;
    env.plate_vx = 0.0f;
    env.plate_vz = 0.0f;
    env.plate_pitch_dot = 0.0f;
    env.pendulum_theta = 0.0f;
    env.pendulum_theta_dot = 0.0f;
    compute_reward(&env, actions, false);
    float calm_upright_reward = env.reward_current;

    env.pendulum_theta = 0.0f;
    env.pendulum_theta_dot = 8.0f;
    compute_reward(&env, actions, false);
    float fast_vertical_reward = env.reward_current;
    float fast_vertical_catch = env.reward_catch_bonus;

    if (!(fast_vertical_catch < 0.001f)) {
        fprintf(stderr, "validity failed: fast vertical catch bonus %.6f is too high\n", fast_vertical_catch);
        return 1;
    }
    if (!(fast_vertical_reward < 0.5f * calm_upright_reward)) {
        fprintf(stderr, "validity failed: fast vertical reward %.6f is too close to calm reward %.6f\n",
            fast_vertical_reward, calm_upright_reward);
        return 1;
    }

    load_default_config(&env);
    env.reset_hanging_pct = 100.0f;
    env.reset_near_upright_pct = 0.0f;
    env.reset_random_pct = 0.0f;
    c_reset(&env);
    int random_steps = 50000;
    int bounds_episodes = 0;
    float max_random_success = 0.0f;
    for (int i = 0; i < random_steps; i++) {
        actions[0] = fp_randf(&env, -1.0f, 1.0f);
        actions[1] = fp_randf(&env, -1.0f, 1.0f);
        actions[2] = fp_randf(&env, -1.0f, 1.0f);
        c_step(&env);
        float success = env.success_steps_max * env.dt;
        if (success > max_random_success) max_random_success = success;
        if (terminals[0] > 0.0f) bounds_episodes += 1;
    }
    if (!(max_random_success < 0.25f)) {
        fprintf(stderr, "validity failed: random hanging policy max success %.6f is too high\n", max_random_success);
        return 1;
    }

    printf("passive_pendulum=ok actions=plate_accel_only\n");
    printf("calm_upright_reward=%.6f fast_vertical_reward=%.6f fast_vertical_catch=%.6f\n",
        calm_upright_reward, fast_vertical_reward, fast_vertical_catch);
    printf("random_hanging_steps=%d max_success_duration=%.6f bounds_or_timeout_resets=%d\n",
        random_steps, max_random_success, bounds_episodes);
    return 0;
}


static float key_axis(int negative_key_a, int negative_key_b, int positive_key_a, int positive_key_b) {
    float value = 0.0f;
    if (IsKeyDown(negative_key_a) || IsKeyDown(negative_key_b)) value -= 1.0f;
    if (IsKeyDown(positive_key_a) || IsKeyDown(positive_key_b)) value += 1.0f;
    return value;
}

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "--validity-check") == 0) {
        return run_validity_check();
    }

    float observations[FP_OBS_SIZE] = {0};
    float actions[FP_NUM_ACTIONS] = {0};
    float rewards[1] = {0};
    float terminals[1] = {0};
    FootPendulum env;

    init_default_env(&env, observations, actions, rewards, terminals);
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
