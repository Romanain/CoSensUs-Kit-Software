#define READDEBUG false

void readSensor(void *pvParameters)
{
  // Run once
#if DEBUG
  Serial << "ReadSensor is running on core " << xPortGetCoreID() << "\n";
#endif

  pinMode(A0, INPUT_PULLUP);

  delay(5000); // Wi-Fi startup delay
  // Run forever
  while (1)
  {
    sampleBuffer.push(analogRead(A0));

    vTaskDelay(sampling_interval / 1000);
  }
}