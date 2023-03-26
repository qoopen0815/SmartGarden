#ifndef ELASTICSEARCHCLIENT_H
#define ELASTICSEARCHCLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

class ElasticsearchClient {
 private:
  String m_host;
  int m_port;

  HTTPClient m_http;

 public:
  ElasticsearchClient(const String& host, const int port);
  bool isIndexExist(const char* indexName);
  void createIndex(const char* indexName);
  void setIndexMapping(const char* indexName, const JsonDocument& mapping);
  void setIndexIlmPolicy(const char* indexName, const JsonDocument& ilmPolicy);
  void uploadData(const char* indexName, const JsonDocument& data,
                  const char* indexId);
};

#endif  // ELASTICSEARCHCLIENT_H