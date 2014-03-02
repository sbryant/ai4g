#pragma once
#include <stdlib.h>
#include <SDL.h>
#include "vector.h"
#include "kinematic.h"

typedef struct s_entity {
    Kinematic *kinematic;
} Entity;

Entity *entity_make(Entity *in);
SDL_Rect *entity_make_rect(Entity *e, int w, int h, SDL_Rect *output, int snap);
Static *entity_make_static(Entity *e, Static *s);
void entity_init(Entity *e);
