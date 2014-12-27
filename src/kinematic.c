#include "kinematic.h"
#include <stdlib.h>
#include <math.h>

KinematicSeek* kmseek_make(KinematicSeek *k) {
    if(!k)
        k = (KinematicSeek*)calloc(1, sizeof(KinematicSeek));

    k->max_speed = 1.0f;
    return k;
}

KinematicArrive* kmarrive_make(KinematicArrive *k) {
    if(!k)
        k = (KinematicArrive*)calloc(1, sizeof(KinematicArrive));

    k->max_speed = 1.0f;
    k->radius = 1.0f;
    k->time_to_target = 0.25f;
    return k;
}

KinematicWander* kmwander_make(KinematicWander *k) {
    if(!k)
        k = (KinematicWander*)calloc(1, sizeof(KinematicWander));

    k->max_speed = 1.0f;
    k->max_rotation = 0.8f;
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
    k->max_accel = 1.0f;
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

void km_update(Kinematic *input, SteeringOutput *steering, const float max_speed, const float time) {
    vec3 new_pos = {}, new_vel = {};
    vec3_mul_scalar(&(input->velocity), time, &new_pos);
    vec3_add(&(input->position), &new_pos, &(input->position));

    input->orientation += input->rotation * time;

    vec3_mul_scalar(&(steering->linear), time, &new_vel);
    vec3_add(&(input->velocity), &new_vel, &(input->velocity));

    input->rotation += steering->angular * time;

    if(vec3_length(&(input->velocity)) > max_speed) {
        vec3_normalize(&(input->velocity), &(input->velocity));
        vec3_mul_scalar(&(input->velocity), max_speed, &(input->velocity));
    }

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

KinematicSteeringOutput* kmarrive_get_steering(KinematicArrive* k) {
    KinematicSteeringOutput *steering = kso_make(NULL);

    // Direction to Target (aka velocity)
    vec3_sub(&(k->target.position), &(k->character.position), &(steering->velocity));

    // Check if we're within radius
    if(vec3_length(&(steering->velocity)) < k->radius) {
        // We're there, no steering to return
        free(steering);
        return NULL;
    }

    // Move to target in time to target seconds
    if(k->time_to_target != 0.0f)
        vec3_div_scalar(&(steering->velocity), k->time_to_target, &(steering->velocity));

    // If we're moving to fast, clip our speed
    if(vec3_length(&(steering->velocity)) > k->max_speed) {
        vec3_normalize(&(steering->velocity), &(steering->velocity));
        vec3_mul_scalar(&(steering->velocity), k->max_speed, &(steering->velocity));
    }

    // face the correct direction we will be moving
    k->character.orientation = get_new_orientation(&(k->character), k->character.orientation, &(steering->velocity));

    steering->rotation = 0.0f;
    return steering;
}

KinematicSteeringOutput* kmwander_get_steering(KinematicWander* k) {
    KinematicSteeringOutput *steering = kso_make(NULL);

    // convert orentation to a vector
    vec3 orientation;
    orientation.x = sinf(k->character.orientation);
    orientation.y = cosf(k->character.orientation);
    orientation.z = 1.0f;

    vec3_mul_scalar(&orientation, k->max_speed, &(steering->velocity));

    /* random value between -1,1 */
    float rand1 = rand() / (float)RAND_MAX;
    float rand2 = rand() / (float)RAND_MAX;
    float rand = rand1 - rand2;

    // Chage the rotation randomly
    steering->rotation = rand * k->max_rotation;

    return steering;
}
