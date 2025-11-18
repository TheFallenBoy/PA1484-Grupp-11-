#ifndef WEATHER_SERVICE_HPP
#define WEATHER_SERVICE_HPP
#include <vector>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
struct ForecastDataPoints
{
    float temp = 0;
    std::string date = " ";

};
class WeatherService
{
    private:
    String BuildURL(float longitude,float latitude); // Weather forecast 7days
    String BuildURL(String station); //historical data
    String APIRequest(String URL); // actually calls the api and returns the json string. Then it will be parsed maybe?
    
    public:
    std::vector<ForecastDataPoints> GetSevenDayForecast(float longitude,float latitude); //returns a vector with ForecastDataPoints
    

};

#endif