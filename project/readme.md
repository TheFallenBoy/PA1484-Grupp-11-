## Introduction
This project is a weather monitoring system running on an ESP32 microcontroller with a touchscreen. It acts as a complete weather dashboard that uses the **SMHI Open Data API** to provide a comprehensive overview of both forecasts and historical weather data without clutter.

The user interface is built using **LVGL** and is organized into a **TileView** layout, allowing the user to swipe between five distinct screens (tiles), each serving a specific purpose:

1.  **Boot Screen:** View group name and software version.
2.  **7-Day Forecast:** A detailed list view showing the forecast for the coming week for the selected city.
3.  **Historical Data:** A graphical chart plotting weather data trends over time.
4.  **Settings:** Select City (5 presets) and Parameter (Temperature, Wind Speed, Humidity, Pressure), save settings as default or reset to default.
6.  **National Overview:** An interactive map of Sweden visualizing temperature differences across 25 regions using color-coded animations.

Most weather apps show either too little information or are cluttered. This device solves this by separating information into logical screens (Tiles), allowing the user to switch between a quick local check, deep historical analysis, and a broad national overview seamlessly.

**Technologies used:**
- **Hardware:** ESP32, Touchscreen display.
- **Framework:** PlatformIO (VS Code), Arduino Framework.
- **GUI Library:** LVGL (Light and Versatile Graphics Library) v8.
- **Data Source:** SMHI API (PMP3g for forecasts, MetObs for historical data).
- **Networking:** ESP32 Wi-Fi & HTTPClient.
- **Data Handling:** ArduinoJson

## Getting started

### Prerequisites

* **Visual Studio Code** with the **PlatformIO** extension installed.
* An **ESP32 development board** with a display supported by LVGL.

### Installation

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/TheFallenBoy/PA1484-Grupp-11-
    ```
2.  **Open in PlatformIO:**
    Open VS Code, navigate to the PlatformIO Home, and select "Open Project". Choose the project folder.
3.  **Install Dependencies:**
    PlatformIO will automatically download the necessary libraries defined in `platformio.ini` (LVGL, ArduinoJson, etc.) upon the first build.
4.  **Wi-Fi Configuration:**
    Open the source file where the network connection is handled, **Project.ino**, and update the credentials:
    ```cpp
    static const char* WIFI_SSID     = "YOUR_SSID";
    static const char* WIFI_PASSWORD = "YOUR_PASSWORD";
    ```

### How to build and upload
1.  Connect the ESP32 to your computer via USB.
2.  Click the **PlatformIO icon** (Ant Icon) in the VS Code sidebar.
3.  Click **Build** (Checkmark icon) to compile.
4.  Click **Upload** (Arrow icon) to flash the firmware.
5.  Open the **Serial Monitor** (Plug icon) to view debug logs.

### Operating the Program
The interface is touch-based. You navigate by swiping or tapping:
* **Swipe Left/Right/Up/Down:** Navigate between the 4 main tiles and scrolling where applicable.
* **Forecast Tile:** Automatically updates based on the selected city.
* **Settings Tile:** Use the dropdowns/buttons to change the active **City** (Karlskrona, Stockholm, Gothenburg, Malmo, Kiruna) and **Parameter** (Temp, Wind, Humidity, Pressure).
* **Map Tile:** Tap the "Get Data" button to refresh the national forecast, this may take up to several minutes.
  

## Features

  ** Start-up **
- [x] US1.1C: As a user, I want to see a starting screen to display the current program
version and group number on the first screen.
- [x] US1.3: As a user, I want to have a screen to view weather forecast data.
- [x] US1.2C: As a user, I want to see the weather forecast for the next 7 days for the
selected city on the second screen in terms of temperature and weather conditions
with symbols (e.g., clear sky, rain, snow, thunder) per day at 12:00.

** Tile Navigation & Access **
- [x] US2.1: As a user, I want to be able to navigate between different screens (like forecast
screen) by sliding a finger over the touch screen.
Weather Data Display
- [x] US3.1: As a user, I want to have a screen to view historical weather data.
- [x] US3.2D: As a user, on the third screen I want to view the latest months (SMHI API
period: latest-months) of historical hourly data for selected weather parameter in the
selected city, using a slider to interact with the historical graph by scrolling where a
depleted slider corresponds to the oldest datapoint and a full slider corresponds to the
latest datapoint.

**Settings & Configuration**
- [x] US4.1: As a user, on the fourth screen, I want to access a single settings screen to
configure both the city and weather parameter options.
- [x] US4.2B: As a user, I want to select from four weather parameters, namely
temperature (1), humidity (6), wind speed (4), and Air pressure (9), using a dropdown
list, to customize the historical graph.
- [x] US4.3B: As a user, I want to select from five different cities, namely
Karlskrona(65090), Stockholm(97400), Göteborg(72420), Malmö(53300), and
Kiruna(180940), using a dropdown, to view their weather data for the historical data
and starting screen forecast.
- [x] US4.4: As a user, I want to reset the selected city and weather parameter to default
using a button.
- [x] US4.5: As a user, I want to set my default city and weather parameter to the current
selection using a button, so they are automatically selected when I start the device.
- [x] US4.6: As a user, I want the device to store my default city and weather parameter so
that they are retained even after a restart.
Extended Forecast & Visualization
- [x] US5.1: As a user, I want to access a screen to view the weather forecast for all of
Sweden.
- [x] US5.2: As a user, I want to see the temperature forecast for each administrative area
of Sweden (“Landskap”) on a map, with:
  - A Color-coded temperature zone per each administrative area, using the center of each area to access the forecast.
  - A looped animation displaying the hourly forecast for the next 24 hours.
