#include <ElasticsearchClient.h>

ElasticsearchClient::ElasticsearchClient(const String& host, const int port)
    : _host(host), _port(port) {}

bool ElasticsearchClient::isIndexExist(const char* index_name) {
    _http.begin(_host, _port, String(index_name) + "?pretty");
    const int http_code = _http.GET();
    _http.end();
    return http_code == HTTP_CODE_OK;
}

void ElasticsearchClient::setIndexMapping(const char* index_name,
                                          const JsonDocument& mapping) {
    String payload;
    serializeJson(mapping, payload);

    _http.begin(_host, _port, String(index_name) + "?pretty");
    _http.addHeader("Content-Type", "application/json");
    const int http_response_code = _http.PUT(payload);

    if (http_response_code > 0) {
        Serial.printf("HTTP Response code: %d\n", http_response_code);
        Serial.println(_http.getString());
    } else {
        Serial.printf("HTTP Error: %d\n", http_response_code);
    }
    _http.end();
}

void ElasticsearchClient::setIndexIlmPolicy(const char* index_name,
                                            const JsonDocument& ilm_policy) {
    String payload;
    serializeJson(ilm_policy, payload);

    _http.begin(_host, _port, String(index_name) + "/_ilm/policy");
    _http.addHeader("Content-Type", "application/json");
    const int http_response_code = _http.PUT(payload);

    if (http_response_code > 0) {
        Serial.printf("HTTP Response code: %d\n", http_response_code);
        Serial.println(_http.getString());
    } else {
        Serial.printf("HTTP Error: %d\n", http_response_code);
    }
    _http.end();
}

void ElasticsearchClient::uploadData(const char* index_name,
                                     const JsonDocument& data,
                                     const char* index_id) {
    String payload;
    serializeJson(data, payload);

    _http.begin(_host, _port,
                String(index_name) + "/_doc/" + index_id + "?pretty");
    _http.addHeader("Content-Type", "application/json");
    const int http_response_code = _http.POST(payload);

    if (http_response_code > 0) {
        Serial.printf("HTTP Response code: %d\n", http_response_code);
        Serial.println(_http.getString());
    } else {
        Serial.printf("HTTP Error: %d\n", http_response_code);
    }
    _http.end();
}
