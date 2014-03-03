#pragma once
#include "kinematic.h"

typedef struct s_seek {
    Kinematic *character;
    Kinematic *target;
    float max_acceleration;
} Seek;

typedef struct s_arrive {
    Kinematic *character;
    Kinematic *target;

    float max_acceleration;
    float max_speed;

    float target_radius;
    float slow_radius;

    float time_to_target;
} Arrive;

typedef struct s_align {
    Kinematic *character;
    Kinematic *target;

    float max_angular_acceleration;
    float max_rotation;

    float target_radius;
    float slow_radius;

    float time_to_target;
} Align;


typedef struct s_velocity_match {
    Kinematic *character;
    Kinematic *target;

    float max_acceleration;
    float time_to_target;
} VelocityMatch;

Seek *seek_make(Seek *in);

SteeringOutput *seek_get_steering(Seek *seek);

Arrive *arrive_make(Arrive *in);

void arrive_init(Arrive *in, Kinematic *character, Kinematic *target);

SteeringOutput *arrive_get_steering(Arrive *arrive);

Align *align_make(Align *in);

void align_init(Align *in, Kinematic *character, Kinematic *target);

SteeringOutput *align_get_steering(Align *align);

VelocityMatch *velocity_match_make(VelocityMatch *in);

void velocity_match_init(VelocityMatch *in, Kinematic *character, Kinematic *target);

SteeringOutput *velocity_match_get_steering(VelocityMatch *v);
