#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "app_wifi.h"
#include "sharedtemp.h"
#include "tx434.h"

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
static const char *TAG = "temp_client";

#define TEMP_REFRESH 30

static void fetch_temperature()
{
    char *buffer = malloc(MAX_HTTP_RECV_BUFFER);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }
    esp_http_client_config_t config = {
        .url = "http://tempserver.local:8000/dashtemp",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        free(buffer);
        return;
    }
    int content_length =  esp_http_client_fetch_headers(client);
    int total_read_len = 0, read_len;
    if (total_read_len < content_length && content_length <= MAX_HTTP_RECV_BUFFER) {
        read_len = esp_http_client_read(client, buffer, content_length);
        if (read_len <= 0) {
            ESP_LOGE(TAG, "Error read data");
        }
        buffer[read_len] = 0;
        ESP_LOGD(TAG, "read_len = %d", read_len);
    }
    ESP_LOGI(TAG, "HTTP Stream reader Status = %d, content_length = %d",
                    esp_http_client_get_status_code(client),
                    esp_http_client_get_content_length(client));
    if (esp_http_client_get_status_code(client) == 200) {
        ESP_LOGI(TAG, "Received: %.*s", content_length, buffer);
        if (content_length >= 3) {
            current_temp = atof(buffer);
            time((time_t *) &last_ts);
            temp_set = true;
            ESP_LOGI(TAG, "Set: %.01f @ %ld", current_temp, last_ts);
        }
    } else {
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(buffer);
}

static void temp_client_task(void *pvParameters)
{
    while (1) {
        ESP_LOGI(TAG, "Waiting for WIFI...");
        app_wifi_wait_connected();
        ESP_LOGI(TAG, "Connected to AP, begin http comm");
        fetch_temperature();
        ESP_LOGI(TAG, "Sleeping");
        vTaskDelay(TEMP_REFRESH * 1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Again");
    }
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    app_wifi_initialise();

    xTaskCreate(&temp_client_task, "temp_client_task", 8192, NULL, 5, NULL);
    xTaskCreate(&tx434_task, "tx434_task", 8192, NULL, 20, NULL);
}
