#ifndef SWEDEN_MAP_H
#define SWEDEN_MAP_H

#include <lvgl.h>
#include <vector>
#include "WeatherService.h" 

// Struct for every region
struct Region {
    const char* name;
    float lat;
    float lon;
    int x; // Pixel X on screen
    int y; // Pixel Y on screen
    lv_obj_t* obj; // Dot
    std::vector<float> temps; // 24h forecast
};

// Function calls from main
void setup_map_tile(lv_obj_t* parent);
void update_map_data_from_api(); 
void animate_map_loop(); 

#endif