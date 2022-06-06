#include <Arduino.h>

#define SERIALON true        // Set tp true to turn Serial communication on, false for off
#define DEBUG true          // Set to true to send debug information over Serial. Turn off if collecting data over Serial.

#include "ArduinoJson.h"

// Include functions
#include "EZSerial.h"
#include "readSensor.h"
#include "sendDataInflux.h"

void setup()
{

#if SERIALON
  Serial.begin(115200);
  while (!Serial)
  {
    // Wait for serial
  }
#endif

  // Initialize Read Sensor task
  // xTaskCreatePinnedToCore(
  //     readSensor,    // Task function
  //     "Read Sensor", // Task name
  //     8 * 1024,      // Allocated memory
  //     NULL,          // Task pointer (will be created)
  //     1,             // Priority of task (higher is more priority)
  //     NULL,          // Task handler
  //     0);            // Core on which task runs

  // Initialize Send Data task
  xTaskCreatePinnedToCore(
      sendData,    // Task function
      "Send Data", // Task name
      96 * 1024,   // Allocated memory
      NULL,        // Task pointer (will be created)
      1,           // Priority of task (higher is more priority)
      NULL,        // Task handler
      1);          // Core on which task runs
}

void loop()
{
  // Empty because tasks are handled in RTOS
  delay(1);
}