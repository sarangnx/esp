#include "http_client.h"

#include <cstring>

const char* HttpClient::TAG = "HttpClient";

HttpClient::HttpClient(const std::string& _url) : url(_url) {}

HttpClient::~HttpClient() {
  if (client) {
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
  }
}

esp_err_t HttpClient::event_handler(esp_http_client_event_t* evt) {
  HttpClient* self = static_cast<HttpClient*>(evt->user_data);

  switch (evt->event_id) {
  case HTTP_EVENT_ON_DATA:
    if (evt->data_len > 0)
      self->response_body.append(static_cast<char*>(evt->data), evt->data_len);
    break;

  case HTTP_EVENT_ON_FINISH:
    self->status_code = esp_http_client_get_status_code(evt->client);
    if (self->callback)
      self->callback(self->status_code, self->response_body, ESP_OK);
    self->response_body.clear();
    break;

  case HTTP_EVENT_ERROR:
    ESP_LOGE(TAG, "HTTP error");
    if (self->callback)
      self->callback(0, "", ESP_FAIL);
    self->response_body.clear();
    break;

  default:
    break;
  }
  return ESP_OK;
}

void HttpClient::get(Callback _callback) {
  callback = _callback;
  response_body.clear();

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.event_handler = event_handler;
  config.user_data = this;  // pass 'this' so handler can reach members
  config.is_async = true;
  config.timeout_ms = 5000;

  client = esp_http_client_init(&config);

  esp_err_t err;
  // esp_http_client_perform returns ESP_ERR_HTTP_EAGAIN while in progress
  do {
    err = esp_http_client_perform(client);
    if (err == ESP_ERR_HTTP_EAGAIN)
      vTaskDelay(pdMS_TO_TICKS(10));
  } while (err == ESP_ERR_HTTP_EAGAIN);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "GET failed: %s", esp_err_to_name(err));
    if (callback)
      callback(0, "", err);
  }
}
