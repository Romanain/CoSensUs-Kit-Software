#define READDEBUG false

#include <MPU9250_WE.h>
#define MPU9250_ADDR 0x68
MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

void readImuFifo()
{
  myMPU9250.findFifoBegin(); /* this is needed for continuous Fifo mode. The Fifo buffer ends with a
  complete data set, but the start is within a data set. 512/6 or 512/12 */
  int count = myMPU9250.getFifoCount();
  int dataSets = myMPU9250.getNumberOfFifoDataSets();

#if DEBUG
  Serial << "Bytes in FIFO: " << count << "\n Data Sets: " << dataSets << "\n";
#endif

  for (int i = 0; i < dataSets; i++)
  {
    /* if you read acceleration (g) and gyroscope values you need to start with g values */
    xyzFloat gValue = myMPU9250.getGValuesFromFifo();
    xyzFloat gyr = myMPU9250.getGyrValuesFromFifo();

    sampleBufferAccX.push(gValue.x * 1000);
    sampleBufferAccY.push(gValue.y * 1000);
    sampleBufferAccZ.push(gValue.z * 1000);
    sampleBufferGyrX.push(gyr.x * 1000);
    sampleBufferGyrY.push(gyr.y * 1000);
    sampleBufferGyrZ.push(gyr.z * 1000);
  }
}

void readSensor(void *pvParameters)
{
  // Run once
#if DEBUG
  Serial << "ReadSensor is running on core " << xPortGetCoreID() << "\n";
#endif

  // Initialize IMU
  Wire.begin();
  if (!myMPU9250.init())
  {
#if DEBUG
    Serial.println("MPU9250 does not respond");
#endif
  }
  else
  {
#if DEBUG
    Serial.println("MPU9250 is connected");
#endif
  }

  // Wait and calibrate sensor
  delay(1000);
  myMPU9250.autoOffsets();

  // Set Digital Low Pass Filter (DLPF)
  myMPU9250.enableGyrDLPF();
  myMPU9250.setGyrDLPF(MPU9250_DLPF_5);

  // Set frequency
  myMPU9250.setSampleRateDivider(1000 / SAMPLING_FREQUENCY - 1);

  // Set ranges
  myMPU9250.setGyrRange(MPU9250_GYRO_RANGE_250); // 250, 500, 1000, 2000 degrees per second
  myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);   // 2G, 4G, 8G, 16G

  // Set more Digital Low Pass Filter (DLPF)
  myMPU9250.enableAccDLPF(true);
  myMPU9250.setAccDLPF(MPU9250_DLPF_0);

  // Start FIFO
  myMPU9250.setFifoMode(MPU9250_CONTINUOUS);
  myMPU9250.enableFifo(true);
  delay(100); // in some cases a delay after enabling Fifo makes sense
  myMPU9250.startFifo(MPU9250_FIFO_ACC_GYR);

  delay(5000);  //Wi-Fi startup delay
  // Run forever
  while (1)
  {
    myMPU9250.stopFifo();
    readImuFifo();
    myMPU9250.resetFifo();
    myMPU9250.startFifo(MPU9250_FIFO_ACC_GYR);

    delay(1000 / SAMPLING_FREQUENCY * FIFOSAMPLES);
  }
}