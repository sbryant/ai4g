#include "kinematic.h"
#include <stdlib.h>
#include <math.h>

KinematicSeek* kmseek_make(KinematicSeek *k) {
    if(!k)
        k = (KinematicSeek*)calloc(1, sizeof(KinematicSeek));

    k->max_speed = 1.0f;
    return k;
}
Static* static_make(Static *s) {
    if(!s)
        s = (Static*)calloc(1, sizeof(Static));

    return s;
}
Kinematic* km_make(Kinematic *k) {
    if(!k)
        k = (Kinematic*)calloc(1, sizeof(Kinematic));

    return k;
}

SteeringOutput* so_make(SteeringOutput *s) {
    if(!s)
        s = (SteeringOutput*)calloc(1, sizeof(SteeringOutput));

    return s;
}

KinematicSteeringOutput* kso_make(KinematicSteeringOutput *s) {
    if(!s)
        s = (KinematicSteeringOutput*)calloc(1, sizeof(KinematicSteeringOutput));

    return s;
}

float get_new_orientation(Static *s, float current, vec3 *velocity) {
    if(vec3_length(velocity) > 0.0f) {
        return atan2f(-s->position.x, s->position.y);
    }
    return current;
}

void km_update(Kinematic *input, SteeringOutput *steering, float time) {
    vec3 new_pos, new_vel;
    vec3_mul_scalar(&(input->velocity), time, &new_pos);
    vec3_add(&(input->position), &new_pos, &(input->position));

    input->orientation += input->rotation * time;

    vec3_mul_scalar(&(steering->linear), time, &new_vel);
    vec3_add(&(input->velocity), &new_vel, &(input->velocity));

    input->rotation += steering->angular * time;

};

KinematicSteeringOutput* kmseek_get_steering(KinematicSeek* k) {
    KinematicSteeringOutput *steering = kso_make(NULL);

    /* Direction to Target */
    vec3_sub(&(k->target.position), &(k->character.position), &(steering->velocity));

    /* full speed along this direction */
    vec3_normalize(&(steering->velocity), &(steering->velocity));
    vec3_mul_scalar(&(steering->velocity), k->max_speed, &(steering->velocity));

    /* face the correct direction we will be moving*/
    k->character.orientation = get_new_orientation(&(k->character), k->character.orientation, &(steering->velocity));

    steering->rotation = 0.0f;
    return steering;
}