#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <time.h>

#include <SDL.h>

#include "vector.h"
#include "kinematic.h"
#include "entity.h"

#define GRID_H 50
#define GRID_W 50

#define GRID_SIZE_W 600
#define GRID_SIZE_H 600

#define GRID_SPACING_X GRID_SIZE_W / GRID_W
#define GRID_SPACING_Y GRID_SIZE_H / 6RID_H

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

void render_entity(SDL_Surface *surface, Entity *e, int w, int h, int r, int g, int b, int a) {
    SDL_Rect rect;
    entity_make_rect(e, w, h, &rect);
    SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, r, g, b, SDL_ALPHA_OPAQUE));
}

void render_grid(SDL_Surface *surface, SDL_Rect *grid, int count) {
    SDL_FillRects(surface, grid, count, SDL_MapRGBA(surface->format, 0, 255, 0, SDL_ALPHA_OPAQUE));
}

void wrap_position(Entity *e) {
    if(roundf(e->kinematic->position.x) > (GRID_W - 1)) {
        e->kinematic->position.x = 0.0f;
    }

    if(roundf(e->kinematic->position.x) < 0.0f) {
        e->kinematic->position.x = GRID_W - 1;
    }

    if(roundf(e->kinematic->position.y) > (GRID_H - 1)) {
        e->kinematic->position.y = 0.0f;
    }

    if(roundf(e->kinematic->position.y) < 0.0f) {
        e->kinematic->position.y = (GRID_H -1);
    }
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
    float spacing_x = (GRID_SIZE_W / (float)num_vert_lines);
    float spacing_y = (GRID_SIZE_H / (float)num_horiz_lines);

    SDL_Rect screen_rect;
    screen_rect.x = 800 / 2 - GRID_SIZE_W / 2;
    screen_rect.y = 600 / 2 - GRID_SIZE_H / 2;
    screen_rect.w = GRID_SIZE_W;
    screen_rect.h = GRID_SIZE_H;

    SDL_Surface *grid_surface = NULL;
    SDL_Texture *grid_texture = NULL;

    grid_surface = SDL_CreateRGBSurface(0, GRID_SIZE_W, GRID_SIZE_H, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    grid_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, GRID_SIZE_W, GRID_SIZE_H);


    SDL_Rect *grid_rects = NULL;
    grid_rects = calloc(num_vert_lines + num_horiz_lines, sizeof(SDL_Rect));

    int index = 0;
    for(int x = 0; x < num_vert_lines; x++,index++) {
        grid_rects[index].x = x * spacing_x;
        grid_rects[index].y = 0;
        grid_rects[index].w = 1;
        grid_rects[index].h = GRID_SIZE_H;
    }

    for(int y = 0; y < num_horiz_lines; y++,index++) {
        grid_rects[index].x = 0;
        grid_rects[index].y = y * spacing_y;
        grid_rects[index].w = GRID_SIZE_W;
        grid_rects[index].h = 1;
    }

    Entity player; bzero(&player, sizeof(Entity));
    entity_init(&player);

    Entity target; bzero(&target, sizeof(Entity));
    entity_init(&target);

    /* initial positions in the world */
    target.kinematic->position.x = 4.0f;
    target.kinematic->position.y = 19.0f;

    player.kinematic->position.x = 20.0f;
    player.kinematic->position.y = 30.0f;

    float simulation_time = 0, now = 0;
    srand(time(NULL));
    while(1) {
        now = SDL_GetTicks();

        while(simulation_time < now) {
            simulation_time += 16.0f;

            // Setup steering algorithim inputs
            KinematicArrive k;
            bzero(&k, sizeof(KinematicArrive));
            k.max_speed = 0.1f;
            k.radius = 1.0f;
            k.time_to_target = 0.25f;
            entity_make_static(&target, &(k.target));
            entity_make_static(&player, &(k.character));

            KinematicWander kw;
            bzero(&kw, sizeof(KinematicWander));
            kw.max_speed = 0.1f;
            kw.max_rotation = 1.0f;
            entity_make_static(&target, &(kw.character));

            // Get velocity and orientation
            KinematicSteeringOutput *ksteering = kmarrive_get_steering(&k);

            if(ksteering) {
                // Setup our steering from the KinematicSteeringOutput
                SteeringOutput steering;
                vec3_set_vec3(&(steering.linear), &(ksteering->velocity));
                steering.angular = 0.0f;

                // Update chasing character's velocity from the KinematicSteeringOutput
                // This will actually stop the character once it is close enough
                vec3_set_vec3(&(player.kinematic->velocity), &(steering.linear));

                // Move the character
                km_update(player.kinematic, &steering, 1.0f);

                // don't wander off the world.
                wrap_position(&player);

                free(ksteering);
            }

            KinematicSteeringOutput *kwsteering = kmwander_get_steering(&kw);
            SteeringOutput wsteering;
            wsteering.angular = kwsteering->rotation;
            vec3_set_vec3(&(wsteering.linear), &(kwsteering->velocity));

            // Cap target velocity
            vec3_set_vec3(&(target.kinematic->velocity), &(wsteering.linear));

            // let the target wander
            km_update(target.kinematic, &wsteering, 1.0f);
            wrap_position(&target);

            free(kwsteering);
        }

        // Draw grid and Entities
        SDL_FillRect(grid_surface, NULL, SDL_MapRGBA(grid_surface->format, 0, 0, 0, 255));
        render_grid(grid_surface, grid_rects, num_vert_lines + num_horiz_lines);

        // entities are spacing_x - 1 wide, / spacing_y - 1 pixels high (fills in a grid)

        // blue player
        render_entity(grid_surface, &player, spacing_x - 1, spacing_y - 1, 0, 0, 255, 0);
        // purple target
        render_entity(grid_surface, &target, spacing_x - 1, spacing_y - 1, 180, 0, 255, 0);

        // Upload pixels to video card
        SDL_UpdateTexture(grid_texture, NULL, grid_surface->pixels, grid_surface->pitch);

        // Reset to black
        SDL_SetRenderDrawColor(renderer,
                               0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Copy texture to framebuffer with rect
        SDL_RenderCopy(renderer, grid_texture, NULL, &screen_rect);

        // Show frame buffer
        SDL_RenderPresent(renderer);

        // Process any events
        SDL_Event event;
        while(SDL_PollEvent(&event));

        if(app.quit)
            break;
    }

    free(player.kinematic);
    free(target.kinematic);
    free(grid_rects);

    SDL_DestroyTexture(grid_texture);
    SDL_FreeSurface(grid_surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
