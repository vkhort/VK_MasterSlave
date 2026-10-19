#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <HTTPClient.h>

#include "config.h"
#include "config_task.h"
#include "html_const.h"

enum class FSType {
    LITTLE_FS,
    SPIFFS_FS
};

// ==================== УНИВЕРСАЛЬНЫЙ КЛАСС ДЛЯ РАБОТЫ С ФАЙЛАМИ ЦЕНТРА ====================
class WorkFS {
private:
    String configPath;
    String tempPath;
    FSType currentFS;
    bool _initialized;

    bool writeRawText(const String& path, const String& content);
    String readRawText(const String& path);

public:
    // Структура хранит настройки самой точки доступа и адреса 4-х подчиненных модулей
    struct ConfigData {
        String apSsid;
        String apPass;

        ConfigData() {
            apSsid = DEFAULT_AP_SSID;
            apPass = DEFAULT_AP_PASS;
        }
    };

    WorkFS(const String& configFile = CONFIG_FILE_PATH);
    
    bool begin(FSType type = FSType::LITTLE_FS, bool formatOnFail = true);
    bool formatFS();
    
    bool exists();                                    
    bool createConfigIfNotExists();                    
    bool loadConfig(ConfigData &data);                
    bool saveConfig(const ConfigData &data);          
    bool updateValue(const String& key, const String& value); 

    void setDefaults(ConfigData &data);
    bool deleteFS();
    size_t getFileSize();
    void printDirectory(const String& path = "/", uint8_t numSpaces = 2);
};

// ==================== СЕТЕВОЙ КЛАСС ЦЕНТРА И ТРАНСЛЯЦИИ КОМАНД ====================
class WiFiConnect {
public:
    enum class Mode { STA_MODE, AP_MODE, DISCONNECTED };
    enum class Status { IDLE, CONNECTING, CONNECTED, FAILED, AP_ACTIVE };
    
    WiFiConnect();
    
    // Метод запуска Центра (поднимает точку доступа на основе конфига)
    bool setupHub(WorkFS& fs, WorkFS::ConfigData& config);
    
    void handle(); // Обслуживает запросы браузера к Центру
    
    String getAPIPAddress();
    Mode getMode();
    Status getStatus();

private:
    Mode currentMode;
    Status currentStatus;
    
    String hubSSID;
    String hubPassword;
    
    // Сетевые паспорта четырех плат для отправки команд
    String targetSlave1;
    String targetSlave2;
    String targetSlave3;
    String targetSlave4;
    
    WorkFS* fsRef;
    WebServer server;

    void startHubAP();
    void setupWebServer();
    void handleRootPage();
    void handleSaveConfig();
    
    // ✅ МЕТОД ФОРВАРДИНГА: ловит клики с веб-страницы Центра и шлет HTTP-пакеты исполнителям
    void handleApiForward();
    
    // Внутренняя утилита отправки GET запроса по Wi-Fi
    void sendGetRequest(const String& url);
};
