#ifndef CONFIG_H
#define CONFIG_H

// ==================== НАСТРОЙКИ ФАЙЛОВОЙ СИСТЕМЫ ====================
#define CONFIG_FILE_PATH     "/config.txt"   // Путь к файлу конфигурации

// ==================== НАСТРОЙКИ ПО УМОЛЧАНИЮ ====================
#define AP_SSID     "VK_STEPPER1"
#define AP_PASSWORD "12345678"
#define CONNECT_SSID     "VK_MASTER_HUB"
#define CONNECT_PASSWORD "12345678"

// ==================== НАСТРОЙКИ СВЯЗИ ПО УМОЛЧАНИЮ ====================
// Было: #define DEFAULT_MDNS_NAME    "vk_stepper1"
#define DEFAULT_MDNS_NAME    "192.168.4.2"         // Наш дефолтный IP адрес для живого конфига
// ==================== СЕТЕВАЯ АРХИТЕКТУРА ХАБА ====================
#define MASTER_GATEWAY       IPAddress(192, 168, 4, 1)   // IP-адрес самого Мастер Хаба
#define MASTER_SUBNET        IPAddress(255, 255, 255, 0) // Маска подсети локальной связи

// ==================== СИСТЕМНЫЕ ПАРАМЕТРЫ ====================
#define WIFI_CONNECT_TIMEOUT 10000                 // Таймаут подключения к Wi-Fi (мс)
#define DEBUG_MODE           1                     // 1 - включить вывод в Serial, 0 - выключить


#endif // CONFIG_H