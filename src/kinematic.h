#pragma once
#include "vector.h"

typedef struct s_static {
    vec3 position;
    float orientation;
} Static;

typedef struct s_kinematic {
    vec3 position;
    float orientation;
    vec3 velocity;
    float rotation;
    float max_accel;
} Kinematic;

typedef struct s_kinematic_steering_output {
    vec3 velocity;
    float rotation;
} KinematicSteeringOutput;

typedef struct s_steering_output {
    vec3 linear;
    float angular;
} SteeringOutput;

typedef struct s_kinematic_seek {
    Static character;
    Static target;
    float max_speed;
} KinematicSeek;

typedef struct s_kinematic_arrive {
    Static character;
    Static target;
    float max_speed;
    float radius;
    float time_to_target;
} KinematicArrive;

typedef struct s_kinematic_wander {
    Static character;
    float max_speed;
    float max_rotation;
} KinematicWander;

Static* static_make(Static *s);
Kinematic* km_make(Kinematic *k);
KinematicSteeringOutput* kso_make(KinematicSteeringOutput *s);
SteeringOutput* so_make(SteeringOutput *s);
KinematicSeek* kmseek_make(KinematicSeek *k);
KinematicArrive* kmarrive_make(KinematicArrive *k);
KinematicWander* kmwander_make(KinematicWander *k);
float get_new_orientation(Static *s, float current, vec3 *velocity);
void km_update(Kinematic *input, SteeringOutput *steering, const float max_speed, const float time);
KinematicSteeringOutput* kmseek_get_steering(KinematicSeek* k);
KinematicSteeringOutput* kmarrive_get_steering(KinematicArrive* k);
KinematicSteeringOutput* kmwander_get_steering(KinematicWander* k);
