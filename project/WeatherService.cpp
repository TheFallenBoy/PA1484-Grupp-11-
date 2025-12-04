#include "WeatherService.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

WeatherService::WeatherService()
{
    currentLongitude = 15.590337;
    currentLatitude = 15.590337;
    currentStationID = 65090;
    currentParameterID = 1;
}

String WeatherService::BuildURL()
{
    // specifika URL (snow1g)
    String url = "https://opendata-download-metfcst.smhi.se/api/category/snow1g/version/1/geotype/point/lon/";
    url += String(currentLongitude, 6);
    url += "/lat/";
    url += String(currentLatitude, 6);
    url += "/data.json?timeseries?parameters=";
    url += currentParameter;
    url += ",symbol_code"; 
    Serial.printf("Forecast url: %s",url);
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
        if (httpCode > 0) {
            Serial.println(http.getString());
        }
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

    String url = BuildURL();
    String payload = APIRequest(url);

    if (payload.isEmpty()) return forecastData;

    // Filter to save memory. The API call might have been to big for the ESP-32 chip...
    JsonDocument filter;
    filter["timeSeries"][0]["time"] = true;
    filter["timeSeries"][0]["data"][currentParameter] = true;
    filter["timeSeries"][0]["data"]["symbol_code"] = true;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

    if (error) {
        Serial.println(error.f_str());
        return forecastData;
    }

    JsonArray series = doc["timeSeries"];
    
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
            
            point.temp = item["data"][currentParameter];
            point.weekday = "Today";
            point.iconID = item["data"]["symbol_code"];
            forecastData.push_back(point);

        }
        else if (t.indexOf("T12:00:00") > 0) {
            
            ForecastDataPoint point;
            point.time = t; 
            
            point.temp = item["data"][currentParameter];
            point.iconID = item["data"]["symbol_code"];
            SetWhatDay(point);
            forecastData.push_back(point);
        }
    }

    Serial.printf("[Info] Hittade %d dagar kl 12:00.\n", forecastData.size());
    
    return forecastData; 
}



void WeatherService::SetStationID(int ID)
{
    switch (ID)
    {
        case 0:
            currentStationID = 65090;
            currentLatitude = 56.182822;
            currentLongitude = 15.590337;
            city = "Karlskrona";
            break;
        case 1:
            currentStationID = 97400;
            currentLatitude = 59.3293;
            currentLongitude = 18.0686;
            city = "Stockholm";
            break;
        case 2:
            currentStationID = 72420;
            currentLatitude = 57.7089;
            currentLongitude = 11.9746;
            city = "Gothenburg";
            break;
        case 3:
            currentStationID = 53300;
            currentLatitude = 55.6050;
            currentLongitude = 13.0038;
            city = "Malmo";
            break;
        case 4:
            currentStationID = 180940;
            currentLatitude = 67.8557;
            currentLongitude = 20.2255;
            city = "Kiruna";            
            break;
        default:
            break;
}
}

int WeatherService::GetStationID()
{
    return currentStationID;
} 

void WeatherService::SetParameterID(int ID)
{
    switch (ID)
    {
        case 0:
            currentParameterID = 1;
            currentParameter = "air_temperature" ;
            unit = "°C";
            break;
        case 1:
            currentParameterID = 6;
            currentParameter = "relative_humidity";
            unit = "%";
            break;
        case 2:
            currentParameterID = 4;
            currentParameter = "wind_speed";
            unit = "m/s";
            break;
        case 3:
            currentParameterID = 9;
            currentParameter = "air_pressure_at_mean_sea_level";
            unit = "hPa";
            break;
        default:
            break;
    }

   
}

int WeatherService::GetParameterID()
{
    return currentParameterID;
}

String WeatherService::BuildHistoricalURL()
{
    String url =  "https://opendata-download-metobs.smhi.se/api/version/latest/parameter/";
    url += String(currentParameterID);
    url += "/station/";
    url += String(currentStationID);
    url += "/period/latest-months/data.json";

    return url;
}

 std::vector<float> WeatherService::GetHistoricalData()
 {
    std::vector<float> historicalDataPoints;
    String url = BuildHistoricalURL();

    while (historicalDataPoints.size()==0)
    {
       
        String payload = APIRequest(url);

        if(payload.isEmpty()) 
        {
        Serial.print("Empty payload in GetHistoricalData()");
        return historicalDataPoints;
        }
        JsonDocument filter;
        filter["value"][0]["value"] = true;

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

        if (error) {
            Serial.println(error.f_str());
        
        // NYTT: Skriv ut de första 100 tecknen av payloaden för att se vad felet är
            Serial.println("[JSON Payload Start] " + payload.substring(0, 100));
            return historicalDataPoints;
        }
        JsonArray series = doc["value"];
        for(JsonObject value : series)
        {
            float dataPoint = float(value["value"]);
            historicalDataPoints.push_back(dataPoint);
        }
        
        Serial.println(historicalDataPoints.size());
    /* code */
    }
    
       return historicalDataPoints;


 }