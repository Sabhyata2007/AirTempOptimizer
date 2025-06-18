
#  Parts List for Smart HVAC System 

| Component                     | Quantity  | Description / Note                                           |
| ----------------------------- | --------- | ------------------------------------------------------------ |
| **ESP32 Dev Board**           | 1         | Main controller, handles Wi-Fi, logic, Blynk, and sensor I/O |
| **DHT11 Sensor**              | 1         | For measuring room temperature and humidity                  |
| **PIR Motion Sensor**         | 1         | To detect human presence and enable/disable HVAC             |
| **Servo Motor (SG90)**        | 1         | Simulates window/vent opening/closing                        |
| **LEDs (Red, Blue, Green)**   | 3–4       | Simulates AC, Heater, Fan, and Humidifier status             |
| **Resistors (220Ω)**          | 3–4       | Current limiting resistors for LEDs                          |
| **Small 5V DC Fan**           | 1         | Used as functional fan output (instead of simulation)        |
| **Breadboard + Jumper Wires** | As needed | For prototyping and connections                              |
| **Power Supply**              | 1         | USB or regulated 5V power supply for ESP32 and components    |

---

##  Optional Additions

| Component                    | Quantity | Description / Note                                     |
| ---------------------------- | -------- | ------------------------------------------------------ |
| **Relay Module (4-channel)** | 1        | If planning to control real AC/heater/humidifier later |
| **Rain Sensor**              | 1        | For future weather-based expansion                     |
| **I2C LCD Display (16x2)**   | 1        | For real-time temperature/humidity display             |
| **Buzzer**                   | 1        | For audio alerts or warnings                           |

---

##  Notes

* LEDs simulate HVAC components for demo purposes.
* Real appliances should be connected **only with relays**, and proper electrical isolation.
* Wind speed and rainfall are fetched via OpenWeatherMap API.

