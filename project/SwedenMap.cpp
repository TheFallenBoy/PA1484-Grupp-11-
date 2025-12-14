#include "SwedenMap.h"
#include <Arduino.h>
#include "WeatherService.h"


LV_IMG_DECLARE(img_swe); 
extern WeatherService ws; 

// UI Variables
static lv_obj_t* map_img;
static lv_obj_t* time_label;
static lv_obj_t* loading_label;

// 2. List of regions. 
std::vector<Region> regions = {
    // Name,           Lat,   Lon,  X, Y, Obj,  Empty vector
    // --- 'Götaland' ---
    {"Skåne",         55.80, 13.50, 166, 1104, NULL, {}},
    {"Blekinge",      56.20, 15.20, 245, 1067, NULL, {}},
    {"Halland",       56.90, 12.80, 124, 1017, NULL, {}},
    {"Småland",       57.00, 14.50, 232, 1006, NULL, {}},
    {"Öland",         56.80, 16.60, 305, 1037, NULL, {}},
    {"Gotland",       57.40, 18.50, 383, 977, NULL, {}},
    {"Östergötland",  58.40, 15.60, 265, 904, NULL, {}},
    {"Västergötland", 58.00, 13.50, 156, 947, NULL, {}},
    {"Bohuslän",      58.50, 11.50, 87, 917, NULL, {}},
    {"Dalsland",      58.80, 12.20, 106, 869, NULL, {}},

    // --- 'Svealand' ---
    {"Södermanland",  59.00, 16.80, 308, 850, NULL, {}},
    {"Uppland",       59.90, 17.80, 356, 784, NULL, {}},
    {"Västmanland",   59.60, 16.20, 264, 795, NULL, {}},
    {"Närke",         59.20, 15.10, 224, 843, NULL, {}},
    {"Värmland",      59.60, 13.20, 150, 791, NULL, {}},
    {"Dalarna",       60.90, 14.20, 204, 711, NULL, {}},

    // --- 'Norrland' ---
    {"Gästrikland",   60.60, 16.70, 307, 725, NULL, {}},
    {"Hälsingland",   61.50, 16.20, 273, 642, NULL, {}},
    {"Härjedalen",    62.30, 13.50, 161, 588, NULL, {}},
    {"Medelpad",      62.50, 17.00, 285, 567, NULL, {}},
    {"Jämtland",      63.20, 14.50, 203, 513, NULL, {}},
    {"Ångermanland",  63.50, 17.50, 342, 506, NULL, {}},
    {"Västerbotten",  64.80, 18.00, 420, 410, NULL, {}},
    {"Norrbotten",    66.00, 22.00, 512, 264, NULL, {}},
    {"Lappland",      66.50, 18.00, 334, 267, NULL, {}}

};
// Help function for colour
lv_color_t get_temp_color(float temp) {
    if (temp < -20) return lv_palette_main(LV_PALETTE_INDIGO);
    if (temp < -10)   return lv_palette_main(LV_PALETTE_BLUE);
    if (temp < 0)  return lv_palette_main(LV_PALETTE_TEAL); 
    if (temp < 10)  return lv_palette_main(LV_PALETTE_YELLOW);
    if (temp < 20)  return lv_palette_main(LV_PALETTE_ORANGE);
    return lv_palette_main(LV_PALETTE_RED);
}

/** Help function for getting dot positions
static void map_click_handler(lv_event_t * e) {
    lv_point_t p;
    lv_indev_get_point(lv_indev_get_act(), &p); // Hämta var fingret tryckte på skärmen
    
    // Hämta kartans position för att räkna ut relativ position
    lv_obj_t* img = lv_event_get_target(e);
    lv_area_t coords;
    lv_obj_get_coords(img, &coords);

    // Matte: Var klickade jag minus var kartan börjar
    int relative_x = p.x - coords.x1;
    int relative_y = p.y - coords.y1;

    Serial.printf("Du klickade på -> X: %d, Y: %d\n", relative_x, relative_y);
}
**/

// Refresh Button Map-tile
static void refresh_btn_handler(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0); 
    
    
    lv_label_set_text(label, "Vänta...");
    
    lv_timer_handler(); 
    
    // Get the data
    update_map_data_from_api();
    
    lv_label_set_text(label, "Update");
}

// Creating row in legend
void create_legend_row(lv_obj_t* parent, lv_color_t color, const char* text) {
    
    // Background
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, 120, 25); 
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(row, 0, 0);       
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_pad_all(row, 0, 0);            

    // Squares
    lv_obj_t* square = lv_obj_create(row);
    lv_obj_set_size(square, 15, 15); 
    lv_obj_set_style_radius(square, 2, 0); 
    lv_obj_set_style_bg_color(square, color, 0);
    lv_obj_set_style_border_width(square, 0, 0);
    lv_obj_align(square, LV_ALIGN_LEFT_MID, 0, 0); 

    // Text
    lv_obj_t* label = lv_label_create(row);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0); 
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 25, 0); 
}

void setup_map_tile(lv_obj_t* parent) {
    // 1. Create Map
    map_img = lv_img_create(parent);
    lv_img_set_src(map_img, &img_swe);
    lv_obj_align(map_img, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(map_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(map_img, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Create dots
    for (auto &reg : regions) {
        reg.obj = lv_obj_create(parent); 
        lv_obj_set_size(reg.obj, 20, 20);
        lv_obj_set_style_radius(reg.obj, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(reg.obj, lv_color_hex(0x888888), 0); 
        lv_obj_set_pos(reg.obj, reg.x, reg.y);
        lv_obj_clear_flag(reg.obj, LV_OBJ_FLAG_SCROLLABLE);
    }

    // 4. Loading label
    loading_label = lv_label_create(parent);
    lv_label_set_text(loading_label, "Hämtar kart-data...");
    lv_obj_center(loading_label);
    lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);

    // The button 
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 20, 20); 
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING); 
    
    lv_obj_add_event_cb(btn, refresh_btn_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, "Get Data");
    lv_obj_center(label);

    // Legend & Time 
    
    lv_obj_t* legend_cont = lv_obj_create(parent);
    lv_obj_set_size(legend_cont, 130, 220); 
    lv_obj_align(legend_cont, LV_ALIGN_RIGHT_MID, -5, 0); 
    lv_obj_add_flag(legend_cont, LV_OBJ_FLAG_FLOATING); 

    // Styling
    lv_obj_set_style_bg_color(legend_cont, lv_color_hex(0x000000), 0); 
    lv_obj_set_style_bg_opa(legend_cont, LV_OPA_40, 0); 
    lv_obj_set_style_border_width(legend_cont, 0, 0);
    
    // Layout
    lv_obj_set_flex_flow(legend_cont, LV_FLEX_FLOW_COLUMN); 
    lv_obj_set_flex_align(legend_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    lv_obj_set_style_pad_row(legend_cont, 0, 0);
    lv_obj_set_style_pad_column(legend_cont, 0, 0);

    lv_obj_clear_flag(legend_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(legend_cont, 10, 0); 

    // Add Colours
    create_legend_row(legend_cont, lv_palette_main(LV_PALETTE_INDIGO), "< -20° C");
    create_legend_row(legend_cont, lv_palette_main(LV_PALETTE_BLUE),   "< -10° C");
    create_legend_row(legend_cont, lv_palette_main(LV_PALETTE_TEAL),   "< 0° C");
    create_legend_row(legend_cont, lv_palette_main(LV_PALETTE_YELLOW), "< 10° C");
    create_legend_row(legend_cont, lv_palette_main(LV_PALETTE_ORANGE), "< 20° C");
    create_legend_row(legend_cont, lv_palette_main(LV_PALETTE_RED),    "> 20° C");

    // Line
    lv_obj_t* spacer1 = lv_obj_create(legend_cont);
    lv_obj_set_size(spacer1, 100, 5); 
    lv_obj_set_style_bg_opa(spacer1, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(spacer1, 0, 0);
    lv_obj_clear_flag(spacer1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* line = lv_obj_create(legend_cont);
    lv_obj_set_size(line, 100, 2); 
    lv_obj_set_style_bg_color(line, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_50, 0); 
    lv_obj_set_style_border_width(line, 0, 0);

    // Forecast Time
    time_label = lv_label_create(legend_cont); 
    lv_label_set_text(time_label, "Forecast: Now");
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_set_style_pad_top(time_label, 10, 0); 
}

void update_map_data_from_api() {
    lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    lv_timer_handler(); 


    int counter = 0;
    for (auto &reg : regions) {

        Serial.printf("Get data for %s...\n", reg.name);
        reg.temps = ws.GetHourlyForecast(reg.lat, reg.lon);
        
        // 2 Sec delay for stability
        delay(2000); 
        lv_timer_handler();
        counter++;
    }
    
    lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Map-data updated!");
}

// Map-loop for 24h looping forecast
void animate_map_loop() {
    static unsigned long last_update = 0;
    static int hour_index = 0;

    if (millis() - last_update > 1500) {
        last_update = millis();

        // text update
        lv_label_set_text_fmt(time_label, "Forecast: +%d h", hour_index);

        // Set Colours
        for (auto &reg : regions) {
            // Check for data
            if (!reg.temps.empty() && reg.temps.size() > hour_index) {
                
                // Get temp
                float t = reg.temps[hour_index];
                Serial.printf("Region %s: Temp %.1f, Index %d\n", "RegionX", t, hour_index);
                
                // Get Colour
                lv_color_t farg = get_temp_color(t);
                
                // Colour dot
                lv_obj_set_style_bg_color(reg.obj, farg, 0);
            } else {
                // Debug: Om vi hamnar här är datan tom!
                //Serial.println("Ingen temp-data hittad för region!");
            }
        }

        // Time step
        hour_index++;
        if (hour_index >= 24) hour_index = 0;
    }
}