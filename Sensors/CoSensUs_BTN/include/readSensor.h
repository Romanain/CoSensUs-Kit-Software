#define READDEBUG false}

void readSensor(void *pvParameters)
{
  // Run once
#if DEBUG
  Serial << "ReadSensor is running on core " << xPortGetCoreID() << "\n";
#endif

  

  delay(5000);  //Wi-Fi startup delay
  // Run forever
  while (1)
  {
    
  }
}