#include "WeatherService.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

WeatherService::WeatherService()
{
}

String WeatherService::BuildURL(float longitude, float latitude)
{
    // Din specifika URL (snow1g)
    String url = "https://opendata-download-metfcst.smhi.se/api/category/snow1g/version/1/geotype/point/lon/";
    url += String(longitude, 6);
    url += "/lat/";
    url += String(latitude, 6);
    url += "/data.json?timeseries?parameters=air_temperature";

    return url;
}

String WeatherService::APIRequest(String URL)
{
    HTTPClient http;
    http.begin(URL);
    
    Serial.println("[API] Request URL: " + URL);
    int httpCode = http.GET();
    String payload = ""; 

    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
    } else {
        Serial.printf("[API] Error code: %d\n", httpCode);
    }

    http.end();
    return payload;
}

std::vector<ForecastDataPoint> WeatherService::GetSevenDayForecast(float longitude, float latitude)
{
    std::vector<ForecastDataPoint> forecastData;

    String url = BuildURL(longitude, latitude);
    String payload = APIRequest(url);

    if (payload.isEmpty()) return forecastData;

    // Filter för att spara minne
    JsonDocument filter;
    filter["timeSeries"][0]["time"] = true;
    filter["timeSeries"][0]["data"]["air_temperature"] = true;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

    if (error) {
        Serial.println(error.f_str());
        return forecastData;
    }

    JsonArray series = doc["timeSeries"];
    
    // --- HÄR ÄR ÄNDRINGEN FÖR FILTRERING ---
    
    for (JsonObject item : series) {
        // 1. Om vi redan har 7 dagar, sluta leta för att spara tid/minne
        if (forecastData.size() >= 7) {
            break;
        }

        const char* timeStr = item["time"]; // T.ex. "2025-11-19T12:00:00Z"
        String t = String(timeStr);

        // 2. Kolla om tiden innehåller "T12:00:00"
        // SMHI returnerar alltid ISO 8601 format.
        if (t.indexOf("T12:00:00") >= 0) {
            
            ForecastDataPoint point;
            point.time = t; 
            
            point.temp = item["data"]["air_temperature"];
            
            forecastData.push_back(point);
        }
    }

    Serial.printf("[Info] Hittade %d dagar kl 12:00.\n", forecastData.size());

    return forecastData; 
}