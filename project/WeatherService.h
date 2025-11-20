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
    std::vector<ForecastDataPoint> GetSevenDayForecast(float longitude, float latitude);

private:
    // Bygger din specifika URL
    String BuildURL(float longitude, float latitude);
    
    // Anropar nätverket
    String APIRequest(String URL);

    void SetWhatDay(struct ForecastDataPoint &datapoint);

    int getWeekday(int year, int month, int day);
};

#endif