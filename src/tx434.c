#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "packet.h"
#include "driver/gpio.h"
#include "sharedtemp.h"
#include "tx434.h"

#define TEMP_TIMEOUT 300
#define TEMP_ERRVAL -50.0
#define TXPIN GPIO_NUM_5
#define LEDPIN GPIO_NUM_2

static const char *TAG = "tx434";

static bool preamble_bits[4] = {0,0,0,0};
static uint16_t bit_length = 8;
static uint16_t zero_bit_length = 2;
static uint16_t one_bit_length = 6;
static uint16_t packet_gap = 29-12;

static void transmit(bool state) {
  uint16_t l = state ? one_bit_length : zero_bit_length;

  gpio_set_level(TXPIN, 1);
  
  vTaskDelay(l / portTICK_PERIOD_MS);

  gpio_set_level(TXPIN, 0);

  vTaskDelay((bit_length - l) / portTICK_PERIOD_MS);
}

static void transmit_array(bool *ar, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    transmit(ar[i]);
  }
}

static void gap() {
  gpio_set_level(TXPIN, 0);

  vTaskDelay(packet_gap / portTICK_PERIOD_MS);
}

static void led_on() {
  gpio_set_level(LEDPIN, 1);
}

static void led_off() {
  gpio_set_level(LEDPIN, 0);
}

void tx434_task(void *pvParameters)
{
    bool packet[PACKET_SIZE];

    ESP_LOGI(TAG, "Configuring");
    gpio_pad_select_gpio(LEDPIN);
    gpio_set_direction(LEDPIN, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(TXPIN);
    gpio_set_direction(TXPIN, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "Entering loop");
    while (1) {
        float temp_to_send = TEMP_ERRVAL;
        time_t now;
        if (temp_set) {
            time(&now);
            if (last_ts + TEMP_TIMEOUT >= now) {
                temp_to_send = current_temp;
            }
        }
        init_with_temp(packet, temp_to_send, false, 1);
        ESP_LOGI(TAG, "Sending: %.01f", temp_to_send);
        led_on();

        transmit_array(preamble_bits, 4);
        transmit_array(preamble_bits, 4);
        transmit_array(packet, PACKET_SIZE);
        gap();
        transmit_array(preamble_bits, 4);
        transmit_array(packet, PACKET_SIZE);
        gap();
        transmit_array(preamble_bits, 4);
        transmit_array(packet, PACKET_SIZE);

        led_off();
        ESP_LOGI(TAG, "Going to sleep");
        vTaskDelay((179 * 1000) / portTICK_PERIOD_MS);
    }
}
