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
    std::vector<ForecastDataPoint> GetSevenDayForecast();

    std::vector<float> GetHistoricalData();

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
   
    
    String BuildURL();
    String APIRequest(String URL);
    void SetWhatDay(struct ForecastDataPoint &datapoint);
    int getWeekday(int year, int month, int day);

    
    
    String BuildHistoricalURL();

};

#endif