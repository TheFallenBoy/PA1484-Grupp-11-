#ifndef SWEDEN_MAP_H
#define SWEDEN_MAP_H

#include <lvgl.h>
#include <vector>
#include "WeatherService.h" // Så vi kan prata med din service

// Struct för varje landskap/region
struct Region {
    const char* name;
    float lat;
    float lon;
    int x; // Pixel X på skärmen
    int y; // Pixel Y på skärmen
    lv_obj_t* obj; // Den lilla pricken
    std::vector<float> temps; // 24h prognos
};

// Funktioner som anropas från main
void setup_map_tile(lv_obj_t* parent);
void update_map_data_from_api(); // Denna kommer ta tid!
void animate_map_loop(); // Körs i loop()

#endif