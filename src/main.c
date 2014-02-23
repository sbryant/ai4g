#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>

#include <SDL.h>

#include "vector.h"
#include "kinematic.h"
#include "entity.h"

#define GRID_H 50
#define GRID_W 50

#define GRID_SIZE_X 600
#define GRID_SIZE_Y 600

#define GRID_SPACING_X GRID_SIZE_X / GRID_W
#define GRID_SPACING_Y GRID_SIZE_Y / 6RID_H

typedef struct s_app_state {
    short quit;

} AppState;

int KeyboardEventFilter(void *user_data, SDL_Event *event) {
    AppState *app = (AppState*)user_data;
    SDL_KeyboardEvent ke;
    switch(event->type) {
    case SDL_KEYUP:
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

void place_entity(Entity *e, int x, int y) {
    e->x = x, e->y = y;

    // offset by a pixel
    e->position.x = e->spacing_x + 1.0f;
    e->position.y = e->spacing_y + 1.0f;

    // offset
    e->w = e->spacing_x - 1.0f;
    e->h = e->spacing_y - 1.0f;

    e->position.x *= (x + 1);
    e->position.x -= x;

    e->position.y *= (y + 1);
    e->position.y -= y;
}

void entity_init(Entity *e, int spacing_x, int spacing_y) {
    bzero(&(e->position), sizeof(vec3));
    e->w = 0, e->h = 0;
    e->x = 0, e->y = 0;
    e->spacing_x = spacing_x, e->spacing_y = spacing_y;
    e->r = 0, e->g = 0, e->b = 0;
    e->a = SDL_ALPHA_OPAQUE;
    e->kinematic->position.x = 0.0f, e->kinematic->position.y = 0.0f, e->kinematic->position.z = 0.0f;
    e->kinematic->orientation = 0.0f;
    place_entity(e, 0, 0);
}

void render_entity(SDL_Renderer *renderer, Entity *e) {
    SDL_Rect rect;
    entity_make_rect(e, &rect);
    // Draw entity A
    SDL_SetRenderDrawColor(renderer,
                           e->r, e->g, e->b, e->a);
    SDL_RenderFillRect(renderer, &rect);
}

void render_grid(SDL_Renderer *renderer, SDL_Point *grid, int w, int h) {
    SDL_SetRenderDrawColor(renderer,
                           0, 255, 0, SDL_ALPHA_OPAQUE);

    // Draw grid
    int index = 0;
    for(int i = 1; i < w; i++, index+=2)
        SDL_RenderDrawLines(renderer, &grid[index], 2);

    for(int i = 0; i < h; i++, index+=2)
        SDL_RenderDrawLines(renderer, &grid[index], 2);
}

int main(int argc, char** argv) {
    AppState app;
    app.quit = 0;

    SDL_Init(SDL_INIT_VIDEO);

    atexit(SDL_Quit);

    const char *program_title = "AI 4 Games";

    SDL_Window *window = SDL_CreateWindow(program_title,
                                          SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_AddEventWatch(KeyboardEventFilter, (void*)&app);

    int num_vert_lines = GRID_W;
    int num_horiz_lines = GRID_H;
    //int x_size = 4;
    //int y_size = 4;
    int index = 0;

    SDL_Point *grid_points = NULL;

    grid_points = calloc((num_vert_lines * 2)  + (num_horiz_lines * 2), sizeof(SDL_Point));

    float spacing_x = (600 / (float)num_vert_lines);
    float spacing_y = (600 / (float)num_horiz_lines);

    //int start = 1280 / 2;

    for(int x = 0; x < num_vert_lines; x++,index+=2) {
        SDL_Point vl_start = { (x+1) * (spacing_x), spacing_y };
        SDL_Point vl_end = { (x+1) * (spacing_x), 600 - spacing_y};

        grid_points[index] = vl_start;
        grid_points[index+1] = vl_end;
    }

    for(int y = 0; y < num_horiz_lines; y++,index+=2) {
        SDL_Point hl_start = { spacing_x,  (y+1) * spacing_y};
        SDL_Point hl_end = { 600, (y+1) * spacing_y };

        grid_points[index] = hl_start;
        grid_points[index+1] = hl_end;
    }

    Entity player;
    player.kinematic = km_make(NULL);
    entity_init(&player, spacing_x, spacing_y);
    player.r = 255, player.b = 255;

    Entity target;
    target.kinematic = km_make(NULL);
    entity_init(&target, spacing_x, spacing_y);
    target.b = 255;

    place_entity(&target, 4.0f, 19.0f);

    float simulation_time = 0, now = 0;
    while(1) {
        now = SDL_GetTicks();

        while(simulation_time < now) {
            simulation_time += 16.0f;

            KinematicSeek k;
            bzero(&k, sizeof(KinematicSeek));
            k.max_speed = 1.0f;
            entity_make_static(&target, &(k.target));
            entity_make_static(&player, &(k.character));

            KinematicSteeringOutput *ksteering = kmseek_get_steering(&k);
            SteeringOutput steering;
            vec3_set_vec3(&(steering.linear), &(ksteering->velocity));

            steering.angular = 0.0f;

            vec3_set_vec3(&(player.kinematic->velocity), &(steering.linear));

            km_update(player.kinematic, &steering, 1.0f);
            free(ksteering);

            place_entity(&player, roundf(player.kinematic->position.x), roundf(player.kinematic->position.y));
        }

        SDL_SetRenderDrawColor(renderer,
                               0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        render_grid(renderer, grid_points, num_vert_lines, num_horiz_lines);

        render_entity(renderer, &player);
        render_entity(renderer, &target);

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while(SDL_PollEvent(&event));

        if(app.quit)
            break;
    }

    free(grid_points);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
