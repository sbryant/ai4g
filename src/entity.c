#include "entity.h"

Entity *entity_make(Entity *in) {
    Entity *out = in;
    if(!in)
        out = (Entity *)calloc(1, sizeof(Entity));

    if(!(out->kinematic))
        out->kinematic = km_make(NULL);
    return out;
}

SDL_Rect *entity_make_rect(Entity *e, int w, int h, SDL_Rect *output, int snap) {
    if(!output)
        output = (SDL_Rect *)malloc(sizeof(SDL_Rect));

    float x = e->kinematic->position.x;
    float y = e->kinematic->position.y;

    if(snap) {
        x = roundf(x);
        y = roundf(y);
    }

    output->x = (w + 1) * x + 1.0f;
    output->y = (h + 1) * y + 1.0f;

    output->w = w;
    output->h = h;

    return output;
}

Static *entity_make_static(Entity *e, Static *s) {
    if(!s)
        s = static_make(NULL);

    s->position.x = e->kinematic->position.x;
    s->position.y = e->kinematic->position.y;
    s->position.z = 1.0f;
    s->orientation = e->kinematic->orientation;

    return s;
}

void entity_init(Entity *e) {
    if(!(e->kinematic))
        e->kinematic = km_make(NULL);
    e->kinematic->position.x = 0.0f, e->kinematic->position.y = 0.0f, e->kinematic->position.z = 0.0f;
    e->kinematic->orientation = 0.0f;
}
