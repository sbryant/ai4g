#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "dynamic.h"

Seek *seek_make(Seek *in) {
    Seek *out = in;
    if(!out)
        out = (Seek *)calloc(1, sizeof(Seek));
    return out;
}

SteeringOutput *seek_get_steering(Seek *seek) {
    SteeringOutput *steering = so_make(NULL);

    // Get the direction to the target
    vec3_sub(&(seek->target->position), &(seek->character->position), &(steering->linear));

    // Maximum acceleration along the direction
    vec3_normalize(&(steering->linear), &(steering->linear));
    vec3_mul_scalar(&(steering->linear), seek->max_acceleration, &(steering->linear));

    steering->angular = 0.0f;

    return steering;
}

Arrive *arrive_make(Arrive *in) {
    Arrive *out = in;
    if(!out)
        out = (Arrive *)calloc(1, sizeof(Arrive));
    return out;
}

void arrive_init(Arrive *in, Kinematic *character, Kinematic *target) {
    Arrive *arrive = in;
    if(!arrive)
        arrive = arrive_make(NULL);
    else
        bzero(in, sizeof(Arrive));
    arrive->time_to_target = 0.1f;
    arrive->character = character;
    arrive->target = target;
}

SteeringOutput *arrive_get_steering(Arrive *arrive) {
    SteeringOutput *steering = NULL;

    vec3 direction;
    // Get the direction to the target
    vec3_sub(&(arrive->target->position), &(arrive->character->position), &direction);

    float distance = vec3_length(&direction);

    if(distance < arrive->target_radius) {
        return NULL;
    }

    steering = so_make(NULL);

    float target_speed;

    if(distance > arrive->slow_radius)
        target_speed = arrive->max_speed;
    else
        target_speed = arrive->max_speed * distance / arrive->slow_radius;

    vec3 target_velocity;
    vec3_set_vec3(&target_velocity, &direction);
    vec3_normalize(&target_velocity, &target_velocity);
    vec3_mul_scalar(&target_velocity, target_speed, &target_velocity);

    vec3_sub(&target_velocity, &(arrive->character->velocity), &(steering->linear));
    vec3_div_scalar(&(steering->linear), arrive->time_to_target, &(steering->linear));

    if(vec3_length(&(steering->linear)) > arrive->max_acceleration) {
        vec3_normalize(&(steering->linear), &(steering->linear));
        vec3_mul_scalar(&(steering->linear), arrive->max_acceleration, &(steering->linear));
    }

    steering->angular = 0.0f;
    return steering;
}

Align *align_make(Align *in) {
    Align *out = in;
    if(!out)
        out = (Align *)calloc(1, sizeof(Align));
    return out;
}

void align_init(Align *in, Kinematic *character, Kinematic *target) {
    Align *align = in;
    if(!align)
        align = align_make(NULL);
    else
        bzero(in, sizeof(Align));
    align->time_to_target = 0.1f;
    align->character = character;
    align->target = target;
}

SteeringOutput *align_get_steering(Align *align) {
    SteeringOutput *steering = NULL;

    float rotation = align->target->orientation - align->character->orientation;

    rotation = fmodf(rotation + M_PI, M_2_PI) - M_PI;

    float rotation_size = fabs(rotation);

    if(rotation_size < align->target_radius) {
        return NULL;
    }

    steering = so_make(NULL);

    float target_rotation;
    if(rotation_size > align->slow_radius)
        target_rotation = align->max_rotation;
    else
        target_rotation = (align->max_rotation * rotation_size) / align->slow_radius;

    target_rotation *= rotation / rotation_size;

    steering->angular = target_rotation - align->character->rotation;

    if(align->time_to_target > 0.0f)
        steering->angular /= align->time_to_target;

    float angular_acceleration = fabs(steering->angular);
    if(angular_acceleration > align->max_angular_acceleration) {
        steering->angular /= angular_acceleration;
        steering->angular *= align->max_angular_acceleration;
    }

    return steering;
}

VelocityMatch *velocity_match_make(VelocityMatch *in) {
    VelocityMatch *out = in;
    if(!out)
        out = calloc(1, sizeof(VelocityMatch));
    return out;
}

void velocity_match_init(VelocityMatch *in, Kinematic *character, Kinematic *target) {
    VelocityMatch *velocity_match = in;
    if(!velocity_match)
        velocity_match = velocity_match_make(NULL);
    else
        bzero(in, sizeof(VelocityMatch));
    velocity_match->time_to_target = 0.1f;
    velocity_match->character = character;
    velocity_match->target = target;
}

SteeringOutput *velocity_match_get_steering(VelocityMatch *v) {
    SteeringOutput *steering = so_make(NULL);

    vec3_sub(&(v->target->velocity), &(v->character->velocity), &(steering->linear));
    if(v->time_to_target > 0.0f)
        vec3_div_scalar(&(steering->linear), v->time_to_target, &(steering->linear));

    if(vec3_length(&(steering->linear)) > v->max_acceleration) {
        vec3_normalize(&(steering->linear), &(steering->linear));
        vec3_mul_scalar(&(steering->linear), v->max_acceleration, &(steering->linear));
    }

    steering->angular = 0.0f;
    return steering;
}
