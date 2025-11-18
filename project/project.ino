#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <lvgl.h>

// Wi-Fi credentials (Delete these before commiting to GitHub)
static const char* WIFI_SSID     = "Säpo spårningsbil";
static const char* WIFI_PASSWORD = "Internet";

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

//API
HTTPClient http;
static const String API_URL = " ";

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

  // Tile #0
  {
    t0_label = lv_label_create(t0);
    lv_label_set_text(t0_label, "boot tile?");
    lv_obj_set_style_text_font(t0_label, &lv_font_montserrat_28, 0);
    lv_obj_center(t0_label);
    apply_tile_colors(t0, t0_label, /*dark=*/false);
  }

  // Tile #1 WEATHER FORECAST TILE
{
    // City title
    t1_label = lv_label_create(t1);
    lv_label_set_text(t1_label, "Karlskrona");
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

  // Example data, to be replaced 
  const char* days[7]  = {"Today", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char* parameters[7] = {"20°", "14°", "16°", "18°", "17°", "15°", "19°"};
  const char* icons[7] = {"S", "C", "S", "R", "C", "S", "R"}; 

  // loop that creates 7 rows of cells, with three columns
  for(int r = 0; r < 7; r++) {
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
      lv_obj_t* day_label = lv_label_create(day_obj);
      lv_label_set_text(day_label, days[r]);
      lv_obj_set_style_text_font(day_label, font, 0);
      lv_obj_center(day_label);

      // Parameter cell
      lv_obj_t* param_obj = lv_obj_create(grid);
      lv_obj_set_grid_cell(param_obj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, r, 1);
      lv_obj_clear_flag(param_obj, LV_OBJ_FLAG_SCROLLABLE);
      // background 
      lv_obj_set_style_bg_color(param_obj, lv_color_hex(0xC0E5F4), 0);
      lv_obj_set_style_bg_opa(param_obj, LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(param_obj, 0, 0);
      //text
      lv_obj_t* param_label = lv_label_create(param_obj);
      lv_label_set_text(param_label, parameters[r]);
      lv_obj_set_style_text_font(param_label, font, 0);
      lv_obj_center(param_label);

      // Icon cell 
      lv_obj_t* icon_obj = lv_obj_create(grid);
      lv_obj_set_grid_cell(icon_obj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, r, 1);
      lv_obj_clear_flag(icon_obj, LV_OBJ_FLAG_SCROLLABLE);
      // background 
      lv_obj_set_style_bg_color(icon_obj, lv_color_hex(0xC0E5F4), 0);
      lv_obj_set_style_bg_opa(icon_obj, LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(icon_obj, 0, 0);
      //text
      lv_obj_t* icon_label = lv_label_create(icon_obj);
      lv_label_set_text(icon_label, icons[r]);
      lv_obj_set_style_text_font(icon_label, font, 0); 
      lv_obj_center(icon_label);
  }
  }

  // Tile #2
  {
    t2_label = lv_label_create(t2);
    lv_label_set_text(t2_label, "Historical weather data / graph");
    lv_obj_set_style_text_font(t2_label, &lv_font_montserrat_28, 0);
    lv_obj_center(t2_label);

    apply_tile_colors(t2, t2_label, /*dark=*/false);
    lv_obj_add_flag(t2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(t2, on_tile2_clicked, LV_EVENT_CLICKED, NULL);
  }

  // Tile #3
  {
    t3_label = lv_label_create(t3);
    lv_label_set_text(t3_label, "third tile?");
    lv_obj_set_style_text_font(t3_label, &lv_font_montserrat_28, 0);
    lv_obj_center(t3_label);
    apply_tile_colors(t3, t3_label, /*dark=*/false);
  }
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

static String ConnectAndGetFromAPI()
{
  Serial.print("[HTTP] begin...");
  http.begin(API_URL);
  Serial.print("[HTTP] GET...");
  int httpCode = http.GET();
  String toReturn = "might not have worked (Object toReturn)"; //placeholder...
  if(httpCode > 0) // if the code is negative means the httprequest didn't work.
  {
    Serial.printf("[HTTP] Get... code:%d\n", httpCode);
    if(httpCode = HTTP_CODE_OK)
    {
      String payload = http.getString();
      Serial.println(payload);
      toReturn = payload;
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return toReturn;
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
  create_ui();
  connect_wifi();
  
}

// Must have function: Loop runs continously on device after setup
void loop()
{
  //Hello world test
  lv_timer_handler();
  delay(5);
}

