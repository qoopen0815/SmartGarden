#ifndef ELASTICSEARCH_CLIENT_H
#define ELASTICSEARCH_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

class ElasticsearchClient {
   private:
    HTTPClient _http;
    String _host;
    int _port;

   public:
    ElasticsearchClient(const String& host, const int port);
    bool isIndexExist(const char* index_name);
    void createIndex(const char* index_name);
    void setIndexMapping(const char* index_name, const JsonDocument& mapping);
    void setIndexIlmPolicy(const char* index_name,
                           const JsonDocument& ilm_policy);
    void uploadData(const char* index_name, const JsonDocument& data,
                    const char* index_id);
};

#endif  // ELASTICSEARCH_CLIENT_H