#pragma once

#include "esp_http_client.h"

#include <functional>
#include <string>

using Callback = std::function<void(int status_code, const std::string& body, esp_err_t err)>;

class HttpClient {
  private:
  esp_http_client_handle_t client = nullptr;

  std::string url;
  Callback callback;

  std::string response_body;
  int status_code = 0;

  static const char* TAG;
  static esp_err_t event_handler(esp_http_client_event_t* evt);

  public:
  HttpClient(const std::string& _url);
  ~HttpClient();

  void get(Callback _callback);
};
