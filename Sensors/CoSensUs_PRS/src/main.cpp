#include <Arduino.h>

#define SERIALON true        // Set tp true to turn Serial communication on, false for off
#define DEBUG false          // Set to true to send debug information over Serial. Turn off if collecting data over Serial.
#define DATAMODESERIAL false // Set to true to enable sending data over serial, false to deploy sensor and send data to InfluxDB
#define PREPROCESSING true   // Set to true to enable on-board pre processing
#define FFTON false          // Set to true to enable FFT pre processing (CURRENTLY UNAVBAILABLE)
#define NNON true            // Set to true to enable TinyML
#define NORMALIZE true       // Set to true to normalize values before feeding to Neural Network

#include "CircularBuffer.h"
#include "ArduinoJson.h"

// Sensor stream buffer setup
#define SAMPLES 32                              // Amount of samples before processing
#define SAMPLING_FREQUENCY 16                   // in Hz
#define BUFFER_NUM 1                            // Amount of buffers
#define SAMPLE_SURPLUS (SAMPLING_FREQUENCY / 2) // Amount of samples that the buffer can accomodate extra before being full
const long sampling_interval = round(1000000 / SAMPLING_FREQUENCY);
CircularBuffer<float, SAMPLES + SAMPLE_SURPLUS> sampleBuffer;
CircularBuffer<float, SAMPLES + SAMPLE_SURPLUS> *buffers[BUFFER_NUM] = {&sampleBuffer};

// Include functions
#include "EZSerial.h"
#include "readSensor.h"

#if FFTON
#include <fft.h>
#endif

#if DATAMODESERIAL
#include "sendDataSerial.h"
#else
#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>
#include "sendDataInflux.h"
#endif

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
  xTaskCreatePinnedToCore(
      readSensor,    // Task function
      "Read Sensor", // Task name
      8 * 1024,      // Allocated memory
      NULL,          // Task pointer (will be created)
      1,             // Priority of task (higher is more priority)
      NULL,          // Task handler
      0);            // Core on which task runs

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
}