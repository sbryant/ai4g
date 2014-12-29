#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <time.h>

#include <SDL.h>

#include "vector.h"
#include "kinematic.h"
#include "entity.h"
#include "dynamic.h"

#define GRID_H 75
#define GRID_W 75

#define SCREEN_SIZE_W 800
#define SCREEN_SIZE_H 600

#define GRID_SIZE_W SCREEN_SIZE_H
#define GRID_SIZE_H SCREEN_SIZE_H

#define GRID_SPACING_X (GRID_SIZE_W / GRID_W)
#define GRID_SPACING_Y (GRID_SIZE_H / GRID_H)

typedef struct s_app_state {
    short quit;
    short pause;
    short snap;
    short run_sim;
} AppState;

static SDL_Rect screen_rect;

int KeyboardEventFilter(void *user_data, SDL_Event *event) {
    AppState *app = (AppState*)user_data;
    SDL_KeyboardEvent ke;
    switch(event->type) {
    case SDL_KEYUP:
        ke = event->key;
        if(ke.keysym.sym == SDLK_SPACE)
            app->pause = app->pause ? 0 : 1;
        if(ke.keysym.sym == SDLK_s)
            app->snap = app->snap ? 0 : 1;
        if(ke.keysym.sym == SDLK_r)
            app->run_sim = app->run_sim ? 0 : 1;
        break;
    case SDL_KEYDOWN:
        ke = event->key;
        if(ke.keysym.sym == SDLK_q && ke.keysym.mod & KMOD_GUI)
            app->quit = 1;
        break;
    case SDL_WINDOWEVENT:
        if(event->window.event == SDL_WINDOWEVENT_CLOSE)
            app->quit = 1;
        break;
    }

    return 0;
}

void render_entity(SDL_Renderer *renderer, Entity *e, int snap, int w, int h, int r, int g, int b, int a) {
    SDL_Rect rect = {};
    entity_make_rect(e, w, h, &rect, snap);
    rect.x += screen_rect.x;
    rect.y += screen_rect.y;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}

void render_grid(SDL_Renderer *renderer, SDL_Rect *grid, int count) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
    SDL_RenderFillRects(renderer, grid, count);

    SDL_Rect last_rect = grid[count - 1];
    SDL_Rect final = last_rect;

    // fill out gutter
    final.x = (GRID_SPACING_X * count) / 2 + screen_rect.x;
    final.y = 0;
    final.h = GRID_SIZE_H;
    final.w = 1;

    SDL_RenderFillRect(renderer, &final);
}

void wrap_position(Entity *e) {
    if(roundf(e->kinematic->position.x) > ((float)GRID_W - 1.0f)) {
        e->kinematic->position.x = 0.0f;
    }

    if(roundf(e->kinematic->position.x) < 0.0f) {
        e->kinematic->position.x = (float)GRID_W - 1.0f;
    }

    if(roundf(e->kinematic->position.y) > ((float)GRID_H - 1.0f)) {
        e->kinematic->position.y = 0.0f;
    }

    if(roundf(e->kinematic->position.y) < 0.0f) {
        e->kinematic->position.y = ((float)GRID_H -1.0f);
    }
}

int main(int argc, char** argv) {
    AppState app;
    app.quit = 0;
    app.pause = 0;
    app.snap = 1;
    app.run_sim = 0;

    SDL_Init(SDL_INIT_VIDEO);

    atexit(SDL_Quit);

    const char *program_title = "AI 4 Games";

    SDL_Window *window = SDL_CreateWindow(program_title,
                                          SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED,
                                          SCREEN_SIZE_W, SCREEN_SIZE_H, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_AddEventWatch(KeyboardEventFilter, (void*)&app);

    int num_vert_lines = GRID_W;
    int num_horiz_lines = GRID_H;
    float spacing_x = (GRID_SIZE_W / (float)num_vert_lines);
    float spacing_y = (GRID_SIZE_H / (float)num_horiz_lines);


    screen_rect.x = SCREEN_SIZE_W / 2 - GRID_SIZE_W / 2;
    screen_rect.y = SCREEN_SIZE_H / 2 - GRID_SIZE_H / 2;
    screen_rect.w = GRID_SIZE_W;
    screen_rect.h = GRID_SIZE_H;

    SDL_Rect *grid_rects = NULL;
    grid_rects = calloc(num_vert_lines + num_horiz_lines, sizeof(SDL_Rect));

    int index = 0;
    for(int x = 0; x < num_vert_lines; x++,index++) {
        grid_rects[index].x = (x * spacing_x) + screen_rect.x;
        grid_rects[index].y = screen_rect.y;
        grid_rects[index].w = 1;
        grid_rects[index].h = GRID_SIZE_H;
    }

    for(int y = 0; y < num_horiz_lines; y++,index++) {
        grid_rects[index].x = screen_rect.x;
        grid_rects[index].y = y * spacing_y;
        grid_rects[index].w = GRID_SIZE_W;
        grid_rects[index].h = 1;
    }

    Entity player = { 0 };
    entity_init(&player);

    Entity target = { 0 };
    entity_init(&target);

    /* initial positions in the world */
    target.kinematic->position.x = 49.0f;
    target.kinematic->position.y = 49.0f;

    player.kinematic->position.x = 0.0f;
    player.kinematic->position.y = 0.0f;

    player.kinematic->max_accel = 10.0f;

    float now = 0.0f, prev = 0.0f;

    // 60fps time step.
    float max_speed = 10.0f;
    float timeout = 0.0f;
    srand(time(NULL));

    while(1) {
        // Process any events
        SDL_Event event;
        while(SDL_PollEvent(&event));

        if(app.quit)
            break;

        if(app.pause)
            continue;

        now = SDL_GetTicks();
        float dt = (now - prev) / 1000.0f;

        if(timeout == 0.0f) {
            // 500 ms timeout
            timeout = now + 500;
        }

        if(SDL_TICKS_PASSED(now, timeout) && app.run_sim) {

            fprintf(stderr, "prev %0.3fms, now %0.3fms dt: %0.3fs\n", prev,  now, dt);

            fprintf(stderr, "player\n");
            vec3_print(&(target.kinematic->position));
            fprintf(stderr, "velocity:%0.3fm/s\n", vec3_length((&(player.kinematic->velocity))));
            fprintf(stderr, "max acceleration:%0.3fm/s\n", (player.kinematic->max_accel));

            fprintf(stderr, "target\n");
            vec3_print(&(target.kinematic->position));
            fprintf(stderr, "velocity:%0.3fm/s\n", vec3_length((&(target.kinematic->velocity))));
            fprintf(stderr, "max acceleration:%0.3fm/s\n", (target.kinematic->max_accel));
            fprintf(stderr, "========\n");
            timeout = 0.0f; // reset timeout next loop
        }


        // Setup steering algorithim inputs
        Arrive k;
        arrive_init(&k, player.kinematic, target.kinematic);

        // Tune dynamic arrive speed and acceleartion
        k.max_speed = max_speed;
        k.max_acceleration = player.kinematic->max_accel;
        k.target_radius = 1.0f;
        k.slow_radius = 1.0f;

        KinematicWander kw = {};
        kw.max_speed = max_speed;
        kw.max_rotation = 0.5f;
        entity_make_static(&target, &(kw.character));

        // Get velocity and orientation
        SteeringOutput ksteering = {};
        if(arrive_get_steering(&k, &ksteering) && app.run_sim) {
            // Move the character
            km_update(player.kinematic, &ksteering, max_speed, dt);

            // don't wander off the world.
            wrap_position(&player);
        }

        KinematicSteeringOutput kwsteering = {};
        kmwander_get_steering(&kw, &kwsteering);

        SteeringOutput wsteering;
        wsteering.angular = kwsteering.rotation;
        vec3_set_vec3(&(wsteering.linear), &(kwsteering.velocity));

        // let the target wander
        if(app.run_sim)
            km_update(target.kinematic, &wsteering, max_speed, dt);
        wrap_position(&target);

        // Reset to black
        SDL_SetRenderDrawColor(renderer,
                               0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Draw grid and Entities
        render_grid(renderer, grid_rects, num_vert_lines + num_horiz_lines);

        // entities are spacing_x - 1 wide, / spacing_y - 1 pixels high (fills in a grid)
        // blue player
        render_entity(renderer, &player, app.snap, spacing_x - 1, spacing_y - 1, 0, 0, 255, 0);
        // purple target
        render_entity(renderer, &target, app.snap, spacing_x - 1, spacing_y - 1, 180, 0, 255, 0);

        // Show frame buffer
        SDL_RenderPresent(renderer);

        prev = now;
    }

    free(player.kinematic);
    free(target.kinematic);
    free(grid_rects);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
