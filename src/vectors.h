#ifndef GAMEPAD_VECTORS_H
#define GAMEPAD_VECTORS_H

#include <Arduino.h>

class vec2{
public:
    float x, y;

    vec2(float x_ = 0, float y_ = 0);

    vec2 operator* (float scale);
    vec2 operator*= (float scale);
    vec2 operator* (vec2 vec);
    vec2 operator*= (vec2 vec);
    vec2 operator+ (vec2 vec);
    vec2 operator+= (vec2 vec);
    vec2 operator- (vec2 vec);
    vec2 operator-= (vec2 vec);

    float mod();
    vec2 norm();
    vec2 fast_norm();

    float dot(vec2 vec);
    float cross_2d(vec2 vec);
};

class vec3{
public:
    float x, y, z;

    vec3(float x_ = 0, float y_ = 0, float z_ = 0);
    vec3(vec2 &vec);

    vec3 operator* (float scale);
    vec3 operator*= (float scale);
    vec3 operator* (vec3 vec);
    vec3 operator*= (vec3 vec);
    vec3 operator+ (vec3 vec);
    vec3 operator+= (vec3 vec);
    vec3 operator- (vec3 vec);
    vec3 operator-= (vec3 vec);

    float mod();
    vec3 norm();
    vec3 fast_norm();

    float dot(vec3 vec);
    vec3 cross(vec3 vec);
};

#endif