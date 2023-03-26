#include "ElasticsearchClient.h"

ElasticsearchClient::ElasticsearchClient(const String& host, const int port)
    : m_host(host), m_port(port) {}

bool ElasticsearchClient::isIndexExist(const char* indexName) {
  m_http.begin(m_host, m_port, String(indexName) + "?pretty");
  const int httpCode = m_http.GET();
  m_http.end();
  return httpCode == HTTP_CODE_OK;
}

void ElasticsearchClient::setIndexMapping(const char* indexName,
                                          const JsonDocument& mapping) {
  String payload;
  serializeJson(mapping, payload);

  m_http.begin(m_host, m_port, String(indexName) + "?pretty");
  m_http.addHeader("Content-Type", "application/json");
  const int httpResponseCode = m_http.PUT(payload);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    Serial.println(m_http.getString());
  } else {
    Serial.printf("HTTP Error: %d\n", httpResponseCode);
  }
  m_http.end();
}

void ElasticsearchClient::setIndexIlmPolicy(const char* indexName,
                                            const JsonDocument& ilmPolicy) {
  String payload;
  serializeJson(ilmPolicy, payload);

  m_http.begin(m_host, m_port, String(indexName) + "/_ilm/policy");
  m_http.addHeader("Content-Type", "application/json");
  const int httpResponseCode = m_http.PUT(payload);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    Serial.println(m_http.getString());
  } else {
    Serial.printf("HTTP Error: %d\n", httpResponseCode);
  }
  m_http.end();
}

void ElasticsearchClient::uploadData(const char* indexName,
                                     const JsonDocument& data,
                                     const char* indexId) {
  String payload;
  serializeJson(data, payload);

  m_http.begin(m_host, m_port,
               String(indexName) + "/_doc/" + indexId + "?pretty");
  m_http.addHeader("Content-Type", "application/json");
  const int httpResponseCode = m_http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    Serial.println(m_http.getString());
  } else {
    Serial.printf("HTTP Error: %d\n", httpResponseCode);
  }
  m_http.end();
}
