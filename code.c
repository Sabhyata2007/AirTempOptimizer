#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

const char* ssid = "xxxxxxxxxx";  
const char* password = "xxxxxxxxxx";  

const char* serverName = "http://api.openweathermap.org/data/2.5/weather?q=Faizabad,IN&APPID=c98a278a93c94710aa01a505f84b17bc";// Replace with your API key and desired location
#define DHTPIN 4
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");
  lcd.clear();
  lcd.init(); // initialize the lcd
  lcd.backlight();
   

}
void loop(){

  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    http.setTimeout(10000); // Set timeout to 10 seconds

    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      // Parse JSON
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      float temp = doc["main"]["temp"];
      float out_temp = (temp-273.15);
      int out_humidity = doc["main"]["humidity"];
      int out_wind = doc["wind"]["speed"];

      Serial.println("Temperature: ");
      Serial.println(out_temp);
      Serial.println("Humidity: ");
      Serial.println(out_humidity);
      Serial.println("Wind speed");
      Serial.println(out_wind);
      if (out_temp >= 28 && out_temp <= 40) {
        cool(); // Normal cooling operation
      }
       else if (out_temp >= 20 && out_temp < 28) {
        // Use outside air if wind speed is high
        if (out_wind > 2) {
          // High wind speed: Use outside air
          useOutsideAir();
        }
       }
      else if (out_temp > 0 && out_temp < 20) {
        heat(); // Heating operation
      }
      int  room_humidity = dht.readHumidity();
      if (isnan(room_humidity)) {
        Serial.println("Failed to read from DHT sensor!");
      } else {
         if (room_humidity<30){
        Humidifier();
        }
        else if (room_humidity>50){
          dehumidifier();
        }
      }
      
    } else {
      Serial.println("Error on HTTP request: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
     
  
  delay(120000); // Delay for  2 min  before fetching weather data again
}

void cool() {
  // Code to activate cooling mechanism (e.g., turn on AC or fans)
  float room_temp = dht.readTemperature();
   if (isnan(room_temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (room_temp>=20 && room_temp<=21 ){
    lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Room cooled");  
  }
  else{
  Serial.println("Cooling activated");
  lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Cooling activated");        // print message at (0, 0)
  lcd.setCursor(2, 1);         // move cursor to   (2, 1)
  lcd.print("Temp : 20.C"); // print message at (2, 1)
  delay(2000); 
  } 
}
void heat() {
   
  // Code to activate heating mechanism (e.g., turn on heater)
   float room_temp = dht.readTemperature();
   if (isnan(room_temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (room_temp>=25 && room_temp<=26 ){
    lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Room heated");  
  }
  else{
  Serial.println("Heating activated");
   lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Heating");        // print message at (0, 0)
  lcd.setCursor(2, 1);         // move cursor to   (2, 1)
  lcd.print("Temp : 25.C");    // print message at (2, 1)
  delay(2000);
  }
}

void useOutsideAir() {

  // Code to utilize outside air for ventilation or cooling
  Serial.println("Using outside air for ventilation");
  // on the fan 
   
}
 void Humidifier(){
    lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Humidifier ON"); 
 }
 void dehumidifier(){
   lcd.clear();                 // clear display
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Humidifier OFF");
 }




