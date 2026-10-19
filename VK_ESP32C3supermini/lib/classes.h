#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WebServer.h> // ← Добавляем встроенный веб-сервер

#include "config.h"
#include "config_task.h"
#include "html_const.h" // ✅ Подключаем наши HTML константы
#include "VK_Servo.h"
#include "VK_Stepper.h" // ✅ Подключаем класс шаговика

enum class FSType {
    LITTLE_FS,
    SPIFFS_FS
};

// ==================== УНИВЕРСАЛЬНЫЙ КЛАСС ДЛЯ РАБОТЫ С ФАЙЛАМИ ====================
class WorkFS {
private:
    String configPath;
    String tempPath;
    FSType currentFS;
    bool _initialized;

    bool writeRawText(const String& path, const String& content);
    String readRawText(const String& path);

public:
    struct ConfigData {
        // Сеть
        String ssid;
        String password;
        String espName;
        
        // Шаговый мотор (Добавлено!)
        int stepPin;
        int dirPin;
        float motorSpeed;
        float motorAccel;
        int stepsPerRev;
        int stepDelay;

        // Сервопривод (Добавлено!)
        int servoPin;
        int servoMinPulse;
        int servoMaxPulse;
        int servoStepDelay; // ✅ Новая переменная в структуре

        ConfigData() {
            ssid = CONNECT_SSID;
            password = CONNECT_PASSWORD;
            espName = AP_SSID;
            
            stepPin = MOTOR_STEP_PIN;
            dirPin = MOTOR_DIR_PIN;
            motorSpeed = DEFAULT_MOTOR_SPEED;
            motorAccel = DEFAULT_MOTOR_ACCEL;
            stepsPerRev = DEFAULT_STEPS_REV;
            stepDelay = DEFAULT_STEP_DELAY;

            servoPin = SERVO_PIN;
            servoMinPulse = SERVO_MIN_PULSE;
            servoMaxPulse = SERVO_MAX_PULSE;
            servoStepDelay = SERVO_STEP_DELAY; // Инициализация макросом
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
    bool deleteFS(); // ✅ Явный метод удаления файла конфигурации для тестов
    size_t getFileSize();
    void printDirectory(const String& path = "/", uint8_t numSpaces = 2);
};

// ==================== КЛАСС WIFI + ВЕБ-СЕРВЕР + OTA ====================
class WiFiConnect {
public:
    enum class Mode { STA_MODE, AP_MODE, DISCONNECTED };
    enum class Status { IDLE, CONNECTING, CONNECTED, FAILED, AP_ACTIVE };
    
    WiFiConnect();
    
    bool setupWiFi(WorkFS& fs, WorkFS::ConfigData& config, unsigned long timeoutMs = WIFI_CONNECT_TIMEOUT);
    void handle(); // ✅ Теперь внутри него будет вызываться и OTA.handle(), и server.handleClient()
    
    String getIPAddress();
    String getAPIPAddress();
    bool isConnected();
    Mode getMode();
    Status getStatus();
    // ✅ Новый метод для передачи ссылки на объект сервы внутрь сетевого класса
    void attachServo(VK_Servo& servoInstance);
    void attachStepper(VK_Stepper& stepperInstance);

private:
    Mode currentMode;
    Status currentStatus;
    unsigned long connectTimeout;
    
    String targetSSID;
    String targetPassword;
    String deviceName;
    
    WorkFS* fsRef;
    WebServer server; // ✅ Объект веб-сервера на 80 порту

    void startAPMode();
    void initOTA(); 
    void setupWebServer();
    void handleRootPage();
    void handleSaveConfig();
    
    // ✅ Новые методы для обработки мгновенных пультовых команд
    void handleApiStepper();
    void handleApiServo();
    VK_Servo* servoRef; // ✅ Указатель на физический объект сервы
    VK_Stepper* stepperRef; // ✅ Указатель на физический объект шаговика
};
