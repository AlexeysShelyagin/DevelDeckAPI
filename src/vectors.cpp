#include "vectors.h"

float fast_inv_sqrt(float n){
    int i;
    float x2, y;
    
    x2 = n * 0.5F;
    y = n;
    i = * (int *) &y;
    i = 0x5f3759df - (i >> 1);
    y = * (float *) &i;
    y = y * (1.5F - (x2 * y * y));

    return y;
}

vec2::vec2(float x_, float y_){
    x = x_; y = y_;
}

vec2 vec2::operator* (float scale){
    return vec2(x * scale, y * scale);
}

vec2 vec2::operator*= (float scale){
    return vec2(x *= scale, y *= scale);
}

vec2 vec2::operator* (vec2 vec){
    return vec2(x * vec.x, y * vec.y);
}

vec2 vec2::operator*= (vec2 vec){
    return vec2(x *= vec.x, y *= vec.y);
}

vec2 vec2::operator+ (vec2 vec){
    return vec2(x + vec.x, y + vec.y);
}

vec2 vec2::operator+= (vec2 vec){
    return vec2(x += vec.x, y += vec.y);
}

vec2 vec2::operator- (vec2 vec){
    return vec2(x - vec.x, y - vec.y);
}

vec2 vec2::operator-= (vec2 vec){
    return vec2(x -= vec.x, y -= vec.y);
}

float vec2::mod(){
    return sqrt(x*x + y*y);
}

vec2 vec2::norm(){
    float module = mod();
    return vec2(x / module, y / module);
}

vec2 vec2::fast_norm(){
    float inv_mod = fast_inv_sqrt(x*x + y*y);
    return vec2(x * inv_mod, y * inv_mod);
}

float vec2::dot(vec2 vec){
    return x*vec.x + y*vec.y;
}

float vec2::cross_2d(vec2 vec){
    return x*vec.y - y*vec.x;
}



vec3::vec3(float x_, float y_, float z_){
    x = x_; y = y_; z = z_;
}

vec3::vec3(vec2 &vec){
    x = vec.x; y = vec.y; z = 0;
}

vec3 vec3::operator* (float scale){
    return vec3(x * scale, y * scale, z * scale);
}

vec3 vec3::operator*= (float scale){
    return vec3(x *= scale, y *= scale, z *= scale);
}

vec3 vec3::operator* (vec3 vec){
    return vec3(x * vec.x, y * vec.y, z * vec.z);
}

vec3 vec3::operator*= (vec3 vec){
    return vec3(x *= vec.x, y *= vec.y, z *= vec.z);
}

vec3 vec3::operator+ (vec3 vec){
    return vec3(x + vec.x, y + vec.y, z + vec.z);
}

vec3 vec3::operator+= (vec3 vec){
    return vec3(x += vec.x, y += vec.y, z += vec.z);
}

vec3 vec3::operator- (vec3 vec){
    return vec3(x - vec.x, y - vec.y, z - vec.z);
}

vec3 vec3::operator-= (vec3 vec){
    return vec3(x -= vec.x, y -= vec.y, z -= vec.z);
}

float vec3::mod(){
    return sqrt(x*x + y*y + z*z);
}

vec3 vec3::norm(){
    float module = mod();
    return vec3(x / module, y / module, z / module);
}

vec3 vec3::fast_norm(){
    float inv_mod = fast_inv_sqrt(x*x + y*y + z*z);
    return vec3(x * inv_mod, y * inv_mod, z * inv_mod);
}

float vec3::dot(vec3 vec){
    return x*vec.x + y*vec.y + z*vec.z;
}

vec3 vec3::cross(vec3 vec){
    return vec3(
        y*vec.z - vec.z*y,
        z*vec.x - vec.x*z,
        x*vec.y - vec.y*x
    );
}