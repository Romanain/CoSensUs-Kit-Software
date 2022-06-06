// Based on wineExample of EloquentTinyML library and SecureBatchWrite from InfluxDbClient library
#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "secrets.h"

// DEVICE SPECIFIC VARIABLES
#define DEVICE "CSU_SU_04"
#define SENSORTYPE "Button"
#define MEASUREMENT "self_report"

// VARIABLES INFLUX
#define INFLUXDB_BUCKET "sensors"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3" // Amsterdam time-zone
#define NTP_SERVER1 "pool.ntp.org"           // Time sync
#define NTP_SERVER2 "time.nis.gov"           // Time sync
#define WRITE_PRECISION WritePrecision::MS
#define MAX_BATCH_SIZE 1
#define WRITE_BUFFER_SIZE 30
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 2
int iterations = 0;

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point sensorStatus(MEASUREMENT);

// Buttons
#define BUTTON_NUM 4
int buttonPins[BUTTON_NUM] = {17, 16, 4, 12};
bool buttonStates[BUTTON_NUM];
bool lastButtonStates[BUTTON_NUM] = {false, false, false, false};
unsigned long lastDebounceTime[BUTTON_NUM] = {0, 0, 0, 0};
unsigned long debounceDelay = 50;
String buttonLabels[BUTTON_NUM] = {"Red", "Yellow", "Green", "Blue"};

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

  for (int i = 0; i < BUTTON_NUM; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLDOWN);
  }

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

  // RUN FOREVER ---
  while (1)
  {
    for (int i = 0; i < BUTTON_NUM; i++)
    {
      int reading = digitalRead(buttonPins[i]);

      if (reading != lastButtonStates[i])
      {
        lastDebounceTime[i] = millis();
        Serial << "Debounce btn " << i << "\n";
      }

      if ((millis() - lastDebounceTime[i]) > debounceDelay)
      {

        if (reading != buttonStates[i])
        {
          buttonStates[i] = reading;

          if (buttonStates[i] == HIGH)
          {
            sensorStatus.addField((buttonLabels[i]), i + 1);

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
        }
      }

      lastButtonStates[i] = reading;
    }

    // Delay so task does not crash
    delay(1);
  }
}
