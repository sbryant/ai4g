#include "entity.h"

Entity *entity_make(Entity *in) {
    Entity *out = in;
    if(!in)
        out = (Entity *)calloc(1, sizeof(Entity));
    return out;
}

SDL_Rect *entity_make_rect(Entity *e, SDL_Rect *output) {
    if(!output)
        output = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    output->x = e->position.x, output->y = e->position.y;
    output->w = e->w, output->h = e->h;

    return output;
}

Static *entity_make_static(Entity *e, Static *s) {
    if(!s)
        s = static_make(NULL);

    vec3_set_vec3(&(s->position), &(e->position));
    s->orientation = 0.0f;

    return s;
}
