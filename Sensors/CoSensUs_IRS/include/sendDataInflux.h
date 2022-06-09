// Based on wineExample of EloquentTinyML library and SecureBatchWrite from InfluxDbClient library
#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "secrets.h"
#if NNON
#include "TinyML.h"
#endif

// DEVICE SPECIFIC VARIABLES
#define DEVICE "CSU_SU_02"
#define SENSORTYPE "IRS"
#define MEASUREMENT "iPadState"

// VARIABLES INFLUX
#define INFLUXDB_BUCKET "sensors"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3" // Amsterdam time-zone
#define NTP_SERVER1 "pool.ntp.org"           // Time sync
#define NTP_SERVER2 "time.nis.gov"           // Time sync
#define WRITE_PRECISION WritePrecision::MS
#define MAX_BATCH_SIZE 3
#define WRITE_BUFFER_SIZE 30
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 2
int iterations = 0;

#if PREPROCESSING
#include "Preprocessing.h"
#endif

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point sensorStatus(MEASUREMENT);

void updateTime()
{
  // Sync time for batching once every so often loops
  if (iterations++ >= (60 * 30))
  {
    timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);
    iterations = 0;
  }
}

void connectWifi()
{
  int attemptCounter = 0;
#if DEBUG
  Serial << "Connecting to wifi\n";
#endif

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
#if DEBUG
    Serial << "...";
#endif

    delay(500);
    attemptCounter++;
    if (attemptCounter > 20)
    { // Reset board if not connected after 10s
#if SERIALON
      Serial.println("Resetting due to Wifi not connecting...");
#endif
      ESP.restart();
    }
  }
#if SERIALON
  Serial << "\nWifi connected\nIP adress: " << WiFi.localIP() << "\n";
#endif
}

void sendData(void *pvParameters)
{
// RUN ONCE ---
#if DEBUG
  Serial << "SendDataInflux is running on core " << xPortGetCoreID() << "\n";
#endif
#if FFTON
  initializeFFT();
#endif

  connectWifi();

  sensorStatus.addTag("device", DEVICE);
  sensorStatus.addTag("sensor", SENSORTYPE);

  timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);

  // Check server connection
  if (client.validateConnection())
  {
#if DEBUG
    Serial << "Connected to InfluxDB: " << client.getServerUrl() << "\n";
  }
  else
  {
    Serial << "InfluxDB connection failed: " << client.getLastErrorMessage() << "\n";
#endif
  }

  // Enable messages batching and retry buffer
  client.setWriteOptions(WriteOptions().writePrecision(WRITE_PRECISION).batchSize(MAX_BATCH_SIZE).bufferSize(WRITE_BUFFER_SIZE));

#if NNON
  StartNN();
#endif

  // RUN FOREVER ---
  while (1)
  {
    if (sampleBuffer.size() >= SAMPLES)
    {

#if PREPROCESSING && !FFTON
      X_test_ptr = preProcessInputs();
      // for (int i = 0; i < N_INPUTS; i++)
      // {
      //   X_test[i] = X_test_ptr[i];
      // }
#endif

      int dof = 3;

#if PREPROCESSING && FFTON
      dof = 6;
#endif

#if FFTON
      //       for (int i = 0; i < dof; i++)
      //       {
      // #if PREPROCESSING
      //         X_test_ptr = preProcessInputs(runFFT(buffers[i]));
      //         X_test[2 * i + 0] = X_test_ptr[0];
      //         X_test[2 * i + 1] = X_test_ptr[1];
      // #else
      //         X_test_ptr = FFTBinner(runFFT(buffers[i]));
      //         for (int j = 0; j < BINS; j++)
      //         {
      //           X_test[i * BINS + j] = X_test_ptr[j];
      //         }
      // #endif
      //       }

#endif

#if NORMALIZE
      // Normalize outputs
      for (int i = 0; i < N_INPUTS; i++)
      {
        X_test[i] = (X_test[i] - mean_array[i]) / std_array[i];
#if DEBUG
        Serial << X_test[i] << ", ";
#endif
      }
#endif

#if NNON
#if DEBUG
      Serial << "\nStart prediction...\nFree heap: " << ESP.getFreeHeap() << "\n";
#endif
      y_pred = PredictNN();
      sensorStatus.addField(MEASUREMENT, int(y_pred + 1));
#elif PREPROCESSING
      for (int i = 0; i < BUFFER_NUM; i++)
      {
        // String measurementName = String(MEASUREMENT) + "_" + String(i) + "_mean";
        // sensorStatus.addField((measurementName), X_test_ptr[i * 2]);
        // measurementName = String(MEASUREMENT) + "_" + String(i) + "_std";
        // sensorStatus.addField((measurementName), X_test_ptr[i * 2 + 1]);
        // Serial << X_test_ptr[i * 2] << "\n";
        if (X_test_ptr[i * 2] < 3950)
        {
          sensorStatus.addField(MEASUREMENT, 2);
        }
        else
        {
          sensorStatus.addField(MEASUREMENT, 1);
        }
      }
#else
      for (int i = 0; i < BUFFER_NUM; i++)
      {
        for (int j = 0; j < SAMPLES; j++)
        {
          String measurementName = String(MEASUREMENT) + "_" + String(i) + "_" + String(j);
          sensorStatus.addField((measurementName), buffers[i]->shift());
        }
      }
#endif

#if DEBUG
      Serial << "Writing: " << client.pointToLineProtocol(sensorStatus) << "\n";
#endif

      client.writePoint(sensorStatus);

      sensorStatus.clearFields();

      if (WiFi.status() != WL_CONNECTED)
      {
#if DEBUG
        Serial << "Wifi connection lost\n";
#endif
      }

      // Give a message if batch is succesfully sent
      if (client.isBufferEmpty())
      {
#if DEBUG
        Serial << "Data sent to influx\n";
#endif
      }

      updateTime();
    }

    // Delay so task does not crash
    delayMicroseconds(sampling_interval / 2);
  }
}
