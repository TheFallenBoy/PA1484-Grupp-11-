#ifndef WEATHER_SERVICE_HPP
#define WEATHER_SERVICE_HPP

#include <Arduino.h>
#include <vector>

// Datastruktur för en punkt i prognosen
struct ForecastDataPoint {
    float temp;
    String time;
    const char* weekday;
    int iconID;
};


class WeatherService {
public:
    WeatherService();
    
    // Hämtar 7-dygnsprognos (eller det din URL ger)
    std::vector<ForecastDataPoint> GetSevenDayForecast();
    std::vector<float> GetHistoricalData();
    // Ny funktion för kartan: Hämtar temp för kommande 24h för en specifik plats
    std::vector<float> GetHourlyForecast(float lat, float lon);
    void SetStationID(int ID);
    int GetStationID();
    void SetParameterID(int ID);
    int GetParameterID();
    String unit;
    String city;

private:
    int currentStationID;
    int currentParameterID;
    float currentLongitude;
    float currentLatitude;
    String currentParameter;

    // Bygger din specifika URL
    String BuildURL();
    // Anropar nätverket
    String APIRequest(String URL);
    void SetWhatDay(struct ForecastDataPoint &datapoint);
    int getWeekday(int year, int month, int day);

    
    
    String BuildHistoricalURL();

};

#endif