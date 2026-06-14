#ifndef DD_VECTORS_H
#define DD_VECTORS_H

#include <Arduino.h>

static float fast_inv_sqrt(float n){
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

class vec2{
public:
    float x, y;

    inline vec2(float x_ = 0.0f, float y_ = 0.0f){  x = x_; y = y_; }

    inline vec2 operator* (float scale){        return vec2(x * scale, y * scale);      }
    inline vec2 operator*= (float scale){       return vec2(x *= scale, y *= scale);    }
    inline vec2 operator* (vec2 vec){           return vec2(x * vec.x, y * vec.y);      }
    inline vec2 operator*= (vec2 vec){          return vec2(x *= vec.x, y *= vec.y);    }
    inline vec2 operator+ (vec2 vec){           return vec2(x + vec.x, y + vec.y);      }
    inline vec2 operator+= (vec2 vec){          return vec2(x += vec.x, y += vec.y);    }
    inline vec2 operator- (vec2 vec){           return vec2(x - vec.x, y - vec.y);      }
    inline vec2 operator-= (vec2 vec){          return vec2(x -= vec.x, y -= vec.y);    }

    inline float mod(){
        return sqrt(x*x + y*y);
    }
    inline vec2 norm(){
        float module = mod();
        return vec2(x / module, y / module);
    }
    inline vec2 fast_norm(){
        float inv_mod = fast_inv_sqrt(x*x + y*y);
        return vec2(x * inv_mod, y * inv_mod);
    }

    inline float dot(vec2 vec){         return x*vec.x + y*vec.y;   }
    inline float cross_2d(vec2 vec){    return x*vec.y - y*vec.x;   }
};

class vec3{
public:
    float x, y, z;

    inline vec3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f){ x = x_; y = y_; z = z_;         }
    inline vec3(vec2 &vec){                                         x = vec.x; y = vec.y; z = 0.0f; }

    inline vec3 operator* (float scale){    return vec3(x * scale, y * scale, z * scale);       }
    inline vec3 operator*= (float scale){   return vec3(x *= scale, y *= scale, z *= scale);    }
    inline vec3 operator* (vec3 vec){       return vec3(x * vec.x, y * vec.y, z * vec.z);       }
    inline vec3 operator*= (vec3 vec){      return vec3(x *= vec.x, y *= vec.y, z *= vec.z);    }
    inline vec3 operator+ (vec3 vec){       return vec3(x + vec.x, y + vec.y, z + vec.z);       }
    inline vec3 operator+= (vec3 vec){      return vec3(x += vec.x, y += vec.y, z += vec.z);    }
    inline vec3 operator- (vec3 vec){       return vec3(x - vec.x, y - vec.y, z - vec.z);       }
    inline vec3 operator-= (vec3 vec){      return vec3(x -= vec.x, y -= vec.y, z -= vec.z);    }

    inline float mod(){
        return sqrt(x*x + y*y + z*z);
    }
    inline vec3 norm(){
        float module = mod();
        return vec3(x / module, y / module, z / module);
    }
    inline vec3 fast_norm(){
        float inv_mod = fast_inv_sqrt(x*x + y*y + z*z);
        return vec3(x * inv_mod, y * inv_mod, z * inv_mod);
    }

    inline float dot(vec3 vec){     return x*vec.x + y*vec.y + z*vec.z;     }
    inline vec3 cross(vec3 vec){
        return vec3(
            y*vec.z - z*vec.y,
            z*vec.x - x*vec.z,
            x*vec.y - y*vec.x
        );
    }
};

#endif