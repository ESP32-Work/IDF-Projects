/* main/main.c */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_server.h"
#include "esp_littlefs.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>




#define WIFI_SSID      "Testwifi"
#define WIFI_PASS      "x11y22z33"
#define WIFI_MAXIMUM_RETRY  5

static const char *TAG = "wifi_station";
static int s_retry_num = 0;

/* HTTP Get Handler */
static esp_err_t file_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Requested URI: %s", req->uri);  // Debugging line

    char filepath[128];
    const char *base_path = "/littlefs";

    snprintf(filepath, sizeof(filepath), "%s%.100s", base_path, req->uri);

    if (strcmp(req->uri, "/") == 0) {
        strcpy(filepath, "/littlefs/index.html");
    }

    ESP_LOGI(TAG, "Opening file: %s", filepath);  // Debugging line

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        ESP_LOGE(TAG, "File Not Found: %s", filepath);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    const char *ext = strrchr(filepath, '.');
    if (ext) {
        if (strcmp(ext, ".css") == 0) {
            httpd_resp_set_type(req, "text/css");
        } else if (strcmp(ext, ".html") == 0) {
            httpd_resp_set_type(req, "text/html");
        } else if (strcmp(ext, ".js") == 0) {
            httpd_resp_set_type(req, "application/javascript");
        }
    }

    char buffer[1024];
    ssize_t read_bytes;
    while ((read_bytes = read(fd, buffer, sizeof(buffer))) > 0) {
        httpd_resp_send_chunk(req, buffer, read_bytes);
    }
    close(fd);
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}



/* URI handler structure for GET */
// Handler for serving index.html when accessing root "/"
static esp_err_t root_get_handler(httpd_req_t *req) {
    return file_get_handler(req);  // Reuse the file serving logic
}

// Register both "/" and "*" separately
static void register_http_server(httpd_handle_t server) {
    static const httpd_uri_t root_page = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_get_handler,
        .user_ctx  = NULL
    };

    static const httpd_uri_t file_server = {
        .uri       = "*",  // Matches all other files
        .method    = HTTP_GET,
        .handler   = file_get_handler,
        .user_ctx  = NULL
    };

    httpd_register_uri_handler(server, &root_page);
    httpd_register_uri_handler(server, &file_server);
}




/* Function to start the web server */
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        // Call this inside `start_webserver()`
        register_http_server(server);
        ESP_LOGI(TAG, "Server started");
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

/* WiFi event handler */
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        
        // Start the web server
        start_webserver();
    }
}

/* WiFi Station initialization */
void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &event_handler,
                                                      NULL,
                                                      &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &event_handler,
                                                      NULL,
                                                      &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

void init_littlefs(void) {
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "littlefs",
        .format_if_mount_failed = true,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "LittleFS mounted successfully");
    }
}


void list_littlefs_files(void) {
    DIR *dir = opendir("/littlefs");
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open /littlefs");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        ESP_LOGI(TAG, "Found file: %s", entry->d_name);
    }
    closedir(dir);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    init_littlefs();  // Initialize 
    list_littlefs_files();  // List files in LittleFS

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}