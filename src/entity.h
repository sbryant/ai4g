#pragma once
#include <stdlib.h>
#include <SDL.h>
#include "vector.h"
#include "kinematic.h"

typedef struct s_entity {
    vec3 position;
    Kinematic *kinematic;
    float w, h;
    int x, y;
    int spacing_x;
    int spacing_y;
    short r,g,b,a;
} Entity;

Entity *entity_make(Entity *in);
SDL_Rect *entity_make_rect(Entity *e, SDL_Rect *output);
Static *entity_make_static(Entity *e, Static *s);
