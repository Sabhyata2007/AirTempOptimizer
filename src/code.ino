#include <EEPROM.h> // for saving manual mode (if blynk crash)
#define EEPROM_SIZE 10
// include header file 
#define BLYNK_TEMPLATE_ID "TMPL3VscFSlv8"
#define BLYNK_TEMPLATE_NAME "smart hvac"
#define BLYNK_AUTH_TOKEN "TPn585yTpkxFD9_LfEzkjBSJ3OdUKKs1"

#include <ESP32Servo.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// defining  variable 
#define PIR_PIN 4  // pir sensor connection
#define DHTPIN 21 // temp pin 
#define DHTTYPE DHT11 // type of temp sensor 
#define FAN_PIN 23
#define HUMIDIFIER_PIN 18
#define AC_PIN 14         // define AC control pin
#define HEATER_PIN 19
#define SERVO_PIN 15 
#define window_switch 13

#define EEPROM_ADDR_MODE        0
#define EEPROM_ADDR_USER_TEMP   1
#define EEPROM_ADDR_FAN_STATE   3 
// set temp
BlynkTimer timer;
Servo myServo;
DHT dht(DHTPIN, DHTTYPE);

const char* serverName = "http://api.openweathermap.org/data/2.5/weather?q=Faizabad,IN&APPID=c98a278a93c94710aa01a505f84b17bc";
const char* ssid = "Sameer Soni 2.4g";// wificonnection 
const char* password = "swastik123";

bool window_open = false;
bool manualMode = false;
bool vent_open = false;
float outside_temp;
int outside_humidity;
float wind_speed;
bool isRaining =false; 
float threshold = 2.0;
//unsigned long lastWeatherCheck = 0;
//unsigned long weatherInterval = 60000; // 60 seconds

int user_set_temp = 24; // Manual mode target
int fan_manual = 0;     // 0 = OFF, 1 = ON
int ac_manual = 0;
int heater_manual = 0;
int humidifier_manual = 0;


  /**************************eeprom saving ******************/
void saveSettingsToEEPROM() {
  EEPROM.write(EEPROM_ADDR_MODE, manualMode);
  EEPROM.write(EEPROM_ADDR_USER_TEMP, (user_set_temp >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_USER_TEMP + 1, user_set_temp & 0xFF);
  EEPROM.write(EEPROM_ADDR_FAN_STATE, fan_manual);
  EEPROM.commit();
  Serial.println("Settings saved to EEPROM.");
}

void loadSettingsFromEEPROM() {
  manualMode = EEPROM.read(EEPROM_ADDR_MODE);
  user_set_temp = (EEPROM.read(EEPROM_ADDR_USER_TEMP) << 8) | EEPROM.read(EEPROM_ADDR_USER_TEMP + 1);
  fan_manual = EEPROM.read(EEPROM_ADDR_FAN_STATE);

  Serial.println("Settings loaded from EEPROM:");
  Serial.print("manualMode: "); Serial.println(manualMode);
  Serial.print("user_set_temp: "); Serial.println(user_set_temp);
  Serial.print("fan_manual: "); Serial.println(fan_manual);
}



/*********************blynk calls****************/
  BLYNK_WRITE(V0) {
      manualMode = param.asInt();
      saveSettingsToEEPROM(); }

  BLYNK_WRITE(V1) { 
     user_set_temp = param.asInt();
     saveSettingsToEEPROM(); }

  BLYNK_WRITE(V2) {
    fan_manual = param.asInt(); 
    saveSettingsToEEPROM();}

  BLYNK_WRITE(V5) {
  ac_manual = param.asInt();}

  BLYNK_WRITE(V6) {
  heater_manual = param.asInt();}

  BLYNK_WRITE(V7) {
  humidifier_manual = param.asInt();}

/***************setup*****************/

void setup(){
      Serial.begin(115200);
         
    // Start connecting to Wi-Fi
      Serial.print("Connecting to ");
      Serial.println(ssid);
      WiFi.begin(ssid, password);

    // Wait until connected
       while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
      }

    // Successfully connected
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      EEPROM.begin(EEPROM_SIZE);    
      loadSettingsFromEEPROM();  
      
      pinMode(PIR_PIN, INPUT);
      pinMode(FAN_PIN, OUTPUT);
      pinMode(HUMIDIFIER_PIN, OUTPUT);
      pinMode(AC_PIN, OUTPUT);
      pinMode(HEATER_PIN, OUTPUT);
      pinMode(window_switch,INPUT);

      Serial.println("PIR Sensor Initialized...");
      myServo.attach(SERVO_PIN);
      Serial.println("Servo ready");
      dht.begin();
      Serial.println("DHT Sensor Reading Started...");

    =
     Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
     timer.setInterval(3000L, sendDataToBlynk); // Send to Blynk every 3s

     Serial.println("System Ready...");

      xTaskCreate(task_blynk, "Blynk Task", 4096, NULL, 1, NULL);
      xTaskCreate(task_weather, "Weather Task", 4096, NULL, 1, NULL);
      xTaskCreate(task_control, "Control Task", 4096, NULL, 1, NULL);


}


  BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0, V1, V2, V5, V6, V7);
  }

/**************outside temp checking*****************/

void task_weather(void *pvParameters) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      int httpCode = http.GET();
      if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        outside_temp = doc["main"]["temp"].as<float>() - 273.15;
        outside_humidity = doc["main"]["humidity"];
        wind_speed = doc["wind"]["speed"];
        const char* weather = doc["weather"][0]["main"];
        isRaining = (strcmp(weather, "Rain") == 0);
      }
      http.end();
    }
    vTaskDelay(600000 / portTICK_PERIOD_MS); // every 10 minutes
  }
}
 
void task_blynk(void *pvParameters){

while (1) {
  
    Blynk.run();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Run often
  }
}

void task_control(void *pvParameters) {

  while (1) {
    int motion = digitalRead(PIR_PIN);
    if (!motion) {
      manualMode = false;
      Blynk.virtualWrite(V0, 0);
      closeWindow();
      closeVent();
      digitalWrite(FAN_PIN, HIGH);
      digitalWrite(AC_PIN, LOW);
      digitalWrite(HEATER_PIN, LOW);
      digitalWrite(HUMIDIFIER_PIN, LOW);
    } else {
      if (manualMode)
            manual_control();
      else
         automatic_control();
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS); // every 10 sec
  }
}



/*******************window and vent*********************/

void openWindow() {
  if (!window_switch) {
    myServo.write(90);  // Adjust angle 
    window_open = true;
    Serial.println("Window Opened");
  }
}

void closeWindow() {
  if (window_switch) {
    myServo.write(0);  // Fully closed position
    window_open = false;
    Serial.println("Window Closed");
  }
}

void openVent() {
  if(!vent_open){
  vent_open = true;
  Serial.println("Vent opened");
  }
}

void closeVent() {
 if (vent_open) {
    vent_open = false;
    Serial.println("Vent closed");
  }
}

float getACTargetTemp(float outsideTemp) {
  if (outsideTemp >= 40) return 20.0;
  else if (outsideTemp >= 35) return 22.0;
  else if (outsideTemp >= 30) return 23.0;
  else return 24.0;
}

float getHeaterTargetTemp(float outsideTemp) {
  if (outsideTemp <= 5) return 26.0;
  else if (outsideTemp <= 10) return 24.0;
  else if (outsideTemp <= 15) return 23.0;
  else return 22.0;
}

/********************data to blynk*****************/

void sendDataToBlynk() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (!isnan(temp)) {
    Blynk.virtualWrite(V3, temp);  // Optional widgets
    Blynk.virtualWrite(V4, hum);
  }
}

/**************************manual control******************/

void manual_control(){

  digitalWrite(FAN_PIN, fan_manual ? LOW : HIGH);
  digitalWrite(AC_PIN, ac_manual ? HIGH : LOW);
  digitalWrite(HEATER_PIN, heater_manual ? HIGH : LOW);
  digitalWrite(HUMIDIFIER_PIN, humidifier_manual ? HIGH : LOW);
}

/*********************automatic control****************/
void automatic_control(){
    float inside_temp = dht.readTemperature();  
    float humidity = dht.readHumidity(); 

  if (isnan(inside_temp)) {
    Serial.println("Failed to read from DHT sensor!");
  } 


  else {
    Serial.print("Temperature: ");
    Serial.print(inside_temp);
    Serial.println(" °C");
    Serial.print(" °C  |  Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
    // Humidifier logic
  if (humidity < 30.0) {
    Serial.println("Dry air → Humidifier ON");
    digitalWrite(HUMIDIFIER_PIN, HIGH);
  } else if (humidity > 70.0) {
    Serial.println("Too humid → Humidifier OFF");
    digitalWrite(HUMIDIFIER_PIN, LOW);
  }
  else {
    digitalWrite(HUMIDIFIER_PIN, LOW); // Normal range
  }

  float acTarget = getACTargetTemp(outside_temp);
  float heaterTarget = getHeaterTargetTemp(outside_temp);

  
  // Climate control logic when raining 
  if (isRaining || wind_speed > 5.0) {
    Serial.println("Unsafe to open vent → RAIN/WIND");
    closeVent();
    digitalWrite(FAN_PIN, HIGH);
    if (inside_temp > acTarget ) {
      Serial.print("Temp > AC Target (");
      digitalWrite(AC_PIN, HIGH); 
      Serial.print(acTarget);
      Serial.println(") → AC ON (simulated)"); 
      
  } else if (inside_temp <  heaterTarget) {
      Serial.print("Temp < Heater Target (");
      digitalWrite(HEATER_PIN, HIGH); 
      Serial.print(heaterTarget);
      Serial.println(") → Heater ON (simulated)");
      
  } else {
    Serial.println("Comfortable temp → No heating/cooling");
    // turnOffAC_Heater();
    
  }
  return;
  }
  // hvac conttrol
  if (inside_temp > acTarget+ threshold) {
    if (outside_temp < 30.0) {
      //turnoff ac or heater
      Serial.println("Ventilation cooling");
      openVent();
      openWindow();
      digitalWrite(FAN_PIN, LOW);
    } 
      else {
      Serial.print("Temp > AC Target (");
      digitalWrite(AC_PIN, HIGH); 
      Serial.print(acTarget);
      Serial.println(") → AC ON (simulated)");
      // Turn on AC
      
      closeVent();
      closeWindow();
      digitalWrite(FAN_PIN, HIGH);
    }
  }

   else if (inside_temp < heaterTarget- threshold) {
    Serial.println("Room cold → Try warming");

    if (outside_temp > 20.0) {
       //turnoff ac or heater
      Serial.println("Ventilation heating");
      openWindow();
      openVent();
      digitalWrite(FAN_PIN, HIGH);
    } else {
      Serial.print("Temp < Heater Target (");
      digitalWrite(HEATER_PIN, HIGH); 
      Serial.print(heaterTarget);
      Serial.println(") → Heater ON (simulated)");
      closeVent();
      closeWindow();
      digitalWrite(FAN_PIN, HIGH);
      
      // Relay Heater logic here
    }

  } else {
    Serial.println("Comfort zone → System idle");
    closeVent();
    closeWindow();
    digitalWrite(FAN_PIN, HIGH);
  }

}

/*********************LOOP**********************/
 void loop(){
   timer.run();
}
