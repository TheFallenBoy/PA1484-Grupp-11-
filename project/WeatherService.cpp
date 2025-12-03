#include "WeatherService.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

WeatherService::WeatherService()
{
    longitude = 15.590337;
    latitude = 56.182822;
}

String WeatherService::BuildURL(float longitude, float latitude)
{
    // specifika URL (snow1g)
    String url = "https://opendata-download-metfcst.smhi.se/api/category/snow1g/version/1/geotype/point/lon/";
    url += String(longitude, 6);
    url += "/lat/";
    url += String(latitude, 6);
    url += "/data.json?timeseries?parameters=air_temperature,symbol_code"; 

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
//AI generated with gemini Pro 3 Metod
int WeatherService::getWeekday(int year, int month, int day) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    
    // Adjust year for Jan/Feb (they are treated as months 13/14 of previous year)
    if (month < 3) {
        year -= 1;
    }
    
    // Tomohiko Sakamoto's algorithm
    return (year + year/4 - year/100 + year/400 + t[month-1] + day + 6 ) % 7; //it was giving us one day ahead so we needed to subtract -1 which is the same as adding 6 in mod 7
}

void WeatherService::SetWhatDay(struct ForecastDataPoint &dataPoint)
{
    String date = dataPoint.time;
    //Format of the string is e.g 2025-11-19T18:00:00Z
    String yearStr = date.substring(0,4); //picks character 0 - 4
    String monthStr = date.substring(5,7);
    String dayStr = date.substring(8,10);
    int year = yearStr.toInt();
    int month = monthStr.toInt();
    int day = dayStr.toInt();
    int weekdayNumber = getWeekday(year,month,day);

    const static char* weekDays[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    dataPoint.weekday = weekDays[weekdayNumber];

}

std::vector<ForecastDataPoint> WeatherService::GetSevenDayForecast()
{
    std::vector<ForecastDataPoint> forecastData;

    String url = BuildURL(longitude, latitude);
    String payload = APIRequest(url);

    if (payload.isEmpty()) return forecastData;

    // Filter to save memory. The API call might have been to big for the ESP-32 chip...
    JsonDocument filter;
    filter["timeSeries"][0]["time"] = true;
    filter["timeSeries"][0]["data"]["air_temperature"] = true;
    filter["timeSeries"][0]["data"]["symbol_code"] = true;

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
        if (forecastData.size() == 0)
        { 
            //week day property will always be Today
            ForecastDataPoint point;
            point.time = t; 
            
            point.temp = item["data"]["air_temperature"];
            point.weekday = "Today";
            point.iconID = item["data"]["symbol_code"];
            forecastData.push_back(point);

            Serial.println(point.time);
            Serial.println(point.temp);
            Serial.println(point.weekday);
            Serial.println(point.iconID);

        }
        else if (t.indexOf("T12:00:00") > 0) {
            
            ForecastDataPoint point;
            point.time = t; 
            
            point.temp = item["data"]["air_temperature"];
            point.iconID = item["data"]["symbol_code"];
            SetWhatDay(point);
            forecastData.push_back(point);
            Serial.println(point.time);
            Serial.println(point.temp);
            Serial.println(point.weekday);
            Serial.println(point.iconID);

        }
    }

    Serial.printf("[Info] Hittade %d dagar kl 12:00.\n", forecastData.size());
    
    return forecastData; 
}

void WeatherService::SetStationID(int ID)
{

}

int WeatherService::GetStationID()
{
    return 0;
} 

void WeatherService::SetParameterID(int ID)
{
    switch (ID)
    {
        case 0:
            currentParameterID = 1;
            break;
        case 1:
            currentParameterID = 6;
            break;
        case 2:
            currentParameterID = 4;
            break;
        case 3:
            currentParameterID = 9;
            break;
        default:
            break;
    }

   
}

int WeatherService::GetParameterID()
{
    return currentParameterID;
}

String WeatherService::BuildHistoricalURL(int stationID)
{
    String url =  "https://opendata-download-metobs.smhi.se/api/version/latest/parameter/";
    url += String(currentParameterID);
    url += "/station/";
    url += String(stationID);
    url += "/period/latest-months/data.json";

    return url;
}

 std::vector<float> WeatherService::GetHistoricalData(int stationID )
 {
    std::vector<float> historicalDataPoints;
    String url = BuildHistoricalURL(stationID);

    Serial.println(url);

    return historicalDataPoints;
 }