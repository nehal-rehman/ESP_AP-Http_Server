// /* Hello World Example

//    This example code is in the Public Domain (or CC0 licensed, at your option.)

//    Unless required by applicable law or agreed to in writing, this
//    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.
// */
// #include <stdio.h>
// #include "sdkconfig.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_system.h"
// #include "esp_spi_flash.h"

// void app_main(void)
// {
//     printf("Hello world!\n");

//     /* Print chip information */
//     esp_chip_info_t chip_info;
//     esp_chip_info(&chip_info);
//     printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
//             CONFIG_IDF_TARGET,
//             chip_info.cores,
//             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
//             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

//     printf("silicon revision %d, ", chip_info.revision);

//     printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
//             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

//     printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

//     for (int i = 10; i >= 0; i--) {
//         printf("Restarting in %d seconds...\n", i);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
//     printf("Restarting now.\n");
//     fflush(stdout);
//     esp_restart();
// }

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_server.h"
#include "esp_netif.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


void wifi_init_softap() {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32-AP",
            .ssid_len = strlen("ESP32-AP"),
            .channel = 1,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen("12345678") == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 0, 1);
    IP4_ADDR(&ip_info.gw, 192, 168, 0, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

    // Get the AP netif after starting Wi-Fi
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (ap_netif != NULL) {
        esp_netif_dhcps_stop(ap_netif);
        esp_netif_set_ip_info(ap_netif, &ip_info);
        esp_netif_dhcps_start(ap_netif);
    }

    ESP_LOGI("wifi_init_softap", "Wi-Fi AP set up with IP: 192.168.0.1");
}

esp_err_t get_handler(httpd_req_t *req) {
    const char* resp_str = 
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>NEHAL ESP32 AP</title>"
        "<style>"
        "body { display: flex; justify-content: center; align-items: center; height: 100vh; flex-direction: column; }"
        "form, h1 { text-align: center; }"
        "h1 { font-weight: bold; }"
        "</style>"
        "</head>"
        "<body>"
        "<h1>ROMAN TEST PRJ - ESP-IDF</h1>"
        "<form action=\"/submit\" method=\"post\">"
        "First name:<br>"
        "<input type=\"text\" name=\"firstname\"><br>"
        "Last name:<br>"
        "<input type=\"text\" name=\"lastname\"><br><br>"
        "<input type=\"submit\" value=\"Save/Send\">"
        "</form>"
        "</body>"
        "</html>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}



esp_err_t post_handler(httpd_req_t *req) {
    char content[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, content, MIN(remaining, sizeof(content)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
        ESP_LOGI("POST", "Received: %.*s", ret, content);
    }

    httpd_resp_send(req, "Data received", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t get_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = get_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t post_uri = {
        .uri       = "/submit",
        .method    = HTTP_POST,
        .handler   = post_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &get_uri);
        httpd_register_uri_handler(server, &post_uri);
    }
}

void app_main() {
    wifi_init_softap();
    start_webserver();
}
