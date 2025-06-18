
#  Project Logic – Smart HVAC System (ESP32)

##  Overview  

This Smart HVAC project uses an ESP32 to create an intelligent climate control system that adapts based on:

* **Indoor conditions** (temperature, humidity, motion)
* **Outdoor data** (via OpenWeatherMap API: temperature, humidity, wind, rain)
* **Manual user inputs** (via Blynk IoT dashboard)

---

##  Key Modes

### 1.  Manual Mode (`V0`)

* Enabled by user via Blynk (V0 = 1)
* User can manually control:

  * Fan (V2)
  * AC (V5)
  * Heater (V6)
  * Humidifier (V7)
* State is saved in **EEPROM** and retained across reboots.

### 2.  Automatic Mode (`V0 = 0`)

* Triggered when PIR detects human presence.
* ESP32 evaluates:

  * **Indoor temperature + humidity**
  * **Outdoor temperature, humidity, wind speed, rain**
* Based on this data:

  * Fan, AC, Heater, and Humidifier are controlled automatically.
  * Ventilation through servo-controlled **vents/windows** when outside conditions are favorable.

---

##  External Weather Data

Fetched every **10 minutes** from OpenWeatherMap API:

* `temp` (Kelvin → °C)
* `humidity`
* `wind speed`
* `weather → "Rain"` used to detect rain

Used for intelligent decisions in automatic mode:

* **Ventilation enabled only** if:

  * `wind_speed < 5.0 m/s`
  * `weather != "Rain"`

---

##  Temperature Control Logic

### Cooling Logic

```text
If (inside_temp > AC_target + threshold)
    If (outside_temp < 30)
        → Open window + vent + turn on fan (ventilation cooling)
    Else
        → Close vent + turn on AC
```

### Heating Logic

```text
If (inside_temp < Heater_target - threshold)
    If (outside_temp > 20)
        → Open window + vent + turn on fan (ventilation heating)
    Else
        → Close vent + turn on heater
```

### AC/Heater Target Temperatures

* Derived dynamically from **outside temperature**
* Logic via `getACTargetTemp()` and `getHeaterTargetTemp()`

---

##  Humidity Control Logic

* If `humidity < 30%` → Humidifier ON
* If `humidity > 70%` → Humidifier OFF

---

##  Task Management (FreeRTOS)

* `task_control`: Runs every 10 seconds to evaluate and control system logic
* `task_weather`: Runs every 10 minutes to fetch weather
* `task_blynk`: Keeps Blynk connected
* `sendDataToBlynk()`: Sends temperature/humidity to Blynk dashboard (V3, V4)

---

##  EEPROM Usage

| EEPROM Address | Purpose              |
| -------------- | -------------------- |
| `0`            | Manual mode flag     |
| `1–2`          | User set temperature |
| `3`            | Manual fan state     |

---

