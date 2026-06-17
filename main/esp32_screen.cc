extern "C"
{
#include "esp32_screen.h"



static uint8_t image_buffer[IMAGE_BUFFER_SIZE];

// OLED命令结构体
typedef struct {
    uint8_t addr;
    uint8_t data;
} oled_cmd_t;

// OLED初始化命令
static const oled_cmd_t oled_init_cmds[] = {
    {0x00, 0xAE}, {0x00, 0xD5}, {0x00, 0x80},
    {0x00, 0xA8}, {0x00, 0x1F}, {0x00, 0xD3},
    {0x00, 0x00}, {0x00, 0x40}, {0x00, 0x8D},
    {0x00, 0x14}, {0x00, 0x20}, {0x00, 0x00},
    {0x00, 0xA1}, {0x00, 0xC8}, {0x00, 0xDA},
    {0x00, 0x02}, {0x00, 0x81}, {0x00, 0x8F},
    {0x00, 0xD9}, {0x00, 0xF1}, {0x00, 0xDB},
    {0x00, 0x40}, {0x00, 0xA4}, {0x00, 0xA6},
    {0x00, 0xAF}
};

static esp_err_t oled_write_cmd2(uint8_t cmd) {
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x00, true);
    i2c_master_write_byte(cmd_handle, cmd, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(OLED_I2C_PORT, cmd_handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

static esp_err_t oled_write_data2(uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x40, true);
    i2c_master_write(cmd_handle, data, len, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(OLED_I2C_PORT, cmd_handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

void oled_init2(void) {
    ESP_LOGI(TAG, "初始化OLED (128x64)");
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = OLED_I2C_SDA_PIN,
        .scl_io_num = OLED_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = OLED_I2C_FREQ_HZ},
    };
    
    ESP_ERROR_CHECK(i2c_param_config(OLED_I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(OLED_I2C_PORT, conf.mode, 0, 0, 0));
    
    for (int i = 0; i < sizeof(oled_init_cmds) / sizeof(oled_cmd_t); i++) {
        oled_write_cmd2(oled_init_cmds[i].data);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    uint8_t clear_buf[SCREEN_WIDTH] = {0};
    for (int page = 0; page < SCREEN_PAGES; page++) {
        oled_write_cmd2(0xB0 + page);
        oled_write_cmd2(0x00);
        oled_write_cmd2(0x10);
        oled_write_data2(clear_buf, SCREEN_WIDTH);
    }
    
    ESP_LOGI(TAG, "OLED初始化完成");
}

void oled_display_image(uint8_t *buffer) {
    for (int page = 0; page < SCREEN_PAGES; page++) {
        oled_write_cmd2(0xB0 + page);
        oled_write_cmd2(0x00);
        oled_write_cmd2(0x10);
        oled_write_data2(buffer + page * SCREEN_WIDTH, SCREEN_WIDTH);
    }
}

void oled_display_text(const char* line1, const char* line2) {
    memset(image_buffer, 0, IMAGE_BUFFER_SIZE);
    
    if (line1) {
        int len = strlen(line1);
        for (int i = 0; i < len && i < 16; i++) {
            for (int j = 0; j < 8; j++) {
                if (i * 8 + j < SCREEN_WIDTH) {
                    image_buffer[SCREEN_WIDTH + i * 8 + j] = 0xFF;
                }
            }
        }
    }
    
    if (line2) {
        int len = strlen(line2);
        for (int i = 0; i < len && i < 16; i++) {
            for (int j = 0; j < 8; j++) {
                if (i * 8 + j < SCREEN_WIDTH) {
                    image_buffer[SCREEN_WIDTH * 2 + i * 8 + j] = 0xFF;
                }
            }
        }
    }
    
    oled_display_image(image_buffer);
}

void tcp_client_task(void *pvParameters) {
    char rx_buffer[128];
    EventBits_t bits;
    while (1) {
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "创建socket失败");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        
        struct hostent *server_host = gethostbyname(SERVER_HOST);
        if (server_host == NULL) {
            ESP_LOGE(TAG, "DNS解析失败: %s", SERVER_HOST);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        memcpy(&server_addr.sin_addr, server_host->h_addr, server_host->h_length);
        
        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            ESP_LOGE(TAG, "连接失败");
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        ESP_LOGI(TAG, "已连接到服务器");
        oled_display_text("Connected!", "Receiving...");
        
        while (1) {
             bits = xEventGroupWaitBits(wifi_event_group, 
                                   WIFI_CONNECTED_BIT | TASK_STOP_BIT,
                                   pdFALSE, pdFALSE, portMAX_DELAY);
        
                // 检查是否收到停止信号
                if (bits & TASK_STOP_BIT) {
                    ESP_LOGI(TAG, "任务收到停止信号");
                    break;
                }
            //ESP_LOGI(TAG, "1");
            uint8_t len_buf[2];
            int len_recv = recv(sock, len_buf, 2, 0);
            if (len_recv <= 0) {
                ESP_LOGW(TAG, "服务器断开连接");
                break;
            }
           
            
            uint16_t data_len = (len_buf[0] << 8) | len_buf[1];
            
            if (data_len != IMAGE_BUFFER_SIZE) {
                ESP_LOGW(TAG, "数据长度错误: %d", data_len);
                while (recv(sock, len_buf, 1, 0) > 0);
                continue;
            }
            
            size_t total_recv = 0;
            while (total_recv < data_len) {
                int recv_len = recv(sock, image_buffer + total_recv, 
                                   data_len - total_recv, 0);
                if (recv_len <= 0) {
                    ESP_LOGE(TAG, "接收数据失败");
                    break;
                }
                total_recv += recv_len;
            }
            
            if (total_recv == data_len) {
                oled_display_image(image_buffer);
            }
        }

       
        close(sock);
        if (xEventGroupGetBits(wifi_event_group) & TASK_STOP_BIT) {
            ESP_LOGI(TAG, "任务收到停止信号2");
           
            break;
        }
        oled_display_text("Disconnected", "Reconnecting...");
        vTaskDelay(pdMS_TO_TICKS(5000));

    } vTaskDelete(NULL);/*
    while (1){
         ESP_LOGI(TAG, "停止...");
         vTaskDelay(pdMS_TO_TICKS(100));
    };*/
}
}