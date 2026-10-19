#ifndef CONFIG_H
#define CONFIG_H

// ==================== НАСТРОЙКИ ФАЙЛОВОЙ СИСТЕМЫ ====================
#define CONFIG_FILE_PATH     "/config.txt"   // Путь к файлу конфигурации

// ==================== НАСТРОЙКИ ТОЧКИ ДОСТУПА ЦЕНТРА (AP) ====================
#define DEFAULT_AP_SSID      "VK_MASTER_HUB" // Имя сети, которую создаёт ESP32-S
#define DEFAULT_AP_PASS      "12345678"      // Пароль от этой сети

// ==================== СИСТЕМНЫЕ ПАРАМЕТРЫ ====================
#define WIFI_CONNECT_TIMEOUT 10000                 // Таймаут подключения к Wi-Fi (мс)
#define DEBUG_MODE           1                     // 1 - включить вывод в Serial, 0 - выключить


#endif // CONFIG_H