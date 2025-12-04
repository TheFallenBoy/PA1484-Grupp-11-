#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <lvgl.h>
#include "WeatherService.h"
#include <preferences.h>
//#include "secrets.h"

// Wi-Fi credentials (Delete these before commiting to GitHub)
static const char* WIFI_SSID     = "BTH_Guest";
static const char* WIFI_PASSWORD = "paprika45svart";

LilyGo_Class amoled;

static lv_obj_t* tileview;
static lv_obj_t* t0;
static lv_obj_t* t1;
static lv_obj_t* t2;
static lv_obj_t* t3;
static lv_obj_t* t0_label;
static lv_obj_t* t1_label;
static lv_obj_t* t2_label;
static lv_obj_t* t3_label;
static bool t2_dark = false;  // start tile #2 in light mode
static lv_obj_t* dayLabel[7];
static lv_obj_t* iconImage[7];
static lv_obj_t* paramLabel[7];
static lv_obj_t* slider;
static lv_obj_t* slider_label; // kanske ska lägga till en slider?
static lv_chart_cursor_t* cursor;
Preferences saved_settings;
lv_obj_t * city_dd; 
lv_obj_t * parameter_dd;
int savedCityIndex;
int savedParamIndex;


static bool needUpdateHistory = false;
static lv_obj_t* chart;
static lv_chart_series_t* ser1;
std::vector<float> historyDataPoint;
static WeatherService ws;





// Function: Tile #2 Color change
static void apply_tile_colors(lv_obj_t* tile, lv_obj_t* label, bool dark)
{
  // Background
  lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(tile, dark ? lv_color_hex(0x34429E) : lv_color_hex(0xB1DFF2), 0);

  // Text
  lv_obj_set_style_text_color(label, dark ? lv_color_white() : lv_color_black(), 0);
}

static void on_tile2_clicked(lv_event_t* e)
{
  LV_UNUSED(e);
  t2_dark = !t2_dark;
  apply_tile_colors(t2, t2_label, t2_dark);
}

// 1. DECLARE IMAGES
// These names must match the "C array name" from the converter (usually the filename)
LV_IMG_DECLARE(img_sun);
LV_IMG_DECLARE(img_cloud);
LV_IMG_DECLARE(img_rain);
LV_IMG_DECLARE(img_snow);

static void parameter_drop_down_event_handler(lv_event_t *e)
{
  lv_obj_t * dropdownParameter = lv_event_get_target(e);
  
  int selected_index_parameter = lv_dropdown_get_selected(dropdownParameter);
  Serial.print("index valt:");
  Serial.println(selected_index_parameter);
 

  ws.SetParameterID(selected_index_parameter);
  needUpdateHistory = true;
  Serial.println("Val ändrat, väntar på att du ska sluta klicka...");
  

}
static void city_drop_down_event_handler(lv_event_t *e)
{

  lv_obj_t * dropdownCity = lv_event_get_target(e);
  int selected_index_city = lv_dropdown_get_selected(dropdownCity);
  Serial.print("city valt:");
  Serial.println(selected_index_city);
  ws.SetStationID(selected_index_city);
  needUpdateHistory = true;
 
  Serial.println("Val ändrat, väntar på att du ska sluta klicka...");
}

// 2. MAPPING FUNCTION
// Maps SMHI symbol code (1-27) to your 4 icons
const lv_img_dsc_t* get_icon_by_id(int id) {
    // SMHI Wsymb2 codes:
    // 1-3: Clear/Variable (Sun)
    // 4-7: Cloudy/Fog (Cloud)
    // 8-11, 18-21: Rain/Thunder (Rain)
    // 12-17, 22-27: Snow/Sleet (Snow)
    
    if (id >= 1 && id <= 3) return &img_sun;
    if (id >= 4 && id <= 7) return &img_cloud;
    if ((id >= 8 && id <= 11) || (id >= 18 && id <= 21)) return &img_rain;
    if ((id >= 12 && id <= 17) || (id >= 22 && id <= 27)) return &img_snow;
    
    // Default fallback
    return &img_cloud; 
}



static void slider_event_cb(lv_event_t* e)
{
  //needUpdateHistory = true;
    lv_obj_t* slider_obj = lv_event_get_target(e);
    
    // Säkerhetskoll: Om vi inte har någon data, gör inget
    if (historyDataPoint.empty()) return;

    // 1. Hämta index från slidern (0 = äldsta, Max = nyaste)
    int index = (int)lv_slider_get_value(slider_obj);

    // Säkerställ att vi inte går utanför vektorn (kraschrisk annars!)
    if (index < 0) index = 0;
    if (index >= historyDataPoint.size()) index = historyDataPoint.size() - 1;

    // 2. Hämta värdet för den punkten
    float value = historyDataPoint[index];

    lv_label_set_text_fmt(slider_label, "Value: %.1f %s", value, ws.unit);
    // 4. Flytta markören i grafen till rätt punkt
    // Parametrar: Grafen, Markören, Serien (ser1), Indexet
    lv_chart_set_cursor_point(chart, cursor, ser1, index);

}
// Function: Creates UI
static void create_ui()
{ 
  // Fullscreen Tileview
  tileview = lv_tileview_create(lv_scr_act());
  lv_obj_set_size(tileview, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
   


  // Add tile positions in a grid
  t0 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR); //Boot tile
  t1 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR); //forecast tile
  t2 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR); //history tile
  t3 = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_HOR); //settings tile
  
  // STARTING SCREEN! 
  {
    t0_label = lv_label_create(t0);
    lv_label_set_text(t0_label, "Grupp 11 V.3");
    lv_obj_set_style_text_font(t0_label, &lv_font_montserrat_28, 0);
    lv_obj_center(t0_label);
    apply_tile_colors(t0, t0_label, /*dark=*/false);
    
  }

  // Tile #1 WEATHER FORECAST TILE
  {

   std::vector<ForecastDataPoint> data = ws.GetSevenDayForecast();
    // City title
    t1_label = lv_label_create(t1);
    lv_label_set_text(t1_label, ws.city.c_str());
    lv_obj_set_style_text_font(t1_label, &lv_font_montserrat_40, 0);
    lv_obj_align(t1_label, LV_ALIGN_TOP_LEFT, 6, 10);
    apply_tile_colors(t1, t1_label, /*dark=*/false);
    
    // Parameter title
    lv_obj_t* t1_sub = lv_label_create(t1);
    lv_label_set_text(t1_sub, "Temperature");
    lv_obj_set_style_text_font(t1_sub, &lv_font_montserrat_32, 0);
    lv_obj_align_to(t1_sub, t1_label, LV_ALIGN_OUT_BOTTOM_LEFT,6, 6);

    // GRID DEFINTIONS
    static lv_coord_t col_dsc[] = {LV_GRID_FR(3), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_t* grid = lv_obj_create(t1);
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

    // Full width and height
    lv_obj_set_size(grid, lv_pct(100), lv_pct(100));

    // Align inside tile
    lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, 120); 

    // Disable scrolling
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(grid, LV_SCROLLBAR_MODE_OFF);

    // grid background
    lv_obj_set_style_bg_color(grid, lv_color_hex(0xB1DFF2), 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_border_width(grid, 0, 0);


    const lv_font_t* font_data = &lv_font_montserrat_18;
    // loop that creates 7 rows of cells, with three columns
    for(int r = 0; r < 7; r++) 
    {
      // Choose font size: bigger for first (today) row
      const lv_font_t* font = (r == 0) ? &lv_font_montserrat_36 : &lv_font_montserrat_26;

      // Day cell
      lv_obj_t* day_obj = lv_obj_create(grid);
      lv_obj_set_grid_cell(day_obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, r, 1);
      lv_obj_clear_flag(day_obj, LV_OBJ_FLAG_SCROLLABLE);
      // visual properties 
      lv_obj_set_style_bg_color(day_obj, lv_color_hex(0xC0E5F4), 0);
      lv_obj_set_style_bg_opa(day_obj, LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(day_obj, 0, 0);
      //text
      dayLabel[r]= lv_label_create(day_obj);
      lv_label_set_text_fmt(dayLabel[r],"%s" , data[r].weekday);
      lv_obj_set_style_text_font(dayLabel[r], font, 0);
      lv_obj_center(dayLabel[r]);

      // Parameter cell
      lv_obj_t* param_obj = lv_obj_create(grid);
      lv_obj_set_grid_cell(param_obj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, r, 1);
      lv_obj_clear_flag(param_obj, LV_OBJ_FLAG_SCROLLABLE);
      // background 
      lv_obj_set_style_bg_color(param_obj, lv_color_hex(0xC0E5F4), 0);
      lv_obj_set_style_bg_opa(param_obj, LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(param_obj, 0, 0);
      //text
      paramLabel[r] = lv_label_create(param_obj);
      lv_label_set_text_fmt(paramLabel[r],"%.1f %s", data[r].temp, ws.unit.c_str());
      lv_obj_set_style_text_font(paramLabel[r], font_data, 0);
      lv_obj_center(paramLabel[r]);

      // Icon cell 
      lv_obj_t* icon_obj = lv_obj_create(grid);
      lv_obj_set_grid_cell(icon_obj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, r, 1);
      lv_obj_clear_flag(icon_obj, LV_OBJ_FLAG_SCROLLABLE);
      // background 
      lv_obj_set_style_bg_color(icon_obj, lv_color_hex(0xC0E5F4), 0);
      lv_obj_set_style_bg_opa(icon_obj, LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(icon_obj, 0, 0);
      
      iconImage[r] = lv_img_create(icon_obj);
      lv_img_set_src(iconImage[r], get_icon_by_id(data[r].iconID));
      lv_obj_center(iconImage[r]);
    }
  }

  
  {
  // Tile #2 - HISTORICAL DATA
    
    // 1. Skapa grafen
    chart = lv_chart_create(t2);
    lv_obj_set_size(chart, 500, 300); // Justera storlek så den passar skärmen
    lv_obj_center(chart);   // Centrera i tilen

    slider_label = lv_label_create(t2);
    
    // Sätt en start-text
    lv_label_set_text(slider_label, "Move the slider...");
    
    // Gör texten stor och tydlig (Vi återanvänder fonten du har i Tile 3)
    lv_obj_set_style_text_font(slider_label, &lv_font_montserrat_28, 0);
    
    // Placera den OVANFÖR (OUT_TOP_MID) grafen med 10px marginal
    lv_obj_align_to(slider_label, chart, LV_ALIGN_OUT_TOP_MID, 0, -20);
    
    // 2. Inställningar för grafen
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); // Linjediagram
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -20, 30); // Start-skala (temp)
    lv_chart_set_point_count(chart, 100); // Hur många punkter som visas (vi ändrar detta dynamiskt sen)
    
    // 3. Lägg till en dataserie (Röd linje)
    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    
    // Styla grafen lite (valfritt)
    lv_obj_set_style_bg_color(chart, lv_color_white(), 0);
    cursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_BLUE), LV_DIR_ALL);


      // --- NYTT: SKAPA SLIDER ---
    slider = lv_slider_create(t2);
    lv_obj_set_width(slider, 480);  
    lv_obj_set_height(slider, 30);

    lv_obj_set_style_pad_all(slider, 8, LV_PART_KNOB); 
    lv_obj_set_style_bg_color(slider, lv_palette_main(LV_PALETTE_BLUE), LV_PART_KNOB);
    lv_obj_align_to(slider, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 20); // Placera under grafen
    
    // Lägg till eventet vi skapade nyss
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  }

  // Tile #3
  {
    t3_label = lv_label_create(t3);
    lv_label_set_text(t3_label, "Settings");
    lv_obj_set_style_text_font(t3_label, &lv_font_montserrat_28, 0);
    lv_obj_center(t3_label);
    apply_tile_colors(t3, t3_label, /*dark=*/false);
    lv_obj_set_flex_flow(t3, LV_FLEX_FLOW_COLUMN);


    // Create container for city row
    lv_obj_t * cont_city = lv_obj_create(t3);
    lv_obj_set_size(cont_city, LV_PCT(100), LV_PCT(30));
    lv_obj_align(cont_city, LV_ALIGN_TOP_MID, 0, LV_PCT(5));
    lv_obj_set_flex_flow(cont_city, LV_FLEX_FLOW_ROW);

    // Create container for parameter row
    lv_obj_t * cont_parameter = lv_obj_create(t3);
    lv_obj_set_size(cont_parameter, LV_PCT(100), LV_PCT(30));
    lv_obj_align(cont_parameter, LV_ALIGN_TOP_MID, 0, LV_PCT(5));
    lv_obj_set_flex_flow(cont_parameter, LV_FLEX_FLOW_ROW);

    // City selection row
    lv_obj_t * city_label = lv_label_create(cont_city);
    city_dd = lv_dropdown_create(cont_city);

    lv_label_set_text(city_label, "City");
    lv_obj_set_style_text_font(city_label, &lv_font_montserrat_28, 0);
    lv_dropdown_set_options(city_dd, "Karlskrona\n" "Stockholm\n" "Gothenburg\n" "Malmo\n" "Kiruna");

    lv_obj_set_width(city_dd, LV_PCT(40));   // Dropdown width set
    lv_obj_add_event_cb(city_dd, city_drop_down_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Parameter selection row
    lv_obj_t * parameter_label = lv_label_create(cont_parameter);
    parameter_dd = lv_dropdown_create(cont_parameter);

    lv_label_set_text(parameter_label, "Parameter");
    lv_obj_set_style_text_font(parameter_label, &lv_font_montserrat_28, 0);
    lv_dropdown_set_options(parameter_dd, "Temperature\n" "Humidity\n" "Wind Speed\n" "Air Pressure"); 

    lv_obj_set_width(parameter_dd, LV_PCT(40));   // Dropdown width set
    lv_obj_add_event_cb(parameter_dd, parameter_drop_down_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    //SAVE BUTTON
    lv_obj_t* save_btn = lv_btn_create(t3);
    lv_obj_set_size(save_btn, 200, 60); // Rejäl storlek
    lv_obj_align(save_btn, LV_ALIGN_BOTTOM_MID, 0, -40); // Längst ner i mitten
    
    // Gör knappen grön så den syns bra
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(0x28a745), 0);

    // Lägg till text på knappen
    lv_obj_t* btn_label = lv_label_create(save_btn);
    lv_label_set_text(btn_label, "Save Settings");
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_28, 0);
    lv_obj_center(btn_label);

    // Koppla ihop med spara-funktionen
    lv_obj_add_event_cb(save_btn, save_button_event_handler, LV_EVENT_CLICKED, NULL);

    lv_dropdown_set_selected(parameter_dd, savedParamIndex);
    lv_dropdown_set_selected(city_dd, savedCityIndex);
  }
}

//mostly AI generated with gemini pro 3, modified by a human
static void save_button_event_handler(lv_event_t* e)
{
  saved_settings.begin("weather-app", false);
  int currentCityIndex = lv_dropdown_get_selected(city_dd);
  int currentParameterIndex = lv_dropdown_get_selected(parameter_dd);

  saved_settings.putInt("cityIdx",currentCityIndex);
  saved_settings.putInt("paramIdx",currentParameterIndex);
  saved_settings.end();  
  lv_obj_t* btn = lv_event_get_target(e);
  lv_obj_t* label = lv_obj_get_child(btn, 0);
  lv_label_set_text(label, "Saved!");
}

void LoadSavedSettings() {
  needUpdateHistory = true;  
  saved_settings.begin("weather-app", true); // true = Read Only (läsläge)
    
    // Hämta sparade värden. "0" är standardvärdet om inget är sparat än.
     savedCityIndex = saved_settings.getInt("cityIdx", 0);
     savedParamIndex = saved_settings.getInt("paramIdx", 0);

    
    saved_settings.end();
    
    // Applicera på WeatherService direkt så vi hämtar rätt data
    ws.SetStationID(savedCityIndex);
    ws.SetParameterID(savedParamIndex);

    
    Serial.printf("Laddade inställningar -> Stad: %d, Param: %d\n", savedCityIndex, savedParamIndex);
}

static void UpdateUI()
{
    std::vector<ForecastDataPoint> forecastData = ws.GetSevenDayForecast(); 
    for (int i = 0; i < 7; i++)
    {
      lv_label_set_text(dayLabel[i], forecastData[i].weekday);
      lv_img_set_src(iconImage[i], get_icon_by_id(forecastData[i].iconID));
      lv_label_set_text_fmt(paramLabel[i], "%.1f %s",forecastData[i].temp, ws.unit);
    }
    lv_label_set_text(t1_label, ws.city.c_str());
    historyDataPoint = ws.GetHistoricalData();
    //AI genererat gemini 3 pro
    // 1. Räkna ut min/max värde i datan för att skala Y-axeln snyggt
    float minVal = 1000;
    float maxVal = -1000;
    for (float val : historyDataPoint) {
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, (int)minVal - 5, (int)maxVal + 5);
    lv_chart_set_point_count(chart, historyDataPoint.size());
    for (float val : historyDataPoint) {
        // LVGL tar int, så vi castar float till int (eller multiplicerar med 10 för precision om du vill trixa)
        lv_chart_set_next_value(chart, ser1, (int)val);
    }

    lv_slider_set_range(slider, 0, historyDataPoint.size() - 1);
    int newestIndex = historyDataPoint.size() - 1;
    lv_slider_set_value(slider, newestIndex, LV_ANIM_ON);

    // Uppdatera även markören och texten manuellt en gång så det matchar
    lv_chart_set_cursor_point(chart, cursor, ser1, newestIndex);
   
}

// Function: Connects to WIFI
static void connect_wifi()
{
  Serial.printf("Connecting to WiFi SSID: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000) {
    delay(250);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected.");
  } else {
    Serial.println("WiFi could not connect (timeout).");
  }
}

// Must have function: Setup is run once on startup
void setup()
{
  Serial.begin(115200);
  delay(200);

 

  if (!amoled.begin()) {
    Serial.println("Failed to init LilyGO AMOLED.");
    while (true) delay(1000);
    
  }
  beginLvglHelper(amoled);   // init LVGL for this board
  connect_wifi();
  LoadSavedSettings();
  create_ui();
  
}

// Must have function: Loop runs continously on device after setup
void loop()
{
  lv_timer_handler();
  delay(5);

  if (needUpdateHistory)
  {
    needUpdateHistory = false;
    UpdateUI();
    
  }
}

