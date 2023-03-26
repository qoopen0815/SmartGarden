#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>
#include <WiFi.h>

#define ANALOG_READ_PIN 34
#define WIFI_SSID "hoge"
#define WIFI_PASS "fuga"
#define ELASTICSEARCH_HOST "piyo"
#define SENSOR_READ_INTERVAL 1000     // msec
#define DATA_UPLOAD_INTERVAL 1800000  // msec

// Elasticsearch index settings
const char* indexBaseName = "plant01";
const char* indexType = "_doc";

HTTPClient http;
StaticJsonDocument<200> doc;

struct tm timeInfo;

// Sensor data variables
long moisture;

// Declare functions
void readSensor(void *pvParameters);
void uploadData(void *pvParameters);
void getLocalTimeFromEpoch(time_t epochTime, struct tm* timeinfo);
bool isIndexExist(const char* host, const char* indexName);
void createNewIndex(JsonDocument& mapping, const char* host, char* indexName);
void createIndexName(const char* baseName, struct tm* timeinfo, char* indexName);
void createJsonPayload(JsonDocument& doc);
void sendHttpRequest(JsonDocument& doc, const char* host, const char* indexName, const char* indexType, const char* indexId);

void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi network
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up time
  configTime(
    0,    // time offset [sec]
    0,    // summer time
    "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");  // ntp server
  while (!time(nullptr))
  {
    delay(1000);
    Serial.println("Waiting for NTP sync...");
  }
  Serial.println("Time synchronized");
  
  // Regist RTC task
  xTaskCreatePinnedToCore(
    readSensor,   // Function to implement the task.
    "task1",
    4*1024,       // The size of the task stack specified as the number of * bytes.
    NULL,         // Pointer that will be used as the parameter for the task * being created.
    1,            // Priority of the task.
    NULL,         // Task handler.
    0);           // Core where the task should run.

  xTaskCreatePinnedToCore(
    uploadData,
    "task2",
    4*1024,
    NULL,
    2,
    NULL,
    0);
}

void loop()
{
}

void readSensor(void * pvParameters)
{
  while(1)
  {
    // ここでセンサ値を格納する
    // moisture = random(0, 10);
    moisture = analogRead(ANALOG_READ_PIN);
    Serial.println(moisture);
    delay(SENSOR_READ_INTERVAL);  // 1 min
  }
}

void uploadData(void * pvParameters)
{
  while(1)
  {
    // Get current date and time
    time_t now = time(nullptr);
    getLocalTimeFromEpoch(now, &timeInfo);

    // Create index name
    char indexName[30];
    createIndexName(indexBaseName, &timeInfo, indexName);

    // Check index exists
    if (!isIndexExist(ELASTICSEARCH_HOST, indexName))
    {
      StaticJsonDocument<192> mapping;
      JsonObject mappings_properties = mapping["mappings"].createNestedObject("properties");
      JsonObject mappings_properties__timestamp = mappings_properties.createNestedObject("@timestamp");
      mappings_properties__timestamp["type"] = "date";
      mappings_properties__timestamp["format"] = "date_time_no_millis";
      mappings_properties["soil_moisture"]["type"] = "long";
      createNewIndex(mapping, ELASTICSEARCH_HOST, indexName);
    }

    // Create upload data
    char timestamp[20];
    sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ",
            timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    StaticJsonDocument<64> doc;
    doc["@timestamp"] = timestamp;
    doc["soil_moisture"] = moisture;

    // Send data to elasticsearch via http
    char docId[20];
    sprintf(docId, "%04d%02d%02d%02d%02d%02d",
            timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    sendHttpRequest(doc, ELASTICSEARCH_HOST, indexName, indexType, docId);

    delay(DATA_UPLOAD_INTERVAL);  // 30 min
  }
}

void getLocalTimeFromEpoch(time_t epochTime, struct tm* timeinfo)
{
  localtime_r(&epochTime, timeinfo);
}

bool isIndexExist(const char* host, const char* indexName)
{
  http.begin(host, 9200, String(indexName) + "?pretty");
  int httpCode = http.GET();
  http.end();
  return httpCode == HTTP_CODE_OK;
}

void createNewIndex(JsonDocument& mapping, const char* host, char* indexName)
{
  Serial.println("create index");
  String payload;
  serializeJson(mapping, payload);
  
  // http.begin("http://" + String(host) + ":" + String(9200) + "/" + String(indexName) + "?pretty");
  http.begin(host, 9200, String(indexName) + "?pretty");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.PUT(payload);

  if (httpResponseCode > 0)
  {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    // String response = http.getString();
    // Serial.println(response);
  }
  else
  {
    Serial.printf("HTTP Error: %d\n", httpResponseCode);
  }
  
  payload.end();
  http.end();
}

void createIndexName(const char* baseName, struct tm* timeinfo, char* indexName)
{
  char dateBuff[20];
  strftime(dateBuff, sizeof(dateBuff), "%Y.%m.%d", timeinfo);
  sprintf(indexName, "%s-%04d.%02d.%02d", baseName, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
}

void sendHttpRequest(JsonDocument& doc, const char* host, const char* indexName, const char* indexType, const char* indexId)
{
  String payload;
  serializeJson(doc, payload);

  http.begin(host, 9200, String(indexName) + "/" + indexType + "/" + indexId + "?pretty");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0)
  {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    // String response = http.getString();
    // Serial.println(response);
  }
  else
  {
    Serial.printf("HTTP Error: %d\n", httpResponseCode);
  }

  payload.end();
  http.end();
}
