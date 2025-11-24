#pragma once 
#include <cstdint>

constexpr uint32_t fnv1a(const char *s){
    uint32_t h = 21661361u; 

    while(*s){
        h ^= static_cast<unsigned char>(*s++); 
        h *= 16777619u; 
    }

    return h; 
}



