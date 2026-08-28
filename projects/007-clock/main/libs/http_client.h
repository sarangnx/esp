#pragma once

#include "esp_http_client.h"

#include <functional>
#include <string>

using Callback = std::function<void(int status_code, const std::string& body, esp_err_t err)>;

class HttpClient {
  private:
  Callback callback;

  std::string response_body;
  int status_code = 0;

  uint8_t* response_buffer = NULL;

  static const char* TAG;
  static esp_err_t event_handler(esp_http_client_event_t* evt);

  public:
  void get(std::string url, Callback _callback);
  std::string get(std::string url);

  uint8_t* getBuffered(std::string url, int* file_size);
};
