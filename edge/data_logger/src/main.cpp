#include <Arduino.h>
#include <ArduinoJson.h>
#include <ElasticsearchClient.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define ANALOG_READ_PIN 34
#define WIFI_SSID "hoge"
#define WIFI_PASS "fuga"
#define ELASTICSEARCH_HOST "piyo"
#define SENSOR_READ_INTERVAL 600000   // msec
#define DATA_UPLOAD_INTERVAL 1800000  // msec

// Elasticsearch settings
ElasticsearchClient elastic(ELASTICSEARCH_HOST, 9200);
const char *index_base_name = "plant01";

// Timestamp
struct tm time_info;

// Sensor data variables
long moisture;

// Declare functions
void readSensor(void *pvParameters);
void uploadData(void *pvParameters);
void getLocalTimeFromEpoch(time_t epoch_time, struct tm *timeinfo);
void createIndexName(const char *base_name, struct tm *timeinfo,
                     char *index_name);
void createMappingJsonPayload(JsonDocument &mapping);
void createIlmPolicyJsonPayload(JsonDocument &doc);

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi network
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Sync NTP server
    configTime(0, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
    while (!time(nullptr)) {
        delay(1000);
        Serial.println("Waiting for NTP sync...");
    }
    Serial.println("Time synchronized");

    // Regist RTOS task
    xTaskCreatePinnedToCore(readSensor,  // Function to implement the task.
                            "task1",
                            4 * 1024,  // The size of the task stack specified
                                       // as the number of * bytes.
                            NULL,  // Pointer that will be used as the parameter
                                   // for the task * being created.
                            1,     // Priority of the task.
                            NULL,  // Task handler.
                            0);    // Core where the task should run.

    xTaskCreatePinnedToCore(uploadData, "task2", 4 * 1024, NULL, 2, NULL, 0);
}

void loop() {}

void readSensor(void *pvParameters) {
    while (1) {
        // ここでセンサ値を格納する
        moisture = analogRead(ANALOG_READ_PIN);
        Serial.println(moisture);
        delay(SENSOR_READ_INTERVAL);
    }
}

void uploadData(void *pvParameters) {
    while (1) {
        // Get current date and time
        time_t now = time(nullptr);
        getLocalTimeFromEpoch(now, &time_info);

        // Create index name
        char index_name[30];
        createIndexName(index_base_name, &time_info, index_name);

        // Check index exists
        if (!elastic.isIndexExist(index_name)) {
            StaticJsonDocument<192> mapping;
            createMappingJsonPayload(mapping);
            elastic.setIndexMapping(index_name, mapping);
            StaticJsonDocument<384> ilm;
            createIlmPolicyJsonPayload(ilm);
            elastic.setIndexIlmPolicy(index_name, ilm);
        }

        // Create upload data
        char timestamp[20];
        sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                time_info.tm_year + 1900, time_info.tm_mon + 1,
                time_info.tm_mday, time_info.tm_hour, time_info.tm_min,
                time_info.tm_sec);
        StaticJsonDocument<64> doc;
        doc["@timestamp"] = timestamp;
        doc["soil_moisture"] = moisture;

        // Send data to elasticsearch via http
        char doc_id[20];
        sprintf(doc_id, "%04d%02d%02d%02d%02d%02d", time_info.tm_year + 1900,
                time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour,
                time_info.tm_min, time_info.tm_sec);
        elastic.uploadData(index_name, doc, doc_id);

        delay(DATA_UPLOAD_INTERVAL);
    }
}

void getLocalTimeFromEpoch(time_t epoch_time, struct tm *timeinfo) {
    localtime_r(&epoch_time, timeinfo);
}

void createIndexName(const char *base_name, struct tm *timeinfo,
                     char *index_name) {
    char date_buff[20];
    strftime(date_buff, sizeof(date_buff), "%Y.%m.%d", timeinfo);
    sprintf(index_name, "%s-%04d.%02d.%02d", base_name,
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
}

void createMappingJsonPayload(JsonDocument &doc) {
    JsonObject mappings_properties =
        doc["mappings"].createNestedObject("properties");
    JsonObject mappings_properties_timestamp =
        mappings_properties.createNestedObject("@timestamp");
    mappings_properties_timestamp["type"] = "date";
    mappings_properties_timestamp["format"] = "date_time_no_millis";
    mappings_properties["soil_moisture"]["type"] = "long";
}

void createIlmPolicyJsonPayload(JsonDocument &doc) {
    JsonObject policy_phases = doc["policy"].createNestedObject("phases");
    JsonObject policy_phases_hot_actions_rollover =
        policy_phases["hot"]["actions"].createNestedObject("rollover");
    policy_phases_hot_actions_rollover["max_age"] = "1d";
    policy_phases_hot_actions_rollover["max_size"] = "10gb";
    JsonObject policy_phases_warm_actions_rollover =
        policy_phases["warm"]["actions"].createNestedObject("rollover");
    policy_phases_warm_actions_rollover["max_age"] = "2d";
    policy_phases_warm_actions_rollover["max_size"] = "10gb";
    JsonObject policy_phases_cold_actions_rollover =
        policy_phases["cold"]["actions"].createNestedObject("rollover");
    policy_phases_cold_actions_rollover["max_age"] = "5d";
    policy_phases_cold_actions_rollover["max_size"] = "10gb";
    JsonObject policy_phases_delete =
        policy_phases.createNestedObject("delete");
    policy_phases_delete["min_age"] = "7d";
    JsonObject policy_phases_delete_actions_delete =
        policy_phases_delete["actions"].createNestedObject("delete");
}
