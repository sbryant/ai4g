#include "entity.h"

Entity *entity_make(Entity *in) {
    Entity *out = in;
    if(!in)
        out = (Entity *)calloc(1, sizeof(Entity));

    if(!(out->kinematic))
        out->kinematic = km_make(NULL);
    return out;
}

SDL_Rect *entity_make_rect(Entity *e, SDL_Rect *output) {
    if(!output)
        output = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    output->x = e->spacing_x * roundf(e->kinematic->position.x) + 1.0f;
    output->y = e->spacing_y * roundf(e->kinematic->position.y) + 1.0f;

    output->w = e->spacing_x - 1.0f;
    output->h = e->spacing_y - 1.0f;

    return output;
}

Static *entity_make_static(Entity *e, Static *s) {
    if(!s)
        s = static_make(NULL);

    s->position.x = roundf(e->kinematic->position.x);
    s->position.y = roundf(e->kinematic->position.y);
    s->position.z = 0.0f;
    s->orientation = 0.0f;

    return s;
}
