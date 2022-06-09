#define CONTINUOUS_SAMPLING true // Currently only continuous sampling is supported
String condition = "Unspecified";

void sendData(void *pvParameters)
{
// Run once
#if DEBUG
  Serial << "SendData is running on core " << xPortGetCoreID() << "\n";
#endif
#if FFTON
  initializeFFT();
#endif

  DynamicJsonDocument doc(1024 * (round(SAMPLES / 10) + 2)); // Check calculation tool at https://arduinojson.org/v6/assistant/

  // Run forever
  while (1)
  {
    // serialEvent does not trigger automatically in these loops
    serialEvent();

    if (messageComplete)
    {
      condition = lastSerialMessage;
      messageComplete = false;
    }

// Check if samples are collected
#if CONTINUOUS_SAMPLING
    if (sampleBuffer.size() >= SAMPLES)
    { // GyrZ is last to be filled
#else
    if (messageComplete)
    {
      messageComplete = false;
#endif

#if DEBUG
      Serial << "Readings: ";
#endif

// Gather data for message
#if DEBUG
      Serial << "FFT GO!\n";
#endif
      doc["Condition"] = condition;

#if FFTON
      // FFT PROCESSED DATA
      for (int b = 0; b < BUFFER_NUM; b++)
      {
        float *tmp = runFFT(buffers[b]);
        for (int i = 0; i < SAMPLES / 2; i++)
        {
          doc[labels[b]][i] = tmp[i];
        }
      }

#else
      // RAW DATA
      for (int i = 0; i < SAMPLES; i++)
      {
        doc["IR"][i] = sampleBuffer.shift();
      }
#endif

      serializeJson(doc, Serial);
      Serial << "\n";
    }

    // Delay so task does not crash
    delayMicroseconds(sampling_interval / 2);
  }
}
