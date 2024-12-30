/*
The sketch is tested on Xiao-C6 and Zigbee2MQTT.
Arduino-ESP32 core 3.1.0 was used.
The sensor is powered from D3 pin to achieve low power consumption.
Average consumption during deep sleep is 14uA @ 4.2V.
Code execution takes around 2.6s and consumes on average around 60mA @ 4.2V.
*/



#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif


#include "Adafruit_SHTC3.h"
#include "Zigbee.h"
#include "esp_sleep.h"

#define TEMP_SENSOR_ENDPOINT_NUMBER 10
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  600         /* Sleep for 55s will + 5s delay for establishing connection => data reported every 1 minute */

uint8_t button = BOOT_PIN;
uint8_t sensePower = D3;


ZigbeeTempSensor zbTempSensor = ZigbeeTempSensor(TEMP_SENSOR_ENDPOINT_NUMBER);
Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();


static void temp_sensor_value_update(void) {
  {
    // Read Sensirion SHTC3
    sensors_event_t humidity, temp;
    shtc3.getEvent(&humidity, &temp);
    float tsens_value = wakeCounter;
    
    Serial.printf("Updated temperature sensor value to %.2f°C\r\n", temp.temperature);
    Serial.printf("Updated humidity sensor value to %.2f°C\r\n", humidity.relative_humidity);
  
    zbTempSensor.setTemperature(temp.temperature);
    zbTempSensor.setHumidity(humidity.relative_humidity);
    
    zbTempSensor.reportTemperature();
    zbTempSensor.reportHumidity();
    delay(100);

  }
}

/********************* Arduino functions **************************/
void setup() {
  Serial.begin(115200);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Init button switch
  pinMode(button, INPUT_PULLUP);
  // Init sensor power
  pinMode(sensePower, OUTPUT);
  digitalWrite(sensePower, HIGH);
  // Wait 100ms for the sensor to start
  delay(100);
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }

  // Set Zigbee device name and model
  zbTempSensor.setManufacturerAndModel("Espressif", "ZigbeeTempSensor");

  // Set minimum and maximum temperature measurement value
  zbTempSensor.setMinMaxValue(0, 100);

  // Optional: Set tolerance for temperature measurement in °C (lowest possible value is 0.01°C)
  zbTempSensor.setTolerance(1);

  zbTempSensor.setPowerSource(ZB_POWER_SOURCE_BATTERY, 100);

  zbTempSensor.addHumiditySensor(0, 100, 1);

  // Add endpoint to Zigbee Core
  Zigbee.addEndpoint(&zbTempSensor);

  Serial.println("Starting Zigbee...");
  // When all EPs are registered, start Zigbee in End Device mode
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  } else {
    Serial.println("Zigbee started successfully!");
  }
  Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  // Wait 20s if boot reason is not wake from sleep. This is required for proper pairing with Zigbee2MQTT
  if ( (int)rtc_get_reset_reason(0) != 5 )  {   //  SW_CPU_RESET=12 ,  POWERON_RESET=1 , DEEPSLEEP_RESET=5
    delay(20000);
  }
}

void loop() {
  // Checking button for factory reset
  if (digitalRead(button) == LOW) { 
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        // If key pressed for more than 3secs, factory reset Zigbee and reboot
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
  }
  temp_sensor_value_update();
  // Turn off power to sensor
  digitalWrite(sensePower, LOW);
  Serial.println("Going to sleep now");
  // Put ESP to sleep
  esp_deep_sleep_start();
}
