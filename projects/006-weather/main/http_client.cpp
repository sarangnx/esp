#include "http_client.h"

#include "esp_log.h"

extern const char weatherapi_pem_start[] asm("_binary_weatherapi_pem_start");
extern const char weatherapi_pem_end[] asm("_binary_weatherapi_pem_end");

const char* HttpClient::TAG = "HttpClient";

esp_err_t HttpClient::event_handler(esp_http_client_event_t* evt) {
  HttpClient* self = static_cast<HttpClient*>(evt->user_data);

  switch (evt->event_id) {
  case HTTP_EVENT_ON_DATA:
    if (evt->data_len > 0)
      self->response_body.append(static_cast<char*>(evt->data), evt->data_len);
    break;

  case HTTP_EVENT_ON_FINISH:
    self->status_code = esp_http_client_get_status_code(evt->client);
    if (self->callback) {
      self->callback(self->status_code, self->response_body, ESP_OK);
      self->response_body.clear();
    }
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

// asynchronous GET request with callback
void HttpClient::get(std::string url, Callback _callback) {
  callback = _callback;
  response_body.clear();

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.event_handler = event_handler;
  config.user_data = this;  // pass 'this' so handler can reach members
  config.is_async = true;
  config.timeout_ms = 5000;
  config.cert_pem = weatherapi_pem_start;

  esp_http_client_handle_t client = esp_http_client_init(&config);

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

  esp_http_client_close(client);
  esp_http_client_cleanup(client);
}

// synchronous GET request that returns the response body string
std::string HttpClient::get(std::string url) {
  response_body.clear();

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.event_handler = event_handler;
  config.user_data = this;  // pass 'this' so handler can reach members
  config.is_async = false;
  config.timeout_ms = 5000;
  config.cert_pem = weatherapi_pem_start;

  esp_http_client_handle_t client = esp_http_client_init(&config);

  esp_err_t err = esp_http_client_perform(client);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "GET failed: %s", esp_err_to_name(err));
  }

  ESP_LOGI(TAG, "HTTP GET completed with status code: %d", status_code);

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  return response_body;
}

uint8_t* HttpClient::getBuffered(std::string url, int* file_size) {
  esp_http_client_handle_t client = nullptr;

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.is_async = false;
  config.timeout_ms = 5000;
  config.cert_pem = weatherapi_pem_start;

  client = esp_http_client_init(&config);

  // Open the connection
  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return nullptr;
  }

  // Get the content length (file size)
  int content_length = esp_http_client_fetch_headers(client);
  if (content_length <= 0) {
    ESP_LOGE(TAG, "Failed to get content length, or size is 0");
    esp_http_client_cleanup(client);
    return nullptr;
  }

  *file_size = content_length;

  // Allocate memory in PSRAM (SPIRAM)
  response_buffer = (uint8_t*)heap_caps_malloc(content_length, MALLOC_CAP_SPIRAM);
  if (response_buffer == NULL) {
    ESP_LOGE(TAG, "Failed to allocate %d bytes in PSRAM", content_length);
    esp_http_client_cleanup(client);
    return nullptr;
  }

  //  Read the data into  buffer
  int read_len = 0;
  while (read_len < content_length) {
    int ret =
        esp_http_client_read(client, (char*)response_buffer + read_len, content_length - read_len);
    if (ret <= 0) {
      ESP_LOGE(TAG, "Error reading data stream");
      break;
    }
    read_len += ret;
  }

  ESP_LOGI(TAG, "HTTP GET completed with status code: %d", status_code);

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  return response_buffer;
}
